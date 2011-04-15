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

struct Rsakeys {
    char pub_exp[4];
    char pub_mod[64];
    char pub_sig[64];
    char pri_exp[64];

    Rsakeys(const char * filename);
    Rsakeys(std::istream & Keymap_stream);
    Rsakeys(){
    }

    void hex_str_to_bin(char* in, char* out, int out_len)
    {
        int in_index;
        int in_len;
        int out_index;
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
