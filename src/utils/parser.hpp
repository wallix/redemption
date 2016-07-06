/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include <string.h>
#include <stdlib.h>

// TODO: there are equivalent functions in parse.hpp, use those from parse

// 10 = 10, 0x10 = 16
static inline unsigned ulong_from_cstr(const char * str) noexcept
{
    if ((*str == '0') && (*(str + 1) == 'x')){
        return strtol(str + 2, nullptr, 16);
    }

    return strtoul(str, nullptr, 10);
}

// 10 = 10, 0x10 = 16
static inline long long_from_cstr(const char * str) noexcept
{
    if ((*str == '0') && (*(str + 1) == 'x')){
        return strtol(str + 2, nullptr, 16);
    }

    return atol(str);
}

static inline signed _long_from_cstr(const char * str) noexcept
{
    return atol(str);
}

// 1, yes, on, true
static inline bool bool_from_cstr(const char * str) noexcept
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

