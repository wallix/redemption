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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/get_file_contents.hpp"
#include "utils/fileutils.hpp"

namespace redemption_unit_test__
{
    struct fsize_impl
    {
        int operator()(char const* s) const noexcept
        {
            return filesize(s);
        }

        template<class T>
        int operator()(T const& s) const noexcept
        {
            return filesize(s.c_str());
        }
    };

    constexpr auto fsize = fn_invoker("filesize", fsize_impl{});

    inline int int_(int n) { return n; }
    inline int_variation int_(int_variation n) { return n; }
}

#define RED_TEST_FSIZE(filename, len) RED_TEST(::redemption_unit_test__::fsize(filename) == ::redemption_unit_test__::int_(len));
#define RED_REQUIRE_FSIZE(filename, len) RED_TEST(::redemption_unit_test__::fsize(filename) == ::redemption_unit_test__::int_(len));

# define RED_TEST_FCONTENTS(lvl, filename, contents)            \
    [](auto&& filename__, auto&& contents__) {                  \
        auto const fcontents__ = get_file_contents(filename__); \
        RED_TEST_CONTEXT("filename: " << filename__) {          \
            RED_##lvl##_SMEM(contents__, fcontents__);          \
        }                                                       \
    }(filename, contents)

# define RED_CHECK_FCONTENTS(filename, contents) \
    RED_TEST_FCONTENTS(CHECK, filename, contents)

# define RED_REQUIRE_FCONTENTS(filename, contents) \
    RED_TEST_FCONTENTS(REQUIRE, filename, contents)
