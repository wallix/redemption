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

   Unit test to string.hpp file
   Using lib boost functions, some tests need to be added

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestStrings
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include <string>
#include "utils/sugar/cast.hpp"
#include "utils/utf.hpp"

BOOST_AUTO_TEST_CASE(TestString)
{
    // toto is a 4 char length string
    std::string str1("toto");
    BOOST_CHECK_EQUAL(str1.length(), 4);

    // but we can get the true length
    int l = UTF8Len(byte_ptr_cast(str1.c_str()));
    BOOST_CHECK_EQUAL(l, 4);

    // olé is also a 4 char length string as it is internally UTF-8 encoded
    std::string str_unicode("olé");
    // It means length is the number of bytes
    BOOST_CHECK_EQUAL(str_unicode.length(), 4);

    // but we can get the true length
    int len = UTF8Len(byte_ptr_cast(str_unicode.c_str()));
    BOOST_CHECK_EQUAL(len, 3);

}
