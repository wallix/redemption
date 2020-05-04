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

#define BOOST_TEST_NO_OLD_TOOLS

#ifdef RED_TEST_MODULE
# define BOOST_TEST_MODULE RED_TEST_MODULE
#endif

#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#include "cxx/diagnostic.hpp"
#include "cxx/compiler_version.hpp"

// FIXME global warning deactivation, should be in each macro
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(9, 0, 0)
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wused-but-marked-unused")
#endif

#define RED_AUTO_TEST_CASE(...) BOOST_AUTO_TEST_CASE(__VA_ARGS__) /*NOLINT*/

#define RED_FAIL(...) BOOST_FAIL(__VA_ARGS__) /*NOLINT*/
#define RED_ERROR(...) BOOST_ERROR(__VA_ARGS__) /*NOLINT*/

#define RED_TEST_INFO BOOST_TEST_INFO
#define RED_TEST_DONT_PRINT_LOG_VALUE BOOST_TEST_DONT_PRINT_LOG_VALUE
#define RED_TEST_PRINT_TYPE_FUNCTION_NAME boost_test_print_type
#define RED_TEST_PRINT_TYPE_STRUCT_NAME boost::test_tools::tt_detail::print_log_value

#if !defined(REDEMPTION_UNIT_TEST_FAST_CHECK)
# define REDEMPTION_UNIT_TEST_FAST_CHECK 0
#endif

#if REDEMPTION_UNIT_TEST_FAST_CHECK
//@{
#  define RED_TEST_CHECKPOINT(...)
#  define RED_TEST_MESSAGE(...)
#  define RED_TEST_PASSPOINT(...)
#  define RED_TEST_CONTEXT(...)

#  define RED_TEST(...) BOOST_TEST((__VA_ARGS__)) /*NOLINT*/
#  define RED_TEST_WARN(...) BOOST_TEST_WARN((__VA_ARGS__)) /*NOLINT*/
#  define RED_TEST_CHECK(...) BOOST_TEST_CHECK((__VA_ARGS__)) /*NOLINT*/
#  define RED_TEST_REQUIRE(...) BOOST_TEST_REQUIRE((__VA_ARGS__)) /*NOLINT*/

#  define RED_CHECK_NO_THROW(...) BOOST_CHECK_NO_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_THROW(...) BOOST_CHECK_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_EXCEPTION(...) BOOST_CHECK_EXCEPTION(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_EQUAL(a, b) BOOST_CHECK(((a) == (b))) /*NOLINT*/
#  define RED_CHECK_NE(a, b) BOOST_CHECK(((a) != (b))) /*NOLINT*/
#  define RED_CHECK_LT(a, b) BOOST_CHECK(((a) < (b))) /*NOLINT*/
#  define RED_CHECK_LE(a, b) BOOST_CHECK(((a) <= (b))) /*NOLINT*/
#  define RED_CHECK_GT(a, b) BOOST_CHECK(((a) > (b))) /*NOLINT*/
#  define RED_CHECK_GE(a, b) BOOST_CHECK(((a) >= (b))) /*NOLINT*/
#  define RED_CHECK(...) BOOST_CHECK((__VA_ARGS__)) /*NOLINT*/
#  define RED_CHECK_MESSAGE(x, ...) BOOST_CHECK_MESSAGE((x), __VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_EQUAL_COLLECTIONS(...) BOOST_CHECK(std::equal(__VA_ARGS__)) /*NOLINT*/
#  define RED_CHECK_PREDICATE(...) BOOST_CHECK_PREDICATE(__VA_ARGS__) /*NOLINT*/

