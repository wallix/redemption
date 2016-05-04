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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean
*/

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#define BOOST_AUTO_TEST_CASE(name) void name(); \
struct TEST_ ## name {\
    TEST_ ## name() {\
        TESTS.add({STRINGIFY(name), &name});\
    }\
} TEST_ ## name;\
\
void name()



#define BOOST_CHECK_EQUAL(x, y) if ((x)!=(y)) {\
std::cout << "Test Failed at " << __LINE__ << "\n";\
std::cout << "Got (" << (x) << ") expected (" << (y) << ") \n";\
}\


#define BOOST_CHECK(x) if (!(x)) { printf("Test Failed at %d\n", __LINE__); }
#define BOOST_REQUIRE_EQUAL(x, y) if ((x)!=(y)) { printf("Test Failed at %d\n", __LINE__); }
#define BOOST_REQUIRE_NE(x, y) if ((x)==(y)) { printf("Test Failed at %d\n", __LINE__); }

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

