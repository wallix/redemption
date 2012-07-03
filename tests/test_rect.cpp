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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to rect object

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRect
#include <boost/test/auto_unit_test.hpp>

#include "rect.hpp"


BOOST_AUTO_TEST_CASE(TestRect)
{

    // A rect is defined by it's top left corner, width and height
    Rect r(10, 110, 10, 10);

    BOOST_CHECK_EQUAL(10, r.x);
    BOOST_CHECK_EQUAL(110, r.y);
    BOOST_CHECK_EQUAL(20, r.right());
    BOOST_CHECK_EQUAL(120, r.bottom());

    /* we can also create an empty rect, it's the default constructor */
    Rect empty;

    BOOST_CHECK_EQUAL(0, empty.x);
    BOOST_CHECK_EQUAL(0, empty.y);
    BOOST_CHECK_EQUAL(0, empty.right());
    BOOST_CHECK_EQUAL(0, empty.bottom());

    /* test if rect is empty */
    BOOST_CHECK_EQUAL(true, empty.isempty());
    BOOST_CHECK_EQUAL(false, r.isempty());

    /* this one is empty because left is after right */
    Rect e1(20, 110, -10, 10);
    BOOST_CHECK_EQUAL(true, e1.isempty());


    Rect e2(10, 110, 0, 0);
    BOOST_CHECK_EQUAL(true, e2.isempty());

    Rect e3(10, 110, 10, 0);
    BOOST_CHECK_EQUAL(true, e3.isempty());

    Rect e4(10, 110, 10, 1);
    BOOST_CHECK_EQUAL(false, e4.isempty());
    BOOST_CHECK_EQUAL(Rect(-10, -20, 10, 1), e4.offset(-20, -130));
    BOOST_CHECK_EQUAL(Rect(-10, -20, 10, 1), Rect(10, 110, 10, 1).offset(-20, -130));

    /* test if a point is inside rect */
    /* lower bounds are included, upper bounds are excluded */

    BOOST_CHECK_EQUAL(true, r.contains_pt(15,115));
    BOOST_CHECK_EQUAL(true, r.contains_pt(19,119));
    BOOST_CHECK_EQUAL(true, r.contains_pt(10,110));
    BOOST_CHECK_EQUAL(true, r.contains_pt(10,119));

    BOOST_CHECK_EQUAL(false, r.contains_pt(0,100));
    BOOST_CHECK_EQUAL(false, r.contains_pt(0,115));
    BOOST_CHECK_EQUAL(false, r.contains_pt(15,100));
    BOOST_CHECK_EQUAL(false, r.contains_pt(15,121));
    BOOST_CHECK_EQUAL(false, r.contains_pt(21,115));
    BOOST_CHECK_EQUAL(false, r.contains_pt(20,120));
    BOOST_CHECK_EQUAL(false,  r.contains_pt(19,120));

    /* we can build the intersection of two rect */
    {
        Rect i1(10, 110, 30, 30);
        Rect i2(20, 120, 10, 10);
        /* here i2 is included in i1 : then it is the intersection */
        Rect res = i1.intersect(i2);
        BOOST_CHECK_EQUAL(20, res.x);
        BOOST_CHECK_EQUAL(120, res.y);
        BOOST_CHECK_EQUAL(30, res.right());
        BOOST_CHECK_EQUAL(130, res.bottom());
    }

    {
        Rect i1(10, 110, 20, 20);
        Rect i2(20, 120, 20, 20);

        Rect res = i1.intersect(i2);
        BOOST_CHECK_EQUAL(20, res.x);
        BOOST_CHECK_EQUAL(120, res.y);
        BOOST_CHECK_EQUAL(30, res.right());
        BOOST_CHECK_EQUAL(130, res.bottom());
    }

    {
        Rect i1(10, 110, 10, 10);
        Rect i2(20, 110, 20, 10);
        /* This one is empty, it could yield any empty rect */
        /* but it returns canonical one */
        Rect res = i1.intersect(i2);
        BOOST_CHECK_EQUAL(true, res.isempty());
        // Is it necessary to force empty rect to be canonical ?
        BOOST_CHECK_EQUAL(0, res.x);
        BOOST_CHECK_EQUAL(0, res.y);
        BOOST_CHECK_EQUAL(0, res.right());
        BOOST_CHECK_EQUAL(0, res.bottom());
    }

    {
        Rect i1(-10, -20, 110, 120);
        Rect i2(-5, -7, 155, 157);
        /* here i2 is included in i1 : then it is the intersection */
        Rect res = i1.intersect(i2);
        BOOST_CHECK_EQUAL(Rect(-5, -7, 105, 107), res);
    }


    {
        /* we can move a rect by some offset */
        Rect res(10, 110, 10, 10);
        BOOST_CHECK_EQUAL(10, res.x);
        BOOST_CHECK_EQUAL(110, res.y);
        BOOST_CHECK_EQUAL(20, res.right());
        BOOST_CHECK_EQUAL(120, res.bottom());

        res = res.offset(10, 100);

        BOOST_CHECK_EQUAL(20, res.x);
        BOOST_CHECK_EQUAL(210, res.y);
        BOOST_CHECK_EQUAL(30, res.right());
        BOOST_CHECK_EQUAL(220, res.bottom());
    }

    {
        /* from a rect we can get subrects of 1 pixel for each sides */
        Rect r(10, 110, 10, 10);
        const Rect & inner = r.upper_side();
        BOOST_CHECK_EQUAL(10, inner.x);
        BOOST_CHECK_EQUAL(110, inner.y);
        BOOST_CHECK_EQUAL(20, inner.right());
        BOOST_CHECK_EQUAL(111, inner.bottom());
    }

    {
        /* check if a rectangle contains another */
        Rect r(10, 10, 10, 10);
        Rect inner(15, 15, 3, 3);

        BOOST_CHECK_EQUAL(true, r.contains(inner));
        BOOST_CHECK_EQUAL(true, r.contains(r));

        Rect bad1(9, 10, 10, 10);
        Rect good1(11, 10, 9, 10);
        BOOST_CHECK_EQUAL(false, r.contains(bad1));
        BOOST_CHECK_EQUAL(true, r.contains(good1));

        Rect bad2(10, 9, 10, 10);
        Rect good2(10, 11, 10, 9);
        BOOST_CHECK_EQUAL(false, r.contains(bad2));
        BOOST_CHECK_EQUAL(true, r.contains(good2));

        Rect bad3(10, 10, 11, 10);
        Rect good3(10, 10, 9, 10);
        BOOST_CHECK_EQUAL(false, r.contains(bad3));
        BOOST_CHECK_EQUAL(true, r.contains(good3));

        Rect bad4(10, 10, 10, 11);
        Rect good4(10, 10, 10, 9);
        BOOST_CHECK_EQUAL(false, r.contains(bad4));
        BOOST_CHECK_EQUAL(true, r.contains(good4));

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

        BOOST_CHECK_EQUAL(true, r.equal(r));
        BOOST_CHECK_EQUAL(true, r.equal(same));
        BOOST_CHECK_EQUAL(true, same.equal(r));
        BOOST_CHECK_EQUAL(false, r.equal(inner));
        BOOST_CHECK_EQUAL(false, r.equal(outer));
        BOOST_CHECK_EQUAL(false, r.equal(bad1));
        BOOST_CHECK_EQUAL(false, r.equal(bad1));
        BOOST_CHECK_EQUAL(false, r.equal(bad3));
        BOOST_CHECK_EQUAL(false, r.equal(bad4));
        BOOST_CHECK_EQUAL(false, r.equal(bad5));
        BOOST_CHECK_EQUAL(false, r.equal(bad6));
        BOOST_CHECK_EQUAL(false, r.equal(bad7));
        BOOST_CHECK_EQUAL(false, r.equal(bad8));

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
    BOOST_CHECK_EQUAL(10, dr.dleft);
    BOOST_CHECK_EQUAL(15, dr.dtop);
    BOOST_CHECK_EQUAL(-4, dr.dwidth);
    BOOST_CHECK_EQUAL(-10, dr.dheight);
    BOOST_CHECK_EQUAL(true, dr.fully_relative());

    DeltaRect dr2(r2, r1);
    BOOST_CHECK_EQUAL(-10, dr2.dleft);
    BOOST_CHECK_EQUAL(-15, dr2.dtop);
    BOOST_CHECK_EQUAL(4, dr2.dwidth);
    BOOST_CHECK_EQUAL(10, dr2.dheight);
    BOOST_CHECK_EQUAL(true, dr2.fully_relative());

    dr2.dheight = 1024;
    BOOST_CHECK_EQUAL(false, dr2.fully_relative());

    {
        /* Test difference */
        Rect a(10, 10, 10, 10);
        Rect b(21, 21, 11, 11);

        struct RectI1 : public Rect::RectIterator {
            void callback(const Rect & b) {
                BOOST_CHECK_EQUAL(b, Rect(10, 10, 10, 10));
            }
        };

        RectI1 it;
        a.difference(b, it);
    }

    {
        Rect a(10, 10, 50, 50);
        Rect b(20, 20, 10, 5);

        struct RectI1 : public Rect::RectIterator {
            int counter;

            RectI1() : counter(0) {}

            void callback(const Rect & b) {
                switch(counter) {
                    case 0:
                        BOOST_CHECK(b == Rect(10, 10, 50, 10));
                    break;
                    case 1:
                        BOOST_CHECK(b == Rect(10, 20, 10, 5));
                    break;
                    case 2:
                        BOOST_CHECK(b == Rect(30, 20, 30, 5));
                    break;
                    case 3:
                        BOOST_CHECK(b == Rect(10, 25, 50, 35));
                    break;
                    default:
                        BOOST_CHECK(false);
                }
                this->counter++;
            }
        };

        RectI1 it;
        a.difference(b, it);
    }

    BOOST_CHECK_EQUAL(Rect(10, 10, 1, 1), Rect().enlarge_to(10, 10));
    BOOST_CHECK_EQUAL(Rect(200, 145, 1, 1054), Rect(200, 1198, 1, 1).enlarge_to(200, 145));
    BOOST_CHECK_EQUAL(Rect(145, 200, 1054, 1), Rect(1198, 200, 1, 1).enlarge_to(145, 200));
    BOOST_CHECK_EQUAL(Rect(10, 10, 91, 91), Rect(10, 10, 1, 1).enlarge_to(100, 100));
    BOOST_CHECK_EQUAL(Rect(10, 10, 91, 91), Rect(100, 100, 1, 1).enlarge_to(10, 10));

}
