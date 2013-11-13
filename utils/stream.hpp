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

#ifndef _REDEMPTION_UTILS_STREAM_HPP_
#define _REDEMPTION_UTILS_STREAM_HPP_

#include "log.hpp"

#include <stdio.h>
#include <string.h> // for memcpy, memset
#include <algorithm>

#include "error.hpp"
#include "bitfu.hpp"
#include "utf.hpp"

// using a template for default size of stream would make sense instead of always using the large buffer below
enum {
     AUTOSIZE = 65536
};

class Stream {
public:
    uint8_t* p;
    uint8_t* end;
protected:
    uint8_t* data;
    size_t capacity;

public:
    virtual ~Stream() {}

    virtual void init(size_t capacity) = 0;

    virtual void reset(){
        this->end = this->p = this->data;
    }

    bool has_room(unsigned n) const {
        return this->get_offset() + n <= this->get_capacity();
    }

    virtual size_t get_capacity() const {
        return this->capacity;
    }

    virtual uint8_t * get_data() const {
        return this->data;
    }

    uint8_t * _get_buffer() const {
        return this->data;
    }

    uint32_t get_offset() const {
        return this->p - this->get_data();
    }

    uint32_t room() const {
        return this->get_capacity() - this->get_offset();
    }

    bool in_check_rem(const unsigned n) const {
    // returns true if there is enough data available to read n bytes
        return (n <= this->in_remain());
    }

    size_t in_remain() const {
        return this->end - this->p;
    }

    bool check_end(void) const {
        return this->p == this->end;
    }

    // =========================================================================
    // Generic binary Data access methods
    // =========================================================================

    signed char in_sint8(void) {
        REDASSERT(this->in_check_rem(1));
        return *((signed char*)(this->p++));
    }

    unsigned char in_uint8(void) {
        REDASSERT(this->in_check_rem(1));
        return *((unsigned char*)(this->p++));
    }

    /* Peek a byte from stream without move <p>. */
    unsigned char peek_uint8(void) {
        REDASSERT(this->in_check_rem(1));
        return *((unsigned char*)(this->p));
    }

    int16_t in_sint16_be(void) {
        REDASSERT(this->in_check_rem(2));
        unsigned int v = this->in_uint16_be();
        return (int16_t)((v > 32767)?v - 65536:v);
    }

    int16_t in_sint16_le(void) {
        REDASSERT(this->in_check_rem(2));
        unsigned int v = this->in_uint16_le();
        return (int16_t)((v > 32767)?v - 65536:v);
    }

    uint16_t in_uint16_le(void) {
        REDASSERT(this->in_check_rem(2));
        this->p += 2;
        return (uint16_t)(this->p[-2] | (this->p[-1] << 8));
    }

    uint16_t in_uint16_be(void) {
        REDASSERT(this->in_check_rem(2));
        this->p += 2;
        return (uint16_t)(this->p[-1] | (this->p[-2] << 8)) ;
    }

    unsigned int in_uint32_le(void) {
        REDASSERT(this->in_check_rem(4));
        this->p += 4;
        return  this->p[-4]
             | (this->p[-3] << 8)
             | (this->p[-2] << 16)
             | (this->p[-1] << 24)
             ;
    }

    unsigned int in_uint32_be(void) {
        REDASSERT(this->in_check_rem(4));
        this->p += 4;
        return  this->p[-1]
             | (this->p[-2] << 8)
             | (this->p[-3] << 16)
             | (this->p[-4] << 24)
             ;
    }

    void in_timeval_from_uint64le_usec(timeval & tv)
    {
        const uint64_t movie_usec_lo = this->in_uint32_le();
        const uint64_t movie_usec_hi = this->in_uint32_le();
        const uint64_t movie_usec = (movie_usec_hi * 0x100000000LL + movie_usec_lo);
        tv.tv_usec = static_cast<uint32_t>(movie_usec % 1000000LL);
        tv.tv_sec = static_cast<uint32_t>(movie_usec / 1000000LL);
    }

