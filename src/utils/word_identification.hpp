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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/sugar/array_view.hpp"

#include <cstring>


int in(const char * needle, std::initializer_list<const char *> haystack);
int in(const char * needle, array_view<const char * const> haystack);
int ends_with(const char * needle, std::initializer_list<const char *> haystack);
int ends_with(const char * needle, array_view<const char * const> haystack);


inline int in(const char * needle, std::initializer_list<const char *> haystack)
{
    return in(needle, array_view<const char * const>{haystack.begin(), haystack.end()});
}

inline int in(const char * needle, array_view<const char * const> haystack)
{
    int result = 0;
    for (auto h: haystack){
        result++;
        if (strcmp(h, needle) == 0){
            return result;
        }
    }
    return 0;
}

inline int ends_with(const char * needle, std::initializer_list<const char *> haystack)
{
    return in(needle, array_view<const char * const>{haystack.begin(), haystack.end()});
}

inline int ends_with(const char * needle, array_view<const char * const> haystack)
{
    int result = 0;
    int nlen = strlen(needle);
    for (auto h: haystack){
        result++;
        int hlen = strlen(h);
        if (nlen >= hlen && 0 == strcmp(h, needle+nlen-hlen)){
            return result;
        }
    }
    return 0;
}
