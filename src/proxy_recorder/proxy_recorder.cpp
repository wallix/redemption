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

void ProxyRecorder::front_step1()
{
    if (frontBuffer.next(TpduBuffer::PDU)) {

        LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_FRONT_STEP1 frontbuffer content ======");
        array_view_u8 currentPacket = frontBuffer.current_pdu_buffer();
        if (this->verbosity > 2048){
            ::hexdump_av_d(currentPacket);
        }
        LOG_IF(this->verbosity > 2048, LOG_INFO, ">>>>>>>> NEGOCIATING_FRONT_STEP1 frontbuffer content >>>>>>");

        InStream x224_stream(currentPacket);
        X224::CR_TPDU_Recv x224(x224_stream, true);
        if (x224._header_size != x224_stream.get_capacity()) {
            LOG(LOG_WARNING,
                "Front::incoming: connection request: all data should have been consumed,"
                " %zu bytes remains",
                x224_stream.get_capacity() - x224._header_size);
        }

        is_tls_client = (x224.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS);
        is_nla_client = (x224.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID);

        StaticOutStream<256> front_x224_stream;
        X224::CC_TPDU_Send(
            front_x224_stream,
            X224::RDP_NEG_RSP,
            RdpNego::EXTENDED_CLIENT_DATA_SUPPORTED,
            select_client_protocol());
        outFile.write_packet(PacketType::DataIn, front_x224_stream.get_bytes());
        frontConn.send(front_x224_stream.get_bytes());

        if (is_tls_client || is_nla_client) {
            frontConn.enable_server_tls("inquisition", nullptr, 0);
        }

        if (is_nla_client) {
            if (this->verbosity > 4) {
                LOG(LOG_INFO, "start NegoServer");
            }
            nego_server = std::make_unique<NegoServer>(
                frontConn, outFile, nla_username, nla_password, this->verbosity > 8);

            rdpCredsspServer::State st = nego_server->recv_data(frontBuffer);

            if (rdpCredsspServer::State::Err == st) {
                throw Error(ERR_NLA_AUTHENTICATION_FAILED);
            }
        }

        nego_client = std::make_unique<NegoClient>(
            !nla_username.empty(),
            x224.rdp_cinfo_flags & X224::RESTRICTED_ADMIN_MODE_REQUIRED,
            backConn, outFile,
            host.c_str(), nla_username.c_str(),
            nla_password.empty() ? "\0" : nla_password.c_str(),
            enable_kerberos, this->verbosity > 8);

        // equivalent to nego_client->send_negotiation_request()
        StaticOutStream<256> back_x224_stream;
        X224::CR_TPDU_Send(
            back_x224_stream, x224.cookie, x224.rdp_neg_type, x224.rdp_neg_flags,
            !nla_username.empty() ? X224::PROTOCOL_HYBRID : X224::PROTOCOL_TLS);
        outFile.write_packet(PacketType::DataOut, back_x224_stream.get_bytes());
        backConn.send(back_x224_stream.get_bytes());

        this->pstate = nego_server ? NEGOCIATING_FRONT_NLA : NEGOCIATING_BACK_NLA;
    }
}

void ProxyRecorder::front_nla()
{
    rdpCredsspServer::State st = nego_server->recv_data(frontBuffer);
    switch (st) {
    case rdpCredsspServer::State::Err: throw Error(ERR_NLA_AUTHENTICATION_FAILED);
    case rdpCredsspServer::State::Cont: break;
    case rdpCredsspServer::State::Finish:
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "stop NegoServer");
            LOG(LOG_INFO, "start NegoClient");
        }
        nego_server.reset();
        this->pstate = NEGOCIATING_BACK_NLA;
        break;
    }
    
    // Note by CGR:
    // We are performing the dump afterward, because when entering back_nla_negociation()
    // We don't yet know which type it will be. We could get the data from dump()
    // but another issue is that the read is not typed at transport layer. This makes
    // things more complicated and harder to read that it should be
    {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_FRONT_NLA (DELAYED) frontbuffer content ======");
        array_view_u8 currentPacket = frontBuffer.current_pdu_buffer();
        if (this->verbosity > 2048){
            ::hexdump_av_d(currentPacket);
        }
        LOG_IF(this->verbosity > 2048, LOG_INFO, ">>>>>>>> NEGOCIATING_FRONT_NLA (DELAYED)  frontbuffer content >>>>>>");
    }
}


void ProxyRecorder::front_initial_pdu_negociation()
{
    if (frontBuffer.next(TpduBuffer::PDU)) {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== NEGOCIATING_INITIAL_PDU frontbuffer content ======");
        array_view_u8 currentPacket = frontBuffer.current_pdu_buffer();
        if (this->verbosity > 2048){
            ::hexdump_av_d(currentPacket);
        }
        LOG_IF(this->verbosity > 8, LOG_INFO, ">>>>>>>> NEGOCIATING_INITIAL_PDU frontbuffer content >>>>>>");


        if (this->verbosity > 2048){
            hexdump_av_d(currentPacket);
        }
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

        outFile.write_packet(PacketType::DataOut, frontBuffer.remaining_data());
        backConn.send(frontBuffer.remaining_data());

        this->pstate = NEGOCIATING_BACK_INITIAL_PDU;
    }
}

