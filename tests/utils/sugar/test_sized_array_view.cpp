/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "cxx/diagnostic.hpp"
#include "utils/sugar/sized_array_view.hpp"

#include <array>
#include <vector>
#include <string>
#include <string_view>

namespace {

int test_ambiguous(chars_view /*unused*/) { return 1; }
int test_ambiguous(s8_array_view /*unused*/) { return 2; }
int test_ambiguous(u8_array_view /*unused*/) { return 3; }

using voidp = void const*;

} // namespace

RED_AUTO_TEST_CASE(TestArrayView)
{
    char a8[3] = {'x', 'y', 'z'};
    int8_t as8[3] = {-1, 0, 3};
    uint8_t au8[3] = {0, 1, 2};

    sized_array_view<char, 3> avc = make_sized_array_view(a8);

    array_view<char>{} = make_sized_array_view(a8);
    avc = make_sized_array_view(a8);
    avc = sized_array_view<char, 3>::assumed(a8);

    RED_CHECK_EQUAL(test_ambiguous(make_writable_sized_array_view(a8)), 1);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_sized_array_view(as8)), 2);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_sized_array_view(au8)), 3);

    RED_CHECK_EQUAL(make_writable_sized_array_view(a8).size(), 3u);

    // same size (as std::string)
    RED_CHECK_EQUAL(avc.size(), 3);
    // same data (same memory address)
    RED_CHECK_EQUAL(voidp(avc.data()), voidp(a8));

    auto av = make_writable_sized_array_view(a8);
    // same type as s
    RED_CHECK_EQUAL(test_ambiguous(av), 1);
    // same size (as std::string)
    RED_CHECK_EQUAL(av.size(), 3);
    // same data (same memory address)
    RED_CHECK_EQUAL(voidp(av.data()), voidp(a8));
    RED_CHECK_EQUAL(av[0], a8[0]);
    RED_CHECK_EQUAL(av[1], a8[1]);
    RED_CHECK_EQUAL(av[2], a8[2]);
    // array view provides begin() and end()
    RED_CHECK_EQUAL(av.end() - av.begin(), 3);
    // begin is an iterator to first char
    RED_CHECK_EQUAL(voidp(av.begin()), voidp(a8));

    auto const av_p = av.first<3>();
    RED_CHECK_EQUAL(static_cast<void const *>(av_p.data()), static_cast<void const *>(av.data()));
    RED_CHECK_EQUAL(av_p.size(), av.size());
    RED_CHECK_EQUAL(av_p[0], av[0]);
    // array view provides begin() and end()
    RED_CHECK_EQUAL(av_p.end() - av_p.begin(), 3);

    // begin is an iterator to first char
    RED_CHECK_EQUAL(voidp(av_p.begin()), voidp(a8));

    RED_CHECK_EQUAL(make_writable_sized_array_view("abc").size(), 4u);
    RED_CHECK_EQUAL(cstr_sized_array_view("abc").size(), 3u);
    RED_CHECK_EQUAL((av.subarray<0, 1>().size()), 1u);

    RED_CHECK(chars_view{nullptr}.empty());
}

RED_AUTO_TEST_CASE(TestSubArray)
{
    auto a = cstr_sized_array_view("abcd");
    RED_CHECK_EQUAL_RANGES(a.first<1>(), cstr_sized_array_view("a"));
    RED_CHECK_EQUAL_RANGES(a.first<3>(), cstr_sized_array_view("abc"));
    RED_CHECK_EQUAL_RANGES(a.last<1>(), cstr_sized_array_view("d"));
    RED_CHECK_EQUAL_RANGES(a.last<3>(), cstr_sized_array_view("bcd"));
    RED_CHECK_EQUAL_RANGES(a.from_offset<3>(), cstr_sized_array_view("d"));
    RED_CHECK_EQUAL_RANGES(a.from_offset<1>(), cstr_sized_array_view("bcd"));
    RED_CHECK_EQUAL_RANGES((a.subarray<1, 2>()), cstr_sized_array_view("bc"));
}

namespace
{
    template<class T>
    struct ptr_ptr
    {
        ptr_ptr(T* p1, T* p2) : p1(p1), p2(p2) {}
        T* p1;
        T* p2;
    };

    template<class T>
    struct ptr_size
    {
        ptr_size(T* p, std::size_t n) : p(p), n(n) {}
        T* p;
        std::size_t n;
    };
}

RED_AUTO_TEST_CASE(TestArrayView_as)
{
    auto a = cstr_sized_array_view("abcd");

    auto x1 = a.as<ptr_ptr<char const>>();
    RED_CHECK(voidp(x1.p1) == voidp(a.begin()));
    RED_CHECK(voidp(x1.p2) == voidp(a.end()));

    auto x2 = a.as<ptr_ptr>();
    RED_CHECK(voidp(x2.p1) == voidp(a.begin()));
    RED_CHECK(voidp(x2.p2) == voidp(a.end()));

    auto y1 = a.as<ptr_size<char const>>();
    RED_CHECK(voidp(y1.p) == voidp(a.data()));
    RED_CHECK(y1.n == a.size());

    auto y2 = a.as<ptr_size>();
    RED_CHECK(voidp(y2.p) == voidp(a.data()));
    RED_CHECK(y2.n == a.size());

    using vec_t = decltype(a.as<std::vector>());
    std::vector<char>() = vec_t(); // std::is_same

    using array_t = decltype(a.as<std::array>());
    std::array<char, 4>() = array_t(); // std::is_same
}
