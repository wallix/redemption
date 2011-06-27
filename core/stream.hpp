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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities

*/

#if !defined(___STREAM_HPP__)
#define ___STREAM_HPP__

#include "log.hpp"

#include <stdio.h>
#include <string.h> // for memcpy, memset

#include "constants.hpp"
#include "ssl_calls.hpp"
#include "file_loc.hpp"
#include "error.hpp"
#include <string.h>
#include "altoco.hpp"
#include <stdio.h>

enum {
     AUTOSIZE = 65536
};

/* parser state */
class Stream {
    public:
    uint8_t* p;
    uint8_t* end;
    uint8_t* data;
    size_t capacity;

    /* offsets of various headers */
    uint8_t* iso_hdr;
    uint8_t* mcs_hdr;
    uint8_t* sec_hdr;
    uint8_t* rdp_hdr;
    uint8_t* channel_hdr;
    uint8_t* next_packet;
    uint8_t autobuffer[AUTOSIZE];

    Stream(size_t size = AUTOSIZE) {
        this->capacity = 0;
        this->init(size);
    }
    ~Stream() {
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
                    this->data = new uint8_t[v];
//                    LOG(LOG_DEBUG, "Stream buffer allocation succeeded : size=%d @%p\n", v, this->data);
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
        this->next_packet = (uint8_t*)0;
    }

//    /* Buffer underflow, false if read after the end of the buffer
//       (a previous read accessed after buffer end)
//    */
#warning:CGR: replace buffer underflow condition by checking if there is enough data in buffer before reading using check_rem(), should be easy to do as there is not many calls to check remaining
    bool check(void) {
        return this->p <= this->end;
    }

    void set_length(int offset, uint8_t * const length_ptr){
        uint16_t length = this->p - length_ptr + offset;
        length_ptr[0] = length;
        length_ptr[1] = (length >> 8);
    }

    int free_size(){
        return this->end - this->p;
    }

    bool check_rem(unsigned n) {
        return (this->p + n) <= this->end;
    }

    bool check_end(void) {
        return this->p == this->end;
    }

    signed char in_sint8(void) {
        return *((signed char*)(this->p++));
    }

    unsigned char in_uint8(void) {
        return *((unsigned char*)(this->p++));
    }

    signed int in_sint16_be(void) {
        unsigned int v = this->in_uint16_be();
        return (v > 32767)?v - 65536:v;
    }

    signed int in_sint16_le(void) {
        unsigned int v = this->in_uint16_le();
        return (v > 32767)?v - 65536:v;
    }

    unsigned int in_uint16_le(void) {
        this->p += 2;
        return ((unsigned char*)this->p)[-2] + ((unsigned char*)this->p)[-1] * 256;
    }

    unsigned int in_uint16_be(void) {
        this->p += 2;
        return ((unsigned char*)this->p)[-1] + ((unsigned char*)this->p)[-2] * 256;
    }

    #warning use in_bytes_le whenever possible
    unsigned int in_uint32_le(void) {
        this->p += 4;
        return this->p[-4]
             | (this->p[-3] << 8)
             | (this->p[-2] << 16)
             | (this->p[-1] << 24)
             ;
    }

    unsigned in_bytes_le(const uint8_t nb){
        this->p += nb;
        return ::in_bytes_le(nb, this->p - nb);
    }


    unsigned int in_uint32_be(void) {
        this->p += 4;
        return ((unsigned char*)this->p)[-1]
               + ((unsigned char*)this->p)[-2] * 0x100
               + ((unsigned char*)this->p)[-3] * 0x10000
               + ((unsigned char*)this->p)[-4] * 0x1000000
               ;
    }

    const uint8_t *in_uint8p(unsigned int n) {
        this->p+=n;
        return this->p - n;
    }

    void skip_uint8(unsigned int n) {
        this->p+=n;
    }

