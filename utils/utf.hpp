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

#ifndef _REDEMPTION_UTILS_UTF_HPP_
#define _REDEMPTION_UTILS_UTF_HPP_

#include <stdint.h>
#include "log.hpp"

REDOC("Check if some string is valid utf8, zero terminated")
static inline size_t UTF8Check(const uint8_t * source, size_t len)
{
    size_t i = 0;
    for (; i < len ; i++){
        uint8_t c = source[i];
        switch (c >> 4){
            case 0:
                // allows control characters
                if (c == 0){
                    i++;
                    goto UTF8Check_exit;
                }
            break;
            case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            break;
            case 8: case 9: case 0xA: case 0xB:
                // either continuation bytes without start byte or 5 or 6 bytes sequence after 0xFX
                // both cases are errors.
                goto UTF8Check_exit;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD:
                if ((i+1 >= len)
                   ||((source[i]&0xFE) == 0xC0)
                   ||((source[i+1] >> 6) != 2)){
                    goto UTF8Check_exit;
                }
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                if ((i+2 >= len) 
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)){
                    goto UTF8Check_exit;
                }
                i+=2;
            break;
            case 0xF:
                if ((i+3 >= len)
                   ||(c > 244)
                   ||((source[i+1] >> 6) != 2)
                   ||((source[i+2] >> 6) != 2)
                   ||((source[i+3] >> 6) != 2)){
                    goto UTF8Check_exit;
                }
                i+=3;
            break;
        }
    }
UTF8Check_exit:
    return i;
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


REDOC("UTF8GetLen find the number of bytes of the len first characters of input. It assumes input is valid utf8, zero terminated (that has been checked before).")
static inline size_t UTF8GetPos(uint8_t * source, size_t len)
{
    len += 1;
    uint8_t c = 0;
    size_t i = 0;
    for (; 0 != (c = source[i]) ; i++){
        len -= ((c >> 6) == 2)?0:1;
        if (len == 0) {
            break;
        }
    }
    return i;
}

REDOC("UTF8TruncateAtLen assumes input is valid utf8, zero terminated, that has been checked before.")
static inline void UTF8TruncateAtPos(uint8_t * source, size_t len)
{
    source[UTF8GetPos(source, len)] = 0;
}

static inline void UTF8TruncateAtPos(char * source, size_t len)
{
    UTF8TruncateAtPos(reinterpret_cast<uint8_t *>(source), len);
}


REDOC("UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before")
REDOC("UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert")
static inline bool UTF8InsertAtPos(uint8_t * source, size_t len, const uint8_t * to_insert, size_t max_source)
{
    len += 1;
    uint8_t c = 0;
    size_t i = 0;
    for (; 0 != (c = source[i]) ; i++){
        len -= ((c >> 6) == 2)?0:1;
        if (len == 0) { break; }
    }
    

    size_t insertion_point = i;
    size_t end_point = insertion_point + strlen(reinterpret_cast<char *>(source+i));
    size_t to_insert_nbbytes = strlen(reinterpret_cast<const char *>(to_insert));
    if (end_point + to_insert_nbbytes + 1 > max_source){
        return false;
    }
    
    memmove(source + insertion_point + to_insert_nbbytes, source + insertion_point, end_point - insertion_point + 1);
    memcpy(source + insertion_point, to_insert, to_insert_nbbytes);
    return true;
}


REDOC("UTF8Len assumes input is valid utf8, zero terminated, that has been checked before")
TODO("Naive immplementation, not working for complex cases")
static inline size_t UTF8CharNbBytes(const uint8_t * source)
{
    uint8_t c = *source;
    return (c<=0x7F)?1:(c<=0xDF)?2:(c<=0xEF)?3:4;
}

REDOC("UTF8RemoveOneAtPos assumes input is valid utf8, zero terminated, that has been checked before")
static inline void UTF8RemoveOneAtPos(uint8_t * source, size_t len)
{
    len += 1;
    uint8_t c = 0;
    size_t i = 0;
    for (; 0 != (c = source[i]) ; i++){
        len -= ((c >> 6) == 2)?0:1;
        if (len == 0) {
            size_t insertion_point = i;
            size_t end_point = insertion_point + strlen(reinterpret_cast<char *>(source+i));
            uint32_t char_len = UTF8CharNbBytes(source+i);
            memmove(source + i, source + i + char_len, end_point - insertion_point + 1 - char_len);
            break; 
        }
    }
    return;
}


REDOC("UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before")
REDOC("UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert")
static inline bool UTF8InsertOneAtPos(uint8_t * source, size_t len, const uint32_t to_insert_char, size_t max_source)
{
    uint8_t lo = to_insert_char & 0xFF;
    uint8_t hi  = (to_insert_char >> 8) & 0xFF;
    uint8_t to_insert[4];

    if (hi & 0xF8){
        // 3 bytes
        to_insert[0] = 0xE0 | ((hi >> 4) & 0x0F);
        to_insert[1] = 0x80 | ((hi & 0x0F) << 2) | (lo >> 6);
        to_insert[2] = 0x80 | (lo & 0x3F);
        to_insert[3] = 0;
    }
    else if (hi || (lo & 0x80)) {
        // 2 bytes
        to_insert[0] = 0xC0 | ((hi << 2) & 0x1C) | ((lo >> 6) & 3);
        to_insert[1] = 0x80 | (lo & 0x3F);
        to_insert[2] = 0;
    }
    else {
        to_insert[0] = lo;
        to_insert[1] = 0;
    }
    return UTF8InsertAtPos(source, len, to_insert, max_source);
}

