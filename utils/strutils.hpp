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

#ifndef _REDEMPTION_UTILS_STRUTILS_HPP_
#define _REDEMPTION_UTILS_STRUTILS_HPP_

#include "log.hpp"

char * in_place_windows_to_linux_newline_convert(char * s) {
    char * src, * dest;

    src = dest = s;

    char * p;
    while ((p = strstr(src, "\r\n"))) {
        const size_t n = p - src;

        if ((dest != src) && n) {
            memmove(dest, src, n);
        }

        dest += n;
        src  =  p + 2;

        *dest = '\n';

        dest++;
    }

    strcpy(dest, src);

    return s;
}

char * in_place_linux_to_windows_newline_convert(char * s, size_t buffer_size) {
    REDASSERT(strlen(s) + 1 /* null character */ <= buffer_size);
    REDASSERT(buffer_size);

    size_t newline_count = 0;
    size_t s_length      = 0;
    for (char * p = s; *p; p++, s_length++) {
        if (*p == '\n') {
            newline_count++;
        }
    }

    size_t estumated_result_length = s_length + newline_count;

    char * src = s + s_length - 1;  // Last character.

    while (estumated_result_length + 1 /* null character */ > buffer_size) {
        estumated_result_length -= ((*src == '\n') ? 2 : 1);
        src--;
    }

    char * dest = s + estumated_result_length;

    *dest = '\0';
    dest--;

    while ((dest != src) && (src >= s)) {
        if ((*src) == '\n') {
            *dest = '\n';
            dest--;
            *dest = '\r';
        }
        else {
            *dest = *src;
        }

        src--;
        dest--;
    }

    return s;
}

#endif  // #ifndef _REDEMPTION_UTILS_STRUTILS_HPP_
