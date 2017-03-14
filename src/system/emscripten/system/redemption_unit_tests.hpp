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

#define REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()

#define BOOST_AUTO_TEST_CASE(name) void name(); \
struct TEST_ ## name {                          \
    TEST_ ## name() {                           \
        REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()  \
        TESTS.add({"" #name, &name});           \
    }                                           \
} TEST_ ## name;                                \
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

#define REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(m, cond, s) do { \
    REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()                   \
    if (cond) TESTS.success++;                               \
    else {                                                   \
        TESTS.failure++;                                     \
        std::cerr                                            \
          << "Test Failed at " STRINGIFY(__LINE__)           \
          " in " << TESTS.current_name << ": " << s          \
          << std::endl                                       \
        ;                                                    \
    }                                                        \
} while (0)

#define REDEMPTION_EMSCRIPTEN_CHECK(m, cond) \
    REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(m, cond, "check " STRINGIZE(#cond) " failed")

#define REDEMPTION_EMSCRIPTEN_CHECK_OP(m, op, x, y) do { \
    REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()               \
    auto && TEST_x = (x);                                \
    auto && TEST_y = (y);                                \
    REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(                 \
        m, TEST_x op TEST_y, "check " STRINGIZE(x) " "   \
        #op " " STRINGIZE(y) " failed [" << TEST_x <<    \
        " " #op " " << TEST_y << "]"                     \
    );                                                   \
} while (0)


#define BOOST_CHECK(cond) REDEMPTION_EMSCRIPTEN_CHECK(CHECK, cond)
#define BOOST_CHECK_MESSAGE(x, s) REDEMPTION_EMSCRIPTEN_CHECK(CHECK, x, s)
#define BOOST_CHECK_EQUAL(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, ==, x, y)
#define BOOST_CHECK_NE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, !=, x, y)
#define BOOST_CHECK_LT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, <, x, y)
#define BOOST_CHECK_LE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, <=, x, y)
#define BOOST_CHECK_GT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, >, x, y)
#define BOOST_CHECK_GE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, >=, x, y)

#define BOOST_REQUIRE(cond) REDEMPTION_EMSCRIPTEN_CHECK(REQUIRE, cond)
#define BOOST_REQUIRE_MESSAGE(x, s) REDEMPTION_EMSCRIPTEN_CHECK(REQUIRE, x, s)
#define BOOST_REQUIRE_EQUAL(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, ==, x, y)
#define BOOST_REQUIRE_NE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, !=, x, y)
#define BOOST_REQUIRE_LT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, <, x, y)
#define BOOST_REQUIRE_LE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, <=, x, y)
#define BOOST_REQUIRE_GT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, >, x, y)
#define BOOST_REQUIRE_GE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, >=, x, y)


#define REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(m, statement, exception, predicate) do { \
    try {                                                              \
        statement;                                                     \
        REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(                           \
            m, false,                                                  \
            "exception " STRINGIZE(exception) " is expected"           \
        );                                                             \
    }                                                                  \
    catch (exception & e__) {                                          \
        BOOST_CHECK(predicate(e__));                                   \
    }                                                                  \
    catch (...) {                                                      \
        TESTS.failure++;                                               \
        REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(                           \
            m, false,                                                  \
            "incorrect exception " STRINGIZE(exception) " is caught"   \
        );                                                             \
    }                                                                  \
} while (0)

#define BOOST_CHECK_EXCEPTION(statement, exception, predicate) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(CHECK, statement, exception, predicate)
#define BOOST_CHECK_THROW(statement, exception) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(CHECK, statement, exception, [](exception const &){ return true; })

#define BOOST_REQUIRE_EXCEPTION(statement, exception, predicate) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(REQUIRE, statement, exception, predicate)

#define BOOST_REQUIRE_THROW(statement, exception) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(REQUIRE, statement, exception, [](exception const &){ return true; })

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


#define REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(m, beg1, end1, beg2, end2) do { \
    auto TEST_first1 = beg1;                                                          \
    auto TEST_last1 = end1;                                                           \
    auto TEST_first2 = beg2;                                                          \
    auto TEST_last2 = end2;                                                           \
    for (; TEST_first1 != TEST_last1                                                  \
        && TEST_first2 != TEST_last2;                                                 \
        void(++TEST_first1), ++TEST_first2                                            \
    ) {                                                                               \
        REDEMPTION_EMSCRIPTEN_CHECK_OP(m, ==, *TEST_first1, *TEST_first2)             \
    }                                                                                 \
    REDEMPTION_EMSCRIPTEN_CHECK_OP(m, ==, *TEST_first1, *TEST_last1);                 \
    REDEMPTION_EMSCRIPTEN_CHECK_OP(m, ==, *TEST_first2, *TEST_last2);                 \
} while (0)

#define BOOST_CHECK_EQUAL_COLLECTIONS(beg1, end1, beg2, end2) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(CHECK, beg1, end1, beg2, end2)

#define BOOST_REQUIRE_EQUAL_COLLECTIONS(beg1, end1, beg2, end2) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(REQUIRE, beg1, end1, beg2, end2)


#define REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_RANGES(m, a, b) \
    do {                                                   \
        auto const & A__CHECK_RANGES = a;                  \
        auto const & B__CHECK_RANGES = b;                  \
        using std::begin;                                  \
        using std::end;                                    \
        REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(     \
            m,                                             \
            begin(A__CHECK_RANGES), end(A__CHECK_RANGES),  \
            begin(B__CHECK_RANGES), end(B__CHECK_RANGES)   \
        );                                                 \
    } while (0)

#define BOOST_CHECK_EQUAL_RANGES(a, b) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_RANGES(CHECK, a, b)

#define BOOST_REQUIRE_EQUAL_RANGES(a_, b_) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_RANGES(REQUIRE, a, b)


#ifdef BOOST_AUTO_TEST_MAIN
struct TESTS {
    struct item {
        const char * name;
        void (*fn)();
    };
    std::vector<item> tests;
    int success = 0;
    int failure = 0;
    const char * current_name;

    void add(item item){
        this->tests.push_back(item);
    }
} TESTS;

#define STRINGIFY_I(x) #x
#define STRINGIFY(x) STRINGIFY_I(x)
#define MODULE_NAME(x) printf("Running test suite " STRINGIFY(x) "\n");

int main(){
    MODULE_NAME(BOOST_TEST_MODULE);
    for (auto& item: TESTS.tests){
        printf("Running test %s\n", item.name);
        TESTS.current_name = item.name;
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
