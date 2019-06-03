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


#include "utils/stream.hpp"
#include "mod/rdp/rdp_verbose.hpp"

#include <emscripten/val.h>

#include <memory>


class Callback;

namespace redjs::channels::clipboard
{

enum class ClipboardFormat : uint32_t
{
    Text = 1,
    UnicodeText = 13,
};

struct FormatListEmptyName
{
    std::size_t size() const noexcept;

    void emit(ClipboardFormat cf, OutStream& out_stream) noexcept;

    bool use_long_format_names = true;
};

enum class CustomFormat : uint32_t
{
    None = 0,

    FileGroupDescriptorW,
    FileContentsSize,
    FileContentsRange,
};

enum class Charset : bool
{
    Ascii,
    Utf16,
};

struct ClipboardChannel
{
    ClipboardChannel(Callback& cb, emscripten::val&& callbacks, RDPVerbose verbose);
    ~ClipboardChannel();

    void receive(cbytes_view data, int flags);

    void send_file_contents_request(
        uint32_t request_type,
        uint32_t stream_id, uint32_t lindex,
        uint32_t pos_low, uint32_t pos_high);

    void send_request_format(uint32_t format_id, CustomFormat custom_cf);
    void send_format(uint32_t format_id, Charset charset, cbytes_view name);
    unsigned add_format(bytes_view data, uint32_t format_id, Charset charset, cbytes_view name);
    void send_header(uint16_t type, uint16_t flags, uint32_t total_data_len, uint32_t channel_flags);
    void send_data(cbytes_view data, uint32_t total_data_len, uint32_t channel_flags);
    void send_data(cbytes_view av);

private:
    void process_filecontents_request(InStream& chunk);
    void process_format_data_request(InStream& chunk);
    void process_capabilities(InStream& chunk);
    void process_monitor_ready();
    void process_format_data_response(cbytes_view data, uint32_t channel_flags, uint32_t data_len);
    void process_format_list(InStream& chunk, uint32_t channel_flags);
    void send_format_list_response_ok();

    struct ResponseBuffer
    {
        std::array<uint8_t, 592> data;
        std::size_t size = 0;

        void set(cbytes_view av);

        void add(cbytes_view av);

        cbytes_view as_bytes() const;

        void clear()
        {
            this->size = 0;
        }
    };

    Callback& cb;
    emscripten::val callbacks;
    FormatListEmptyName format_list;
    uint32_t requested_format_id = 0;
    uint32_t data_len = 0;
    CustomFormat custom_cf {};
    bool wating_for_data = false;
    bool verbose;
    ResponseBuffer response_buffer;
};

} // namespace redjs::channel::clipboard

namespace redjs
{
    using ClipboardChannel = channels::clipboard::ClipboardChannel;
}
