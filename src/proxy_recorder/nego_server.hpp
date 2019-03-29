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

#include "utils/difftimeval.hpp"
#include "utils/fixed_random.hpp"
#include "core/RDP/nego.hpp"
#include "proxy_recorder/nla_tee_transport.hpp"
#include "proxy_recorder/extract_user_domain.hpp"

class NegoServer
{
    FixedRandom rand;
    LCGTime timeobj;
    std::string extra_message;
    NlaTeeTransport trans;
    rdpCredsspServer credssp;

public:
    NegoServer(
        Transport& trans, RecorderFile& outFile,
        std::string const& user, std::string const& password,
        uint64_t verbosity)
    : trans(trans, outFile, NlaTeeTransport::Type::Server)
    , credssp(
        this->trans, false, false, rand, timeobj, extra_message, Translation::EN,
        [&](SEC_WINNT_AUTH_IDENTITY& identity){
            LOG(LOG_INFO, "NTLM Check identity");
            auto check = [vec = std::vector<uint8_t>{}](
                std::string const& str, Array& arr
            ) mutable {
                vec.resize(str.size() * 2);
                UTF8toUTF16(str, vec.data(), vec.size());
                return vec.size() == arr.size()
                    && 0 == memcmp(vec.data(), arr.get_data(), vec.size());
            };

            auto [username, domain] = extract_user_domain(user.c_str());
            std::string NTLMUser((char*)identity.User.get_data());
            std::string NTLMDomain((char*)identity.Domain.get_data());
            LOG(LOG_INFO, "NTML IDENTITY: identity.User=%s identity.Domain=%s username=%s, domain=%s",
                NTLMUser.c_str(), NTLMDomain.c_str(), username, domain); 
            if (check(username, identity.User)
             // domain is empty
             && check(domain, identity.Domain)
            ) {
                identity.SetPasswordFromUtf8(byte_ptr_cast(password.c_str()));
                return Ntlm_SecurityFunctionTable::PasswordCallback::Ok;
            }

            LOG(LOG_ERR, "Ntlm: bad identity");
            return Ntlm_SecurityFunctionTable::PasswordCallback::Error;
        }, verbosity)
    {
        this->credssp.credssp_server_authenticate_init();
    }

    rdpCredsspServer::State recv_data(TpduBuffer& buffer)
    {
        rdpCredsspServer::State st = rdpCredsspServer::State::Cont;
        while (buffer.next(TpduBuffer::CREDSSP) && rdpCredsspServer::State::Cont == st) {
            InStream in_stream(buffer.current_pdu_buffer());
            st = this->credssp.credssp_server_authenticate_next(in_stream);
        }
        return st;
    }
};

