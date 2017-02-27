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


#pragma once

#include "core/RDP/clipboard.hpp"
#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "utils/stream.hpp"
#include "widget2/edit.hpp"
#include "core/channel_names.hpp"

#include <utility>
#include <algorithm>

class CopyPaste
{
    FrontAPI * front_ = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;
    WidgetEdit * paste_edit_ = nullptr;

    class LimitString
    {
        char buf_[1024/* * 4*/];
        size_t size_ = 0;

    public:
        LimitString() = default;

        static constexpr size_t max_size() {
            return sizeof(buf_) / sizeof(buf_[0]) - 1;
        }

        void utf16_push_back(const uint8_t * s, size_t n) {
            this->size_ += UTF16toUTF8(
                s, n,
                reinterpret_cast<uint8_t*>(this->buf_ + this->size_),
                this->max_size() - this->size_
            );
            this->buf_[this->size_] = 0;
        }

        void assign(char const * s, size_t n) {
            this->size_ = ((this->max_size() >= n) ? n :
                           ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(s), this->max_size()));
            memcpy(this->buf_, s, this->size_);
            this->buf_[this->size_] = 0;
        }

        const char * c_str() const {
            return this->buf_;
        }

        size_t size() const {
            return this->size_;
        }

        void clear() {
            this->size_ = 0;
        }
    };

    LimitString clipboard_str_;
    bool has_clipboard_ = false;
    size_t long_data_response_size = 0;

