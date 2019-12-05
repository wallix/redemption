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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#include "cxx/diagnostic.hpp"
#include "utils/sugar/bytes_view.hpp"
#include <iterator> // std:begin / std::end
#include <type_traits>


#if defined(IN_IDE_PARSER) && !defined(REDEMPTION_UNIT_TEST_CPP)

namespace redemption_unit_test__
{
    void X(bool);

    struct Stream
    {
        template<class T>
        Stream operator<<(T const&)
        {
            return *this;
        }

        explicit operator bool() const { return true; }
    };

    template<class> class red_test_print_type_t;

    template<class F>
    struct fn_caller
    {
        F f;

        template<class... Ts>
        constexpr auto operator()(Ts&&... xs) const
        {
            return f(static_cast<Ts>(xs)...);
        }
    };

    template<class F>
    constexpr fn_caller<F> fn_invoker(char const* /*name*/, F f);
} // namespace redemption_unit_test__

template<class T, class U>  bool operator==(array_view<T>, array_view<U>);
template<class T> bool operator==(array_view<T>, bytes_view);
template<class U> bool operator==(bytes_view, array_view<U>);
bool operator==(bytes_view, bytes_view);

template<class T, class U>  bool operator!=(array_view<T>, array_view<U>);
template<class T> bool operator!=(array_view<T>, bytes_view);
template<class U> bool operator!=(bytes_view, array_view<U>);
bool operator!=(bytes_view, bytes_view);

# define FIXTURES_PATH "./tests/fixtures"
# define CFG_PATH "./sys/etc/rdpproxy"

# define RED_FAIL(ostream_expr) ::redemption_unit_test__::Stream{} << ostream_expr
# define RED_ERROR(ostream_expr) ::redemption_unit_test__::Stream{} << ostream_expr
# define RED_TEST_CHECKPOINT(ostream_expr) ::redemption_unit_test__::Stream{} << ostream_expr
# define RED_TEST_MESSAGE(ostream_expr) ::redemption_unit_test__::Stream{} << ostream_expr
# define RED_TEST_PASSPOINT() do { } while(0)
# define RED_TEST_DONT_PRINT_LOG_VALUE(type)
# define RED_TEST_PRINT_TYPE_FUNCTION_NAME red_test_print_type
# define RED_TEST_PRINT_TYPE_STRUCT_NAME redemption_unit_test__::red_test_print_type_t


# define RED_AUTO_TEST_CASE(test_name)       \
    struct test_name { void operator()(); }; \
    void test_name::operator()()

/// CHECK
//@{
# define RED_TEST(expr) RED_TEST_CHECK(expr)
# define RED_TEST_CHECK(expr) RED_CHECK(expr)
# define RED_TEST_CONTEXT(ostream_expr) if (::redemption_unit_test__::Stream{} << ostream_expr)

# define RED_TEST_INVOKER(fname) fname
# define RED_TEST_FUNC_CTX(fname) ([](auto&&... xs) { \
    return fname(static_cast<decltype(xs)&&>(xs)...); })
# define RED_TEST_FUNC(fname, ...) ::redemption_unit_test__::X(bool(fname __VA_ARGS__))
# define RED_REQUIRE_FUNC(fname, ...) ::redemption_unit_test__::X(bool(fname __VA_ARGS__))

# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, id) do { stmt; (void)id; } while (0)
# define RED_CHECK_NO_THROW(stmt) do { stmt; } while (0)
# define RED_CHECK_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_CHECK_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_CHECK_EQUAL(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
# define RED_CHECK_NE(a, b) ::redemption_unit_test__::X(bool((a) != (b)))
# define RED_CHECK_LT(a, b) ::redemption_unit_test__::X(bool((a) < (b)))
# define RED_CHECK_LE(a, b) ::redemption_unit_test__::X(bool((a) <= (b)))
# define RED_CHECK_GT(a, b) ::redemption_unit_test__::X(bool((a) > (b)))
# define RED_CHECK_GE(a, b) ::redemption_unit_test__::X(bool((a) >= (b)))
# define RED_CHECK(a) ::redemption_unit_test__::X(bool(a))
# define RED_CHECK_MESSAGE(a, ostream_expr) ::redemption_unit_test__::X(bool(a)), \
    ::redemption_unit_test__::Stream{} << ostream_expr