void ProxyRecorder::back_nla_negociation()
{
    if (this->verbosity > 8) {
        LOG(LOG_INFO, "NEGOCIATING_BACK_NLA");
    }
    NullServerNotifier null_notifier;
    if (not nego_client->recv_next_data(backBuffer, null_notifier)) {
        if (this->verbosity > 4) {
            LOG(LOG_INFO, "stop NegoClient");
        }
        this->nego_client.reset();
        this->pstate = NEGOCIATING_FRONT_INITIAL_PDU;
        outFile.write_packet(PacketType::ClientCert, backConn.get_public_key());
    }
    
    // Note by CGR:
    // We are performing the dump afterward, because when entering back_nla_negociation()
    // We don't yet know which type it will be. We could get the data from dump()
    // but another issue is that the read is not typed at transport layer. This makes
    // things more complicated and harder to read that it should be
    {
        LOG(LOG_INFO, "======== NEGOCIATING_BACK_NLA (DELAYED) backbuffer content ======");
        array_view_u8 currentPacket = backBuffer.current_pdu_buffer();
        if (this->verbosity > 2048){
            ::hexdump_av_d(currentPacket);
        }
        LOG(LOG_INFO, ">>>>>>>> NEGOCIATING_BACK_NLA (DELAYED)  backbuffer content >>>>>>");
    }
}

void ProxyRecorder::back_initial_pdu_negociation()
{
    if (backBuffer.next(TpduBuffer::PDU)) {
        LOG_IF(this->verbosity > 8, LOG_INFO, "======== BACK_INITIAL_PDU_NEGOCIATION backbuffer content ======");
        array_view_u8 currentPacket = backBuffer.current_pdu_buffer();
        if (this->verbosity > 2048){
            ::hexdump_av_d(currentPacket);
        }
        LOG_IF(this->verbosity > 2048, LOG_INFO, ">>>>>>>> BACK_INITIAL_PDU_NEGOCIATION backbuffer content >>>>>>");

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


void ProxyRecorder::run()
{
    fd_set rset;
    int const front_fd = frontConn.get_fd();
    int const back_fd = backConn.get_fd();

    frontConn.enable_trace = this->verbosity > 256;
    backConn.enable_trace = this->verbosity > 256;

    for (;;) {
        FD_ZERO(&rset);
        switch(this->pstate) {
        case NEGOCIATING_FRONT_NLA:
        case NEGOCIATING_FRONT_STEP1:
        case NEGOCIATING_FRONT_INITIAL_PDU:
            // Negotiation with back delayed until front finished
            FD_SET(front_fd, &rset);
            break;
        case NEGOCIATING_BACK_NLA:
        case NEGOCIATING_BACK_INITIAL_PDU:
            // Now start negociation with back
            // FIXME: use front NLA parameters!
            FD_SET(back_fd, &rset);
            break;
        case FORWARD:
            FD_SET(front_fd, &rset);
            FD_SET(back_fd, &rset);
            break;
        }

        int status = select(std::max(front_fd, back_fd) + 1, &rset, nullptr, nullptr, nullptr);
        if (status < 0) {
            break;
        }

        switch(this->pstate) {
        case NEGOCIATING_FRONT_NLA:
            if (FD_ISSET(front_fd, &rset)) {
                frontBuffer.load_data(this->frontConn);
                this->front_nla();
            }
            break;

        case NEGOCIATING_FRONT_STEP1:
            if (FD_ISSET(front_fd, &rset)) {
                frontBuffer.load_data(this->frontConn);
                this->front_step1();
            }
            break;

        case NEGOCIATING_FRONT_INITIAL_PDU:
            if (FD_ISSET(front_fd, &rset)) {
                frontBuffer.load_data(this->frontConn);
                this->front_initial_pdu_negociation();
            }
            break;

        case NEGOCIATING_BACK_NLA:
            if (FD_ISSET(back_fd, &rset)) {
                this->back_nla_negociation();
            }
            break;

        case NEGOCIATING_BACK_INITIAL_PDU:
            // Now start negociation with back
            // FIXME: use front NLA parameters!
            if (FD_ISSET(back_fd, &rset)) {
                backBuffer.load_data(this->backConn);
                this->back_initial_pdu_negociation();
            }
            break;

        case FORWARD:
            if (FD_ISSET(front_fd, &rset)) {
                if (this->verbosity > 128) {
                    LOG(LOG_INFO, "FORWARD (FRONT TO BACK)");
                }
                uint8_t tmpBuffer[0xffff];
                size_t ret = frontConn.partial_read(make_array_view(tmpBuffer));
                if (ret > 0) {
                    outFile.write_packet(PacketType::DataOut, {tmpBuffer, ret});
                    backConn.send(tmpBuffer, ret);
                }
            }
            if (FD_ISSET(back_fd, &rset)) {
                if (this->verbosity > 128) {
                    LOG(LOG_INFO, "FORWARD (BACK to FRONT)");
                }
                uint8_t tmpBuffer[0xffff];
                size_t ret = backConn.partial_read(make_array_view(tmpBuffer));
                if (ret > 0) {
                    frontConn.send(tmpBuffer, ret);
                    outFile.write_packet(PacketType::DataIn, {tmpBuffer, ret});
                }
            }
            break;
        }
    }
}


