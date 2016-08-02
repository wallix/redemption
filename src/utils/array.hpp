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

#include "utils/log.hpp"

#include <stdio.h>
#include <string.h> // for memcpy, memset
#include <algorithm>

#include "core/error.hpp"
#include "utils/bitfu.hpp"
//#include "utils/utf.hpp"

// using a template for default size of stream would make sense instead of always using the large buffer below

class Array {
    enum {
        AUTOSIZE = 65536
    };

    uint8_t* data;
    size_t capacity;
    private:
    uint8_t autobuffer[AUTOSIZE];

    public:
    explicit Array(size_t size = AUTOSIZE)
        : data(nullptr)
        , capacity(0)
        , autobuffer()
    {
        this->data = this->autobuffer;
        this->init(size);
    }

    ~Array() {
        // <this->data> is allocated dynamically.
        if (this->capacity > AUTOSIZE) {
            delete [] this->data;
        }
    }

private:
    Array(Array const &) /* = delete*/;
    Array& operator=(Array const &) /* = delete*/;

public:
    size_t size() const {
        return this->capacity;
    }

    uint8_t * get_data() const {
        return this->data;
    }

    void copy(Array & other) {
        this->init(other.size());
        memcpy(this->get_data(), other.get_data(), this->size());
    }

    // a default buffer of 65536 bytes is allocated automatically, we will only allocate dynamic memory if we need more.
    void init(size_t v) {
        if (v != this->capacity) {
            // <this->data> is allocated dynamically.
            if (this->capacity > AUTOSIZE){
                delete [] this->data;
            }

            this->capacity = v;
            if (v > AUTOSIZE){
                this->data = new(std::nothrow) uint8_t[v];
                if (!this->data) {
                    this->capacity = 0;
                    LOG(LOG_ERR, "failed to allocate buffer : size asked = %d\n", static_cast<int>(v));
                    throw Error(ERR_STREAM_MEMORY_ALLOCATION_ERROR);
                }
            }
            else {
                this->data = &(this->autobuffer[0]);
            }
        }
    }

    void copy(const uint8_t * source, size_t size, uint32_t offset = 0) {
        REDASSERT(this->capacity >= size + offset);
        memcpy(this->data + offset, source, size);
    }
};