# define RED_CHECK_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    ::redemption_unit_test__::X(first1 == last1 && first2 == last2)
# define RED_CHECK_EQUAL_RANGES(a, b) ::redemption_unit_test__::X((void(a), void(b), true))
# define RED_CHECK_PREDICATE(pred, arg_list) pred arg_list
# define RED_CHECK_MEM(mem, memref) void(mem), void(memref)
# define RED_CHECK_SMEM(mem, memref) void(mem), void(memref)
# define RED_CHECK_BMEM(mem, memref) void(mem), void(memref)
# define RED_CHECK_HMEM(mem, memref) void(mem), void(memref)
# define RED_CHECK_CMEM(mem, memref) void(mem), void(memref)
//@}

/// REQUIRE
//@{
# define RED_TEST_REQUIRE(expr) RED_REQUIRE(expr)
# define RED_REQUIRE_EXCEPTION_ERROR_ID(stmt, id) do { stmt; (void)id; } while (0)
# define RED_REQUIRE_NO_THROW(stmt) do { stmt; } while (0)
# define RED_REQUIRE_THROW(stmt, exception) do { stmt; [](exception) {}; } while (0)
# define RED_REQUIRE_EXCEPTION(stmt, exception, predicate) do { \
    stmt; [](exception & e) { predicate(e); }; } while (0)
# define RED_REQUIRE_EQUAL(a, b) ::redemption_unit_test__::X(bool((a) == (b)))
# define RED_REQUIRE_NE(a, b) ::redemption_unit_test__::X(bool((a) != (b)))
# define RED_REQUIRE_LT(a, b) ::redemption_unit_test__::X(bool((a) < (b)))
# define RED_REQUIRE_LE(a, b) ::redemption_unit_test__::X(bool((a) <= (b)))
# define RED_REQUIRE_GT(a, b) ::redemption_unit_test__::X(bool((a) > (b)))
# define RED_REQUIRE_GE(a, b) ::redemption_unit_test__::X(bool((a) >= (b)))
# define RED_REQUIRE(a) ::redemption_unit_test__::X(bool(a))
# define RED_REQUIRE_MESSAGE(a, ostream_expr) ::redemption_unit_test__::X(bool(a)), \
    ::redemption_unit_test__::Stream{} << ostream_expr
# define RED_REQUIRE_EQUAL_COLLECTIONS(first1, last1, first2, last2) \
    ::redemption_unit_test__::X(first1 == last1 && first2 == last2)
# define RED_REQUIRE_EQUAL_RANGES(a, b) ::redemption_unit_test__::X((void(a), void(b), true))
# define RED_REQUIRE_PREDICATE(pred, arg_list) pred arg_list
# define RED_REQUIRE_MEM(mem, memref) void(mem), void(memref)
# define RED_REQUIRE_SMEM(mem, memref) void(mem), void(memref)
# define RED_REQUIRE_RMEM(mem, memref) void(mem), void(memref)
# define RED_REQUIRE_HMEM(mem, memref) void(mem), void(memref)
# define RED_REQUIRE_CMEM(mem, memref) void(mem), void(memref)
//@}

/// WARN
//@{
# define RED_TEST_WARN(expr) RED_CHECK(expr)
//@}

#else

# include "impl/redemption_unit_tests_impl.hpp"

# define RED_CHECK_MEM(mem, memref) RED_TEST_MEM(CHECK, mem, memref, 'a')
# define RED_CHECK_SMEM(mem, memref) RED_TEST_MEM(CHECK, mem, memref, 'S')
# define RED_CHECK_BMEM(mem, memref) RED_TEST_MEM(CHECK, mem, memref, 'b')
# define RED_CHECK_HMEM(mem, memref) RED_TEST_MEM(CHECK, mem, memref, 'd')
# define RED_CHECK_CMEM(mem, memref) RED_TEST_MEM(CHECK, mem, memref, 'C')

# define RED_REQUIRE_MEM(mem, memref) RED_TEST_MEM(REQUIRE, mem, memref, 'a')
# define RED_REQUIRE_SMEM(mem, memref) RED_TEST_MEM(REQUIRE, mem, memref, 'S')
# define RED_REQUIRE_RMEM(mem, memref) RED_TEST_MEM(REQUIRE, mem, memref, 'b')
# define RED_REQUIRE_HMEM(mem, memref) RED_TEST_MEM(REQUIRE, mem, memref, 'd')
# define RED_REQUIRE_CMEM(mem, memref) RED_TEST_MEM(REQUIRE, mem, memref, 'C')

