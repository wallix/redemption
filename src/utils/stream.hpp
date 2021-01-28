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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities
*/

#pragma once

#include "utils/bitfu.hpp"
#include "utils/parse.hpp"
#include "utils/sugar/buffer_view.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/buf_maker.hpp"

#include <memory>

#include <cassert>
#include <cstring> // for memcpy, memset


class InStream
{
    uint8_t const * begin = nullptr;
    uint8_t const * end = nullptr;
    // relative coordinate between begin and end (at begin, p = 0)
    Parse p;

public:
    explicit InStream(buffer_view buf) noexcept
    : begin(buf.begin())
    , end(buf.end())
    , p(this->begin)
    {
    }

    InStream() = default;
    InStream(InStream &&) = default;
    InStream & operator=(InStream &&) = default;
    InStream & operator=(InStream const &) = delete;

private:
    InStream(InStream const &) = default;

public:
    [[nodiscard]] InStream clone() const noexcept {
        return InStream(*this);
    }

    [[nodiscard]] bytes_view get_consumed_bytes() const noexcept {
        return {this->get_data(), this->get_offset()};
    }

    [[nodiscard]] bytes_view remaining_bytes() const noexcept {
        return {this->get_current(), this->in_remain()};
    }

    [[nodiscard]] uint8_t const * get_data() const noexcept {
        return this->begin;
    }

    [[nodiscard]] uint8_t const * get_data_end() const noexcept {
        return this->end;
    }

    [[nodiscard]] uint8_t const * get_current() const noexcept {
        return this->p.p;
    }

    [[nodiscard]] size_t get_offset() const noexcept {
        return static_cast<size_t>(this->p.p - this->begin);
    }

    // returns true if there is enough data available to read n bytes
    [[nodiscard]] bool in_check_rem(const unsigned n) const noexcept {
        return (n <= this->in_remain());
    }

    [[nodiscard]] size_t in_remain() const noexcept {
        assert(this->p.p <= this->end);
        return this->end - this->p.p;
    }


    [[nodiscard]] bool check_end() const noexcept {
        return this->p.p == this->end;
    }

    [[nodiscard]] size_t get_capacity() const noexcept {
        return this->end - this->begin;
    }

    // go back by the given amount (like rewind but relative)
    void unget(size_t n) noexcept {
        assert(this->begin + n < this->p.p);
        this->p.unget(n);
    }

    /// set current position to start buffer (\a p = \a begin)
    void rewind(std::size_t offset) noexcept {
        assert(this->begin + offset <= this->end);
        this->p.p = this->begin + offset;
    }

    /// set current position to start buffer (\a p = \a begin)
    void rewind() noexcept {
        this->p.p = this->begin;
    }

    // =========================================================================
    // Generic binary Data access methods
    // =========================================================================

    int8_t in_sint8() noexcept {
        assert(this->in_check_rem(1));
        return this->p.in_sint8();
    }

    // ---------------------------------------------------------------------------

    uint8_t in_uint8() noexcept {
        assert(this->in_check_rem(1));
        return this->p.in_uint8();
    }

    /* Peek a byte from stream without move <p>. */
    uint8_t peek_uint8() const noexcept {
        assert(this->in_check_rem(1));
        return *this->p.p;
    }

    int16_t in_sint16_be() noexcept {
        assert(this->in_check_rem(2));
        return this->p.in_sint16_be();
    }

    int16_t in_sint16_le() noexcept {
        assert(this->in_check_rem(2));
        return this->p.in_sint16_le();
    }

    uint16_t in_uint16_le() noexcept {
        assert(this->in_check_rem(2));
        return this->p.in_uint16_le();
    }

    uint16_t in_uint16_be() noexcept {
        assert(this->in_check_rem(2));
        return this->p.in_uint16_be();
    }

    uint32_t in_uint32_le() noexcept {
        assert(this->in_check_rem(4));
        return this->p.in_uint32_le();
    }

    uint32_t in_uint32_be() noexcept {
        assert(this->in_check_rem(4));
        return this->p.in_uint32_be();
    }

    uint32_t in_uint24_be() noexcept {
        assert(this->in_check_rem(3));
        uint32_t value = (this->in_uint8() << 16);
        value += this->in_uint16_be();
        return value;
    }

