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
    : credssp(key, rand, timeobj, extra_message, Translation::EN,
        [&](cbytes_view user_av, cbytes_view domain_av, std::vector<uint8_t> & password_array){
            LOG(LOG_INFO, "NTLM Check identity");
            
            // fROM COMMAND LINE
            auto [username, domain] = extract_user_domain(user.c_str());
            // from protocol
            auto utf8_user = ::encode_UTF16_to_UTF8(user_av);
            auto utf8_domain = ::encode_UTF16_to_UTF8(domain_av);

            LOG(LOG_INFO, "NTML IDENTITY: identity.User=%*s identity.Domain=%*s username=%s, domain=%s",
                int(utf8_user.size()),reinterpret_cast<char*>(utf8_user.data()), 
                int(utf8_domain.size()),reinterpret_cast<char*>(utf8_domain.data()), username, domain);

            if (utf8_domain.size() == 0){
                auto [identity_username, identity_domain] = extract_user_domain(utf8_user);
                if (are_buffer_equal({const_cast<const uint8_t*>(reinterpret_cast<uint8_t*>(username.data())),
                    username.size()}, identity_username)
                && are_buffer_equal({const_cast<const uint8_t*>(reinterpret_cast<uint8_t*>(domain.data())), 
                    domain.size()}, identity_domain)) {
                    password_array = UTF8toUTF16(password);
                    return PasswordCallback::Ok;
                }
            }

            if ((utf8_domain.size() != 0) 
            && are_buffer_equal({const_cast<const uint8_t*>(reinterpret_cast<uint8_t*>(username.data())),
                    username.size()}, utf8_user)
            && are_buffer_equal({const_cast<const uint8_t*>(reinterpret_cast<uint8_t*>(domain.data())), 
                    domain.size()}, utf8_domain)) {
                password_array = UTF8toUTF16(password);
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

