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

#include "proxy_recorder/nla_tee_transport.hpp"
#include "proxy_recorder/nego_client.hpp"
#include "proxy_recorder/nego_server.hpp"

#include "core/RDP/x224.hpp"
#include "core/RDP/nla/nla_server_ntlm.hpp"
#include "core/RDP/nla/nla_client_ntlm.hpp"
#include "core/RDP/nla/nla_client_kerberos.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/listen.hpp"
#include "core/server_notifier_api.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "transport/socket_trace_transport.hpp"
#include "utils/cli.hpp"
#include "utils/fixed_random.hpp"
#include "utils/netutils.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/utf.hpp"

#include <vector>
#include <chrono>
#include <iostream>

#include <cerrno>
#include <cstring>
#include <csignal>

#include <netinet/tcp.h>
#include <sys/select.h>
#include <openssl/ssl.h>

/** @brief a front connection with a RDP client */
class ProxyRecorder
{

    X224::CR_TPDU_Data front_CR_TPDU;

    using PacketType = RecorderFile::PacketType;
public:
    ProxyRecorder(NlaTeeTransport & back_nla_tee_trans, 
            RecorderFile & outFile,
            TimeObj & timeobj,
            const char * host,
            std::string nla_username, std::string nla_password,
            bool enable_kerberos,
            uint64_t verbosity
    )
        : back_nla_tee_trans(back_nla_tee_trans)
        , outFile(outFile)
        , timeobj(timeobj)
        , host(host)
        , nla_username(std::move(nla_username))
        , nla_password(std::move(nla_password))
        , enable_kerberos(enable_kerberos)
        , verbosity(verbosity)
    {
        this->frontBuffer.trace_pdu = (this->verbosity > 512);
        this->backBuffer.trace_pdu = (this->verbosity > 512);;

        if (!this->nla_password.empty()) {
            this->nla_password.push_back('\0');
        }
    }

    void front_step1(Transport & frontConn);
    void back_step1(array_view_u8 key, Transport & backConn);
    void front_nla(Transport & frontConn);
    void front_initial_pdu_negociation(Transport & backConn);
    void back_nla_negociation(Transport & backConn);
    void back_initial_pdu_negociation(Transport & frontConn);

public:
    enum {
        NEGOCIATING_FRONT_STEP1,
        NEGOCIATING_FRONT_NLA,
        NEGOCIATING_BACK_NLA,
        NEGOCIATING_FRONT_INITIAL_PDU,
        NEGOCIATING_BACK_INITIAL_PDU,
        FORWARD
    } pstate = NEGOCIATING_FRONT_STEP1;


    NlaTeeTransport & back_nla_tee_trans;
    RecorderFile & outFile;
    TimeObj & timeobj;
    const char * host;

    TpduBuffer frontBuffer;
    TpduBuffer backBuffer;

    std::unique_ptr<NegoClient> nego_client;
    std::unique_ptr<NegoServer> nego_server;

    std::string nla_username;
    std::string nla_password;
    bool enable_kerberos;
    bool is_tls_client = false;
    bool is_nla_client = false;
    uint64_t verbosity;
};

