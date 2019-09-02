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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan,
              Jennifer Inthavong
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for Lightweight UTF library

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/utf.hpp"
#include "utils/sugar/cast.hpp"
#include <string_view>
#include <cstring>

using namespace std::string_view_literals;


RED_AUTO_TEST_CASE(TestUTF8Len_2)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    RED_CHECK_EQUAL(11u, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8Len)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    RED_CHECK_EQUAL(2u, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF16ByteLen)
{
    RED_CHECK_EQUAL(0u, UTF16ByteLen("\0\0"_av));
    RED_CHECK_EQUAL(0u, UTF16ByteLen("\0"_av));
    RED_CHECK_EQUAL(6u, UTF16ByteLen("a\0b\0c\0\0\0"_av));
    RED_CHECK_EQUAL(6u, UTF16ByteLen("a\0b\0c\0\0\0\0"_av));
    RED_CHECK_EQUAL(8u, UTF16ByteLen("a\0\xe9\0\xe7\0\x0e\0"_av));
}

RED_AUTO_TEST_CASE(TestUTF16ToUTF8_buf)
{
    uint8_t source[24];
    auto buf = make_array_view(source);
    RED_CHECK_SMEM(""_av, UTF16toUTF8_buf("\0\0"_av, buf));
    RED_CHECK_SMEM(""_av, UTF16toUTF8_buf("\0"_av, buf));
    RED_CHECK_SMEM("abc"_av, UTF16toUTF8_buf("a\0b\0c\0\0\0"_av, buf));
    RED_CHECK_SMEM("abc"_av, UTF16toUTF8_buf("a\0b\0c\0\0\0\0"_av, buf));
    RED_CHECK_SMEM("a\xc3\xa9\xc3\xa7\xc3\xa0"_av, UTF16toUTF8_buf("a\0\xe9\0\xe7\0\xe0\0"_av, buf));
}

RED_AUTO_TEST_CASE(TestUTF8LenChar)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    RED_CHECK_EQUAL(2u, UTF8Len(source));
}

RED_AUTO_TEST_CASE(TestUTF8InsertUtf16)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f'};
    RED_CHECK(UTF8InsertUtf16(make_array_view(source), 8, 'x'));
    RED_CHECK_SMEM(make_array_view(source).first(9), "xabcedef\x00"_av);
    RED_CHECK(!UTF8InsertUtf16(make_array_view(source).first(9), 9, 'y'));
    RED_CHECK_SMEM(make_array_view(source).first(9), "xabcedef\x00"_av);
}

struct Data
{
    std::size_t pos;
    array_view_const_char expected;
};

