/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_MOD_RDP_RDP_CHANNEL_AUTH_HPP_
#define _REDEMPTION_MOD_RDP_RDP_CHANNEL_AUTH_HPP_

#include "cast.hpp"

void mod_rdp::process_auth_event(const CHANNELS::ChannelDef & mod_channel,
        Stream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
    std::string auth_channel_message(char_ptr_cast(stream.p), stream.in_remain());

    LOG(LOG_INFO, "Auth channel data=\"%s\"", auth_channel_message.c_str());

    this->auth_channel_flags  = flags;
    this->auth_channel_chanid = mod_channel.chanid;

    if (this->acl) {
        this->acl->set_auth_channel_target(auth_channel_message.c_str());
    }
}

#endif  // #ifndef _REDEMPTION_MOD_RDP_RDP_CHANNEL_AUTH_HPP_
