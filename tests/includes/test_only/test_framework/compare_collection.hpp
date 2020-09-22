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
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#pragma once

#include "cxx/cxx.hpp"
#include "utils/pp.hpp"

#include <boost/test/test_tools.hpp>

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include <algorithm>


#define RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, t3, func, name, rev)          \
    namespace boost::test_tools::assertion::op {                                  \
        template<RED_PP_IDENTITY tpl>                                             \
        struct name<RED_PP_IDENTITY t1, RED_PP_IDENTITY t2, RED_PP_IDENTITY t3>   \
        {                                                                         \
            using result_type = assertion_result;                                 \
            using OP = name;                                                      \
                                                                                  \
            static assertion_result                                               \
            eval(RED_PP_IDENTITY t1 const& lhs, RED_PP_IDENTITY t2 const& rhs)    \
            {                                                                     \
                return func(lhs, rhs);                                            \
            }                                                                     \
                                                                                  \
            template<class PrevExprType>                                          \
            static void                                                           \
            report(std::ostream&, PrevExprType const&, RED_PP_IDENTITY t2 const&) \
            {}                                                                    \
                                                                                  \
            static char const* revert()                                           \
            { return " " #rev " "; }                                              \
        };                                                                        \
    }

#define RED_TEST_DISPATCH_COMPARISON_EQ(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, (void), func, EQ, !=)
#define RED_TEST_DISPATCH_COMPARISON_NE(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, (void), func, NE, !=)
#define RED_TEST_DISPATCH_COMPARISON_LT(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, (void), func, LT, <)
#define RED_TEST_DISPATCH_COMPARISON_LE(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, (void), func, LE, <=)
#define RED_TEST_DISPATCH_COMPARISON_GT(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, (void), func, GT, >)
#define RED_TEST_DISPATCH_COMPARISON_GE(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, (void), func, GE, >=)

#define RED_TEST_DISPATCH_COMPARISON_EQ2(tpl, t1, t2, t3, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, t3, func, EQ, !=)
#define RED_TEST_DISPATCH_COMPARISON_NE2(tpl, t1, t2, t3, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, t3, func, NE, !=)

#define RED_TEST_DISPATCH_COMPARISONS(tpl, t1, t2, func)     \
    RED_TEST_DISPATCH_COMPARISON_EQ(tpl, t1, t2, func ## EQ) \
    RED_TEST_DISPATCH_COMPARISON_NE(tpl, t1, t2, func ## NE) \
    RED_TEST_DISPATCH_COMPARISON_LT(tpl, t1, t2, func ## LT) \
    RED_TEST_DISPATCH_COMPARISON_LE(tpl, t1, t2, func ## LE) \
    RED_TEST_DISPATCH_COMPARISON_GT(tpl, t1, t2, func ## GT) \
    RED_TEST_DISPATCH_COMPARISON_GE(tpl, t1, t2, func ## GE)

namespace ut
{
    using assertion_result = boost::test_tools::assertion_result;

    template<class Cmp>
    struct comparator;

    template<class Cmp, class T>
    struct comparator_with
    {
        Cmp cmp;
        T x;
        bool nocolor;
    };

    template<class Cmp>
    struct comparator
    {
        Cmp cmp;
        bool nocolor = false;

        template<class T>
        comparator_with<Cmp, T> operator()(T&& x) const
        {
            return {cmp, static_cast<T&&>(x), nocolor};
        }
    };

    template<class F, class Cmp = std::equal_to<>>
    struct render;

    template<class F, class Cmp, class T>
    struct render_with
    {
        F f;
        Cmp cmp;
        T x;
        bool nocolor;
    };

    template<class F, class Cmp>
    struct render
    {
        F f;
        Cmp cmp = Cmp();
        bool nocolor = false;

        template<class T>
        render_with<F, Cmp, T> operator()(T&& x) const
        {
            return {f, cmp, static_cast<T&&>(x), nocolor};
        }
    };

    template<class T, class U, class F>
    void put_data_with_diff(
        std::ostream& out, T const& x, char const* op, U const& y, F&& f,
        bool nocolor = false /*NOLINT*/)
    {
        std::ostringstream oss;

        f(oss, x);
        const auto s1 = oss.str();
        oss.str({});
        f(oss, y);

        put_message_with_diff(out, s1, op, oss.str(), nocolor);
    }


    template<class T, class U, class F>
    assertion_result create_assertion_result(
        bool r, T const& x, char const* op, U const& y, F&& f,
        bool nocolor = false /*NOLINT*/)
    {
        assertion_result ar(r);

        if (!r) {
            auto& out = ar.message().stream();
            out << "[";
            put_data_with_diff(out, x, op, y, f, nocolor);
            out << "]";
        }

        return ar;
    }

    struct put_default_ostream_t
    {
        template<class T>
        void operator()(std::ostream& out, T const& x) const
        {
            out << x;
        }
    };

    namespace detail
    {
        void boost_test_print_type(std::ostream&, int);
    }

    struct put_default_print_type_t
    {
        template<class T>
        void operator()(std::ostream& out, T const& x) const
        {
            using detail::boost_test_print_type;
            boost_test_print_type(out, x);
        }
    };
}

#if REDEMPTION_UNIT_TEST_FAST_CHECK
namespace ut
{
    template<class Cmp, class T, class U>
    bool operator==(comparator_with<Cmp, T> const& data, U const& y)
    {
        return data.cmp(data.x, y);
    }

    template<class F, class Cmp, class T, class U>
    bool operator==(render_with<F, Cmp, T> const& data, U const& y)
    {
        return data.cmp(data.x, y);
    }

    template<class Cmp, class T, class U>
    bool operator!=(comparator_with<Cmp, T> const& data, U const& y)
    {
        return !data.cmp(data.x, y);
    }

    template<class F, class Cmp, class T, class U>
    bool operator!=(render_with<F, Cmp, T> const& data, U const& y)
    {
        return !data.cmp(data.x, y);
    }
}
#else
namespace ut::detail
{
    template<class Cmp, class T, class U>
    assertion_result comparator_with_EQ(comparator_with<Cmp, T> const& data, U const& y)
    {
        return create_assertion_result(
            data.cmp(data.x, y),
            data.x, " != ", y, put_default_ostream_t{}, data.nocolor);
    }

    template<class Cmp, class T, class U>
    assertion_result comparator_with_NE(comparator_with<Cmp, T> const& data, U const& y)
    {
        return create_assertion_result(
            !data.cmp(data.x, y),
            data.x, " == ", y, put_default_ostream_t{}, data.nocolor);
    }

    template<class F, class Cmp, class T, class U>
    assertion_result render_with_EQ(render_with<F, Cmp, T> const& data, U const& y)
    {
        return create_assertion_result(
            data.cmp(data.x, y),
            data.x, " != ", y, data.f, data.nocolor);
    }

    template<class F, class Cmp, class T, class U>
    assertion_result render_with_NE(render_with<F, Cmp, T> const& data, U const& y)
    {
        return create_assertion_result(
            !data.cmp(data.x, y),
            data.x, " == ", y, data.f, data.nocolor);
    }
}

RED_TEST_DISPATCH_COMPARISON_EQ((class Cmp, class T, class U), (ut::comparator_with<Cmp, T>), (U), ::ut::detail::comparator_with_EQ)
RED_TEST_DISPATCH_COMPARISON_NE((class Cmp, class T, class U), (ut::comparator_with<Cmp, T>), (U), ::ut::detail::comparator_with_NE)
RED_TEST_DISPATCH_COMPARISON_EQ((class F, class Cmp, class T, class U), (ut::render_with<F, Cmp, T>), (U), ::ut::detail::render_with_EQ)
RED_TEST_DISPATCH_COMPARISON_NE((class F, class Cmp, class T, class U), (ut::render_with<F, Cmp, T>), (U), ::ut::detail::render_with_NE)
#endif

namespace ut::ops
{
    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_EQ(C1 const& a, C2 const& b, FMismatch&& fmismatch)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = !(pos == a.size() && a.size() == b.size());

        if (REDEMPTION_UNLIKELY(r))
        {
            ar = false;

            auto& out = ar.message().stream();

            out << "[";
            fmismatch(out, pos, "!=");
            out << "]\nMismatch at position " << pos;

            if (a.size() != b.size())
            {
                out << "\nCollections size mismatch: "
                    << a.size() << " != " << b.size()
                ;
            }
        }

        return ar;
    }


    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_NE(C1 const& a, C2 const& b, FMismatch&& fmismatch)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = pos == a.size() && a.size() == b.size();
        if (REDEMPTION_UNLIKELY(r))
        {
            ar = false;

            auto& out = ar.message().stream();

            out << "[";
            fmismatch(out, pos, "==");
            out << "]";
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_LT(C1 const& a, C2 const& b, FMismatch&& fmismatch)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == a.size())
            ? a.size() < b.size()
            : (pos != b.size() && a[pos] < b[pos]);
        if (REDEMPTION_UNLIKELY(r))
        {
            ar = false;

            auto& out = ar.message().stream();

            out << "[";
            fmismatch(out, pos, ">=");
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_LE(C1 const& a, C2 const& b, FMismatch&& fmismatch)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == a.size())
            ? a.size() <= b.size()
            : (pos != b.size() && a[pos] <= b[pos]);
        if (REDEMPTION_UNLIKELY(r))
        {
            ar = false;

            auto& out = ar.message().stream();

            out << "[";
            fmismatch(out, pos, ">");
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_GT(C1 const& a, C2 const& b, FMismatch&& fmismatch)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == b.size())
            ? a.size() > b.size()
            : (pos != a.size() && a[pos] > b[pos]);
        if (REDEMPTION_UNLIKELY(r))
        {
            ar = false;

            auto& out = ar.message().stream();

            out << "[";
            fmismatch(out, pos, ">=");
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_GE(C1 const& a, C2 const& b, FMismatch&& fmismatch)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == b.size())
            ? a.size() >= b.size()
            : (pos != a.size() && a[pos] >= b[pos]);
        if (REDEMPTION_UNLIKELY(r))
        {
            ar = false;

            auto& out = ar.message().stream();

            out << "[";
            fmismatch(out, pos, "<");
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }
}
