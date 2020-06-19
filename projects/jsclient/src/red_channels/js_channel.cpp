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
#include "red_emscripten/bind.hpp"


#include "core/callback.hpp"
#include "utils/stream.hpp"


redjs::channels::JsChannel::JsChannel(
    Callback& cb, emscripten::val&& js_handler,
    CHANNELS::ChannelNameId channel_name)
: cb(cb)
, js_handler(std::move(js_handler))
, channel_receiver(make_channel_receiver<&JsChannel::receive>(channel_name, this))
{}

redjs::channels::JsChannel::~JsChannel()
{
    emval_call(this->js_handler, "free");
}

void redjs::channels::JsChannel::receive(
    bytes_view data, uint32_t total_data_len, uint32_t channel_flags)
{
    redjs::emval_call(
        this->js_handler, "receiveData",
        data.data(), checked_cast<uint32_t>(data.size()),
        total_data_len, channel_flags);
}

void redjs::channels::JsChannel::send(
    bytes_view data, uint32_t total_data_len, uint32_t channel_flags)
{
    InStream in_stream(data);
    this->cb.send_to_mod_channel(
        this->channel_receiver.channel_name,
        in_stream,
        total_data_len,
        channel_flags
    );
}


EMSCRIPTEN_BINDINGS(channel_js_channel)
{
    using JsChannel = redjs::channels::JsChannel;

    redjs::class_<JsChannel>("CustomChannel")
        .constructor([](
            uintptr_t icb,
            emscripten::val&& callbacks,
            std::string const& channel_name
        ) {
            assert(channel_name.size() <= 7);
            char id_name[8]{};
            channel_name.copy(id_name, std::size(id_name));

            auto* pcb = reinterpret_cast<Callback*>(icb);

            return new JsChannel{*pcb, std::move(callbacks), CHANNELS::ChannelNameId{id_name}};
        })
        .function_ptr("getChannelReceiver", [](JsChannel& jschannel) {
            return reinterpret_cast<uintptr_t>(&jschannel.channel_receiver);
        })
        .function_ptr("sendData", [](JsChannel& jschannel,
            std::string data, uint32_t total_data_len, uint32_t channel_flags)
        {
            jschannel.send(data, total_data_len, channel_flags);
        })
        .function_ptr("sendRawData", [](JsChannel& jschannel,
            uintptr_t idata, uint32_t idata_len,
            uint32_t total_data_len, uint32_t channel_flags)
        {
            auto* ptr = reinterpret_cast<uint8_t const*>(idata);
            jschannel.send({ptr, idata_len}, total_data_len, channel_flags);
        })
    ;
}
