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


#define RED_TEST_MODULE TestArrayView
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/array_view.hpp"

#include <string>

namespace {

int test_ambiguous(array_view_const_char) { return 1; }
int test_ambiguous(array_view_const_s8) { return 2; }
int test_ambiguous(array_view_const_u8) { return 3; }

using voidp = void const*;

}

RED_AUTO_TEST_CASE(TestArrayView)
{
    char a8[3] = {'x', 'y', 'z'};
    int8_t as8[3] = {-1, 0, 3};
    uint8_t au8[3] = {0, 1, 2};

    array_view<const char>{} = array_view<char>{};
    array_view<const char>{} = array_view<char>(a8, short(2));

    RED_CHECK_EQUAL(test_ambiguous(a8), 1);
    RED_CHECK_EQUAL(test_ambiguous(as8), 2);
    RED_CHECK_EQUAL(test_ambiguous(au8), 3);

    RED_CHECK_EQUAL(make_array_view(a8).size(), 3u);

    std::string s;
    RED_CHECK_EQUAL(test_ambiguous(s), 1);

    s = "abc";
    array_view<const char> avc(s);
    // same size (as std::string)
    RED_CHECK_EQUAL(avc.size(), s.size());
    // same data (same memory address)
    RED_CHECK_EQUAL(avc.data(), s.data());

    auto av = make_array_view(s);
    // same type as s
    RED_CHECK_EQUAL(test_ambiguous(av), 1);
    // same size (as std::string)
    RED_CHECK_EQUAL(av.size(), s.size());
    // same data (same memory address)
    RED_CHECK_EQUAL(av.data(), s.data());
    RED_CHECK_EQUAL(av[0], s[0]);
    RED_CHECK_EQUAL(av[1], s[1]);
    RED_CHECK_EQUAL(av[2], s[2]);
    // array view provides begin() and end()
    RED_CHECK_EQUAL(av.end() - av.begin(), 3);
    // begin is an iterator to first char
    RED_CHECK_EQUAL(*av.begin(), 'a');
    auto it = av.begin();
    it++;
    RED_CHECK_EQUAL(*it, 'b');
    --it;
    RED_CHECK_EQUAL(*it, 'a');
    ++it;
    RED_CHECK_EQUAL(*it, 'b');

    auto const av_p = make_array_view(&s[0], &s[3]);
    RED_CHECK_EQUAL(static_cast<void const *>(av_p.data()), static_cast<void const *>(av.data()));
    RED_CHECK_EQUAL(av_p.size(), av.size());
    RED_CHECK_EQUAL(av_p[0], av[0]);
    RED_CHECK_EQUAL(av_p[1], s[1]);
    RED_CHECK_EQUAL(av_p[2], s[2]);
    // array view provides begin() and end()
    RED_CHECK_EQUAL(av_p.end() - av_p.begin(), 3);

    // begin is an iterator to first char
    RED_CHECK_EQUAL(*av_p.begin(), 'a');
    auto it2 = av_p.begin();
    it2++;
    RED_CHECK_EQUAL(*it2, 'b');

    RED_CHECK_EQUAL(make_array_view("abc").size(), 4u);
    RED_CHECK_EQUAL(cstr_array_view("abc").size(), 3u);
    RED_CHECK_EQUAL(make_array_view(av.data(), 1).size(), 1u);

    RED_CHECK(array_view_char{nullptr}.empty());

    {
    char ca8[3] = {'x', 'y', 'z'};
    const char * left = &ca8[1];
    char * right = &ca8[2];
    auto const avi = make_array_view(left, right);

    RED_CHECK_EQUAL(avi.size(), 1u);
    RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
    char ca8[3] = {'x', 'y', 'z'};
    char * left = &ca8[1];
    const char * right = &ca8[2];
    auto const avi = make_array_view(left, right);

    RED_CHECK_EQUAL(avi.size(), 1u);
    RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
    char ca8[] = {'x', 'y', 'z', 't'};
    const char * left = &ca8[1];
    auto const avi = make_const_array_view(left, 2);

    RED_CHECK_EQUAL(avi.size(), 2u);
    RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
    char ca8[] = {'x', 'y', 'z', 't'};
    const char * left = &ca8[1];
    const char * right = &ca8[1];
    auto const avi = make_const_array_view(left, right);

    RED_CHECK_EQUAL(avi.size(), 0u);
    RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
    const char ca8[] = {'x', 'y', 'z', 't'};
    auto const avi = make_const_array_view(ca8);

    RED_CHECK_EQUAL(avi.size(), 4u);
    RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[0]));
    }

    {
    auto const avi = cstr_array_view("0123456789");
    RED_CHECK_EQUAL(avi.size(), 10u);
//    RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[0]));
    }
}

RED_AUTO_TEST_CASE(TestSubArray)
{
    auto a = cstr_array_view("abcd");
    RED_CHECK_EQUAL_RANGES(a.first(1), cstr_array_view("a"));
    RED_CHECK_EQUAL_RANGES(a.first(3), cstr_array_view("abc"));
    RED_CHECK_EQUAL_RANGES(a.last(1), cstr_array_view("d"));
    RED_CHECK_EQUAL_RANGES(a.last(3), cstr_array_view("bcd"));
    RED_CHECK_EQUAL_RANGES(a.array_from_offset(3), cstr_array_view("d"));
    RED_CHECK_EQUAL_RANGES(a.array_from_offset(1), cstr_array_view("bcd"));
    RED_CHECK_EQUAL_RANGES(a.subarray(1, 2), cstr_array_view("bc"));
}

template<class T>
auto check_call(T && a, int) -> decltype(cstr_array_view(a), true)
{
    return true;
}


template<class T>
bool check_call(T &&, char)
{
    return false;
}

RED_AUTO_TEST_CASE(TestCStrOnlyWorksForLiterals)
{
    char cstr[5] = {'0', '1', '2', '\0', '5'};
    RED_CHECK_EQUAL(check_call(cstr, 1), false);
    RED_CHECK_EQUAL(check_call("abc", 1), true);
    char const * p = nullptr;
    RED_CHECK_EQUAL(check_call(p, 1), false);
}
