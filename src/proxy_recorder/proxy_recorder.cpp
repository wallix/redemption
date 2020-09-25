/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): David Fort

   A proxy that will capture all the traffic to the target
*/

#include "proxy_recorder/proxy_recorder.hpp"

#include "proxy_recorder/nla_tee_transport.hpp"
#include "proxy_recorder/nego_client.hpp"
#include "proxy_recorder/nego_server.hpp"

#include "core/RDP/mcs.hpp"
#include "core/RDP/gcc/userdata/cs_core.hpp"
#include "core/RDP/gcc/userdata/sc_core.hpp"
#include "core/RDP/gcc.hpp"
#include "core/server_notifier_api.hpp"


ProxyRecorder::ProxyRecorder(
    NlaTeeTransport & back_nla_tee_trans,
    RecorderFile & outFile,
    TimeBase & time_base,
    const char * host,
    bool enable_kerberos,
    uint64_t verbosity
)
: back_nla_tee_trans(back_nla_tee_trans)
, outFile(outFile)
, time_base(time_base)
, host(host)
, enable_kerberos(enable_kerberos)
, verbosity(verbosity)
{
    this->frontBuffer.trace_pdu = (this->verbosity > 512);
    this->backBuffer.trace_pdu = (this->verbosity > 512);
}

ProxyRecorder::~ProxyRecorder() = default;

void ProxyRecorder::front_step1(Transport & frontConn)
{
    LOG(LOG_INFO, "front step 1");

    LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_FRONT_STEP1 frontbuffer content ======");
    writable_u8_array_view currentPacket = this->frontBuffer.current_pdu_buffer();
    LOG_IF(this->verbosity > 512, LOG_INFO, ">>>>>>>> NEGOCIATING_FRONT_STEP1 frontbuffer content >>>>>>");

    InStream x224_stream(currentPacket);
    this->front_CR_TPDU = X224::CR_TPDU_Data_Recv(x224_stream, false, this->verbosity);
    if (this->front_CR_TPDU._header_size != x224_stream.get_capacity()) {
        LOG(LOG_WARNING,
            "Front::incoming: connection request: all data should have been consumed,"
            " %zu bytes remains",
            x224_stream.get_capacity() - this->front_CR_TPDU._header_size);
    }

    LOG_IF((this->verbosity > 8) && (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS), LOG_INFO, "TLS Front");
    LOG_IF((this->verbosity > 8) && (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID), LOG_INFO, "Hybrid (NLA) Front");

    StaticOutStream<256> front_x224_stream;
    X224::CC_TPDU_Send(
        front_x224_stream,
        X224::RDP_NEG_RSP,
        RdpNego::EXTENDED_CLIENT_DATA_SUPPORTED,
        (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID)?X224::PROTOCOL_HYBRID:
        (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS)?X224::PROTOCOL_TLS:
        X224::PROTOCOL_RDP);
    outFile.write_packet(PacketType::DataIn, front_x224_stream.get_produced_bytes());
    frontConn.send(front_x224_stream.get_produced_bytes());

    if ((this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS)
    || (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID)) {
        frontConn.enable_server_tls("inquisition", nullptr, 0 /* tls_min_level */, 0  /* tls_max_level */, true);
    }
}

void ProxyRecorder::back_step1(writable_u8_array_view key, Transport & backConn, std::string const& nla_username, std::string nla_password)
{
    if (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID) {
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "start NegoServer");
        }
        this->nego_server = std::make_unique<NegoServer>(key, this->time_base, this->verbosity > 8);
        this->pstate = PState::NEGOCIATING_FRONT_NLA;
    }
    else {
        this->pstate = PState::NEGOCIATING_BACK_NLA;
    }

    TLSClientParams tls_client_params;
    tls_client_params.tls_min_level = 0;
    tls_client_params.tls_max_level = 0;
    tls_client_params.show_common_cipher_list = false;
    nla_password += '\0';
    // tls_client_params.cipher_string;
    this->nego_client = std::make_unique<NegoClient>(
        !nla_username.empty(),
        this->front_CR_TPDU.cinfo.flags & X224::RESTRICTED_ADMIN_MODE_REQUIRED,
        this->back_nla_tee_trans, this->time_base,
        this->host, nla_username.c_str(),
        nla_password.c_str(),
        enable_kerberos, tls_client_params,
        safe_cast<RdpNego::Verbose>(uint32_t(this->verbosity >> 32)));

    // equivalent to nego_client->send_negotiation_request()
    StaticOutStream<256> back_x224_stream;
    X224::CR_TPDU_Send(
        back_x224_stream,
        this->front_CR_TPDU.cookie.data,
        this->front_CR_TPDU.rdp_neg_type,
        this->front_CR_TPDU.rdp_neg_flags,
        !nla_username.empty() ? X224::PROTOCOL_HYBRID : X224::PROTOCOL_TLS);

    outFile.write_packet(PacketType::DataOut, back_x224_stream.get_produced_bytes());
    backConn.send(back_x224_stream.get_produced_bytes());
}

