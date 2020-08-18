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

#pragma once

#include "transport/recorder_transport.hpp"
#include "core/RDP/x224.hpp"
#include "core/RDP/tpdu_buffer.hpp"

#include <string>
#include <memory>


class NlaTeeTransport;
class NegoClient;
class NegoServer;
class TimeBase;


/** @brief a front connection with a RDP client */
class ProxyRecorder
{
    X224::CR_TPDU_Data front_CR_TPDU;

    using PacketType = RecorderFile::PacketType;

public:
    ProxyRecorder(
        NlaTeeTransport & back_nla_tee_trans,
        RecorderFile & outFile,
        TimeBase & time_base,
        const char * host,
        bool enable_kerberos,
        uint64_t verbosity
    );

    ~ProxyRecorder();

    void front_step1(Transport & frontConn);
    void back_step1(writable_u8_array_view key, Transport & backConn, std::string const& nla_username, std::string nla_password);
    void front_nla(Transport & frontConn);
    void front_initial_pdu_negociation(Transport & backConn, bool is_nla);
    void back_nla_negociation(Transport & backConn);
    void back_initial_pdu_negociation(Transport & frontConn, bool is_nla);

public:
    enum class PState : unsigned {
        NEGOCIATING_FRONT_STEP1,
        NEGOCIATING_FRONT_NLA,
        NEGOCIATING_BACK_NLA,
        NEGOCIATING_FRONT_INITIAL_PDU,
        NEGOCIATING_BACK_INITIAL_PDU,
        FORWARD
    } pstate = PState::NEGOCIATING_FRONT_STEP1;


    NlaTeeTransport & back_nla_tee_trans;
    RecorderFile & outFile;
    TimeBase & time_base;
    const char * host;

    TpduBuffer frontBuffer;
    TpduBuffer backBuffer;

    std::unique_ptr<NegoClient> nego_client;
    std::unique_ptr<NegoServer> nego_server;

    bool enable_kerberos;
    bool is_tls_client = false;
    bool is_nla_client = false;
    uint64_t verbosity;
};