RED_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos0)
{
    RED_TEST_CONTEXT_DATA(Data const& data, "insert at position " << data.pos, {
        Data{0,  "bcedef\xC3\xA0\xC3\xA7\xC3\xA0@\0"_av},
        Data{1,  "acedef\xC3\xA0\xC3\xA7\xC3\xA0@\0"_av},
        Data{7,  "abcedef\xC3\xA7\xC3\xA0@\0"_av},
        Data{8,  "abcedef\xC3\xA0\xC3\xA0@\0"_av},
        Data{9,  "abcedef\xC3\xA0\xC3\xA7@\0"_av},
        Data{10, "abcedef\xC3\xA0\xC3\xA7\xC3\xA0\0"_av},
        Data{11, "abcedef\xC3\xA0\xC3\xA7\xC3\xA0@\0"_av},
        Data{12, "abcedef\xC3\xA0\xC3\xA7\xC3\xA0@\0"_av}
    }) {
        uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

        UTF8RemoveOneAtPos(source, data.pos);

        RED_CHECK_SMEM(array_view(source, data.expected.size()), data.expected);
    }
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

    size_t nbbytes_utf16 = UTF8toUTF16({std::data(source), std::size(source)-1}, target, target_length);

    // Check result
    RED_CHECK_EQUAL(target_length, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(target, expected_target);

    uint8_t source_round_trip[15];

    size_t nbbytes_utf8 = UTF16toUTF8(target, nbbytes_utf16 / 2, source_round_trip, sizeof(source_round_trip));
    RED_CHECK_EQUAL(14u, nbbytes_utf8);
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

    size_t nbbytes_utf16 = UTF8toUTF16({std::data(source), std::size(source)-1}, target, target_length);

    // Check result
    RED_CHECK_EQUAL(target_length, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(target, expected_target);

    uint8_t source_round_trip[16];

    size_t nbbytes_utf8 = UTF16toUTF8(target, nbbytes_utf16 / 2, source_round_trip, sizeof(source_round_trip));
    RED_CHECK_EQUAL(15u, nbbytes_utf8);
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

    size_t nbbytes_utf16 = UTF8toUTF16({std::data(source), std::size(source)-1}, targetCr, target_lengthCr);

    // Check result
    RED_CHECK_EQUAL(target_lengthCr, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(targetCr, expected_targetCr);

    nbbytes_utf16 = UTF8toUTF16_CrLf({std::data(source), std::size(source)-1}, targetCrLf, target_lengthCrLf);

    // Check result
    RED_CHECK_EQUAL(target_lengthCrLf, nbbytes_utf16);
    RED_CHECK_EQUAL_RANGES(targetCrLf, expected_targetCrLf);
}

RED_AUTO_TEST_CASE(TestUTF32toUTF8) {
    uint8_t buf[5]{};
    RED_REQUIRE_EQUAL(1u, UTF32toUTF8(byte_ptr_cast("a\0\0\0"), 1, buf, 5));
    RED_REQUIRE_EQUAL('a', buf[0]);
    RED_REQUIRE_EQUAL(1u, UTF32toUTF8('a', buf, 4));
    RED_REQUIRE_EQUAL('a', buf[0]);
}

RED_AUTO_TEST_CASE(TestUTF8toUnicodeIterator) {
    const char * s = "Ëa\nŒo";
    UTF8toUnicodeIterator u(s);
    RED_CHECK(*u);            ++u;
    RED_CHECK_EQUAL(*u, uint8_t('a')); ++u;
    RED_CHECK_EQUAL(*u, uint8_t('\n'));++u;
    RED_CHECK(*u);            ++u;
    RED_CHECK_EQUAL(*u, uint8_t('o')); ++u;
    RED_CHECK_EQUAL(*u, uint8_t(0));
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

    RED_CHECK(char_ptr_cast(latin1_dst) == "trap\xe9zo\xef" "dal"sv);
}

RED_AUTO_TEST_CASE(TestUTF16ToLatin1_1) {
    const uint8_t utf16_src[] = "\x31\x00\x30\x00\x30\x00\x20\x00"  // "100 €"
                                "\xac\x20\x00\x00";

    const size_t number_of_characters = sizeof(utf16_src) / 2;

    uint8_t latin1_dst[32] = {};

    auto x = UTF16toLatin1(utf16_src, number_of_characters * 2, latin1_dst, sizeof(latin1_dst));

    RED_CHECK_EQUAL(x, number_of_characters);

    RED_CHECK(char_ptr_cast(latin1_dst) == "100 \x80"sv);
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF16)
{
    const uint8_t latin1_src[] = "trap\xe9zo\xef" "dal";
    const size_t number_of_characters = sizeof(latin1_src) - 1;
    const size_t utf16_chars_count = number_of_characters * 2;

    // "trapézoïdal"
    const auto utf16_expected = "\x74\x00\x72\x00\x61\x00\x70\x00"
                                "\xe9\x00\x7a\x00\x6f\x00\xef\x00"
                                "\x64\x00\x61\x00\x6c\x00\x00\x00"_av;

    uint8_t utf16_dst[32];

    RED_CHECK_EQUAL(
        Latin1toUTF16({latin1_src, number_of_characters+1}, utf16_dst, sizeof(utf16_dst)),
        utf16_chars_count + 2);

    RED_CHECK_SMEM(make_array_view(utf16_dst, utf16_chars_count + 2), utf16_expected);

    uint8_t utf8_dst[16]{};

    RED_CHECK_EQUAL(
        UTF16toUTF8(utf16_dst, utf16_chars_count + 2, utf8_dst, sizeof(utf8_dst)),
        number_of_characters + 1 + 2 /* 'é' => 0xC3 0xA9, 'ï' => 0xC3 0xAF */);

    RED_CHECK(char_ptr_cast(utf8_dst) == "trapézoïdal"sv);
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF16_1) {
    auto latin1_src = "100 \x80\0"_av;

    auto utf16_expected = "\x31\x00\x30\x00\x30\x00\x20\x00"  // "100 €"
                          "\xac\x20\x00\x00"_av;

    uint8_t utf16_dst[32];

    RED_CHECK_MEM(array_view(utf16_dst, Latin1toUTF16(latin1_src, utf16_dst, sizeof(utf16_dst))),
        utf16_expected);

    uint8_t utf8_dst[16];

    RED_CHECK_MEM(array_view(utf8_dst, UTF16toUTF8(utf16_dst, latin1_src.size() * 2, utf8_dst, sizeof(utf8_dst))),
        "100 €\0"_av); /* '€' => 0xE2 0x82 0xAC */
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF16_2) {
    auto latin1_src = "100 \x80"
                      "\x0a"
                      "trap\xe9zo\xef" "dal\0"_av;

    auto utf16_expected = "\x31\x00\x30\x00\x30\x00\x20\x00"  // "100 €"
                          "\xac\x20"
                          "\x0d\x00\x0a\x00"                  // "\r\n"
                          "\x74\x00\x72\x00\x61\x00\x70\x00"  // "trapézoïdal"
                          "\xe9\x00\x7a\x00\x6f\x00\xef\x00"
                          "\x64\x00\x61\x00\x6c\x00\x00\x00"_av;

    uint8_t utf16_dst[64];

    RED_CHECK_MEM(array_view(utf16_dst, Latin1toUTF16(latin1_src, utf16_dst, sizeof(utf16_dst))),
        utf16_expected);

    uint8_t utf8_dst[32];

    RED_CHECK_MEM(array_view(utf8_dst, UTF16toUTF8(utf16_dst, latin1_src.size() * 2, utf8_dst, sizeof(utf8_dst))),
        "100 €\r\ntrapézoïdal\0"_av);
}

RED_AUTO_TEST_CASE(TestLatin1ToUTF8) {
    auto latin1_src = "100 \x80"                 // "100 €"
                      "\n"                       // \n
                      "trap\xe9zo\xef" "dal"_av; // "trapézoïdal"

    auto utf8_expected = "100 \xc2\x80"
                         "\x0a"
                         "trap\xc3\xa9zo\xc3\xaf" "dal"_av;

    uint8_t utf8_dst[64];

    RED_CHECK_MEM(
        array_view(utf8_dst, Latin1toUTF8(byte_ptr_cast(latin1_src.data()), latin1_src.size(), utf8_dst, sizeof(utf8_dst))),
        utf8_expected);
}

RED_AUTO_TEST_CASE(TestUTF8StringAdjustedNbBytes) {
    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast(""), 6), 0u);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 6), 4u);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 3), 2u);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 1), 0u);

    RED_CHECK_EQUAL(UTF8StringAdjustedNbBytes(byte_ptr_cast("èè"), 0), 0u);
}

