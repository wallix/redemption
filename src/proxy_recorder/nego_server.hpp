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

class NegoServer
{
    FixedRandom rand;
    LCGTime timeobj;
    std::string extra_message;
    rdpCredsspServerNTLM credssp;

public:
    NegoServer(array_view_u8 key, std::string const& user, std::string const& password, uint64_t verbosity)
    : credssp(key, false, rand, timeobj, extra_message, Translation::EN,
        [&](cbytes_view user_av, cbytes_view domain_av, Array & password_array){
            LOG(LOG_INFO, "NTLM Check identity");

            auto [username, domain] = extract_user_domain(user.c_str());
            
            char utf8_user_buffer[1024] = {};
            UTF16toUTF8(user_av.data(), user_av.size(), 
                reinterpret_cast<uint8_t *>(utf8_user_buffer), sizeof(utf8_user_buffer));

            char utf8_domain_buffer[1024] = {};
            UTF16toUTF8(domain_av.data(), domain_av.size(), 
                reinterpret_cast<uint8_t *>(utf8_domain_buffer), sizeof(utf8_domain_buffer));

            bool check_identities = false;
            if (utf8_domain_buffer[0] == 0){
                auto [identity_username, identity_domain] = extract_user_domain(utf8_user_buffer);
                LOG(LOG_INFO, "NTML IDENTITY: identity.User=%s identity.Domain=%s username=%s, domain=%s",
                    identity_username, identity_domain, username, domain); 
                if ((username == identity_username) && (domain == identity_domain)) {
                    check_identities = true;
                }
            }
            else {
                if ((username == std::string(utf8_user_buffer)) && (domain == std::string(utf8_domain_buffer))) {
                    check_identities = true;
                }
            }

            LOG(LOG_INFO, "NTML IDENTITY: identity.User=%s identity.Domain=%s username=%s, domain=%s",
                utf8_user_buffer, utf8_domain_buffer, username, domain); 

            if (check_identities){
                size_t user_len = UTF8Len(byte_ptr_cast(password.c_str()));
                password_array.init(user_len * 2);
                UTF8toUTF16({password.c_str(), strlen(char_ptr_cast(byte_ptr_cast(password.c_str())))}, password_array.get_data(), user_len * 2);
                return PasswordCallback::Ok;
            }

            LOG(LOG_ERR, "Ntlm: bad identity");
            return PasswordCallback::Error;
        }, verbosity)
    {
    }

    credssp::State recv_data(TpduBuffer& buffer, OutStream & out_stream)
    {
        credssp::State st = credssp::State::Cont;
        while (buffer.next(TpduBuffer::CREDSSP) && credssp::State::Cont == st) {
            InStream in_stream(buffer.current_pdu_buffer());
            st = this->credssp.credssp_server_authenticate_next(in_stream, out_stream);
        }
        return st;
    }
};

