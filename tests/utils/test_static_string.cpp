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

#include "utils/static_string.hpp"
#include "utils/sugar/bounded_array_view.hpp"

#include <array>


namespace
{
    template<std::size_t>
    struct uint_c
    {};
}

RED_AUTO_TEST_CASE(TestStaticString)
{
    static_string<10> s1;

    uint_c<12>() = uint_c<sizeof(s1)>();

    static_string<11> s2{s1};

    RED_CHECK(s1.empty());
    RED_CHECK(s2.empty());

    s1 = "abcd"_sized_av;

    RED_CHECK_EQUAL(chars_view{s1}, "abcd"_av);

    auto s3 = static_str_concat(s1, s2, 'e', "fg"_sized_av);
    s3 = static_str_concat<sizeof(s3)>(s1, s2, 'e', "fg"_sized_av);
    uint_c<26>() = uint_c<sizeof(s3)>();
    RED_CHECK_EQUAL(s3, "abcdefg"_av);

    static_str_assign(s2, s1, std::array<char, 1>{{'x'}});
    RED_CHECK_EQUAL(s2, "abcdx"_av);

    s3 = s2;
    RED_CHECK_EQUAL(s3, "abcdx"_av);
    RED_CHECK_EQUAL(s3.view_with_null_terminator(), "abcdx\0"_av);

    s3.clear();
    RED_CHECK_EQUAL(s3, ""_av);

    // set_string_len()
    s3.delayed_build([](auto buffer) {
        auto array = buffer.chars_without_null_terminated();
        std::size_t i = 0;
        array[i++] = 'a';
        array[i++] = 'b';
        array[i++] = 'c';
        return buffer.set_string_len(i);
    });
    RED_CHECK_EQUAL(s3, "abc"_av);

    // set_null_terminated_string_len()
    s3.delayed_build([](auto buffer) {
        auto array = buffer.chars_without_null_terminated();
        std::size_t i = 0;
        array[i++] = 'x';
        array[i++] = 'y';
        array[i++] = 'z';
        // '\0' is already present
        return buffer.compute_strlen();
    });
    RED_CHECK_EQUAL(s3, "xyz"_av);

    // set_end_string_ptr()
    s3.delayed_build([](auto buffer) {
        auto array = buffer.chars_without_null_terminated().data();
        *array++ = 'd';
        *array++ = 'e';
        return buffer.set_end_string_ptr(array);
    });
    RED_CHECK_EQUAL(s3, "de"_av);

    // set_buffer_len()
    s3.delayed_build([](auto buffer) {
        auto array = buffer.chars_with_null_terminated();
        std::size_t i = 0;
        array[i++] = 'f';
        array[i++] = 'g';
        array[i++] = 'h';
        array[i++] = '\0';
        return buffer.set_buffer_len(i);
    });
    RED_CHECK_EQUAL(s3, "fgh"_av);

    // set_end_buffer_ptr()
    s3.delayed_build([](auto buffer) {
        auto array = buffer.chars_with_null_terminated().data();
        *array++ = 'i';
        *array++ = 'j';
        *array++ = '\0';
        return buffer.set_end_buffer_ptr(array);
    });
    RED_CHECK_EQUAL(s3, "ij"_av);

    // test that there is no assertion
    s3.delayed_build([&](auto buffer){
        auto array = buffer.chars_with_null_terminated();
        memset(array.data(), 0, array.size());
        return buffer.set_buffer_len(s3.max_capacity());
    });
}

RED_AUTO_TEST_CASE(TestStaticStringTruncated)
{
    static_string<5> s = truncated_bounded_array_view("abcdefg"_sized_av);
    RED_CHECK_EQUAL(s, "abcde"_av);
}
