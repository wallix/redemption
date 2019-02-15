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

#if !defined(NDEBUG) \
 && !defined(REDEMPTION_NO_STACKTRACE) \
 && __has_include(<boost/stacktrace.hpp>)

# define REDEMPTION_WITH_STACKTRACE

# include <boost/stacktrace.hpp>

namespace red
{
    template<class F>
    void print_stacktrace(F&& f)
    {
        int i = 0;
#       ifdef BOOST_STACKTRACE_DYN_LINK
        bool is_test = false;
#       endif
        auto&& frames = boost::stacktrace::stacktrace();
        auto&& first = frames.begin();
        auto&& last = frames.end();
        if (first == last) {
            return ;
        }
        while (++first != last) {
            auto&& frame = *first;
            if (!frame.empty()){
#               ifdef BOOST_STACKTRACE_DYN_LINK
                auto&& file = frame.source_file();
                if (0 == file.compare(0, 6, "tests/")) {
                    is_test = true;
                }
                else if (is_test) {
                    // abort stacktrace
                    break;
                }
#               endif
                f(i, boost::stacktrace::to_string(frame));
                ++i;
            }
        }
        // std::cerr << boost::stacktrace::stacktrace() << std::flush;
    }
} // namespace
#else
namespace red
{
    template<class F>
    void print_stacktrace(F&&)
    {}
}
#endif
