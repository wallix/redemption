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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <cstdio>
#include <ctime>

#include <string>
#include <chrono>

#include "utils/sugar/bytes_t.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "transport/transport.hpp"

#include "gdi/capture_api.hpp"
#include "capture/new_kbdcapture.hpp"


namespace {
    constexpr array_view_const_char session_meta_kbd_prefix() { return cstr_array_view("[Kbd]"); }
    constexpr array_view_const_char session_meta_kbd_suffix() { return cstr_array_view("\n"); }
}

/*
* Format:
*
* $date ' ' [+-] ' ' $title? '[Kbd]' $kbd
* $date ' - ' $line
*/
class SessionMeta final : public TextKbd<SessionMeta>, public gdi::CaptureApi, public gdi::CaptureProbeApi
{
    uint8_t kbd_buffer[1024];
    timeval last_snapshot;
    time_t last_flush;
    Transport & trans;
    std::string title;
    bool require_kbd = false;
    char current_seperator = '+';
    bool is_probe_enabled_session = false;

public:
    SessionMeta(const timeval & now, Transport & trans)
    : TextKbd<SessionMeta>({
        this->kbd_buffer + session_meta_kbd_prefix().size(),
        sizeof(this->kbd_buffer) - session_meta_kbd_prefix().size() - session_meta_kbd_suffix().size()
    })
    , last_snapshot(now)
    , last_flush(now.tv_sec)
    , trans(trans)
    {
        OutStream(this->kbd_buffer).out_copy_bytes(session_meta_kbd_prefix().data(), session_meta_kbd_prefix().size());

        // force file creation even if no text recognized
        this->trans.send("", 0);
    }

    ~SessionMeta() {
        this->send_kbd();
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys();
            }
        }
        else {
            this->write_keys(uchar);
        }
        return true;
    }

    void title_changed(time_t rawtime, array_view_const_char title) {
        this->send_kbd();
        this->send_date(rawtime, '+');
        this->trans.send(title.data(), title.size());
        this->last_flush = rawtime;

        this->title.assign(title.data(), title.size());
        this->require_kbd = true;
    }

    void send_line(time_t rawtime, array_view_const_char line) {
        this->send_kbd();
        this->send_date(rawtime, '+');
        this->trans.send(line.data(), line.size());
        this->trans.send("\n", 1);
        this->last_flush = rawtime;
    }

    void session_update(const timeval& /*now*/, array_view_const_char message) override {
        this->is_probe_enabled_session = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
    }

    void possible_active_window_change() override {
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const time_to_wait = std::chrono::seconds{2};
        std::chrono::microseconds const diff {difftimeval(now, this->last_snapshot)};

        if (diff < time_to_wait && this->kbd_stream.get_offset() < 8 * sizeof(uint32_t)) {
            return time_to_wait;
        }

        this->send_kbd();

        this->last_snapshot = now;
        this->last_flush = this->last_snapshot.tv_sec;

        return time_to_wait;
    }

    friend TextKbd<SessionMeta>;
    void flush() {
        this->send_kbd();
    }

    void send_date(time_t rawtime, char sep) {
        tm ptm;
        localtime_r(&rawtime, &ptm);

        char string_date[256];

        auto const data_sz = std::sprintf(
            string_date, "%4d-%02d-%02d %02d:%02d:%02d %c ",
            ptm.tm_year+1900, ptm.tm_mon+1, ptm.tm_mday,
            ptm.tm_hour, ptm.tm_min, ptm.tm_sec, sep
        );

        this->trans.send(string_date, data_sz);
    }

    void send_kbd() {
        if (this->kbd_stream.get_offset()) {
            if (!this->require_kbd) {
                this->send_date(this->last_flush, this->current_seperator);
                this->trans.send(this->title.data(), this->title.size());
            }
            auto end = this->kbd_stream.get_current();
            memcpy(end, session_meta_kbd_suffix().data(), session_meta_kbd_suffix().size());
            end += session_meta_kbd_suffix().size();
            this->trans.send(this->kbd_buffer, std::size_t(end - this->kbd_buffer));
            this->kbd_stream.rewind();
            this->require_kbd = false;
        }
        else if (this->require_kbd) {
            this->trans.send("\n", 1);
            this->require_kbd = false;
        }
        this->current_seperator = '-';
    }
};