    void out_timeval_to_uint64le_usec(const timeval & tv)
    {
        uint64_t usec = tv.tv_sec * 1000000ULL + tv.tv_usec;
        this->out_uint32_le(static_cast<uint32_t>(usec));
        this->out_uint32_le(static_cast<uint32_t>(usec >> 32));
    }

    void out_uint64_le(uint64_t v) {
        REDASSERT(this->has_room(8));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p[4] = (v >> 32) & 0xFF;
        this->p[5] = (v >> 40) & 0xFF;
        this->p[6] = (v >> 48) & 0xFF;
        this->p[7] = (uint8_t)(v >> 56) & 0xFF;
        this->p+=8;
    }

    uint64_t in_uint64_le(void) {
        REDASSERT(this->in_check_rem(8));
        uint64_t low = this->in_uint32_le();
        uint64_t high = this->in_uint32_le();
        return low + (high << 32);
    }

    void out_uint64_be(uint64_t v) {
        REDASSERT(this->has_room(8));
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

    uint64_t in_uint64_be(void) {
        REDASSERT(this->in_check_rem(8));
        uint64_t high = this->in_uint32_be();
        uint64_t low = this->in_uint32_be();
        return low + (high << 32);
    }

    unsigned in_bytes_le(const uint8_t nb){
        REDASSERT(this->in_check_rem(nb));
        this->p += nb;
        return ::in_uint32_from_nb_bytes_le(nb, this->p - nb);
    }

    unsigned in_bytes_be(const uint8_t nb){
        REDASSERT(this->in_check_rem(nb));
        this->p += nb;
        return ::in_uint32_from_nb_bytes_be(nb, this->p - nb);
    }

    void in_copy_bytes(uint8_t * v, size_t n) {
        REDASSERT(this->in_check_rem(n));
        memcpy(v, this->p, n);
        this->p += n;
    }

    void in_copy_bytes(char * v, size_t n) {
        this->in_copy_bytes((uint8_t*)(v), n);
    }

    const uint8_t *in_uint8p(unsigned int n) {
        REDASSERT(this->in_check_rem(n));
        this->p+=n;
        return this->p - n;
    }

    uint8_t * out_uint8p(unsigned int n) {
        REDASSERT(this->has_room(n));
        this->p+=n;
        return this->p - n;
    }

    void in_skip_bytes(unsigned int n) {
        REDASSERT(this->in_check_rem(n));
        this->p+=n;
    }

    void out_skip_bytes(unsigned int n) {
        REDASSERT(this->has_room(n));
        this->p+=n;
    }

    void out_uint8(unsigned char v) {
        REDASSERT(this->has_room(1));
        *(this->p++) = v;
    }

    void set_out_uint8(unsigned char v, size_t offset) {
        (this->get_data())[offset] = v;
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

    void out_2BUE(uint16_t v){
        if (v <= 127){
            this->out_uint8(static_cast<uint8_t>(v));
        }
        else {
            this->out_uint16_be(v|0x8000);
        }
    }

    void set_out_2BUE(uint16_t v, size_t offset){
        if (v <= 127){
            this->set_out_uint8(static_cast<uint8_t>(v), offset);
        }
        else {
            this->set_out_uint16_be(v|0x8000, offset);
        }
    }

    uint16_t in_2BUE()
    {
        uint16_t length = this->in_uint8();
        if (length & 0x80){
            length = ((length & 0x7F) << 8);
            length += this->in_uint8();
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

    void out_DEP(int16_t point) {
        if (abs(point) > 0x3F) {
            uint16_t data;

            memcpy(&data, &point, sizeof(data));
            data |= 0x8000;

            if (point > 0) {
                data &= ~0x4000;
            }
            else {
                data |= 0x4000;
            }

            this->out_uint16_be(data);
        }
        else {
            int8_t  _yDelta;
            uint8_t data;

            _yDelta = point;

            memcpy(&data, &_yDelta, sizeof(data));
            data &= ~0x80;

            if (point > 0) {
                data &= ~0x40;
            }
            else {
                data |= 0x40;
            }

            this->out_uint8(data);
        }
    }

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

    void out_sint8(char v) {
        REDASSERT(this->has_room(1));
        *(this->p++) = v;
    }

    void set_out_sint8(char v, size_t offset) {
        (this->get_data())[offset] = v;
    }

    void out_uint16_le(unsigned int v) {
        REDASSERT(this->has_room(2));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_uint16_le(unsigned int v, size_t offset) {
        (this->get_data())[offset] = v & 0xFF;
        (this->get_data())[offset+1] = (v >> 8) & 0xFF;
    }

    void out_sint16_le(signed int v) {
        REDASSERT(this->has_room(2));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_sint16_le(signed int v, size_t offset) {
        (this->get_data())[offset] = v & 0xFF;
        (this->get_data())[offset+1] = (v >> 8) & 0xFF;
    }

    void out_uint16_be(unsigned int v) {
        REDASSERT(this->has_room(2));
        this->p[1] = v & 0xFF;
        this->p[0] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_uint16_be(unsigned int v, size_t offset) {
        (this->get_data())[offset+1] = v & 0xFF;
        (this->get_data())[offset] = (v >> 8) & 0xFF;
    }

    void out_uint32_le(unsigned int v) {
        REDASSERT(this->has_room(4));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (uint8_t)(v >> 24) & 0xFF;
        this->p+=4;
    }

    void set_out_uint32_le(unsigned int v, size_t offset) {
        (this->get_data())[offset+0] = v & 0xFF;
        (this->get_data())[offset+1] = (v >> 8) & 0xFF;
        (this->get_data())[offset+2] = (v >> 16) & 0xFF;
        (this->get_data())[offset+3] = (uint8_t)(v >> 24) & 0xFF;
    }

    void out_uint32_be(unsigned int v) {
        REDASSERT(this->has_room(4));
        this->p[0] = (uint8_t)(v >> 24) & 0xFF;
        this->p[1] = (v >> 16) & 0xFF;
        this->p[2] = (v >> 8) & 0xFF;
        this->p[3] = v & 0xFF;
        this->p+=4;
    }

    void set_out_uint32_be(unsigned int v, size_t offset) {
        REDASSERT(this->has_room(4));
        (this->get_data())[offset+0] = (uint8_t)(v >> 24) & 0xFF;
        (this->get_data())[offset+1] = (v >> 16) & 0xFF;
        (this->get_data())[offset+2] = (v >> 8) & 0xFF;
        (this->get_data())[offset+3] = v & 0xFF;
    }

    void out_unistr(const char* text)
    {
        const size_t len = UTF8toUTF16(reinterpret_cast<const uint8_t*>(text), this->p, this->room());
        this->p += len;
        this->out_clear_bytes(2);
    }

    void out_unistr_crlf(const char* text)
    {
        const size_t len = UTF8toUTF16_CrLf(reinterpret_cast<const uint8_t*>(text), this->p, this->room());
        this->p += len;
        this->out_clear_bytes(2);
    }

    void out_date_name(const char* text, const size_t buflen)
    {
        size_t i = 0;
        for (; i < (buflen/2) ; i++) {
            if (!text[i]){
                break;
            }
            this->out_uint8(text[i]);
            this->out_uint8(0);
        }
        for (; i < (buflen/2) ; i++) {
            this->out_uint8(0);
            this->out_uint8(0);
        }
    }

    void out_utf16(const uint16_t utf16[], size_t length)
    {
        for (size_t i = 0; i < length ; i ++){
            this->out_uint16_le(utf16[i]);
        }
    }

    void in_utf16(uint16_t utf16[], size_t length)
    {
        for (size_t i = 0; i < length ; i ++){
            utf16[i] = this->in_uint16_le();
        }
    }

    // sz utf16 bytes are translated to ascci, 00 terminated
    void in_uni_to_ascii_str(uint8_t * text, size_t sz, size_t bufsz)
    {
        UTF16toUTF8(this->p, sz / 2, text, bufsz);
        this->p += sz;
    }

    void mark_end() {
        this->end = this->p;
    }

    virtual void rewind(){
        this->p = this->data;
    }

    size_t size() const {
        return this->end - this->get_data();
    }

    // Output zero terminated string, including trailing 0
    void out_sz(const char * v) {
        size_t len = strlen(v);
        this->out_copy_bytes((uint8_t*)v, len+1);
    }

    // Output zero terminated string, non including trailing 0
    void out_string(const char * v) {
        size_t len = strlen(v);
        this->out_copy_bytes((uint8_t*)v, len);
    }


    void out_copy_bytes(Stream & stream) {
        this->out_copy_bytes(stream.get_data(), stream.size());
    }

    void out_copy_bytes(const uint8_t * v, size_t n) {
        REDASSERT(this->has_room(n));
        memcpy(this->p, v, n);
        this->p += n;
    }

    void set_out_copy_bytes(const uint8_t * v, size_t n, size_t offset) {
        memcpy(this->get_data()+offset, v, n);
    }

    void out_copy_bytes(const char * v, size_t n) {
        this->out_copy_bytes((uint8_t*)v, n);
    }

    void set_out_copy_bytes(const char * v, size_t n, size_t offset) {
        this->set_out_copy_bytes((uint8_t*)v, n, offset);
    }

    void out_concat(const char * v) {
        this->out_copy_bytes(v, strlen(v));
    }

    void set_out_concat(const char * v, size_t offset) {
        this->set_out_copy_bytes((uint8_t*)v, strlen(v), offset);
    }


    void out_clear_bytes(size_t n) {
        REDASSERT(this->has_room(n));
        memset(this->p, 0, n);
        this->p += n;
    }

    void set_out_clear_bytes(size_t n, size_t offset) {
        memset(this->get_data()+offset, 0, n);
    }

    void out_bytes_le(const uint8_t nb, const unsigned value){
        REDASSERT(this->has_room(nb));
        ::out_bytes_le(this->p, nb, value);
        this->p += nb;
    }

    void set_out_bytes_le(const uint8_t nb, const unsigned value, size_t offset){
        ::out_bytes_le(this->get_data()+offset, nb, value);
    }

    // =========================================================================
    // xER encoding rules support methods
    // =========================================================================

    enum {
        ER_CLASS_MASK = 0xC0,
        ER_PC_MASK    = 0x20,
        ER_TAG_MASK   = 0x1F,
    };

    enum {
        // Tag - bits 5 to 1
        ER_TAG_BOOLEAN          = 0x01,
        ER_TAG_INTEGER          = 0x02,
        ER_TAG_BIT_STRING       = 0x03,
        ER_TAG_OCTET_STRING     = 0x04,
        ER_TAG_OBJECT_IDENFIER  = 0x06,
        ER_TAG_ENUMERATED       = 0x0A,
        ER_TAG_SEQUENCE         = 0x10,
        ER_TAG_SEQUENCE_OF      = 0x10,
        ER_TAG_GENERAL_STRING   = 0x1B,
        ER_TAG_GENERALIZED_TIME = 0x18,
        // P/C - bit 6
        ER_PRIMITIVE            = 0x00, // 0
        ER_CONSTRUCT            = 0x20, // 1
        // Class - bits 8 and 7
        ER_CLASS_UNIV           = 0x00, // 0 0
        ER_CLASS_APPL           = 0x40, // 0 1
        ER_CLASS_CTXT           = 0x80, // 1 0
        ER_CLASS_PRIV           = 0xC0, // 1 1
    };

    // =========================================================================
    // DER encoding rules support methods
    // =========================================================================

    // return the number of bytes of der length
    uint8_t out_der_length(uint16_t length){
        if (length < 0x80){
            this->out_uint8(length);
            return 1;
        }
        else if (length < 0x100){
            this->out_uint8(0x81);
            this->out_uint8(length);
            return 2;
        }
        else {
            this->out_uint8(0x82);
            this->out_uint8(length);
            return 3;
        }
    }

    void out_der_general_string(const char* str)
    {
        uint16_t length = strlen(str);
        this->out_uint8(ER_CLASS_UNIV | ER_PRIMITIVE | ER_TAG_GENERAL_STRING);
        this->out_der_length(length);
        this->out_copy_bytes(str, length);
    }


    // =========================================================================
    // PER encoding rules support methods
    // =========================================================================

    uint16_t in_per_length_with_check(bool & result)
    {
        uint16_t length = 0;

        result = true;
        if (this->in_check_rem(1)){
            length = this->in_uint8();
            if (length & 0x80){
                if (this->in_check_rem(1)){
                    length = ((length & 0x7F) << 8);
                    length += this->in_uint8();
                }
                else {
                    // error
                    LOG(LOG_ERR, "Truncated PER length (need=1, remain=0)");
                    length = 0;
                    result = false;
                }
            }
        }
        else {
            LOG(LOG_ERR, "Truncated PER length (need=1, remain=0)");
            result = false;
        }
        return length;
    }
};

// BStream is for "buffering stream", as this stream allocate a work buffer.
class BStream : public Stream {
    private:
    uint8_t autobuffer[AUTOSIZE];

    public:
    BStream(size_t size = AUTOSIZE) {
        this->capacity = 0;
        this->init(size);
    }

    virtual ~BStream() {
        // <this->data> is allocated dynamically.
        if (this->capacity > AUTOSIZE) {
            delete [] this->data;
        }
    }

    // a default buffer of 65536 bytes is allocated automatically, we will only allocate dynamic memory if we need more.
    virtual void init(size_t v) {
        if (v != this->capacity) {
            try {
                // <this->data> is allocated dynamically.
                if (this->capacity > AUTOSIZE){
                    delete [] this->data;
                }
                if (v > AUTOSIZE){
                    this->data = new uint8_t[v];
                }
                else {
                    this->data = &(this->autobuffer[0]);
                }
                this->capacity = v;
            }
            catch (...){
                this->data = 0;
                this->capacity = 0;
                LOG(LOG_ERR, "failed to allocate buffer : size asked = %d\n", static_cast<int>(v));
                throw Error(ERR_STREAM_MEMORY_ALLOCATION_ERROR);
            }
        }
        this->p = this->data;
        this->end = this->data;
    }
};

class HStream : public BStream {
public:
    size_t    reserved_leading_space;
    uint8_t * data_start;

    HStream(size_t reserved_leading_space, size_t total_size = AUTOSIZE)
            : BStream(total_size)
            , reserved_leading_space(reserved_leading_space) {
        if (reserved_leading_space > total_size) {
            LOG( LOG_ERR
               , "failed to allocate buffer : total_size=%u, reserved_leading_space=%u\n"
               , total_size, reserved_leading_space);
            throw Error(ERR_STREAM_VALUE_TOO_LARGE_FOR_RESERVED_LEADING_SPACE);
        }

        this->p          += this->reserved_leading_space;
        this->data_start  = this->p;
        this->end         = this->p;
    }

    virtual ~HStream() {}

    void copy_to_head(const uint8_t * v, size_t n) {
        if (this->data_start - this->data >= (ssize_t)n) {
            this->data_start -= n;

            ::memcpy(this->data_start, v, n);
        }
        else {
            LOG( LOG_ERR
               , "reserved leading space too small : size available = %d, size asked = %d\n"
               , this->data_start - this->data
               , static_cast<int>(n));
            throw Error(ERR_STREAM_RESERVED_LEADING_SPACE_TOO_SMALL);
        }
    }

    void copy_to_head(const char * v, size_t n) {
        if (this->data_start - this->data >= (ssize_t)n) {
            this->data_start -= n;

            ::memcpy(this->data_start, v, n);
        }
        else {
            LOG( LOG_ERR
               , "reserved leading space too small : size available = %d, size asked = %d\n"
               , this->data_start - this->data
               , static_cast<int>(n));
            throw Error(ERR_STREAM_RESERVED_LEADING_SPACE_TOO_SMALL);
        }
    }

    void copy_to_head(const Stream & stream) {
        size_t n = stream.size();
        if (this->data_start - this->data >= (ssize_t)n) {
            this->data_start -= n;

            ::memcpy(this->data_start, stream.get_data(), n);
        }
        else {
            LOG( LOG_ERR
               , "reserved leading space too small : size available = %d, size asked = %d\n"
               , this->data_start - this->data
               , static_cast<int>(n));
            throw Error(ERR_STREAM_RESERVED_LEADING_SPACE_TOO_SMALL);
        }
    }

    virtual size_t get_capacity() const {
        return this->capacity - this->reserved_leading_space;
    }

    virtual uint8_t * get_data() const {
        return this->data_start;
    }

    virtual void init(size_t body_size) {
        BStream::init(this->reserved_leading_space + body_size);

        this->p          += this->reserved_leading_space;
        this->data_start  = this->p;
        this->end         = this->p;
    }

    virtual void reset() {
        BStream::reset();

        this->p          += this->reserved_leading_space;
        this->data_start  = this->p;
        this->end         = this->p;
    }

    virtual void rewind() {
        this->p          = this->data + this->reserved_leading_space;
        this->data_start = this->p;
    }
};

// SubStream does not allocate any buffer
// (and the buffer pointed to should probably not be modifiable,
// but I'm not yet doing any distinction between stream that can or can't be modified
// many at some future time)
class SubStream : public Stream {
    public:
    SubStream(){}  // not yet initialized

    SubStream(const Stream & stream, size_t offset = 0, size_t new_size = 0)
    {
        if ((offset + new_size) > stream.get_capacity()){
            LOG(LOG_ERR, "Substream allocation overflow capacity=%u offset=%u new_size=%u",
                static_cast<unsigned>(stream.get_capacity()),
                static_cast<unsigned>(offset),
                static_cast<unsigned>(new_size));
            throw Error(ERR_SUBSTREAM_OVERFLOW_IN_CONSTRUCTOR);
        }
        this->p = this->data = stream.get_data() + offset;
        this->capacity = (new_size == 0)?(stream.get_capacity() - offset):new_size;
        this->end = this->data + this->capacity;
    }

    void resize(const Stream & stream, size_t new_size){
        this->data = this->p = stream.p;
        if (new_size > (stream.room())){
            LOG(LOG_ERR, "Substream resize overflow capacity=%u offset=%u new_size=%u",
                static_cast<unsigned>(stream.get_capacity()),
                static_cast<unsigned>(stream.get_offset()),
                static_cast<unsigned>(new_size));
            throw Error(ERR_SUBSTREAM_OVERFLOW_IN_RESIZE);
        }
        this->capacity = new_size;
        this->end = stream.p + new_size;
    }

    virtual ~SubStream() {}

    // Not allowed on SubStreams
    virtual void init(size_t v) {}
};

// FixedSizeStream does not allocate/reallocate any buffer
class FixedSizeStream : public Stream {
    public:
    FixedSizeStream(){}  // not yet initialized

    FixedSizeStream(uint8_t * data, size_t len){
        this->p = this->data = data;
        this->capacity = len;
        this->end = this->data + this->capacity;
    }

    // Not allowed on SubStreams
    virtual void init(size_t v) {}
};

// StaticStream does not allocate/reallocate any buffer
// All write operations are forbidden
class StaticStream : public FixedSizeStream {
    public:
    StaticStream(){}  // not yet initialized

    StaticStream(const uint8_t * data, size_t len){
        this->p = this->data = const_cast<uint8_t *>(data);
        this->capacity = len;
        this->end = this->data + this->capacity;
    }

    StaticStream(const char * data, size_t len){
        this->p = this->data = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(data));
        this->capacity = len;
        this->end = this->data + this->capacity;
    }

    // Not allowed on SubStreams
    virtual void init(size_t v) {}
};

#endif
