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

   Unit test to test back and forth conversion of aligned to compact bitmaps

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestALignedToCompact
#include <boost/test/auto_unit_test.hpp>

#include "altoco.hpp"
#include <sstream>
#include <iostream>
#include <string>
#include <string.h>
#include <memory.h>


BOOST_AUTO_TEST_CASE(TestCompactToAlignedSizeComputing)
{
    BOOST_CHECK_EQUAL(480000, aligned_size(800, 600, 8));
    BOOST_CHECK_EQUAL(960000, aligned_size(800, 600, 16));
    BOOST_CHECK_EQUAL(1920000, aligned_size(800, 600, 24));

    BOOST_CHECK_EQUAL(480000, aligned_size(799, 600, 8));
    BOOST_CHECK_EQUAL(960000, aligned_size(799, 600, 16));
    BOOST_CHECK_EQUAL(1920000, aligned_size(799, 600, 24));

    BOOST_CHECK_EQUAL(480000, aligned_size(798, 600, 8));
    BOOST_CHECK_EQUAL(960000, aligned_size(798, 600, 16));
    BOOST_CHECK_EQUAL(1920000, aligned_size(798, 600, 24));

    BOOST_CHECK_EQUAL(480000, aligned_size(797, 600, 8));
    BOOST_CHECK_EQUAL(960000, aligned_size(797, 600, 16));
    BOOST_CHECK_EQUAL(1920000, aligned_size(797, 600, 24));

    BOOST_CHECK_EQUAL(477600, aligned_size(796, 600, 8));
    BOOST_CHECK_EQUAL(955200, aligned_size(796, 600, 16));
    BOOST_CHECK_EQUAL(1910400, aligned_size(796, 600, 24));

    BOOST_CHECK_EQUAL(800, row_size(800, 8));
    BOOST_CHECK_EQUAL(1600, row_size(800, 16));
    BOOST_CHECK_EQUAL(2400, row_size(800, 24));
    BOOST_CHECK_EQUAL(3200, row_size(800, 32));

    BOOST_CHECK_EQUAL(800, row_size(799, 8));
    BOOST_CHECK_EQUAL(1600, row_size(799, 16));
    BOOST_CHECK_EQUAL(2400, row_size(799, 24));
    BOOST_CHECK_EQUAL(3196, row_size(799, 32));

    BOOST_CHECK_EQUAL(800, row_size(798, 8));
    BOOST_CHECK_EQUAL(1596, row_size(798, 16));
    BOOST_CHECK_EQUAL(2396, row_size(798, 24));
    BOOST_CHECK_EQUAL(3192, row_size(798, 32));

    BOOST_CHECK_EQUAL(800, row_size(797, 8));
    BOOST_CHECK_EQUAL(1596, row_size(797, 16));
    BOOST_CHECK_EQUAL(2392, row_size(797, 24));
    BOOST_CHECK_EQUAL(3188, row_size(797, 32));

    BOOST_CHECK_EQUAL(796, row_size(796, 8));
    BOOST_CHECK_EQUAL(1592, row_size(796, 16));
    BOOST_CHECK_EQUAL(2388, row_size(796, 24));
    BOOST_CHECK_EQUAL(3184, row_size(796, 32));
}

BOOST_AUTO_TEST_CASE(TestCompactToAligned8bpp)
{
    BOOST_CHECK_EQUAL(36, row_size(10, 8) * 3);

    uint8_t compact[36] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 0, 0,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 0,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0, 0
    };

    BOOST_CHECK_EQUAL(36, aligned_size(10, 3, 8));
    uint8_t aligned[36];

    uint8_t expected_aligned[36] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 0, 0,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 0,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0, 0
    };

    compact_to_aligned(10, 3, 8, compact, aligned);

    BOOST_CHECK_EQUAL(0, memcmp(aligned, expected_aligned, aligned_size(10, 3, 8)));
}

BOOST_AUTO_TEST_CASE(TestCompactToAligned16bpp)
{
    BOOST_CHECK_EQUAL(36, row_size(5, 16) * 3);

    uint8_t compact[36] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 0, 0,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 0,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0, 0
    };

    BOOST_CHECK_EQUAL(48, aligned_size(5, 3, 16));
    uint8_t aligned[48];

    uint8_t expected_aligned[48] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 0, 0, 0, 0, 0, 0,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, 0, 0, 0, 0, 0,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0, 0, 0, 0, 0, 0
    };

    compact_to_aligned(5, 3, 16, compact, aligned);

    BOOST_CHECK_EQUAL(0, memcmp(aligned, expected_aligned, aligned_size(5, 3, 16)));
}


BOOST_AUTO_TEST_CASE(TestCompactToAligned24bpp)
{
    BOOST_CHECK_EQUAL(36, row_size(4, 24) * 3);

    uint8_t compact[36] = {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    };

    BOOST_CHECK_EQUAL(48, aligned_size(4, 3, 24));
    uint8_t aligned[36];

    uint8_t expected_aligned[48] = {
         0,  1,  2,  0, 3,  4,  5,  0, 6,  7,  8,  0, 9, 10, 11, 0,
         0,  1,  2,  0, 3,  4,  5,  0, 6,  7,  8,  0, 9, 10, 11, 0,
         0,  1,  2,  0, 3,  4,  5,  0, 6,  7,  8,  0, 9, 10, 11, 0,
    };

    compact_to_aligned(4, 3, 24, compact, aligned);

    BOOST_CHECK_EQUAL(0, memcmp(aligned, expected_aligned, aligned_size(4, 3, 24)));
}

//BOOST_AUTO_TEST_CASE(TestAlignedToCompact24bpp)
//{
//    BOOST_CHECK_EQUAL(48, aligned_size(4, 3, 24));
//    const uint8_t aligned[48] = {
//         0,  1,  2,  0, 3,  4,  5,  0, 6,  7,  8,  0, 9, 10, 11, 0,
//         0,  1,  2,  0, 3,  4,  5,  0, 6,  7,  8,  0, 9, 10, 11, 0,
//         0,  1,  2,  0, 3,  4,  5,  0, 6,  7,  8,  0, 9, 10, 11, 0,
//    };

//    BOOST_CHECK_EQUAL(36, row_size(4, 24) * 3);
//    uint8_t compact[36];


//    uint8_t expected_compact[36] = {
//         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
//         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
//         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
//    };

//    aligned_to_compact(4, 3, 24, aligned, compact);

//    BOOST_CHECK_EQUAL(0, compact[0]);
//    BOOST_CHECK_EQUAL(1, compact[1]);
//    BOOST_CHECK_EQUAL(2, compact[2]);
//    BOOST_CHECK_EQUAL(3, compact[3]);

//    BOOST_CHECK_EQUAL(0, memcmp(compact, expected_compact, row_size(4, 24) * 3));
//}
