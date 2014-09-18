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

#ifndef REDEMPTION_MOD_INTERNAL_COPY_PASTE_UTILITY_HPP
#define REDEMPTION_MOD_INTERNAL_COPY_PASTE_UTILITY_HPP

#include "RDP/clipboard.hpp"
#include "RDP/gcc.hpp"
#include "channel_list.hpp"
#include "stream.hpp"
#include "array.hpp"
#include "front_api.hpp"

#include <utility>
#include <algorithm>

class WidgetEdit;

namespace aux_ {
    void insert_text_in_widget_edit(const char *, WidgetEdit &);
}

//TODO Cut
class CopyPaste
{
    FrontAPI * front_ = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;
    WidgetEdit * paste_edit_ = nullptr;

    struct LimitString
    {
        char buf[1024 * 4];
        size_t size;

        LimitString()
        : size(0)
        {}

        /*C++14 constexpr*/ size_t max_size() const {
            return sizeof(this->buf) / sizeof(this->buf[0]) - 1;
        }

        void push_back(char const * s, size_t n) {
            n = std::min(this->size + n, this->max_size());
            memcpy(this->buf + this->size, s, n);
            this->size += n;
            this->buf[this->size] = 0;
        }

        const char * c_str() const {
            return this->buf;
        }
    };

    LimitString clipboard_str;

public:
    CopyPaste() = default;
    CopyPaste(const CopyPaste &) = delete;
    CopyPaste & operator=(const CopyPaste &) = delete;

    bool ready(FrontAPI & front) {
        this->front_ = &front;
        this->channel_ = front.get_channel_list().get_by_name(CLIPBOARD_VIRTUAL_CHANNEL_NAME);

        if (this->channel_) {
            this->send_to_front_channel(RDPECLIP::ServerMonitorReadyPDU());
            return true;
        }

        return false;
    }

    operator bool () const noexcept {
        return this->channel_;
    }

    void paste(WidgetEdit & edit) {
        this->clipboard_str.size = 0;
        this->send_to_front_channel(RDPECLIP::FormatDataRequestPDU(RDPECLIP::CF_UNICODETEXT));
        this->paste_edit_ = &edit;
    }

//     void copy() {
//
//     }

    void send_to_mod_channel(Stream & chunk, uint32_t flags)
    {
        SubStream stream(chunk, 0, chunk.size());
        RDPECLIP::RecvFactory recv_factory(stream);

        switch (recv_factory.msgType) {
            case RDPECLIP::CB_FORMAT_LIST:
                RDPECLIP::FormatListPDU().recv(stream, recv_factory);
                this->send_to_front_channel(RDPECLIP::FormatListResponsePDU(true));
                break;
            //case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            //    break;
            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                RDPECLIP::FormatDataRequestPDU().recv(stream, recv_factory);
                this->send_to_front_channel(RDPECLIP::FormatDataResponsePDU(true), "");
                break;
            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
                format_data_response_pdu.recv(stream, recv_factory);
                if (format_data_response_pdu.msgFlags() == RDPECLIP::CB_RESPONSE_OK) {
                    if ((flags & CHANNELS::CHANNEL_FLAG_LAST) != 0) {
                        if (!stream.in_check_rem(format_data_response_pdu.dataLen())) {
                            LOG( LOG_ERR
                            , "selector::send_to_selector truncated CB_FORMAT_DATA_RESPONSE dataU16, need=%u remains=%u"
                            , format_data_response_pdu.dataLen(), stream.in_remain());
                            throw Error(ERR_RDP_PROTOCOL);
                        }

                        this->clipboard_str.size = UTF16toUTF8(
                            stream.p
                          , format_data_response_pdu.dataLen() / 2
                          , reinterpret_cast<uint8_t*>(this->clipboard_str.buf)
                          , this->clipboard_str.max_size()
                        );
                        this->clipboard_str.buf[this->clipboard_str.size] = 0;

                        if (this->paste_edit_) {
                            aux_::insert_text_in_widget_edit(this->clipboard_str.c_str(), *this->paste_edit_);
                            this->paste_edit_ = nullptr;
                        }
                    }
                    else {
                        // Virtual channel data span in multiple Virtual Channel PDUs.

                        if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) == 0) {
                            LOG(LOG_ERR, "selector::send_to_selector flag CHANNEL_FLAG_FIRST expected");
                            throw Error(ERR_RDP_PROTOCOL);
                        }

                        this->clipboard_str.push_back(reinterpret_cast<char const *>(stream.get_data()), stream.size());
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
    void send_to_front_channel(PDU && pdu, Args && ...args) {
        BStream out_s(256);
        pdu.emit(out_s, std::move(args)...);
        const size_t length     = out_s.size();
        const size_t chunk_size = length;
        this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                      CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
    }
};



#endif