/// CHECK
//@{
# define RED_CHECK_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_TEST_EXCEPTION_ERROR_ID(CHECK, stmt, ErrId)

# define RED_CHECK_EQUAL_RANGES(a, b) \
    RED_TEST_EQUAL_RANGES(CHECK, a, b)
//@}

/// REQUIRE
//@{
# define RED_REQUIRE_EXCEPTION_ERROR_ID(stmt, ErrId) \
    RED_TEST_EXCEPTION_ERROR_ID(REQUIRE, stmt, ErrId)

# define RED_REQUIRE_EQUAL_RANGES(a, b) \
    RED_TEST_EQUAL_RANGES(REQUIRE, a, b)
//@}

# define RED_TEST_STRING_CHECK "check"
# define RED_TEST_STRING_REQUIRE "require"

# define RED_TEST_EXCEPTION_ERROR_ID(lvl, stmt, ErrId) \
    RED_##lvl##_EXCEPTION(                             \
        stmt, Error,                                   \
        [&](Error const & e__) {                       \
            RED_CHECK_EQUAL(e__.id, ErrId);            \
            return (e__.id == (ErrId));                \
        }                                              \
    )

# define RED_TEST_EQUAL_RANGES(lvl, a, b)     \
    [](auto const & a__, auto const & b__) {  \
        using std::begin;                     \
        using std::end;                       \
        RED_##lvl##_EQUAL_COLLECTIONS(        \
            (void(#a), begin(a__)), end(a__), \
            (void(#b), begin(b__)), end(b__)  \
        );                                    \
    }(a, b)

# define RED_TEST_MEM(lvl, mem, memref, c)                \
    [](bytes_view x_mem__, bytes_view x_memref__){        \
        size_t pos__ = 0;                                 \
        RED_##lvl##_MESSAGE(                              \
            ::redemption_unit_test__::compare_bytes(      \
                pos__, x_mem__, x_memref__),              \
            RED_TEST_STRING_##lvl " "                     \
            #mem " == " #memref " has failed "            \
            << (::redemption_unit_test__::Put2Mem{        \
                pos__, x_mem__, x_memref__, c, " != "})); \
    }(mem, memref)


namespace redemption_unit_test__
{
    struct Enum
    {
        template<class E, class = std::enable_if_t<std::is_enum<E>::value>>
        Enum(E e) noexcept
        : name(get_type_name(__PRETTY_FUNCTION__))
        , x(static_cast<long long>(e))
        , is_signed(std::is_signed_v<std::underlying_type_t<E>>)
        {}

        template<std::size_t N>
        static array_view_const_char get_type_name(char const(&s)[N]) noexcept
        {
            return get_type_name(s, N-1);
        }

        static array_view_const_char get_type_name(char const* s, std::size_t n) noexcept;

        array_view_const_char name;
        long long x;
        bool is_signed;
    };

    struct BytesView
    {
        BytesView(bytes_view bytes) noexcept : bytes(bytes) {}
        BytesView(writable_bytes_view bytes) noexcept : bytes(bytes) {}
        BytesView(array_view_const_char bytes) noexcept : bytes(bytes) {}
        // BytesView(array_view_const_s8 bytes) noexcept : bytes(bytes) {}
        BytesView(array_view_const_u8 bytes) noexcept : bytes(bytes) {}

        bytes_view bytes;
    };

} // namespace redemption_unit_test__

#if !defined(REDEMPTION_UNIT_TEST_FAST_CHECK) || REDEMPTION_UNIT_TEST_FAST_CHECK != 1

namespace std
{
    // hack hack hack :D
    std::ostream& operator<<(std::ostream& out, ::redemption_unit_test__::Enum const& e);
    std::ostream& operator<<(std::ostream& out, ::redemption_unit_test__::BytesView const& v);
}

#endif

#endif

#define RED_TEST_DELEGATE_PRINT(type, stream_expr)          \
  template<>                                                \
  struct RED_TEST_PRINT_TYPE_STRUCT_NAME<type>              \
  {                                                         \
    void operator()(std::ostream& out,type const & x) const \
    {                                                       \
      out << stream_expr; /* NOLINT */                      \
    }                                                       \
  }

#define RED_TEST_DELEGATE_PRINT_ENUM(type) \
  RED_TEST_DELEGATE_PRINT(type,            \
    #type << "{" << +::std::underlying_type_t<type>(x) << "}")


#define RED_TEST_CONTEXT_DATA(type_value, iocontext, ...) \
    for (type_value : __VA_ARGS__)                        \
        RED_TEST_CONTEXT(iocontext) /*NOLINT*/

namespace redemption_unit_test__
{

unsigned long current_count_error();

struct int_variation
{
    int left;
    int right;
    int value;
    int variant;
    bool is_percent;
};

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator==(T const& x, int_variation const& variation) noexcept
{
    return variation.left <= x && x <= variation.right;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator!=(T const& x, int_variation const& variation) noexcept
{
    return !(x == variation);
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator<(T const& x, int_variation const& variation) noexcept
{
    return x < variation.right;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator>(T const& x, int_variation const& variation) noexcept
{
    return x > variation.left;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator<=(T const& x, int_variation const& variation) noexcept
{
    return x <= variation.right;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator>=(T const& x, int_variation const& variation) noexcept
{
    return x >= variation.left;
}


template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator==(int_variation const& variation, T const& x) noexcept
{
    return x == variation;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator!=(int_variation const& variation, T const& x) noexcept
{
    return !(x == variation);
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator<(int_variation const& variation, T const& x) noexcept
{
    return x >= variation;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator>(int_variation const& variation, T const& x) noexcept
{
    return x <= variation;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator<=(int_variation const& variation, T const& x) noexcept
{
    return x > variation;
}

template<class T>
std::enable_if_t<std::is_integral_v<T>, bool>
operator>=(int_variation const& variation, T const& x) noexcept
{
    return x < variation;
}

struct def_variation1
{
    int variantion;
    bool is_percent;
};

struct def_variation2
{
    int variantion;
    bool is_percent;
};

inline def_variation2 operator-(def_variation1 const& variation) noexcept
{
    return {variation.variantion, variation.is_percent};
}

template<class T>
int_variation operator+(T const& x_, def_variation2 const& variation) noexcept
{
    const int x = x_;
    if (variation.is_percent) {
        auto a = x * variation.variantion / 100;
        return {x - a, x + a, x, variation.variantion, true};
    }
    return {x - variation.variantion, x + variation.variantion, x, variation.variantion, false};
}

namespace literals
{
    inline def_variation1 operator""_percent(unsigned long long x) noexcept { return {int(x), true}; }
    inline def_variation1 operator""_v(unsigned long long x) noexcept { return {int(x), false}; }
}

} // namespace redemption_unit_test__

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wheader-hygiene")
using namespace redemption_unit_test__::literals; // NOLINT
REDEMPTION_DIAGNOSTIC_POP

template<>
struct RED_TEST_PRINT_TYPE_STRUCT_NAME<redemption_unit_test__::int_variation>
{
    void operator()(std::ostream& out, redemption_unit_test__::int_variation const & x) const;
};

/// CHECK
//@{
#define RED_CHECK_EQ RED_CHECK_EQUAL
#define RED_CHECK_EQ_RANGES RED_CHECK_EQUAL_RANGES

#define RED_CHECK_MEM_AA(mem, memref) \
    RED_CHECK_MEM(make_array_view(mem), make_array_view(memref))

#define RED_CHECK_SMEM_AA(mem, memref) \
    RED_CHECK_SMEM(make_array_view(mem), make_array_view(memref))

#define RED_CHECK_BMEM_AA(mem, memref) \
    RED_CHECK_BMEM(make_array_view(mem), make_array_view(memref))
//@}

/// REQUIRE
//@{
#define RED_REQUIRE_EQ RED_REQUIRE_EQUAL
#define RED_REQUIRE_EQ_RANGES RED_REQUIRE_EQUAL_RANGES

#define RED_REQUIRE_MEM_AA(mem, memref) \
    RED_REQUIRE_MEM(make_array_view(mem), make_array_view(memref))

#define RED_REQUIRE_SMEM_AA(mem, memref) \
    RED_REQUIRE_SMEM(make_array_view(mem), make_array_view(memref))

#define RED_REQUIRE_RMEM_AA(mem, memref) \
    RED_REQUIRE_SMEM(make_array_view(mem), make_array_view(memref))
//@}

#define RED_ERROR_COUNT redemption_unit_test__::current_count_error()
