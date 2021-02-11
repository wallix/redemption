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
   Author(s): Proxies team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/to_timeval.hpp"

using namespace std::literals::chrono_literals;

template<class Ch, class Tr>
static inline std::basic_ostream<Ch, Tr>&
operator<<(std::basic_ostream<Ch, Tr>& out, timeval const& tv)
{
    return out << "{" << tv.tv_sec << ", " << tv.tv_usec << "}";
}

static inline bool operator==(timeval const& a, timeval const& b) noexcept
{
    return a.tv_sec == b.tv_sec && a.tv_usec == b.tv_usec;
}

RED_AUTO_TEST_CASE(TestOps)
{
    RED_TEST(to_timeval(MonotonicTimePoint(10s)) == (timeval{10, 0}));
    RED_TEST(to_timeval(MonotonicTimePoint(10ms)) == (timeval{0, 10'000}));
    RED_TEST(to_timeval(MonotonicTimePoint(123'456'789ms)) == (timeval{123'456, 789'000}));
    RED_TEST(to_timeval(MonotonicTimePoint(123'456'789ms)) == (timeval{123'456, 789'000}));
    RED_TEST(to_timeval(MonotonicTimePoint(123'456'789us)) == (timeval{123, 456'789}));
    RED_TEST(to_timeval(MonotonicTimePoint(123'456'789us)) == (timeval{123, 456'789}));
}
