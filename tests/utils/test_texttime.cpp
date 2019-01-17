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
*1324
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Clément Moroldo, Christophe Grosjean
*/

#define RED_TEST_MODULE TestTexttime
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/timeval_ops.hpp"
#include "utils/texttime.hpp"

RED_AUTO_TEST_CASE(TestTextdate)
{
    using namespace std::literals::chrono_literals;

    RED_CHECK_SMEM("1970-01-01"_av, text_gmdate(to_timeval(0s)));
    RED_CHECK_SMEM("2018-08-02"_av, text_gmdate(to_timeval(1533211681s)));

    RED_CHECK_SMEM("1970-01-01 00:00:00"_av, text_gmdatetime(to_timeval(0s)));
    RED_CHECK_SMEM("2018-08-02 12:08:01"_av, text_gmdatetime(to_timeval(1533211681s)));

    RED_CHECK_SMEM("1970-01-01_00-00-00"_av, filename_gmdatetime(to_timeval(0s)));
    RED_CHECK_SMEM("2018-08-02_12-08-01"_av, filename_gmdatetime(to_timeval(1533211681s)));

    RED_CHECK_SMEM("Jan 01 1970 00:00:00"_av, arcsight_gmdatetime(to_timeval(0s)));
    RED_CHECK_SMEM("Aug 02 2018 12:08:01"_av, arcsight_gmdatetime(to_timeval(1533211681s)));
}
