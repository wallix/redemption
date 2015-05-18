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

#include "error.hpp"
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

    memmove(dest, src, strlen(src) + 1);

    return s;
}

//char * in_place_linux_to_windows_newline_convert(char * s, size_t buffer_size,
//                                                 bool cancel_if_buffer_too_small,
//                                                 size_t & out_result_size) {
//    REDASSERT(strlen(s) + 1 /* null character */ <= buffer_size);
//    REDASSERT(buffer_size);
//
//    size_t newline_count = 0;
//    size_t s_length      = 0;
//    for (char * p = s; *p; p++, s_length++) {
//        if (*p == '\n') {
//            newline_count++;
//        }
//    }
//
//    size_t estumated_result_length = s_length + newline_count;
//    out_result_size = estumated_result_length + 1;
//    if (cancel_if_buffer_too_small && ((estumated_result_length + 1) > buffer_size)) {
//        return nullptr;
//    }
//
//    char * src = s + s_length - 1;  // Last character.
//
//    while (estumated_result_length + 1 /* null character */ > buffer_size) {
//        estumated_result_length -= ((*src == '\n') ? 2 : 1);
//        src--;
//    }
//
//    char * dest = s + estumated_result_length;
//
//    *dest = '\0';
//    dest--;
//
//    while ((dest != src) && (src >= s)) {
//        if ((*src) == '\n') {
//            *dest = '\n';
//            dest--;
//            *dest = '\r';
//        }
//        else {
//            *dest = *src;
//        }
//
//        src--;
//        dest--;
//    }
//
//    return s;
//}

size_t linux_to_windows_newline_convert(char const * s, size_t s_length, char * d, size_t max_d_length) {
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

#endif  // #ifndef _REDEMPTION_UTILS_STRUTILS_HPP_
