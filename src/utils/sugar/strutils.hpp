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

#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"

inline static char * in_place_windows_to_linux_newline_convert(char * s) {
    char * src, * dest;

    REDASSERT(s);

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

    memmove(dest, src, strlen(src) + 1);

    return s;
}

inline static size_t linux_to_windows_newline_convert(char const * s,
        size_t s_length, char * d, size_t max_d_length) {
    size_t d_length = 0;

    while (char const * p = reinterpret_cast<char const *>(memchr(s, '\n', s_length))) {
        size_t l = p - s;

        if (l + 2 /* CRLF(2) */ > max_d_length) {
            LOG(LOG_ERR,
                "linux_to_windows_newline_convert: "
                    "Destination buffer is too small to hold all the result data.");
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }

        if (l) {
            memcpy(d, s, l);

            d_length += l;

            d            += l;
            max_d_length -= l;

            s        += l;
            s_length -= l;
        }

        *d++ = '\r';
        *d++ = '\n';

        d_length += 2 /* CRLF(2) */ ;

        max_d_length -= (l + 2 /* CRLF(2) */);

        s++;    // LF(1)

        s_length--; // LF(1)
    }

    if (s_length > max_d_length) {
        LOG(LOG_ERR,
            "linux_to_windows_newline_convert: "
                "Destination buffer is too small to hold all the result data.");
        throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
    }

    memcpy(d, s, s_length);

    return (d_length + s_length);
}

inline static bool ends_with(const char * str, size_t str_len, const char * suffix, size_t suffix_len) {
    return ((str_len >= suffix_len) && (strcmp(str + str_len - suffix_len, suffix) == 0));
}

inline static bool ends_case_with(const char * str, size_t str_len, const char * suffix, size_t suffix_len) {
    return ((str_len >= suffix_len) && (strcasecmp(str + str_len - suffix_len, suffix) == 0));
}

// A multi-sz contains a sequence of null-terminated strings,
//  terminated by an empty string (\0) so that the last two
//  characters are both null terminators.
inline static void SOHSeparatedStringsToMultiSZ(char * dest, size_t dest_size, const char * src) {
    REDASSERT(dest_size > 1);

    char const * e = src + dest_size - 2;
    for (; src != e && *src; ++dest, ++src) {
        *dest = ('\x01' == *src) ? '\0' : *src;
    }
    memset(dest, 0, e-src + 2);
}

// A multi-sz contains a sequence of null-terminated strings,
//  terminated by an empty string (\0) so that the last two
//  characters are both null terminators.
inline static void MultiSZCopy(char * dest, size_t dest_size, const char * src) {
    REDASSERT(dest_size > 1);

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


