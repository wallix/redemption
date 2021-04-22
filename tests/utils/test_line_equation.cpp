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


#include "utils/line_equation.hpp"


RED_AUTO_TEST_CASE(TestLineEquation)
{
    // Test intersection of a rect with a line (defined by its end points)

    Rect r(100, 200, 300, 150);
    using Region = LineEquation::t_region;
    RED_CHECK_EQUAL((Region::LEFT | Region::UP   ), LineEquation::region_pt(r, 50 , 50 ));
    RED_CHECK_EQUAL((Region::LEFT | Region::UP   ), LineEquation::region_pt(r, 60 , 110));
    RED_CHECK_EQUAL((Region::UP                  ), LineEquation::region_pt(r, 150, 100));
    RED_CHECK_EQUAL((Region::UP   | Region::RIGHT), LineEquation::region_pt(r, 432, 103));
    RED_CHECK_EQUAL((Region::LEFT                ), LineEquation::region_pt(r, 70 , 300));
    RED_CHECK_EQUAL((Region::LEFT | Region::DOWN ), LineEquation::region_pt(r, 70 , 415));
    RED_CHECK_EQUAL((Region::IN                  ), LineEquation::region_pt(r, 170, 299));
    RED_CHECK_EQUAL((Region::RIGHT               ), LineEquation::region_pt(r, 405, 322));
    RED_CHECK_EQUAL((Region::DOWN                ), LineEquation::region_pt(r, 242, 500));
    RED_CHECK_EQUAL((Region::DOWN | Region::RIGHT), LineEquation::region_pt(r, 555, 477));

    // up and left border are in, right and bottom borders are out
    RED_CHECK_EQUAL((Region::IN                  ), LineEquation::region_pt(r, 100, 200));
    RED_CHECK_EQUAL((Region::DOWN | Region::RIGHT), LineEquation::region_pt(r, 400, 350));
    RED_CHECK_EQUAL((Region::RIGHT               ), LineEquation::region_pt(r, 400, 200));
    RED_CHECK_EQUAL((Region::DOWN                ), LineEquation::region_pt(r, 100, 350));

    LineEquation equa(300, 100, 50, 300);
    RED_CHECK(equa.resolve(r));
    RED_CHECK_EQUAL(equa.segin.a.x, 175);
    RED_CHECK_EQUAL(equa.segin.a.y, 200);
    RED_CHECK_EQUAL(equa.segin.b.x, 100);
    RED_CHECK_EQUAL(equa.segin.b.y, 260);

    equa = LineEquation(50, 215, 250, 215);
    RED_CHECK(equa.resolve(r));
    RED_CHECK_EQUAL(equa.segin.a.x, 100);
    RED_CHECK_EQUAL(equa.segin.a.y, 215);
    RED_CHECK_EQUAL(equa.segin.b.x, 250);
    RED_CHECK_EQUAL(equa.segin.b.y, 215);

    equa = LineEquation(50, 215, 500, 215);
    RED_CHECK(equa.resolve(r));
    RED_CHECK_EQUAL(equa.segin.a.x, 100);
    RED_CHECK_EQUAL(equa.segin.a.y, 215);
    RED_CHECK_EQUAL(equa.segin.b.x, 399);
    RED_CHECK_EQUAL(equa.segin.b.y, 215);

    equa = LineEquation(50, 400, 450, 400);
    RED_CHECK(not equa.resolve(r));
    RED_CHECK_EQUAL(equa.segin.a.x, 0);
    RED_CHECK_EQUAL(equa.segin.a.y, 0);
    RED_CHECK_EQUAL(equa.segin.b.x, 0);
    RED_CHECK_EQUAL(equa.segin.b.y, 0);

    equa = LineEquation(150, 100, 450, 400);
    RED_CHECK(equa.resolve(r));
    RED_CHECK_EQUAL(equa.segin.a.x, 250);
    RED_CHECK_EQUAL(equa.segin.a.y, 200);
    RED_CHECK_EQUAL(equa.segin.b.x, 399);
    RED_CHECK_EQUAL(equa.segin.b.y, 349);

    equa = LineEquation(10, 250, 150, 50);
    RED_CHECK(not equa.resolve(r));
    RED_CHECK_EQUAL(equa.segin.a.x, 0);
    RED_CHECK_EQUAL(equa.segin.a.y, 0);
    RED_CHECK_EQUAL(equa.segin.b.x, 0);
    RED_CHECK_EQUAL(equa.segin.b.y, 0);
}
