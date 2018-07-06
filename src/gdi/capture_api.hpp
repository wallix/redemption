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


#pragma once

#include "utils/sugar/noncopyable.hpp"

#include <chrono>
#include <vector>

#include <cassert>

#include "utils/rect.hpp"

struct timeval;

namespace RDP {
    namespace RAIL {
        class Rectangle;
    } // namespace RAIL
} // namespace RDP

namespace gdi {

struct CaptureApi : private noncopyable
{
    virtual ~CaptureApi() = default;

    // non_negative<std::chrono::microseconds>
    struct Microseconds
    {
        Microseconds() = default;

        Microseconds(std::chrono::microseconds const & ms)
          : ms_(ms)
        {
            assert(ms_.count() >= 0);
        }

        template<class Rep, class Period>
        Microseconds(std::chrono::duration<Rep, Period> const & duration)
          : Microseconds(std::chrono::microseconds{duration})
        {}

        std::chrono::microseconds::rep count() const noexcept { return this->ms_.count(); }

        std::chrono::microseconds const & ms() const noexcept { return this->ms_; }
        operator std::chrono::microseconds const & () const noexcept { return this->ms_; }

    private:
        std::chrono::microseconds ms_;
    };

    virtual Microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) = 0;

    virtual void frame_marker_event(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    )
    {
        this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    virtual void visibility_rects_event(Rect const & /*rect*/) {}
};


struct ExternalCaptureApi : private noncopyable
{
    virtual void external_breakpoint() = 0;
    virtual void external_time(timeval const & now) = 0;

    virtual ~ExternalCaptureApi() = default;
};


inline bool operator==(CaptureApi::Microseconds const & a, CaptureApi::Microseconds const & b) noexcept
{
    return a.count() == b.count();
}

inline bool operator!=(CaptureApi::Microseconds const & a, CaptureApi::Microseconds const & b) noexcept
{
    return a.count() != b.count();
}

inline bool operator<(CaptureApi::Microseconds const & a, CaptureApi::Microseconds const & b) noexcept
{
    return a.count() < b.count();
}

inline bool operator>(CaptureApi::Microseconds const & a, CaptureApi::Microseconds const & b) noexcept
{
    return a.count() > b.count();
}

inline bool operator<=(CaptureApi::Microseconds const & a, CaptureApi::Microseconds const & b) noexcept
{
    return a.count() <= b.count();
}

inline bool operator>=(CaptureApi::Microseconds const & a, CaptureApi::Microseconds const & b) noexcept
{
    return a.count() >= b.count();
}

}  // namespace gdi

