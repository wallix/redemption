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
#ifdef IN_IDE_PARSER
    struct flagged_bytes_view;
    enum class PatternView : char;
#endif

    using assertion_result = boost::test_tools::assertion_result;

    namespace detail
    {
        void print_hex_int_compare(
            std::ostream& out, uint64_t x, uint64_t y, int ndigit, char const* rev_op);
    }

    namespace op
    {
        template<class OP>
        struct op_to_predicate;

        #define DEC_OP_PREDICATE(name, OP)                                       \
            template<class T, class U>                                           \
            struct op_to_predicate<boost::test_tools::assertion::op::name<T, U>> \
            {                                                                    \
                static bool compute(T const& x, U const& y) { return x OP y; }   \
            }

        DEC_OP_PREDICATE(EQ, ==);
        DEC_OP_PREDICATE(NE, !=);
        DEC_OP_PREDICATE(LT, <);
        DEC_OP_PREDICATE(LE, <=);
        DEC_OP_PREDICATE(GT, >);
        DEC_OP_PREDICATE(GE, >=);
        #undef DEC_OP_PREDICATE
    }

    namespace detail
    {
        struct lazy_ostream_create_decorator_result_59
        {
            using result_type = int;

            static result_type result(::boost::unit_test::lazy_ostream const& /*out*/)
            {
                return 0;
            }
        };

        struct lazy_ostream_create_decorator_result_73
        {
            using result_type = ::boost::unit_test::lazy_ostream const&;

            static result_type result(::boost::unit_test::lazy_ostream const& out)
            {
                return out;
            }
        };

        using lazy_ostream_create_decorator_result = std::conditional_t<
            std::is_same_v<int, decltype(
                *static_cast<::boost::unit_test::lazy_ostream const *>(nullptr)
                << ::boost::test_tools::bitwise()
            )>
          , lazy_ostream_create_decorator_result_59
          , lazy_ostream_create_decorator_result_73
        >;
    }

#define RED_TEST_CREATE_DECORATOR(name, f)                                         \
    template<class T, class U, typename OP>                                        \
    inline assertion_result operator<<(                                            \
        ::boost::test_tools::tt_detail::assertion_evaluate_t<                      \
            ::boost::test_tools::assertion::binary_expr<T,U,OP>> const& ae,        \
        name const& decorator)                                                     \
    {                                                                              \
        return f(ae.m_e.lhs().value(), ae.m_e.rhs(), decorator,                    \
                 static_cast<OP*>(nullptr));                                       \
    }                                                                              \
                                                                                   \
    inline ::ut::detail::lazy_ostream_create_decorator_result::result_type         \
    operator<<(::boost::unit_test::lazy_ostream const& out, name const&)           \
    {                                                                              \
        return ::ut::detail::lazy_ostream_create_decorator_result::result(out);    \
    }                                                                              \
                                                                                   \
    inline ::boost::test_tools::tt_detail::check_type                              \
    operator<<(::boost::test_tools::tt_detail::assertion_type const&, name const&) \
    {                                                                              \
        return ::boost::test_tools::tt_detail::CHECK_BUILT_ASSERTION;              \
    }

    namespace detail
    {
        template<class OP>
        inline assertion_result
        hex_int_compare(uint64_t x, uint64_t y, hex_int h, OP const*)
        {
            bool const r = op::op_to_predicate<OP>::compute(x, y);
            assertion_result ar(r);

            if (!r) {
                detail::print_hex_int_compare(ar.message().stream(), x, y, h.ndigit, OP::revert());
            }

            return ar;
        }
    }
    RED_TEST_CREATE_DECORATOR(hex_int, detail::hex_int_compare)
}

namespace redemption_unit_test__
{
    struct Put2Mem
    {
        size_t & pos;
        bytes_view lhs;
        bytes_view rhs;
        char const* revert;
        unsigned min_len;
        ::ut::PatternView pattern;

        friend std::ostream & operator<<(std::ostream & out, Put2Mem const & x);
    };

