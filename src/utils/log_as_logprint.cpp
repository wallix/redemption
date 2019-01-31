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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#include "cxx/diagnostic.hpp"
#include "cxx/compiler_version.hpp"
REDEMPTION_DIAGNOSTIC_PUSH
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
#endif
#include "utils/log.hpp"
REDEMPTION_DIAGNOSTIC_POP

#ifdef __EMSCRIPTEN__
# include "red_emscripten/em_asm.hpp"
#endif

#include <cstdarg>
#include <cstdio>

void LOG__REDEMPTION__INTERNAL__IMPL(int priority, char const * format, ...) /*NOLINT(cert-dcl50-cpp)*/
{
    (void)priority;
    va_list ap;
    va_start(ap, format);

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
#ifdef __EMSCRIPTEN__
    char buffer[4096];
    int len = std::vsnprintf(buffer, sizeof(buffer)-2, format, ap); /*NOLINT*/
    va_end(ap);
    buffer[len] = 0;
    RED_EM_ASM({console.log(Pointer_stringify($0));}, buffer);
#else
    std::vprintf(format, ap); /*NOLINT*/
    std::puts("");
#endif
    REDEMPTION_DIAGNOSTIC_POP

    va_end(ap);
}
