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

   Unit test for Lightweight UTF library

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestLul
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/cast.hpp"
#include "utils/utf.hpp"

// RED_AUTO_TEST_CASE(TestUTF32isValid)
// {
//     RED_CHECK_EQUAL(true, UTF32isValid('a'));
//     RED_CHECK_EQUAL(false, UTF32isValid(0x1FFFFF));
//     RED_CHECK_EQUAL(false, UTF32isValid(0x00D800));
//     RED_CHECK_EQUAL(false, UTF32isValid(0x00DBFF));
//     RED_CHECK_EQUAL(false, UTF32isValid(0x00DC00));
//     RED_CHECK_EQUAL(false, UTF32isValid(0x00DFFF));
//
//     RED_CHECK_EQUAL(true, UTF32isValid(0x00FFFE));
//     RED_CHECK_EQUAL(true, UTF32isValid(0x00FFFF));
//     RED_CHECK_EQUAL(true, UTF32isValid(0x01FFFF));
//     RED_CHECK_EQUAL(true, UTF32isValid(0x10FFFF));
//     RED_CHECK_EQUAL(true, UTF32isValid(0x11000));
// }

RED_AUTO_TEST_CASE(TestUTF8Len_2)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    RED_CHECK_EQUAL(11, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8Len)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    RED_CHECK_EQUAL(2, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8LenChar)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    RED_CHECK_EQUAL(2, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestString)
{
    // toto is a 4 char length string
    std::string str1("toto");
    RED_CHECK_EQUAL(str1.length(), 4);

    // but we can get the true length
    int l = UTF8Len(byte_ptr_cast(str1.c_str()));
    RED_CHECK_EQUAL(l, 4);

    // olé is also a 4 char length string as it is internally UTF-8 encoded
    std::string str_unicode("olé");
    // It means length is the number of bytes
    RED_CHECK_EQUAL(str_unicode.length(), 4);

    // but we can get the true length
    int len = UTF8Len(byte_ptr_cast(str_unicode.c_str()));
    RED_CHECK_EQUAL(len, 3);

}

// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos)
// {
//     uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 20);
//     RED_CHECK_EQUAL(11, UTF8Len(source));
// }
//
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_0)
// {
//     uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 0);
//     RED_CHECK_EQUAL(0, source[0]);
//     RED_CHECK_EQUAL(0, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_0_v2)
// {
//     uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 0);
//     RED_CHECK_EQUAL(0, source[0]);
//     RED_CHECK_EQUAL(0, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_1)
// {
//     uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 1);
//
//     RED_CHECK_EQUAL('a', source[0]);
//     RED_CHECK_EQUAL(0, source[1]);
//     RED_CHECK_EQUAL(1, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_1_v2)
// {
//     uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 1);
//
//     RED_CHECK_EQUAL(0xC3, source[0]);
//     RED_CHECK_EQUAL(0xA9, source[1]);
//     RED_CHECK_EQUAL(0, source[2]);
//     RED_CHECK_EQUAL(1, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_2)
// {
//     uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 2);
//
//     RED_CHECK_EQUAL('a', source[0]);
//     RED_CHECK_EQUAL('b', source[1]);
//     RED_CHECK_EQUAL(0, source[2]);
//     RED_CHECK_EQUAL(2, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_2_v2)
// {
//     uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 2);
//
//     RED_CHECK_EQUAL(0xC3, source[0]);
//     RED_CHECK_EQUAL(0xA9, source[1]);
//     RED_CHECK_EQUAL('a', source[2]);
//     RED_CHECK_EQUAL(0, source[3]);
//     RED_CHECK_EQUAL(2, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_8)
// {
//     uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 8);
//
//     uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0};
//     RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
//     RED_CHECK_EQUAL(8, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_8_v2)
// {
//     uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 8);
//
//     uint8_t expected_result[] = {0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0};
//     RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
//     RED_CHECK_EQUAL(8, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_9)
// {
//     uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 9);
//
//     uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0};
//     RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
//     RED_CHECK_EQUAL(9, UTF8Len(source));
// }
//
// RED_AUTO_TEST_CASE(TestUTF8TruncateAtPos_9_v2)
// {
//     uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
//
//     UTF8TruncateAtPos(source, 9);
//
//     uint8_t expected_result[] = {0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0};
//     RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
//     RED_CHECK_EQUAL(9, UTF8Len(source));
// }

