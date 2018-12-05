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
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of Verifier module
*/

#define RED_TEST_MODULE TestVerifier
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/ocr/classification.hh"

RED_AUTO_TEST_CASE(TestLatinToCyrillic)
{
    ocr::locale::latin_to_cyrillic_context ctx;
    std::string s = "aБВ9ГДEжзийo aБВ9ГДEжзийo Блокнот internet aБВ9ГДEжзийo";
    char const * result = "аБВ9ГДЕжзийо аБВ9ГДЕжзийо Блокнот internet аБВ9ГДЕжзийо";

    RED_CHECK_NE(s, result);
    ctx.latin_to_cyrillic(s);
    RED_CHECK_EQUAL(s, result);
    ctx.latin_to_cyrillic(s);
    RED_CHECK_EQUAL(s, result);

    s = "aбc";
    result = "aбc";

    RED_CHECK_EQUAL(s, result);
    ctx.latin_to_cyrillic(s);
    RED_CHECK_EQUAL(s, result);

    s = "a?Б?ББaa";
    result = "a?Б?ББaa";

    RED_CHECK_EQUAL(s, result);
    ctx.latin_to_cyrillic(s);
    RED_CHECK_EQUAL(s, result);

    s = "Уcтpoйcтвa и пpинтepы";
    result = "Устройства и принтеры";

    RED_CHECK_NE(s, result);
    ctx.latin_to_cyrillic(s);
    RED_CHECK_EQUAL(s, result);
}
