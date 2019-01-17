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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE TestCast
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/cast.hpp"

RED_AUTO_TEST_CASE(TestCast)
{
    using Ptr = void const*;

    char const* a = "";
    uint8_t const* b = byte_ptr_cast(a);
    char const* c = char_ptr_cast(b);

    RED_CHECK_EQ(Ptr(a), Ptr(b));
    RED_CHECK_EQ(Ptr(b), Ptr(c));
}

struct testUnderlyingCast
{
    testUnderlyingCast()
    {
        enum E1 : char {A};
        enum E2 : long {B, C};
        constexpr auto a = underlying_cast(A);
        constexpr auto c = underlying_cast(C);
        static_assert(std::is_same_v<const char, decltype(a)>);
        static_assert(std::is_same_v<const long, decltype(c)>);
        static_assert(c == 1);
        static_assert(a == 0);
    }
};
