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
#include <unicode/ustring.h>
#include <stdio.h>

//BOOST_AUTO_TEST_CASE(TestUTF8_UTF32)
//{
//    uint8_t source[] = "abcedeféçà@";
//    int32_t i = 0;
//    size_t length = sizeof(source);
//    uint32_t c = 0;
//    uint32_t expected_target[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xE9 /* é */, 0xE7 /* ç */, 0xE0 /* à */, '@', 0 };
//    size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
//    uint32_t target[target_length];


//    uint32_t t_count = 0;
//    while (i < length){    
//        U8_NEXT(source, i, length, c);
//        target[t_count++] = c;
//    }

//    // Check UTF32 result (array of code points)
//    BOOST_CHECK_EQUAL(target_length, t_count);
//    for (size_t q = 0 ; q < target_length ; q++){
//        if (expected_target[q] != target[q]){
//            printf("at %u: expected %u, got %u\n", q, expected_target[q] ,target[q]);
//            BOOST_CHECK(false);
//        }
//    }
//}

BOOST_AUTO_TEST_CASE(TestUTF8_UTF32)
{
    char source[] = "abcedeféçà@";
    size_t source_length = sizeof(source);
    UChar expected_target[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xE9 /* é */, 0xE7 /* ç */, 0xE0 /* à */, '@', 0 };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    UChar target[target_length];

    int32_t nb = 0;
    UErrorCode error = U_ZERO_ERROR;
    u_strFromUTF8(target, target_length, &nb, source, source_length, &error);

    // Check UTF32 result (array of code points)
    BOOST_CHECK_EQUAL(target_length, nb);
    for (size_t q = 0 ; q < target_length ; q++){
        if (expected_target[q] != target[q]){
            printf("at %u: expected %u, got %u\n", q, expected_target[q] ,target[q]);
            BOOST_CHECK(false);
        }
    }
}


BOOST_AUTO_TEST_CASE(TestUTF8_UTF16)
{
    uint8_t source[] = "abcedeféçà@";
    size_t source_length = sizeof(source);
    uint16_t expected_target[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xE9 /* é */, 0xE7 /* ç */, 0xE0 /* à */, '@', 0 };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    uint16_t target[target_length];

    int32_t nb = 0;
    
    uint8_t * psource = source;
    uint16_t * ptarget = target;
    UTF8toUTF16(&psource, source_length, &ptarget, target_length);

    // Check UTF32 result (array of code points)
    BOOST_CHECK_EQUAL(target_length, ptarget-target);
    for (size_t q = 0 ; q < target_length ; q++){
        if (expected_target[q] != target[q]){
            printf("at %u: expected %u, got %u\n", q, expected_target[q] ,target[q]);
            BOOST_CHECK(false);
        }
    }
}