RED_AUTO_TEST_CASE(TestUTF16StrLen) {
    RED_CHECK_EQUAL(UTF16StrLen(byte_ptr_cast("\x00\x00")), 0u);

    RED_CHECK_EQUAL(UTF16StrLen(byte_ptr_cast("\x31\x00\x30\x00\x30\x00\x00\x00")), 3u);

    RED_CHECK_EQUAL(UTF16StrLen(byte_ptr_cast("\x31\x00\x30\x00\x30\x00\x00\x00\x31\x00\x30\x00\x30\x00")), 3u);
}

RED_AUTO_TEST_CASE(TestUtf16UpperCase)
{
    uint8_t test[] =  "\x92\x02\xaf\x03\xdb\x03\x51\x04"  /* ʒ ί ϛ ё */
                      "\x45\x04\x64\x05\x11\x1e\x01\x1f"  /* х դ ḑ ἁ */
                      "\x41\x1f\x45\xff\x72\x00\x54\x00"  /* ὁ ｅ r T */
                      "\x3e\x02\xcc\x1f\x5a\xff";         /* ⱦ ῃ ｚ  */
    /*
    Lower    Upper case
    0x0292 ; 0x01B7     # LATIN CAPITAL LETTER EZH
    0x03AF ; 0x038A     # GREEK CAPITAL LETTER IOTA WITH TONOS
    0x03DB ; 0x03DA     # GREEK LETTER STIGMA
    0x0451 ; 0x0401     # CYRILLIC CAPITAL LETTER IO

    0x0445 ; 0x0425     # CYRILLIC CAPITAL LETTER HA
    0x0564 ; 0x0534     # ARMENIAN CAPITAL LETTER DA
    0x1E11 ; 0x1E10     # LATIN CAPITAL LETTER D WITH CEDILLA
    0x1F01 ; 0x1F09     # GREEK CAPITAL LETTER ALPHA WITH DASIA

    0x1F41 ; 0x1F49     # GREEK CAPITAL LETTER OMICRON WITH DASIA
    0xFF45 ; 0xFF25     # FULLWIDTH LATIN CAPITAL LETTER E
    0x0072 ; 0x0052     # LATIN CAPITAL LETTER R
    0x0074 ; 0x0054     # LATIN CAPITAL LETTER T

    0x023E ; 0x2C66     # LATIN SMALL LETTER T WITH DIAGONAL STROKE
    0x1FCC ; 0x1FC3     # GREEK SMALL LETTER ETA WITH PROSGEGRAMMENI
    0xFF5A ; 0xFF3A     # FULLWIDTH LATIN CAPITAL LETTER Z
    */
    int number_of_elements = sizeof(test)/sizeof(test[0])-2;
    auto expected = "\xb7\x01\x8A\x03\xda\x03\x01\x04"  /* Ʒ Ί Ϛ Ё */
                    "\x25\x04\x34\x05\x10\x1e\x09\x1f"  /* Х Դ Ḑ Ἁ */
                    "\x49\x1f\x25\xff\x52\x00\x54\x00"  /* Ὁ Ｅ R T */
                    "\x66\x2c\xc3\x1f\x3a\xff\x00"      /* Ⱦ ΗΙ Ｚ  */
                    ""_av;
    UTF16Upper(test, number_of_elements);

    RED_CHECK_MEM(make_array_view(test), expected);
}