struct utf8_str {
    const uint8_t * data;
};

// UTF8toUTF16 never writes the trailing zero
static inline size_t UTF8toUTF16(const uint8_t * source, uint8_t * target, size_t t_len)
{
    size_t i_t = 0; 
    uint32_t ucode = 0;
    unsigned c = 0;
    for (size_t i = 0; (ucode = c = source[i]) != 0 ; i++){
        switch (c >> 4){
            case 0:
                // allows control characters
                if (c == 0){
                    // should never happen, catched by test above
                    goto UTF8toUTF16_exit;
                }

                ucode = c;
            break;
            case 1: case 2: case 3: 
            case 4: case 5: case 6: case 7:
            ucode = c;
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD: 
                ucode = ((c & 0x1F) << 6)|(source[i+1] & 0x3F);
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                ucode = ((c & 0x0F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=2;
            break;
            case 0xF:
                c = ((c & 0x07) << 18)|((source[i] & 0x3F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=3;
            break;
            case 8: case 9: case 0x0A: case 0x0B:
                // should never happen on valid UTF8
                goto UTF8toUTF16_exit;
            break;
        }
        if (i_t + 2 > t_len) { goto UTF8toUTF16_exit; }
        target[i_t] = ucode & 0xFF;
        target[i_t + 1] = (ucode >> 8) & 0xFF;
        i_t += 2;
    }
    // write final 0
UTF8toUTF16_exit:
    return i_t;
}

// UTF8toUnicode never writes the trailing zero
static inline size_t UTF8toUnicode(const uint8_t * source, uint32_t * target, size_t t_len)
{
    size_t i_t = 0;
    uint32_t ucode = 0;
    size_t i = 0;
    for (; (ucode = source[i]) != 0 ; i++){
        switch (ucode >> 4){
            case 0:
                // should never happen, catched by test above
                goto UTF8toUnicode_exit;
            break;
            case 1: case 2: case 3: 
            case 4: case 5: case 6: case 7:
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0xC: case 0xD: 
                ucode = ((ucode & 0x1F) << 6)|(source[i+1] & 0x3F);
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xE:
                ucode = ((ucode & 0x0F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
                i+=2;
            break;
            case 0xF:
                ucode = ((ucode & 0x07) << 18)|((source[i+1] & 0x3F) << 12)|((source[i+2] & 0x3F) << 6)|(source[i+3] & 0x3F);
                i+=3;
            break;
            case 8: case 9: case 0x0A: case 0x0B:
                // these should never happen on valid UTF8
                goto UTF8toUnicode_exit;
            break;
        }
        if (i_t + 1 > t_len) { goto UTF8toUnicode_exit; }
        target[i_t] = ucode;
        i_t += 1;
    }
    // write final 0
UTF8toUnicode_exit:
    return i_t;
}

TODO("API may be clearer if we always return a len and an error code in case of failure (error defaulting to 0)"
     "Another option could be to be to return a negative value in cas of error like other C functions")
static inline bool UTF8toUnicodeWithCheck(const uint8_t ** s, size_t s_len, uint32_t ** t, size_t t_len)
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

// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
static inline size_t UTF16toUTF8(const uint8_t * utf16_source, size_t utf16_len, uint8_t * utf8_target, size_t target_len)
{
    size_t i_t = 0; 
    size_t i_s = 0;
    for (size_t i = 0 ; i < utf16_len ; i++){
        uint8_t lo = utf16_source[i_s];
        uint8_t hi  = utf16_source[i_s+1];
        if (lo == 0 && hi == 0){
            if ((i_t + 1) > target_len) { break; }
            utf8_target[i_t] = 0;
            i_t++;
            break;
        }
        i_s += 2;

        if (hi & 0xF8){
            // 3 bytes
            if ((i_t + 3) > target_len) { break; }
            utf8_target[i_t] = 0xE0 | ((hi >> 4) & 0x0F);
            utf8_target[i_t + 1] = 0x80 | ((hi & 0x0F) << 2) | (lo >> 6);
            utf8_target[i_t + 2] = 0x80 | (lo & 0x3F);
            i_t += 3;
        }
        else if (hi || (lo & 0x80)) {
            // 2 bytes
            if ((i_t + 2) > target_len) { break; }
            utf8_target[i_t] = 0xC0 | ((hi << 2) & 0x1C) | ((lo >> 6) & 3);
            utf8_target[i_t + 1] = 0x80 | (lo & 0x3F);
            i_t += 2;
        }
        else {
            if ((i_t + 1) > target_len) { break; }
            utf8_target[i_t] = lo;
            i_t++;
        }
    }
    return i_t;
}

#endif
