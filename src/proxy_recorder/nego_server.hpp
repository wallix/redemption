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
#include "utils/utf.hpp"
#include "core/RDP/nego.hpp"
#include "proxy_recorder/extract_user_domain.hpp"
#include "core/RDP/nla/nla_server_ntlm.hpp"


class NegoServer
{
    FixedRandom rand;
    LCGTime timeobj;
    std::string user;
    std::string password;
public:
    NtlmServer credssp;

    std::pair<PasswordCallback,array_md4> get_password_hash(bytes_view user_av, bytes_view domain_av)
    {
        LOG(LOG_INFO, "NTLM Check identity");
        hexdump_d(user_av);
        
        auto [username, domain] = extract_user_domain(this->user);
        // from protocol
        auto tmp_utf8_user = ::encode_UTF16_to_UTF8(user_av);
        std::string u8user(reinterpret_cast<char*>(tmp_utf8_user.data()),
                             reinterpret_cast<char*>(tmp_utf8_user.data())+tmp_utf8_user.size());
        auto tmp_utf8_domain = ::encode_UTF16_to_UTF8(domain_av);
        std::string u8domain(reinterpret_cast<char*>(tmp_utf8_domain.data()),
                             reinterpret_cast<char*>(tmp_utf8_domain.data())+tmp_utf8_domain.size());

        LOG(LOG_INFO, "NTML IDENTITY(message): identity.User=%s identity.Domain=%s username=%s, domain=%s",
            u8user, u8domain, username, domain);

        if (u8domain.size() == 0){
            auto [identity_username, identity_domain] = extract_user_domain(u8user);

            bool user_match = username == identity_username;
            bool domain_match = domain == identity_domain;

            if (user_match && domain_match){
                LOG(LOG_INFO, "known identity");
                return {PasswordCallback::Ok, Md4(::UTF8toUTF16(this->password))};
            }
        }
        else if ((username == u8user) && (domain == u8domain)){
            return {PasswordCallback::Ok, Md4(::UTF8toUTF16(this->password))};
        }

        LOG(LOG_ERR, "Ntlm: unknwon identity");
        return {PasswordCallback::Error, {}};
    }

public:
    NegoServer(array_view_u8 key, std::string const& user, std::string const& password, uint64_t verbosity)
    : user(user)
    , password(password)
    , credssp(false, true, "WIN7"_av, "WIN7"_av,"WIN7"_av,"win7"_av,"win7"_av, "win7"_av, key, 
            {MsvAvNbDomainName,MsvAvNbComputerName,MsvAvDnsDomainName,MsvAvDnsComputerName,MsvAvTimestamp}, rand, timeobj,
        [this](bytes_view user_av, bytes_view domain_av){
            return this->get_password_hash(user_av, domain_av);
        }, 6, 
        NtlmVersion{WINDOWS_MAJOR_VERSION_6, WINDOWS_MINOR_VERSION_1, 7601, NTLMSSP_REVISION_W2K3},
        false, verbosity)
    {
    }

    std::pair<credssp::State,std::vector<uint8_t>> recv_data2(TpduBuffer& buffer)
    {
        LOG(LOG_INFO, "NegoServer recv_data");
        std::vector<uint8_t> result;
        credssp::State st = credssp::State::Cont;
        while (credssp::State::Cont == st) {
            LOG(LOG_INFO, "NegoServer recv_data authenticate_next");
            result << this->credssp.authenticate_next(buffer.current_pdu_buffer());
            st = this->credssp.state;
        }
        return {st, result};
    }

    std::pair<credssp::State,std::vector<uint8_t>> recv_data(TpduBuffer& buffer)
    {
        LOG(LOG_INFO, "NegoServer recv_data");
        std::vector<uint8_t> result;
        credssp::State st = credssp::State::Cont;
        while (buffer.next(TpduBuffer::CREDSSP) && credssp::State::Cont == st) {
            LOG(LOG_INFO, "NegoServer recv_data authenticate_next");
            result << this->credssp.authenticate_next(buffer.current_pdu_buffer());
            st = this->credssp.state;
        }
        return {st, result};
    }
};

