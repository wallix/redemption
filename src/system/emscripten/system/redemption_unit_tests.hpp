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

#pragma once

#include <cstdio>
#include <iostream>
#include <vector>
#include <cstring>

#define BOOST_AUTO_TEST_CASE(name) void name(); \
struct TEST_ ## name {\
    TEST_ ## name() {\
        TESTS.add({STRINGIFY(name), &name});\
    }\
} TEST_ ## name;\
\
void name()

template<class T, class U>
bool cmp(T const & x, U const & y) {
	return x != y;
}

inline bool cmp(char const * s1, char const * s2) {
	return strcmp(s1, s2) != 0;
}

inline bool cmp(char * s1, char const * s2) {
    return strcmp(s1, s2) != 0;
}

inline bool cmp(char const * s1, char * s2) {
	return strcmp(s1, s2) != 0;
}



#define BOOST_CHECK_EQUAL(x, y) if (cmp(x,y)) {\
std::cout << "Test Failed at " << __LINE__ << "\n";\
std::cout << "Got (" << (x) << ") expected (" << (y) << ") \n";\
TESTS.failure++;} else { TESTS.success++;}


#define BOOST_CHECK(x) if (!(x)) { TESTS.failure++; printf("Test Failed at %d\n", __LINE__); } else { TESTS.success++;}
#define BOOST_REQUIRE(x) if (!(x)) { TESTS.failure++; printf("Test Failed at %d\n", __LINE__); } else { TESTS.success++;}
#define BOOST_REQUIRE_EQUAL(x, y) if ((x)!=(y)) { TESTS.failure++; printf("Test Failed at %d\n", __LINE__); } else { TESTS.success++;}
#define BOOST_REQUIRE_NE(x, y) if ((x)==(y)) { TESTS.failure++; printf("Test Failed at %d\n", __LINE__); } else { TESTS.success++;}
#define BOOST_CHECK_MESSAGE(x,s) if(x) {TESTS.success++; printf("%s", s);} else {TESTS.failure++;};

#define BOOST_CHECK_EXCEPTION(statement, exception, predicate)\
    try {\
        statement;\
        TESTS.failure++;\
        printf("Test Failed at %d: not exception is caught\n", __LINE__);\
    }\
    catch (exception & e__) {\
        BOOST_CHECK(predicate(e__));\
    }\
    catch (...) {\
        TESTS.failure++;\
        printf("Test Failed at %d: incorrect exception " #exception " is caught\n", __LINE__);\
    }

#define BOOST_CHECK_THROW(statement, exception) \
    BOOST_CHECK_EXCEPTION(statement, exception, [](exception const &){ return true; })

#define CHECK_EXCEPTION_ERROR_ID(stmt, ErrId)   \
    BOOST_CHECK_EXCEPTION(                      \
        stmt, Error,                            \
        [&](Error const & e) {                  \
            if (e.id == ErrId) {                \
                BOOST_CHECK_EQUAL(e.id, ErrId); \
                return true;                    \
            }                                   \
            LOG(LOG_ERR, "Exception=%d", e.id); \
            return false;                       \
        }                                       \
    )

#ifdef BOOST_AUTO_TEST_MAIN
struct TESTS {
    struct item {
        const char * name;
        void (*fn)();
    };
    std::vector<item> tests;
    int success = 0;
    int failure = 0;

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
    printf("%d tests succeeded, %d tests failed\n",
        TESTS.success, TESTS.failure);
    if (TESTS.failure > 0) {
        return -1;
    }
    return 0;
}
#endif
