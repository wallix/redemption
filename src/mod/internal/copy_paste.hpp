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

#include "mod/internal/widget/notify_api.hpp"

#include <cstdint>


class FrontAPI;
class WidgetEdit;
class InStream;
namespace CHANNELS
{
    class ChannelDef;
}


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
        std::size_t size_ = 0;

    public:
        LimitString() = default;

        static constexpr std::size_t max_size()
        {
            return sizeof(buf_) / sizeof(buf_[0]) - 1;
        }

        void utf16_push_back(const uint8_t * s, std::size_t n);

        void assign(char const * s, std::size_t n);

        const char * c_str() /*const*/;

        std::size_t size() const
        {
            return this->size_;
        }

        void clear()
        {
            this->size_ = 0;
            this->widget_edit_buf_is_computed = false;
        }
    };

    LimitString clipboard_str_;
    bool has_clipboard_ = false;
    std::size_t long_data_response_size = 0;

    bool verbose;

public:
    CopyPaste(bool verbose = false)
    : verbose(verbose)
    {}

    CopyPaste(const CopyPaste &) = delete;
    CopyPaste & operator=(const CopyPaste &) = delete;

    bool ready(FrontAPI & front);

    explicit operator bool () const noexcept
    {
        return this->channel_;
    }

    void paste(WidgetEdit & edit);

    void copy(const char * s, std::size_t n);

    void copy(const char * s);

    void send_to_mod_channel(InStream & chunk, uint32_t flags);
};

void copy_paste_process_event(
    CopyPaste & copy_paste, WidgetEdit & widget_edit,
    NotifyApi::notify_event_t event);
