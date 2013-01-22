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

   Unit test for Lightweight UTF library

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestLul
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "utf.hpp"
//#include <unicode/ustring.h>
#include <stdio.h>


BOOST_AUTO_TEST_CASE(TestUTF8Len)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 20);
    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_0)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 0);
    BOOST_CHECK_EQUAL(0, source[0]);
    BOOST_CHECK_EQUAL(0, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_0_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 0);
    BOOST_CHECK_EQUAL(0, source[0]);
    BOOST_CHECK_EQUAL(0, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_1)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 1);
    
    BOOST_CHECK_EQUAL('a', source[0]);
    BOOST_CHECK_EQUAL(0, source[1]);
    BOOST_CHECK_EQUAL(1, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_1_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 1);
    
    BOOST_CHECK_EQUAL(0xC3, source[0]);
    BOOST_CHECK_EQUAL(0xA9, source[1]);
    BOOST_CHECK_EQUAL(0, source[2]);
    BOOST_CHECK_EQUAL(1, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_2)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 2);
    
    BOOST_CHECK_EQUAL('a', source[0]);
    BOOST_CHECK_EQUAL('b', source[1]);
    BOOST_CHECK_EQUAL(0, source[2]);
    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_2_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 2);
    
    BOOST_CHECK_EQUAL(0xC3, source[0]);
    BOOST_CHECK_EQUAL(0xA9, source[1]);
    BOOST_CHECK_EQUAL('a', source[2]);
    BOOST_CHECK_EQUAL(0, source[3]);
    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_8)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 8);
    
    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(8, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_8_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 8);
    
    uint8_t expected_result[] = {0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(8, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_9)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 9);
    
    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(9, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_9_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 9);
    
    uint8_t expected_result[] = {0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(9, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_0)
{
    uint8_t source[255] = { 0 };
    uint8_t to_insert[] = { 0xC3, 0xA9, 0};

    UTF8InsertAtPos(source, 3, to_insert, sizeof(source));
    uint8_t expected_result[] = { 0xC3, 0xA9, 0};

    BOOST_CHECK_EQUAL(0xC3, source[0]);
    BOOST_CHECK_EQUAL(0xA9, source[1]);
    BOOST_CHECK_EQUAL(0, source[2]);

    BOOST_CHECK_EQUAL(1, UTF8Len(source));
}


BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_beyond_end)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 20, to_insert, sizeof(source));
    
    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 
                                  0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));

    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_start)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 0, to_insert, sizeof(source));
    
    uint8_t expected_result[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_1)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 1, to_insert, sizeof(source));
    
    uint8_t expected_result[] = {'a', 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 8, to_insert, sizeof(source));
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9,
                                 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8_UTF16)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    size_t source_length = sizeof(source);
    uint8_t expected_target[] = { 'a', 0, 'b', 0, 'c', 0, 'e', 0, 'd', 0,
                                  'e', 0, 'f', 0, 
                                  0xE9, 0 /* é */,
                                  0xE7, 0 /* ç */,
                                  0xE0, 0 /* à */,
                                  '@', 0, 0, 0 };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    uint8_t target[target_length];

    const uint8_t * psource = source;
    uint8_t * ptarget = target;
    UTF8toUTF16(&psource, source_length, &ptarget, target_length);

    // Check result
    BOOST_CHECK_EQUAL(target_length, ptarget-target);
    for (size_t q = 0 ; q < target_length ; q++){
        if (expected_target[q] != target[q]){
            printf("at %u: expected %u, got %u\n", (unsigned)q, expected_target[q] ,target[q]);
            BOOST_CHECK(false);
        }
    }

    uint8_t source_round_trip[15];
    const uint8_t * psource2 = expected_target;
    uint8_t * ptarget2 = source_round_trip;

    UTF16toUTF8(&psource2, target_length, &ptarget2, source_length);

    // Check round trip result
    BOOST_CHECK_EQUAL(source_length, ptarget2-source_round_trip);
    for (size_t q = 0 ; q < source_length ; q++){
        if (source_round_trip[q] != source[q]){
            printf("at %u: expected %x, got %x\n", (unsigned)q, source[q], source_round_trip[q]);
            BOOST_CHECK(false);
        }
    }
}


BOOST_AUTO_TEST_CASE(TestUTF8_UTF16_invalid_utf8)
{
    uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3 };
    size_t source_length = sizeof(source);
    uint8_t expected_target[] = { 0xE9, 0 /* é */ };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    uint8_t target[target_length];

    const uint8_t * psource = source;
    uint8_t * ptarget = target;
    UTF8toUTF16(&psource, source_length, &ptarget, target_length);

    // Check result
    BOOST_CHECK_EQUAL(target_length, ptarget - target);
    BOOST_CHECK_EQUAL(2, psource - source);
    for (size_t q = 0 ; q < target_length ; q++){
        if (expected_target[q] != target[q]){
            printf("at %u: expected %u, got %u\n", (unsigned)q, expected_target[q], target[q]);
            BOOST_CHECK(false);
        }
    }

    uint8_t source_round_trip[15];
    const uint8_t * psource2 = expected_target;
    uint8_t * ptarget2 = source_round_trip;

    UTF16toUTF8(&psource2, target_length, &ptarget2, source_length);

    // Check round trip result
    BOOST_CHECK_EQUAL(source_length - 1, ptarget2 - source_round_trip);
    for (size_t q = 0 ; q < source_length - 1 ; q++){
        if (source_round_trip[q] != source[q]){
            printf("at %u: expected %x, got %x\n", (unsigned)q, source[q], source_round_trip[q]);
            BOOST_CHECK(false);
        }
    }

}