RED_AUTO_TEST_CASE(TestUTF16ToUTF8)
{
    {
        uint8_t u16_1[]{'a', 0, 'b', 0, 'c', 0, 0, 0};
        uint8_t dest[32]{'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};

        RED_CHECK(UTF16toUTF8(u16_1, 4, dest, sizeof(dest)) == 4u);
        RED_CHECK(char_ptr_cast(dest) == "abc"sv);
    }
    {
        uint16_t u16_2[]{'a', 'b', 'c', 0};
        uint8_t dest[32]{'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};

        RED_CHECK(UTF16toUTF8(u16_2, 4, dest, sizeof(dest)) == 4u);
        RED_CHECK(char_ptr_cast(dest) == "abc"sv);
    }
}

RED_AUTO_TEST_CASE(TestUTF8ToUTF16Limit)
{
    uint8_t expected_target[]{ 'a', 0, 'b', 0, 'c', 0, 'd', 0 };

    const size_t target_length = 8;
    uint8_t target[target_length];

    memset(target, 0xfe, sizeof(target));

    size_t nbbytes_utf16 = UTF8toUTF16("abcdef"_av, target, target_length);

    // Check result
    RED_CHECK_EQUAL(target_length, nbbytes_utf16);      // 8
    RED_CHECK_EQUAL_RANGES(target, expected_target);    // "\x61\x00\x62\x00\x63\x00\x64\x00"
}

RED_AUTO_TEST_CASE(TestUTF8ToUTF16)
{
    uint8_t expected_target[]{ 'a', 0, 'b', 0, 'c', 0, 0xfe, 0xfe };

    const size_t target_length = 8;
    uint8_t target[target_length];

    memset(target, 0xfe, sizeof(target));

    size_t nbbytes_utf16 = UTF8toUTF16("abc"_av, target, target_length);

    // Check result
    RED_CHECK_EQUAL(6u, nbbytes_utf16);      // 6
    RED_CHECK_EQUAL_RANGES(target, expected_target);    // "\x61\x00\x62\x00\x63\x00\x64\x00"
}

RED_AUTO_TEST_CASE(TestUTF8StrLenInChar)
{
    RED_CHECK_EQUAL(4u, UTF8StrLenInChar(byte_ptr_cast("abcd")));

    RED_CHECK_EQUAL(0u, UTF8StrLenInChar(byte_ptr_cast("")));

    RED_CHECK_EQUAL(4u, UTF8StrLenInChar("éric"));
    RED_CHECK_EQUAL(4u, UTF8StrLenInChar("€uro"));
}

RED_AUTO_TEST_CASE(Testis_ASCII_string)
{
    RED_CHECK(is_ASCII_string(byte_ptr_cast("abcd")));

    RED_CHECK(!is_ASCII_string(byte_ptr_cast("éric")));

    RED_CHECK(is_ASCII_string(byte_ptr_cast("")));
}