RED_AUTO_TEST_CASE(TestUTF8InsertAtPos_0)
{
    uint8_t source[255] = { 0 };
    uint8_t to_insert[] = { 0xC3, 0xA9, 0};

    UTF8InsertAtPos(source, 3, to_insert, sizeof(source));

    RED_CHECK_EQUAL(0xC3, source[0]);
    RED_CHECK_EQUAL(0xA9, source[1]);
    RED_CHECK_EQUAL(0, source[2]);

    RED_CHECK_EQUAL(1, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8InsertAtPos_beyond_end)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 20, to_insert, sizeof(source));

    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@',
                                  0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0
    };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));

    RED_CHECK_EQUAL(17, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_start)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 0, to_insert, sizeof(source));

    uint8_t expected_result[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(17, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_1)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 1, to_insert, sizeof(source));

    uint8_t expected_result[] = {'a', 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(17, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 8, to_insert, sizeof(source));

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9,
                                 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(17, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8InsertOneAtPos_at_8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8InsertOneAtPos(source, 8, 0xE9, sizeof(source));

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0,
                                 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(12, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos0)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 0);

    uint8_t expected_result[] = {'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(10, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos1)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 1);

    uint8_t expected_result[] = {'a', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(10, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos7)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 7);

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(10, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 8);

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA0, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(10, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos9)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 9);

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(10, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos10)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 10);

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(10, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos11)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 11);

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(11, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos12)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 12);

    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    RED_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    RED_CHECK_EQUAL(11, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8_UTF16)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t expected_target[] = { 'a', 0, 'b', 0, 'c', 0, 'e', 0, 'd', 0,
                                  'e', 0, 'f', 0,
                                  0xE9, 0 /* é */,
                                  0xE7, 0 /* ç */,
                                  0xE0, 0 /* à */,
                                  '@', 0, };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    uint8_t target[target_length];

    size_t nbbytes_utf16 = UTF8toUTF16(source, target, target_length);

    // Check result
    RED_CHECK_EQUAL(target_length, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(target, expected_target);

    uint8_t source_round_trip[15];

    size_t nbbytes_utf8 = UTF16toUTF8(target, nbbytes_utf16 / 2, source_round_trip, sizeof(source_round_trip));
    RED_CHECK_EQUAL(14, nbbytes_utf8);
}

RED_AUTO_TEST_CASE(TestUTF8_UTF16_witch_control_character)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0x0A, 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t expected_target[] = { 'a', 0, 'b', 0, 'c', 0, 'e', 0, 'd', 0,
                                  'e', 0, 'f', 0,
                                  0x0A, 0 /* newline */,
                                  0xE9, 0 /* é */,
                                  0xE7, 0 /* ç */,
                                  0xE0, 0 /* à */,
                                  '@', 0, };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    uint8_t target[target_length];

    size_t nbbytes_utf16 = UTF8toUTF16(source, target, target_length);

    // Check result
    RED_CHECK_EQUAL(target_length, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(target, expected_target);

    uint8_t source_round_trip[16];

    size_t nbbytes_utf8 = UTF16toUTF8(target, nbbytes_utf16 / 2, source_round_trip, sizeof(source_round_trip));
    RED_CHECK_EQUAL(15, nbbytes_utf8);
}

