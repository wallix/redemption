/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/monotonic_clock.hpp"
#include "utils/timeval_ops.hpp"

RED_AUTO_TEST_CASE(TestClock)
{
    auto t1 = get_monotonic_timeval_clock();
    auto t2 = get_monotonic_ms_clock();
    auto t3 = get_monotonic_us_clock();

    RED_CHECK(t1.tv_sec > 0);
    RED_CHECK(t2.count() > 0);
    RED_CHECK(t3.count() > 0);

    RED_CHECK((t3 >= t2));
    RED_CHECK((t2 >= to_ms(t1)));
}