    int32_t in_sint32_le() noexcept {
        return this->p.in_sint32_le();
    }

    int32_t in_sint32_be() noexcept {
        return this->p.in_sint32_be();
    }

    int64_t in_sint64_le() noexcept {
        return this->p.in_sint64_le();
    }

    uint64_t in_uint64_le() noexcept {
        assert(this->in_check_rem(8));
        return this->p.in_uint64_le();
    }

    uint64_t in_uint64_be() noexcept {
        assert(this->in_check_rem(8));
        return this->p.in_uint64_be();
    }

    uint32_t in_bytes_le(const uint8_t nb) noexcept {
        assert(this->in_check_rem(nb));
        return this->p.in_bytes_le(nb);
    }

    uint32_t in_bytes_be(const uint8_t nb) noexcept {
        assert(this->in_check_rem(nb));
        return this->p.in_bytes_be(nb);
    }

    void in_copy_bytes(writable_buffer_view v) noexcept {
        assert(this->in_check_rem(v.size()));
        return this->p.in_copy_bytes(v);
    }

    void in_copy_bytes(writable_byte_ptr v, size_t n) noexcept {
        return this->in_copy_bytes({v, n});
    }

    std::vector<uint8_t> in_copy_bytes_as_vector(size_t n) noexcept {
        assert(this->in_check_rem(n));
        std::vector<uint8_t> v(this->get_current(), this->get_current()+n);
        this->p.in_skip_bytes(n);
        return v;
    }

    const uint8_t *in_uint8p(unsigned int n) noexcept {
        assert(this->in_check_rem(n));
        return this->p.in_uint8p(n);
    }

    [[nodiscard]] bytes_view view_bytes(unsigned int n) const noexcept {
        assert(this->in_check_rem(n));
        return u8_array_view{this->get_current(), n};
    }