RED_AUTO_TEST_CASE(TestUTF8_UTF16_witch_CrLf)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0x0A, '@', 0};
    uint8_t expected_targetCr[] =   { 'a', 0, 'b', 0, 'c', 0, 'e', 0, 'd', 0,
                                      'e', 0, 'f', 0,
                                      0x0A, 0 /* newline */,
                                      '@', 0, };
    uint8_t expected_targetCrLf[] = { 'a', 0, 'b', 0, 'c', 0, 'e', 0, 'd', 0,
                                      'e', 0, 'f', 0,
                                      0x0D, 0 /* carriage return */,
                                      0x0A, 0 /* newline */,
                                      '@', 0, };
    const size_t target_lengthCr   = sizeof(expected_targetCr)/sizeof(expected_targetCr[0]);
    const size_t target_lengthCrLf = sizeof(expected_targetCrLf)/sizeof(expected_targetCrLf[0]);
    uint8_t targetCr[target_lengthCr];
    uint8_t targetCrLf[target_lengthCrLf];

    size_t nbbytes_utf16 = UTF8toUTF16(source, targetCr, target_lengthCr);

    // Check result
    RED_CHECK_EQUAL(target_lengthCr, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(targetCr, expected_targetCr);

    nbbytes_utf16 = UTF8toUTF16_CrLf(source, targetCrLf, target_lengthCrLf);

    // Check result
    RED_CHECK_EQUAL(target_lengthCrLf, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(targetCrLf, expected_targetCrLf);
}

RED_AUTO_TEST_CASE(TestUTF32toUTF8) {
    uint8_t buf[5]{};
    RED_REQUIRE_EQUAL(1, UTF32toUTF8(reinterpret_cast<uint8_t const *>("a\0\0\0"), 1, buf, 5));
    RED_REQUIRE_EQUAL('a', buf[0]);
    RED_REQUIRE_EQUAL(1, UTF32toUTF8('a', buf, 4));
    RED_REQUIRE_EQUAL('a', buf[0]);
}

//RED_AUTO_TEST_CASE(TestUTF8toUnicode)
//{
//    uint8_t source[16] = "Red";
//    uint32_t uni[16];
//
//    // Check result
//    RED_CHECK_EQUAL(3, UTF8toUnicode(source, uni, sizeof(uni)/sizeof(uni[0])));
//}

//RED_AUTO_TEST_CASE(TestUTF8Check_zero)
//{
//    uint8_t source[] = {0x00};
//    size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
//    // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)
//
//    // Check result
//    RED_CHECK_EQUAL(1, UTF8Check(source, source_length));
//}

//RED_AUTO_TEST_CASE(TestUTF8Check_control_characters)
//{
//    uint8_t source[] = {0x20, 0x00};
//    size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
//    // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)
//
//    // Check result
//    RED_CHECK_EQUAL(2, UTF8Check(source, source_length));
//}

//RED_AUTO_TEST_CASE(TestUTF8Check_continuation_at_start)
//{
//    uint8_t source[] = {0x82, 0x00};
//    size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
//    // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)
//
//   // Check result
//    RED_CHECK_EQUAL(0, UTF8Check(source, source_length));
// }

// RED_AUTO_TEST_CASE(TestUTF8Check_tilde)
// {
//     uint8_t source[] = {126};
//     size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
//     // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)
//
//     // Check result
//     RED_CHECK_EQUAL(1, UTF8Check(source, source_length));
// }

// RED_AUTO_TEST_CASE(TestUTF8Check_invalid_utf8)
// {
//     uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3 };
//     size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
//     // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)
//
//     // Check result
//     RED_CHECK_EQUAL(2, UTF8Check(source, source_length));
// }

// RED_AUTO_TEST_CASE(TestUTF8Check_witch_control_character)
// {
//     uint8_t source[] = {0xC3, 0xA9 /* é */, 0x09, 0xC3, 0xA9 };
//     size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
//     // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)
//
//     // Check result
//     RED_CHECK_EQUAL(5, UTF8Check(source, source_length));
// }

// RED_AUTO_TEST_CASE(TestUTF8Check_valid_utf8_no_trailing_zero)
// {
//     uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3, 0xA9 };
//     size_t source_length = sizeof(source);
//
//     // Check result
//     RED_CHECK_EQUAL(4, UTF8Check(source, source_length));
//
// }

// RED_AUTO_TEST_CASE(TestUTF8Check_valid_utf8_trailing_zero)
// {
//     uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3, 0xA9, 0, 'a' };
//     size_t source_length = sizeof(source);
//
//     // Check result
//     RED_CHECK_EQUAL(5, UTF8Check(source, source_length));
// }


// RED_AUTO_TEST_CASE(TestUTF8GetFirstCharLen)
// {
//     uint8_t   source[] = "aÉ€𝄞";
//     uint8_t * p        = source;
//
//     RED_CHECK_EQUAL(10, strlen(reinterpret_cast<char *>(p)));
//
//     RED_CHECK_EQUAL(1, UTF8GetFirstCharLen(p));
//     p++;
//
//     RED_CHECK_EQUAL(2, UTF8GetFirstCharLen(p));
//     p += 2;
//
//     RED_CHECK_EQUAL(3, UTF8GetFirstCharLen(p));
//     p += 3;
//
//     RED_CHECK_EQUAL(4, UTF8GetFirstCharLen(p));
// }

RED_AUTO_TEST_CASE(TestUTF8ToUTF8LCopy)
{
    uint8_t   source[11] = "aÉ€𝄞"; // 0x61, |0xc3, 0x89, |0xe2, 0x82, 0xac,| 0xf0, 0x9d, 0x84, 0x9e,| 0x00

    {
        uint8_t dest[11] = {};
        int res = UTF8ToUTF8LCopy(dest, 11, source);
        RED_CHECK_EQUAL(4, res);
    }

    {
        uint8_t dest[11] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(4, res);
        RED_CHECK_EQUAL(10, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[10] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(3, res);
        RED_CHECK_EQUAL(6, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[9] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(3, res);
        RED_CHECK_EQUAL(6, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[7] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(3, res);
        RED_CHECK_EQUAL(6, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[6] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(2, res);
        RED_CHECK_EQUAL(3, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[5] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(2, res);
        RED_CHECK_EQUAL(3, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[2] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(1, res);
        RED_CHECK_EQUAL(1, strlen(reinterpret_cast<char *>(dest)));
    }

    {
        uint8_t dest[1] = {};
        int res = UTF8ToUTF8LCopy(dest, sizeof(dest), source);
        RED_CHECK_EQUAL(0, res);
        RED_CHECK_EQUAL(0, strlen(reinterpret_cast<char *>(dest)));
    }
}

// RED_AUTO_TEST_CASE(TestUpperFunctions)
// {
//     uint8_t word[] = "Traçâbèlëté_e";
//     const size_t target_length = 13 * 2;
//     uint8_t target[target_length];
//     size_t nbbytes_utf16 = UTF8toUTF16(word, target, target_length);
//     RED_CHECK_EQUAL(nbbytes_utf16, target_length);
//     hexdump_c(target, target_length);
//
//     uint8_t uptarget[target_length];
//     memcpy(uptarget, target, target_length);
//     UTF16Upper(uptarget, target_length / 2);
//     hexdump_c(uptarget, target_length);
//
//     uint8_t uptargetw[target_length];
//     memcpy(uptargetw, target, target_length);
//     UTF16UpperW(uptargetw, target_length / 2);
//     hexdump_c(uptargetw, target_length);
// }

RED_AUTO_TEST_CASE(TestUTF8toUnicodeIterator) {
    const char * s = "Ëa\nŒo";
    UTF8toUnicodeIterator u(s);
    RED_CHECK(*u);            ++u;
    RED_CHECK_EQUAL(*u, 'a'); ++u;
    RED_CHECK_EQUAL(*u, '\n');++u;
    RED_CHECK(*u);            ++u;
    RED_CHECK_EQUAL(*u, 'o'); ++u;
    RED_CHECK_EQUAL(*u, 0);
}

RED_AUTO_TEST_CASE(TestIsUtf8String) {
    RED_CHECK(is_utf8_string(byte_ptr_cast("abcd"), -1));

    RED_CHECK(is_utf8_string(byte_ptr_cast("Téléphone"), -1));

    RED_CHECK(is_utf8_string(byte_ptr_cast("T\xc3\x9al\xc3\x9aphone"), -1));

    RED_CHECK(!is_utf8_string(byte_ptr_cast("T\xc3\x9al\xc3""aphone"), -1));

    RED_CHECK(is_utf8_string(byte_ptr_cast("T\xc3\x9al\xc3""aphone"), 4));

    RED_CHECK(!is_utf8_string(byte_ptr_cast("T\xe9l\xe9phone"), -1));

    RED_CHECK(is_utf8_string(byte_ptr_cast("Téléphone"), 10));
}

RED_AUTO_TEST_CASE(TestUTF16ToLatin1) {
    const uint8_t utf16_src[] = "\x74\x00\x72\x00\x61\x00\x70\x00"  // "trapézoïdal"
                                "\xe9\x00\x7a\x00\x6f\x00\xef\x00"
                                "\x64\x00\x61\x00\x6c\x00\x00\x00";

    const size_t number_of_characters = sizeof(utf16_src) / 2;

    uint8_t latin1_dst[32];

    RED_CHECK_EQUAL(
        UTF16toLatin1(utf16_src, number_of_characters * 2, latin1_dst, sizeof(latin1_dst)),
        number_of_characters);

    RED_CHECK_EQUAL(std::string(char_ptr_cast(latin1_dst)), "trap\xe9zo\xef" "dal");
}

RED_AUTO_TEST_CASE(TestUTF16ToLatin1_1) {
    const uint8_t utf16_src[] = "\x31\x00\x30\x00\x30\x00\x20\x00"  // "100 €"
                                "\xac\x20\x00\x00";

    const size_t number_of_characters = sizeof(utf16_src) / 2;

    uint8_t latin1_dst[32] = {};

    auto x = UTF16toLatin1(utf16_src, number_of_characters * 2, latin1_dst, sizeof(latin1_dst));

    RED_CHECK_EQUAL(x, number_of_characters);

    RED_CHECK_EQUAL(std::string(char_ptr_cast(latin1_dst)), "100 \x80");
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF16) {
    const uint8_t latin1_src[] = "trap\xe9zo\xef" "dal";

    const size_t number_of_characters = strlen(char_ptr_cast(latin1_src)) + 1;

    const uint8_t utf16_expected[] = "\x74\x00\x72\x00\x61\x00\x70\x00"  // "trapézoïdal"
                                     "\xe9\x00\x7a\x00\x6f\x00\xef\x00"
                                     "\x64\x00\x61\x00\x6c\x00\x00\x00";

    uint8_t utf16_dst[32];

    RED_CHECK_EQUAL(
        Latin1toUTF16(latin1_src, number_of_characters, utf16_dst, sizeof(utf16_dst)),
        number_of_characters * 2);

    RED_CHECK_EQUAL(memcmp(utf16_dst, utf16_expected, number_of_characters * 2), 0);

    uint8_t utf8_dst[16];

    RED_CHECK_EQUAL(
        UTF16toUTF8(utf16_dst, number_of_characters * 2, utf8_dst, sizeof(utf8_dst)),
        number_of_characters + 2 /* 'é' => 0xC3 0xA9, 'ï' => 0xC3 0xAF */);

    RED_CHECK_EQUAL(std::string(char_ptr_cast(utf8_dst)), "trapézoïdal");
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF16_1) {
    const uint8_t latin1_src[] = "100 \x80";

    const size_t number_of_characters = strlen(char_ptr_cast(latin1_src)) + 1;

    const uint8_t utf16_expected[] = "\x31\x00\x30\x00\x30\x00\x20\x00"  // "100 €"
                                     "\xac\x20\x00\x00";

    uint8_t utf16_dst[32];

    RED_CHECK_EQUAL(
        Latin1toUTF16(latin1_src, number_of_characters, utf16_dst, sizeof(utf16_dst)),
        number_of_characters * 2);

    RED_CHECK_EQUAL(memcmp(utf16_dst, utf16_expected, number_of_characters * 2), 0);

    uint8_t utf8_dst[16];

    RED_CHECK_EQUAL(
        UTF16toUTF8(utf16_dst, number_of_characters * 2, utf8_dst, sizeof(utf8_dst)),
        number_of_characters + 2 /* '€' => 0xE2 0x82 0xAC */ );

    RED_CHECK_EQUAL(std::string(char_ptr_cast(utf8_dst)), "100 €");
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF16_2) {
    const uint8_t latin1_src[] = "100 \x80"
                                 "\x0a"
                                 "trap\xe9zo\xef" "dal";

    const size_t number_of_characters = strlen(char_ptr_cast(latin1_src)) + 1;

    const uint8_t utf16_expected[] = "\x31\x00\x30\x00\x30\x00\x20\x00"  // "100 €"
                                     "\xac\x20"
                                     "\x0d\x00\x0a\x00"                  // "\r\n"
                                     "\x74\x00\x72\x00\x61\x00\x70\x00"  // "trapézoïdal"
                                     "\xe9\x00\x7a\x00\x6f\x00\xef\x00"
                                     "\x64\x00\x61\x00\x6c\x00\x00\x00";

    uint8_t utf16_dst[64];

    RED_CHECK_EQUAL(
        Latin1toUTF16(latin1_src, number_of_characters, utf16_dst, sizeof(utf16_dst)),
        number_of_characters * 2 + 2 /* '\n' -> 0x0D 0x00 0x0A 0x00 */);

    RED_CHECK_EQUAL(memcmp(utf16_dst, utf16_expected,
                             number_of_characters * 2 + 2   // '\n' -> 0x0D 0x00 0x0A 0x00
                            ), 0);

    uint8_t utf8_dst[32];

    RED_CHECK_EQUAL(
        UTF16toUTF8(utf16_dst, number_of_characters * 2, utf8_dst, sizeof(utf8_dst)),
        number_of_characters + 2 /* '€'  => 0xE2 0x82 0xAC */
                             + 1 /* '\n' => 0x0D 0x0A */
                             + 2 /* 'é'  => 0xC3 0xA9, 'ï' => 0xC3 0xAF */);

    RED_CHECK_EQUAL(std::string(char_ptr_cast(utf8_dst)), "100 €\r\ntrapézoïdal");
}

RED_AUTO_TEST_CASE(TestUTF8StringAdjustedNbBytes) {
    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast(""), 6), 0);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 6), 4);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 3), 2);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 1), 0);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 0), 0);
}

RED_AUTO_TEST_CASE(TestUTF16StrLen) {
    RED_CHECK_EQUAL(UTF16StrLen(byte_ptr_cast("\x00\x00")), 0);

    RED_CHECK_EQUAL(UTF16StrLen(byte_ptr_cast("\x31\x00\x30\x00\x30\x00\x00\x00")), 3);

    RED_CHECK_EQUAL(UTF16StrLen(byte_ptr_cast("\x31\x00\x30\x00\x30\x00\x00\x00\x31\x00\x30\x00\x30\x00")), 3);
}
