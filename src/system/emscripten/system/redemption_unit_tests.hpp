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
    return ::redemption_unit_test__::execute_tests(RED_PP_STRINGIFY(RED_TEST_MODULE));
}

# define RED_AUTO_TEST_CASE(name)                          \
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
#endif


#define REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()


#define RED_EMSCRIPTEN_TEST_MESSAGE(lvl, cond, s) do {  \
    REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()              \
    if (!(cond)) {                                      \
        ::redemption_unit_test__::failure();            \
        ::redemption_unit_test__::get_output()          \
            << __FILE__ "(" RED_PP_STRINGIFY(__LINE__)  \
               "): error: in \""                        \
            << ::redemption_unit_test__::current_name() \
            << "\": " << s << std::endl                 \
        ;                                               \
    }                                                   \
} while (0)


#define RED_EMSCRIPTEN_TEST(lvl, cond) \
    RED_EMSCRIPTEN_TEST_MESSAGE(lvl, cond, "check " RED_PP_STRINGIFY(#cond) " has failed")


#define RED_EMSCRIPTEN_TEST_OP(lvl, op, x, y) do {                 \
    REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT()                         \
    auto && x__ = (x);                                             \
    auto && y__ = (y);                                             \
    RED_EMSCRIPTEN_TEST_MESSAGE(lvl,                               \
        ::redemption_unit_test__::get_a(x__, y__) op y__, "check " \
        RED_PP_STRINGIFY(x) " " #op " " RED_PP_STRINGIFY(y)        \
        " has failed [" << x__ << " " #op " " << y__ << "]"        \
    );                                                             \
} while (0)


#define RED_EMSCRIPTEN_TEST_PREDICATE(lvl, pred, arg_list)   \
    [](auto&& f__pred__) {                                   \
        if (!(f__pred__())) { RED_EMSCRIPTEN_TEST_MESSAGE(   \
            lvl, false,                                      \
            #pred " with " #arg_list "is not satisfied for " \
            << f__pred__(1, 1)                               \
        ); }                                                 \
    }(redemption_unit_test__::faccumulator(pred) arg_list)


#define RED_EMSCRIPTEN_TEST_NO_THROW(lvl, stmt) do {  \
    try {                                             \
        stmt;                                         \
        REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT();       \
    }                                                 \
    catch (...) {                                     \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, false,       \
            "unexpected exception thrown by " #stmt); \
    }                                                 \
} while(0)


#define RED_EMSCRIPTEN_TEST_THROW(lvl, stmt, exception) do { \
    try {                                                    \
        stmt;                                                \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, false,              \
            "exception " #exception " is expected");         \
    }                                                        \
    catch (exception const&) {                               \
        REDEMPTION_EMSCRIPTEN_TEST_PASSPOINT();              \
    }                                                        \
    catch (...) {                                            \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, false,              \
            "incorrect exception " #exception " is caught"); \
    }                                                        \
} while(0)


#define RED_EMSCRIPTEN_TEST_EXCEPTION(lvl, stmt, exception, predicate) do { \
    try {                                                                   \
        stmt;                                                               \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, false,                             \
            "exception " #exception " is expected");                        \
    }                                                                       \
    catch (exception const& exception__) {                                  \
        RED_EMSCRIPTEN_TEST(lvl, predicate(exception__));                   \
    }                                                                       \
    catch (...) {                                                           \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, false,                             \
            "incorrect exception " #exception " is caught");                \
    }                                                                       \
} while(0)


#define RED_EMSCRIPTEN_TEST_EXCEPTION_ERROR_ID(lvl, stmt, ErrId) \
    RED_EMSCRIPTEN_TEST_EXCEPTION(                               \
        m, stmt, Error, [&](Error const & e__) {                 \
            RED_EMSCRIPTEN_TEST_OP(lvl, ==, e__.id, ErrId);      \
            return (e__.id == ErrId);                            \
        }                                                        \
    )


#define RED_EMSCRIPTEN_TEST_EQUAL_COLLECTIONS(lvl, first1, last1, first2, last2) \
    RED_EMSCRIPTEN_TEST_MESSAGE(                                                 \
        lvl, std::equal(first1, last1, first2, last2),                           \
        "check " "std::equal( " RED_PP_STRINGIFY(first1) ", "                    \
        RED_PP_STRINGIFY(last1) ", " RED_PP_STRINGIFY(first2) ", "               \
        RED_PP_STRINGIFY(last2) ") has failed")


