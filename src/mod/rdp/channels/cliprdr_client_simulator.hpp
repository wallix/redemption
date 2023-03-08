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
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan, Clément Moroldo
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#pragma once

#include "core/RDP/clipboard/format_list_serialize.hpp"

#include "mod/rdp/channels/virtual_channel_filter.hpp"

#include <cassert>

class CliprdrClientSimulator :
    public RemovableVirtualChannelFilter<CliprdrVirtualChannelProcessor>
{
public:
    CliprdrClientSimulator(bool verbose) : verbose(verbose) {}

    void process_client_message(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data, RDPECLIP::CliprdrHeader const* decoded_header) override
    {
        (void)total_length;
        (void)flags;
        (void)chunk_data;
        (void)decoded_header;

        // Not expected to receive messages from the previous filter!
    }

    void process_server_message(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data, RDPECLIP::CliprdrHeader const* decoded_header) override
    {
        (void)total_length;
        (void)decoded_header;

        InStream s(chunk_data);

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) &&
            (s.in_remain() >= 8 /* msgType(2) + msgFlags(2) + dataLen(4) */)) {

            const uint16_t msgType = s.in_uint16_le();
            if (RDPECLIP::CB_CLIP_CAPS == msgType) {
                s.in_skip_bytes(6 /* msgFlags(2) + dataLen(4) */);

                RDPECLIP::ClipboardCapabilitiesPDU pdu;
                pdu.recv(s);

                RDPECLIP::GeneralCapabilitySet caps;
                caps.recv(s);

                this->server_supports_long_format_name =
                    bool(caps.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

                LOG_IF(verbose, LOG_INFO,
                    "CliprdrClientSimulator::process_server_message: CB_CLIP_CAPS : ServerSupportsLongFormatName=%s",
                    this->server_supports_long_format_name ? "Yes" : "No");
            }
            else if (RDPECLIP::CB_MONITOR_READY == msgType) {
                LOG_IF(verbose, LOG_INFO,
                    "CliprdrClientSimulator::process_server_message: CB_MONITOR_READY : CB_CLIP_CAPS to server");

                assert(this->get_next_filter_ptr() != this);

                RDPECLIP::GeneralCapabilitySet general_cap_set(
                    RDPECLIP::CB_CAPS_VERSION_1, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
                RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1);
                RDPECLIP::CliprdrHeader caps_clipboard_header(
                    RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE_NONE,
                    clipboard_caps_pdu.size() + general_cap_set.size());

                StaticOutStream<128> caps_stream;

                caps_clipboard_header.emit(caps_stream);
                clipboard_caps_pdu.emit(caps_stream);
                general_cap_set.emit(caps_stream);

                this->get_next_filter_ptr()->process_client_message(
                        caps_stream.get_offset(),
                          CHANNELS::CHANNEL_FLAG_FIRST
                        | CHANNELS::CHANNEL_FLAG_LAST
                        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
                        caps_stream.get_produced_bytes(),
                        &caps_clipboard_header
                    );

                LOG_IF(verbose, LOG_INFO,
                    "CliprdrClientSimulator::process_server_message: CB_MONITOR_READY : CB_FORMAT_LIST to server");

                StaticOutStream<256> list_stream;
                Cliprdr::format_list_serialize_with_header(
                    list_stream, Cliprdr::IsLongFormat(this->server_supports_long_format_name),
                    std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_PRIVATELAST, {}}});

                this->get_next_filter_ptr()->process_client_message(
                        list_stream.get_offset(),
                          CHANNELS::CHANNEL_FLAG_FIRST
                        | CHANNELS::CHANNEL_FLAG_LAST
                        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
                        list_stream.get_produced_bytes(),
                        nullptr
                    );
            }
        }
    }

private:
    bool verbose = false;

    bool server_supports_long_format_name = false;
};
