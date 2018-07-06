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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Meng Tan,
              Jennifer Inthavong
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities
*/


#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "utils/unicode_case_conversion.hpp"
#include "utils/sugar/cast.hpp"

enum {
      maximum_length_of_utf8_character_in_bytes = 4
};

// bool UTF32isValid(uint32_t c):
// abstract: Check if some code point is a valid char or not (some UNICODE ranges are forbiden)
// input: c = 32 bits Unicode codepoint
// output: true or false
// static inline bool UTF32isValid(uint32_t c)
// {
//     // Note: FFFE and FFFF are specifically permitted by the
//     // Unicode standard for application internal use, but are not
//     // allowed for interchange.
//     return c < 0xD800 || (c > 0xDFFF && c <= 0x10FFFF);
// }


// Check if some string is valid utf8, zero terminated"
// Returns number of valid bytes
//static inline size_t UTF8Check(const uint8_t * source, size_t len)
//{
//    size_t i = 0;
//    for (; i < len ; i++){
//        uint8_t c = source[i];
//        switch (c >> 4){
//            case 0:
//                // allows control characters
//                if (c == 0){
//                    i++;
//                    goto UTF8Check_exit;
//                }
//            break;
//            case 1: case 2: case 3: case 4: case 5: case 6: case 7:
//            break;
//            case 8: case 9: case 0xA: case 0xB:
//                // either continuation bytes without start byte or 5 or 6 bytes sequence after 0xFX
//                // both cases are errors.
//                goto UTF8Check_exit;
//            /* handle U+0080..U+07FF inline : 2 bytes sequences */
//            case 0xC: case 0xD:
//                if ((i+1 >= len)
//                   ||((source[i]&0xFE) == 0xC0)
//                   ||((source[i+1] >> 6) != 2)){
//                    goto UTF8Check_exit;
//                }
//                i+=1;
//            break;
//             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */
//            case 0xE:
//                if ((i+2 >= len)
//                   ||((source[i+1] >> 6) != 2)
//                   ||((source[i+2] >> 6) != 2)){
//                    goto UTF8Check_exit;
//                }
//                i+=2;
//            break;
//            case 0xF:
//                if ((i+3 >= len)
//                   ||(c > 244)
//                   ||((source[i+1] >> 6) != 2)
//                   ||((source[i+2] >> 6) != 2)
//                   ||((source[i+3] >> 6) != 2)){
//                    goto UTF8Check_exit;
//                }
//                i+=3;
//            break;
//        }
//    }
//UTF8Check_exit:
//    return i;
//}

// UTF8Len assumes input is valid utf8, zero terminated, that has been checked before
static inline size_t UTF8Len(const uint8_t * source)
{
    size_t len = 0;
    uint8_t c = 0;
    for (size_t i = 0 ; 0 != (c = source[i]) ; i++){
        len += ((c >> 6) == 2)?0:1;
    }
    return len;
}

// source_size is size of source in bytes
//static inline void UTF8Upper(uint8_t * source, size_t source_size) {
//    // size_t len = 0;
//    uint8_t c = 0;
//    for (size_t i = 0 ; i < source_size && 0 != (c = source[i]) ; i++){
//        if (c >= 0x61 && c <= 0x7A) {
//            source[i] -= 0x20;
//        }
//    }
//}

// static inline uint8_t findup(uint8_t c) {
//     const uint8_t uppertable[] = { 0x38, 0x49, 0x78, 0x7F, 0x86 };
//     uint8_t i = 0;
//     for (; i < sizeof(uppertable) ; i++) {
//         if (c < uppertable[i]) {
//             break;
//         }
//     }
// }

static inline void UTF16Upper(uint8_t * source, size_t max_len)
{
    for (size_t i = 0 ; i < max_len ; i=i+2){
        unsigned int wc = source[i];
        wc += source[i+1] << 8;

        for (unsigned int  j = 0 ; j < sizeof(lowers)/sizeof(lowers[0]); j++){
            uint16_t c = lowers[j];
            if (wc == c) {
                source[i] = uppers[j] & 0xFF;
                source[i+1] = (uppers[j] >> 8) & 0x00FF;
                break;
            }
        }
    }
}

