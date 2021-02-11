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

#include "utils/monotonic_clock.hpp"
#include "utils/real_clock.hpp"

struct MonotonicTimeToRealTime
{
    MonotonicTimeToRealTime(MonotonicTimePoint monotonic_time, RealTimePoint real_time)
    : monotonic_to_real(real_time.time_since_epoch() - monotonic_time.time_since_epoch())
    {}

    RealTimePoint to_real_time_point(MonotonicTimePoint monotonic_time) const
    {
        return RealTimePoint(to_real_time_duration(monotonic_time));
    }

    MonotonicTimePoint::duration to_real_time_duration(MonotonicTimePoint monotonic_time) const
    {
        return (monotonic_time + monotonic_to_real).time_since_epoch();
    }

private:
    MonotonicTimePoint::duration monotonic_to_real;
};
