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
#include "utils/fileutils.hpp"
#include <cstring>


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
} // namespace redemption_unit_test__

namespace tu
{
    inline std::string get_file_contents(const char * filename)
    {
        std::string s;
        auto append_file_contents = RED_TEST_FUNC_CTX(::append_file_contents);
        RED_TEST(append_file_contents(filename, s) == FileContentsError::None);
        return s;
    }

    inline std::string get_file_contents(std::string const& name)
    {
        return get_file_contents(name.c_str());
    }

    template<class T>
    std::string get_file_contents(T const& name)
    {
        return get_file_contents(static_cast<char const*>(name));
    }

    constexpr auto fsize = redemption_unit_test__::fn_invoker("filesize", /* NOLINT */
        redemption_unit_test__::fsize_impl{});

    inline int int_(int n) { return n; }
    inline redemption_unit_test__::int_variation int_(redemption_unit_test__::int_variation n) { return n; }
} // namespace tu

#define RED_TEST_FILE_SIZE(filename, len) RED_TEST(::tu::fsize(filename) == ::tu::int_(len));
#define RED_REQUIRE_FILE_SIZE(filename, len) RED_TEST(::tu::fsize(filename) == ::tu::int_(len));

# define RED_TEST_FILE_CONTENTS(lvl, filename, contents)                     \
    [](auto&& filename__, auto&& contents__) {                               \
        RED_TEST_CONTEXT("filename: " << filename__) {                       \
            std::string file_contents__ = tu::get_file_contents(filename__); \
            RED_##lvl##_SMEM(contents__, file_contents__);                   \
        }                                                                    \
    }(filename, contents)

# define RED_CHECK_FILE_CONTENTS(filename, contents) \
    RED_TEST_FILE_CONTENTS(CHECK, filename, contents)

# define RED_REQUIRE_FILE_CONTENTS(filename, contents) \
    RED_TEST_FILE_CONTENTS(REQUIRE, filename, contents)
