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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "utils/base64.hpp"

bytes_view base64_encode(const_bytes_view data, bytes_view output) noexcept
{
    char const* encoding_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    using size_t = std::size_t;

    size_t const remain = data.size() % 3;
    size_t const output_length = base64_encode_size(data.size());

    if (output.size() < output_length)
    {
        return {};
    }

    size_t i, j;

    for (i = 0, j = 0; i < data.size() - remain;)
    {
        uint32_t a = data[i++];
        uint32_t b = data[i++];
        uint32_t c = data[i++];

        uint32_t triple = (a << 0x10) + (b << 0x08) + c;

        output[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        output[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        output[j++] = encoding_table[(triple >> 6) & 0x3F];
        output[j++] = encoding_table[triple & 0x3F];
    }

    if (remain)
    {
        uint32_t a = data[i++];
        uint32_t b = (remain == 1) ? 0 : data[i];
        uint32_t triple = (a << 0x10) + (b << 0x08);
        output[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        output[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        output[j++] = (remain == 1) ? '=' : encoding_table[(triple >> 6) & 0x3F];
        output[j++] = '=';
    }

    return output.subarray(0, output_length);
}

// size_t unbase64(char *buffer, size_t bufsiz, const char *txt)
// {
//     const uint8_t _base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//     unsigned int bits = 0;
//     int nbits = 0;
//     char base64tbl[256];
//     size_t nbytes = 0;
//
//     memset(base64tbl, -1, sizeof base64tbl);
//
//     for (unsigned i = 0; _base64chars[i]; i++) {
//         base64tbl[_base64chars[i]] = i;
//     }
//
//     base64tbl[int('.')] = 62;
//     base64tbl[int('-')] = 62;
//     base64tbl[int('_')] = 63;
//
//     for (; *txt; ++txt) {
//         char const v = base64tbl[static_cast<uint8_t>(*txt)];
//         if (v >= 0) {
//             bits <<= 6;
//             bits += v;
//             nbits += 6;
//             if (nbits >= 8) {
//                 if (nbytes < bufsiz) {
//                     *buffer++ = (bits >> (nbits - 8));
//                 }
//                 nbytes++;
//                 nbits -= 8;
//             }
//         }
//     }
//
//     return nbytes;
// }
