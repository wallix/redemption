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
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/static_fmt.hpp"

RED_AUTO_TEST_CASE(TestStaticFmt)
{
    RED_CHECK("%u-%s"_static_fmt(1u, 3) == "1-3"_av);
    RED_CHECK("%u-%s"_static_fmt.set_max_size<50>()(1u, 3) == "1-3"_av);
    RED_CHECK("%u-%s-%s"_static_fmt(1u, "plop"_sized_av, 3) == "1-plop-3"_av);
    RED_CHECK("%x-%X-%x-%X-%X-%c"_static_fmt(
        uint16_t(0xfedc), uint16_t(0xfedc), uint16_t(0xa), uint16_t(0), uint16_t(0x123), 'n'
    ) == "fedc-FEDC-a-0-123-n"_av);
    RED_CHECK("%h-%H"_static_fmt(uint16_t(0xfed), uint16_t(0xfed)) == "0fed-0FED"_av);

    static_string<100> s;
    "%u-%s-%s"_static_fmt.write_to(s, 1u, "plop"_sized_av, 3);
    RED_CHECK(s == "1-plop-3"_av);
}
