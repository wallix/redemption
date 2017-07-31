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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
*/


#pragma once

#include <stdint.h>
#include <string.h> // for memset
#include <stdlib.h> // for free
#include <stdio.h> // for stderr
#include <arpa/inet.h> // for ntohl

#include "utils/log.hpp"
#include "utils/parse.hpp"
#include "sashimi/serialize.hpp"

#include "sashimi/libssh/libssh.h"
#include "sashimi/libcrypto.hpp"

#define DEBUG_BUFFER

#include <memory> // for unique_ptr


inline size_t power2up(size_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    if (sizeof(size_t) == 8){
        value |= value >> (sizeof(size_t)*4);
    }
    return value;
}

/*
 * Describes a buffer state
 * [XXXXXXXXXXXXDATA PAYLOAD       XXXXXXXXXXXXXXXXXXXXXXXX]
 * ^            ^                  ^                       ^]
 * \_data points\_pos points here  \_used points here |    /
 *   here                                          Allocated
 */
struct ssh_buffer_struct {
    uint8_t *data;
    uint32_t used;
    uint32_t allocated;
    uint32_t pos;

    ssh_buffer_struct()
    : data(nullptr)
    , used(0)
    , allocated(0)
    , pos(0)
    {
        LOG(LOG_INFO, "ssh_buffer_struct::ssh_buffer_struct()");
    }

#define DEBUG_BUFFER
    void buffer_verify(){
#ifdef DEBUG_BUFFER
      int doabort=0;

      if((this->data == nullptr) && (this->allocated != 0)){
        LOG(LOG_INFO, "data is nullptr!!! allocated=%d", this->allocated);
        doabort=1;
      }
      if(this->used > this->allocated){
        LOG(LOG_INFO, "Buffer error : allocated %u, used %u\n",this->allocated, this->used);
        doabort=1;
      }
      if(this->pos > this->used){
        LOG(LOG_INFO, "Buffer error : position %u, used %u\n",this->pos, this->used);
        doabort=1;
      }
      if(this->pos > this->allocated){
          LOG(LOG_INFO, "Buffer error : position %u, allocated %u\n",this->pos, this->allocated);
          doabort=1;
      }
      if(doabort){
        abort();
      }
#endif
    }


    ~ssh_buffer_struct() {
      this->buffer_verify();

      if (this->data) {
        /* burn the data */
        memset(this->data, 0, this->allocated);
        delete [] this->data;
        this->data = nullptr;
      }
      memset(this, 'X', sizeof(*this));
    }

    /**
     * @internal
     *
     * @brief Reinitialize a SSH buffer.
     *
     * @return              0 on success, < 0 on error.
     */
    int buffer_reinit() {
      memset(this->data, 0, this->used);
      this->used = 0;
      this->pos = 0;
      this->allocated = 0;
      if (this->data) {
          delete [] this->data;
          this->data = nullptr;
      }
      return 0;
    }

     void make_headroom_for(size_t len)
     {
//        LOG(LOG_INFO, "headroom for %d allocated=%d used=%d pos=%d", len, this->allocated, this->used, this->pos);
        if (this->pos >= len) { return; }
        if ((this->pos == this->used)
        && (this->allocated > 16384)
        && (len <= 4096)) {
            this->pos = this->used = 4096;
            return;
        }

        // New buffer: we keep some headroom for prepending
        // we also keep the size big enough for our purposes
        size_t newpos = (this->pos<4096)?4096:this->pos;
        if (newpos < len){
            newpos = power2up(len);
        }

        size_t needed = power2up(16000 | (this->used + len - this->pos + newpos)) ;

//        LOG(LOG_INFO, "allocating %d allocated=%d used=%d pos=%d newpos=%d", needed, this->allocated, this->used, this->pos, newpos);

        uint8_t *newb = new uint8_t[needed];
        if (this->used - this->pos > 0) {
            memcpy(newb + newpos, this->data + this->pos, this->used - this->pos);
        }
        this->pos = newpos;
        this->used = this->used - this->pos + newpos;
        delete [] this->data;
        this->data = newb;
        this->allocated = needed;
        this->buffer_verify();
    }

