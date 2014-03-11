/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2014
 *   Author(s): Christophe Grosjean, Meng Tan
 */
#ifndef _REDEMPTION_UTILS_GAPBUFFER_HPP_
#define _REDEMPTION_UTILS_GAPBUFFER_HPP_

#include <stdint.h>
#include <stdio.h>
class GapBuffer {
    uint8_t * p;
    uint8_t * data;
    uint8_t * gap_begin;
    uint8_t * gap_end;
    uint8_t * end;

    uint8_t buffer[1024];

    void init_static_buffer() {
        this->data = this->buffer;
        this->p = this->gap_begin = this->buffer;
        this->end = this->gap_end = this->buffer + sizeof(this->buffer);
    }

    size_t gap_size() {
        return this->gap_end - this->gap_begin;
    }
    size_t left_size() {
        return this->gap_begin - this->data;
    }
    size_t right_size() {
        return this->end - this->gap_end;
    }

    void move(uint8_t * dest, uint8_t * src, size_t length) {
        if ((dest == src) || (length == 0)) {
            return;
        }

        if (src > dest) {
            if ((src + length) >= this->end) {
                length = this->end - src - 1;
            }

            for (; length > 0; length--) {
                *(dest++) = *(src++);
            }
        }
        else {
            src += length;
            dest += length;
            for (; length > 0; length--) {
                *(--dest) = *(--src);
            }
        }
    }


public:
    GapBuffer() {
        this->init_static_buffer();
    }


    virtual ~GapBuffer() {
        // if (this->data) {
        //     free(this->data);
        // }
    }

    int size() {
        return (end - data) - this->gap_size();
    }

    void move_gap_to_p() {
        if (this->p == this->gap_begin) {
            return;
        }
        if (this->p == this->gap_end) {
            this->p = this->gap_begin;
            return;
        }

        if (this->p < this->gap_begin) {
            size_t length = this->gap_begin - this->p;
            this->move(this->p + this->gap_size(), this->p, length);
            this->gap_end -= length;
            this->gap_begin = this->p;
        }
        else {
            size_t length = this->p - this->gap_end;
            this->move(this->gap_begin, this->gap_end, length);
            this->gap_begin += length;
            this->gap_end = this->p;
            this->p = this->gap_begin;
        }
    }

    uint8_t get_byte() {
        if (this->p == this->gap_begin) {
            this->p = this->gap_end;
        }
        return *(this->p);
    }
    uint8_t get_prev_byte() {
        if (this->p == this->gap_end) {
            this->p = this->gap_begin;
        }
        return *(--this->p);
    }

    void replace_byte(uint8_t byte) {
        if (this->p == this->gap_begin) {
            this->p = this->gap_end;
        }
        if (this->p == this->end) {
            this->put_byte(byte);
        }
        *(this->p) = byte;
    }

    uint8_t next_byte() {
        if (this->p == this->gap_begin) {
            this->p = this->gap_end;
            return *(this->p);
        }
        return *(++(this->p));
    }

    void put_byte(uint8_t byte) {
        this->insert_byte(byte);
        this->p++;
    }

    void insert_byte(uint8_t byte) {
        this->move_gap_to_p();

        if (this->gap_begin == this->gap_end) {
            // buffer is full !
            return;
        }
        *(this->gap_begin++) = byte;
    }

    void p_forward() {
        this->p = (this->gap_end - this->p) * (this->p == this->gap_begin) + this->p;
        this->p += (this->p != this->end);
    }

    void p_backward() {
        this->p = (this->gap_begin - this->p) * (this->p == this->gap_end) + this->p;
        this->p -= (this->p != this->data);
    }

    int p_pos() {
        return this->p - this->data - (this->gap_size() * (this->p > this->gap_begin));
    }


    void delete_bytes(size_t size) {
        this->move_gap_to_p();
        this->gap_end += size;
    }

    int insert_bytes(const uint8_t * bytes, size_t length) {
        this->move_gap_to_p();
        if (length > this->gap_size()) {
            length = this->gap_size();
        }

        memcpy(this->p, bytes, length);
        this->p += length;
        this->gap_begin += length;
        return length;
    }

    size_t get_buff(uint8_t * buff, int buff_size, size_t offset = 0) {
        int length = this->size() - offset;
        if (length <= 0) {
            return 0;
        }
        if (buff_size < length) {
            length = buff_size;
        }
        int left_length = this->gap_begin - (this->data + offset);
        bool right_side = true;
        int right_offset = 0;
        if (left_length > 0) {
            if (length <= left_length) {
                left_length = length;
                right_side = false;
            }
            memcpy(buff, this->data + offset, left_length);
        }
        else {
            right_offset = -left_length;
            left_length = 0;
        }
        if (right_side) {
            memcpy(buff + left_length, this->gap_end + right_offset,
                   length - left_length);
        }
        return length;
    }



};

#endif
