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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/module_manager/acl_new_line_converter.hpp"

RED_AUTO_TEST_CASE(TestAclMessageConvert)
{
    RED_TEST(AclNewLineConverter(""_zv).zstring() == ""_av_ascii);
    RED_TEST(AclNewLineConverter("a"_zv).zstring() == "a"_av_ascii);
    RED_TEST(AclNewLineConverter("abc"_zv).zstring() == "abc"_av_ascii);
    RED_TEST(AclNewLineConverter("abcd"_zv).zstring() == "abcd"_av_ascii);
    RED_TEST(AclNewLineConverter("abcdefghij"_zv).zstring() == "abcdefghij"_av_ascii);

    RED_TEST(AclNewLineConverter("<br>"_zv).zstring() == "\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br><br>"_zv).zstring() == "\n\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br><br><br>"_zv).zstring() == "\n\n\n"_av_ascii);

    RED_TEST(AclNewLineConverter("<br>a"_zv).zstring() == "\na"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abc"_zv).zstring() == "\nabc"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abcd"_zv).zstring() == "\nabcd"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abcdefghij"_zv).zstring() == "\nabcdefghij"_av_ascii);

    RED_TEST(AclNewLineConverter("a<br>"_zv).zstring() == "a\n"_av_ascii);
    RED_TEST(AclNewLineConverter("abc<br>"_zv).zstring() == "abc\n"_av_ascii);
    RED_TEST(AclNewLineConverter("abcd<br>"_zv).zstring() == "abcd\n"_av_ascii);
    RED_TEST(AclNewLineConverter("abcdefghij<br>"_zv).zstring() == "abcdefghij\n"_av_ascii);

    RED_TEST(AclNewLineConverter("<br>a<br>"_zv).zstring() == "\na\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abc<br>"_zv).zstring() == "\nabc\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abcd<br>"_zv).zstring() == "\nabcd\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abcdefghij<br>"_zv).zstring() == "\nabcdefghij\n"_av_ascii);

    RED_TEST(AclNewLineConverter("<br>a<br><br>a<br>"_zv).zstring() == "\na\n\na\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abc<br><br>abc<br>"_zv)
        .zstring() == "\nabc\n\nabc\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abcd<br><br>abcd<br>"_zv)
        .zstring() == "\nabcd\n\nabcd\n"_av_ascii);
    RED_TEST(AclNewLineConverter("<br>abcdefghij<br><br>abcdefghij<br>"_zv)
        .zstring() == "\nabcdefghij\n\nabcdefghij\n"_av_ascii);

    RED_TEST(AclNewLineConverter("x<br>a<br><br>a<br>x"_zv)
        .zstring() == "x\na\n\na\nx"_av_ascii);
    RED_TEST(AclNewLineConverter("x<br>abc<br><br>abc<br>x"_zv)
        .zstring() == "x\nabc\n\nabc\nx"_av_ascii);
    RED_TEST(AclNewLineConverter("x<br>abcd<br><br>abcd<br>x"_zv)
        .zstring() == "x\nabcd\n\nabcd\nx"_av_ascii);
    RED_TEST(AclNewLineConverter("x<br>abcdefghij<br><br>abcdefghij<br>x"_zv)
        .zstring() == "x\nabcdefghij\n\nabcdefghij\nx"_av_ascii);

    RED_TEST(AclNewLineConverter("<br>a<br>b<br>cd<br>ef<br>ghi<br>jab<br>cdef<br>ghij<br>"_zv)
        .zstring() == "\na\nb\ncd\nef\nghi\njab\ncdef\nghij\n"_av_ascii);
    RED_TEST(AclNewLineConverter("a<br>b<br>cdefghijab<br>cdef<br>ghij"_zv)
        .zstring() == "a\nb\ncdefghijab\ncdef\nghij"_av_ascii);
}
