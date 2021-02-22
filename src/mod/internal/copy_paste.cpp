/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#include "mod/internal/copy_paste.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "utils/stream.hpp"
#include "mod/internal/widget/widget.hpp"
#include "core/channel_names.hpp"
#include "core/stream_throw_helpers.hpp"

#include <algorithm>

namespace
{
    constexpr auto cliprdr_channel_flags
        = CHANNELS::CHANNEL_FLAG_FIRST
        | CHANNELS::CHANNEL_FLAG_LAST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
        ;

   template<class... PDU>
   void send_to_front_channel(
       FrontAPI & front, const CHANNELS::ChannelDef channel,
       uint16_t msgType, uint16_t msgFlags, PDU&&... pdu)
   {
       StaticOutStream<2048> out_s;

       RDPECLIP::CliprdrHeader clip_header(msgType, msgFlags, (0 + ... + pdu.size()));
       clip_header.emit(out_s);
       (..., pdu.emit(out_s));

       front.send_to_channel(
           channel, out_s.get_produced_bytes(), out_s.get_offset(),
           cliprdr_channel_flags
       );
   }
} // namespace

void CopyPaste::LimitString::utf16_push_back(const uint8_t * s, size_t n)
{
    this->size_ += UTF16toUTF8(
        s, n,
        byte_ptr_cast(this->buf_ + this->size_),
        this->max_size() - this->size_
    );
    this->buf_[this->size_] = 0;
}

void CopyPaste::LimitString::assign(char const * s, size_t n)
{
    this->size_ = ((this->max_size() >= n)
        ? n
        : ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(s), this->max_size()));
    memcpy(this->buf_, s, this->size_);
    this->buf_[this->size_] = 0;
}

zstring_view CopyPaste::LimitString::zstring() const
{
    return zstring_view{zstring_view::is_zero_terminated(), this->buf_, this->size_};
}


bool CopyPaste::ready(FrontAPI & front)
{
    LOG_IF(this->verbose, LOG_INFO, "CopyPaste::ready");

    this->front_ = &front;
    this->channel_ = front.get_channel_list().get_by_name(channel_names::cliprdr);

    if (this->channel_) {
        send_to_front_channel(
            *this->front_, *this->channel_,
            RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE__NONE_,
            RDPECLIP::ClipboardCapabilitiesPDU(1),
            RDPECLIP::GeneralCapabilitySet(
                RDPECLIP::CB_CAPS_VERSION_2,
                RDPECLIP::CB_USE_LONG_FORMAT_NAMES
            )
        );

        send_to_front_channel(
            *this->front_, *this->channel_,
            RDPECLIP::CB_MONITOR_READY, RDPECLIP::CB_RESPONSE__NONE_,
            RDPECLIP::ServerMonitorReadyPDU()
        );

        return true;
    }

    return false;
}

void CopyPaste::paste(Widget & widget)
{
    if (REDEMPTION_UNLIKELY(!*this)) {
        return;
    }

    if (this->has_clipboard_) {
        this->pasted_widget_ = nullptr;
        widget.clipboard_insert_utf8(this->clipboard_str_.zstring());
    }
    else if (!this->pasted_widget_) {
        this->pasted_widget_ = &widget;

        send_to_front_channel(
            *this->front_, *this->channel_,
            RDPECLIP::CB_FORMAT_DATA_REQUEST, RDPECLIP::CB_RESPONSE__NONE_,
            RDPECLIP::FormatDataRequestPDU(RDPECLIP::CF_UNICODETEXT)
        );
    }
}

void CopyPaste::copy(chars_view str)
{
    if (REDEMPTION_UNLIKELY(!*this)) {
        return;
    }

    LOG_IF(this->verbose, LOG_INFO, "CopyPaste::copy '%.*s'", int(str.size()), str.data());

    this->has_clipboard_ = true;
    this->clipboard_str_.assign(str.data(), str.size());

    StaticOutStream<256> out_s;
    Cliprdr::format_list_serialize_with_header(
        out_s,
        Cliprdr::IsLongFormat(this->client_use_long_format_names),
        std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_UNICODETEXT, {}}});

    this->front_->send_to_channel(
        *this->channel_, out_s.get_produced_bytes(),
        out_s.get_offset(), cliprdr_channel_flags);
}