#  define RED_REQUIRE_NO_THROW(...) BOOST_REQUIRE_NO_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_THROW(...) BOOST_REQUIRE_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_EXCEPTION(...) BOOST_REQUIRE_EXCEPTION(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_EQUAL(a, b) BOOST_REQUIRE(((a) == (b))) /*NOLINT*/
#  define RED_REQUIRE_NE(a, b) BOOST_REQUIRE(((a) != (b))) /*NOLINT*/
#  define RED_REQUIRE_LT(a, b) BOOST_REQUIRE(((a) < (b))) /*NOLINT*/
#  define RED_REQUIRE_LE(a, b) BOOST_REQUIRE(((a) <= (b))) /*NOLINT*/
#  define RED_REQUIRE_GT(a, b) BOOST_REQUIRE(((a) > (b))) /*NOLINT*/
#  define RED_REQUIRE_GE(a, b) BOOST_REQUIRE(((a) >= (b))) /*NOLINT*/
#  define RED_REQUIRE(...) BOOST_REQUIRE((__VA_ARGS__)) /*NOLINT*/
#  define RED_REQUIRE_MESSAGE(x, ...) BOOST_REQUIRE_MESSAGE((x), __VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_EQUAL_COLLECTIONS(...) BOOST_REQUIRE(std::equal(__VA_ARGS__)) /*NOLINT*/
#  define RED_REQUIRE_PREDICATE(...) BOOST_REQUIRE_PREDICATE(__VA_ARGS__) /*NOLINT*/
//@}
#else
//@{
#  define RED_TEST_CHECKPOINT(...) BOOST_TEST_CHECKPOINT(__VA_ARGS__) /*NOLINT*/
#  define RED_TEST_MESSAGE(...) BOOST_TEST_MESSAGE(__VA_ARGS__) /*NOLINT*/
#  define RED_TEST_PASSPOINT(...) BOOST_TEST_PASSPOINT(__VA_ARGS__) /*NOLINT*/

#  define RED_TEST_CONTEXT(...) BOOST_TEST_CONTEXT(__VA_ARGS__) /*NOLINT*/

#  define RED_TEST(...) BOOST_TEST(__VA_ARGS__) /*NOLINT*/
#  define RED_TEST_WARN(...) BOOST_TEST_WARN(__VA_ARGS__) /*NOLINT*/
#  define RED_TEST_CHECK(...) BOOST_TEST_CHECK(__VA_ARGS__) /*NOLINT*/
#  define RED_TEST_REQUIRE(...) BOOST_TEST_REQUIRE(__VA_ARGS__) /*NOLINT*/

#  define RED_CHECK_NO_THROW(...) BOOST_CHECK_NO_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_THROW(...) BOOST_CHECK_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_EXCEPTION(...) BOOST_CHECK_EXCEPTION(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_EQUAL(a, b) BOOST_CHECK((a) == (b)) /*NOLINT*/
#  define RED_CHECK_NE(a, b) BOOST_CHECK((a) != (b)) /*NOLINT*/
#  define RED_CHECK_LT(a, b) BOOST_CHECK((a) < (b)) /*NOLINT*/
#  define RED_CHECK_LE(a, b) BOOST_CHECK((a) >= (b)) /*NOLINT*/
#  define RED_CHECK_GT(a, b) BOOST_CHECK((a) > (b)) /*NOLINT*/
#  define RED_CHECK_GE(a, b) BOOST_CHECK((a) >= (b)) /*NOLINT*/
#  define RED_CHECK(...) BOOST_CHECK(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_MESSAGE(...) BOOST_CHECK_MESSAGE(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_EQUAL_COLLECTIONS(...) BOOST_CHECK_EQUAL_COLLECTIONS(__VA_ARGS__) /*NOLINT*/
#  define RED_CHECK_PREDICATE(...) BOOST_CHECK_PREDICATE(__VA_ARGS__) /*NOLINT*/


#  define RED_REQUIRE_NO_THROW(...) BOOST_REQUIRE_NO_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_THROW(...) BOOST_REQUIRE_THROW(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_EXCEPTION(...) BOOST_REQUIRE_EXCEPTION(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_EQUAL(a, b) BOOST_REQUIRE((a) == (b)) /*NOLINT*/
#  define RED_REQUIRE_NE(a, b) BOOST_REQUIRE((a) != (b)) /*NOLINT*/
#  define RED_REQUIRE_LT(a, b) BOOST_REQUIRE((a) < (b)) /*NOLINT*/
#  define RED_REQUIRE_LE(a, b) BOOST_REQUIRE((a) <= (b)) /*NOLINT*/
#  define RED_REQUIRE_GT(a, b) BOOST_REQUIRE((a) > (b)) /*NOLINT*/
#  define RED_REQUIRE_GE(a, b) BOOST_REQUIRE((a) >= (b)) /*NOLINT*/
#  define RED_REQUIRE(...) BOOST_REQUIRE(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_MESSAGE(...) BOOST_REQUIRE_MESSAGE(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_EQUAL_COLLECTIONS(...) BOOST_REQUIRE_EQUAL_COLLECTIONS(__VA_ARGS__) /*NOLINT*/
#  define RED_REQUIRE_PREDICATE(...) BOOST_REQUIRE_PREDICATE(__VA_ARGS__) /*NOLINT*/
//@}
#endif

