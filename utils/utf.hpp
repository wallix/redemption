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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities

*/

#if !defined(___UTILS_UTF_HPP__)
#define ___UTILS_UTF_HPP__

#include <stdint.h>
#include "log.hpp"

static inline void UTF8toUTF16(const uint8_t ** s, size_t s_len, uint8_t ** t, size_t t_len)
{
    const uint8_t * source = *s;
    uint8_t * target = *t;
    // naive first implementation, not check for length, not check for error or invalid sequences
    size_t i = 0;
    while (i < s_len){
        unsigned c = source[i];
        i++;
        switch (c >> 4){
            case 0: case 1: case 2: case 3: 
            case 4: case 5: case 6: case 7:
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD: 
                c = ((c & 0x1F) << 6) | (source[i] & 0x3F);
                i++;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                c = (c << 12)|((source[i] & 0x3F) << 6)|(source[i+1] & 0x3F);
                i += 2;
            break;
            case 0xF0:
                LOG(LOG_ERR, "complicated cases not yet supported");
                goto UTF8toUTF16_exit;
            break;
        }
        *target = c & 0xFF;
        target++;
        *target = (c >> 8) & 0xFF;
        target++;
    }
UTF8toUTF16_exit:
    *s = source + i;
    *t = target;
}

static inline void UTF16toUTF8(const uint8_t ** s, size_t s_len, uint8_t ** t, size_t t_len)
{
    const uint8_t * source = *s;
    uint8_t * target = *t;
    // naive first implementation, not check for length, not check for error or invalid sequences
    
    size_t i = 0;
    while (i < s_len){
        uint8_t lo = source[i];
        uint8_t hi  = source[i+1];

        if (hi & 0xF8){
            // 3 bytes
            *target = 0xE0 | ((hi >> 4) & 0x0F);
            target++;
            *target = 0x80 | ((hi & 0x0F) << 2) | (lo >> 6);
            target++;
            *target = 0x80 | (lo & 0x3F);
        }
        else if (hi || (lo & 0x80)) {
            // 2 bytes
            *target = 0xC0 | ((hi << 2) & 0x1C) | ((lo >> 6) & 3);
            target++;
            *target = 0x80 | (lo & 0x3F);
        }
        else {
            *target = lo;
        }
        target++;
        i+=2;
    }
    *s = source + i;
    *t = target;
}

#endif
