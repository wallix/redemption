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

   Unit test to region object
   Using lib boost functions, some tests need to be added

*/

#define RED_TEST_MODULE TestSubRegion
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/region.hpp"


RED_AUTO_TEST_CASE(TestSubRegion)
{
    /* create a region */

    Rect r1(10, 110, 10, 10);
    SubRegion region;
    region.add_rect(r1);
    region.add_rect(r1);
    region.add_rect(r1);

    int sum_left = 0;
    for (Rect const & rect : region.rects){
        sum_left += rect.x;
    }
    RED_CHECK_EQUAL(30, sum_left);

    /* A region is basically a zone defined by adding or substracting rects */
    // if we subtract a rectangle inside region, we get 4 smaller rectangle around it

    //   x----------------x
    //   x                x
    //   x     x-----x    x
    //   x     x     x    x
    //   x     x-----x    x
    //   x                x
    //   x                x
    //   x----------------x
    //
    SubRegion region2;
    region2.add_rect(Rect(10,10,90,90));
    RED_CHECK_EQUAL(1u, region2.rects.size());

    // (10,10)
    //   x----------------x
    //   x        A       x A= (10, 10, 100, 30) Rect(10, 10, 90, 20)
    //   x-----x-----x----x
    //   x  B  x     x C  x B= (10, 30, 30, 50)  Rect(10, 30, 20, 20)
    //   x-----x-----x----x C= (50, 30, 100, 50) Rect(50, 30, 50, 20)
    //   x                x
    //   x       D        x D= (10, 50, 100, 100) Rect(10, 50, 90, 50);
    //   x----------------x
    //                  (100, 100)
    region2.subtract_rect(Rect(30, 30, 20, 20));
    RED_CHECK_EQUAL(4u, region2.rects.size());

    RED_CHECK_EQUAL(region2.rects[0], Rect(10, 10, 90, 20)); // A
    RED_CHECK_EQUAL(region2.rects[1], Rect(10, 30, 20, 20)); // B
    RED_CHECK_EQUAL(region2.rects[2], Rect(50, 30, 50, 20)); // C
    RED_CHECK_EQUAL(region2.rects[3], Rect(10, 50, 90, 50)); // D

    // we substract a traversing rectangle
    SubRegion region3;
    region3.add_rect(Rect(10,10,90,90));
    RED_CHECK_EQUAL(1u, region3.rects.size());


    //         x-----x
    //         x     x
    //   x-----x-----x----x
    //   x     x     x    x
    //   x     x     x    x
    //   x  A  x     x  B x
    //   x     x     x    x
    //   x     x     x    x
    //   x     x     x    x
    //   x-----x-----x----x
    //         x     x
    //         x-----x

    region3.subtract_rect(Rect(30, 5, 20, 150));
    RED_CHECK_EQUAL(2u, region3.rects.size());

    RED_CHECK_EQUAL(region3.rects[0], Rect(10, 10, 20, 90)); // A
    RED_CHECK_EQUAL(region3.rects[1], Rect(50, 10, 50, 90)); // B
}