    boost::test_tools::assertion_result bytes_EQ(bytes_view a, bytes_view b, ::ut::PatternView pattern, unsigned min_len);
    boost::test_tools::assertion_result bytes_NE(bytes_view a, bytes_view b, ::ut::PatternView pattern, unsigned min_len);
    boost::test_tools::assertion_result bytes_LT(bytes_view a, bytes_view b, ::ut::PatternView pattern, unsigned min_len);
    boost::test_tools::assertion_result bytes_LE(bytes_view a, bytes_view b, ::ut::PatternView pattern, unsigned min_len);
    boost::test_tools::assertion_result bytes_GT(bytes_view a, bytes_view b, ::ut::PatternView pattern, unsigned min_len);
    boost::test_tools::assertion_result bytes_GE(bytes_view a, bytes_view b, ::ut::PatternView pattern, unsigned min_len);

    template<class T> struct is_array_view : std::false_type {};
    template<class T> struct is_array_view<array_view<T>> : std::true_type {};
    template<class T> struct is_array_view<writable_array_view<T>> : std::true_type {};
    template<> struct is_array_view<::ut::flagged_bytes_view> : std::true_type {};
    template<> struct is_array_view<bytes_view> : std::true_type {};
    template<> struct is_array_view<writable_bytes_view> : std::true_type {};
} // namespace redemption_unit_test__

#if REDEMPTION_UNIT_TEST_FAST_CHECK
namespace redemption_unit_test__
{
    template<class T>
    auto normalize_view(T const& v)
    {
        if constexpr (std::is_convertible_v<T, bytes_view>) {
            return bytes_view{v};
        }
        else if constexpr (std::is_same_v<T, ::ut::flagged_bytes_view>) {
            return v.bytes;
        }
        else {
            return array_view{v};
        }
    }

    template<class Op, class T, class U>
    bool av_equal(Op op, T const& x, U const& y)
    {
        if constexpr (std::is_same_v<T, bytes_view> || std::is_same_v<U, bytes_view>) {
            bytes_view a {x};
            bytes_view b {y};
            return std::equal(a.begin(), a.end(), b.begin(), b.end(), op);
        }
        else
        {
            return std::equal(x.begin(), x.end(), y.begin(), y.end(), op);
        }
    }

    template<class T, class U>
    using enable_comparison_if_view = std::enable_if_t<
        is_array_view<T>::value || is_array_view<U>::value, bool>;

    namespace ops
    {
        template<class T, class U>
        enable_comparison_if_view<T, U>
        operator == (T const& x, U const& y)
        {
            return av_equal(std::equal_to{}, normalize_view(x), normalize_view(y));
        }

        template<class T, class U>
        enable_comparison_if_view<T, U>
        operator != (T const& x, U const& y)
        {
            return !av_equal(std::equal_to{}, normalize_view(x), normalize_view(y));
        }

        template<class T, class U>
        enable_comparison_if_view<T, U>
        operator < (T const& x, U const& y)
        {
            return av_equal(std::less{}, normalize_view(x), normalize_view(y));
        }

        template<class T, class U>
        enable_comparison_if_view<T, U>
        operator >= (T const& x, U const& y)
        {
            return !av_equal(std::less{}, normalize_view(x), normalize_view(y));
        }

        template<class T, class U>
        enable_comparison_if_view<T, U>
        operator > (T const& x, U const& y)
        {
            return av_equal(std::less{}, normalize_view(y), normalize_view(x));
        }

        template<class T, class U>
        enable_comparison_if_view<T, U>
        operator <= (T const& x, U const& y)
        {
            return !av_equal(std::less{}, normalize_view(y), normalize_view(x));
        }
    }
} // namespace redemption_unit_test__

using redemption_unit_test__::ops::operator ==;
using redemption_unit_test__::ops::operator !=;
using redemption_unit_test__::ops::operator <;
using redemption_unit_test__::ops::operator <=;
using redemption_unit_test__::ops::operator >;
using redemption_unit_test__::ops::operator >=;

#else

