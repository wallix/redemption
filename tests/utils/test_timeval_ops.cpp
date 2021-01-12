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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/timeval_ops.hpp"

template<class Ch, class Tr>
std::basic_ostream<Ch, Tr> &
operator<<(std::basic_ostream<Ch, Tr> & out, timeval const & tv)
{
    return out << "{" << tv.tv_sec << ", " << tv.tv_usec << "}";
}

namespace std /*NOLINT*/
{
    // this is a hack...
    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, std::chrono::microseconds const & duration)
    {
        return out << duration.count();
    }
} // namespace std

RED_AUTO_TEST_CASE(TestOps)
{
    RED_TEST(!(timeval{1, 6} == timeval{2, 6}));
    RED_TEST((timeval{1, 6} == timeval{1, 6}));

    RED_TEST((timeval{1, 6} != timeval{2, 6}));
    RED_TEST(!(timeval{1, 6} != timeval{1, 6}));

    RED_TEST((timeval{1, 6} < timeval{2, 6}));
    RED_TEST((timeval{1, 6} < timeval{2, 0}));
    RED_TEST(!(timeval{1, 6} < timeval{1, 6}));
    RED_TEST(!(timeval{1, 6} < timeval{1, 0}));
    RED_TEST(!(timeval{3, 0} < timeval{2, 999'999}));

    RED_TEST((timeval{1, 6} <= timeval{2, 6}));
    RED_TEST((timeval{1, 6} <= timeval{2, 0}));
    RED_TEST((timeval{1, 6} <= timeval{1, 6}));
    RED_TEST(!(timeval{1, 6} <= timeval{1, 0}));
    RED_TEST(!(timeval{3, 0} <= timeval{2, 999'999}));

    RED_TEST(!(timeval{1, 6} > timeval{2, 6}));
    RED_TEST(!(timeval{1, 6} > timeval{2, 0}));
    RED_TEST(!(timeval{1, 6} > timeval{1, 6}));
    RED_TEST((timeval{1, 6} > timeval{1, 0}));
    RED_TEST((timeval{3, 0} > timeval{2, 999'999}));

    RED_TEST(!(timeval{1, 6} >= timeval{2, 6}));
    RED_TEST(!(timeval{1, 6} >= timeval{2, 0}));
    RED_TEST((timeval{1, 6} >= timeval{1, 6}));
    RED_TEST((timeval{1, 6} >= timeval{1, 0}));
    RED_TEST((timeval{3, 0} >= timeval{2, 999'999}));

    RED_TEST((timeval{1, 6} - timeval{1, 8}) ==         -2us);
    RED_TEST((timeval{1, 6} - timeval{2, 1}) ==   -999'995us);
    RED_TEST((timeval{1, 6} - timeval{1, 6}) ==          0us);
    RED_TEST((timeval{1, 6} - timeval{1, 2}) ==          4us);
    RED_TEST((timeval{1, 6} - timeval{0, 2}) ==  1'000'004us);
    RED_TEST((timeval{5, 6} - timeval{2, 2}) ==  3'000'004us);
    RED_TEST((timeval{3, 0} - timeval{2, 999'999}) ==    1us);
    RED_TEST((timeval{3, 0} - timeval{0, 1'999'999}) == 1'000'001us);
    RED_TEST((timeval{3, 0} - timeval{0, 4'999'999}) == -1'999'999us);

    RED_TEST((timeval{123, 456'789} + 1us) == (timeval{123, 456'790}));
    RED_TEST((timeval{123, 456'789} + 600'000us) == (timeval{124, 56'789}));
    RED_TEST((timeval{123, 456'789} + 1'900ms) == (timeval{125, 356'789}));
    RED_TEST((timeval{123, 456'789} + 1'234'567us) == (timeval{124, 691'356}));
    RED_TEST((timeval{123, 456'789} + 1'800'000us) == (timeval{125, 256'789}));
    RED_TEST((timeval{123, 1'456'789} + 1'800'000us) == (timeval{126, 256'789}));

    auto as_ref = [](timeval&& tv) -> timeval& { return tv; };
    RED_TEST((as_ref(timeval{123, 456'789}) += 1us) == (timeval{123, 456'790}));
    RED_TEST((as_ref(timeval{123, 456'789}) += 600'000us) == (timeval{124, 56'789}));
    RED_TEST((as_ref(timeval{123, 456'789}) += 1'900ms) == (timeval{125, 356'789}));
    RED_TEST((as_ref(timeval{123, 456'789}) += 1'234'567us) == (timeval{124, 691'356}));
    RED_TEST((as_ref(timeval{123, 456'789}) += 1'800'000us) == (timeval{125, 256'789}));
    RED_TEST((as_ref(timeval{123, 1'456'789}) += 1'800'000us) == (timeval{126, 256'789}));

    RED_TEST(to_timeval(10s) == (timeval{10, 0}));
    RED_TEST(to_timeval(10ms) == (timeval{0, 10'000}));
    RED_TEST(to_timeval(123'456'789ms) == (timeval{123'456, 789'000}));
    RED_TEST(to_timeval(123'456'789ms) == (timeval{123'456, 789'000}));
    RED_TEST(to_timeval(123'456'789us) == (timeval{123, 456'789}));
    RED_TEST(to_timeval(123'456'789us) == (timeval{123, 456'789}));

    RED_TEST(to_ms(timeval{2, 44}) == 2'000ms);
    RED_TEST(to_ms(timeval{2, 1'005'000}) == 3'005ms);

    RED_TEST(ustime(timeval{2, 44}) == 2'000'044us);
    RED_TEST(ustime(timeval{2, 1'005'000}) == 3'005'000us);
}
