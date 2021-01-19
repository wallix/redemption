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

#include "utils/monotonic_clock.hpp"

#include <sys/time.h>

inline timeval to_timeval(MonotonicTimePoint::duration d)
{
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(d).count();
    timeval tv;
    tv.tv_sec = us / 1'000'000;
    tv.tv_usec = us % 1'000'000;
    return tv;
}

inline timeval to_timeval(MonotonicTimePoint tp)
{
    return to_timeval(tp.time_since_epoch());
}
