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

#include "utils/sugar/split.hpp"
#include "utils/sugar/zstring_view.hpp"
#include <string>

template<class Splitter>
void test_splitter(char const* ctx, Splitter&& splitter, chars_view result)
{
    std::string s;
    s.clear();
    for (auto r : splitter) {
        s.append(r.begin(), r.end()) += ':';
    }

    RED_TEST_CONTEXT(ctx) {
        RED_CHECK_EQUAL(s, result);
    }
}

template<class Chars>
void test_splitters(Chars& str, chars_view result)
{
    test_splitter<
        detail::SplitterCView<detail::SplitterCViewDataStr<char const*>, chars_view>
    >("char const*", split_with(static_cast<char const*>(str), ','), result);

    test_splitter<
        detail::SplitterCView<detail::SplitterCViewDataStr<char*>, writable_chars_view>
    >("char*", split_with(str, ','), result);

    test_splitter<
        detail::SplitterCView<detail::SplitterCViewDataView<char const*>, chars_view>
    >("zstring", split_with(zstring_view::from_null_terminated(str), ','), result);

    test_splitter<
        SplitterView<chars_view, char>
    >("string_view", split_with(std::string_view(str), ','), result);

    struct Char
    {
        bool operator == (char c) const
        {
            return c == ',';
        }
    };

    test_splitter<
        SplitterView<chars_view, Char>
    >("string_view + sep as Fn", split_with(std::string_view(str), Char()), result);
}

RED_AUTO_TEST_CASE(TestSplitter)
{
    {
        char str[] = "abc,de,efg,h,,,ijk,lmn";
        test_splitters(str, "abc:de:efg:h:::ijk:lmn:"_av);
    }

    {
        char str[] = ",abc,de,";
        test_splitters(str, ":abc:de:"_av);
    }

    {
        char str[] = "";
        test_splitters(str, ""_av);
    }

    {
        char str[] = "a";
        test_splitters(str, "a:"_av);
    }

    {
        char str[] = ",";
        test_splitters(str, ":"_av);
    }
}
