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

#define RED_TEST_MODULE TestBytesT
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/byte.hpp"

#include <type_traits>
#include <vector>

template<class T, class U>
constexpr auto is_callable_impl(U & x, int)
-> decltype((void)(T(x)), std::true_type{})
{ return 1; }

template<class T, class U>
constexpr std::false_type is_callable_impl(U &, char)
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
    std::vector<uint8_t> uv;
    char * s = a+1;
    uint8_t * us = ua;
    char const * cs = ca;
    uint8_t const * cus = cua;

    using voidp = void const *;

    byte_ptr{a};
    byte_ptr{s};
    byte_ptr{us};

    const_byte_ptr{a};
    const_byte_ptr{s};
    const_byte_ptr{us};
    const_byte_ptr{ca};
    const_byte_ptr{cs};
    const_byte_ptr{cus};
    const_byte_ptr{byte_ptr{a}};

    RED_CHECK_EQUAL(voidp(byte_ptr(a).to_charp()), voidp(a));
    RED_CHECK_EQUAL(voidp(byte_ptr(ua).to_charp()), voidp(ua));
    RED_CHECK_EQUAL(voidp(byte_ptr(s).to_charp()), voidp(s));
    RED_CHECK_EQUAL(voidp(byte_ptr(us).to_charp()), voidp(us));

    RED_CHECK_EQUAL(voidp(byte_ptr(a).to_charp()), voidp(byte_ptr(a).to_u8p()));

    RED_CHECK(bool(byte_ptr(a)));
    RED_CHECK(!bool(byte_ptr{}));


    RED_CHECK_EQUAL(voidp(const_byte_ptr(a).to_charp()), voidp(a));
    RED_CHECK_EQUAL(voidp(const_byte_ptr(ua).to_charp()), voidp(ua));
    RED_CHECK_EQUAL(voidp(const_byte_ptr(s).to_charp()), voidp(s));
    RED_CHECK_EQUAL(voidp(const_byte_ptr(us).to_charp()), voidp(us));

    RED_CHECK_EQUAL(voidp(const_byte_ptr(ca).to_charp()), voidp(ca));
    RED_CHECK_EQUAL(voidp(const_byte_ptr(cua).to_charp()), voidp(cus));
    RED_CHECK_EQUAL(voidp(const_byte_ptr(cs).to_charp()), voidp(cs));
    RED_CHECK_EQUAL(voidp(const_byte_ptr(cus).to_charp()), voidp(cus));

    RED_CHECK_EQUAL(voidp(const_byte_ptr(a).to_charp()), voidp(const_byte_ptr(a).to_u8p()));

    RED_CHECK(bool(const_byte_ptr(a)));
    RED_CHECK(!bool(const_byte_ptr{}));

    const_byte_array{uv};

    byte_ptr bs{s};
    const_byte_ptr cbs{cs};
    byte_array ba;
    const_byte_array cba;
    array_view_char av{s, 1};
    array_view_u8 uav{us, 1};
    array_view_const_char cav{cs, 1};
    array_view_const_u8 cuav{cus, 1};

    std::false_type no;

    byte_array{bs, 1};
    is_callable<byte_array>(a) = no;
    is_callable<byte_array>(ua) = no;
    byte_array{av};
    byte_array{uav};
    byte_array{ba};

    const_byte_array{bs, 1};
    is_callable<const_byte_array>(a) = no;
    is_callable<const_byte_array>(ua) = no;
    const_byte_array{av};
    const_byte_array{uav};
    const_byte_array{ba};

    const_byte_array{cbs, 1};
    is_callable<const_byte_array>(ca) = no;
    is_callable<const_byte_array>(cua) = no;
    const_byte_array{cav};
    const_byte_array{cuav};
    const_byte_array{cba};

    std::is_assignable<byte_array, decltype(a)>::type{} = no;
    std::is_assignable<byte_array, decltype(ua)>::type{} = no;
    byte_array{} = av;
    byte_array{} = uav;
    byte_array{} = ba;

    std::is_assignable<const_byte_array, decltype(a)>::type{} = no;
    std::is_assignable<const_byte_array, decltype(ua)>::type{} = no;
    const_byte_array{} = av;
    const_byte_array{} = uav;
    const_byte_array{} = ba;

    std::is_assignable<const_byte_array, decltype(ca)>::type{} = no;
    std::is_assignable<const_byte_array, decltype(cua)>::type{} = no;
    const_byte_array{} = cav;
    const_byte_array{} = cuav;
    const_byte_array{} = cba;

    array_view_u8{} = ba;
    array_view_const_u8{} = ba;
    array_view_const_u8{} = cba;

    [](byte_array){}(av);
    [](byte_array){}(uav);
    [](byte_array){}(ba);

    [](const_byte_array){}(av);
    [](const_byte_array){}(uav);
    [](const_byte_array){}(ba);

    [](const_byte_array){}(cav);
    [](const_byte_array){}(cuav);
    [](const_byte_array){}(cba);
}
