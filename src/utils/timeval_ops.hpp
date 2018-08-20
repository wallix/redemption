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

#include <chrono>

#include <cstdint>
#include <cassert>
#include <sys/time.h>


inline bool operator<(const timeval & a, const timeval & b) noexcept {
    // return ustime(a) < ustime(b)
    return a.tv_sec < b.tv_sec
       || (a.tv_sec == b.tv_sec
        && a.tv_usec < b.tv_usec);
}

inline bool operator==(const timeval & a, const timeval & b) noexcept {
    return a.tv_sec == b.tv_sec && a.tv_usec == b.tv_usec;
}

inline bool operator!=(const timeval & a, const timeval & b) noexcept {
    return !(a == b);
}

inline bool operator>(const timeval & a, const timeval & b) noexcept {
    return b < a;
}

inline bool operator<=(const timeval & a, const timeval & b) noexcept {
    return !(b < a);
}

inline bool operator>=(const timeval & a, const timeval & b) noexcept {
    return !(a < b);
}

inline timeval operator-(timeval const & endtime, timeval const & starttime)
{
    assert(endtime >= starttime);

    timeval result;

    result.tv_sec = endtime.tv_sec - starttime.tv_sec;

    if (endtime.tv_usec >= starttime.tv_usec) {
        result.tv_usec = endtime.tv_usec - starttime.tv_usec;
    }
    else {
        result.tv_sec--;

        result.tv_usec = 1000000LL - starttime.tv_usec + endtime.tv_usec;
    }

    return result;
}

inline timeval operator+(timeval const & a, timeval const & b)
{
    timeval result;

    result.tv_sec  = a.tv_sec  + b.tv_sec;
    result.tv_usec = a.tv_usec + b.tv_usec;
    if (result.tv_usec >= 1000000LL) {
        result.tv_sec++;

        result.tv_usec -= 1000000LL;
    }

    return result;
}

inline timeval& operator+=(timeval& tv, std::chrono::seconds const& seconds)
{
    tv.tv_sec += seconds.count();
    return tv;
}

inline timeval to_timeval(std::chrono::seconds const& seconds)
{
    return {seconds.count(), 0};
}
