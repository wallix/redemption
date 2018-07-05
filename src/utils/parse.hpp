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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean

   wrapper class around byte pointer used to parse
*/


#pragma once

#include "utils/sugar/cast.hpp"

#include <cstdint> // for sized types
#include <cstring> // for memcpy
#include <cstdlib> // strto*

class Parse {
public:
    uint8_t const * p = nullptr;

public:
    Parse() = default;
    explicit Parse(uint8_t const * p) : p(p) {}

    void unget(size_t n)
    {
        this->p -= n;
    }

    int8_t in_sint8(void) {
        return *(reinterpret_cast<int8_t const *>(this->p++));
    }

    uint8_t in_uint8(void) {
        return *this->p++;
    }

    int16_t in_sint16_be(void) {
        unsigned int v = this->in_uint16_be();
        return static_cast<int16_t>(v > 32767 ? v - 65536 : v);
    }

    int16_t in_sint16_le(void) {
        unsigned int v = this->in_uint16_le();
        return static_cast<int16_t>(v > 32767 ? v - 65536 : v);
    }

    uint16_t in_uint16_le(void) {
        this->p += 2;
        return static_cast<uint16_t>(this->p[-2] | (this->p[-1] << 8));
    }

    uint16_t in_uint16_be(void) {
        this->p += 2;
        return static_cast<uint16_t>(this->p[-1] | (this->p[-2] << 8)) ;
    }

    uint32_t in_uint32_le(void) {
        this->p += 4;
        return  this->p[-4]
             | (this->p[-3] << 8)
             | (this->p[-2] << 16)
             | (this->p[-1] << 24)
             ;
    }

    uint32_t in_uint32_be(void) {
        this->p += 4;
        return  this->p[-1]
             | (this->p[-2] << 8)
             | (this->p[-3] << 16)
             | (this->p[-4] << 24)
             ;
    }

    int32_t in_sint32_le(void) {
        uint64_t v = this->in_uint32_le();
        return static_cast<int32_t>((v > 0x7FFFFFFF) ? v - 0x100000000LL : v);
    }

    int32_t in_sint32_be(void) {
        uint64_t v = this->in_uint32_be();
        return static_cast<int32_t>((v > 0x7FFFFFFF) ? v - 0x100000000LL : v);
    }

    int64_t in_sint64_le(void) {
        int64_t res;
        *(reinterpret_cast<uint64_t *>(&res)) = this->in_uint64_le();
        return res;
    }

    uint64_t in_uint64_le(void) {
        uint64_t low = this->in_uint32_le();
        uint64_t high = this->in_uint32_le();
        return low + (high << 32);
    }

    uint64_t in_uint64_be(void) {
        uint64_t high = this->in_uint32_be();
        uint64_t low = this->in_uint32_be();
        return low + (high << 32);
    }

    uint32_t in_bytes_le(const uint8_t nb){
        uint32_t res = 0;
        for (int b = 0 ; b < nb ; ++b){
            res |= this->p[b] << (8 * b);
        }
        this->p += nb;
        return res;
    }


    uint32_t in_bytes_be(const uint8_t nb){
        uint32_t res = 0;
        for (int b = 0 ; b < nb ; ++b){
            res = (res << 8) | this->p[b];
        }
        this->p += nb;
        return res;
    }

    void in_copy_bytes(uint8_t * v, size_t n) {
        memcpy(v, this->p, n);
        this->p += n;
    }

    void in_copy_bytes(char * v, size_t n) {
        this->in_copy_bytes(byte_ptr_cast(v), n);
    }

    const uint8_t *in_uint8p(unsigned int n) {
        this->p+=n;
        return this->p - n;
    }

    void in_skip_bytes(unsigned int n) {
        this->p+=n;
    }


    // 10 = 10, 0x10 = 16
    unsigned long ulong_from_cstr() noexcept
    {
        uint8_t * endptr = nullptr;
        unsigned long res = ((this->p[0] == '0') && (this->p[1] == 'x'))
            ? strtoul(char_ptr_cast(this->p), reinterpret_cast<char**>(&endptr), 16)
            : strtoul(char_ptr_cast(this->p), reinterpret_cast<char**>(&endptr), 10);
        this->p = endptr;
        return res;
    }

    // 10 = 10, 0x10 = 16
    long long_from_cstr() noexcept
    {
        uint8_t * endptr = nullptr;
        long res = ((this->p[0] == '0') && (this->p[1] == 'x'))
            ? strtol(char_ptr_cast(this->p), reinterpret_cast<char**>(&endptr), 16)
            : strtol(char_ptr_cast(this->p), reinterpret_cast<char**>(&endptr), 10);
        this->p = endptr;
        return res;
    }

    // 1, yes, on, true
    bool bool_from_cstr() noexcept
    {
        if (0 == strncasecmp("1", char_ptr_cast(this->p), 1))
        {
            this->p += 1;
            return true;
        }
        else if (0 == strncasecmp("yes", char_ptr_cast(this->p), 3))
        {
            this->p += 3;
            return true;
        }
        else if (0 == strncasecmp("on", char_ptr_cast(this->p), 2))
        {
            this->p += 2;
            return true;
        }
        else if (0 == strncasecmp("true", char_ptr_cast(this->p), 4))
        {
            this->p += 4;
            return true;
        }
        return false;
    }

    bool bool_from_cstrz() noexcept
    {
        return this->bool_from_cstr() && this->p[0] == 0;
    }

    // MS-RDPEGDI : 2.2.2.2.1.2.1.2 Two-Byte Unsigned Encoding
    // =======================================================
    // (TWO_BYTE_UNSIGNED_ENCODING)

