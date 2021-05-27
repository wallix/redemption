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
#include "utils/sugar/array_view.hpp"

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

    array_view<char>{} = array_view<char>{};
    array_view<char>{} = array_view<char>(a8, short(2));
    array_view<char>{} = writable_array_view<char>{};
    array_view<char>{} = writable_array_view<char>(a8, short(2));
    writable_array_view<char>{} = writable_array_view<char>{};

    RED_CHECK_EQUAL(test_ambiguous(make_writable_array_view(a8)), 1);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_array_view(as8)), 2);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_array_view(au8)), 3);

    RED_CHECK_EQUAL(make_writable_array_view(a8).size(), 3u);

    std::string s;
    RED_CHECK_EQUAL(test_ambiguous(s), 1);

    s = "abc";
    array_view<char> avc(s);
    // same size (as std::string)
    RED_CHECK_EQUAL(avc.size(), s.size());
    // same data (same memory address)
    RED_CHECK_EQUAL(avc.data(), s.data());

    auto av = make_writable_array_view(s);
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

    auto const av_p = make_writable_array_view(&s[0], &s[3]);
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

    RED_CHECK_EQUAL(make_writable_array_view("abc").size(), 4u);
    RED_CHECK_EQUAL(cstr_array_view("abc").size(), 3u);
    RED_CHECK_EQUAL(make_writable_array_view(av.data(), 1).size(), 1u);

    RED_CHECK(chars_view{nullptr}.empty());

    {
        char ca8[3] = {'x', 'y', 'z'};
        const char * left = &ca8[1];
        char * right = &ca8[2];
        auto const avi = make_array_view(left, right);

        RED_CHECK_EQUAL(avi.size(), 1u);
        RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
        char ca8[] = {'x', 'y', 'z', 't'};
        const char * left = &ca8[1];
        auto const avi = make_array_view(left, 2);

        RED_CHECK_EQUAL(avi.size(), 2u);
        RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
        char ca8[] = {'x', 'y', 'z', 't'};
        const char * left = &ca8[1];
        const char * right = &ca8[1];
        auto const avi = make_array_view(left, right);

        RED_CHECK_EQUAL(avi.size(), 0u);
        RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[1]));
    }

    {
        const char ca8[] = {'x', 'y', 'z', 't'};
        auto const avi = make_array_view(ca8);

        RED_CHECK_EQUAL(avi.size(), 4u);
        RED_CHECK_EQUAL(voidp(avi.data()), voidp(&ca8[0]));
    }

    {
        auto const avi = cstr_array_view("0123456789");
        RED_CHECK_EQUAL(avi.size(), 10u);
        RED_CHECK_EQUAL(voidp(avi.data()), voidp(&avi[0]));
    }
}

RED_AUTO_TEST_CASE(TestSubArray)
{
    auto a = cstr_array_view("abcd");
    RED_CHECK_EQUAL_RANGES(a.first(1), cstr_array_view("a"));
    RED_CHECK_EQUAL_RANGES(a.first(3), cstr_array_view("abc"));
    RED_CHECK_EQUAL_RANGES(a.last(1), cstr_array_view("d"));
    RED_CHECK_EQUAL_RANGES(a.last(3), cstr_array_view("bcd"));
    RED_CHECK_EQUAL_RANGES(a.from_offset(3), cstr_array_view("d"));
    RED_CHECK_EQUAL_RANGES(a.from_offset(1), cstr_array_view("bcd"));
    RED_CHECK_EQUAL_RANGES(a.subarray(1, 2), cstr_array_view("bc"));
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
    auto a = cstr_array_view("abcd");

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
    std::vector<char> v = vec_t(); // std::is_same
}

template<class T>
constexpr auto check_cstr_array_view_call(T && a, int /*unused*/) -> decltype(cstr_array_view(a), true)
{
    return true;
}


template<class T>
constexpr bool check_cstr_array_view_call(T && /*unused*/, char /*unused*/)
{
    return false;
}

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wcomma")
template<class V, class T>
constexpr auto check_array_view_call(T && a, int /*unused*/)
  -> decltype(void(array_view<V>(std::forward<T>(a))), true)
{
    return true;
}

template<class T>
constexpr auto check_array_view_guide(T && a, int /*unused*/)
  -> decltype(void(array_view{std::forward<T>(a)}), true)
{
    return true;
}
REDEMPTION_DIAGNOSTIC_POP()

template<class V, class T>
constexpr bool check_array_view_call(T && /*unused*/, char /*unused*/)
{
    return false;
}

template<class T>
constexpr bool check_array_view_guide(T && /*unused*/, char /*unused*/)
{
    return false;
}

namespace
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunneeded-member-function")
    struct Range
    {
        char* data() const { return nullptr; } /*NOLINT*/
        std::size_t size() const { return 0; } /*NOLINT*/
    };
    REDEMPTION_DIAGNOSTIC_POP()

    char cstr[5] = {'0', '1', '2', '\0', '5'};
    char const * p = nullptr;
    std::string str;
    std::string_view strv;
    int ints[3]{};
    Range rng;
} // anonymous namespace

static_assert(not check_cstr_array_view_call(cstr, 1));
static_assert(check_cstr_array_view_call("abc", 1));
static_assert(not check_cstr_array_view_call(p, 1));

static_assert(not check_array_view_call<char>(cstr, 1));
static_assert(not check_array_view_call<char>("abc", 1));
static_assert(not check_array_view_call<char>(p, 1));
static_assert(check_array_view_call<char>(str, 1));
static_assert(check_array_view_call<char>(strv, 1));
static_assert(check_array_view_call<char>(rng, 1));

static_assert(not check_array_view_call<const char>(cstr, 1));
static_assert(not check_array_view_call<const char>("abc", 1));
static_assert(not check_array_view_call<const char>(p, 1));
static_assert(check_array_view_call<const char>(str, 1));
static_assert(check_array_view_call<const char>(strv, 1));
static_assert(check_array_view_call<const char>(std::string_view{}, 1));
static_assert(check_array_view_call<const char>(Range{}, 1));
static_assert(check_array_view_call<const char>(rng, 1));

static_assert(not check_array_view_call<int>(cstr, 1));
static_assert(not check_array_view_call<int>(p, 1));
static_assert(check_array_view_call<int>(ints, 1));
static_assert(not check_array_view_call<int>(rng, 1));

// deduction guide
static_assert(not check_array_view_guide(cstr, 1));
static_assert(not check_array_view_guide("abc", 1));
static_assert(not check_array_view_guide(p, 1));
static_assert(check_array_view_guide(str, 1));
static_assert(check_array_view_guide(strv, 1));
static_assert(check_array_view_guide(rng, 1));