#define RED_EMSCRIPTEN_TEST_EQUAL_RANGES(lvl, a, b) \
    [](auto const & a__, auto const & b__) {        \
        using std::begin;                           \
        using std::end;                             \
        RED_EMSCRIPTEN_TEST_EQUAL_COLLECTIONS(lvl,  \
            (void(#a), begin(a__)), end(a__),       \
            (void(#b), begin(b__)), end(b__)        \
        );                                          \
    }(a, b)


#define RED_EMSCRIPTEN_TEST_MEM(lvl, a, b)           \
    [](const_bytes_view a__, const_bytes_view b__) { \
        RED_EMSCRIPTEN_TEST_EQUAL_COLLECTIONS(lvl,   \
            (void(#a), a__.begin()), a__.end(),      \
            (void(#b), b__.begin()), b__.end()       \
        );                                           \
    }(a, b)


// require #include "utils/fileutils.hpp"
#define RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN(lvl, filename, size) \
    [](auto&& filename__, ::std::size_t size__) {                    \
        RED_EMSCRIPTEN_TEST_OP(lvl, ==,                              \
            filesize((void(#filename), filename__)), size__);        \
        ::unlink(filename__);                                        \
    }(filename, size1)


#define RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN2(lvl, filename, size1, size2) \
    [](auto&& filename__, ::std::size_t size1__, ::std::size_t size2__) {     \
        size_t fsize__ = filesize(filename__);                                \
        if (fsize__ != size2__) {                                             \
            RED_EMSCRIPTEN_TEST_OP(lvl, ==,                                   \
                (void("filesize(" #filename ")"), fsize__), size1__);         \
        }                                                                     \
        ::unlink(filename__);                                                 \
    }(filename, size1, size2)


#define RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN3(lvl, filename, size1, size2, size3)             \
    [](auto&& filename__, ::std::size_t size1__, ::std::size_t size2__, ::std::size_t size3__) { \
        size_t fsize__ = filesize(filename__);                                                   \
        if (fsize__ != size2__ && fsize__ != size3__){                                           \
            RED_EMSCRIPTEN_TEST_OP(lvl, ==,                                                      \
                (void("filesize(" #filename ")"), fsize__), size1__);                            \
        }                                                                                        \
        ::unlink(filename__);                                                                    \
    }(filename, size1, size2, size3)


#define RED_EMSCRIPTEN_TEST_FILE_EXISTS(lvl, filename)                \
    [](auto&& filename__) {                                           \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, file_exist(filename__),      \
            "check file_exist(\"" << filename__ << "\") has failed"); \
    }(filename)


#define RED_EMSCRIPTEN_TEST_FILE_NOT_EXISTS(lvl, filename)             \
    [](auto&& filename__) {                                            \
        RED_EMSCRIPTEN_TEST_MESSAGE(lvl, !file_exist(filename__),      \
            "check !file_exist(\"" << filename__ << "\") has failed"); \
    }(filename)


// require #include "test_only/get_file_contents.hpp"
#define RED_EMSCRIPTEN_TEST_FILE_CONTENTS(lvl, filename, contents)          \
    [](auto&& filename__, auto&& contents__) {                              \
        RED_EMSCRIPTEN_TEST_OP(lvl, ==,                                     \
            get_file_contents((void(#filename), filename__)), contents___); \
        ::unlink(filename__);                                               \
    }(filename, contents)


# define RED_FAIL(ostream_expr) RED_CHECK_MESSAGE(false, ostream_expr)
# define RED_ERROR(ostream_expr) RED_REQUIRE_MESSAGE(false, ostream_expr)


/// RED_CHECK functions

#define RED_CHECK_MESSAGE(x, s) RED_EMSCRIPTEN_TEST_MESSAGE(CHECK, x, s)
#define RED_CHECK(cond) RED_EMSCRIPTEN_TEST(CHECK, cond)
#define RED_CHECK_EQUAL(x, y) RED_EMSCRIPTEN_TEST_OP(CHECK, ==, x, y)
#define RED_CHECK_EQ RED_CHECK_EQUAL
#define RED_CHECK_NE(x, y) RED_EMSCRIPTEN_TEST_OP(CHECK, !=, x, y)
#define RED_CHECK_LT(x, y) RED_EMSCRIPTEN_TEST_OP(CHECK, <, x, y)
#define RED_CHECK_LE(x, y) RED_EMSCRIPTEN_TEST_OP(CHECK, <=, x, y)
#define RED_CHECK_GT(x, y) RED_EMSCRIPTEN_TEST_OP(CHECK, >, x, y)
#define RED_CHECK_GE(x, y) RED_EMSCRIPTEN_TEST_OP(CHECK, >=, x, y)

#define RED_CHECK_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    RED_CHECK(std::equal(first1, last1, first2, last2))

#define RED_CHECK_PREDICATE(pred, arg_list) \
    RED_EMSCRIPTEN_TEST_PREDICATE(CHECK, pred, arg_list)

#define RED_CHECK_NO_THROW(stmt) \
    RED_EMSCRIPTEN_TEST_NO_THROW(CHECK, stmt)

#define RED_CHECK_THROW(stmt, exception) \
    RED_EMSCRIPTEN_TEST_THROW(CHECK, stmt, exception)

#define RED_CHECK_EXCEPTION(stmt, exception, predicate) \
    RED_EMSCRIPTEN_TEST_EXCEPTION(CHECK, stmt, exception, exception)

#define RED_CHECK_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_EMSCRIPTEN_TEST_EXCEPTION_ERROR_ID(CHECK, stmt, ErrId)

#define RED_CHECK_EQUAL_RANGES(a, b) \
    RED_EMSCRIPTEN_TEST_EQUAL_RANGES(CHECK, a, b)

#define RED_CHECK_EQ_RANGES RED_CHECK_EQUAL_RANGES

#define RED_CHECK_MEM(mem, memref) \
    RED_EMSCRIPTEN_TEST_MEM(CHECK, mem, memref)

#define RED_CHECK_MEM_C(mem, memref) \
    RED_CHECK_MEM(mem, cstr_array_view("" memref))

#define RED_CHECK_MEM_AC(mem, memref) \
    RED_CHECK_MEM(make_array_view(mem), cstr_array_view("" memref))

#define RED_CHECK_MEM_AA(mem, memref) \
    RED_CHECK_MEM(make_array_view(mem), make_array_view(memref))

#define RED_REQUIRE_MEM_C(mem, memref) \
    RED_REQUIRE_MEM(mem, cstr_array_view("" memref))

#define RED_REQUIRE_MEM_AC(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), cstr_array_view("" memref))

#define RED_REQUIRE_MEM_AA(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), make_array_view(memref))

#define RED_CHECK_SMEM RED_CHECK_MEM
#define RED_CHECK_SMEM_C RED_CHECK_MEM_C
#define RED_CHECK_SMEM_AC RED_CHECK_MEM_AC
#define RED_CHECK_SMEM_AA RED_CHECK_MEM_AA

// require #include "utils/fileutils.hpp"
#define RED_CHECK_FILE_SIZE_AND_CLEAN(filename, size) \
    RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN(CHECK, filename, size)

#define RED_CHECK_FILE_SIZE_AND_CLEAN2(filename, size1, size2) \
    RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN2(CHECK, filename, size1, size2)

#define RED_CHECK_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3) \
    RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN3(CHECK, filename, size1, size2, size3)

#define RED_CHECK_FILE_EXISTS(filename) \
    RED_EMSCRIPTEN_TEST_FILE_EXISTS(CHECK, filename)

#define RED_CHECK_FILE_NOT_EXISTS(filename) \
    RED_EMSCRIPTEN_TEST_FILE_NOT_EXISTS(CHECK, filename)

// require #include "test_only/get_fikle_contents.hpp"
#define RED_CHECK_FILE_CONTENTS(filename, contents) \
    RED_EMSCRIPTEN_TEST_FILE_CONTENTS(CHECK, filename, contents)




/// RED_REQUIRE functions

#define RED_REQUIRE_MESSAGE(x, s) RED_EMSCRIPTEN_TEST_MESSAGE(REQUIRE, x, s)
#define RED_REQUIRE(cond) RED_EMSCRIPTEN_TEST(REQUIRE, cond)
#define RED_REQUIRE_EQUAL(x, y) RED_EMSCRIPTEN_TEST_OP(REQUIRE, ==, x, y)
#define RED_REQUIRE_EQ RED_REQUIRE_EQUAL
#define RED_REQUIRE_NE(x, y) RED_EMSCRIPTEN_TEST_OP(REQUIRE, !=, x, y)
#define RED_REQUIRE_LT(x, y) RED_EMSCRIPTEN_TEST_OP(REQUIRE, <, x, y)
#define RED_REQUIRE_LE(x, y) RED_EMSCRIPTEN_TEST_OP(REQUIRE, <=, x, y)
#define RED_REQUIRE_GT(x, y) RED_EMSCRIPTEN_TEST_OP(REQUIRE, >, x, y)
#define RED_REQUIRE_GE(x, y) RED_EMSCRIPTEN_TEST_OP(REQUIRE, >=, x, y)

#define RED_REQUIRE_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    RED_REQUIRE(std::equal(first1, last1, first2, last2))

#define RED_REQUIRE_PREDICATE(pred, arg_list) \
    RED_EMSCRIPTEN_TEST_PREDICATE(REQUIRE, pred, arg_list)

#define RED_REQUIRE_NO_THROW(stmt) \
    RED_EMSCRIPTEN_TEST_NO_THROW(REQUIRE, stmt)

#define RED_REQUIRE_THROW(stmt, exception) \
    RED_EMSCRIPTEN_TEST_THROW(REQUIRE, stmt, exception)

#define RED_REQUIRE_EXCEPTION(stmt, exception, predicate) \
    RED_EMSCRIPTEN_TEST_EXCEPTION(REQUIRE, stmt, exception, exception)

#define RED_REQUIRE_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_EMSCRIPTEN_TEST_EXCEPTION_ERROR_ID(REQUIRE, stmt, ErrId)

#define RED_REQUIRE_EQUAL_RANGES(a, b) \
    RED_EMSCRIPTEN_TEST_EQUAL_RANGES(REQUIRE, a, b)

#define RED_REQUIRE_EQ_RANGES RED_REQUIRE_EQUAL_RANGES

#define RED_REQUIRE_MEM(mem, memref) \
    RED_EMSCRIPTEN_TEST_MEM(REQUIRE, mem, memref)

#define RED_REQUIRE_MEM_C(mem, memref) \
    RED_REQUIRE_MEM(mem, cstr_array_view("" memref))

#define RED_REQUIRE_MEM_AC(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), cstr_array_view("" memref))

#define RED_REQUIRE_MEM_AA(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), make_array_view(memref))

#define RED_REQUIRE_MEM_C(mem, memref) \
    RED_REQUIRE_MEM(mem, cstr_array_view("" memref))

#define RED_REQUIRE_MEM_AC(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), cstr_array_view("" memref))

#define RED_REQUIRE_MEM_AA(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), make_array_view(memref))

#define RED_REQUIRE_SMEM RED_REQUIRE_MEM
#define RED_REQUIRE_SMEM_C RED_REQUIRE_MEM_C
#define RED_REQUIRE_SMEM_AC RED_REQUIRE_MEM_AC
#define RED_REQUIRE_SMEM_AA RED_REQUIRE_MEM_AA

// require #include "utils/fileutils.hpp"
#define RED_REQUIRE_FILE_SIZE_AND_CLEAN(filename, size) \
    RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN(REQUIRE, filename, size)

#define RED_REQUIRE_FILE_SIZE_AND_CLEAN2(filename, size1, size2) \
    RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN2(REQUIRE, filename, size1, size2)

#define RED_REQUIRE_FILE_SIZE_AND_CLEAN3(filename, size1, size2, size3) \
    RED_EMSCRIPTEN_TEST_FILE_SIZE_AND_CLEAN3(REQUIRE, filename, size1, size2, size3)

#define RED_REQUIRE_FILE_EXISTS(filename) \
    RED_EMSCRIPTEN_TEST_FILE_EXISTS(REQUIRE, filename)

#define RED_REQUIRE_FILE_NOT_EXISTS(filename) \
    RED_EMSCRIPTEN_TEST_FILE_NOT_EXISTS(REQUIRE, filename)

// require #include "test_only/get_fikle_contents.hpp"
#define RED_REQUIRE_FILE_CONTENTS(filename, contents) \
    RED_EMSCRIPTEN_TEST_FILE_CONTENTS(REQUIRE, filename, contents)




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
    T1&& select(T1&& x, U1&&, T2&&, U2&&)
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

    class delegate_ostream {};

    template<class F, class... Xs>
    auto faccumulator(F&& f, Xs&&... xs)
    {
        return [&](auto&&... y){
            if constexpr (sizeof...(y) == 1) {
                return faccumulator(
                    static_cast<F&&>(f),
                    static_cast<Xs&&>(xs)...,
                    static_cast<decltype(y)&&>(y)...
                );
            }
            else if constexpr (sizeof...(y) == 2) {
                return [&](std::ostream& out){
                    ((out << xs << ", ") << ...);
                };
            }
            else {
                return static_cast<F&&>(f)(static_cast<Xs&&>(xs)...);
            }
        };
    }
}
