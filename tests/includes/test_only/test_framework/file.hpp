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
#include <string_view>
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
    using ::append_file_contents;

    template<class T>
    FileContentsError append_file_contents(T const& filename, std::string& buffer)
    {
        return append_file_contents(static_cast<char const*>(filename), buffer);
    }

    constexpr auto fsize = redemption_unit_test__::fn_invoker("filesize", /* NOLINT */
        redemption_unit_test__::fsize_impl{});

    inline int int_(int n) { return n; }
    inline redemption_unit_test__::int_variation int_(redemption_unit_test__::int_variation n) { return n; }
} // namespace tu

#define RED_TEST_FILE_SIZE(filename, len) RED_TEST(::tu::fsize(filename) == ::tu::int_(len))
#define RED_REQUIRE_FILE_SIZE(filename, len) RED_TEST(::tu::fsize(filename) == ::tu::int_(len))

# define RED_TEST_GET_FILE_CONTENTS(lvl, filename) [](auto&& filename_){                \
    std::string s;                                                                      \
    RED_TEST_CONTEXT("filename: " << filename_) {                                       \
        RED_##lvl(::tu::append_file_contents(filename_, s) == FileContentsError::None); \
    }                                                                                   \
    return s;                                                                           \
}(filename)

# define RED_TEST_LEVEL_FILE_CONTENTS(lvl1, filename, content)                        \
[&](auto&& filename_, auto&& expected_){                                              \
    std::string file_contents_;                                                       \
    auto current_count_error = ::redemption_unit_test__::current_count_error();       \
    [[maybe_unused]] std::string_view strctx = #content;                              \
    RED_TEST_CONTEXT("expr: " << #filename <<                                         \
        "\n    filename: " << filename_ << "\n    content: "                          \
    << std::string_view(strctx.data(), strctx.size() > 40 ? 40 : strctx.size())       \
    << (strctx.size() > 40 ? "[...]" : "")) {                                         \
        RED_##lvl1(::tu::append_file_contents(filename_,                              \
            file_contents_) == FileContentsError::None);                              \
        if (current_count_error == ::redemption_unit_test__::current_count_error()) { \
            RED_##lvl1(file_contents_ == expected_);                                  \
        }                                                                             \
    }                                                                                 \
}(filename, content)

# define RED_TEST_FILE_CONTENTS(filename, contents) \
    RED_TEST_LEVEL_FILE_CONTENTS(CHECK, filename, contents)

# define RED_CHECK_FILE_CONTENTS(filename, contents) \
    RED_TEST_LEVEL_FILE_CONTENTS(CHECK, filename, contents)

# define RED_CHECK_GET_FILE_CONTENTS(filename) \
    RED_TEST_GET_FILE_CONTENTS(CHECK, filename)

# define RED_REQUIRE_FILE_CONTENTS(filename, contents) \
    RED_TEST_LEVEL_FILE_CONTENTS(REQUIRE, filename, contents)

# define RED_REQUIRE_GET_FILE_CONTENTS(filename) \
    RED_TEST_GET_FILE_CONTENTS(REQUIRE, filename)