// static inline void UTF16UpperW(uint8_t * source, size_t max_len) {
//     size_t i_s = 0;
//     for (size_t i = 0 ; i < max_len ; i++){
//         unsigned int wc = source[i_s];
//         wc += source[i_s+1] << 8;
//         // local should be set
//         const unsigned int uwc = towupper(wc);
//         if (uwc != wc) {
//             source[i_s] = uwc & 0xFF;
//             source[i_s+1] = (uwc >> 8) & 0xFF;
//         }
//         i_s += 2;
//     }
// }

// UTF8GetLen find the number of bytes of the len first characters of input.
// It assumes input is valid utf8, zero terminated (that has been checked before).
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

// // UTF8GetFirstCharLen returns the length in bytes of first character of input. It assumes input is valid utf8, zero terminated (that has been checked before).
// static inline size_t UTF8GetFirstCharLen(const uint8_t * source)
// {
//     size_t    len = 0;
//     const uint8_t * p   = source;
//
//     while (*p)
//     {
//         switch (*p >> 6)
//         {
//         case 2:
//             len++;
//             p++;
//             break;
//
//         case 3:
//             if (len)
//             {
//                 return len;
//             }
//             else
//             {
//                 len++;
//                 p++;
//             }
//             break;
//
//         default:
//         case 0:
//             return 1;
//         }
//     }
//
//     return len;
// }

// // UTF8TruncateAtLen assumes input is valid utf8, zero terminated, that has been checked before.
// static inline void UTF8TruncateAtPos(uint8_t * source, size_t len)
// {
//     source[UTF8GetPos(source, len)] = 0;
// }

// UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before
// UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert
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
    size_t end_point = insertion_point + strlen(char_ptr_cast(source+i));
    size_t to_insert_nbbytes = strlen(char_ptr_cast(to_insert));
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

// UTF8Len assumes input is valid utf8, zero terminated, that has been checked before
static inline size_t UTF8StringAdjustedNbBytes(const uint8_t * source, size_t max_len)
{
    size_t adjust_len = 0;
    while (*source) {
        const size_t char_nb_bytes = UTF8CharNbBytes(source);
        if (adjust_len + char_nb_bytes >= max_len) {
            break;
        }

        adjust_len += char_nb_bytes;
        source += char_nb_bytes;
    }

    return adjust_len;
}

// UTF8RemoveOneAtPos assumes input is valid utf8, zero terminated, that has been checked before
static inline void UTF8RemoveOneAtPos(uint8_t * source, size_t len)
{
    len += 1;
    uint8_t c = 0;
    size_t i = 0;
    for (; 0 != (c = source[i]) ; i++){
        len -= ((c >> 6) == 2)?0:1;
        if (len == 0) {
            size_t insertion_point = i;
            size_t end_point = insertion_point + strlen(char_ptr_cast(source+i));
            uint32_t char_len = UTF8CharNbBytes(source+i);
            memmove(source + i, source + i + char_len, end_point - insertion_point + 1 - char_len);
            break;
        }
    }
    return;
}

// UTF8InsertAtPos assumes input is valid utf8, zero terminated, that has been checked before
// UTF8InsertAtPos won't insert anything and return false if modified string buffer does not have enough space to insert
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
                // TODO This is trouble: we may have to use extended UTF16 sequence because the ucode may be more than 16 bits long
                ucode = ((c & 0x07) << 18)|((source[i] & 0x3F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
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
                ucode = ((c & 0x07) << 18)|((source[i] & 0x3F) << 12)|((source[i+1] & 0x3F) << 6)|(source[i+2] & 0x3F);
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

        if (ucode > 0xFFFF) {
            // TODO:We should choose a behavior for ucodes larger than 16 bits:
            //      ignore char, replace by generic char, support multiwords UTF16,
            //      or raise an error if we are really dealing with UCS-2 and
            //      those chars are not allowed.
            //      For now what we do is ignoring that char.
        }
        else {
            if (i_t + 2 > t_len) { goto UTF8toUTF16_exit; }
            target[i_t] = ucode & 0xFF;
            target[i_t + 1] = (ucode >> 8) & 0xFF;
            i_t += 2;
        }
    }
    // write final 0
UTF8toUTF16_exit:
    return i_t;
}


