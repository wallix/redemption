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
Copyright (C) Wallix 2020
Author(s):Christophe Grosjean
*/

#pragma once

#include "utils/monotonic_clock.hpp"

#include <cassert>


struct TimeBase
{
    explicit TimeBase(
        MonotonicTimePoint monotonic_time,
        DurationFromMonotonicTimeToRealTime monotonic_to_real)
    : monotonic_time(monotonic_time)
    , monotonic_to_real(monotonic_to_real)
    {}

    void set_current_time(MonotonicTimePoint const& now)
    {
        assert(now >= this->monotonic_time);
        this->monotonic_time = now;
    }

    [[nodiscard]] MonotonicTimePoint get_current_time() const
    {
        return this->monotonic_time;
    }

    [[nodiscard]] DurationFromMonotonicTimeToRealTime
    get_duration_from_monotonic_time_to_real_time() const
    {
        return this->monotonic_to_real;
    }

private:
    MonotonicTimePoint monotonic_time;
    DurationFromMonotonicTimeToRealTime monotonic_to_real;
};
