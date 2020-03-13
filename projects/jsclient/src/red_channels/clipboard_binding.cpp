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
        .constructor<uintptr_t, emscripten::val, bool>([](
            uintptr_t&& icb, emscripten::val&& callbacks, bool&& verbose
        ) {
            auto* pcb = reinterpret_cast<Callback*>(icb);
            return new clipboard::ClipboardChannel(*pcb, std::move(callbacks), verbose);
        })
        .function_ptr("getChannelReceiver", [](clipboard::ClipboardChannel& clip) {
            auto receiver = [&clip](bytes_view data, int channel_flags){
                clip.receive(data, channel_flags);
            };
            return redjs::ChannelReceiver(channel_names::cliprdr, receiver);
        })
        .function_ptr("sendRequestFormat", [](clipboard::ClipboardChannel& clip, uint32_t id, int custom_cf) {
            clip.send_request_format(id, clipboard::CustomFormat(custom_cf));
        })
        .function_ptr("sendFileContentsRequest", [](clipboard::ClipboardChannel& clip,
            uint32_t request_type, uint32_t stream_id, uint32_t lindex,
            uint32_t pos_low, uint32_t pos_high, uint32_t max_bytes_to_read,
            bool has_lock_id, uint32_t lock_id)
        {
            clip.send_file_contents_request(request_type,
                stream_id, lindex, pos_low, pos_high, max_bytes_to_read, has_lock_id, lock_id);
        })
        .function_ptr("sendHeader", [](clipboard::ClipboardChannel& clip,
            uint16_t type, uint16_t flags, uint32_t total_data_len, uint32_t channel_flags)
        {
            clip.send_header(type, flags, total_data_len, channel_flags);
        })
        .function_ptr("sendData", [](clipboard::ClipboardChannel& clip,
            std::string data, uint32_t total_data_len, uint32_t channel_flags)
        {
            clip.send_data(data, total_data_len, channel_flags);
        })
        .function_ptr("sendRawData", [](clipboard::ClipboardChannel& clip,
            std::ptrdiff_t idata, std::size_t idata_len,
            uint32_t total_data_len, uint32_t channel_flags)
        {
            auto* ptr = reinterpret_cast<uint8_t const*>(idata);
            clip.send_data({ptr, idata_len}, total_data_len, channel_flags);
        })
        .function_ptr("sendDataWithHeader", [](clipboard::ClipboardChannel& clip,
            uint16_t type, std::ptrdiff_t idata, std::size_t idata_len)
        {
            auto* ptr = reinterpret_cast<uint8_t const*>(idata);
            clip.send_header(type, 1/*Ok*/, idata_len, 0);
            clip.send_data({ptr, idata_len}, 0, 2/*last*/);
        })
        .function_ptr("addFormat", [](clipboard::ClipboardChannel& clip,
            std::ptrdiff_t idata, std::size_t idata_len, uint32_t format_id, int charset, std::string name)
        {
            auto* ptr = reinterpret_cast<uint8_t*>(idata);
            return clip.add_format({ptr, idata_len}, format_id, clipboard::Charset(charset), name);
        })
        .function_ptr("sendFormat", [](clipboard::ClipboardChannel& clip,
            uint32_t format_id, int charset, std::string name)
        {
            clip.send_format(format_id, clipboard::Charset(charset), name);
        })
        // .function("receive", &clipboard::ClipboardChannel::receive)
    ;
}