void ProxyRecorder::front_nla(Transport & frontConn)
{
    LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_FRONT_NLA frontbuffer content ======");

    TpduBuffer & buffer = this->frontBuffer;
    std::vector<uint8_t> result;
    credssp::State st = credssp::State::Cont;
    while ((this->nego_server->credssp.ntlm_state == NTLM_STATE_WAIT_PASSWORD
                || buffer.next(TpduBuffer::CREDSSP))
            && credssp::State::Cont == st) {
        if (this->nego_server->credssp.ntlm_state == NTLM_STATE_WAIT_PASSWORD){
            result << this->nego_server->credssp.authenticate_next({});
        }
        else {
            result << this->nego_server->credssp.authenticate_next(buffer.current_pdu_buffer());
        }
        st = this->nego_server->credssp.state;
    }

    frontConn.send(result);

    switch (st) {
    case credssp::State::Err: throw Error(ERR_NLA_AUTHENTICATION_FAILED);
    case credssp::State::Cont: break;
    case credssp::State::Finish:
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "stop NegoServer");
            LOG(LOG_INFO, "start NegoClient");
        }
        this->nego_server.reset();
        this->pstate = PState::NEGOCIATING_BACK_NLA;
        break;
    }
}


void ProxyRecorder::front_initial_pdu_negociation(Transport & backConn, bool is_nla)
{
    if (this->frontBuffer.next(TpduBuffer::PDU)) {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_INITIAL_PDU : front receive : frontbuffer content ======");
        writable_u8_array_view currentPacket = this->frontBuffer.current_pdu_buffer();
        LOG_IF(this->verbosity > 512, LOG_INFO, ">>>>>>>> NEGOCIATING_INITIAL_PDU frontbuffer content >>>>>>");

        if (is_nla) {
            if (this->verbosity > 4) {
                LOG(LOG_INFO, "Back: force protocol PROTOCOL_HYBRID");
            }
            InStream new_x224_stream(currentPacket);
            X224::DT_TPDU_Recv x224(new_x224_stream);
            MCS::CONNECT_INITIAL_PDU_Recv mcs_ci(x224.payload, MCS::BER_ENCODING);
            GCC::Create_Request_Recv gcc_cr(mcs_ci.payload);
            GCC::UserData::RecvFactory f(gcc_cr.payload);
            // force X224::PROTOCOL_HYBRID
            if (f.tag == CS_CORE) {
                GCC::UserData::CSCore cs_core;
                cs_core.recv(f.payload);
                if (cs_core.length >= 216) {
                    auto const idx = f.payload.get_current() - currentPacket.data() + (216-cs_core.length) - 4;
                    currentPacket[idx] = X224::PROTOCOL_HYBRID;
                }
            }
        }

        outFile.write_packet(PacketType::DataOut, this->frontBuffer.remaining_data());
        backConn.send(this->frontBuffer.remaining_data());

        this->pstate = PState::NEGOCIATING_BACK_INITIAL_PDU;
    }
}

void ProxyRecorder::back_nla_negociation(Transport & backConn)
{
    LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_BACK_NLA : front receive : frontbuffer content ======");

    NullServerNotifier null_notifier;
    if (not this->nego_client->recv_next_data(backBuffer, null_notifier)) {
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "stop NegoClient");
        }
        this->nego_client.reset();
        this->pstate = PState::NEGOCIATING_FRONT_INITIAL_PDU;
        outFile.write_packet(PacketType::ClientCert, backConn.get_public_key());
    }
}

void ProxyRecorder::back_initial_pdu_negociation(Transport & frontConn, bool is_nla)
{
    if (backBuffer.next(TpduBuffer::PDU)) {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== BACK_INITIAL_PDU_NEGOCIATION  : back receive : backbuffer content ======");
        writable_u8_array_view currentPacket = backBuffer.current_pdu_buffer();
        LOG_IF(this->verbosity > 512, LOG_INFO, ">>>>>>>> BACK_INITIAL_PDU_NEGOCIATION backbuffer content >>>>>>");

        if (is_nla) {
            if (this->verbosity > 4) {
                LOG(LOG_INFO, "Front: force protocol tls=%u nla=%u",
                    (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS),
                    (this->front_CR_TPDU.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID));
            }
            InStream new_x224_stream(currentPacket);
            X224::DT_TPDU_Recv x224(new_x224_stream);
            MCS::CONNECT_RESPONSE_PDU_Recv mcs(x224.payload, MCS::BER_ENCODING);
            GCC::Create_Response_Recv gcc_cr(mcs.payload);
            GCC::UserData::RecvFactory f(gcc_cr.payload);
            if (f.tag == SC_CORE) {
                GCC::UserData::SCCore sc_core;
                sc_core.recv(f.payload);
                if (sc_core.length >= 12) {
                    hexdump_d(f.payload.get_consumed_bytes());
                    auto const offset = (sc_core.length >= 16) ? 8 : 4;
                    auto const idx = f.payload.get_current() - currentPacket.data() - offset;
                    currentPacket[idx] = this->front_CR_TPDU.rdp_neg_requestedProtocols & 0xFF;
                    hexdump_d(f.payload.get_consumed_bytes());
                }
            }
        }

        outFile.write_packet(PacketType::DataIn, backBuffer.remaining_data());
       frontConn.send(backBuffer.remaining_data());

        this->pstate = PState::FORWARD;
    }
}

