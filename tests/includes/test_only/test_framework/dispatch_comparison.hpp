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

#include "utils/pp.hpp"

#include <boost/test/test_tools.hpp>

#include "test_only/test_framework/redemption_unit_tests.hpp"


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
}
