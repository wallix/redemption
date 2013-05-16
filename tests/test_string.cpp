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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Raphael Zhou
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to string object
   Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestString
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "string.hpp"

BOOST_AUTO_TEST_CASE(TestString)
{
    redemption::string str;

    BOOST_CHECK(!strcmp("", str));


    const char * template_string = "0123456789";

    str = template_string;

    BOOST_CHECK(!strcmp(template_string, str));


    str = 0;

    BOOST_CHECK(!strcmp("", str));


    str += "0123456789";

    BOOST_CHECK(!strcmp("0123456789", str));


    str += "ABCDEF";

    BOOST_CHECK(!strcmp("0123456789ABCDEF", str));


    str += 0;

    BOOST_CHECK(!strcmp("0123456789ABCDEF", str));


    str = "";

    BOOST_CHECK(!strcmp("", str));


    template_string = "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"    // 1000 bytes
                      "01234567890123456789012345678901234567890123456789"
                      "01234567890123456789012345678901234567890123456789"    // 1100 bytes
                      ;

    str = template_string;

    BOOST_CHECK(!strcmp(template_string, str));
}

BOOST_AUTO_TEST_CASE(TestString1)
{
    const char * template_string = "0123456789";

    redemption::string str(template_string);

    BOOST_CHECK(!strcmp(template_string, str));
}