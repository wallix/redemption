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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Meng Tan

   Base64 converter
*/


#ifndef _REDEMPTION_UTILS_BASE64_HPP_
#define _REDEMPTION_UTILS_BASE64_HPP_


#include <stdint.h>
#include "error.hpp"
#include "log.hpp"

class Base64 {
    unsigned char encoding_table[64];

    unsigned char decoding_table[256];

    void build_encoding_table() {
        unsigned char code = 'A';
        unsigned int i = 0;
        for (unsigned int j = 0; j < 26; j++)
            encoding_table[i+j] = code++;
        code = 'a';
        i += 26;
        for (unsigned int j = 0; j < 26; j++)
            encoding_table[i+j] = code++;
        code = '0';
        i += 26;
        for (unsigned int j = 0; j < 10; j++)
            encoding_table[i+j] = code++;
        i += 10;
        encoding_table[i] = '+';
        encoding_table[++i] = '/';

        // encoding_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    }

    void build_decoding_table() {
        for (unsigned int i = 0; i < 64; i++)
            decoding_table[(unsigned char) encoding_table[i]] = i;
        // for base64url decoding only
        decoding_table[(unsigned char) '-'] = 62;
        decoding_table[(unsigned char) '_'] = 63;
    }

public:
    Base64() {
        build_encoding_table();
        build_decoding_table();
    }
    ~Base64() {}

    size_t encode(unsigned char * output, size_t output_size, const unsigned char *input,
                  size_t input_length) {

        size_t output_length = 0;

        output_length = 4 * ((input_length + 2) / 3);
        if (output_size < output_length)
            return 0;

        for (unsigned int i = 0, j = 0; i < input_length;) {

            uint32_t octet_a = i < input_length ? input[i++] : 0;
            uint32_t octet_b = i < input_length ? input[i++] : 0;
            uint32_t octet_c = i < input_length ? input[i++] : 0;

            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

            output[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
            output[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
            output[j++] = encoding_table[(triple >> 6) & 0x3F];
            output[j++] = encoding_table[(triple >> 0) & 0x3F];
        }

        switch (input_length % 3) {
        case 1:
            output[output_length - 2] = '=';
        case 2:
            output[output_length - 1] = '=';
        }

        return output_length;
    }


    size_t decode(unsigned char * output, size_t output_size, const unsigned char *input,
                         size_t input_length) {

        size_t output_length = 0;

        if (input_length % 4 != 0)
            return 0;

        output_length = input_length / 4 * 3;

        if (input[input_length - 1] == '=')
            output_length--;
        if (input[input_length - 2] == '=')
            output_length--;

        if (output_size < output_length)
            return 0;

        for (unsigned int i = 0, j = 0; i < input_length;) {

            uint32_t sextet_a = input[i] == '=' ? 0 & i++ : decoding_table[input[i++]];
            uint32_t sextet_b = input[i] == '=' ? 0 & i++ : decoding_table[input[i++]];
            uint32_t sextet_c = input[i] == '=' ? 0 & i++ : decoding_table[input[i++]];
            uint32_t sextet_d = input[i] == '=' ? 0 & i++ : decoding_table[input[i++]];

            uint32_t triple = ((sextet_a & 0x3F) << 3 * 6)
                            + ((sextet_b & 0x3F) << 2 * 6)
                            + ((sextet_c & 0x3F) << 6)
                            + ((sextet_d & 0x3F) << 0);

            if (j < output_length) output[j++] = (triple >> 2 * 8) & 0xFF;
            if (j < output_length) output[j++] = (triple >> 8) & 0xFF;
            if (j < output_length) output[j++] = (triple >> 0) & 0xFF;
        }

        return output_length;
    }

    void mode_url() {
        encoding_table[62] = '-';
        encoding_table[63] = '_';
        decoding_table[(unsigned char) '-'] = 62;
        decoding_table[(unsigned char) '_'] = 63;
    }

    void mode_normal() {
        encoding_table[62] = '+';
        encoding_table[63] = '/';
    }
};

#endif
