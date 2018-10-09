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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#include "utils/sugar/multisz.hpp"

#include <algorithm>
#include <cstring>
#include <cassert>


void SOHSeparatedStringsToMultiSZ(char * dest, size_t dest_size, const char * src)
{
    assert(dest_size > 1);

    char const * e = dest + dest_size - 2;
    for (; dest != e && *src; ++dest, ++src) {
        *dest = ('\x01' == *src) ? '\0' : *src;
    }
    memset(dest, 0, e-dest + 2);
}

void MultiSZCopy(char * dest, size_t dest_size, const char * src)
{
    assert(dest_size > 1);

    memset(dest, 0, dest_size);
    size_t total_len = 0;
    for (const char * p = src; *p; ) {
        size_t sz_len = strlen(p);
        if (!sz_len) {
            break;
        }
        sz_len++;
        total_len += sz_len;
        p += sz_len;
    }
    total_len++;
    memcpy(dest, src, std::min(total_len, dest_size - 2));
}