#if BOOST_VERSION < 106300
namespace boost {
namespace test_tools {
namespace tt_detail {

template<>
struct BOOST_TEST_DECL print_log_value<decltype(nullptr)> {
    void operator()(std::ostream& ostr, decltype(nullptr)) { ostr<<"nullptr"; }
};

}}}
#endif


namespace redemption_unit_test__
{

template<class T, class Fd>
struct fn_ctx_t
{
    char const* name;
    T x;
    Fd g;

    operator T const& () const noexcept { return x; }
    operator T& () noexcept { return x; }
};

#define MK_OP(op)                                                         \
    template<class T, class Fd, class U>                                  \
    bool operator op (fn_ctx_t<T, Fd> const& fctx, U const& x)            \
    {                                                                     \
        return fctx.x op x;                                               \
    }                                                                     \
    template<class T, class Fd, class U>                                  \
    bool operator op (fn_ctx_t<T, Fd> const&,                             \
        boost::test_tools::assertion::value_expr<U> const&) = delete;     \
    template<class U, class T, class Fd>                                  \
    bool operator op (U const& x, fn_ctx_t<T, Fd> const& fctx)            \
    {                                                                     \
        return x op fctx.x;                                               \
    }                                                                     \
    template<class U, class T, class Fd>                                  \
    bool operator op (boost::test_tools::assertion::value_expr<U> const&, \
        fn_ctx_t<T, Fd> const&) = delete

MK_OP(==);
MK_OP(!=);
MK_OP(<);
MK_OP(<=);
MK_OP(>);
MK_OP(>=);

#undef MK_OP

template<class X, class... Xs>
auto fn_ctx_arg(X const& x, Xs const&... xs)
{
    return [&](std::ostream& out){
        out << x;
        ((out << ", " << boost::test_tools::tt_detail::print_helper(xs)), ...);
    };
}

template<class F, class... Ts>
auto fn_ctx(char const* name, F f, Ts&&... xs)
{
    auto print = fn_ctx_arg(xs...);
    return fn_ctx_t<decltype(f(static_cast<Ts&&>(xs)...)), decltype(print)>{
        name, f(static_cast<Ts&&>(xs)...), print
    };
}

template<class Fr, class Fd>
std::ostream& operator<<(std::ostream& out, fn_ctx_t<Fr, Fd> const& fctx)
{
    out << fctx.name << "(";
    fctx.g(out);
    out << ")= " << fctx.x;
    return out;
}

template<class F>
struct fn_invoker_t
{
    char const* name;
    F f;

    template<class... Xs>
    decltype(auto) operator()(Xs&&... xs) const
    {
        auto print = fn_ctx_arg(xs...);
        return fn_ctx_t<decltype(f(static_cast<Xs&&>(xs)...)), decltype(print)>{
            name, f(static_cast<Xs&&>(xs)...), print
        };
    }

    template<class... Xs>
    decltype(auto) operator()(Xs&&... xs)
    {
        auto print = fn_ctx_arg(xs...);
        return fn_ctx_t<decltype(f(static_cast<Xs&&>(xs)...)), decltype(print)>{
            name, f(static_cast<Xs&&>(xs)...), print
        };
    }
};

template<class F>
constexpr fn_invoker_t<F> fn_invoker(char const* name, F f)
{
    return {name, f};
}

} // namespace redemption_unit_test__

#define RED_TEST_FUNC_CTX(fname) ::redemption_unit_test__::fn_invoker( \
    #fname, [](auto&&... args){ return fname(args...); })

#define RED_TEST_INVOKER(fname) ::redemption_unit_test__::fn_invoker( \
    #fname, [&](auto&&... args){ return fname(args...); })

#define RED_TEST_FUNC(fname, ...) [&]{                             \
    auto BOOST_PP_CAT(fctx__, __LINE__) = RED_TEST_INVOKER(fname); \
    RED_TEST(BOOST_PP_CAT(fctx__, __LINE__)__VA_ARGS__); }()

#define RED_REQUIRE_FUNC(fname, ...) [&]{                          \
    auto BOOST_PP_CAT(fctx__, __LINE__) = RED_TEST_INVOKER(fname); \
    RED_REQUIRE(BOOST_PP_CAT(fctx__, __LINE__)__VA_ARGS__); }()


//
// COLLECTIONS
//

#include "utils/sugar/bytes_view.hpp"
namespace ut
{
    struct flagged_bytes_view;
}

namespace redemption_unit_test__
{
    struct Put2Mem
    {
        size_t & pos;
        bytes_view lhs;
        bytes_view rhs;
        char const* revert;
        std::size_t min_len;
        char pattern;

        friend std::ostream & operator<<(std::ostream & out, Put2Mem const & x);
    };

