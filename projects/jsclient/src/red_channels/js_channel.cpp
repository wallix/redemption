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

#include "red_channels/js_channel.hpp"

#include "red_emscripten/val.hpp"

#include "core/callback.hpp"
#include "utils/stream.hpp"

void redjs::channels::JsChannel::receive(bytes_view data, int flags)
{
    redjs::emval_call(this->js_handler, "receiveData", data.data(), data.size(), flags);
}

void redjs::channels::JsChannel::send_data(
    bytes_view data, uint32_t total_data_len, uint32_t channel_flags)
{
    InStream in_stream(data);
    this->cb.send_to_mod_channel(
        this->channel_name_id,
        in_stream,
        total_data_len,
        channel_flags
    );
}
