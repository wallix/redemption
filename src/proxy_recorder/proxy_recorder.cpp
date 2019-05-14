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


void ProxyRecorder::front_step1(Transport & frontConn)
{
    LOG(LOG_INFO, "front step 1");

    LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_FRONT_STEP1 frontbuffer content ======");
    array_view_u8 currentPacket = this->frontBuffer.current_pdu_buffer();
    LOG_IF(this->verbosity > 512, LOG_INFO, ">>>>>>>> NEGOCIATING_FRONT_STEP1 frontbuffer content >>>>>>");

    InStream x224_stream(currentPacket);
    X224::CR_TPDU_Recv x224(x224_stream, false, this->verbosity);
    if (x224._header_size != x224_stream.get_capacity()) {
        LOG(LOG_WARNING,
            "Front::incoming: connection request: all data should have been consumed,"
            " %zu bytes remains",
            x224_stream.get_capacity() - x224._header_size);
    }

    this->is_tls_client = (x224.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS);
    this->is_nla_client = (x224.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID);

    LOG_IF((this->verbosity > 8) && this->is_tls_client, LOG_INFO, "TLS Client");
    LOG_IF((this->verbosity > 8) && this->is_nla_client, LOG_INFO, "NLA Client");

    StaticOutStream<256> front_x224_stream;
    X224::CC_TPDU_Send(
        front_x224_stream,
        X224::RDP_NEG_RSP,
        RdpNego::EXTENDED_CLIENT_DATA_SUPPORTED,
        select_client_protocol());
    outFile.write_packet(PacketType::DataIn, front_x224_stream.get_bytes());
    frontConn.send(front_x224_stream.get_bytes());

    if (this->is_tls_client || this->is_nla_client) {
        frontConn.enable_server_tls("inquisition", nullptr, 0);
    }
    
    nego_client = std::make_unique<NegoClient>(
    !nla_username.empty(),
    x224.rdp_cinfo_flags & X224::RESTRICTED_ADMIN_MODE_REQUIRED,
    this->back_nla_tee_trans, this->timeobj,
    this->host, nla_username.c_str(),
    nla_password.empty() ? "\0" : nla_password.c_str(),
    enable_kerberos, this->verbosity > 8);

    // equivalent to nego_client->send_negotiation_request()
    StaticOutStream<256> back_x224_stream;
    X224::CR_TPDU_Send(
        back_x224_stream, x224.cookie, x224.rdp_neg_type, x224.rdp_neg_flags,
        !nla_username.empty() ? X224::PROTOCOL_HYBRID : X224::PROTOCOL_TLS);
    outFile.write_packet(PacketType::DataOut, back_x224_stream.get_bytes());
    this->backConn.send(back_x224_stream.get_bytes());

}
        
void ProxyRecorder::back_step1(array_view_u8 key)
{
    if (this->is_nla_client) {
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "start NegoServer");
        }
        this->nego_server = std::make_unique<NegoServer>(key, nla_username, nla_password, this->verbosity > 8);
    }

    // TODO: here I am starting back end client connection
    this->pstate = this->nego_server ? NEGOCIATING_FRONT_NLA : NEGOCIATING_BACK_NLA;
}

void ProxyRecorder::front_nla(Transport & frontConn)
{
    LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_FRONT_NLA frontbuffer content ======");
    StaticOutStream<65535> frontResponse; 
    rdpCredsspServer::State st = this->nego_server->recv_data(this->frontBuffer, frontResponse);
    frontConn.send(frontResponse.get_bytes());

    switch (st) {
    case rdpCredsspServer::State::Err: throw Error(ERR_NLA_AUTHENTICATION_FAILED);
    case rdpCredsspServer::State::Cont: break;
    case rdpCredsspServer::State::Finish:
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "stop NegoServer");
            LOG(LOG_INFO, "start NegoClient");
        }
        this->nego_server.reset();
        this->pstate = NEGOCIATING_BACK_NLA;
        break;
    }
}


void ProxyRecorder::front_initial_pdu_negociation()
{
    if (this->frontBuffer.next(TpduBuffer::PDU)) {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_INITIAL_PDU : front receive : frontbuffer content ======");
        array_view_u8 currentPacket = this->frontBuffer.current_pdu_buffer();
        LOG_IF(this->verbosity > 512, LOG_INFO, ">>>>>>>> NEGOCIATING_INITIAL_PDU frontbuffer content >>>>>>");

        if (!nla_username.empty()) {
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
        this->backConn.send(this->frontBuffer.remaining_data());

        this->pstate = NEGOCIATING_BACK_INITIAL_PDU;
    }
}

void ProxyRecorder::back_nla_negociation()
{
    LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_BACK_NLA : front receive : frontbuffer content ======");

    NullServerNotifier null_notifier;
    if (not nego_client->recv_next_data(backBuffer, null_notifier)) {
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "stop NegoClient");
        }
        this->nego_client.reset();
        this->pstate = NEGOCIATING_FRONT_INITIAL_PDU;
        outFile.write_packet(PacketType::ClientCert, this->backConn.get_public_key());
    }
}

void ProxyRecorder::back_initial_pdu_negociation(Transport & frontConn)
{
    if (backBuffer.next(TpduBuffer::PDU)) {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== BACK_INITIAL_PDU_NEGOCIATION  : back receive : backbuffer content ======");
        array_view_u8 currentPacket = backBuffer.current_pdu_buffer();
        LOG_IF(this->verbosity > 512, LOG_INFO, ">>>>>>>> BACK_INITIAL_PDU_NEGOCIATION backbuffer content >>>>>>");

        if (!nla_username.empty()) {
            if (this->verbosity > 4) {
                LOG(LOG_INFO, "Front: force protocol tls=%d nla=%d", is_tls_client, is_nla_client);
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
                    hexdump_av_d(f.payload.get_bytes());
                    auto const offset = (sc_core.length >= 16) ? 8 : 4;
                    auto const idx = f.payload.get_current() - currentPacket.data() - offset;
                    currentPacket[idx] = select_client_protocol();
                    hexdump_av_d(f.payload.get_bytes());
                }
            }
        }

        outFile.write_packet(PacketType::DataIn, backBuffer.remaining_data());
       frontConn.send(backBuffer.remaining_data());

        this->pstate = FORWARD;
    }
}




