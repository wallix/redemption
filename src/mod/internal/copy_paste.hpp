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
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/zstring_view.hpp"

#include <cstdint>
#include <cstddef>


class FrontAPI;
class Widget;
class InStream;
namespace CHANNELS
{
    class ChannelDef;
} // namespace CHANNELS


class CopyPaste
{
public:
    explicit CopyPaste(bool verbose)
    : verbose(verbose)
    {}

    CopyPaste(const CopyPaste &) = delete;
    CopyPaste & operator=(const CopyPaste &) = delete;

    bool ready(FrontAPI & front);

    explicit operator bool () const noexcept
    {
        return this->channel_;
    }

    void paste(Widget & widget);

    void copy(chars_view str);

    void send_to_mod_channel(InStream & chunk, uint32_t flags);

private:
    FrontAPI * front_ = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;
    Widget * pasted_widget_ = nullptr;

    class LimitString
    {
    public:
        static const std::size_t static_size = 1024/* * 4*/;

        char buf_[static_size];
        std::size_t size_ = 0;

    public:
        LimitString() = default;

        static constexpr std::size_t max_size()
        {
            return static_size - 1;
        }

        void utf16_push_back(const uint8_t * s, std::size_t n);

        void assign(char const * s, std::size_t n);

        zstring_view zstring() const;

        void clear()
        {
            this->size_ = 0;
        }
    };

    LimitString clipboard_str_;
    size_t long_data_response_size = 0;
    bool has_clipboard_ = false;
    bool client_use_long_format_names = false;
    bool verbose;
};

void copy_paste_process_event(
    CopyPaste & copy_paste, Widget & widget,
    NotifyApi::notify_event_t event);
