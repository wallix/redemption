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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPattern
#include "system/redemption_unit_tests.hpp"


//#define LOGNULL
#define LOGPRINT
#include "utils/log.hpp"
#include "utils/winpr/pattern.hpp"

BOOST_AUTO_TEST_CASE(TestFilePatternFindNextWildcardA)
{
    {
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA("toto.exe", &pFlags);
        BOOST_CHECK(reinterpret_cast<const void*>(posWildcard) == nullptr);
        BOOST_CHECK(pFlags == 0);
    }

    {
        const char * input = "toto*.exe";
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA(input, &pFlags);
        BOOST_CHECK(posWildcard - input == 4);
        BOOST_CHECK(memcmp(input, "toto", 4) == 0);
        BOOST_CHECK(pFlags == WILDCARD_STAR);
    }

    {
        const char * input = "toto*?.exe";
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA(input, &pFlags);
        BOOST_CHECK(posWildcard - input == 4);
        BOOST_CHECK(memcmp(input, "toto", 4) == 0);
        BOOST_CHECK(pFlags == WILDCARD_STAR);
    }
    {
        const char * input = "toto?.exe";
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA(input, &pFlags);
        BOOST_CHECK(posWildcard - input == 4);
        BOOST_CHECK(memcmp(input, "toto", 4) == 0);
        BOOST_CHECK(pFlags == WILDCARD_QM);
    }

    {
        const char * input = "toto~*.exe";
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA(input, &pFlags);
        BOOST_CHECK(posWildcard - input == 4);
        BOOST_CHECK(memcmp(input, "toto", 4) == 0);
        BOOST_CHECK(pFlags == WILDCARD_DOS_STAR);
    }

    {
        const char * input = "toto~?.exe";
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA(input, &pFlags);
        BOOST_CHECK(posWildcard - input == 4);
        BOOST_CHECK(memcmp(input, "toto", 4) == 0);
        BOOST_CHECK(pFlags == WILDCARD_DOS_QM);
    }
    {
        const char * input = "toto~.exe";
        unsigned int pFlags = 0;
        const char * posWildcard = FilePatternFindNextWildcardA(input, &pFlags);
        BOOST_CHECK(posWildcard - input == 4);
        BOOST_CHECK(memcmp(input, "toto", 4) == 0);
        BOOST_CHECK(pFlags == WILDCARD_DOS_DOT);
    }
}


BOOST_AUTO_TEST_CASE(TestFilePatternMatchA)
{
    {
        bool res = FilePatternMatchA("toto.exe", "*.exe");
        BOOST_CHECK(res);
    }
    {
        bool res = FilePatternMatchA("toto.exe", "*.com");
        BOOST_CHECK(res == false);
    }
}
