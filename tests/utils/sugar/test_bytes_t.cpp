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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBytesT
#include "system/redemption_unit_tests.hpp"

#include "utils/sugar/bytes_t.hpp"

#include <type_traits>

template<class T, class U>
constexpr auto is_callable_impl(U & x, int)
-> decltype(void(T(x)), std::true_type{})
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
    char * s = a+1;
    uint8_t * us = ua;
    char const * cs = ca;
    uint8_t const * cus = cua;

    using voidp = void const *;

    bytes_t{a};
    bytes_t{s};
    bytes_t{us};

    const_bytes_t{a};
    const_bytes_t{s};
    const_bytes_t{us};
    const_bytes_t{ca};
    const_bytes_t{cs};
    const_bytes_t{cus};
    const_bytes_t{bytes_t{a}};

    RED_CHECK_EQUAL(voidp(bytes_t(a).to_charp()), voidp(a));
    RED_CHECK_EQUAL(voidp(bytes_t(ua).to_charp()), voidp(ua));
    RED_CHECK_EQUAL(voidp(bytes_t(s).to_charp()), voidp(s));
    RED_CHECK_EQUAL(voidp(bytes_t(us).to_charp()), voidp(us));

    RED_CHECK_EQUAL(voidp(bytes_t(a).to_charp()), voidp(bytes_t(a).to_u8p()));

    RED_CHECK(bool(bytes_t(a)));
    RED_CHECK(!bool(bytes_t{}));


    RED_CHECK_EQUAL(voidp(const_bytes_t(a).to_charp()), voidp(a));
    RED_CHECK_EQUAL(voidp(const_bytes_t(ua).to_charp()), voidp(ua));
    RED_CHECK_EQUAL(voidp(const_bytes_t(s).to_charp()), voidp(s));
    RED_CHECK_EQUAL(voidp(const_bytes_t(us).to_charp()), voidp(us));

    RED_CHECK_EQUAL(voidp(const_bytes_t(ca).to_charp()), voidp(ca));
    RED_CHECK_EQUAL(voidp(const_bytes_t(cua).to_charp()), voidp(cus));
    RED_CHECK_EQUAL(voidp(const_bytes_t(cs).to_charp()), voidp(cs));
    RED_CHECK_EQUAL(voidp(const_bytes_t(cus).to_charp()), voidp(cus));

    RED_CHECK_EQUAL(voidp(const_bytes_t(a).to_charp()), voidp(const_bytes_t(a).to_u8p()));

    RED_CHECK(bool(const_bytes_t(a)));
    RED_CHECK(!bool(const_bytes_t{}));


    bytes_t bs{s};
    const_bytes_t cbs{cs};
    bytes_array ba;
    const_bytes_array cba;
    array_view_char av{s, 1};
    array_view_u8 uav{us, 1};
    array_view_const_char cav{cs, 1};
    array_view_const_u8 cuav{cus, 1};

    std::false_type no;

    bytes_array{bs, 1};
    is_callable<bytes_array>(a) = no;
    is_callable<bytes_array>(ua) = no;
    bytes_array{av};
    bytes_array{uav};
    bytes_array{ba};

    const_bytes_array{bs, 1};
    is_callable<const_bytes_array>(a) = no;
    is_callable<const_bytes_array>(ua) = no;
    const_bytes_array{av};
    const_bytes_array{uav};
    const_bytes_array{ba};

    const_bytes_array{cbs, 1};
    is_callable<const_bytes_array>(ca) = no;
    is_callable<const_bytes_array>(cua) = no;
    const_bytes_array{cav};
    const_bytes_array{cuav};
    const_bytes_array{cba};

    std::is_assignable<bytes_array, decltype(a)>::type{} = no;
    std::is_assignable<bytes_array, decltype(ua)>::type{} = no;
    bytes_array{} = av;
    bytes_array{} = uav;
    bytes_array{} = ba;

    std::is_assignable<const_bytes_array, decltype(a)>::type{} = no;
    std::is_assignable<const_bytes_array, decltype(ua)>::type{} = no;
    const_bytes_array{} = av;
    const_bytes_array{} = uav;
    const_bytes_array{} = ba;

    std::is_assignable<const_bytes_array, decltype(ca)>::type{} = no;
    std::is_assignable<const_bytes_array, decltype(cua)>::type{} = no;
    const_bytes_array{} = cav;
    const_bytes_array{} = cuav;
    const_bytes_array{} = cba;

    array_view_u8{} = ba;
    array_view_const_u8{} = ba;
    array_view_const_u8{} = cba;

    [](bytes_array){}(av);
    [](bytes_array){}(uav);
    [](bytes_array){}(ba);

    [](const_bytes_array){}(av);
    [](const_bytes_array){}(uav);
    [](const_bytes_array){}(ba);

    [](const_bytes_array){}(cav);
    [](const_bytes_array){}(cuav);
    [](const_bytes_array){}(cba);
}