public:
    CopyPaste() = default;
    CopyPaste(const CopyPaste &) = delete;
    CopyPaste & operator=(const CopyPaste &) = delete;

    bool ready(FrontAPI & front) {
        this->front_ = &front;
        this->channel_ = front.get_channel_list().get_by_name(channel_names::cliprdr);

        if (this->channel_) {
            StaticOutStream<256> out_s;
            RDPECLIP::ClipboardCapabilitiesPDU general_pdu(1, RDPECLIP::GeneralCapabilitySet::size());
            general_pdu.emit(out_s);
            RDPECLIP::GeneralCapabilitySet general_caps(RDPECLIP::CB_CAPS_VERSION_2, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            general_caps.emit(out_s);

            const size_t length     = out_s.get_offset();
            const size_t chunk_size = length;
            this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                          CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);

            this->send_to_front_channel(RDPECLIP::ServerMonitorReadyPDU());
            return true;
        }

        return false;
    }

    explicit operator bool () const noexcept {
        return this->channel_;
    }

    void paste(WidgetEdit & edit) {
        if (this->has_clipboard_) {
            this->paste_edit_ = nullptr;
            edit.insert_text(this->clipboard_str_.c_str());
        }
        else {
            this->paste_edit_ = &edit;
            this->send_to_front_channel(RDPECLIP::FormatDataRequestPDU(RDPECLIP::CF_UNICODETEXT));
        }
    }

    void copy(const char * s, size_t n) {
        this->has_clipboard_ = true;
        this->clipboard_str_.assign(s, n);
        this->send_to_front_channel(RDPECLIP::FormatListPDU());
    }

    void copy(const char * s) {
        this->copy(s, strlen(s));
    }

    //bool has_text() const noexcept {
    //    return this->has_clipboard_;
    //}

    void send_to_mod_channel(InStream & chunk, uint32_t flags)
    {
        InStream stream(chunk.get_data(), chunk.get_capacity());

        if (this->long_data_response_size) {
            size_t available_data_length =
                std::min<size_t>(this->long_data_response_size, stream.in_remain());

//            if (this->long_data_response_size < stream.in_remain()) {
//                LOG( LOG_ERR
//                   , "selector::send_to_selector truncated CB_FORMAT_DATA_RESPONSE dataU16, need=%u remains=%u"
//                   , this->long_data_response_size, stream.in_remain());
//                throw Error(ERR_RDP_PROTOCOL);
//            }

            if (available_data_length) {
//            this->long_data_response_size -= stream.in_remain();
                this->long_data_response_size -= available_data_length;
//            this->clipboard_str_.utf16_push_back(stream.p, stream.in_remain() / 2);
                this->clipboard_str_.utf16_push_back(stream.get_current(), available_data_length / 2);
            }

//            if (!this->long_data_response_size && this->paste_edit_) {
            if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                if (this->paste_edit_) {
                    this->paste_edit_->insert_text(this->clipboard_str_.c_str());
                    this->paste_edit_ = nullptr;
                }

                this->long_data_response_size = 0;
            }

            return ;
        }

        RDPECLIP::RecvPredictor rp(stream);

        switch (rp.msgType) {
            case RDPECLIP::CB_FORMAT_LIST:
                RDPECLIP::FormatListPDU().recv(stream);
                this->send_to_front_channel(RDPECLIP::FormatListResponsePDU(true));
                this->has_clipboard_ = false;
                this->clipboard_str_.clear();
                break;
            //case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            //    break;
            //case RDPECLIP::CB_FORMAT_DATA_REQUEST:
            //    RDPECLIP::FormatDataRequestPDU().recv(stream);
            //    this->send_to_front_channel_and_set_buf_size(
            //        this->clipboard_str_.size() * 2 /*utf8 to utf16*/ + sizeof(RDPECLIP::CliprdrHeader) + 4 /*data_len*/,
            //        RDPECLIP::FormatDataResponsePDU(true), this->clipboard_str_.c_str()
            //    );
            //    break;
            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
                format_data_response_pdu.recv(stream);
                if (format_data_response_pdu.header.msgFlags() == RDPECLIP::CB_RESPONSE_OK) {

                    if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                        if (!stream.in_check_rem(format_data_response_pdu.header.dataLen())) {
                            LOG( LOG_ERR
                               , "selector::send_to_selector truncated CB_FORMAT_DATA_RESPONSE dataU16, need=%" PRIu32 " remains=%zu"
                               , format_data_response_pdu.header.dataLen(), stream.in_remain());
                            throw Error(ERR_RDP_PROTOCOL);
                        }

                        this->clipboard_str_.utf16_push_back(stream.get_current(), format_data_response_pdu.header.dataLen() / 2);

                        if (this->paste_edit_) {
                            this->paste_edit_->insert_text(this->clipboard_str_.c_str());
                            this->paste_edit_ = nullptr;
                        }

                        this->has_clipboard_ = true;
                    }
                    else {
                        // Virtual channel data span in multiple Virtual Channel PDUs.

                        if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
                            LOG(LOG_ERR, "selector::send_to_selector flag CHANNEL_FLAG_FIRST expected");
                            throw Error(ERR_RDP_PROTOCOL);
                        }

                        this->long_data_response_size = format_data_response_pdu.header.dataLen() - stream.in_remain();
                        this->clipboard_str_.utf16_push_back(stream.get_current(), stream.in_remain() / 2);
                    }
                }
                break;
            }
            default:
                break;
        }
    }

private:
    template<class PDU, class... Args>
    void send_to_front_channel_(OutStream & out_s, PDU && pdu, Args && ...args) {
        pdu.emit(out_s, args...);
        const size_t length     = out_s.get_offset();
        const size_t chunk_size = length;
        this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                      CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
    }

    template<class PDU, class... Args>
    void send_to_front_channel(PDU && pdu, Args && ...args) {
        StaticOutStream<256> out_s;
        this->send_to_front_channel_(out_s, std::move(pdu), args...);
    }
};


inline
void copy_paste_process_event(CopyPaste & copy_paste, WidgetEdit & widget_edit, NotifyApi::notify_event_t event) {
    switch(event) {
        case NOTIFY_PASTE:
            copy_paste.paste(widget_edit);
            break;
        //TODO enable copy/cut
        //case NOTIFY_COPY:
        //    copy_paste.copy(widget_edit.get_text());
        //    break;
        //case NOTIFY_CUT:
        //    copy_paste.copy(widget_edit.get_text());
        //    widget_edit.set_text("");
        //    break;
        default: ;
    }
}

