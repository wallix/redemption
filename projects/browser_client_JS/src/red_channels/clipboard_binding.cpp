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

#include "red_channels/clipboard.hpp"
#include "redjs/channel_receiver.hpp"

#include "red_emscripten/bind.hpp"

#include "core/channel_names.hpp"


EMSCRIPTEN_BINDINGS(channel_clipboard)
{
    namespace clipboard = redjs::channels::clipboard;

    redjs::class_<clipboard::ClipboardChannel>("ClipboardChannel")
        .constructor<uintptr_t, emscripten::val, unsigned long>([](
            uintptr_t&& icb, emscripten::val&& callbacks, unsigned long&& verbose
        ) {
            auto* pcb = reinterpret_cast<Callback*>(icb);
            return new clipboard::ClipboardChannel(*pcb, std::move(callbacks), RDPVerbose(verbose));
        })
        .function_ptr("getChannelReceiver", [](clipboard::ClipboardChannel& clip) {
            auto receiver = [&clip](cbytes_view data, int channel_flags){
                clip.receive(data, channel_flags);
            };
            return redjs::ChannelReceiver(channel_names::cliprdr, receiver);
        })
        .function_ptr("sendRequestFormat", [](clipboard::ClipboardChannel& clip, uint32_t id, int custom_cf) {
            clip.send_request_format(id, clipboard::CustomFormat(custom_cf));
        })
        .function_ptr("sendMessage", [](clipboard::ClipboardChannel& clip, emscripten::val val) {
            // TODO abort copy
            auto data = val.as<std::string>();
            clip.send_to_mod_channel(data);
        })
        .function_ptr("sendFileContentsRequest", [](clipboard::ClipboardChannel& clip,
            uint32_t request_type, uint32_t stream_id, uint32_t lindex, uint32_t pos_low, uint32_t pos_high)
        {
            clip.send_file_contents_request(request_type, stream_id, lindex, pos_low, pos_high);
        })
        .function_ptr("sendData", [](clipboard::ClipboardChannel& clip,
            uint16_t msg_flags, std::string data,
            uint32_t total_data_len, uint32_t channel_flags,
            bool encode_utf8_to_utf16)
        {
            clip.send_data(msg_flags, data, total_data_len, channel_flags, encode_utf8_to_utf16);
        })
        .function_ptr("sendFormat", [](clipboard::ClipboardChannel& clip,
            uint32_t format_id, int charset, std::string name, bool is_last)
        {
            clip.send_format(format_id, clipboard::Charset(charset), name, is_last);
        })
        // .function("receive", &clipboard::ClipboardChannel::receive)
    ;
}
