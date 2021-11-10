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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/bounded_bytes_view.hpp"

#include <type_traits>
#include <array>

template<class T, class U>
constexpr auto is_callable_impl(U & x, int /*unused*/)
-> decltype((void)(T(x)), std::true_type{})
{ return 1; }

template<class T, class U>
constexpr std::false_type is_callable_impl(U & /*unused*/, char /*unused*/)
{ return {}; }

template<class T, class U>
constexpr auto is_callable(U & x)
-> decltype(is_callable_impl<T>(x, 1))
{ return {}; }

RED_AUTO_TEST_CASE(TestBytesT)
{
    char a[2]{};
    uint8_t ua[2]{};
    char const ca[2]{};
    uint8_t const cua[2]{};
    std::array<uint8_t, 2> uv;
    char * s = a+1;
    uint8_t * us = ua;
    char const * cs = ca;
    uint8_t const * cus = cua;

    auto cba = bounded_bytes_view{uv};
    auto ba = writable_bounded_bytes_view{uv};

    writable_chars_view av{s, 1};
    writable_u8_array_view uav{us, 1};
    chars_view cav{cs, 1};
    u8_array_view cuav{cus, 1};

    std::false_type no;

    is_callable<writable_sized_bytes_view<2>>(a) = no;
    is_callable<writable_sized_bytes_view<2>>(ua) = no;
    writable_sized_bytes_view<2>{ba};

    is_callable<sized_bytes_view<2>>(a) = no;
    is_callable<sized_bytes_view<2>>(ua) = no;
    sized_bytes_view<2>{ba};

    is_callable<sized_bytes_view<2>>(ca) = no;
    is_callable<sized_bytes_view<2>>(cua) = no;
    sized_bytes_view<2>{cba};

    std::is_assignable<writable_sized_bytes_view<2>, decltype(a)>::type{} = no;
    std::is_assignable<writable_sized_bytes_view<2>, decltype(ua)>::type{} = no;
    writable_sized_bytes_view<2>{uv} = ba;

    std::is_assignable<sized_bytes_view<2>, decltype(a)>::type{} = no;
    std::is_assignable<sized_bytes_view<2>, decltype(ua)>::type{} = no;
    sized_bytes_view<2>{uv} = ba;

    std::is_assignable<sized_bytes_view<2>, decltype(ca)>::type{} = no;
    std::is_assignable<sized_bytes_view<2>, decltype(cua)>::type{} = no;
    sized_bytes_view<2>{uv} = cba;

    writable_u8_array_view{ba};
    u8_array_view{} = ba;
    u8_array_view{} = cba;

    [](writable_sized_bytes_view<2> /*dummy*/){}(ba);
    [](sized_bytes_view<2> /*dummy*/){}(ba);
    [](sized_bytes_view<2> /*dummy*/){}(cba);
}
