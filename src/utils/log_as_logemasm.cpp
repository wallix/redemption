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

#include "utils/log.hpp"

#include <cstdarg>

#ifndef __EMSCRIPTEN__
# define EM_ASM_(...)
#endif

void LOG__REDEMPTION__INTERNAL__IMPL(int /*priority*/, char const * format, ...)
{
    va_list ap;
    va_start(ap, format);
    char buffer[4096];
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
    int len = std::vsnprintf(buffer, sizeof(buffer)-2, format, ap);
    REDEMPTION_DIAGNOSTIC_POP
    va_end(ap);

    buffer[len] = '\n';
    buffer[len+1] = 0;
    EM_ASM_({console.log(Pointer_stringify($0));}, buffer);
}
