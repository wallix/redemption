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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#define RED_TEST_MODULE TestImageDataFromPointer
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "redjs/image_data_from_pointer.hpp"
#include "core/RDP/rdp_pointer.hpp"

struct ReadableCursor
{
    char str[32*32+32+1];

    ReadableCursor(redjs::ImageData const& img) noexcept
    {
        auto* it = img.data();
        auto* p = str;
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                *p++ = !it[3] ? '.' : (it[0] == 0xff && it[1] == 0xff && it[2] == 0xff) ? 'X' : '+';
                it += 4;
            }
            *p++ = '\n';
        }
        *p = 0;
    }
};


RED_AUTO_TEST_CASE(TestImageDataFromNormalPointer)
{
    redjs::ImageData img = redjs::image_data_from_pointer(normal_pointer());

    RED_REQUIRE(32u == img.width());
    RED_REQUIRE(32u == img.height());

    ReadableCursor readable_cursor(img);

    RED_CHECK_EQ(readable_cursor.str,
        "X...............................\n"
        "XX..............................\n"
        "X+X.............................\n"
        "X++X............................\n"
        "X+++X...........................\n"
        "X++++X..........................\n"
        "X+++++X.........................\n"
        "X++++++X........................\n"
        "X+++++++X.......................\n"
        "X++++++++X......................\n"
        "X+++++XXXXX.....................\n"
        "X++X++X.........................\n"
        "X+X.X++X........................\n"
        "XX..X++X........................\n"
        "X....X++X.......................\n"
        ".....X++X.......................\n"
        "......X++X......................\n"
        "......X++X......................\n"
        ".......XX.......................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
        "................................\n"
    );
}
