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
#define BOOST_TEST_MODULE TestUtf
//#include <boost/test/auto_unit_test.hpp>

//#define LOGNULL
#define LOGPRINT

#include <stdio.h>

#include "utils/cast.hpp"
#include "utils/utf.hpp"

#include <vector>

#define BOOST_AUTO_TEST_CASE(name) void name(); \
struct TEST_ ## name {\
    TEST_ ## name() {\
        TESTS.add({STRINGIFY(name), &name});\
    }\
} TEST_ ## name;\
\
void name()



#define BOOST_CHECK_EQUAL(x, y) if ((x)!=(y)) { printf("Test Failed at %d ", __LINE__); }

#ifdef BOOST_AUTO_TEST_MAIN
struct TESTS {
    struct item {
        const char * name;
        void (*fn)();
    };
    std::vector<item> tests;
    void add(item item){
        this->tests.push_back(item);
    }
} TESTS;

#define STRINGIFY(x) #x 
#define MODULE_NAME(x) printf("Running test suite " STRINGIFY(x) "\n");

int main(){
    MODULE_NAME(BOOST_TEST_MODULE);
    for (auto& item: TESTS.tests){
        printf("Running test %s\n", item.name);
        item.fn();
    }
}    
#endif

BOOST_AUTO_TEST_CASE(TestUTF8Len_2)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8Len)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8LenChar)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}