    bytes_view in_skip_bytes(unsigned int n) noexcept {
        assert(this->in_check_rem(n));
        bytes_view ret(this->get_current(), n);
        this->p.in_skip_bytes(n);
        return ret;
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

    uint16_t in_2BUE() noexcept
    {
        return this->p.in_2BUE();
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

    uint32_t in_4BUE() noexcept
    {
        return this->p.in_4BUE();
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
    int16_t in_DEP() noexcept {
        return this->p.in_DEP();
    }

    void in_utf16(uint16_t utf16[], size_t length) noexcept
    {
        return this->p.in_utf16(utf16, length);
    }

    // extract a zero terminated UTF16 string from stream
    // of at most length UTF16 chars
    // return UTF16 string length (number of chars, not bytes)
    // if number returned in same as input length, it means no
    // zero char has been found
    size_t in_utf16_sz(uint16_t utf16[], size_t length) noexcept
    {
        return this->p.in_utf16_sz(utf16, length);
    }
};


// TODO: OutStream should be based on some output object (like it is done between InStream and Parse)
// where output object doesn't care about checking boundaries (OutStream job)

class OutStream
{
    uint8_t * begin = nullptr;
    uint8_t * end = nullptr;
    uint8_t * p = nullptr;

public:
    explicit OutStream(writable_buffer_view buf) noexcept
    : begin(buf.begin())
    , end(buf.end())
    , p(this->begin)
    {
    }

    OutStream() = default;
    OutStream(OutStream &&) = default;
    OutStream(OutStream const &) = delete;
    OutStream & operator=(OutStream &&) = default;
    OutStream & operator=(OutStream const &) = delete;

    [[nodiscard]] size_t tailroom() const noexcept {
        return  this->end - this->p;
    }

    [[nodiscard]] writable_bytes_view get_tail() const noexcept {
        return {this->get_current(), this->tailroom()};
    }

    [[nodiscard]] bool has_room(size_t n) const noexcept {
        return  n <= this->tailroom();
    }

    [[nodiscard]] writable_bytes_view get_produced_bytes() const noexcept {
        return {this->get_data(), this->get_offset()};
    }

    [[nodiscard]] OutStream stream_at(std::size_t i) const noexcept {
        assert(i < this->get_capacity());
        return OutStream({this->get_data() + i, this->end});
    }

    [[nodiscard]] uint8_t * get_data() const noexcept {
        return this->begin;
    }

    [[nodiscard]] uint8_t * get_data_end() const noexcept {
        return this->end;
    }

    [[nodiscard]] uint8_t * get_current() const noexcept {
        return this->p;
    }

    [[nodiscard]] size_t get_offset() const noexcept {
        return static_cast<size_t>(this->p - this->begin);
    }

    void out_uint64_le(uint64_t v) noexcept {
        assert(this->has_room(8));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p[4] = (v >> 32) & 0xFF;
        this->p[5] = (v >> 40) & 0xFF;
        this->p[6] = (v >> 48) & 0xFF;
        this->p[7] = (v >> 56) & 0xFF;
        this->p+=8;
    }

    void out_uint64_be(uint64_t v) noexcept {
        assert(this->has_room(8));
        this->p[0] = (v >> 56) & 0xFF;
        this->p[1] = (v >> 48) & 0xFF;
        this->p[2] = (v >> 40) & 0xFF;
        this->p[3] = (v >> 32) & 0xFF;
        this->p[4] = (v >> 24) & 0xFF;
        this->p[5] = (v >> 16) & 0xFF;
        this->p[6] = (v >> 8) & 0xFF;
        this->p[7] = v & 0xFF;
        this->p+=8;
    }

    void out_sint64_le(int64_t v) noexcept {
        assert(this->has_room(8));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p[4] = (v >> 32) & 0xFF;
        this->p[5] = (v >> 40) & 0xFF;
        this->p[6] = (v >> 48) & 0xFF;
        this->p[7] = (v >> 56) & 0xFF;
        this->p+=8;
    }

    writable_bytes_view out_skip_bytes(unsigned int n) noexcept {
        assert(this->has_room(n));
        writable_bytes_view ret(this->get_current(), n);
        this->p += n;
        return ret;
    }

    void out_uint8(uint8_t v) noexcept {
        assert(this->has_room(1));
        *this->p++ = v;
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

    void out_2BUE(uint16_t v) noexcept {
        if (v <= 127){
            this->out_uint8(static_cast<uint8_t>(v));
        }
        else {
            this->out_uint16_be(v|0x8000);
        }
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


    void out_4BUE(uint32_t v) noexcept {
        assert(!(v & 0xC0000000));
        if      (v <= 0x3F      ) {
            this->out_uint8(static_cast<uint8_t>(v));
        }
        else if (v <= 0x3FFF    ) {
            this->out_uint8(0x40 | ((v >> 8 ) & 0x3F));
            this->out_uint8(         v        & 0xFF );
        }
        else if (v <= 0x3FFFFF  ) {
            this->out_uint8(0x80 | ((v >> 16) & 0x3F));
            this->out_uint8(        (v >> 8 ) & 0xFF );
            this->out_uint8(         v        & 0xFF );
        }
        else/* if (v <= 0x3FFFFFFF)*/ {
            this->out_uint8(0xC0 | ((v >> 24) & 0x3F));
            this->out_uint8(        (v >> 16) & 0xFF );
            this->out_uint8(        (v >> 8 ) & 0xFF );
            this->out_uint8(         v        & 0xFF );
        }
    }


    // MS-RDPEGDI : 2.2.2.2.1.1.1.4 Delta-Encoded Points (DELTA_PTS_FIELD)
    // ===================================================================

    // ..., the delta value it  represents is encoded in a packet signed
    //  format:

    //  * If the high bit (0x80) is not set in the first encoding byte, the
    //    field is 1 byte long and is encoded as a signed delta in the lower
    //    7 bits of the byte.

    //  * If the high bit of the first encoding byte is set, the lower 7 bits
    //    of the first byte and the 8 bits of the next byte are concatenated
    //    (the first byte containing the high-order bits) to create a 15-bit
    //    signed delta value.

    void out_DEP(int16_t point) noexcept {
        if ((point > 0x3F)||(point < -64)){
            this->out_uint16_be(point|0x8000);
        }
        else {
            this->out_uint8(point&0x7F);
        }
    }

    void out_sint8(char v) noexcept {
        assert(this->has_room(1));
        *(this->p++) = v;
    }

    void out_uint16_le(unsigned int v) noexcept {
        assert(this->has_room(2));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void out_sint16_le(signed int v) noexcept {
        assert(this->has_room(2));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void out_uint16_be(unsigned int v) noexcept {
        assert(this->has_room(2));
        this->p[1] = v & 0xFF;
        this->p[0] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void out_uint16_be_bis(unsigned int v) noexcept {
        assert(this->has_room(2));
        this->p[1] = v & 0xFF;
        this->p[0] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void out_uint32_le_bis(unsigned int v) noexcept {
        assert(this->has_room(4));
        this->p[0] = (v & 0xFF);
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p+=4;
    }

    void out_uint32_le(unsigned int v) noexcept {
        assert(this->has_room(4));
        this->p[0] = (v & 0xFF);
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p+=4;
    }

    void out_uint32_be(unsigned int v) noexcept {
        assert(this->has_room(4));
        this->p[0] = (v >> 24) & 0xFF;
        this->p[1] = (v >> 16) & 0xFF;
        this->p[2] = (v >> 8) & 0xFF;
        this->p[3] = v & 0xFF;
        this->p+=4;
    }

    void out_sint32_le(int64_t v) noexcept {
        assert(this->has_room(4));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p+=4;
    }

    void out_utf16(const uint16_t utf16[], size_t length) noexcept
    {
        for (size_t i = 0; i < length ; i ++){
            this->out_uint16_le(utf16[i]);
        }
    }

    [[nodiscard]] size_t get_capacity() const noexcept {
        return this->end - this->begin;
    }

    /// set current position to start buffer (\a p = \a begin)
    void rewind(std::size_t offset = 0) noexcept /*NOLINT*/ {
        assert(this->begin + offset <= this->end);
        this->p = this->begin + offset;
    }

    void out_copy_bytes(bytes_view data) noexcept {
        assert(this->has_room(data.size()));
        if (not data.empty()){
            memcpy(this->p, data.data(), data.size());
            this->p += data.size();
        }
    }

    void out_copy_bytes(byte_ptr v, size_t n) noexcept {
        this->out_copy_bytes({v, n});
    }

    void out_clear_bytes(size_t n) noexcept {
        assert(this->has_room(n));
        memset(this->p, 0, n);
        this->p += n;
    }

    void out_bytes_le(const uint8_t nb, const unsigned value) noexcept {
        assert(this->has_room(nb));
        ::out_bytes_le(this->p, nb, value);
        this->p += nb;
    }
};


template<std::size_t N>
struct StaticOutStream : OutStream
{
    explicit StaticOutStream() noexcept
    : OutStream(this->array_)
    {}

    StaticOutStream(StaticOutStream const &) = delete;
    StaticOutStream & operator = (StaticOutStream const &) = delete;

    static constexpr std::size_t original_capacity() noexcept
    {
        return N;
    }

private:
    uint8_t array_[N];
};


template<std::size_t OrignalLen>
struct StreamBufMaker
{
    OutStream reserve_out_stream(std::size_t n) &
    {
        return OutStream(this->buf_maker_.dyn_array(n));
    }

    InStream reserve_in_stream(std::size_t n) &
    {
        return InStream(this->buf_maker_.dyn_array(n));
    }

private:
    BufMaker<OrignalLen> buf_maker_;
};

// class DynamicOutStream;
// class DynamicInStream;

/**
 * @brief an OutStream that has some space reserved before the payload
 *
 *                v  payload_stream
 *      |------------+----------------------------------------------|
 *            ^ head_ptr
 *       <------>   reserved_leading_space
 *    <----------------- full_size ----------------------------->
 */
struct OutReservedStreamHelper
{
    OutReservedStreamHelper(uint8_t * data, std::size_t reserved_leading_space, std::size_t buf_len) noexcept
    : head_ptr(data + reserved_leading_space)
    , reserved_leading_space(reserved_leading_space)
    , payload_stream({this->head_ptr, buf_len - reserved_leading_space})
    {}

    [[nodiscard]] writable_bytes_view get_packet() const noexcept {
        return writable_bytes_view{this->head_ptr, std::size_t(this->payload_stream.get_current() - this->head_ptr)};
    }

    [[nodiscard]] std::size_t get_reserved_leading_space() const noexcept {
        return this->reserved_leading_space;
    }

    OutStream & get_data_stream() noexcept {
        return this->payload_stream;
    }

    /** returns an OutReservedStreamHelper that will start at offset with size len
     * @param offset starts the sub stream at this offset
     * @param len take only len bytes
     * @return the corresponding substream
     */
    OutReservedStreamHelper get_sub_stream(size_t offset, size_t len = 0) noexcept /*NOLINT*/ {
        // this object is like:
        //    reserved_leading_space
        //  <----->
        //        v  headPtr
        //  |-----------+-------------------------|
        //              ^  payloadStream
        //  <------------------------------------->

        // and we wanna return:
        //        |<--- offset -->|<-- len -->|
        //                        v headPtr
        //  |---------------------+-----------|
        //                        ^ payloadStream

        if (len == 0) {
            len = (this->payload_stream.get_offset() - offset);
        }
        uint8_t *originalBuf = this->head_ptr - this->reserved_leading_space;

        OutReservedStreamHelper ret(originalBuf, this->reserved_leading_space + offset, this->reserved_leading_space + offset + len);
        ret.payload_stream.rewind(len);
        return ret;
    }

    writable_bytes_view copy_to_head(bytes_view data) noexcept {
        assert(data.size() <= this->reserved_leading_space);
        this->head_ptr -= data.size();
        this->reserved_leading_space -= data.size();
        memcpy(this->head_ptr, data.data(), data.size());

        return get_packet();
    }

    writable_bytes_view copy_to_head(bytes_view data1, bytes_view data2) noexcept {
        auto const total_stream_size = data1.size() + data2.size();
        assert(total_stream_size <= this->reserved_leading_space);
        this->reserved_leading_space -= total_stream_size;
        this->head_ptr -= total_stream_size;

        auto start = this->head_ptr;
        memcpy(start, data1.data(), data1.size());
        start += data1.size();
        memcpy(start, data2.data(), data2.size());

        return get_packet();
    }

    void rewind() noexcept {
        this->reserved_leading_space += this->payload_stream.get_data() - this->head_ptr;
        this->head_ptr = this->payload_stream.get_data();
        this->payload_stream.rewind();
    }

    void rewind_head() noexcept {
        this->reserved_leading_space += this->payload_stream.get_data() - this->head_ptr;
        this->head_ptr = this->payload_stream.get_data();
    }

private:
    uint8_t * head_ptr;
    std::size_t reserved_leading_space;
    OutStream payload_stream;
};

template<std::size_t HeaderSz, std::size_t StreamSz>
struct StaticOutReservedStreamHelper : OutReservedStreamHelper
{
    StaticOutReservedStreamHelper() noexcept
    : OutReservedStreamHelper(this->data, HeaderSz, HeaderSz + StreamSz)
    {}

private:
    uint8_t data[HeaderSz + StreamSz];
};

/**
 * \addtogroup stream-utility
 * \addtogroup transport-utility
 * @{
 */
template<class Writer>
struct DynamicStreamWriter
{
    [[nodiscard]] std::size_t packet_size() const noexcept {
        return this->stream_size_;
    }

    void operator()(std::size_t /*unused*/, OutStream & ostream) const {
        assert(ostream.get_capacity() == this->stream_size_);
        this->apply_writer1(ostream, this->writer_, 1);
    }

    void operator()(std::size_t /*unused*/, OutStream & ostream, uint8_t * buf, std::size_t used_buf_sz) const {
        assert(ostream.get_capacity() == this->stream_size_);
        this->apply_writer2(ostream, buf, used_buf_sz, this->writer_, 1);
    }

    Writer writer_;
    std::size_t stream_size_;

private:
    template<class W>
    auto apply_writer1(OutStream & ostream, W & writer, int /*unused*/) const
    -> decltype(writer(ostream))
    { writer(ostream); }

    template<class W>
    void apply_writer1(OutStream & ostream, W & writer, unsigned) const;

    template<class W>
    auto apply_writer2(OutStream & ostream, uint8_t * /*unused*/, std::size_t used_buf_sz, W & writer, int /*unused*/) const
    -> decltype(writer(ostream, used_buf_sz))
    { writer(ostream, used_buf_sz); }

    template<class W>
    void apply_writer2(OutStream & ostream, uint8_t * buf, std::size_t used_buf_sz, W & writer, unsigned /*unused*/) const
    { writer(ostream, buf, used_buf_sz); }
};

template<std::size_t N>
using StreamSize = std::integral_constant<std::size_t, N>;
/**
 * @}
 */

namespace details_ {
    /// Extract stream size of Writer
    /// @{
    template<class R, class C, class Sz, class ... Args>
    constexpr Sz packet_size_from_mfunc(R(C::* /*unused*/)(Sz, OutStream &, Args...)) noexcept {
        return Sz{};
    }

    template<class R, class C, class Sz, class ... Args>
    constexpr Sz packet_size_from_mfunc(R(C::* /*unused*/)(Sz, OutStream &, Args...) const) noexcept {
        return Sz{};
    }

    template<class F>
    constexpr auto packet_size_impl(F const & /*unused*/, std::false_type /*unused*/)
    -> decltype(packet_size_from_mfunc(&F::operator())){
        return packet_size_from_mfunc(&F::operator());
    }

    template<class Writer>
    constexpr auto packet_size_impl(Writer & writer, std::true_type /*unused*/)
    -> decltype(writer.packet_size()) {
        return writer.packet_size();
    }

    template<class F, class = void>
    struct has_packet_size : std::false_type
    {};

    template<class F>
    struct has_packet_size<F, decltype(void(std::declval<F>().packet_size()))> : std::true_type
    {};

    template<class F>
    constexpr auto packet_size(F const & f)
    -> decltype(packet_size_impl(f, has_packet_size<F>{})) {
        return packet_size_impl(f, has_packet_size<F>{});
    }

    template<class R, class Sz, class ... Args>
    constexpr Sz packet_size(R(* /*unused*/)(Sz, OutStream &, Args...)) {
        return Sz{};
    }
    /// @}


    /// Extract stream size of multiple Writers
    /// @{
    template<class T, class U>
    std::integral_constant<std::size_t, T::value + U::value>
    packet_size_add(T const & /*unused*/, U const & /*unused*/) noexcept {
        return {};
    }

    template<class T, class U>
    typename std::enable_if<
        std::is_same<T, std::size_t>::value
     || std::is_same<U, std::size_t>::value
    , std::size_t>::type
    packet_size_add(T const & a, U const & b) noexcept {
        return a + b;
    }

    template<class AccuSize>
    constexpr AccuSize packets_size_impl(AccuSize accu) {
        return accu;
    }

    template<class AccuSize, class... Fns>
    struct build_packets_size_type;

    template<class AccuSize, class Fn, class... Fns>
    struct build_packets_size_type<AccuSize, Fn, Fns...>
    {
        using type = typename build_packets_size_type<
            decltype(packet_size_add(AccuSize{}, packet_size(std::declval<Fn>()))),
            Fns...
        >::type;
    };

    template<class AccuSize>
    struct build_packets_size_type<AccuSize>
    { using type = AccuSize; };

    template<class AccuSize, class Fn, class... Fns>
    constexpr typename build_packets_size_type<AccuSize, Fn, Fns...>::type
    packets_size_impl(AccuSize accu, Fn const & f, Fns const & ... fns) {
        return packets_size_impl(packet_size_add(accu, packet_size(f)), fns...);
    }

    template<class... Fs>
    constexpr auto packets_size(Fs const & ... fns)
    -> decltype(packets_size_impl(StreamSize<0>{}, fns...)) {
        return packets_size_impl(StreamSize<0>{}, fns...);
    }
    /// @}


    template<class StreamSz, class Writer>
    auto apply_writer(StreamSz sz, OutStream & ostream, uint8_t * /*unused*/, std::size_t used_buf_sz, Writer & writer, int /*unused*/)
    -> decltype(writer(sz, ostream, used_buf_sz))
    { writer(sz, ostream, used_buf_sz); }

    template<class StreamSz, class Writer>
    void apply_writer(StreamSz sz, OutStream & ostream, uint8_t * buf, std::size_t used_buf_sz, Writer & writer, unsigned /*unused*/)
    { writer(sz, ostream, writable_bytes_view(buf, used_buf_sz)); }


    template<class StreamSz, class Writer>
    void write_packet(uint8_t * & full_buf, std::size_t & used_buf_sz, StreamSz sz, Writer & writer) {
        if (!StreamSz::value) {
            OutStream ostream;
            apply_writer(sz, ostream, full_buf, used_buf_sz, writer, 1);
        }
        else {
            uint8_t data_buf[StreamSz::value ? StreamSz::value : 1 /*disable warning: zero size arrays*/];
            OutStream ostream(data_buf);
            apply_writer(sz, ostream, full_buf, used_buf_sz, writer, 1);
            used_buf_sz += ostream.get_offset();
            full_buf -= ostream.get_offset();
            memcpy(full_buf, ostream.get_data(), ostream.get_offset());
        }
    }

    template<class DataBufSz, class HeaderBufSz, class Transport, class DataWriter, class... HeaderWriters>
    void write_packets_impl(
        DataBufSz data_buf_sz, HeaderBufSz header_buf_sz, uint8_t * buf, /*NOLINT*/
        Transport & trans, DataWriter & data_writer, HeaderWriters & ... header_writers)
    {
        OutStream data_stream({buf + header_buf_sz, data_buf_sz});
        data_writer(data_buf_sz, data_stream);
        auto * start = data_stream.get_data();
        std::size_t used_buf_sz = data_stream.get_offset();
        (..., details_::write_packet(start, used_buf_sz, details_::packet_size(header_writers), header_writers));
        trans.send(start, used_buf_sz);
    }

    template<class DataBufSz, class HeaderBufSz, class Transport, class DataWriter, class... HeaderWriters>
    void write_packets(
        DataBufSz data_buf_sz, HeaderBufSz header_buf_sz, std::size_t /*unused*/,
        Transport & trans, DataWriter & data_writer, HeaderWriters & ... header_writers)
    {
        if (data_buf_sz + header_buf_sz < 65536) {
            uint8_t buf[65536];
            write_packets_impl(data_buf_sz, header_buf_sz, buf, trans, data_writer, header_writers...);
        }
        else {
            auto u = std::make_unique<uint8_t[]>(data_buf_sz + header_buf_sz);
            write_packets_impl(data_buf_sz, header_buf_sz, u.get(), trans, data_writer, header_writers...);
        }
    }

    template<
        class DataBufSz, class HeaderBufSz, class TotalSz,
        class Transport, class DataWriter, class... HeaderWriters
    >
    void write_packets(
        DataBufSz data_buf_sz, HeaderBufSz header_buf_sz, TotalSz /*unused*/,
        Transport & trans, DataWriter & data_writer, HeaderWriters & ... header_writers)
    {
        uint8_t buf[TotalSz::value];
        write_packets_impl(data_buf_sz, header_buf_sz, buf, trans, data_writer, header_writers...);
    }
} // namespace details_


/**
 * \addtogroup stream-utility
 * \addtogroup transport-utility
 *
 * \param data_writer  function(IntegralConstant||std:size_t, OutStream &)
 * \param header_writers  function(IntegralConstant||std:size_t, OutStream &, [uint8_t * data,] std::size_t data_sz)
 *
 * write_packets(t, f1, f2, f3) write a packet [f3-f2-f1] in t
 */
template<class Transport, class DataWriter, class... HeaderWriters>
void write_packets(Transport & trans, DataWriter data_writer, HeaderWriters... header_writers)
{
    auto const data_buf_sz = details_::packet_size(data_writer);
    auto const header_buf_sz = details_::packets_size(header_writers...);
    details_::write_packets(
        data_buf_sz, header_buf_sz,
        details_::packet_size_add(header_buf_sz, data_buf_sz),
        trans, data_writer, header_writers...
    );
}

template<class Writer>
DynamicStreamWriter<Writer>
dynamic_packet(std::size_t stream_sz, Writer writer) {
    return {std::move(writer), stream_sz};
}