    boost::test_tools::assertion_result bytes_EQ(bytes_view a, bytes_view b, char pattern, std::size_t min_len);
    boost::test_tools::assertion_result bytes_NE(bytes_view a, bytes_view b, char pattern, std::size_t min_len);
    boost::test_tools::assertion_result bytes_LT(bytes_view a, bytes_view b, char pattern, std::size_t min_len);
    boost::test_tools::assertion_result bytes_LE(bytes_view a, bytes_view b, char pattern, std::size_t min_len);
    boost::test_tools::assertion_result bytes_GT(bytes_view a, bytes_view b, char pattern, std::size_t min_len);
    boost::test_tools::assertion_result bytes_GE(bytes_view a, bytes_view b, char pattern, std::size_t min_len);


    template<class T> struct is_bytes_view : std::false_type {};
    template<> struct is_bytes_view<bytes_view> : std::true_type {};
    template<> struct is_bytes_view<writable_bytes_view> : std::true_type {};

    template<class T> struct is_array_view : std::false_type {};
    template<class T> struct is_array_view<array_view<T>>
    : std::integral_constant<bool, !is_bytes_view<array_view<T>>::value>
    {};
    template<> struct is_array_view<::ut::flagged_bytes_view> : std::true_type {};

    template<class T, class U>
    struct is_array_view_comparable : std::integral_constant<bool,
        (is_array_view<T>::value || is_array_view<U>::value)
        && not (is_bytes_view<T>::value || is_bytes_view<U>::value)
    >
    {};

    template<class T, class U, bool, bool>
    struct is_bytes_comparable_impl : std::false_type {};

    template<class T, class U>
    struct is_bytes_comparable_impl<T, U, true, true>
    : std::true_type
    {};

    template<class T, class U>
    struct is_bytes_comparable_impl<T, U, false, true>
    : std::is_convertible<T, bytes_view>
    {};

    template<class T, class U>
    struct is_bytes_comparable_impl<T, U, true, false>
    : std::is_convertible<U, bytes_view>
    {};


    template<class T, class U>
    struct is_bytes_comparable
    : is_bytes_comparable_impl<T, U, is_bytes_view<T>::value, is_bytes_view<U>::value>
    {};

    // boost::unit_test::is_forward_iterable<array_view<T>> -> false (see bellow)
    template<class T>
    struct View : array_view<T const>
    {
        using array_view<T const>::array_view;

        View(array_view<T const> v) noexcept : array_view<T const>(v) {}
    };

#if REDEMPTION_UNIT_TEST_FAST_CHECK
    template<class T, class U>
    struct is_view_comparable
    : std::integral_constant<bool,
        is_bytes_comparable<T, U>::value
     || is_array_view_comparable<T, U>::value>
    {};

    template<class Op, class T, class U>
    bool av_equal(Op op, T const& x,  U const& y)
    {
        using std::begin;
        using std::end;
        if constexpr (is_bytes_view<T>::value || is_bytes_view<U>::value
         || (std::is_convertible_v<T, bytes_view> && std::is_convertible_v<U, bytes_view>))
        {
            bytes_view a = x;
            bytes_view b = y;
            return std::equal(a.begin(), a.end(), b.begin(), b.end(), op);
        }
        else if constexpr (is_array_view<T>::value)
        {
            T v{x};
            return std::equal(begin(v), end(v), begin(y), end(y), op);
        }
        else
        {
            U v{y};
            return std::equal(begin(x), end(x), begin(v), end(v), op);
        }
    }

    namespace ops
    {
        template<class T, class U>
        std::enable_if_t<is_view_comparable<T, U>::value, bool>
        operator == (T const& x,  U const& y)
        {
            return av_equal(std::equal_to{}, x, y);
        }

        template<class T, class U>
        std::enable_if_t<is_view_comparable<T, U>::value, bool>
        operator != (T const& x,  U const& y)
        {
            return !av_equal(std::equal_to{}, x, y);
        }

        template<class T, class U>
        std::enable_if_t<is_view_comparable<T, U>::value, bool>
        operator < (T const& x,  U const& y)
        {
            return av_equal(std::less{}, x, y);
        }

        template<class T, class U>
        std::enable_if_t<is_view_comparable<T, U>::value, bool>
        operator >= (T const& x,  U const& y)
        {
            return !av_equal(std::less{}, x, y);
        }

        template<class T, class U>
        std::enable_if_t<is_view_comparable<T, U>::value, bool>
        operator > (T const& x,  U const& y)
        {
            return av_equal(std::less{}, y, x);
        }

