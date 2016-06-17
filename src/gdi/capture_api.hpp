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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_GDI_CAPTURE_API_HPP
#define REDEMPTION_GDI_CAPTURE_API_HPP

#include "utils/sugar/noncopyable.hpp"

#include <chrono>

#include <cassert>


struct timeval;
class Inifile;

namespace gdi {

struct CaptureApi : private noncopyable
{
    virtual ~CaptureApi() = default;

    std::chrono::microseconds snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) {
        // assert(now >= previous);
        auto next_duration = this->do_snapshot(now, cursor_x, cursor_x, ignore_frame_in_timeval);
        assert(next_duration.count() >= 0);
        return next_duration;
    }

    void pause_capture(timeval const & now) {
        // assert(now >= previous);
        this->do_pause_capture(now);
    }

    void resume_capture(timeval const & now) {
        // assert(now >= previous);
        this->do_resume_capture(now);
    }

private:
    virtual std::chrono::microseconds do_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) = 0;

    virtual void do_pause_capture(timeval const &) {}
    virtual void do_resume_capture(timeval const &) {}
};


struct ExternalCaptureApi : private noncopyable
{
    virtual void external_breakpoint() = 0;
    virtual void external_time(timeval const & now) = 0;

    virtual ~ExternalCaptureApi() = default;
};


struct UpdateConfigCaptureApi : private noncopyable
{
    virtual void update_config(Inifile const & ini) = 0;

    virtual ~UpdateConfigCaptureApi() = default;
};

}

#endif