     void make_tailroom_for(size_t len)
     {
//        LOG(LOG_INFO, "tailroom [%p] for %d allocated=%d used=%d pos=%d", this, static_cast<unsigned>(len), this->allocated, this->used, this->pos);
        if ((this->pos == this->used) && (this->allocated > 4096)){
            this->pos = this->used = 4096;
        }

        if (this->allocated >= (this->used + len)) {
            return;
        }

        // New buffer: we keep some headroom for prepending
        // we also keep the size big enough for our purposes
        size_t newpos = (this->pos<4096)?4096:this->pos;
        size_t needed = power2up(16000 | (this->used + len - this->pos + newpos)) ;

//        LOG(LOG_INFO, "allocating %d allocated=%d used=%d pos=%d newpos=%d", needed, this->allocated, this->used, this->pos, newpos);

        uint8_t *newb = new uint8_t[needed];
        if (this->used - this->pos > 0) {
            memcpy(newb + newpos, this->data + this->pos, this->used - this->pos);
        }
        this->used = this->used - this->pos + newpos;
        this->pos = newpos;
        delete [] this->data;
        this->data = newb;
        this->allocated = needed;

//        LOG(LOG_INFO, "done %d allocated=%d used=%d pos=%d", needed, this->allocated, this->used, this->pos);

        this->buffer_verify();
    }



    /**
     * @internal
     *
     * @brief Add data at the head of a buffer.
     *
     * @param[in]  data     The data to prepend.
     *
     * @param[in]  len      The length of data to prepend.
     *
     * @return              0 on success, -1 on error.
     */
    int buffer_prepend_data(const void *data, uint32_t len) {
        this->buffer_verify();
        this->make_headroom_for(len);
        memcpy(this->data + (this->pos - len), data, len);
        this->pos -= len;
        this->buffer_verify();
        return 0;
    }

    /**
     * @internal
     *
     * @brief Get the remaining data out of the buffer and adjust the read pointer.
     *
     * @param[in]  data     The data buffer where to store the data.
     *
     * @param[in]  len      The length to read from the buffer.
     *
     * @returns             0 if there is not enough data in buffer, len otherwise.
     */
    uint32_t buffer_get_data(void *data, uint32_t len){
        memcpy(data, this->data+this->pos,len);
        this->pos+=len;
        return len;   /* no yet support for partial reads (is it really needed ?? ) */
    }

    uint64_t in_uint64_le(void) {
        Parse p(this->data+this->pos);
        uint64_t res = p.in_uint64_le();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }

    void out_uint64_le(uint64_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint64_le(v);
        this->used += p.p - (this->data+this->used);
    }

    void out_bignum(BIGNUM * num) {
        // output the bignum as a length prefixed sequence of bytes
          int pad = 0;
          unsigned int len = BN_num_bytes(num);
          unsigned int bits = BN_num_bits(num);

          // If the first bit is set we have a negative number
          // We force it to be positive
          if (!(bits % 8) && BN_is_bit_set(num, bits - 1)) {
            pad++;
          }

          std::unique_ptr<uint8_t[]> tmp(new uint8_t[len + pad]);

          /* We have a negative number so we need a leading zero */
          if (pad) {
            tmp[0] = 0;
          }

          BN_bn2bin(num, &tmp[pad]);

          this->out_uint32_be(len+pad);
          this->out_blob(&tmp[0], len+pad);
    }


    // Network order
    uint64_t in_uint64_be(void) {
        Parse p(this->data+this->pos);
        uint64_t res = p.in_uint64_be();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }

    void out_uint64_be(uint64_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint64_be(v);
        this->used += p.p - (this->data+this->used);
    }

    uint8_t in_uint8(void) {
        Parse p(this->data+this->pos);
        uint8_t res = p.in_uint8();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }

    // Host (x86) order
    uint16_t in_uint16_le(void) {
        Parse p(this->data+this->pos);
        uint16_t res = p.in_uint16_le();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }


    // Network order
    uint16_t in_uint16_be(void) {
        Parse p(this->data+this->pos);
        uint16_t res = p.in_uint16_be();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }


    // Host (x86) order
    uint32_t in_uint32_le(void) {
        Parse p(this->data+this->pos);
        uint32_t res = p.in_uint32_le();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }


    // Network order
    uint32_t in_uint32_be(void) {
        Parse p(this->data+this->pos);
        uint32_t res = p.in_uint32_be();
        this->pos += p.p - (this->data+this->pos);
        return res;
    }

    void out_uint8(uint16_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint8(v);
        this->used += p.p - (this->data+this->used);
    }

    // Network order
    void out_uint16_be(uint16_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint16_be(v);
        this->used += p.p - (this->data+this->used);
    }

    void out_uint16_le(uint16_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint16_le(v);
        this->used += p.p - (this->data+this->used);
    }

    // Network order
    void out_uint32_be(uint32_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint32_be(v);
        this->used += p.p - (this->data+this->used);
    }

    void out_uint32_le(uint32_t v) {
        this->make_tailroom_for(sizeof(v));
        Serialize p(this->data+this->used);
        p.out_uint32_le(v);
        this->used += p.p - (this->data+this->used);
    }


    // Output some zero terminated C string to
    // ssh network format length prefixed string (uint32 prefixed)
    // (The terminating zero is not part of the string and is not sent on the wire)
    void out_length_prefixed_cstr(const char * cstr) {
        uint32_t len = strlen(cstr);
        this->out_uint32_be(len);
        this->out_blob(reinterpret_cast<const uint8_t*>(cstr), len);
    }

    void out_sshstring(const SSHString & string) {
        this->out_uint32_be(string.size());
        this->out_blob(reinterpret_cast<const uint8_t*>(&string[0]), string.size());
    }

    void out_blob(const char * blob, uint32_t len) {
        return this->out_blob(reinterpret_cast<const uint8_t *>(blob), len);
    }


    void out_blob(const uint8_t * blob, uint32_t len) {
//    this->buffer_verify();
      this->make_tailroom_for(len);
      memcpy(this->data+this->used, blob, len);
      this->used += len;
//    this->buffer_verify();
    }

    void out_blob_char(const char * blob, uint32_t len) {
        this->out_blob(reinterpret_cast<const uint8_t*>(blob), len);
    }

    void out_length_prefixed_blob_char(const char * blob, uint32_t len) {
        this->out_uint32_be(len);
        this->out_blob(reinterpret_cast<const uint8_t*>(blob), len);
    }

    SSHString in_strdup_cstr() {
        uint32_t hostlen = this->in_uint32_be();
//        char * newstr = new char[hostlen+1];
//        memcpy(newstr, this->data+this->pos, hostlen);
        SSHString str(reinterpret_cast<char *>(&this->data[this->pos]), hostlen);
        this->pos+=hostlen;
//        return str;
//    }

//        this->buffer_get_data(newstr, hostlen);
//        newstr[hostlen] = 0;
        return str;
    }

     // for output should be out_ready() (len of data ready to send) instead
     size_t in_remain() const {
        return this->used - this->pos;
    }

    uint8_t * get_pos_ptr() const {
        return this->data + this->pos;
    }

    void in_skip_bytes(size_t len)
    {
        this->pos+=len;
    }

    uint32_t buffer_pass_bytes_end(uint32_t len)
    {
      this->buffer_verify();

      if (this->used < len) {
          return 0;
      }

      this->used-=len;
      this->buffer_verify();
      return len;
    }

    int buffer_add_buffer(ssh_buffer_struct *source) {
      this->out_blob(source->get_pos_ptr(), source->in_remain());

      return 0;
    }

};


static inline void hexdump(const char * data, size_t size, unsigned line_length)
{
    char buffer[2048];
    size_t j = 0;
    for (j = 0 ; j < size ; j += line_length){
        char * line = buffer;
        line += sprintf(line, "/* %.4x */ ", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            line += sprintf(line, "0x%.2x, ", static_cast<unsigned char>(data[j+i]));
        }
        if (i < line_length){
            line += sprintf(line, "%*c", static_cast<unsigned>((line_length-i)*3), ' ');
        }

        line += sprintf(line, " // ");

        for (i = 0; i < line_length; i++){
            if (j+i >= size){ break; }
            unsigned char tmp = static_cast<unsigned>(data[j+i]);
            if ((tmp < ' ') || (tmp > '~') || (tmp == '\\')){
                tmp = '.';
            }
            line += sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            LOG(LOG_INFO, "%s", buffer);
            buffer[0]=0;
        }
    }
}