constexpr uint32_t utf8_2_bytes_to_ucs(uint8_t a, uint8_t b) noexcept
{ return ((a & 0x1F) << 6 ) |  (b & 0x3F); }

constexpr uint32_t utf8_3_bytes_to_ucs(uint8_t a, uint8_t b, uint8_t c) noexcept
{ return ((a & 0x0F) << 12) | ((b & 0x3F) << 6) |  (c & 0x3F); }

constexpr uint32_t utf8_4_bytes_to_ucs(uint8_t a, uint8_t b, uint8_t c, uint8_t d) noexcept
{ return ((a & 0x07) << 18) | ((b & 0x3F) << 12) | ((c & 0x3F) << 6) | (d & 0x3F); }

class UTF8toUnicodeIterator
{
    const uint8_t * source;
    uint32_t ucode = 0;

public:
    explicit UTF8toUnicodeIterator(const uint8_t * str)
    : source(str)
    { ++*this; }

    explicit UTF8toUnicodeIterator(const char * str)
    : UTF8toUnicodeIterator(byte_ptr_cast(str))
    {}

    UTF8toUnicodeIterator & operator++()
    {
        this->ucode = *source;
        ++source;
        switch (this->ucode >> 4 ){
            case 0:
            case 1: case 2: case 3:
            case 4: case 5: case 6: case 7:
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */
            case 0xC: case 0xD:
                this->ucode = utf8_2_bytes_to_ucs(this->ucode, source[0]);
                source += 1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */
            case 0xE:
                this->ucode = utf8_3_bytes_to_ucs(this->ucode, source[0], source[1]);
                source += 2;
            break;
            case 0xF:
                this->ucode = utf8_4_bytes_to_ucs(this->ucode, source[0], source[1], source[2]);
                source += 3;
            break;
            // these should never happen on valid UTF8
            case 8: case 9: case 0x0A: case 0x0B:
                ucode = 0;
            break;
        }
        return *this;
    }

    uint32_t operator*() const
    { return this->ucode; }

    uint32_t code() const
    { return this->ucode; }

    uint8_t const * pos() const
    { return this->source; }
};

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

