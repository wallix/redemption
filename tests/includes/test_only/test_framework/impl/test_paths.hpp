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
Copyright (C) Wallix 2010-2020
*/

#include "utils/sugar/algostring.hpp"

#include <boost/test/framework.hpp>

#include <algorithm>
#include <string>
#include <string_view>

#include <cstdlib>

#include <unistd.h>

namespace
{
namespace ut_impl
{

static inline std::string_view tempbase()
{
    static const std::string base = []{
        std::string dirname;
        char const* s = std::getenv("TMPDIR");
        if (s) {
            if (*s) {
                dirname = s;
                if (dirname.back() != '/') {
                    dirname += '/';
                }
            }
        }
        else {
            dirname = "/tmp/";
        }
        char buf[1024*4];
        auto dsz = dirname.size();
        dirname += getcwd(buf, sizeof(buf));
        for (char& c : writable_chars_view(dirname).from_offset(dsz)) {
            if (c == '/') {
                c = ':';
            }
        }
        return dirname;
    }();
    return base;
}

static inline std::string_view test_module_name()
{
    static const std::string name = []{
        std::string modname = boost::unit_test::framework::master_test_suite().p_name.get();
        auto pos = std::find_if(modname.begin(), modname.end(), [](char c) {
            return c != '.' && c != '/';
        });
        pos = std::transform(pos, modname.end(), modname.begin(), [](char c){
            return c == '/' ? '-' : c;
        });
        modname.erase(pos, modname.end());
        return modname;
    }();
    return name;
}

static inline std::string_view compiler_suffix()
{
    return
        "@" RED_PP_STRINGIFY(REDEMPTION_COMP_NAME) "-"
        REDEMPTION_COMP_STRING_VERSION
        #ifdef RED_COMPILE_TYPE
        "@" RED_PP_STRINGIFY(RED_COMPILE_TYPE) "__"
        #else
        "__"
        #endif
    ;
}

template<class... Strs>
std::string compute_test_path(Strs const&... strs)
{
    return str_concat(
        tempbase(),
        '@',
        boost::unit_test::framework::current_test_case().p_name.get(),
        '@',
        test_module_name(),
        compiler_suffix(),
        strs...
    );
}

} // namespace ut_impl
} // anonymous namespace
