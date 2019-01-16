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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to rect object

*/

#define RED_TEST_MODULE TestRect
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/rect.hpp"


RED_AUTO_TEST_CASE(TestRect)
{
    // A rect is defined by it's top left corner, width and height
    Rect r(10, 110, 10, 10);

    RED_CHECK_EQUAL(10, r.x);
    RED_CHECK_EQUAL(110, r.y);
    RED_CHECK_EQUAL(20, r.right());
    RED_CHECK_EQUAL(120, r.bottom());

    /* we can also create an empty rect, it's the default constructor */
    Rect empty;

    RED_CHECK_EQUAL(0, empty.x);
    RED_CHECK_EQUAL(0, empty.y);
    RED_CHECK_EQUAL(0, empty.right());
    RED_CHECK_EQUAL(0, empty.bottom());

    /* test if rect is empty */
    RED_CHECK_EQUAL(true, empty.isempty());
    RED_CHECK_EQUAL(false, r.isempty());

    /* this one is empty because left is after right */
    Rect e1(20, 110, -10, 10);
    RED_CHECK_EQUAL(true, e1.isempty());


    Rect e2(10, 110, 0, 0);
    RED_CHECK_EQUAL(true, e2.isempty());

    Rect e3(10, 110, 10, 0);
    RED_CHECK_EQUAL(true, e3.isempty());

    Rect e4(10, 110, 10, 1);
    RED_CHECK_EQUAL(false, e4.isempty());
    RED_CHECK_EQUAL(Rect(-10, -20, 10, 1), e4.offset(-20, -130));
    RED_CHECK_EQUAL(Rect(-10, -20, 10, 1), Rect(10, 110, 10, 1).offset(-20, -130));

    /* test if a point is inside rect */
    /* lower bounds are include ", upper bounds are excluded */

    RED_CHECK_EQUAL(true, r.contains_pt(15,115));
    RED_CHECK_EQUAL(true, r.contains_pt(19,119));
    RED_CHECK_EQUAL(true, r.contains_pt(10,110));
    RED_CHECK_EQUAL(true, r.contains_pt(10,119));

    RED_CHECK_EQUAL(false, r.contains_pt(0,100));
    RED_CHECK_EQUAL(false, r.contains_pt(0,115));
    RED_CHECK_EQUAL(false, r.contains_pt(15,100));
    RED_CHECK_EQUAL(false, r.contains_pt(15,121));
    RED_CHECK_EQUAL(false, r.contains_pt(21,115));
    RED_CHECK_EQUAL(false, r.contains_pt(20,120));
    RED_CHECK_EQUAL(false,  r.contains_pt(19,120));

    /* we can build the intersection of two rect */
    {
        Rect i1(10, 110, 30, 30);
        Rect i2(20, 120, 10, 10);
        /* here i2 is include " in i1 : then it is the intersection */
        Rect res = i1.intersect(i2);
        RED_CHECK_EQUAL(20, res.x);
        RED_CHECK_EQUAL(120, res.y);
        RED_CHECK_EQUAL(30, res.right());
        RED_CHECK_EQUAL(130, res.bottom());
    }

    {
        Rect i1(10, 110, 20, 20);
        Rect i2(20, 120, 20, 20);

        Rect res = i1.intersect(i2);
        RED_CHECK_EQUAL(20, res.x);
        RED_CHECK_EQUAL(120, res.y);
        RED_CHECK_EQUAL(30, res.right());
        RED_CHECK_EQUAL(130, res.bottom());
    }

    {
        Rect i1(10, 110, 10, 10);
        Rect i2(20, 110, 20, 10);
        /* This one is empty, it could yield any empty rect */
        /* but it returns canonical one */
        Rect res = i1.intersect(i2);
        RED_CHECK_EQUAL(true, res.isempty());
        // Is it necessary to force empty rect to be canonical ?
        RED_CHECK_EQUAL(20, res.x);
        RED_CHECK_EQUAL(110, res.y);
        RED_CHECK_EQUAL(20, res.right());
        RED_CHECK_EQUAL(110, res.bottom());
    }

    {
        Rect i1(-10, -20, 110, 120);
        Rect i2(-5, -7, 155, 157);
        /* here i2 is include " in i1 : then it is the intersection */
        Rect res = i1.intersect(i2);
        RED_CHECK_EQUAL(Rect(-5, -7, 105, 107), res);
    }


    /* we can build the union of two rect */
    {
        Rect i1(10, 110, 30, 30);
        Rect i2(20, 120, 10, 10);
        /* here i2 is include " in i1 : then it is the intersection */
        Rect res = i1.disjunct(i2);
        RED_CHECK_EQUAL(10, res.x);
        RED_CHECK_EQUAL(110, res.y);
        RED_CHECK_EQUAL(40, res.right());
        RED_CHECK_EQUAL(140, res.bottom());
    }

    {
        Rect i1(10, 110, 20, 20);
        Rect i2(20, 120, 20, 20);

        Rect res = i1.disjunct(i2);
        RED_CHECK_EQUAL(10, res.x);
        RED_CHECK_EQUAL(110, res.y);
        RED_CHECK_EQUAL(40, res.right());
        RED_CHECK_EQUAL(140, res.bottom());
    }

    {
        Rect i1(10, 110, 10, 10);
        Rect i2(10, 110, 10, 10);
        Rect res = i1.disjunct(i2);

        RED_CHECK_EQUAL(10, res.x);
        RED_CHECK_EQUAL(110, res.y);
        RED_CHECK_EQUAL(20, res.right());
        RED_CHECK_EQUAL(120, res.bottom());
    }

    {
        Rect i1(-10, -20, 110, 120);
        Rect i2(-5, -7, 155, 157);
        /* here i2 is include " in i1 : then it is the intersection */
        Rect res = i1.disjunct(i2);
        RED_CHECK_EQUAL(Rect(-10, -20, 160, 170), res);
    }


    {
        /* we can move a rect by some offset */
        Rect res(10, 110, 10, 10);
        RED_CHECK_EQUAL(10, res.x);
        RED_CHECK_EQUAL(110, res.y);
        RED_CHECK_EQUAL(20, res.right());
        RED_CHECK_EQUAL(120, res.bottom());

        res = res.offset(10, 100);

        RED_CHECK_EQUAL(20, res.x);
        RED_CHECK_EQUAL(210, res.y);
        RED_CHECK_EQUAL(30, res.right());
        RED_CHECK_EQUAL(220, res.bottom());
    }

//     {
//         /* from a rect we can get subrects of 1 pixel for each sides */
//         Rect r(10, 110, 10, 10);
//         const Rect inner = r.upper_side();
//         RED_CHECK_EQUAL(10, inner.x);
//         RED_CHECK_EQUAL(110, inner.y);
//         RED_CHECK_EQUAL(20, inner.right());
//         RED_CHECK_EQUAL(111, inner.bottom());
//     }

    {
        /* check if a rectangle contains another */
        Rect r(10, 10, 10, 10);
        Rect inner(15, 15, 3, 3);

        RED_CHECK_EQUAL(true, r.contains(inner));
        RED_CHECK_EQUAL(true, r.contains(r));

        Rect bad1(9, 10, 10, 10);
        Rect good1(11, 10, 9, 10);
        RED_CHECK_EQUAL(false, r.contains(bad1));
        RED_CHECK_EQUAL(true, r.contains(good1));

        Rect bad2(10, 9, 10, 10);
        Rect good2(10, 11, 10, 9);
        RED_CHECK_EQUAL(false, r.contains(bad2));
        RED_CHECK_EQUAL(true, r.contains(good2));

        Rect bad3(10, 10, 11, 10);
        Rect good3(10, 10, 9, 10);
        RED_CHECK_EQUAL(false, r.contains(bad3));
        RED_CHECK_EQUAL(true, r.contains(good3));

        Rect bad4(10, 10, 10, 11);
        Rect good4(10, 10, 10, 9);
        RED_CHECK_EQUAL(false, r.contains(bad4));
        RED_CHECK_EQUAL(true, r.contains(good4));

    }

    {
        /* check if two rectangles are identicals */
        Rect r(10, 10, 10, 10);

        Rect same(10, 10, 10, 10);
        Rect inner(15, 15, 3, 3);
        Rect outer(1, 1, 30, 30);
        Rect bad1(9, 10, 10, 10);
        Rect bad2(11, 10, 9, 10);
        Rect bad3(10, 9, 10, 10);
        Rect bad4(10, 11, 10, 9);
        Rect bad5(10, 10, 11, 10);
        Rect bad6(10, 10, 9, 10);
        Rect bad7(10, 10, 10, 11);
        Rect bad8(10, 10, 10, 9);

        RED_CHECK_EQUAL(r, r);
        RED_CHECK_EQUAL(r, same);
        RED_CHECK_EQUAL(same, r);
        RED_CHECK_NE(r, inner);
        RED_CHECK_NE(r, outer);
        RED_CHECK_NE(r, bad1);
        RED_CHECK_NE(r, bad1);
        RED_CHECK_NE(r, bad3);
        RED_CHECK_NE(r, bad4);
        RED_CHECK_NE(r, bad5);
        RED_CHECK_NE(r, bad6);
        RED_CHECK_NE(r, bad7);
        RED_CHECK_NE(r, bad8);

    }

    // for RDP relative sending we often need to have difference of coordinates
    // between two rectangles. That is what DeltaRect is made for
    // Notice it uses top/left/right/bottom instead of x/x/cx/cy to clearly
    // differentiate bothe types of rects.

    // 222222222222222
    // 2             2
    // 2             2
    // 2             2
    // 2             2
    // 2             2
    // 2             2
    // 2             2
    // 2             2
    // 2             2
    // 2         11112111111
    // 2         1   2     1
    // 2         1   2     1
    // 2         1   2     1
    // 222222222212222     1
    //           1         1
    //           1         1
    //           1         1
    //           1         1
    //           11111111111
    //


    Rect r1(10, 15, 11, 10);
    Rect r2(0, 0, 15, 20);
    DeltaRect dr(r1, r2);
    RED_CHECK_EQUAL(10, dr.dleft);
    RED_CHECK_EQUAL(15, dr.dtop);
    RED_CHECK_EQUAL(-4, dr.dwidth);
    RED_CHECK_EQUAL(-10, dr.dheight);
    RED_CHECK_EQUAL(true, dr.fully_relative());

    DeltaRect dr2(r2, r1);
    RED_CHECK_EQUAL(-10, dr2.dleft);
    RED_CHECK_EQUAL(-15, dr2.dtop);
    RED_CHECK_EQUAL(4, dr2.dwidth);
    RED_CHECK_EQUAL(10, dr2.dheight);
    RED_CHECK_EQUAL(true, dr2.fully_relative());

    dr2.dheight = 1024;
    RED_CHECK_EQUAL(false, dr2.fully_relative());

    {
        /* Test difference */
        Rect a(10, 10, 10, 10);
        Rect b(21, 21, 11, 11);

        a.difference(b, [](const Rect & b) {
            RED_CHECK_EQUAL(b, Rect(10, 10, 10, 10));
        });
    }

    {
        Rect a(10, 10, 50, 50);
        Rect b(20, 20, 10, 5);

        int counter = 0;
        a.difference(b, [&counter](const Rect & b) {
            switch(counter) {
                case 0:
                    RED_CHECK(b == Rect(10, 10, 50, 10));
                break;
                case 1:
                    RED_CHECK(b == Rect(10, 20, 10, 5));
                break;
                case 2:
                    RED_CHECK(b == Rect(30, 20, 30, 5));
                break;
                case 3:
                    RED_CHECK(b == Rect(10, 25, 50, 35));
                break;
                default:
                    RED_CHECK(false);
            }
            ++counter;
        });
    }

    RED_CHECK_EQUAL(Rect(10, 10, 1, 1), Rect().enlarge_to(10, 10));
    RED_CHECK_EQUAL(Rect(200, 145, 1, 1054), Rect(200, 1198, 1, 1).enlarge_to(200, 145));
    RED_CHECK_EQUAL(Rect(145, 200, 1054, 1), Rect(1198, 200, 1, 1).enlarge_to(145, 200));
    RED_CHECK_EQUAL(Rect(10, 10, 91, 91), Rect(10, 10, 1, 1).enlarge_to(100, 100));
    RED_CHECK_EQUAL(Rect(10, 10, 91, 91), Rect(100, 100, 1, 1).enlarge_to(10, 10));

    RED_CHECK_EQUAL(Rect(10, 10, 20, 45).getCenteredX(), 20);

    RED_CHECK_EQUAL(Rect(10, 10, 100, 50).getCenteredY(), 35);


    RED_CHECK_EQUAL(Rect(0, 0, 20, 45), Rect(45, 57, 20, 45).wh());

    RED_CHECK_EQUAL(Rect(20, 30, 50, 80).shrink(15), Rect(35, 45, 20, 50));

    {
        Rect r(10, 110, 10, 10);
//         RED_CHECK_EQUAL(r.upper_side(), Rect(10, 110, 10, 1));
//         RED_CHECK_EQUAL(r.left_side(), Rect(10, 110, 1, 10));
//         RED_CHECK_EQUAL(r.lower_side(), Rect(10, 119, 10, 1));
//         RED_CHECK_EQUAL(r.right_side(), Rect(19, 110, 1, 10));

        RED_CHECK_EQUAL(r.intersect(15,115), Rect(10, 110, 5, 5));
    }


    {
        // Test intersection of a rect with a line (defined by its end points)

        Rect r(100, 200, 300, 150);
        RED_CHECK_EQUAL((Rect::LEFT | Rect::UP   ), r.region_pt(50 , 50 ));
        RED_CHECK_EQUAL((Rect::LEFT | Rect::UP   ), r.region_pt(60 , 110));
        RED_CHECK_EQUAL((Rect::UP                ), r.region_pt(150, 100));
        RED_CHECK_EQUAL((Rect::UP | Rect::RIGHT  ), r.region_pt(432, 103));
        RED_CHECK_EQUAL((Rect::LEFT              ), r.region_pt(70 , 300));
        RED_CHECK_EQUAL((Rect::LEFT | Rect::DOWN ), r.region_pt(70 , 415));
        RED_CHECK_EQUAL((Rect::IN                ), r.region_pt(170, 299));
        RED_CHECK_EQUAL((Rect::RIGHT             ), r.region_pt(405, 322));
        RED_CHECK_EQUAL((Rect::DOWN              ), r.region_pt(242, 500));
        RED_CHECK_EQUAL((Rect::DOWN | Rect::RIGHT), r.region_pt(555, 477));

        // up and left border are in, right and bottom borders are out
        RED_CHECK_EQUAL((Rect::IN                ), r.region_pt(100, 200));
        RED_CHECK_EQUAL((Rect::DOWN | Rect::RIGHT), r.region_pt(400, 350));
        RED_CHECK_EQUAL((Rect::RIGHT             ), r.region_pt(400, 200));
        RED_CHECK_EQUAL((Rect::DOWN              ), r.region_pt(100, 350));

        LineEquation equa(300, 100, 50, 300);
        RED_CHECK_EQUAL(true, equa.resolve(r));
        RED_CHECK_EQUAL(equa.segin.a.x, 175);
        RED_CHECK_EQUAL(equa.segin.a.y, 200);
        RED_CHECK_EQUAL(equa.segin.b.x, 100);
        RED_CHECK_EQUAL(equa.segin.b.y, 260);

        equa = LineEquation(50, 215, 250, 215);
        RED_CHECK_EQUAL(true, equa.resolve(r));
        RED_CHECK_EQUAL(equa.segin.a.x, 100);
        RED_CHECK_EQUAL(equa.segin.a.y, 215);
        RED_CHECK_EQUAL(equa.segin.b.x, 250);
        RED_CHECK_EQUAL(equa.segin.b.y, 215);

        equa = LineEquation(50, 215, 500, 215);
        RED_CHECK_EQUAL(true, equa.resolve(r));
        RED_CHECK_EQUAL(equa.segin.a.x, 100);
        RED_CHECK_EQUAL(equa.segin.a.y, 215);
        RED_CHECK_EQUAL(equa.segin.b.x, 399);
        RED_CHECK_EQUAL(equa.segin.b.y, 215);

        equa = LineEquation(50, 400, 450, 400);
        RED_CHECK_EQUAL(false, equa.resolve(r));
        RED_CHECK_EQUAL(equa.segin.a.x, 0);
        RED_CHECK_EQUAL(equa.segin.a.y, 0);
        RED_CHECK_EQUAL(equa.segin.b.x, 0);
        RED_CHECK_EQUAL(equa.segin.b.y, 0);

        equa = LineEquation(150, 100, 450, 400);
        RED_CHECK_EQUAL(true, equa.resolve(r));
        RED_CHECK_EQUAL(equa.segin.a.x, 250);
        RED_CHECK_EQUAL(equa.segin.a.y, 200);
        RED_CHECK_EQUAL(equa.segin.b.x, 399);
        RED_CHECK_EQUAL(equa.segin.b.y, 349);

        equa = LineEquation(10, 250, 150, 50);
        RED_CHECK_EQUAL(false, equa.resolve(r));
        RED_CHECK_EQUAL(equa.segin.a.x, 0);
        RED_CHECK_EQUAL(equa.segin.a.y, 0);
        RED_CHECK_EQUAL(equa.segin.b.x, 0);
        RED_CHECK_EQUAL(equa.segin.b.y, 0);

        RED_CHECK_EQUAL(false, !4567);

    }

    {
        Rect r(10,10,10,10);
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(100,10,10,10)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(10,100,10,10)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(100,100,10,10)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(10,20,5,5)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(20,10,5,5)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(0,10,5,5)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(10,0,5,5)));
        RED_CHECK_EQUAL(false, r.has_intersection(Rect(0,0,5,5)));
        RED_CHECK_EQUAL(false, r.has_intersection(5,5));

        RED_CHECK_EQUAL(true, r.has_intersection(Rect(10,10,10,10)));
        RED_CHECK_EQUAL(true, r.has_intersection(Rect(5,10,10,10)));
        RED_CHECK_EQUAL(true, r.has_intersection(Rect(10,5,10,10)));
        RED_CHECK_EQUAL(true, r.has_intersection(Rect(15,10,10,10)));
        RED_CHECK_EQUAL(true, r.has_intersection(Rect(10,15,10,10)));
        RED_CHECK_EQUAL(true, r.has_intersection(Rect(15,15,5,5)));
        RED_CHECK_EQUAL(true, r.has_intersection(Rect(0,0,40,40)));
        RED_CHECK_EQUAL(true, r.has_intersection(15, 15));
    }
}

RED_AUTO_TEST_CASE(TestRect2)
{
    Rect r1(10, 10, 20, 20);
    Rect r2(15, 15, 0, 0);

    RED_CHECK_EQUAL(false, r1.has_intersection(r2));
    RED_CHECK_EQUAL(false, r2.has_intersection(r1));

    RED_CHECK_EQUAL(Rect(15, 15, 0, 0), r1.intersect(r2));
    RED_CHECK_EQUAL(Rect(15, 15, 0, 0), r2.intersect(r1));
}
