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

#include "utils/sugar/bytes_view.hpp"

#include <type_traits>
#include <vector>

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
    std::vector<uint8_t> uv;
    char * s = a+1;
    uint8_t * us = ua;
    char const * cs = ca;
    uint8_t const * cus = cua;

    using voidp = void const *;

    writable_byte_ptr{a};
    writable_byte_ptr{s};
    writable_byte_ptr{us};

    byte_ptr{a};
    byte_ptr{s};
    byte_ptr{us};
    byte_ptr{ca};
    byte_ptr{cs};
    byte_ptr{cus};
    byte_ptr{writable_byte_ptr{a}};

    RED_CHECK_EQUAL(voidp(writable_byte_ptr(a).as_charp()), voidp(a));
    RED_CHECK_EQUAL(voidp(writable_byte_ptr(ua).as_charp()), voidp(ua));
    RED_CHECK_EQUAL(voidp(writable_byte_ptr(s).as_charp()), voidp(s));
    RED_CHECK_EQUAL(voidp(writable_byte_ptr(us).as_charp()), voidp(us));

    RED_CHECK_EQUAL(voidp(writable_byte_ptr(a).as_charp()), voidp(writable_byte_ptr(a).as_u8p()));

    RED_CHECK(bool(writable_byte_ptr(a)));
    RED_CHECK(!bool(writable_byte_ptr{}));


    RED_CHECK_EQUAL(voidp(byte_ptr(a).as_charp()), voidp(a));
    RED_CHECK_EQUAL(voidp(byte_ptr(ua).as_charp()), voidp(ua));
    RED_CHECK_EQUAL(voidp(byte_ptr(s).as_charp()), voidp(s));
    RED_CHECK_EQUAL(voidp(byte_ptr(us).as_charp()), voidp(us));

    RED_CHECK_EQUAL(voidp(byte_ptr(ca).as_charp()), voidp(ca));
    RED_CHECK_EQUAL(voidp(byte_ptr(cua).as_charp()), voidp(cus));
    RED_CHECK_EQUAL(voidp(byte_ptr(cs).as_charp()), voidp(cs));
    RED_CHECK_EQUAL(voidp(byte_ptr(cus).as_charp()), voidp(cus));

    RED_CHECK_EQUAL(voidp(byte_ptr(a).as_charp()), voidp(byte_ptr(a).as_u8p()));

    RED_CHECK(bool(byte_ptr(a)));
    RED_CHECK(!bool(byte_ptr{}));

    bytes_view{uv};

    writable_byte_ptr bs{s};
    byte_ptr cbs{cs};
    writable_bytes_view ba;
    bytes_view cba;
    array_view_char av{s, 1};
    array_view_u8 uav{us, 1};
    array_view_const_char cav{cs, 1};
    array_view_const_u8 cuav{cus, 1};

    std::false_type no;

    writable_bytes_view{bs, 1};
    is_callable<writable_bytes_view>(a) = no;
    is_callable<writable_bytes_view>(ua) = no;
    writable_bytes_view{av};
    writable_bytes_view{uav};
    writable_bytes_view{ba};

    bytes_view{bs, 1};
    is_callable<bytes_view>(a) = no;
    is_callable<bytes_view>(ua) = no;
    bytes_view{av};
    bytes_view{uav};
    bytes_view{ba};

    bytes_view{cbs, 1};
    is_callable<bytes_view>(ca) = no;
    is_callable<bytes_view>(cua) = no;
    bytes_view{cav};
    bytes_view{cuav};
    bytes_view{cba};

    std::is_assignable<writable_bytes_view, decltype(a)>::type{} = no;
    std::is_assignable<writable_bytes_view, decltype(ua)>::type{} = no;
    writable_bytes_view{} = av;
    writable_bytes_view{} = uav;
    writable_bytes_view{} = ba;

    std::is_assignable<bytes_view, decltype(a)>::type{} = no;
    std::is_assignable<bytes_view, decltype(ua)>::type{} = no;
    bytes_view{} = av;
    bytes_view{} = uav;
    bytes_view{} = ba;

    std::is_assignable<bytes_view, decltype(ca)>::type{} = no;
    std::is_assignable<bytes_view, decltype(cua)>::type{} = no;
    bytes_view{} = cav;
    bytes_view{} = cuav;
    bytes_view{} = cba;

    array_view_u8{} = ba;
    array_view_const_u8{} = ba;
    array_view_const_u8{} = cba;

    [](writable_bytes_view){}(av);
    [](writable_bytes_view){}(uav);
    [](writable_bytes_view){}(ba);

    [](bytes_view){}(av);
    [](bytes_view){}(uav);
    [](bytes_view){}(ba);

    [](bytes_view){}(cav);
    [](bytes_view){}(cuav);
    [](bytes_view){}(cba);
}
