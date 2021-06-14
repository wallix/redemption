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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "cxx/diagnostic.hpp"
#include "utils/sugar/bounded_array_view.hpp"

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

RED_AUTO_TEST_CASE(TestLimitedArrayView)
{
    char a8[3] = {'x', 'y', 'z'};
    int8_t as8[3] = {-1, 0, 3};
    uint8_t au8[3] = {0, 1, 2};
    std::array<char, 3> stda{{9, 8, 7}};

    bounded_array_view<char, 2, 4> avc = make_bounded_array_view(a8);
    bounded_array_view<char, 2, 4> cp = avc;
    bounded_array_view<char, 2, 4>{cp};
    array_view<char>{cp};

    auto aw = make_writable_bounded_array_view(a8);
    writable_array_view<char>{aw};
    array_view<char>{aw};

    writable_bounded_array_view<char, 2, 4> aw2 = make_writable_bounded_array_view(a8);
    writable_array_view<char>{aw2};
    array_view<char>{aw2};

    array_view<char>{} = make_bounded_array_view(a8);
    avc = make_bounded_array_view(stda);
    avc = make_bounded_array_view(a8);
    avc = make_bounded_array_view(avc);
    avc = bounded_array_view<char, 2, 4>::assumed(a8);

    RED_CHECK_EQUAL(test_ambiguous(make_writable_bounded_array_view(stda)), 1);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_bounded_array_view(a8)), 1);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_bounded_array_view(as8)), 2);
    RED_CHECK_EQUAL(test_ambiguous(make_writable_bounded_array_view(au8)), 3);

    RED_CHECK_EQUAL(make_writable_bounded_array_view(a8).size(), 3u);

    // same size (as std::string)
    RED_CHECK_EQUAL(avc.size(), 3);
    // same data (same memory address)
    RED_CHECK_EQUAL(voidp(avc.data()), voidp(a8));

    auto av = make_writable_bounded_array_view(a8);
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

    RED_CHECK_EQUAL(make_writable_bounded_array_view("abc").size(), 4u);
    RED_CHECK_EQUAL((av.subarray<0, 1>().size()), 1u);

    RED_CHECK(chars_view{nullptr}.empty());
}

RED_AUTO_TEST_CASE(TestSizedArrayView)
{
    char a8[3] = {'x', 'y', 'z'};
    int8_t as8[3] = {-1, 0, 3};
    uint8_t au8[3] = {0, 1, 2};
    std::array<char, 3> stda{{9, 8, 7}};

    sized_array_view<char, 3> avc = make_sized_array_view(a8);

    array_view<char>{} = make_sized_array_view(a8);
    avc = make_sized_array_view(stda);
    avc = make_sized_array_view(a8);
    avc = make_sized_array_view(avc);
    avc = sized_array_view<char, 3>::assumed(a8);

    RED_CHECK_EQUAL(test_ambiguous(make_writable_sized_array_view(stda)), 1);
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

    RED_CHECK("abc"_av == "abc"_sized_av);
}

RED_AUTO_TEST_CASE(TestSubBoundedArray)
{
    char s[]{'a', 'b', 'c', 'd'};
    auto a = make_bounded_array_view<2, 5>(s);
    RED_CHECK_EQUAL_RANGES(a.first<1>(), "a"_av);
    RED_CHECK_EQUAL_RANGES(a.first(3), "abc"_av);
    RED_CHECK_EQUAL_RANGES(a.last<1>(), "d"_av);
    RED_CHECK_EQUAL_RANGES(a.last(3), "bcd"_av);
    RED_CHECK_EQUAL_RANGES(a.from_offset(3), "d"_av);
    RED_CHECK_EQUAL_RANGES(a.from_offset<1>(), "bcd"_av);
    RED_CHECK_EQUAL_RANGES((a.subarray<1, 1>()), "b"_av);
    RED_CHECK_EQUAL_RANGES((a.subarray(1, 2)), "bc"_av);
}

RED_AUTO_TEST_CASE(TestSubSizedArray)
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
} // anonymous namespace

RED_AUTO_TEST_CASE(TestLimitedArrayView_as)
{
    auto a = make_bounded_array_view<2, 5>("abcd");

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
}

RED_AUTO_TEST_CASE(TestSizedArrayView_as)
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


REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wcomma")
template<class V, class T>
constexpr auto check_bounded_array_view_call(T && a, int /*unused*/)
  -> decltype(void(bounded_array_view<V, 2, 4>(std::forward<T>(a))), true)
{
    return true;
}

template<class T>
constexpr auto check_bounded_array_view_guide(T && a, int /*unused*/)
  -> decltype(void(bounded_array_view{std::forward<T>(a)}), true)
{
    return true;
}

template<class V, class T>
constexpr auto check_sized_array_view_call(T && a, int /*unused*/)
  -> decltype(void(sized_array_view<V, 3>(std::forward<T>(a))), true)
{
    return true;
}
REDEMPTION_DIAGNOSTIC_POP()

template<class V, class T>
constexpr bool check_bounded_array_view_call(T && /*unused*/, char /*unused*/)
{
    return false;
}

template<class T>
constexpr bool check_bounded_array_view_guide(T && /*unused*/, char /*unused*/)
{
    return false;
}

template<class V, class T>
constexpr bool check_sized_array_view_call(T && /*unused*/, char /*unused*/)
{
    return false;
}

namespace
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-function")
    void bounded_array_view_assert()
    {
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunneeded-member-function")
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-member-function")
        struct Range
        {
            char* data() const { return nullptr; } /*NOLINT*/
            std::size_t size() const { return 0; } /*NOLINT*/
        };

        char cstr[3] = {'0', '1', '2'};
        char const * p = nullptr;
        std::string str;
        std::string_view strv;
        int ints[3]{};
        Range rng;

        static_assert(not check_bounded_array_view_call<char>(cstr, 1));
        static_assert(not check_bounded_array_view_call<char>(p, 1));
        static_assert(not check_bounded_array_view_call<char>(str, 1));
        static_assert(not check_bounded_array_view_call<char>(strv, 1));
        static_assert(not check_bounded_array_view_call<char>(rng, 1));

        static_assert(not check_bounded_array_view_call<const char>(cstr, 1));
        static_assert(not check_bounded_array_view_call<const char>(p, 1));
        static_assert(not check_bounded_array_view_call<const char>(str, 1));
        static_assert(not check_bounded_array_view_call<const char>(strv, 1));
        static_assert(not check_bounded_array_view_call<const char>(std::string_view{}, 1));
        static_assert(not check_bounded_array_view_call<const char>(Range{}, 1));
        static_assert(not check_bounded_array_view_call<const char>(rng, 1));

        static_assert(not check_bounded_array_view_call<int>(cstr, 1));
        static_assert(not check_bounded_array_view_call<int>(p, 1));
        static_assert(check_bounded_array_view_call<int>(ints, 1));
        static_assert(not check_bounded_array_view_call<int>(rng, 1));

        // deduction guide
        static_assert(not check_bounded_array_view_guide(cstr, 1));
        static_assert(not check_bounded_array_view_guide(p, 1));
        static_assert(not check_bounded_array_view_guide(str, 1));
        static_assert(not check_bounded_array_view_guide(strv, 1));
        static_assert(not check_bounded_array_view_guide(rng, 1));
        static_assert(check_bounded_array_view_guide(ints, 1));

        // sized view

        static_assert(not check_sized_array_view_call<char>(cstr, 1));
        static_assert(not check_sized_array_view_call<char>(p, 1));
        static_assert(not check_sized_array_view_call<char>(str, 1));
        static_assert(not check_sized_array_view_call<char>(strv, 1));
        static_assert(not check_sized_array_view_call<char>(rng, 1));

        static_assert(not check_sized_array_view_call<const char>(cstr, 1));
        static_assert(not check_sized_array_view_call<const char>(p, 1));
        static_assert(not check_sized_array_view_call<const char>(str, 1));
        static_assert(not check_sized_array_view_call<const char>(strv, 1));
        static_assert(not check_sized_array_view_call<const char>(std::string_view{}, 1));
        static_assert(not check_sized_array_view_call<const char>(Range{}, 1));
        static_assert(not check_sized_array_view_call<const char>(rng, 1));

        static_assert(not check_sized_array_view_call<int>(cstr, 1));
        static_assert(not check_sized_array_view_call<int>(p, 1));
        static_assert(check_sized_array_view_call<int>(ints, 1));
        static_assert(not check_sized_array_view_call<int>(rng, 1));
    }
    REDEMPTION_DIAGNOSTIC_POP()
} // anonymous namespace
