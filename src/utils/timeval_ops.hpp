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

// date differences is returning how long to wait to reach ultimatum
// which means 0 if starttime if after ultimatum
inline std::chrono::microseconds operator-(timeval const & ultimatum, timeval const & starttime)
{
    using namespace std::chrono_literals;    
    if (ultimatum <= starttime) {
        return 0us;
    }
    return std::chrono::seconds(ultimatum.tv_sec) + std::chrono::microseconds(ultimatum.tv_usec)
         - std::chrono::seconds(starttime.tv_sec) - std::chrono::microseconds(starttime.tv_usec);
}

// TODO: should not exist, adding two dates is meaningless
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

inline timeval to_timeval(std::chrono::seconds const& seconds)
{
    return {seconds.count(), 0};
}

inline timeval to_timeval(std::chrono::microseconds const& usec)
{
    return {usec.count()/1000000, usec.count()%1000000};
}

// Returns the beginning of the timeslice of width seconds containing timeval
// origin of intervals is midnight 1 jan 1970
inline timeval timeslice(timeval const & a, std::chrono::seconds const& seconds)
{
    return timeval{a.tv_sec - a.tv_sec % seconds.count(), 0};
}

inline bool is_midnight(timeval const & a)
{
    using namespace std::chrono_literals;
    return (a.tv_sec % std::chrono::seconds(24h).count()) == 0;
}

inline timeval operator+(timeval const & a, std::chrono::seconds const& seconds)
{
    timeval result = a;
    result.tv_sec  += seconds.count();
    return result;
}

inline timeval operator+(timeval const & a, std::chrono::milliseconds const& ms)
{
    std::chrono::microseconds usec = std::chrono::seconds(a.tv_sec) + std::chrono::microseconds(a.tv_usec) + ms;
    return to_timeval(usec);
}

inline timeval& operator+=(timeval& tv, std::chrono::seconds const& seconds)
{
    tv.tv_sec += seconds.count();
    return tv;
}


