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

    bool use_filler;

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
    Base64()
        : use_filler(true)
    {
        build_encoding_table();
        build_decoding_table();
    }

    ~Base64() {}

    size_t encode(unsigned char * output, size_t output_size,
                  const unsigned char *input, size_t input_length) {

        size_t output_length = 0;
        unsigned int remain = input_length % 3;

        output_length = 4 * ((input_length + 2) / 3);

        if (output_size < output_length)
            return 0;


        unsigned int i, j;

        for (i = 0, j = 0; i < input_length - remain;) {

            uint32_t octet_a = input[i++];
            uint32_t octet_b = input[i++];
            uint32_t octet_c = input[i++];

            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

            output[j++] = this->encoding_table[(triple >> 3 * 6) & 0x3F];
            output[j++] = this->encoding_table[(triple >> 2 * 6) & 0x3F];
            output[j++] = this->encoding_table[(triple >> 6) & 0x3F];
            output[j++] = this->encoding_table[triple & 0x3F];
        }


        if (remain != 0) {
            uint32_t octet_a = input[i++];
            uint32_t octet_b = (remain == 1) ? 0 : input[i];
            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08);
            output[j++] = this->encoding_table[(triple >> 3 * 6) & 0x3F];
            output[j++] = this->encoding_table[(triple >> 2 * 6) & 0x3F];
            if (this->use_filler) {
                output[j++] = (remain == 1) ? '=' : this->encoding_table[(triple >> 6) & 0x3F];
                output[j++] = '=';
            }
            else {
                output_length--;
                if (remain == 1) {
                    output_length--;
                }
                else {
                    output[j++] = this->encoding_table[(triple >> 6) & 0x3F];
                }
            }
        }
        if (output_length < output_size) output[output_length] = 0;
        return output_length;
    }

    size_t decode(unsigned char * output, size_t output_size,
                  const unsigned char *input, size_t input_length) {

        size_t output_length = 0;

        unsigned int remain = input_length % 4;

        if (input_length == 0)
            return 0;
        if (remain == 1) // even if ignoring '=', this case is not possible.
            return 0;

        output_length = (input_length / 4)  * 3;

        bool filler = false;
        if ((remain == 0) && (input[input_length - 2] == '=')) {
            output_length--;
            filler = true;
        }
        if ((remain == 0) && (input[input_length - 1] == '=')) {
            output_length--;
            filler = true;
        }
        if (remain > 1) output_length++;
        if (remain > 2) output_length++;

        if (output_size < output_length)
            return 0;
        unsigned int i, j;
        for (i = 0, j = 0; i < input_length - remain - filler*4;) {

            uint32_t sextet_a = this->decoding_table[input[i++]];
            uint32_t sextet_b = this->decoding_table[input[i++]];
            uint32_t sextet_c = this->decoding_table[input[i++]];
            uint32_t sextet_d = this->decoding_table[input[i++]];

            uint32_t triple = ((sextet_a & 0x3F) << 3 * 6)
                            + ((sextet_b & 0x3F) << 2 * 6)
                            + ((sextet_c & 0x3F) << 6)
                            + ( sextet_d & 0x3F);

            output[j++] = (triple >> 2 * 8) & 0xFF;
            output[j++] = (triple >> 8) & 0xFF;
            output[j++] = triple & 0xFF;
        }

        if ((remain != 0) || filler) {
            uint32_t sextet_a = this->decoding_table[input[i++]];
            uint32_t sextet_b = this->decoding_table[input[i++]];
            uint32_t sextet_c = ((input[i] == '=') || (remain == 2)) ? 0 : this->decoding_table[input[i]];
            uint32_t triple = ((sextet_a & 0x3F) << 3 * 6)
                            + ((sextet_b & 0x3F) << 2 * 6)
                            + ((sextet_c & 0x3F) << 6);

            output[j++] = (triple >> 2 * 8) & 0xFF;
            if (j < output_length) output[j] = (triple >> 8) & 0xFF;
        }
        if (output_length < output_size) output[output_length] = 0;
        return output_length;
    }



    void set_filler() {
        this->use_filler = true;
    }

    void remove_filler() {
        this->use_filler = false;
    }

    bool is_use_filler() {
        return this->use_filler;
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
