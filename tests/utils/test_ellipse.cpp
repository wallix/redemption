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
   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
              Meng Tan

   Unit test to ellipse object
*/


#define RED_TEST_MODULE TestEllipse
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/ellipse.hpp"

RED_TEST_DONT_PRINT_LOG_VALUE(Ellipse)


RED_AUTO_TEST_CASE(TestEllipse)
{

    Ellipse nil;

    RED_CHECK_EQUAL(0, nil.center_x());
    RED_CHECK_EQUAL(0, nil.center_y());
    RED_CHECK_EQUAL(0, nil.radius_x());
    RED_CHECK_EQUAL(0, nil.radius_y());

    Ellipse empty(0, 0, 0, 0);

    RED_CHECK_EQUAL(0, nil.center_x());
    RED_CHECK_EQUAL(0, nil.center_y());
    RED_CHECK_EQUAL(0, nil.radius_x());
    RED_CHECK_EQUAL(0, nil.radius_y());

    Ellipse elli = Ellipse::since_center(70, 60, 45, 32);

    RED_CHECK_EQUAL(70, elli.center_x());
    RED_CHECK_EQUAL(60, elli.center_y());
    RED_CHECK_EQUAL(45, elli.radius_x());
    RED_CHECK_EQUAL(32, elli.radius_y());

    RED_CHECK_EQUAL(25, elli.left());
    RED_CHECK_EQUAL(28, elli.top());
    RED_CHECK_EQUAL(115, elli.right());
    RED_CHECK_EQUAL(92, elli.bottom());

    RED_CHECK_EQUAL(90, elli.width());
    RED_CHECK_EQUAL(64, elli.height());

    Rect rect = elli.get_rect();

    RED_CHECK_EQUAL(rect.x, elli.left());
    RED_CHECK_EQUAL(rect.y, elli.top());
    RED_CHECK_EQUAL(rect.cx, elli.width());
    RED_CHECK_EQUAL(rect.cy, elli.height());

    Ellipse elliclone(rect);

    RED_CHECK(elli == elliclone);

    Ellipse elli2(Rect(100, 200, 40, 60));
    Rect rect2 = elli2.get_rect();

    RED_CHECK_EQUAL(rect2.x, 100);
    RED_CHECK_EQUAL(rect2.y, 200);
    RED_CHECK_EQUAL(rect2.cx, 40);
    RED_CHECK_EQUAL(rect2.cy, 60);
}
