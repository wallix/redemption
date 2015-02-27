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

#ifndef _REDEMPTION_MOD_RDP_RDP_CHANNEL_CLIPBOARD_HPP_
#define _REDEMPTION_MOD_RDP_RDP_CHANNEL_CLIPBOARD_HPP_

#include "cast.hpp"

void mod_rdp::process_clipboard_event(
        const CHANNELS::ChannelDef & mod_channel, Stream & stream,
        uint32_t length, uint32_t flags, size_t chunk_size) {
    if (this->verbose & 1) {
        LOG(LOG_INFO, "mod_rdp server clipboard PDU");
    }

    const uint16_t msgType = stream.in_uint16_le();
    if (this->verbose & 1) {
        LOG(LOG_INFO, "mod_rdp server clipboard PDU: msgType=%d", msgType);
    }

    bool cencel_pdu = false;

    if (msgType == RDPECLIP::CB_FORMAT_LIST) {
        if (!this->authorization_channels.cliprdr_up_is_authorized() &&
            !this->authorization_channels.cliprdr_down_is_authorized()) {
            if (this->verbose & 1) {
                LOG(LOG_INFO, "mod_rdp clipboard is fully disabled (s)");
            }

            // Build and send the CB_FORMAT_LIST_RESPONSE (with status = OK)
            // 03 00 01 00 00 00 00 00
            BStream out_s(256);
            const bool response_ok = true;
            RDPECLIP::FormatListResponsePDU(response_ok).emit(out_s);

            this->send_to_channel(
                mod_channel, out_s, out_s.size(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
            );

            cencel_pdu = true;
        }
    }
    else if (msgType == RDPECLIP::CB_FORMAT_DATA_REQUEST) {
        if (!this->authorization_channels.cliprdr_up_is_authorized()) {
            if (this->verbose & 1) {
                LOG(LOG_INFO, "mod_rdp clipboard up is unavailable");
            }

            BStream out_s(256);
            RDPECLIP::FormatDataResponsePDU(false).emit(out_s, "\0");

            this->send_to_channel(
                mod_channel, out_s, out_s.size(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
            );

            cencel_pdu = true;
        }
    }
    else if (msgType == RDPECLIP::CB_FILECONTENTS_REQUEST) {
        if (!this->authorization_channels.cliprdr_file_is_authorized()) {
            if (this->verbose & 1) {
                LOG(LOG_INFO, "mod_rdp requesting the contents of client file is denied");
            }

            BStream out_s(256);
            const bool response_ok = false;
            RDPECLIP::FileContentsResponse(response_ok).emit(out_s);

            this->send_to_channel(
                mod_channel, out_s, out_s.size(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST
            );

            cencel_pdu = true;
        }
    }

    if (!cencel_pdu) {
        this->update_total_clipboard_data(msgType, length);
        stream.p -= 2;
        this->send_to_front_channel(
            mod_channel.name, stream.p, length, chunk_size, flags
        );
    }
}

#endif  // #ifndef _REDEMPTION_MOD_RDP_RDP_CHANNEL_CLIPBOARD_HPP_