void CopyPaste::send_to_mod_channel(InStream & chunk, uint32_t flags)
{
    if (REDEMPTION_UNLIKELY(!*this)) {
        return;
    }

    InStream stream({chunk.get_data(), chunk.get_capacity()});

    if (this->long_data_response_size) {
        auto available_data_length = std::min<size_t>(
            this->long_data_response_size, stream.in_remain());

        if (available_data_length) {
            this->long_data_response_size -= available_data_length;
            this->clipboard_str_.utf16_push_back(stream.get_current(), available_data_length / 2);
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            if (this->pasted_widget_) {
                this->pasted_widget_->clipboard_insert_utf8(this->clipboard_str_.zstring());
                this->pasted_widget_ = nullptr;
            }

            this->has_clipboard_ = true;

            this->long_data_response_size = 0;
        }

        return ;
    }

    RDPECLIP::RecvPredictor rp(stream);

    LOG_IF(this->verbose, LOG_INFO, "CopyPaste::send_to_mod_channel msgType=%s",
           RDPECLIP::get_msgType_name(rp.msgType()));

    switch (rp.msgType()) {
        case RDPECLIP::CB_FORMAT_LIST: {
            send_to_front_channel(
                *this->front_, *this->channel_,
                RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK,
                RDPECLIP::FormatListResponsePDU()
            );

            this->has_clipboard_ = false;
            this->clipboard_str_.clear();

            break;
        }

        //case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
        //    break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            // RDPECLIP::FormatDataRequestPDU format_data_request;
            // format_data_request.recv(stream);

            constexpr auto header_size = RDPECLIP::CliprdrHeader::size();
            StaticOutStream<header_size + 2048> out;
            auto header_data = out.out_skip_bytes(header_size);

            auto data_len = UTF8toUTF16(this->clipboard_str_.zstring(), out.get_tail());
            out.out_skip_bytes(data_len);

            OutStream out_header(header_data);
            RDPECLIP::CliprdrHeader(
                RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL,
                data_len
            ).emit(out_header);

            this->front_->send_to_channel(
                *this->channel_, out.get_produced_bytes(), out.get_offset(),
                cliprdr_channel_flags
            );

            break;
        }

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
            RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 0);
            header.recv(stream);
            if (header.msgFlags() == RDPECLIP::CB_RESPONSE_OK) {
                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    ::check_throw(stream, header.dataLen(), "CopyPaste::send_to_mod_channel truncated CB_FORMAT_DATA_RESPONSE", ERR_RDP_PROTOCOL);

                    this->clipboard_str_.utf16_push_back(stream.get_current(), header.dataLen() / 2);

                    if (this->pasted_widget_) {
                        this->pasted_widget_->clipboard_insert_utf8(this->clipboard_str_.zstring());
                        this->pasted_widget_ = nullptr;
                    }

                    this->has_clipboard_ = true;
                }
                // Virtual channel data span in multiple Virtual Channel PDUs.
                else if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->long_data_response_size = header.dataLen() - stream.in_remain();
                    this->clipboard_str_.utf16_push_back(stream.get_current(), stream.in_remain() / 2);
                }
                else {
                    LOG(LOG_ERR, "CopyPaste::send_to_mod_channel flag CHANNEL_FLAG_FIRST expected");
                    throw Error(ERR_RDP_PROTOCOL);
                }
            }
            break;
        }

        case RDPECLIP::CB_CLIP_CAPS: {
            RDPECLIP::CliprdrHeader clipboard_header;
            clipboard_header.recv(stream);

            ::check_throw(stream, 4, "CopyPaste::send_to_mod_channel:CLIPRDR_CAPS", ERR_RDP_PROTOCOL);

            LOG_IF(this->verbose, LOG_INFO,
                "CopyPaste::send_to_mod_channel: Clipboard Capabilities PDU");

            const uint16_t cCapabilitiesSets = stream.in_uint16_le();
            LOG_IF(this->verbose, LOG_INFO, "cCapabilitiesSets=%u", cCapabilitiesSets);
            assert(1 == cCapabilitiesSets);

            stream.in_skip_bytes(2); // pad1(2)

            for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
                RDPECLIP::CapabilitySetRecvFactory f(stream);

                if (f.capabilitySetType() == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                    RDPECLIP::GeneralCapabilitySet general_caps;

                    general_caps.recv(stream, f);

                    if (this->verbose) {
                        LOG(LOG_INFO, "CopyPaste::send_to_mod_channel: General Capability Set");
                        general_caps.log(LOG_INFO);
                    }

                    this->client_use_long_format_names =
                        bool(general_caps.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
                }
            }
        }
        break;
    }
}


void copy_paste_process_event(
    CopyPaste & copy_paste, Widget & widget,
    NotifyApi::notify_event_t event)
{
    switch(event) {
        case NOTIFY_PASTE: widget.clipboard_paste(copy_paste); break;
        case NOTIFY_COPY: widget.clipboard_copy(copy_paste); break;
        case NOTIFY_CUT: widget.clipboard_cut(copy_paste); break;
    }
}
