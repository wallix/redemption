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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities
*/

#ifndef _REDEMPTION_UTILS_UTF_HPP_
#define _REDEMPTION_UTILS_UTF_HPP_

#include <stdint.h>
#define LOGNULL
#include "log.hpp"

// bool UTF32isValid(uint32_t c):
// abstract: Check if some code point is a valid char or not (some UNICODE ranges are forbiden)
// input: c = 32 bits Unicode codepoint
// output: true or false
static inline bool UTF32isValid(uint32_t c)
{
    // Note: FFFE and FFFF are specifically permitted by the
    // Unicode standard for application internal use, but are not
    // allowed for interchange.
    return c < 0xD800 || (c > 0xDFFF && c <= 0x10FFFF);
}


// Check if some string is valid utf8, zero terminated"
// Returns number of valid bytes
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


REDOC("UTF8Len assumes input is valid utf8, zero terminated, that has been checked before");
static inline size_t UTF8Len(const uint8_t * source)
{
    size_t len = 0;
    uint8_t c = 0;
    for (size_t i = 0 ; 0 != (c = source[i]) ; i++){
        len += ((c >> 6) == 2)?0:1;
    }
    return len;
}


REDOC("UTF8GetLen find the number of bytes of the len first characters of input."
      " It assumes input is valid utf8, zero terminated (that has been checked before).");
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

REDOC("UTF8GetFirstCharLen returns the length in bytes of first character of input. It assumes input is valid utf8, zero terminated (that has been checked before).");
static inline size_t UTF8GetFirstCharLen(const uint8_t * source)
{
    size_t    len = 0;
    const uint8_t * p   = source;

    while (*p)
    {
        switch (*p >> 6)
        {
        case 2:
            len++;
            p++;
            break;

        case 3:
            if (len)
            {
                return len;
            }
            else
            {
                len++;
                p++;
            }
            break;

        default:
        case 0:
            return 1;
        }
    }

    return len;
}

REDOC("UTF8TruncateAtLen assumes input is valid utf8, zero terminated, that has been checked before.");
static inline void UTF8TruncateAtPos(uint8_t * source, size_t len)
{
    source[UTF8GetPos(source, len)] = 0;
}

REDOC("UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before")
REDOC("UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert");
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


 
// UTF8CharNbBytes: 
// ----------------
// input: 'source' is the beginning of a char contained in a valid utf8 zero terminated string.
//        (valid means "that has been checked before". It means we are in a secure context).
// output: number of bytes for 'one' char

static inline size_t UTF8CharNbBytes(const uint8_t * source)
{
    uint8_t c = *source;
    return (c<=0x7F)?1:(c<=0xDF)?2:(c<=0xEF)?3:4;
}

REDOC("UTF8RemoveOneAtPos assumes input is valid utf8, zero terminated, that has been checked before");
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
REDOC("UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert");
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

// UTF8toUTF16 never writes the trailing zero (with Lf to CrLf conversion).
static inline size_t UTF8toUTF16_CrLf(const uint8_t * source, uint8_t * target, size_t t_len)
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

        if ((ucode == 0x0D) && (source[i+1] == 0x0A)){
           continue;
        }
        else if (ucode == 0x0A) {
            if (i_t + 4 /* CrLf en unicode */ > t_len) { goto UTF8toUTF16_exit; }
            target[i_t]     = 0x0D;
            target[i_t + 1] = 0x00;
            target[i_t + 2] = 0x0A;
            target[i_t + 3] = 0x00;
            i_t += 4;

            continue;
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

// Return number of UTF8 bytes used to encode UTF32 input
// do not write trailing 0
static inline size_t UTF32toUTF8(const uint8_t * utf32_source, size_t utf32_len, uint8_t * utf8_target, size_t target_len)
{
    size_t i_t = 0;
    size_t i_s = 0;
    for (size_t i = 0 ; i < utf32_len ; i++){
        uint8_t lo = utf32_source[i_s];
        uint8_t hi = utf32_source[i_s+1];
        if (lo == 0 && hi == 0){
            if ((i_t + 1) > target_len) { break; }
            utf8_target[i_t] = 0;
            i_t++;
            break;
        }
        i_s += 4;

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

// Copy as many characters from source to dest fitting in dest buffer.
// Returns the number of UTF8 characters copied.
// The destination string will always be 0 terminated (dest_size 0 is forbidden)
// The buffer after final 0 is not padded.

static inline size_t UTF8ToUTF8LCopy(uint8_t * dest, size_t dest_size, const uint8_t * source)
{
    size_t source_len     = strlen(reinterpret_cast<const char *>(source));
    if (source_len > dest_size - 1){
        // rule out malformed UTF8 source, we need to check that or the following loop may never end
        if ((source[0] & 0xC0) == 0x80) {
            dest[0] = 0;
            return 0;
        }
        source_len = dest_size - 1;
        while ((source[source_len] & 0xC0) == 0x80){
            source_len--;
        }
        // we have found the beginning of last char
        size_t lg = 0;
        switch (source[source_len] >> 4){
            case 8: case 9: case 0x0A: case 0x0B:
                // these are bogus (continuation and should never happen with valid input)
                break;
            case 0: case 1: case 2: case 3:
            case 4: case 5: case 6: case 7:
                lg = 1;
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */
            case 0xC: case 0xD:
                lg = 2;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */
            case 0xE:
                lg = 3;
            break;
            case 0xF:
                lg = 4;
            break;
        }
        if (source_len + lg < dest_size - 1){
            source_len += lg;
        }
    }
    memcpy(dest, source, source_len);
    dest[source_len] = 0;
    return UTF8Len(dest); // number of char
}

#endif
