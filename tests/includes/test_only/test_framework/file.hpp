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
}

namespace
{
    constexpr auto fsize = redemption_unit_test__::fn_invoker(
        "filesize", redemption_unit_test__::fsize_impl{});

    int int_(int n) { return n; }
    redemption_unit_test__::int_variation int_(redemption_unit_test__::int_variation n) { return n; }
}

#define RED_TEST_FSIZE(filename, len) RED_TEST(::fsize(filename) == ::int_(len));
#define RED_REQUIRE_FSIZE(filename, len) RED_TEST(::fsize(filename) == ::int_(len));
