/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Object used to save rsa_keys.ini information

*/

#if !defined(__RSA_KEYS__)
#define __RSA_KEYS__

#include <fstream>
#include <string.h>
#include <stdint.h>

struct Rsakeys {
    uint8_t pub_exp[4];
    uint8_t pub_mod[64];
    uint8_t pub_sig[64];
    uint8_t pri_exp[64];

    Rsakeys(const char * filename);
    Rsakeys(std::istream & Keymap_stream);
    Rsakeys(){
        const uint8_t pub_exp[4] = { 0x01, 0x00, 0x01, 0x00 };
        const uint8_t pub_mod[64] = {
            0x67, 0xab, 0x0e, 0x6a, 0x9f, 0xd6, 0x2b, 0xa3,
            0x32, 0x2f, 0x41, 0xd1, 0xce, 0xee, 0x61, 0xc3,
            0x76, 0x0b, 0x26, 0x11, 0x70, 0x48, 0x8a, 0x8d,
            0x23, 0x81, 0x95, 0xa0, 0x39, 0xf7, 0x5b, 0xaa,
            0x3e, 0xf1, 0xed, 0xb8, 0xc4, 0xee, 0xce, 0x5f,
            0x6a, 0xf5, 0x43, 0xce, 0x5f, 0x60, 0xca, 0x6c,
            0x06, 0x75, 0xae, 0xc0, 0xd6, 0xa4, 0x0c, 0x92,
            0xa4, 0xc6, 0x75, 0xea, 0x64, 0xb2, 0x50, 0x5b
        };
        const uint8_t pub_sig[64] = {
            0x6a, 0x41, 0xb1, 0x43, 0xcf, 0x47, 0x6f, 0xf1,
            0xe6, 0xcc, 0xa1, 0x72, 0x97, 0xd9, 0xe1, 0x85,
            0x15, 0xb3, 0xc2, 0x39, 0xa0, 0xa6, 0x26, 0x1a,
            0xb6, 0x49, 0x01, 0xfa, 0xa6, 0xda, 0x60, 0xd7,
            0x45, 0xf7, 0x2c, 0xee, 0xe4, 0x8e, 0x64, 0x2e,
            0x37, 0x49, 0xf0, 0x4c, 0x94, 0x6f, 0x08, 0xf5,
            0x63, 0x4c, 0x56, 0x29, 0x55, 0x5a, 0x63, 0x41,
            0x2c, 0x20, 0x65, 0x95, 0x99, 0xb1, 0x15, 0x7c
        };
        const uint8_t pri_exp[64] = {
            0x41, 0x93, 0x05, 0xB1, 0xF4, 0x38, 0xFC, 0x47,
            0x88, 0xC4, 0x7F, 0x83, 0x8C, 0xEC, 0x90, 0xDA,
            0x0C, 0x8A, 0xB5, 0xAE, 0x61, 0x32, 0x72, 0xF5,
            0x2B, 0xD1, 0x7B, 0x5F, 0x44, 0xC0, 0x7C, 0xBD,
            0x8A, 0x35, 0xFA, 0xAE, 0x30, 0xF6, 0xC4, 0x6B,
            0x55, 0xA7, 0x65, 0xEF, 0xF4, 0xB2, 0xAB, 0x18,
            0x4E, 0xAA, 0xE6, 0xDC, 0x71, 0x17, 0x3B, 0x4C,
            0xC2, 0x15, 0x4C, 0xF7, 0x81, 0xBB, 0xF0, 0x03
        };
        memcpy(this->pub_exp, pub_exp, 4);
        memcpy(this->pub_mod, pub_mod, 64);
        memcpy(this->pub_sig, pub_sig, 64);
        memcpy(this->pri_exp, pri_exp, 64);
    }

    void hex_str_to_bin(char* in, uint8_t * out, size_t out_len)
    {
        int in_index;
        int in_len;
        size_t out_index;
        int val;
        char hex[16];

        in_len = strlen(in);
        out_index = 0;
        in_index = 0;
        while (in_index <= (in_len - 4)) {
            if ((in[in_index] == '0') && (in[in_index + 1] == 'x')) {
                hex[0] = in[in_index + 2];
                hex[1] = in[in_index + 3];
                hex[2] = 0;
                if (out_index < out_len) {
                    val = this->htoi(hex);
                    out[out_index] = val;
                }
                out_index++;
            }
            in_index++;
        }
    }

    int htoi(char* str)
    {
        int len;
        int index;
        int rv;
        int val;
        int shift;

        rv = 0;
        len = strlen(str);
        index = len - 1;
        shift = 0;
        while (index >= 0) {
            val = 0;
            switch (str[index]) {
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                val = str[index]-'0';
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                val = str[index]-'a'+10;
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                val = str[index]-'A'+10;
                break;
            }
            rv = rv | (val << shift);
            index--;
            shift += 4;
        }
        return rv;
    }

    private:
        void init(std::istream & Keymap_stream);
};

#endif
