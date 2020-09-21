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

#include <algorithm>


#define RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, name, rev)              \
    namespace boost::test_tools::assertion::op {                                  \
        template<RED_PP_IDENTITY tpl>                                             \
        struct name<RED_PP_IDENTITY t1, RED_PP_IDENTITY t2>                       \
        {                                                                         \
            using result_type = assertion_result;                                 \
            using OP = name;                                                      \
                                                                                  \
            static assertion_result                                               \
            eval(RED_PP_IDENTITY t1 const& lhs, RED_PP_IDENTITY t2 const& rhs)    \
            {                                                                     \
                return ::func ## name(lhs, rhs);                                  \
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
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, EQ, !=)
#define RED_TEST_DISPATCH_COMPARISON_NE(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, NE, !=)
#define RED_TEST_DISPATCH_COMPARISON_LT(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, LT, <)
#define RED_TEST_DISPATCH_COMPARISON_LE(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, LE, <=)
#define RED_TEST_DISPATCH_COMPARISON_GT(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, GT, >)
#define RED_TEST_DISPATCH_COMPARISON_GE(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_I(tpl, t1, t2, func, GE, >=)

#define RED_TEST_DISPATCH_COMPARISONS(tpl, t1, t2, func) \
    RED_TEST_DISPATCH_COMPARISON_EQ(tpl, t1, t2, func)   \
    RED_TEST_DISPATCH_COMPARISON_NE(tpl, t1, t2, func)   \
    RED_TEST_DISPATCH_COMPARISON_LT(tpl, t1, t2, func)   \
    RED_TEST_DISPATCH_COMPARISON_LE(tpl, t1, t2, func)   \
    RED_TEST_DISPATCH_COMPARISON_GT(tpl, t1, t2, func)   \
    RED_TEST_DISPATCH_COMPARISON_GE(tpl, t1, t2, func)

namespace ut::ops
{
    using assertion_result = boost::test_tools::assertion_result;

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_EQ(C1 const& a, C2 const& b, FMismatch&& fmismatch, bool force_fail = false)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = !(pos == a.size() && a.size() == b.size());

        if (REDEMPTION_UNLIKELY(r || force_fail))
        {
            ar = false;

            auto& out = ar.message();

            out << "[";
            fmismatch(out, pos, "==", r);
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
    assertion_result compare_collection_NE(C1 const& a, C2 const& b, FMismatch&& fmismatch, bool force_fail = false)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = pos == a.size() && a.size() == b.size();
        if (REDEMPTION_UNLIKELY(r || force_fail))
        {
            ar = false;

            auto& out = ar.message();

            out << "[";
            fmismatch(out, pos, "!=", r);
            out << "]";
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_LT(C1 const& a, C2 const& b, FMismatch&& fmismatch, bool force_fail = false)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == a.size())
            ? a.size() < b.size()
            : (pos != b.size() && a[pos] < b[pos]);
        if (REDEMPTION_UNLIKELY(r || force_fail))
        {
            ar = false;

            auto& out = ar.message();

            out << "[";
            fmismatch(out, pos, "<", r);
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_LE(C1 const& a, C2 const& b, FMismatch&& fmismatch, bool force_fail = false)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == a.size())
            ? a.size() <= b.size()
            : (pos != b.size() && a[pos] <= b[pos]);
        if (REDEMPTION_UNLIKELY(r || force_fail))
        {
            ar = false;

            auto& out = ar.message();

            out << "[";
            fmismatch(out, pos, "<=", r);
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_GT(C1 const& a, C2 const& b, FMismatch&& fmismatch, bool force_fail = false)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == b.size())
            ? a.size() > b.size()
            : (pos != a.size() && a[pos] > b[pos]);
        if (REDEMPTION_UNLIKELY(r || force_fail))
        {
            ar = false;

            auto& out = ar.message();

            out << "[";
            fmismatch(out, pos, "<", r);
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }

    template<class C1, class C2, class FMismatch>
    assertion_result compare_collection_GE(C1 const& a, C2 const& b, FMismatch&& fmismatch, bool force_fail = false)
    {
        assertion_result ar(true);

        size_t pos = std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first - a.begin();
        const bool r = (pos == b.size())
            ? a.size() >= b.size()
            : (pos != a.size() && a[pos] >= b[pos]);
        if (REDEMPTION_UNLIKELY(r || force_fail))
        {
            ar = false;

            auto& out = ar.message();

            out << "[";
            fmismatch(out, pos, ">=", r);
            out << "]\nMismatch at position " << pos;
        }

        return ar;
    }
}
