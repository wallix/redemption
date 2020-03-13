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

#pragma once


#include "utils/sugar/bytes_view.hpp"

#include <emscripten/val.h>

#include <array>


class Callback;
class InStream;

namespace redjs::channels::clipboard
{

enum class CustomFormat : uint32_t
{
    None,
    FileGroupDescriptorW = 33333,
};

enum class Charset : bool
{
    Ascii,
    Utf16,
};

struct ClipboardChannel
{
    ClipboardChannel(Callback& cb, emscripten::val&& callbacks, bool verbose);
    ~ClipboardChannel();

    void receive(bytes_view data, uint32_t flags);

    void send_file_contents_request(
        uint32_t request_type,
        uint32_t stream_id, uint32_t lindex,
        uint32_t pos_low, uint32_t pos_high,
        uint32_t max_bytes_to_read,
        bool has_lock_id, uint32_t lock_id);

    void send_request_format(uint32_t format_id, CustomFormat custom_cf);
    void send_format(uint32_t format_id, Charset charset, bytes_view name);
    unsigned add_format(writable_bytes_view data, uint32_t format_id, Charset charset, bytes_view name);
    void send_header(uint16_t type, uint16_t flags, uint32_t total_data_len, uint32_t channel_flags);
    void send_data(bytes_view data, uint32_t total_data_len, uint32_t channel_flags);
    void send_data(bytes_view av);

private:
    void process_filecontents_request(InStream& chunk);
    void process_format_data_request(InStream& chunk);
    void process_capabilities(InStream& chunk);
    void process_monitor_ready();
    void process_format_data_response(bytes_view data, uint32_t channel_flags, uint32_t data_len);
    void process_filecontents_response(bytes_view data, uint32_t channel_flags, uint32_t data_len);
    void process_format_list(InStream& chunk, uint32_t channel_flags);

    struct ResponseBuffer
    {
        std::array<uint8_t, 592 /*RDPECLIP::FileDescriptor::size()*/> data;
        std::size_t size = 0;

        void push(bytes_view av);

        bytes_view as_bytes() const;

        void clear()
        {
            this->size = 0;
        }
    };

    Callback& cb;
    emscripten::val callbacks;
    uint32_t general_flags = 0;
    uint32_t remaining_data_len;
    uint32_t stream_id;
    CustomFormat custom_cf = CustomFormat();

    enum ResponseState : uint8_t
    {
        None,
        Data,
        FileContents,
    };

    ResponseState response_state = ResponseState::None;
    bool verbose;
    ResponseBuffer response_buffer;
};

} // namespace redjs::channel::clipboard

namespace redjs
{
    using ClipboardChannel = channels::clipboard::ClipboardChannel;
}
