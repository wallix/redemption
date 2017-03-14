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
        static const std::size_t static_size = 1024/* * 4*/;

        char buf_[static_size];
        char widget_edit_buf_[static_size];
        char * widget_edit_buf_selected_;
        bool widget_edit_buf_is_computed = false;
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
            this->widget_edit_buf_is_computed = false;
        }

        void assign(char const * s, size_t n) {
            this->size_ = ((this->max_size() >= n) ? n :
                           ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(s), this->max_size()));
            memcpy(this->buf_, s, this->size_);
            this->buf_[this->size_] = 0;
            this->widget_edit_buf_is_computed = false;
        }

        const char * c_str() /*const*/ {
            if (!this->widget_edit_buf_is_computed) {
                this->widget_edit_buf_is_computed = true;
                auto const data = this->buf_;
                auto const sz = this->size();
                auto const data_end = data + sz;
                auto p = std::find_if(data, data_end, [](uint8_t c){
                    static constexpr bool test[256]{0,0,0,0,0,0,0,0,0,1,1,0,0,1}; // \t \n \r
                    return test[c];
                });
                if (p == data_end) {
                    this->widget_edit_buf_selected_ = this->buf_;
                }
                // ignore multi-line
                else {
                    this->widget_edit_buf_selected_ = this->widget_edit_buf_;
                    memcpy(this->widget_edit_buf_, data, p - data);
                    auto pnew = this->widget_edit_buf_ + (p - data);
                    if (*p == '\t') {
                        static constexpr bool test[256]{0,0,0,0,0,0,0,0,0,0,1,0,0,1}; // \n \r
                        static constexpr uint8_t lookup[256]{
                            0,1,2,3,4,5,6,7,8,' ',10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
                            25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,
                            47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,
                            69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,
                            91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,
                            110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,
                            127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
                            144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
                            161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,
                            178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,
                            195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,
                            212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,
                            229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,
                            246,247,248,249,250,251,252,253,254,255
                        }; // \t -> ' '
                        for (; p != data_end && !test[uint8_t(*p)]; ++pnew, ++p) {
                            *pnew = char(lookup[uint8_t(*p)]);
                        }
                    }
                    *pnew = '\0';
                }
            }

            return this->widget_edit_buf_selected_;
        }

        size_t size() const {
            return this->size_;
        }

        void clear() {
            this->size_ = 0;
            this->widget_edit_buf_is_computed = false;
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

