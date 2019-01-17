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
#include <string_view>

#include <cstdio>
#include <cstring>


namespace redemption_unit_test__
{
namespace emscripten
{
    void add_test_case(char const* filename, int line, char const* name, void(*fn)());
    int execute_tests(char const* module_name);

    char const* current_name() noexcept;
    void failure() noexcept;
    std::ostream& get_output() noexcept;

    void passpoint(char const* filename, int line) noexcept;
    void checkpoint(char const* filename, int line, char const* name) noexcept;

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
    T1&& select(T1&& x, U1&&, T2&&, U2&&) noexcept
    {
        return static_cast<T1&&>(x);
    }

    template<class T1, class U1>
    std::string_view select(T1&&, U1&&, char const* x, char const*) noexcept
    {
        return x;
    }

    template<class T, class U>
    decltype(auto) get_a(T& a, U& b)
    {
        return select(a, b, to_const(a), to_const(b));
    }

    struct PrintableChar
    {
        unsigned char c;
    };

    std::ostream& operator<<(std::ostream& out, PrintableChar const& c);

    PrintableChar ostream_wrap(unsigned char x) noexcept;
    PrintableChar ostream_wrap(char x) noexcept;
    PrintableChar ostream_wrap(signed char x) noexcept;

    template<class T>
    struct delegate_ostream
    {
        T const& x;
    };

    template<class T>
    std::ostream& red_test_print_type(std::ostream& out, T const& x)
    {
        return out << x;
    }

    template<class T>
    std::ostream& operator<<(std::ostream& out, delegate_ostream<T> const& d)
    {
        return red_test_print_type(out, d.x);
    }

    template<class T>
    delegate_ostream<T> ostream_wrap(T const& x) noexcept
    {
        return delegate_ostream<T>{x};
    }


    template<class... Xs>
    auto faccumulator(Xs&&... xs)
    {
        return [&]([[maybe_unused]] auto&&... y) {
            if constexpr (sizeof...(y)) {
                return faccumulator(
                    static_cast<Xs&&>(xs)...,
                    static_cast<decltype(y)&&>(y)...
                );
            }
            else {
                return [&](auto f){
                    f(static_cast<Xs&&>(xs)...);
                };
            }
        };
    }

    template<class F>
    struct ostream_function
    {
        F f;
    };

    template<class F>
    ostream_function(F f) -> ostream_function<F>;

    template<class F>
    std::ostream& operator<<(std::ostream& out, ostream_function<F> const& f)
    {
        f.f(out);
        return out;
    }

} // namespace emscripten
} // namespace redemption_unit_test__

#ifdef RED_TEST_MODULE
int main()
{
    return ::redemption_unit_test__::emscripten::execute_tests(RED_PP_STRINGIFY(RED_TEST_MODULE));
}
#endif


#define RED_TEST_PRINT_TYPE_FUNCTION_NAME red_test_print_type

#define RED_TEST_DONT_PRINT_LOG_VALUE(type)    \
    template<> struct redemption_unit_test__:: \
    emscripten::delegate_ostream<::type> {     \
        delegate_ostream(::type const&){}      \
        char const* x = "";                    \
    };                                         \


#define RED_AUTO_TEST_CASE(name)                       \
    namespace redemption_unit_test__::emscripten::D {  \
        void test_##name##__();                        \
        const struct TEST_##name##__ {                 \
            TEST_##name##__() {                        \
                RED_TEST_PASSPOINT();                  \
                ::redemption_unit_test__::emscripten:: \
                    add_test_case(                     \
                        __FILE__, __LINE__,            \
                        #name, test_##name##__);       \
            }                                          \
        } TEST_##name##__;                             \
    }                                                  \
    void redemption_unit_test__::emscripten::D::test_##name##__()

#define RED_FAIL(ostream_expr) RED_CHECK_MESSAGE(false, ostream_expr)
#define RED_ERROR(ostream_expr) RED_REQUIRE_MESSAGE(false, ostream_expr)

#define RED_TEST_PASSPOINT() \
    ::redemption_unit_test__::emscripten::passpoint(__FILE__, __LINE__)
#define RED_TEST_CHECKPOINT(name) \
    ::redemption_unit_test__::emscripten::checkpoint(__FILE__, __LINE__, name)


#define RED_TEST_MESSAGE(lvl, cond, s) do {                         \
    RED_TEST_PASSPOINT();                                           \
    if (!(cond)) {                                                  \
        ::redemption_unit_test__::emscripten::failure();            \
        ::redemption_unit_test__::emscripten::get_output()          \
            << __FILE__ "(" RED_PP_STRINGIFY(__LINE__)              \
               "): error: in \""                                    \
            << ::redemption_unit_test__::emscripten::current_name() \
            << "\": " << s << std::endl                             \
        ;                                                           \
    }                                                               \
} while (0)


