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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to ModContext.parse_username(string)
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestParseUsername
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include <stdint.h>

#include "config.hpp"

// Test the parsing of a string with two different separator characters : '@' and ':'
// each separator can appear one or two time, with a different behaviour for each case

// full prototype of a string : <U>@<D>:<P>:<A>
//    with :
//      - <U> = user that may contain an @
//      -  @  = first separator
//      - <D> = device
//      -  :  = second separator
//      - <P> = protocol that can be omitted
//      -  :  = third separator (absent if <P> is omitted)
//      - <A> = auth user

BOOST_AUTO_TEST_CASE(TestParseUsername1)
{
    Inifile ini;

    // TEST 1 (simplest case - three separators)
    char username_1[256] = "u@d:p:a";
    ini.parse_username(username_1);

    BOOST_CHECK_EQUAL("u", ini.globals.target_user);
    BOOST_CHECK_EQUAL("d", ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("p", ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("a", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername2)
{
    Inifile ini;

    // TEST 2 (three separators with a '@' include " in User)
    char username_2[256] = "u@u1@d:p:a";
    ini.parse_username(username_2);

    BOOST_CHECK_EQUAL("u@u1", ini.globals.target_user);
    BOOST_CHECK_EQUAL("d",    ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("p",    ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("a",    ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername3)
{
    Inifile ini;

    // TEST 3 (only two separators with a '@' in User)
    char username_3[256] = "u@u1@d:a";
    ini.parse_username(username_3);

    BOOST_CHECK_EQUAL("u@u1", ini.globals.target_user);
    BOOST_CHECK_EQUAL("d",    ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",  ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("a",    ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername4)
{
    Inifile ini;

    // TEST 4 (only two separators)
    char username_4[256] = "u@d:a";
    ini.parse_username(username_4);

    BOOST_CHECK_EQUAL("u",    ini.globals.target_user);
    BOOST_CHECK_EQUAL("d",    ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",  ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("a",    ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername5)
{
    Inifile ini;

    // TEST 5 (real case)
    char username_5[256] = "administrateur@qa@devwin1:RDP_1:user12";
    ini.parse_username(username_5);

    BOOST_CHECK_EQUAL("administrateur@qa", ini.globals.target_user);
    BOOST_CHECK_EQUAL("devwin1",           ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP_1",             ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("user12",            ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername6)
{
    Inifile ini;

    // TEST 6 (3 separators with an @ in wabuser)
    char username_6[256] = "administrateur@qa@devwin1:RDP_1:administrateur@qa";
    ini.parse_username(username_6);

    BOOST_CHECK_EQUAL("administrateur@qa", ini.globals.target_user);
    BOOST_CHECK_EQUAL("devwin1",           ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP_1",             ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("administrateur@qa", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername7)
{
    Inifile ini;

    // TEST 7 (no : separator)
    char username[] = "admin";
    ini.parse_username(username);

    BOOST_CHECK_EQUAL("",      ini.globals.target_user);
    BOOST_CHECK_EQUAL("",      ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",   ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("admin", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername8)
{
    Inifile ini;

    // TEST 8 (no : separator, one arobas)
    char username[] = "admin@qa";
    ini.parse_username(username);

    BOOST_CHECK_EQUAL("",         ini.globals.target_user);
    BOOST_CHECK_EQUAL("",         ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",      ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("admin@qa", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername9)
{
    Inifile ini;

    // TEST 9 (no : separator, two arobas)
    char username[] = "admin@qa@win";
    ini.parse_username(username);

    BOOST_CHECK_EQUAL("",             ini.globals.target_user);
    BOOST_CHECK_EQUAL("",             ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",          ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("admin@qa@win", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername10)
{
    Inifile ini;

    // TEST 10, one ':' separator, no arobas => implicit target login
    char username[] = "win:admin";
    ini.parse_username(username);

    BOOST_CHECK_EQUAL("admin", ini.globals.target_user);
    BOOST_CHECK_EQUAL("win",   ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",   ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("admin", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)

BOOST_AUTO_TEST_CASE(TestParseUsername11)
{
    Inifile ini;

    // TEST 10, one ':' separator, no arobas => implicit target login
    char username[] = "win:admin@qa";
    ini.parse_username(username);

    BOOST_CHECK_EQUAL("admin@qa", ini.globals.target_user);
    BOOST_CHECK_EQUAL("win",      ini.globals.target_device.get().c_str());
    BOOST_CHECK_EQUAL("RDP",      ini.context.target_protocol.c_str());
    BOOST_CHECK_EQUAL("admin@qa", ini.globals.auth_user);
} // END FUNCTION - BOOST_AUTO_TEST_CASE(TestParseUsername)
