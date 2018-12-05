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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#include <string>
#include <stdexcept>

#include "core/error.hpp"
#include "cxx/compiler_version.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"

# ifdef IN_IDE_PARSER
#   include "system/register_exception.hpp"
# else
#   define RED_TEST_INCLUDE(backend, path) <test_only/test_framework/backend/path>
#   include RED_TEST_INCLUDE(REDEMPTION_TEST_BACKEND, register_exception.hpp)
#   undef RED_TEST_INCLUDE
# endif


namespace
{
    struct register_exception
    {
        register_exception()
        {
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
            #if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
                REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
            #endif
            redemption_unit_test__::register_exception_translator<Error>(+[](Error const & e){
                const auto prefix_msg_error = "Exception of type 'Error': "_av;
                if (e.errnum) {
                    throw std::runtime_error{str_concat(
                        prefix_msg_error, e.errmsg(), ", errno=", std::to_string(e.errnum)
                    )};
                }
                throw std::runtime_error{str_concat(prefix_msg_error, e.errmsg())};
            });
            REDEMPTION_DIAGNOSTIC_POP
        }
    };
    const register_exception Init; /*NOLINT*/
} // namespace
