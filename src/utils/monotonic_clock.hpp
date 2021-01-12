/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxy Team
*/

#pragma once

#include <chrono>

#include <time.h>

// TODO
#include <sys/time.h>
inline timeval tvtime()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv;
}

inline timespec get_monotonic_timespec_clock()
{
    timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp;
}

inline timeval get_monotonic_timeval_clock()
{
    auto tp = get_monotonic_timespec_clock();
    return {tp.tv_sec, tp.tv_nsec / 1000};
}

inline std::chrono::milliseconds get_monotonic_ms_clock()
{
    auto tp = get_monotonic_timespec_clock();
    return std::chrono::milliseconds(tp.tv_sec*1000 + tp.tv_nsec/1000000);
}

inline std::chrono::microseconds get_monotonic_us_clock()
{
    auto tp = get_monotonic_timespec_clock();
    return std::chrono::microseconds(tp.tv_sec*1000000 + tp.tv_nsec/1000);
}
