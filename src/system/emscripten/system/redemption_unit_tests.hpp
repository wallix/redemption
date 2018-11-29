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

#include "utils/pp.hpp"

#include <ostream>
#include <string>

#include <cstdio>
#include <cstring>

#ifndef RED_TEST_MODULE
# ifndef REDEMPTION_UNIT_TEST_CPP
#   error Missing RED_TEST_MODULE
# endif
#endif


#define REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()

#define RED_AUTO_TEST_CASE(name)                           \
    namespace redemption_unit_test__ {                     \
        void test_##name##__();                            \
        struct TEST_##name##__ {                           \
            TEST_##name##__() {                            \
                REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()     \
                ::redemption_unit_test__::                 \
                    add_test_case(#name, test_##name##__); \
            }                                              \
        } TEST_##name##__;                                 \
    }                                                      \
    void redemption_unit_test__::test_##name##__()



namespace redemption_unit_test__
{
    template<class T>
    T const& to_const(T& x) noexcept
    {
        return x;
    }

    template<class T>
    T const* to_const(T* x) noexcept
    {
        return x;
    }

    template<class T1, class U1, class T2, class U2>
    T1& select(T1& x, U1&, T2&, U2&)
    {
        return x;
    }

    inline std::string select(char const*, char const*, char const* x, char const*)
    {
        return x;
    }

    template<class T, class U>
    decltype(auto) get_a(T& a, U& b)
    {
        return select(a, b, to_const(a), to_const(b));
    }
}

#define REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(cond, s) do { \
    REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()                \
    if (!(cond)) {                                        \
        ::redemption_unit_test__::failure();              \
        ::redemption_unit_test__::get_output()            \
            << __FILE__ "(" PP_STRINGIFY(__LINE__)        \
               "): error: in \""                          \
            << ::redemption_unit_test__::current_name()   \
            << "\": " << s << std::endl                   \
        ;                                                 \
    }                                                     \
} while (0)

#define REDEMPTION_EMSCRIPTEN_CHECK(m, cond) \
    REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(cond, "check " PP_STRINGIFY(#cond) " has failed")

#define REDEMPTION_EMSCRIPTEN_CHECK_OP(m, op, x, y) do {     \
    REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()                   \
    auto && x__ = (x);                                       \
    auto && y__ = (y);                                       \
    REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(                     \
        ::redemption_unit_test__::get_a(x__, y__) op y__,    \
        "check " PP_STRINGIFY(x) " " #op " " PP_STRINGIFY(y) \
        " has failed [" << x__ << " " #op " " << y__ << "]"  \
    );                                                       \
} while (0)


#define RED_CHECK(cond) REDEMPTION_EMSCRIPTEN_CHECK(CHECK, cond)
#define RED_CHECK_MESSAGE(x, s) REDEMPTION_EMSCRIPTEN_CHECK(CHECK, x, s)
#define RED_CHECK_EQUAL(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, ==, x, y)
#define RED_CHECK_EQ RED_CHECK_EQUAL
#define RED_CHECK_NE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, !=, x, y)
#define RED_CHECK_LT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, <, x, y)
#define RED_CHECK_LE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, <=, x, y)
#define RED_CHECK_GT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, >, x, y)
#define RED_CHECK_GE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(CHECK, >=, x, y)

#define RED_REQUIRE(cond) REDEMPTION_EMSCRIPTEN_CHECK(REQUIRE, cond)
#define RED_REQUIRE_MESSAGE(x, s) REDEMPTION_EMSCRIPTEN_CHECK(REQUIRE, x, s)
#define RED_REQUIRE_EQUAL(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, ==, x, y)
#define RED_REQUIRE_NE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, !=, x, y)
#define RED_REQUIRE_LT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, <, x, y)
#define RED_REQUIRE_LE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, <=, x, y)
#define RED_REQUIRE_GT(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, >, x, y)
#define RED_REQUIRE_GE(x, y) REDEMPTION_EMSCRIPTEN_CHECK_OP(REQUIRE, >=, x, y)


#define REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(m, statement, exception, predicate) do { \
    try {                                                                              \
        statement;                                                                     \
        REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(                                           \
            false, "exception " PP_STRINGIFY(exception) " is expected"                 \
        );                                                                             \
    }                                                                                  \
    catch (exception & e__) {                                                          \
        RED_CHECK(predicate(e__));                                                     \
    }                                                                                  \
    catch (...) {                                                                      \
        TESTS.failure++;                                                               \
        REDEMPTION_EMSCRIPTEN_CHECK_MESSAGE(                                           \
            false, "incorrect exception " PP_STRINGIFY(exception) " is caught"         \
        );                                                                             \
    }                                                                                  \
} while (0)

#define RED_CHECK_EXCEPTION(statement, exception, predicate) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(CHECK, statement, exception, predicate)
#define RED_CHECK_THROW(statement, exception) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(CHECK, statement, exception, [](exception const &){ return true; })

#define RED_REQUIRE_EXCEPTION(statement, exception, predicate) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(REQUIRE, statement, exception, predicate)

#define RED_REQUIRE_THROW(statement, exception) \
    REDEMPTION_EMSCRIPTEN_CHECK_EXCEPTION(REQUIRE, statement, exception, [](exception const &){ return true; })

#define CHECK_EXCEPTION_ERROR_ID(stmt, ErrId)   \
    RED_CHECK_EXCEPTION(                        \
        stmt, Error,                            \
        [&](Error const & e) {                  \
            if (e.id == ErrId) {                \
                RED_CHECK_EQUAL(e.id, ErrId);   \
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

#define RED_CHECK_EQUAL_COLLECTIONS(beg1, end1, beg2, end2) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(CHECK, beg1, end1, beg2, end2)

#define RED_REQUIRE_EQUAL_COLLECTIONS(beg1, end1, beg2, end2) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(REQUIRE, beg1, end1, beg2, end2)


#define REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_RANGES(m, a, b) \
    do {                                                  \
        auto const & A__CHECK_RANGES = a;                 \
        auto const & B__CHECK_RANGES = b;                 \
        using std::begin;                                 \
        using std::end;                                   \
        REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_COLLECTIONS(    \
            m,                                            \
            begin(A__CHECK_RANGES), end(A__CHECK_RANGES), \
            begin(B__CHECK_RANGES), end(B__CHECK_RANGES)  \
        );                                                \
    } while (0)

#define RED_CHECK_EQUAL_RANGES(a, b) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_RANGES(CHECK, a, b)

#define RED_REQUIRE_EQUAL_RANGES(a_, b_) \
    REDEMPTION_EMSCRIPTEN_CHECK_EQUAL_RANGES(REQUIRE, a, b)


namespace redemption_unit_test__
{
    void add_test_case(char const* name, void(*fn)());
    int execute_tests(char const* module_name);

    char const* current_name() noexcept;
    void failure() noexcept;
    std::ostream& get_output() noexcept;
}

#ifndef REDEMPTION_UNIT_TEST_CPP
int main()
{
    return ::redemption_unit_test__::execute_tests(PP_STRINGIFY(RED_TEST_MODULE));
}
#endif
