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

#include "redjs/channel_receiver.hpp"
#include "red_channels/js_channel.hpp"

#include "red_emscripten/bind.hpp"


EMSCRIPTEN_BINDINGS(channel_js_channel)
{
    using JsChannel = redjs::channels::JsChannel;

    redjs::class_<JsChannel>("CustomChannel")
        .constructor<uintptr_t, emscripten::val, std::string>([](
            uintptr_t&& icb,
            emscripten::val&& callbacks,
            std::string&& channel_name
        ) {
            auto* pcb = reinterpret_cast<Callback*>(icb);
            char id_name[8]{};
            channel_name.copy(id_name, std::size(id_name));
            return new JsChannel{*pcb, std::move(callbacks), CHANNELS::ChannelNameId{id_name}};
        })
        .function_ptr("getChannelReceiver", [](JsChannel& jschannel) {
            auto receiver = [&jschannel](cbytes_view data, int channel_flags){
                jschannel.receive(data, channel_flags);
            };
            return redjs::ChannelReceiver(jschannel.channel_name_id, receiver);
        })
        .function_ptr("sendData", [](JsChannel& jschannel,
            std::string data, uint32_t total_data_len, uint32_t channel_flags)
        {
            jschannel.send_data(data, total_data_len, channel_flags);
        })
        .function_ptr("sendRawData", [](JsChannel& jschannel,
            std::ptrdiff_t idata, std::size_t idata_len,
            uint32_t total_data_len, uint32_t channel_flags)
        {
            auto* ptr = reinterpret_cast<uint8_t const*>(idata);
            jschannel.send_data({ptr, idata_len}, total_data_len, channel_flags);
        })
    ;
}
