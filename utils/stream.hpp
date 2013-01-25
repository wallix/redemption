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

#if !defined(___UTILS_STREAM_HPP__)
#define ___UTILS_STREAM_HPP__

#include "log.hpp"

#include <stdio.h>
#include <string.h> // for memcpy, memset
#include <algorithm>

#include "constants.hpp"
#include "ssl_calls.hpp"
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
    uint8_t* data;
    size_t capacity;

    virtual ~Stream() {}

    virtual void init(size_t capacity) = 0;

    void reset(){
        this->end = this->p = this->data;
    }

    bool has_room(unsigned n) const {
        return this->get_offset() + n <= this->capacity;
    }

    uint16_t get_offset() const {
        return this->p - this->data;
    }

    uint16_t room() const {
        return this->capacity - this->get_offset();
    }

    bool in_check_rem(unsigned n) {
    // returns true if there is enough data available to read n bytes
        bool res = (this->p + n) <= this->end;
        return res;
    }

    bool check_end(void) {
        return this->p == this->end;
    }

    // =========================================================================
    // Generic binary Data access methods
    // =========================================================================

    signed char in_sint8(void) {
        REDASSERT(in_check_rem(1));
        return *((signed char*)(this->p++));
    }

    unsigned char in_uint8(void) {
        REDASSERT(in_check_rem(1));
        return *((unsigned char*)(this->p++));
    }

    int16_t in_sint16_be(void) {
        REDASSERT(in_check_rem(2));
        unsigned int v = this->in_uint16_be();
        return (int16_t)((v > 32767)?v - 65536:v);
    }

    int16_t in_sint16_le(void) {
        REDASSERT(in_check_rem(2));
        unsigned int v = this->in_uint16_le();
        return (int16_t)((v > 32767)?v - 65536:v);
    }

    uint16_t in_uint16_le(void) {
        REDASSERT(in_check_rem(2));
        this->p += 2;
        return (uint16_t)(this->p[-2] | (this->p[-1] << 8));
    }

    uint16_t in_uint16_be(void) {
        REDASSERT(in_check_rem(2));
        this->p += 2;
        return (uint16_t)(this->p[-1] | (this->p[-2] << 8)) ;
    }

    unsigned int in_uint32_le(void) {
        REDASSERT(in_check_rem(4));
        this->p += 4;
        return  this->p[-4]
             | (this->p[-3] << 8)
             | (this->p[-2] << 16)
             | (this->p[-1] << 24)
             ;
    }

    unsigned int in_uint32_be(void) {
        REDASSERT(in_check_rem(4));
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
        REDASSERT(has_room(8));
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
        REDASSERT(in_check_rem(8));
        uint64_t low = this->in_uint32_le();
        uint64_t high = this->in_uint32_le();
        return low + (high << 32);
    }

    void out_uint64_be(uint64_t v) {
        REDASSERT(has_room(8));
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
        REDASSERT(in_check_rem(8));
        uint64_t high = this->in_uint32_be();
        uint64_t low = this->in_uint32_be();
        return low + (high << 32);
    }

    unsigned in_bytes_le(const uint8_t nb){
        REDASSERT(in_check_rem(nb));
        this->p += nb;
        return ::in_bytes_le(nb, this->p - nb);
    }

    unsigned in_bytes_be(const uint8_t nb){
        REDASSERT(in_check_rem(nb));
        this->p += nb;
        return ::in_bytes_be(nb, this->p - nb);
    }

    void in_copy_bytes(uint8_t * v, size_t n) {
        REDASSERT(in_check_rem(n));
        memcpy(v, this->p, n);
        this->p += n;
    }

    void in_copy_bytes(char * v, size_t n) {
        this->in_copy_bytes((uint8_t*)(v), n);
    }

    const uint8_t *in_uint8p(unsigned int n) {
        REDASSERT(in_check_rem(n));
        this->p+=n;
        return this->p - n;
    }

    void in_skip_bytes(unsigned int n) {
        REDASSERT(in_check_rem(n));
        this->p+=n;
    }

    void out_skip_bytes(unsigned int n) {
        REDASSERT(has_room(n));
        this->p+=n;
    }

    void out_uint8(unsigned char v) {
        REDASSERT(has_room(1));
        *(this->p++) = v;
    }

    void set_out_uint8(unsigned char v, size_t offset) {
        this->data[offset] = v;
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
            this->out_uint8((uint8_t)v);
        }
        else {
            this->out_uint16_be(v|0x8000);
        }
    }

    void set_out_2BUE(uint16_t v, size_t offset){
        if (v <= 127){
            this->set_out_uint8((uint8_t)v, offset);
        }
        else {
            this->set_out_uint16_be(v|0x8000, offset);
        }
    }


    void out_sint8(char v) {
        REDASSERT(has_room(1));
        *(this->p++) = v;
    }

    void set_out_sint8(char v, size_t offset) {
        this->data[offset] = v;
    }

    void out_uint16_le(unsigned int v) {
        REDASSERT(has_room(2));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_uint16_le(unsigned int v, size_t offset) {
        this->data[offset] = v & 0xFF;
        this->data[offset+1] = (v >> 8) & 0xFF;
    }

    void out_sint16_le(signed int v) {
        REDASSERT(has_room(2));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_sint16_le(signed int v, size_t offset) {
        this->data[offset] = v & 0xFF;
        this->data[offset+1] = (v >> 8) & 0xFF;
    }

    void out_uint16_be(unsigned int v) {
        REDASSERT(has_room(2));
        this->p[1] = v & 0xFF;
        this->p[0] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_uint16_be(unsigned int v, size_t offset) {
        this->data[offset+1] = v & 0xFF;
        this->data[offset] = (v >> 8) & 0xFF;
    }

    void out_uint32_le(unsigned int v) {
        REDASSERT(has_room(4));
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (uint8_t)(v >> 24) & 0xFF;
        this->p+=4;
    }

    void set_out_uint32_le(unsigned int v, size_t offset) {
        this->data[offset+0] = v & 0xFF;
        this->data[offset+1] = (v >> 8) & 0xFF;
        this->data[offset+2] = (v >> 16) & 0xFF;
        this->data[offset+3] = (uint8_t)(v >> 24) & 0xFF;
    }

    void out_uint32_be(unsigned int v) {
        REDASSERT(has_room(4));
        this->p[0] = (uint8_t)(v >> 24) & 0xFF;
        this->p[1] = (v >> 16) & 0xFF;
        this->p[2] = (v >> 8) & 0xFF;
        this->p[3] = v & 0xFF;
        this->p+=4;
    }

    void set_out_uint32_be(unsigned int v, size_t offset) {
        REDASSERT(has_room(4));
        this->data[offset+0] = (uint8_t)(v >> 24) & 0xFF;
        this->data[offset+1] = (v >> 16) & 0xFF;
        this->data[offset+2] = (v >> 8) & 0xFF;
        this->data[offset+3] = v & 0xFF;
    }

    void out_unistr(const char* text)
    {
        const size_t len = UTF8toUTF16(reinterpret_cast<const uint8_t*>(text), this->p, this->room());
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

    void rewind(){
        this->p = this->data;
    }

    size_t size() const {
        return this->end - this->data;
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

    void out_copy_bytes(const uint8_t * v, size_t n) {
        REDASSERT(has_room(n));
        memcpy(this->p, v, n);
        this->p += n;
    }

    void set_out_copy_bytes(const uint8_t * v, size_t n, size_t offset) {
        memcpy(this->data+offset, v, n);
    }

    void out_copy_bytes(const char * v, size_t n) {
        out_copy_bytes((uint8_t*)v, n);
    }

    void set_out_copy_bytes(const char * v, size_t n, size_t offset) {
        set_out_copy_bytes((uint8_t*)v, n, offset);
    }

    void out_concat(const char * v) {
        out_copy_bytes(v, strlen(v));
    }

    void set_out_concat(const char * v, size_t offset) {
        set_out_copy_bytes((uint8_t*)v, strlen(v), offset);
    }


    void out_clear_bytes(size_t n) {
        REDASSERT(has_room(n));
        memset(this->p, 0, n);
        this->p += n;
    }

    void set_out_clear_bytes(size_t n, size_t offset) {
        memset(this->data+offset, 0, n);
    }

    void out_bytes_le(const uint8_t nb, const unsigned value){
        REDASSERT(has_room(nb));
        ::out_bytes_le(this->p, nb, value);
        this->p += nb;
    }

    void set_out_bytes_le(const uint8_t nb, const unsigned value, size_t offset){
        ::out_bytes_le(this->data+offset, nb, value);
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
    // BER encoding rules support methods
    // =========================================================================

    enum {
        BER_TAG_BOOLEAN      =    1,
        BER_TAG_INTEGER      =    2,
        BER_TAG_OCTET_STRING =    4,
        BER_TAG_RESULT       =   10,
    };


    // return string length or -1 on error
    int in_ber_octet_string(uint8_t * target, uint16_t target_len) 
    {
        if (this->in_uint8() != BER_TAG_OCTET_STRING){
            LOG(LOG_ERR, "Octet string BER tag expected");
            return -1;
        }
        size_t len = this->in_ber_len();
        if (len > target_len){
            LOG(LOG_ERR, "target string too large (max=%u, got=%u)", target_len, len);
            return -1;
        }
        this->in_copy_bytes(target, len);
        return len;
    }

    // return 0 if false, 1 if true, -1 on error
    int in_ber_boolean() 
    {
        if (this->in_uint8() != BER_TAG_BOOLEAN){
            LOG(LOG_ERR, "Boolean BER tag expected");
            return -1;
        }
        size_t len = this->in_ber_len();
        if (len != 1){
            LOG(LOG_ERR, "Boolean BER should be one byte");
            return -1;
        }
        return this->in_uint8();
    }

    unsigned int in_ber_len(void) {
        uint8_t l = this->in_uint8();
        if (l & 0x80) {
            const uint8_t nbbytes = (uint8_t)(l & 0x7F);
            unsigned int len = 0;
            for (uint8_t i = 0 ; i < nbbytes ; i++) {
                len = (len << 8) | this->in_uint8();
            }
            return len;
        }
        return l;
    }

    void out_ber_len(unsigned int v){
        if (v >= 0x80) {
            if (v >= 0x100){
                this->out_uint8(0x82);
                this->out_uint16_be(v);
            }
            else {
                this->out_uint8(0x81);
                this->out_uint8(v);
            }
        }
        else {
            this->out_uint8((uint8_t)v);
        }
    }

    void out_ber_int8(unsigned int v){
        this->out_uint8(BER_TAG_INTEGER);
        this->out_uint8(1);
        this->out_uint8((uint8_t)v);
    }

    void set_out_ber_int8(unsigned int v, size_t offset){
        this->set_out_uint8(BER_TAG_INTEGER, offset);
        this->set_out_uint8(1, offset+1);
        this->set_out_uint8((uint8_t)v, offset+2);
    }

    void out_ber_int16(int value)
    {
        this->out_uint8(BER_TAG_INTEGER);
        this->out_uint8(2);
        this->out_uint8((uint8_t)(value >> 8));
        this->out_uint8((uint8_t)value);
    }

    void set_out_ber_int16(unsigned int v, size_t offset){
        this->set_out_uint8(BER_TAG_INTEGER, offset);
        this->set_out_uint8(2,               offset+1);
        this->set_out_uint8((uint8_t)(v >> 8),        offset+2);
        this->set_out_uint8((uint8_t)v,               offset+3);
    }

    void out_ber_int24(int value)
    {
        this->out_uint8(BER_TAG_INTEGER);
        this->out_uint8(3);
        this->out_uint8((uint8_t)(value >> 16));
        this->out_uint8((uint8_t)(value >> 8));
        this->out_uint8((uint8_t)value);
    }

    void set_out_ber_int24(int value, size_t offset)
    {
        this->set_out_uint8(BER_TAG_INTEGER, offset);
        this->set_out_uint8(3,               offset+1);
        this->set_out_uint8((uint8_t)(value >> 16),     offset+2);
        this->set_out_uint8((uint8_t)(value >> 8),      offset+3);
        this->set_out_uint8((uint8_t)value,           offset+4);
    }

    void set_out_ber_len_uint7(unsigned int v, size_t offset){
        if (v >= 0x80) {
            LOG(LOG_INFO, "Value too large for out_ber_len_uint7");
            throw Error(ERR_STREAM_VALUE_TOO_LARGE_FOR_OUT_BER_LEN_UINT7);
        }
        this->set_out_uint8((uint8_t)v, offset+0);
    }

    void out_ber_len_uint7(unsigned int v){
        if (v >= 0x80) {
            LOG(LOG_INFO, "Value too large for out_ber_len_uint7");
            throw Error(ERR_STREAM_VALUE_TOO_LARGE_FOR_OUT_BER_LEN_UINT7);
        }
        this->out_uint8((uint8_t)v);
    }

    void set_out_ber_len_uint16(unsigned int v, size_t offset){
        this->set_out_uint8(0x82, offset+0);
        this->set_out_uint16_be(v, offset+1);
    }

    void out_ber_len_uint16(unsigned int v){
        this->out_uint8(0x82);
        this->out_uint16_be(v);
    }

    void set_out_ber_len(unsigned int v, size_t offset){
        if (v>= 0x80){
            REDASSERT(v < 65536);
            this->data[offset+0] = 0x82;
            this->set_out_uint16_be(v, offset+1);
        }
        else {
            this->data[offset+0] = (uint8_t)v;
        }
    }

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

    uint16_t in_per_length()
    {
        uint16_t length = this->in_uint8();
        if (length & 0x80){
            length = ((length & 0x7F) << 8);
            length += this->in_uint8();
        }
        return length;
    }

    void out_per_length(uint16_t length)
    {
        if (length & 0xFF80){
            this->out_uint16_be(length|0x8000);
        }
        else {
            this->out_uint8(length);
        }
    }

    void set_out_per_length(uint16_t length, size_t offset)
    {
        if (length & 0xFF80){
            this->set_out_uint16_be(length|0x8000, offset);
        }
        else {
            this->set_out_uint8(length, offset);
        }
    }

    uint8_t in_per_choice()
    {
        return this->in_uint8();
    }

    void out_per_choice(uint8_t choice)
    {
        this->out_uint8(choice);
    }

    uint8_t in_per_selection()
    {
        return this->in_uint8();
    }

    void out_per_selection(uint8_t selection)
    {
        this->out_uint8(selection);
    }

    uint8_t in_per_number_of_sets()
    {
        return this->in_uint8();
    }

    void out_per_number_of_sets(uint8_t number)
    {
        this->out_uint8(number);
    }

    void in_per_padding(uint8_t length)
    {
        this->in_skip_bytes(length);
    }

    void out_per_padding(uint8_t length)
    {
        this->out_clear_bytes(length);
    }

    uint32_t in_per_integer()
    {
        switch (this->in_per_length()){
        case 0: // 0 is bogus bug rdesktop sends that...
        case 1:
            return this->in_uint8();
        case 2:
            return this->in_uint16_be();
        case 4:
            return this->in_uint32_be();
        default:
            REDASSERT(0);
            return 0;
        }
    }

    void out_per_integer(uint32_t integer)
    {
        uint8_t length = (integer & 0xFFFF0000)?4:(integer & 0xFF00)?2:1;
        this->out_per_length(length);
        switch (length){
        case 4:
            this->out_uint32_be(integer);
            break;
        case 2:
            this->out_uint16_be((uint16_t)integer);
            break;
        default:
            this->out_uint8((uint8_t)integer);
            break;
        }
    }

    uint16_t in_per_integer16(uint16_t min)
    {
        return this->in_uint16_be() + min;
    }

    void out_per_integer16(uint16_t integer, uint16_t min)
    {
        this->out_uint16_be(integer+min);
    }

    uint8_t in_per_enumerated(uint8_t count)
    {
        uint8_t enumerated = this->in_uint8();
        REDASSERT(enumerated <= count);
        return enumerated;
    }

    void out_per_enumerated(uint8_t enumerated, uint8_t count)
    {
        REDASSERT(enumerated <= count);
        this->out_uint8(enumerated);
    }

    TODO("looks like it really is not an input function as we check we are getting what we expect")
    void in_per_object_identifier(const uint8_t * oid)
    {
        uint16_t length = this->in_per_length();
        (void)length;
        REDASSERT(length == 5);

        uint8_t t12 = this->in_uint8(); /* first two tuples */
        uint8_t a_oid_0 = (t12 >> 4);
        uint8_t a_oid_1 = (t12 & 0x0F);
        uint8_t a_oid_2 = this->in_uint8(); /* tuple 3 */
        uint8_t a_oid_3 = this->in_uint8(); /* tuple 4 */
        uint8_t a_oid_4 = this->in_uint8(); /* tuple 5 */
        uint8_t a_oid_5 = this->in_uint8(); /* tuple 6 */

        // this is to silent warnings when not in debug mode
        (void)a_oid_0;
        (void)a_oid_1;
        (void)a_oid_2;
        (void)a_oid_3;
        (void)a_oid_4;
        (void)a_oid_5;

        REDASSERT(a_oid_0 == oid[0]
               && a_oid_1 == oid[1]
               && a_oid_2 == oid[2]
               && a_oid_3 == oid[3]
               && a_oid_4 == oid[4]
               && a_oid_5 == oid[5]);
    }

    void out_per_object_identifier(const uint8_t * oid)
    {
        const uint8_t t12 = (oid[0] << 4) & (oid[1] & 0x0F);
        this->out_per_length(5); // length
        this->out_uint8(t12);    // first two tuples
        this->out_uint8(oid[2]); // tuple 3
        this->out_uint8(oid[3]); // tuple 4
        this->out_uint8(oid[4]); // tuple 5
        this->out_uint8(oid[5]); // tuple 6
    }

    void out_per_string(uint8_t * str, uint32_t length)
    {
        this->out_copy_bytes(str, length);
    }

//    16 Encoding the octetstring type
//    ================================
//    NOTE – Octet strings of fixed length less than or equal to two octets are
//    not octet-aligned. All other octet strings are octet-aligned in the ALIGNED
//    variant. Fixed length octet strings encode with no length octets if they
//    are shorter than 64K. For unconstrained octet strings the length is
//    explicitly encoded (with fragmentation if necessary).

//    16.1 PER-visible constraints can only constrain the length of the octetstring.

//    16.2 Let the maximum number of octets in the octetstring (as determined by
//    PER-visible constraints on the length) be "ub" and the minimum number of
//    octets be "lb". If there is no finite maximum we say that "ub" is unset.
//    If there is no constraint on the minimum then "lb" has the value zero.
//    Let the length of the actual octetstring value to be encoded be "n" octets.

//    16.3 If there is a PER-visible size constraint and an extension marker is
//    present in it, a single bit shall be added to the field-list in a bit-field
//    of length one. The bit shall be set to 1 if the length of this encoding is
//    not within the range of the extension root, and zero otherwise. In the
//    former case 16.8 shall be invoked to add the length as a semi-constrained
//    whole number to the field-list, followed by the octetstring value. In the
//    latter case the length and value shall be encoded as if the extension
//    marker is not present.

//    16.4 If an extension marker is not present in the constraint specification
//    of the octetstring type, then 16.5 to 16.8 apply.

//    16.5 If the octetstring is constrained to be of zero length ("ub" equals
//    zero), then it shall not be encoded (no additions to the field-list),
//    completing the procedures of this clause.

//    16.6 If all values of the octetstring are constrained to be of the same
//    length ("ub" equals "lb") and that length is less than or equal to two
//    octets, the octetstring shall be placed in a bit-field with a number of
//    bits equal to the constrained length "ub" multiplied by eight which shall
//    be appended to the field-list with no length determinant, completing the
//    procedures of this clause.

//    16.7 If all values of the octetstring are constrained to be of the same
//    length ("ub" equals "lb") and that length is greater than two octets but
//    less than 64K, then the octetstring shall be placed in a bit-field
//    (octet-aligned in the ALIGNED variant) with the constrained length "ub"
//    octets which shall be appended to the field-list with no length determinant,
//    completing the procedures of this clause.

//    16.8 If 16.5 to 16.7 do not apply, the octetstring shall be placed in a
//    bit-field (octet-aligned in the ALIGNED variant) of length "n" octets and
//    the procedures of 10.9 shall be invoked to add this bit-field (octet-aligned
//    in the ALIGNED variant) of "n" octets to the field-list, preceded by a
//    length determinant equal to "n" octets as a constrained whole number if "ub"
//    is set, and as a semi-constrained whole number if "ub" is unset. "lb" is
//    as determined above.

//    NOTE – The fragmentation procedures may apply after 16K, 32K, 48K, or 64K octets.

TODO("check if implementation below is conforming to obfuscated text above (I have not the faintest idea of what it means). This looks like a piece of what could be called administrative joke, isn't it. And lawyers should be forbidden to write specs documents...")
    bool check_per_octet_string(uint8_t * oct_str, uint32_t length, uint32_t min)
    {
        uint16_t mlength = this->in_per_length();
        if (mlength != std::min(length, min)){
            return false;
        }
        if (!this->in_check_rem(length)){
            return false;
        }
        this->out_skip_bytes(mlength);
        return 0 == memcmp(this->p - length, oct_str, length);
    }

    // return actual string size
    uint32_t in_per_octet_string(uint8_t * oct_str, uint32_t length, uint32_t min)
    {
        uint16_t mlength = this->in_per_length();
        REDASSERT(mlength <= length);
        if (mlength >= min){
            this->in_copy_bytes(oct_str, mlength);
            return mlength;
        }
        else {
            this->in_copy_bytes(oct_str, mlength);
            bzero(oct_str + mlength, min - mlength);
            return min;
        }
    }

    void out_per_octet_string(const uint8_t * oct_str, uint32_t length, uint32_t min)
    {
        if (length >= min){
            this->out_per_length(length - min);
            this->out_copy_bytes(oct_str, length);
        }
        else {
            TODO("Check this length, looks dubious")
            this->out_per_length(min);
            this->out_copy_bytes(oct_str, length);
            this->out_clear_bytes(min-length);
        }
    }

    // Note by CGR:
    // As far as I understand numeric string is some octet_string with alphabet
    // constraint stating that all chars should be numbers.
    // Chars are encoded in a restricted alphabet (one that can encode every
    // digits representation, 4 bits for each number should thus be enough).

    // 3.6.16 known-multiplier character string type: A restricted character
    // string type where the number of octets in the encoding is a known fixed
    // multiple of the number of characters in the character string for all
    // permitted character string values. The known-multiplier character string
    // types are IA5String, PrintableString, VisibleString, NumericString,
    // UniversalString and BMPString.

    // I should check in ISO 6093:1985, Information processing
    // – Representation of numerical values in character strings for
    // information interchange.


    void skip_per_numeric_string(uint8_t min)
    {
        uint16_t mlength = this->in_per_length();
        uint16_t length = (mlength + min + 1) >> 1;
        this->in_skip_bytes(length);
    }

    void out_per_numeric_string(uint8_t * num_str, uint16_t length, uint16_t min)
    {
        TODO("Check this length computing, looks dubious")
        uint16_t mlength = (length - min >= 0) ? length - min : min;

        this->out_per_length(mlength);

        for (uint16_t i = 0; i < length; i += 2)
        {
            uint8_t c1 = num_str[i] - '0';
            uint8_t c2 = ((i + 1) < length) ? num_str[i + 1] - '0': 0;

            this->out_uint8((c1 << 4) | c2);
        }
    }

    // =========================================================================
    // Helper methods for RDP RLE bitmap compression support
    // =========================================================================
    void out_count(const int in_count, const int mask){
        if (in_count < 32) {
            this->out_uint8((uint8_t)((mask << 5) | in_count));
        }
        else if (in_count < 256 + 32){
            this->out_uint8((uint8_t)(mask << 5));
            this->out_uint8((uint8_t)(in_count - 32));
        }
        else {
            this->out_uint8((uint8_t)(0xf0 | mask));
            this->out_uint16_le(in_count);
        }
    }

    // Background Run Orders
    // ~~~~~~~~~~~~~~~~~~~~~

    // A Background Run Order encodes a run of pixels where each pixel in the
    // run matches the uncompressed pixel on the previous scanline. If there is
    // no previous scanline then each pixel in the run MUST be black.

    // When encountering back-to-back background runs, the decompressor MUST
    // write a one-pixel foreground run to the destination buffer before
    // processing the second background run if both runs occur on the first
    // scanline or after the first scanline (if the first run is on the first
    // scanline, and the second run is on the second scanline, then a one-pixel
    // foreground run MUST NOT be written to the destination buffer). This
    // one-pixel foreground run is counted in the length of the run.

    // The run length encodes the number of pixels in the run. There is no data
    // associated with Background Run Orders.

    // +-----------------------+-----------------------------------------------+
    // | 0x0 REGULAR_BG_RUN    | The compression order encodes a regular-form  |
    // |                       | background run. The run length is stored in   |
    // |                       | the five low-order bits of  the order header  |
    // |                       | byte. If this value is zero, then the run     |
    // |                       | length is encoded in the byte following the   |
    // |                       | order header and MUST be incremented by 32 to |
    // |                       | give the final value.                         |
    // +-----------------------+-----------------------------------------------+
    // | 0xF0 MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA     |
    // |                       | background run. The run length is stored in   |
    // |                       | the two bytes following the order header      |
    // |                       | (in little-endian format).                    |
    // +-----------------------+-----------------------------------------------+

    void out_fill_count(const int in_count)
    {
        this->out_count(in_count, 0x00);
    }

    // Foreground Run Orders
    // ~~~~~~~~~~~~~~~~~~~~~

    // A Foreground Run Order encodes a run of pixels where each pixel in the
    // run matches the uncompressed pixel on the previous scanline XOR’ed with
    // the current foreground color. If there is no previous scanline, then
    // each pixel in the run MUST be set to the current foreground color (the
    // initial foreground color is white).

    // The run length encodes the number of pixels in the run.
    // If the order is a "set" variant, then in addition to encoding a run of
    // pixels, the order also encodes a new foreground color (in little-endian
    // format) in the bytes following the optional run length. The current
    // foreground color MUST be updated with the new value before writing
    // the run to the destination buffer.

    // +---------------------------+-------------------------------------------+
    // | 0x1 REGULAR_FG_RUN        | The compression order encodes a           |
    // |                           | regular-form foreground run. The run      |
    // |                           | length is stored in the five low-order    |
    // |                           | bits of the order header byte. If this    |
    // |                           | value is zero, then the run length is     |
    // |                           | encoded in the byte following the order   |
    // |                           | header and MUST be incremented by 32 to   |
    // |                           | give the final value.                     |
    // +---------------------------+-------------------------------------------+
    // | 0xF1 MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA |
    // |                           | foreground run. The run length is stored  |
    // |                           | in the two bytes following the order      |
    // |                           | header (in little-endian format).         |
    // +---------------------------+-------------------------------------------+
    // | 0xC LITE_SET_FG_FG_RUN    | The compression order encodes a "set"     |
    // |                           | variant lite-form foreground run. The run |
    // |                           | length is stored in the four low-order    |
    // |                           | bits of the order header byte. If this    |
    // |                           | value is zero, then the run length is     |
    // |                           | encoded in the byte following the order   |
    // |                           | header and MUST be incremented by 16 to   |
    // |                           | give the final value.                     |
    // +---------------------------+-------------------------------------------+
    // | 0xF6 MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set"     |
    // |                           | variant MEGA_MEGA foreground run. The run |
    // |                           | length is stored in the two bytes         |
    // |                           | following the order header (in            |
    // |                           | little-endian format).                    |
    // +---------------------------+-------------------------------------------+

    void out_mix_count(const int in_count)
    {
        this->out_count(in_count, 0x01);
    }

    void out_mix_count_set(const int in_count)
    {
        const uint8_t mask = 0x06;
        if (in_count < 16) {
            this->out_uint8((uint8_t)(0xc0 | in_count));
        }
        else if (in_count < 256 + 16){
            this->out_uint8(0xc0);
            this->out_uint8((uint8_t)(in_count - 16));
        }
        else {
            this->out_uint8(0xf0 | mask);
            this->out_uint16_le(in_count);
        }
    }

    // Foreground / Background Image Orders
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // A Foreground/Background Image Order encodes a binary image where each
    // pixel in the image that is not on the first scanline fulfils exactly one
    // of the following two properties:

    // (a) The pixel matches the uncompressed pixel on the previous scanline
    // XOR'ed with the current foreground color.

    // (b) The pixel matches the uncompressed pixel on the previous scanline.

    // If the pixel is on the first scanline then it fulfils exactly one of the
    // following two properties:

    // (c) The pixel is the current foreground color.

    // (d) The pixel is black.

    // The binary image is encoded as a sequence of byte-sized bitmasks which
    // follow the optional run length (the last bitmask in the sequence can be
    // smaller than one byte in size). If the order is a "set" variant then the
    // bitmasks MUST follow the bytes which specify the new foreground color.
    // Each bit in the encoded bitmask sequence represents one pixel in the
    // image. A bit that has a value of 1 represents a pixel that fulfils
    // either property (a) or (c), while a bit that has a value of 0 represents
    // a pixel that fulfils either property (b) or (d). The individual bitmasks
    // MUST each be processed from the low-order bit to the high-order bit.

    // The run length encodes the number of pixels in the run.

    // If the order is a "set" variant, then in addition to encoding a binary
    // image, the order also encodes a new foreground color (in little-endian
    // format) in the bytes following the optional run length. The current
    // foreground color MUST be updated with the new value before writing
    // the run to the destination buffer.

    // +--------------------------------+--------------------------------------+
    // | 0x2 REGULAR_FGBG_IMAGE         | The compression order encodes a      |
    // |                                | regular-form foreground/background   |
    // |                                | image. The run length is encoded in  |
    // |                                | the five low-order bits of the order |
    // |                                | header byte and MUST be multiplied   |
    // |                                | by 8 to give the final value. If     |
    // |                                | this value is zero, then the run     |
    // |                                | length is encoded in the byte        |
    // |                                | following the order header and MUST  |
    // |                                | be incremented by 1 to give the      |
    // |                                | final value.                         |
    // +--------------------------------+--------------------------------------+
    // | 0xF2 MEGA_MEGA_FGBG_IMAGE      | The compression order encodes a      |
    // |                                | MEGA_MEGA foreground/background      |
    // |                                | image. The run length is stored in   |
    // |                                | the two bytes following the order    |
    // |                                | header (in little-endian format).    |
    // +--------------------------------+--------------------------------------+
    // | 0xD LITE_SET_FG_FGBG_IMAGE     | The compression order encodes a      |
    // |                                | "set" variant lite-form              |
    // |                                | foreground/background image. The run |
    // |                                | length is encoded in the four        |
    // |                                | low-order bits of the order header   |
    // |                                | byte and MUST be multiplied by 8 to  |
    // |                                | give the final value. If this value  |
    // |                                | is zero, then the run length is      |
    // |                                | encoded in the byte following the    |
    // |                                | order header and MUST be incremented |
    // |                                | by 1 to give the final value.        |
    // +--------------------------------+--------------------------------------+
    // | 0xF7 MEGA_MEGA_SET_FGBG_IMAGE  | The compression order encodes a      |
    // |                                | "set" variant MEGA_MEGA              |
    // |                                | foreground/background image. The run |
    // |                                | length is stored in the two bytes    |
    // |                                | following the order header (in       |
    // |                                | little-endian format).               |
    // +-----------------------------------------------------------------------+

    void out_fom_count(const int in_count)
    {
        if (in_count < 256){
            if (in_count & 7){
                this->out_uint8(0x40);
                this->out_uint8((uint8_t)(in_count - 1));
            }
            else{
                this->out_uint8((uint8_t)(0x40 | (in_count >> 3)));
            }
        }
        else{
            this->out_uint8(0xf2);
            this->out_uint16_le(in_count);
        }
    }

    void out_fom_sequence(const int count, const uint8_t * masks) {
        this->out_fom_count(count);
        this->out_copy_bytes(masks, nbbytes_large(count));
    }

    void out_fom_count_set(const int in_count)
    {
        if (in_count < 256){
            if (in_count & 0x87){
                this->out_uint8(0xD0);
                this->out_uint8((uint8_t)(in_count - 1));
            }
            else{
                this->out_uint8((uint8_t)(0xD0 | (in_count >> 3)));
            }
        }
        else{
            this->out_uint8(0xf7);
            this->out_uint16_le(in_count);
        }
    }

    void out_fom_sequence_set(const uint8_t Bpp, const int count,
                              const unsigned foreground, const uint8_t * masks) {
        this->out_fom_count_set(count);
        this->out_bytes_le(Bpp, foreground);
        this->out_copy_bytes(masks, nbbytes_large(count));
    }

    // Color Run Orders
    // ~~~~~~~~~~~~~~~~

    // A Color Run Order encodes a run of pixels where each pixel is the same
    // color. The color is encoded (in little-endian format) in the bytes
    // following the optional run length.

    // The run length encodes the number of pixels in the run.

    // +--------------------------+--------------------------------------------+
    // | 0x3 REGULAR_COLOR_RUN    | The compression order encodes a            |
    // |                          | regular-form color run. The run length is  |
    // |                          | stored in the five low-order bits of the   |
    // |                          | order header byte. If this value is zero,  |
    // |                          | then the run length is encoded in the byte |
    // |                          | following the order header and MUST be     |
    // |                          | incremented by 32 to give the final value. |
    // +--------------------------+--------------------------------------------+
    // | 0xF3 MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA  |
    // |                          | color run. The run length is stored in the |
    // |                          | two bytes following the order header (in   |
    // |                          | little-endian format).                     |
    // +--------------------------+--------------------------------------------+

    void out_color_sequence(const uint8_t Bpp, const int count, const uint32_t color)
    {
        this->out_color_count(count);
        this->out_bytes_le(Bpp, color);
    }

    void out_color_count(const int in_count)
    {
        this->out_count(in_count, 0x03);
    }

    // Color Image Orders
    // ~~~~~~~~~~~~~~~~~~

    // A Color Image Order encodes a run of uncompressed pixels.

    // The run length encodes the number of pixels in the run. So, to compute
    // the actual number of bytes which follow the optional run length, the run
    // length MUST be multiplied by the color depth (in bits-per-pixel) of the
    // bitmap data.

    // +-----------------------------+-----------------------------------------+
    // | 0x4 REGULAR_COLOR_IMAGE     | The compression order encodes a         |
    // |                             | regular-form color image. The run       |
    // |                             | length is stored in the five low-order  |
    // |                             | bits of the order header byte. If this  |
    // |                             | value is zero, then the run length is   |
    // |                             | encoded in the byte following the order |
    // |                             | header and MUST be incremented by 32 to |
    // |                             | give the final value.                   |
    // +-----------------------------+-----------------------------------------+
    // | 0xF4 MEGA_MEGA_COLOR_IMAGE  | The compression order encodes a         |
    // |                             | MEGA_MEGA color image. The run length   |
    // |                             | is stored in the two bytes following    |
    // |                             | the order header (in little-endian      |
    // |                             | format).                                |
    // +-----------------------------+-----------------------------------------+

    void out_copy_sequence(const uint8_t Bpp, const int count, const uint8_t * data)
    {
        this->out_copy_count(count);
        this->out_copy_bytes(data, count * Bpp);
    }

    void out_copy_count(const int in_count)
    {
        this->out_count(in_count, 0x04);
    }

    // Dithered Run Orders
    // ~~~~~~~~~~~~~~~~~~~

    // A Dithered Run Order encodes a run of pixels which is composed of two
    // alternating colors. The two colors are encoded (in little-endian format)
    // in the bytes following the optional run length.

    // The run length encodes the number of pixel-pairs in the run (not pixels).

    // +-----------------------------+-----------------------------------------+
    // | 0xE LITE_DITHERED_RUN       | The compression order encodes a         |
    // |                             | lite-form dithered run. The run length  |
    // |                             | is stored in the four low-order bits of |
    // |                             | the order header byte. If this value is |
    // |                             | zero, then the run length is encoded in |
    // |                             | the byte following the order header and |
    // |                             | MUST be incremented by 16 to give the   |
    // |                             | final value.                            |
    // +-----------------------------+-----------------------------------------+
    // | 0xF8 MEGA_MEGA_DITHERED_RUN | The compression order encodes a         |
    // |                             | MEGA_MEGA dithered run. The run length  |
    // |                             | is stored in the two bytes following    |
    // |                             | the order header (in little-endian      |
    // |                             | format).                                |
    // +-----------------------------+-----------------------------------------+

    void out_bicolor_sequence(const uint8_t Bpp, const int count,
                              const unsigned color1, const unsigned color2)
    {
        this->out_bicolor_count(count);
        this->out_bytes_le(Bpp, color1);
        this->out_bytes_le(Bpp, color2);
    }

    void out_bicolor_count(const int in_count)
    {
        const uint8_t mask = 0x08;
        if (in_count / 2 < 16){
            this->out_uint8((uint8_t)(0xe0 | (in_count / 2)));
        }
        else if (in_count / 2 < 256 + 16){
            this->out_uint8((uint8_t)0xe0);
            this->out_uint8((uint8_t)(in_count / 2 - 16));
        }
        else{
            this->out_uint8(0xf | mask);
            this->out_uint16_le(in_count / 2);
        }
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
        if (this->capacity > AUTOSIZE) {
//            LOG(LOG_DEBUG, "Stream buffer freed : size=%d @%p\n", this->capacity, this->data);
            delete [] this->data;
        }
    }

    // a default buffer of 8192 bytes is allocated automatically, we will only allocate dynamic memory if we need more.
    void init(size_t v) {
        if (v != this->capacity) {
            try {
                if (this->capacity > AUTOSIZE){
//                    LOG(LOG_DEBUG, "Stream buffer freed : size=%d @%p\n", this->capacity, this->data);
                    delete [] this->data;
                }
                if (v > AUTOSIZE){
//                    LOG(LOG_DEBUG, "Stream buffer allocation succeeded : size=%d @%p\n", v, this->data);
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
                LOG(LOG_ERR, "failed to allocate buffer : size asked = %d\n", (int)v);
                throw Error(ERR_STREAM_MEMORY_ALLOCATION_ERROR);
            }
        }
        this->p = this->data;
        this->end = this->data;
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
        if ((offset + new_size) > stream.capacity){
            LOG(LOG_ERR, "Substream allocation overflow capacity=%u offset=%u new_size=%u", 
                static_cast<unsigned>(stream.capacity),
                static_cast<unsigned>(offset), 
                static_cast<unsigned>(new_size));
            throw Error(ERR_SUBSTREAM_OVERFLOW_IN_CONSTRUCTOR);
        }
        this->p = this->data = stream.data + offset;
        this->capacity = (new_size == 0)?(stream.capacity - offset):new_size;
        this->end = this->data + this->capacity;
    }

    void resize(const Stream & stream, size_t new_size){
        this->data = this->p = stream.p;
        if (new_size > (stream.room())){
            LOG(LOG_ERR, "Substream resize overflow capacity=%u offset=%u new_size=%u", 
                static_cast<unsigned>(stream.capacity),
                static_cast<unsigned>(stream.get_offset()), 
                static_cast<unsigned>(new_size));
            throw Error(ERR_SUBSTREAM_OVERFLOW_IN_RESIZE);
        }
        this->capacity = new_size;
        this->end = stream.p + new_size;
    }

    virtual ~SubStream() {}

    // Not allowed on SubStreams
    void init(size_t v) {}

};

#endif
