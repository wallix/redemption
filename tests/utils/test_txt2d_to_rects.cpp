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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/txt2d_to_rects.hpp"


RED_AUTO_TEST_CASE(TestTxt2dToRects)
{
    constexpr auto icon1 = TXT2D_TO_RECTS(
        "####    ",
        "####    ",
        "  ####  ",
        "  ####  ",
        "    ####",
        "    ####",
    );

    RED_CHECK_EQUAL_RANGES(icon1, (std::array{
        Rect(0, 0, 2, 2),
        Rect(1, 2, 2, 2),
        Rect(2, 4, 2, 2),
    }));

    constexpr auto icon2 = TXT2D_TO_RECTS(
        "    ####",
        "    ####",
        "  ####  ",
        "  ####  ",
        "####    ",
        "####    ",
    );

    RED_CHECK_EQUAL_RANGES(icon2, (std::array{
        Rect(2, 0, 2, 2),
        Rect(1, 2, 2, 2),
        Rect(0, 4, 2, 2),
    }));
}