    // The TWO_BYTE_UNSIGNED_ENCODING structure is used to encode a value in
    // the range 0x0000 to 0x7FFF by using a variable number of bytes.
    // For example, 0x1A1B is encoded as { 0x9A, 0x1B }.
    // The most significant bit of the first byte encodes the number of bytes
    // in the structure.

    // c (1 bit): A 1-bit, unsigned integer field that contains an encoded
    // representation of the number of bytes in this structure. 0 implies val2 field
    // is not present, if 1 val2 is present.

    // val1 (7 bits): A 7-bit, unsigned integer field containing the most
    // significant 7 bits of the value represented by this structure.

    // val2 (1 byte): An 8-bit, unsigned integer containing the least significant
    // bits of the value represented by this structure.

    uint16_t in_2BUE()
    {
        uint16_t length = this->in_uint8();
        if (length & 0x80){
            length = ((length & 0x7F) << 8);
            length += this->in_uint8();
        }
        return length;
    }

// [MS-RDPEGDI] - 2.2.2.2.1.2.1.4 Four-Byte Unsigned Encoding
// (FOUR_BYTE_UNSIGNED_ENCODING)
// ==========================================================

// The FOUR_BYTE_UNSIGNED_ENCODING structure is used to encode a value in the
//  range 0x00000000 to 0x3FFFFFFF by using a variable number of bytes. For
//  example, 0x001A1B1C is encoded as { 0x9A, 0x1B, 0x1C }. The two most
//  significant bits of the first byte encode the number of bytes in the
//  structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | c |    val1   |val2 (optional)|val3 (optional)|val4 (optional)|
// +---+-----------+---------------+---------------+---------------+

// c (2 bits): A 2-bit, unsigned integer field containing an encoded
//  representation of the number of bytes in this structure.

// +----------------+----------------------------------------------------------+
// | Value          | Meaning                                                  |
// +----------------+----------------------------------------------------------+
// | ONE_BYTE_VAL   | Implies that the optional val2, val3, and val4 fields    |
// | 0              | are not present. Hence, the structure is 1 byte in size. |
// +----------------+----------------------------------------------------------+
// | TWO_BYTE_VAL   | Implies that the optional val2 field is present while    |
// | 1              | the optional val3 and val4 fields are not present.       |
// |                | Hence, the structure is 2 bytes in size.                 |
// +----------------+----------------------------------------------------------+
// | THREE_BYTE_VAL | Implies that the optional val2 and val3 fields are       |
// | 2              | present while the optional val4 fields are not present.  |
// |                | Hence, the structure is 3 bytes in size.                 |
// +----------------+----------------------------------------------------------+
// | FOUR_BYTE_VAL  | Implies that the optional val2, val3, and val4 fields    |
// | 3              | are all present. Hence, the structure is 4 bytes in      |
// |                | size.                                                    |
// +----------------+----------------------------------------------------------+

// val1 (6 bits): A 6-bit, unsigned integer field containing the most
//  significant 6 bits of the value represented by this structure.

// val2 (1 byte): An 8-bit, unsigned integer containing the second most
//  significant bits of the value represented by this structure.

// val3 (1 byte): An 8-bit, unsigned integer containing the third most
//  significant bits of the value represented by this structure.

// val4 (1 byte): An 8-bit, unsigned integer containing the least significant
//  bits of the value represented by this structure.

    uint32_t in_4BUE()
    {
        uint32_t length = this->in_uint8();
        switch (length & 0xC0)
        {
            case 0xC0:
                length =  ((length & 0x3F)  << 24);
                length += (this->in_uint8() << 16);
                length += (this->in_uint8() << 8 );
                length +=  this->in_uint8();
            break;
            case 0x80:
                length =  ((length & 0x3F)  << 16);
                length += (this->in_uint8() << 8 );
                length +=  this->in_uint8();
            break;
            case 0x40:
                length =  ((length & 0x3F)  << 8 );
                length +=  this->in_uint8();
            break;
        }

        return length;
    }

    // MS-RDPEGDI : 2.2.2.2.1.1.1.4 Delta-Encoded Points (DELTA_PTS_FIELD)
    // ===================================================================

    // ..., the delta value it  represents is encoded in a packed signed
    //  format:

    //  * If the high bit (0x80) is not set in the first encoding byte, the
    //    field is 1 byte long and is encoded as a signed delta in the lower
    //    7 bits of the byte.

    //  * If the high bit of the first encoding byte is set, the lower 7 bits
    //    of the first byte and the 8 bits of the next byte are concatenated
    //    (the first byte containing the high-order bits) to create a 15-bit
    //    signed delta value.
    int16_t in_DEP(void) {
        int16_t point = this->in_uint8();
        if (point & 0x80)
        {
            point = ((point & 0x7F) << 8) + this->in_uint8();
            if (point & 0x4000)
                point = - ((~(point - 1)) & 0x7FFF);
        }
        else
        {
            if (point & 0x40)
                point = - ((~(point - 1)) & 0x7F);
        }
        return point;
    }

    void in_utf16(uint16_t utf16[], size_t length)
    {
        for (size_t i = 0; i < length ; i ++){
            utf16[i] = this->in_uint16_le();
        }
    }

    size_t in_utf16_sz(uint16_t utf16[], size_t length)
    {
        for (size_t i = 0; i < length ; i++){
            utf16[i] = this->in_uint16_le();
            if (utf16[i] == 0){
                return i;
            }
        }
        return length;
    }

};

