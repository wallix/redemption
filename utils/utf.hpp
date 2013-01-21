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

REDOC("Check some string is valid utf8, zero terminated")
static inline bool UTF8Check(const uint8_t * source, size_t len)
{
    for (size_t i = 0 ; i < len ; i++){
        uint8_t c = source[i];
        switch (c >> 4){
            case 0:
            case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            break;
            case 8: case 9: case 0xA: case 0xB:
                // either continuation bytes without start byte or 5 or 6 bytes sequence after 0xFX
                // both cases are errors.
                return false;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD:
                if ((i+1 >= len)
                   ||((source[i]&0xFE) == 0xC0)
                   ||((source[i+1] >> 6) != 2)){
                    return false;
                }
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                if ((i+2 >= len) 
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)){
                    return false;
                }
                i+=2;
            break;
            case 0xF:
                if ((i+3 >= len)
                   ||(c > 244)
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)
                   ||((source[i+3] >> 6) != 2)){
                    return false;
                }
                i+=3;
            break;
        }
    }
}


REDOC("UTF8Len assumes input is valid utf8, zero terminated, that has been checked before")
static inline size_t UTF8Len(const uint8_t * source)
{
    size_t len = 0; 
    uint8_t c = 0;
    for (size_t i = 0 ; 0 != (c = source[i]) ; i++){
        len += ((c >> 6) == 2)?0:1;
    }
    return len;
}

static inline size_t UTF8Len(const char * source)
{
    return UTF8Len(reinterpret_cast<const uint8_t *>(source));
}


struct utf8_str {
    const uint8_t * data;
};

TODO("API may be clearer if we always return a len and an error code in case of failure (error defaulting to 0)"
     "Another option could be to be to return a negative value in cas of error like other C functions")
static inline bool UTF8toUTF16(const uint8_t ** s, size_t s_len, uint8_t ** t, size_t t_len)
{
    bool res = false;
    const uint8_t * source = *s;
    uint8_t * target = *t;
    size_t i_t = 0; 
    uint32_t ucode = 0;
    size_t i = 0;
    for (; i < s_len ; i++){
        unsigned c = source[i];
        switch (c >> 4){
            case 0: case 1: case 2: case 3: 
            case 4: case 5: case 6: case 7:
            ucode = c;
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD: 
                if ((i+1 >= s_len)
                   ||((c & 0xFE) == 0xC0)
                   ||((source[i+1] >> 6) != 2)){
                    goto UTF8toUTF16_exit;
                }
                ucode = ((c & 0x1F) << 6)|(source[i+1] & 0x3F);
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                if ((i+2 >= s_len) 
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)){
                    goto UTF8toUTF16_exit;
                }
                ucode = ((c & 0x0F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=2;
            break;
            case 0xF:
                if ((i+3 >= s_len)
                   ||(c > 244)
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)
                   ||((source[i+3] >> 6) != 2)){
                    goto UTF8toUTF16_exit;
                }
                c = ((c & 0x07) << 18)|((source[i] & 0x3F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=3;
            break;
            case 8: case 9: case 0x0A: case 0x0B:
                goto UTF8toUTF16_exit;
            break;
        }
        if (i_t + 2 > t_len) { goto UTF8toUTF16_exit; }
        target[i_t] = ucode & 0xFF;
        target[i_t + 1] = (ucode >> 8) & 0xFF;
        i_t += 2;
    }
    res = true;
UTF8toUTF16_exit:
    *s = source + i;
    *t = target + i_t;
    return res;
}

TODO("API may be clearer if we always return a len and an error code in case of failure (error defaulting to 0)"
     "Another option could be to be to return a negative value in cas of error like other C functions")
static inline bool UTF8toUnicode(const uint8_t ** s, size_t s_len, uint32_t ** t, size_t t_len)
{
    bool res = false;
    const uint8_t * source = *s;
    uint32_t * target = *t;
    size_t i_t = 0; 
    uint32_t ucode = 0;
    size_t i = 0;
    for (; i < s_len ; i++){
        unsigned c = source[i];
        switch (c >> 4){
            case 0: case 1: case 2: case 3: 
            case 4: case 5: case 6: case 7:
            ucode = c;
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD: 
                if ((i+1 >= s_len)
                   ||((c & 0xFE) == 0xC0)
                   ||((source[i+1] >> 6) != 2)){
                    goto UTF8toUnicode_exit;
                }
                ucode = ((c & 0x1F) << 6)|(source[i+1] & 0x3F);
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                if ((i+2 >= s_len) 
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)){
                    goto UTF8toUnicode_exit;
                }
                ucode = ((c & 0x0F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=2;
            break;
            case 0xF:
                if ((i+3 >= s_len)
                   ||(c > 244)
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)
                   ||((source[i+3] >> 6) != 2)){
                    goto UTF8toUnicode_exit;
                }
                c = ((c & 0x07) << 18)|((source[i] & 0x3F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=3;
            break;
            case 8: case 9: case 0x0A: case 0x0B:
                goto UTF8toUnicode_exit;
            break;
        }
        if (i_t + 1 > t_len) { goto UTF8toUnicode_exit; }
        target[i_t] = ucode;
        i_t += 1;
    }
    res = true;
UTF8toUnicode_exit:
    *s = source + i;
    *t = target + i_t;
    return res;
}

static inline void UTF16toUTF8(const uint8_t ** s, size_t s_len, uint8_t ** t, size_t t_len)
{
    const uint8_t * source = *s;
    uint8_t * target = *t;
    size_t i_t = 0; 
    // naive first implementation, not check for source length, not check for error or invalid sequences
    
    size_t i = 0;
    while (i < s_len){
        uint8_t lo = source[i];
        uint8_t hi  = source[i+1];

        if (hi & 0xF8){
            // 3 bytes
            if ((i_t + 3) > t_len) { break; }
            target[i_t] = 0xE0 | ((hi >> 4) & 0x0F);
            target[i_t + 1] = 0x80 | ((hi & 0x0F) << 2) | (lo >> 6);
            target[i_t + 2] = 0x80 | (lo & 0x3F);
            i_t += 3;
        }
        else if (hi || (lo & 0x80)) {
            // 2 bytes
            if ((i_t + 2) > t_len) { break; }
            target[i_t] = 0xC0 | ((hi << 2) & 0x1C) | ((lo >> 6) & 3);
            target[i_t + 1] = 0x80 | (lo & 0x3F);
            i_t += 2;
        }
        else {
            if ((i_t + 1) > t_len) { break; }
            target[i_t] = lo;
            i_t++;
        }
        i+=2;
    }
    *s = source + i;
    *t = target + i_t;
}

#endif