        template<class T, class U>
        std::enable_if_t<is_view_comparable<T, U>::value, bool>
        operator <= (T const& x,  U const& y)
        {
            return !av_equal(std::less{}, y, x);
        }
    }
#endif
} // namespace redemption_unit_test__

#if REDEMPTION_UNIT_TEST_FAST_CHECK
using redemption_unit_test__::ops::operator ==;
using redemption_unit_test__::ops::operator !=;
using redemption_unit_test__::ops::operator <;
using redemption_unit_test__::ops::operator <=;
using redemption_unit_test__::ops::operator >;
using redemption_unit_test__::ops::operator >=;
#endif

namespace boost {

namespace unit_test {
    // disable collection_compare for array_view like type
    // but this disable also test_tools::per_element() and test_tools::lexicographic()
    // BOOST_TEST(a == b, tt::per_element())
    template<class T> struct is_forward_iterable<array_view<T>> : mpl::false_ {};
    template<> struct is_forward_iterable<bytes_view> : mpl::false_ {};
    template<> struct is_forward_iterable<writable_bytes_view> : mpl::false_ {};
}

namespace test_tools {
namespace assertion {
namespace op {

// BOOST_TEST_FOR_EACH_COMP_OP(action)
// action( oper, name, rev )
#define DEFINE_COLLECTION_COMPARISON(oper, name, rev)                     \
template<class T, class U>                                                \
struct name<T, U, std::enable_if_t<                                       \
    ::redemption_unit_test__::is_bytes_comparable<T, U>::value>>          \
{                                                                         \
    using result_type = assertion_result;                                 \
                                                                          \
    static assertion_result                                               \
    eval( bytes_view lhs, bytes_view rhs )                                \
    {                                                                     \
        return ::redemption_unit_test__::bytes_##name(lhs, rhs, 'a', 0);  \
    }                                                                     \
                                                                          \
    template<class PrevExprType>                                          \
    static void                                                           \
    report( std::ostream&,                                                \
            PrevExprType const&,                                          \
            bytes_view const&)                                            \
    {}                                                                    \
                                                                          \
    static char const* revert()                                           \
    { return " " #rev " "; }                                              \
};                                                                        \
                                                                          \
template<class T, class U>                                                \
struct name<T, U, std::enable_if_t<                                       \
    ::redemption_unit_test__::is_array_view_comparable<T, U>::value>>     \
{                                                                         \
    using result_type = assertion_result;                                 \
    using OP = name<T, U>;                                                \
    using L = typename T::value_type;                                     \
    using R = typename U::value_type;                                     \
    using elem_op = op::name<L, R>;                                       \
                                                                          \
    static assertion_result                                               \
    eval( T const& lhs, U const& rhs )                                    \
    {                                                                     \
        if constexpr (std::is_convertible_v<T, bytes_view>                \
                   && std::is_convertible_v<U, bytes_view>)               \
        {                                                                 \
            char flag = 'a';                                              \
            std::size_t min_len = 0;                                      \
            if constexpr (std::is_same_v<T, ::ut::flagged_bytes_view>) {  \
               flag = char(lhs.pattern);                                  \
               min_len = std::max(min_len, lhs.min_len);                  \
            }                                                             \
            if constexpr (std::is_same_v<U, ::ut::flagged_bytes_view>) {  \
               flag = char(rhs.pattern);                                  \
               min_len = std::max(min_len, rhs.min_len);                  \
            }                                                             \
            return ::redemption_unit_test__                               \
                ::bytes_##name(lhs, rhs, flag, min_len);                  \
        }                                                                 \
        else                                                              \
        {                                                                 \
            return boost::test_tools::assertion::op::compare_collections( \
                redemption_unit_test__::View<L const>{lhs},               \
                redemption_unit_test__::View<R const>{rhs},               \
                static_cast<boost::type<elem_op>*>(nullptr),              \
                mpl::true_());                                            \
        }                                                                 \
    }                                                                     \
                                                                          \
    template<class PrevExprType>                                          \
    static void                                                           \
    report( std::ostream&,                                                \
            PrevExprType const&,                                          \
            U const&)                                                     \
    {}                                                                    \
                                                                          \
    static char const* revert()                                           \
    { return " " #rev " "; }                                              \
};

BOOST_TEST_FOR_EACH_COMP_OP(DEFINE_COLLECTION_COMPARISON)
#undef DEFINE_COLLECTION_COMPARISON

} // namespace op
} // namespace assertion
} // namespace test_tools
} // namespace boost