// Return number of UTF8 bytes used to encode UTF16 input
// do not write trailing 0
static inline size_t UTF16toUTF8(const uint16_t * utf16_source, size_t utf16_len, uint8_t * utf8_target, size_t target_len)
{
    size_t i_t = 0;
    for (size_t i = 0 ; i < utf16_len ; i++){
        uint8_t lo = utf16_source[i*2];
        uint8_t hi  = utf16_source[i*2+1];
        if (lo == 0 && hi == 0){
            if ((i_t + 1) > target_len) { break; }
            utf8_target[i_t] = 0;
            i_t++;
            break;
        }

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

// Return number of UTF8 bytes used to encode UTF32 input
// do not write trailing 0
static inline size_t UTF32toUTF8(uint32_t utf32_char, uint8_t * utf8_target, size_t target_len)
{
    size_t i_t = 0;
    uint8_t lo = (utf32_char & 0xffu);
    uint8_t hi = (utf32_char & 0xff00u) >> 8;
    if (lo == 0 && hi == 0){
        if ((i_t + 1) > target_len) { return i_t; }
        utf8_target[i_t] = 0;
        i_t++;
        return i_t;
    }

    if (hi & 0xF8){
        // 3 bytes
        if ((i_t + 3) > target_len) { return i_t; }
        utf8_target[i_t] = 0xE0 | ((hi >> 4) & 0x0F);
        utf8_target[i_t + 1] = 0x80 | ((hi & 0x0F) << 2) | (lo >> 6);
        utf8_target[i_t + 2] = 0x80 | (lo & 0x3F);
        i_t += 3;
    }
    else if (hi || (lo & 0x80)) {
        // 2 bytes
        if ((i_t + 2) > target_len) { return i_t; }
        utf8_target[i_t] = 0xC0 | ((hi << 2) & 0x1C) | ((lo >> 6) & 3);
        utf8_target[i_t + 1] = 0x80 | (lo & 0x3F);
        i_t += 2;
    }
    else {
        if ((i_t + 1) > target_len) { return i_t; }
        utf8_target[i_t] = lo;
        i_t++;
    }
    return i_t;
}

// Copy as many characters from source to dest fitting in dest buffer.
// Returns the number of UTF8 characters copied.
// The destination string will always be 0 terminated (dest_size 0 is forbidden)
// The buffer after final 0 is not padded.

static inline size_t UTF8ToUTF8LCopy(uint8_t * dest, size_t dest_size, const uint8_t * source)
{
    size_t source_len     = strlen(char_ptr_cast(source));
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

static inline size_t get_utf8_char_size(uint8_t const * c) {
    if ((*c >> 3) == 0x1E) {
        return 4;
    }
    if ((*c >> 4) == 0x0E) {
        return 3;
    }
    if ((*c >> 5) == 0x06) {
        return 2;
    }

    return 1;
}

static inline bool is_utf8_string(uint8_t const * s, int length = -1) {
    enum class Stat : uint8_t {
        ASCII,
        FIRST_UTF8_CHAR,
        SECOND_UTF8_CHAR,
        THIRD_UTF8_CHAR,
        FOURTH_UTF8_CHAR
    };

    Stat stat = Stat::ASCII;

    for (uint8_t const * const s_end = ((length >= 0) ? s + length : nullptr);
         *s && (!s_end || (s < s_end)); s++) {
        switch (stat) {
            case Stat::ASCII:
                if (*s <= 0x7F) continue;
                if ((*s >> 6) == 0x3) { stat = Stat::FIRST_UTF8_CHAR; continue; }
                return false;
            break;
            case Stat::FIRST_UTF8_CHAR:
                if ((*s >> 6) == 0x2) { stat = Stat::SECOND_UTF8_CHAR; continue; }
                return false;
            break;
            case Stat::SECOND_UTF8_CHAR:
                if (*s <= 0x7F) { stat = Stat::ASCII; continue; }
                if ((*s >> 6) == 0x2) { stat = Stat::THIRD_UTF8_CHAR; continue; }
                return false;
            break;
            case Stat::THIRD_UTF8_CHAR:
                if (*s <= 0x7F) { stat = Stat::ASCII; continue; }
                if ((*s >> 6) == 0x2) { stat = Stat::FOURTH_UTF8_CHAR; continue; }
                return false;
            break;
            case Stat::FOURTH_UTF8_CHAR:
                if (*s <= 0x7F) { stat = Stat::ASCII; continue; }
                return false;
            break;
        }
    }

    return (stat == Stat::ASCII);
}

static inline size_t UTF16toLatin1(const uint8_t * utf16_source_, size_t utf16_len, uint8_t * latin1_target, size_t latin1_len) {

    utf16_len &= ~1;

    auto converter = [](uint16_t src, uint8_t * dst) -> bool {
        if ((src < 0x0080) || ((src > 0x9F) && (src < 0x100))) {
            *dst = src;
            return true;
        }

        static struct UTF16ToLatin1Pair {
            uint16_t utf16;
            uint8_t  latin1;
        } UTF16ToLatin1LUT[] = {
            { 0x0081, 0x81 }, { 0x008D, 0x8D }, { 0x008F, 0x8F }, { 0x0090, 0x90 },
            { 0x009D, 0x9D }, { 0x0152, 0x8C }, { 0x0153, 0x9C }, { 0x0160, 0x8A },
            { 0x0161, 0x9A }, { 0x0178, 0x9F }, { 0x017D, 0x8E }, { 0x017E, 0x9E },
            { 0x0192, 0x83 }, { 0x02C6, 0x88 }, { 0x02DC, 0x98 }, { 0x2013, 0x96 },
            { 0x2014, 0x97 }, { 0x2018, 0x91 }, { 0x2019, 0x92 }, { 0x201A, 0x82 },
            { 0x201C, 0x93 }, { 0x201D, 0x94 }, { 0x201E, 0x84 }, { 0x2020, 0x86 },
            { 0x2021, 0x87 }, { 0x2022, 0x95 }, { 0x2026, 0x85 }, { 0x2030, 0x89 },
            { 0x2039, 0x8B }, { 0x203A, 0x9B }, { 0x20AC, 0x80 }, { 0x2122, 0x99 }
        };

        if (src > UTF16ToLatin1LUT[sizeof(UTF16ToLatin1LUT) / sizeof(UTF16ToLatin1LUT[0]) - 1].utf16) {
            return false;
        }

        for (unsigned int i = 0; i < sizeof(UTF16ToLatin1LUT) / sizeof(UTF16ToLatin1LUT[0]); i++) {
            if (UTF16ToLatin1LUT[i].utf16 == src) {
                *dst = UTF16ToLatin1LUT[i].latin1;
                return true;
            }
            else if (UTF16ToLatin1LUT[i].utf16 > src) {
                break;
            }
        }

        return false;
    };

    uint8_t  * current_latin1_target = latin1_target;
    for (size_t remaining_utf16_len = utf16_len / 2, remaining_latin1_len = latin1_len;
         remaining_utf16_len && remaining_latin1_len; utf16_source_+=2, remaining_utf16_len--) {
        if (converter(utf16_source_[1]*256+utf16_source_[0], current_latin1_target)) {
            current_latin1_target++;
            remaining_latin1_len--;
        }
    }

    return current_latin1_target - latin1_target;
}

static inline size_t Latin1toUTF16(const uint8_t * latin1_source, size_t latin1_len,
        uint8_t * utf16_target_, size_t utf16_len) {
    uint16_t * utf16_target = reinterpret_cast<uint16_t *>(utf16_target_);

    auto converter = [](uint8_t src, uint16_t *& dst, size_t & remaining_dst_len) -> bool {
        if ((src < 0x80) || (src > 0x9F)) {
            if (src == 0x0A) {
                if (remaining_dst_len > 1) {
                    *dst++ = 0x0D;
                    remaining_dst_len--;
                }
                else {
                    return false;
                }
            }

            *dst++ = src;
            remaining_dst_len--;
            return true;
        }

        uint16_t Latin1ToUTF16LUT[] = {
            0x20AC, 0x0081, 0x201A, 0x0192,
            0x201E, 0x2026, 0x2020, 0x2021,
            0x02C6, 0x2030, 0x0160, 0x2039,
            0x0152, 0x008D, 0x017D, 0x008F,
            0x0090, 0x2018, 0x2019, 0x201C,
            0x201D, 0x2022, 0x2013, 0x2014,
            0x02DC, 0x2122, 0x0161, 0x203A,
            0x0153, 0x009D, 0x017E, 0x0178
        };

        *dst++ = Latin1ToUTF16LUT[src - 0x80];
        remaining_dst_len--;

        return true;
    };

    const uint8_t  * current_latin1_source = latin1_source;
          uint16_t * current_utf16_target  = utf16_target;
    for (size_t remaining_latin1_len = latin1_len, remaining_utf16_len = utf16_len / 2;
         remaining_latin1_len && remaining_utf16_len;
         current_latin1_source++, remaining_latin1_len--) {
        if (!converter(*current_latin1_source, current_utf16_target, remaining_utf16_len)) {
            break;
        }
    }

    return (current_utf16_target - utf16_target) * 2;
}

static inline size_t Latin1toUTF8(
    const uint8_t * latin1_source, size_t latin1_len,
    uint8_t * utf8_target, size_t utf8_len)
{
    auto converter = [](uint8_t src, uint8_t *& dst, size_t & remaining_dst_len) -> bool {
        if (src < 0x80) {
            *dst++ = src;
            remaining_dst_len--;
            return true;
        }

        if (remaining_dst_len < 2) {
            return false;
        }

        *dst++ = (src >> 6) | 0xC0;
        *dst++ = (src & 0x3f) | 0x80;
        remaining_dst_len -= 2;
        return true;
    };

    uint8_t * current_utf8_target = utf8_target;
    for (const uint8_t * latin1_source_end = latin1_source + latin1_len
      ; latin1_source != latin1_source_end; ++latin1_source) {
        if (!converter(*latin1_source, current_utf8_target, utf8_len)) {
            break;
        }
    }

    return (current_utf8_target - utf8_target);
}

static inline size_t UTF16StrLen(const uint8_t * utf16_s) {
//    const uint16_t* utf16_str = reinterpret_cast<const uint16_t*>(utf16_s);

    size_t length = 0;
    for (; *utf16_s || *(utf16_s + 1); utf16_s += 2, length++);

    return length;
}