#define RED_TEST(lvl, cond) \
    RED_TEST_MESSAGE(lvl, cond, "check " #cond " has failed")


#define RED_TEST_OP(lvl, op, x, y)                                             \
    [](auto&& x__, auto&& y__) {                                               \
        RED_TEST_MESSAGE(lvl,                                                  \
            ::redemption_unit_test__::emscripten::get_a(x__, y__) op y__,      \
            RED_TEST_STRING_##lvl " " #x " " #op " " #y " has failed [" <<     \
            ::redemption_unit_test__::emscripten::ostream_wrap(x__) << " "     \
            #op " " << ::redemption_unit_test__::emscripten::ostream_wrap(y__) \
            << "]"                                                             \
        );                                                                     \
    }(x, y)



#define RED_TEST_PREDICATE(lvl, pred, arg_list)                          \
    redemption_unit_test__::emscripten::faccumulator arg_list ()(        \
        [&](auto&& arg1__, auto&&... args__) {                           \
            RED_TEST_MESSAGE(                                            \
                lvl, pred(                                               \
                    static_cast<decltype(arg1__)&&>(arg1__),             \
                    static_cast<decltype(args__) &&>(args__)...),        \
                #pred " with " #arg_list " is not satisfied for "        \
                << redemption_unit_test__::emscripten::ostream_function{ \
                    [&](::std::ostream& out__) {                         \
                        out__ << arg1__;                                 \
                        ((out__ << ", " << args__), ...);                \
                    }}                                                   \
            );                                                           \
        }                                                                \
    )


#define RED_TEST_NO_THROW(lvl, stmt) do {             \
    try {                                             \
        RED_TEST_PASSPOINT();                         \
        stmt;                                         \
    }                                                 \
    catch (...) {                                     \
        RED_TEST_MESSAGE(lvl, false,                  \
            "unexpected exception thrown by " #stmt); \
    }                                                 \
} while(0)


#define RED_TEST_THROW(lvl, stmt, exception) do {            \
    try {                                                    \
        RED_TEST_PASSPOINT();                                \
        stmt;                                                \
        RED_TEST_MESSAGE(lvl, false,                         \
            "exception " #exception " is expected");         \
    }                                                        \
    catch (exception const&) {                               \
    }                                                        \
    catch (...) {                                            \
        RED_TEST_MESSAGE(lvl, false,                         \
            "incorrect exception " #exception " is caught"); \
    }                                                        \
} while(0)


#define RED_TEST_EXCEPTION(lvl, stmt, exception, predicate) do { \
    try {                                                        \
        RED_TEST_PASSPOINT();                                    \
        stmt;                                                    \
        RED_TEST_MESSAGE(lvl, false,                             \
            "exception " #exception " is expected");             \
    }                                                            \
    catch (exception const& exception__) {                       \
        RED_TEST(lvl, predicate(exception__));                   \
    }                                                            \
    catch (...) {                                                \
        RED_TEST_MESSAGE(lvl, false,                             \
            "incorrect exception " #exception " is caught");     \
    }                                                            \
} while(0)


#define RED_TEST_EQUAL_COLLECTIONS(lvl, first1, last1, first2, last2) \
    RED_TEST_MESSAGE(                                                 \
        lvl, std::equal(first1, last1, first2, last2),                \
        "check " "std::equal( " #first1 ", " #last1 ",                \
        " #first2 ", " #last2 ") has failed")



/// RED_CHECK functions

#define RED_CHECK_MESSAGE(x, s) RED_TEST_MESSAGE(CHECK, x, s)
#define RED_CHECK(cond) RED_TEST(CHECK, cond)
#define RED_CHECK_EQUAL(x, y) RED_TEST_OP(CHECK, ==, x, y)
#define RED_CHECK_EQ RED_CHECK_EQUAL
#define RED_CHECK_NE(x, y) RED_TEST_OP(CHECK, !=, x, y)
#define RED_CHECK_LT(x, y) RED_TEST_OP(CHECK, <, x, y)
#define RED_CHECK_LE(x, y) RED_TEST_OP(CHECK, <=, x, y)
#define RED_CHECK_GT(x, y) RED_TEST_OP(CHECK, >, x, y)
#define RED_CHECK_GE(x, y) RED_TEST_OP(CHECK, >=, x, y)

#define RED_CHECK_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    RED_TEST_EQUAL_COLLECTIONS(CHECK, first1, last1, first2, last2)

#define RED_CHECK_PREDICATE(pred, arg_list) \
    RED_TEST_PREDICATE(CHECK, pred, arg_list)

#define RED_CHECK_NO_THROW(stmt) \
    RED_TEST_NO_THROW(CHECK, stmt)

#define RED_CHECK_THROW(stmt, exception) \
    RED_TEST_THROW(CHECK, stmt, exception)

#define RED_CHECK_EXCEPTION(stmt, exception, predicate) \
    RED_TEST_EXCEPTION(CHECK, stmt, exception, predicate)



/// RED_REQUIRE functions

#define RED_REQUIRE_MESSAGE(x, s) RED_TEST_MESSAGE(REQUIRE, x, s)
#define RED_REQUIRE(cond) RED_TEST(REQUIRE, cond)
#define RED_REQUIRE_EQUAL(x, y) RED_TEST_OP(REQUIRE, ==, x, y)
#define RED_REQUIRE_EQ RED_REQUIRE_EQUAL
#define RED_REQUIRE_NE(x, y) RED_TEST_OP(REQUIRE, !=, x, y)
#define RED_REQUIRE_LT(x, y) RED_TEST_OP(REQUIRE, <, x, y)
#define RED_REQUIRE_LE(x, y) RED_TEST_OP(REQUIRE, <=, x, y)
#define RED_REQUIRE_GT(x, y) RED_TEST_OP(REQUIRE, >, x, y)
#define RED_REQUIRE_GE(x, y) RED_TEST_OP(REQUIRE, >=, x, y)

#define RED_REQUIRE_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    RED_TEST_EQUAL_COLLECTIONS(REQUIRE, first1, last1, first2, last2)

#define RED_REQUIRE_PREDICATE(pred, arg_list) \
    RED_TEST_PREDICATE(REQUIRE, pred, arg_list)

#define RED_REQUIRE_NO_THROW(stmt) \
    RED_TEST_NO_THROW(REQUIRE, stmt)

#define RED_REQUIRE_THROW(stmt, exception) \
    RED_TEST_THROW(REQUIRE, stmt, exception)

#define RED_REQUIRE_EXCEPTION(stmt, exception, predicate) \
    RED_TEST_EXCEPTION(REQUIRE, stmt, exception, predicate)
