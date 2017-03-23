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


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestEllipse
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "utils/ellipse.hpp"


BOOST_AUTO_TEST_CASE(TestEllipse)
{

    Ellipse nil;

    BOOST_CHECK_EQUAL(0, nil.center_x());
    BOOST_CHECK_EQUAL(0, nil.center_y());
    BOOST_CHECK_EQUAL(0, nil.radius_x());
    BOOST_CHECK_EQUAL(0, nil.radius_y());

    Ellipse empty(0, 0, 0, 0);

    BOOST_CHECK_EQUAL(0, nil.center_x());
    BOOST_CHECK_EQUAL(0, nil.center_y());
    BOOST_CHECK_EQUAL(0, nil.radius_x());
    BOOST_CHECK_EQUAL(0, nil.radius_y());

    Ellipse elli = Ellipse::since_center(70, 60, 45, 32);

    BOOST_CHECK_EQUAL(70, elli.center_x());
    BOOST_CHECK_EQUAL(60, elli.center_y());
    BOOST_CHECK_EQUAL(45, elli.radius_x());
    BOOST_CHECK_EQUAL(32, elli.radius_y());

    BOOST_CHECK_EQUAL(25, elli.left());
    BOOST_CHECK_EQUAL(28, elli.top());
    BOOST_CHECK_EQUAL(115, elli.right());
    BOOST_CHECK_EQUAL(92, elli.bottom());

    BOOST_CHECK_EQUAL(90, elli.width());
    BOOST_CHECK_EQUAL(64, elli.height());

    Rect rect = elli.get_rect();

    BOOST_CHECK_EQUAL(rect.x, elli.left());
    BOOST_CHECK_EQUAL(rect.y, elli.top());
    BOOST_CHECK_EQUAL(rect.cx, elli.width());
    BOOST_CHECK_EQUAL(rect.cy, elli.height());

    Ellipse elliclone(rect);

    BOOST_CHECK(elli == elliclone);

    Ellipse elli2(Rect(100, 200, 40, 60));
    Rect rect2 = elli2.get_rect();

    BOOST_CHECK_EQUAL(rect2.x, 100);
    BOOST_CHECK_EQUAL(rect2.y, 200);
    BOOST_CHECK_EQUAL(rect2.cx, 40);
    BOOST_CHECK_EQUAL(rect2.cy, 60);
}
