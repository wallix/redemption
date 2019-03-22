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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "redjs/channel.hpp"
#include "utils/stream.hpp"
#include "core/callback.hpp"

redjs::Channel::Channel(CHANNELS::ChannelNameId name_id, receiver_type receiver)
: name_id(name_id)
, do_receive(std::move(receiver))
{}

void redjs::Channel::send(cbytes_view data, int channel_flags)
{
    InStream chunk(data);
    this->callback().send_to_mod_channel(this->name_id, chunk, data.size(), channel_flags);
}

void redjs::Channel::receive(cbytes_view data, int channel_flags)
{
    this->do_receive(*this, data, channel_flags);
}

Callback& redjs::Channel::callback() noexcept
{
    assert(this->cb);
    return *this->cb;
}

void redjs::Channel::set_cb(not_null_ptr<Callback> cb) noexcept
{
    this->cb = cb;
}


#include "red_emscripten/bind.hpp"

EMSCRIPTEN_BINDINGS(channel)
{
    redjs::class_<redjs::Channel>("Channel")
        .function_ptr("send", [](redjs::Channel& channel, intptr_t iptr, unsigned len, int channel_flags){
            channel.send({reinterpret_cast<uint8_t*>(iptr), len}, channel_flags);
        })
    ;
}
