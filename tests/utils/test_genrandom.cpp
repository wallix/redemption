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

   Unit test to region object
   Using lib boost functions, some tests need to be added

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGenRandom
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/genrandom.hpp"

RED_AUTO_TEST_CASE(TestGenRandom)
{
    {
        UdevRandom rnd;
        unsigned mem[128] = {};
        rnd.random(mem, sizeof(mem));
        unsigned count_null = 0;
        for (size_t i = 0; i < 128 ; i++){
            if (!mem[i]) { count_null++; }
        }
        // well, theoratically as we are testing a random generator,
        // this test may possibly fail and even generator yield as many zeroes
        // as we (dont) want.
        // however this should be quite rare.
        // so, if the test fails, I guess we should just run it again
        RED_CHECK(count_null == 0);

        unsigned mem2[1024] = {};
        rnd.random(mem2, sizeof(mem2));

        RED_CHECK(memcmp(mem, mem2, sizeof(mem)) != 0);
    }

    {
        LCGRandom rnd(0);
        unsigned mem[128] = {};
        rnd.random(mem, sizeof(mem));
        unsigned count_null = 0;
        for (size_t i = 0; i < 128 ; i++){
            if (!mem[i]) { count_null++; }
        }
        // well, theoratically as we are testing a random generator,
        // this test may possibly fail and even generator yield as many zeroes
        // as we (dont) want.
        // however this should be quite rare.
        // so, if the test fails, I guess we should just run it again
        RED_CHECK(count_null == 0);

        unsigned mem2[1024] = {};
        rnd.random(mem2, sizeof(mem2));

        RED_CHECK(memcmp(mem, mem2, sizeof(mem)) != 0);
        uint8_t mem3[1024] = {};
        rnd.random(mem3, sizeof(mem3));
        hexdump_c(mem3, sizeof(mem3));

    }
}