    void out_uint8(unsigned char v) {
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
            this->out_uint8(v);
        }
        else {
            this->out_uint16_be(v|0x8000);
        }
    }

    void set_out_2BUE(uint16_t v, size_t offset){
        if (v <= 127){
            this->set_out_uint8(v, offset);
        }
        else {
            this->set_out_uint16_be(v|0x8000, offset);
        }
    }


    void out_sint8(char v) {
        *(this->p++) = v;
    }

    void set_out_sint8(char v, size_t offset) {
        this->data[offset] = v;
    }

    void out_uint16_le(unsigned int v) {
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_uint16_le(unsigned int v, size_t offset) {
        this->data[offset] = v & 0xFF;
        this->data[offset+1] = (v >> 8) & 0xFF;
    }

    void out_uint16_be(unsigned int v) {
        this->p[1] = v & 0xFF;
        this->p[0] = (v >> 8) & 0xFF;
        this->p+=2;
    }

    void set_out_uint16_be(unsigned int v, size_t offset) {
        this->data[offset+1] = v & 0xFF;
        this->data[offset] = (v >> 8) & 0xFF;
    }

    void out_uint32_le(unsigned int v) {
        this->p[0] = v & 0xFF;
        this->p[1] = (v >> 8) & 0xFF;
        this->p[2] = (v >> 16) & 0xFF;
        this->p[3] = (v >> 24) & 0xFF;
        this->p+=4;
    }

    void set_out_uint32_le(unsigned int v, size_t offset) {
        this->data[offset+0] = v & 0xFF;
        this->data[offset+1] = (v >> 8) & 0xFF;
        this->data[offset+2] = (v >> 16) & 0xFF;
        this->data[offset+3] = (v >> 24) & 0xFF;
    }

    void out_uint32_be(unsigned int v) {
        this->p[0] = (v >> 24) & 0xFF;
        this->p[1] = (v >> 16) & 0xFF;
        this->p[2] = (v >> 8) & 0xFF;
        this->p[3] = v & 0xFF;
        this->p+=4;
    }

    void set_out_uint32_be(unsigned int v, size_t offset) {
        this->data[offset+0] = (v >> 24) & 0xFF;
        this->data[offset+1] = (v >> 16) & 0xFF;
        this->data[offset+2] = (v >> 8) & 0xFF;
        this->data[offset+3] = v & 0xFF;
    }

    void out_unistr(const char* text)
    {
        for (int i=0; text[i]; i++) {
            this->out_uint8(text[i]);
            this->out_uint8(0);
        }
        this->out_uint8(0);
        this->out_uint8(0);
    }

    void set_out_unistr(const char* text, size_t offset)
    {
        int i=0;
        for (; text[i]; i++) {
            this->set_out_uint8(text[i], offset+i*2);
            this->set_out_uint8(0, offset+i*2+1);
        }
        this->set_out_uint8(0, offset+i*2);
        this->set_out_uint8(0, offset+i*2+1);
    }

    // sz utf16 bytes are translated to ascci, 00 terminated
    void in_uni_to_ascii_str(char* text, size_t sz)
    {
        int i = 0;
        int max = (sz>>1)-1;
        while (i < max) {
            this->skip_uint8(1);
            text[i++] = this->in_uint8();
        }
        text[i] = 0;
        this->skip_uint8(2);
    }

    void mark_end() {
        this->end = this->p;
    }

    void goto_end() {
        this->p = this->end;
    }

    void out_copy_bytes(const uint8_t * v, size_t n) {
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

    void out_clear_bytes(size_t n) {
        memset(this->p, 0, n);
        this->p += n;
    }

    void set_out_clear_bytes(size_t n, size_t offset) {
        memset(this->data+offset, 0, n);
    }

    void out_bytes_le(const uint8_t nb, const unsigned value){
        ::out_bytes_le(this->p, nb, value);
        this->p += nb;
    }

    void set_out_bytes_le(const uint8_t nb, const unsigned value, size_t offset){
        ::out_bytes_le(this->data+offset, nb, value);
    }

    // functions below are used in bitmap compress, it should be some kind of specialized stream instead of main stream
    /*****************************************************************************/
    void out_count(const int in_count, const int mask){
        if (in_count < 32) {
            this->out_uint8((mask << 5) | in_count);
        }
        else if (in_count < 256 + 32){
            this->out_uint8(mask << 5);
            this->out_uint8(in_count - 32);
        }
        else {
            this->out_uint8(0xf0 | mask);
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
            this->out_uint8(0xc0 | in_count);
        }
        else if (in_count < 256 + 16){
            this->out_uint8(0xc0);
            this->out_uint8(in_count - 16);
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
                this->out_uint8(in_count - 1);
            }
            else{
                this->out_uint8(0x40 | (in_count >> 3));
            }
        }
        else{
            this->out_uint8(0xf2);
            this->out_uint16_le(in_count);
        }
    }

    void out_fom_sequence(const int count, const uint8_t * masks) {
        this->out_fom_count(count);
        this->out_copy_bytes(masks, nbbytes(count));
    }

    void out_fom_count_set(const int in_count)
    {
        if (in_count < 256){
            if (in_count & 0x87){
                this->out_uint8(0xD0);
                this->out_uint8(in_count - 1);
            }
            else{
                this->out_uint8(0xD0 | (in_count >> 3));
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
        this->out_copy_bytes(masks, nbbytes(count));
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
            this->out_uint8(0xe0 | (in_count / 2));
        }
        else if (in_count / 2 < 256 + 16){
            this->out_uint8(0xe0);
            this->out_uint8(in_count / 2 - 16);
        }
        else{
            this->out_uint8(0xf | mask);
            this->out_uint16_le(in_count / 2);
        }
    }


};

#endif