namespace redemption_unit_test__
{
    // boost::unit_test::is_forward_iterable<array_view<T>> -> false (see bellow)
    template<class T>
    struct View : array_view<T>
    {
        using array_view<T>::array_view;

        View(array_view<T> v) noexcept : array_view<T>(v) {}
    };
} // namespace redemption_unit_test__

namespace boost {

namespace unit_test {
    // disable collection_compare for array_view like type
    // but this disable also test_tools::per_element() and test_tools::lexicographic()
    // BOOST_TEST(a == b, tt::per_element())
    template<class T> struct is_forward_iterable<array_view<T>> : mpl::false_ {};
    template<class T> struct is_forward_iterable<writable_array_view<T>> : mpl::false_ {};
    template<> struct is_forward_iterable<bytes_view> : mpl::false_ {};
    template<> struct is_forward_iterable<writable_bytes_view> : mpl::false_ {};
}

namespace test_tools {
namespace assertion {
namespace op {

#define DEFINE_COLLECTION_COMPARISON_REV(rev, ...) " " #rev " "
// BOOST_TEST_FOR_EACH_COMP_OP(action)
// action( oper, name, rev )
// since Boost 1.75:
// action( oper, name, rev, name_inverse )
#define DEFINE_COLLECTION_COMPARISON(oper, name, ...)                     \
template<class T, class U>                                                \
struct name<T, U, std::enable_if_t<(                                      \
    ::redemption_unit_test__::is_array_view<T>::value                     \
 || ::redemption_unit_test__::is_array_view<U>::value                     \
)>>                                                                       \
{                                                                         \
    using result_type = assertion_result;                                 \
    using OP = name<T, U>;                                                \
                                                                          \
    static assertion_result                                               \
    eval( T const& lhs, U const& rhs )                                    \
    {                                                                     \
        if constexpr (                                                    \
            (std::is_convertible_v<T, bytes_view>                         \
            || std::is_same_v<T, ::ut::flagged_bytes_view>)               \
         && (std::is_convertible_v<T, bytes_view>                         \
            || std::is_same_v<T, ::ut::flagged_bytes_view>)               \
        ) {                                                               \
            auto flag = ::ut::default_pattern_view;                       \
            unsigned min_len = ::ut::default_ascii_min_len;               \
            bytes_view a;                                                 \
            bytes_view b;                                                 \
                                                                          \
            if constexpr (std::is_same_v<T, ::ut::flagged_bytes_view>) {  \
                flag = lhs.pattern;                                       \
                min_len = std::max(min_len, lhs.min_len);                 \
                a = lhs.bytes;                                            \
            }                                                             \
            else {                                                        \
                a = lhs;                                                  \
            }                                                             \
                                                                          \
            if constexpr (std::is_same_v<U, ::ut::flagged_bytes_view>) {  \
                flag = rhs.pattern;                                       \
                min_len = std::max(min_len, rhs.min_len);                 \
                b = rhs.bytes;                                            \
            }                                                             \
            else {                                                        \
                b = rhs;                                                  \
            }                                                             \
                                                                          \
            return ::redemption_unit_test__                               \
                ::bytes_##name(a, b, flag, min_len);                      \
        }                                                                 \
        else                                                              \
        {                                                                 \
            using L = typename T::value_type;                             \
            using R = typename U::value_type;                             \
            using elem_op = op::name<L, R>;                               \
            return boost::test_tools::assertion::op::compare_collections( \
                ::redemption_unit_test__::View<L>{lhs},                   \
                ::redemption_unit_test__::View<R>{rhs},                   \
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
    { return DEFINE_COLLECTION_COMPARISON_REV(__VA_ARGS__, 0); }          \
};

BOOST_TEST_FOR_EACH_COMP_OP(DEFINE_COLLECTION_COMPARISON)
#undef DEFINE_COLLECTION_COMPARISON
#undef DEFINE_COLLECTION_COMPARISON_REV

} // namespace op
} // namespace assertion
} // namespace test_tools
} // namespace boost

#endif
