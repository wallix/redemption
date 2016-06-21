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
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once


#include "utils/sugar/noncopyable.hpp"

#include "core/RDP/pointer.hpp"


enum {
    POINTER_TO_SEND         = 0,
    POINTER_ALLREADY_SENT
};

enum {
    MAX_POINTER_COUNT = 32
};

/* difference caches */
class PointerCache : noncopyable {
    int pointer_cache_entries;

    /* pointer */
    int pointer_stamp = 0;
    int stamps[MAX_POINTER_COUNT] = { 0 };

    bool cached[MAX_POINTER_COUNT] = { false };

public:
    Pointer Pointers[MAX_POINTER_COUNT];

public:
    explicit PointerCache(int pointer_cache_entries = 0)
    : pointer_cache_entries(pointer_cache_entries)
    {}

    ~PointerCache() = default;

    void add_pointer_static(const Pointer & cursor, int index) {
        REDASSERT((index >= 0) && (index < MAX_POINTER_COUNT));
        this->Pointers[index].x = cursor.x;
        this->Pointers[index].y = cursor.y;
        this->Pointers[index].width = cursor.width;
        this->Pointers[index].height = cursor.height;
        this->Pointers[index].bpp = cursor.bpp;
        memcpy(this->Pointers[index].data, cursor.data, cursor.data_size());
        memcpy(this->Pointers[index].mask, cursor.mask, cursor.mask_size());
        this->stamps[index] = this->pointer_stamp;

        this->cached[index] = true;
    }

    /* check if the pointer is in the cache or not and if it should be sent      */
    int add_pointer(const Pointer & cursor, int & cache_idx)
    {
        int i;
        int oldest = 0x7fffffff;
        int index = 0;

        this->pointer_stamp++;
        /* look for match */
        for (i = 0; i < this->pointer_cache_entries; i++) {
            if (this->Pointers[i].x == cursor.x
            &&  this->Pointers[i].y == cursor.y
            &&  this->Pointers[i].width == cursor.width
            &&  this->Pointers[i].height == cursor.height
            &&  this->Pointers[i].bpp == cursor.bpp
            &&  (memcmp(this->Pointers[i].data, cursor.data, cursor.data_size()) == 0)
            &&  (memcmp(this->Pointers[i].mask, cursor.mask, cursor.mask_size()) == 0)) {
                this->stamps[i] = this->pointer_stamp;
                cache_idx = i;
                return POINTER_ALLREADY_SENT;
            }
        }
        /* look for oldest */
        for (i = 0; i < this->pointer_cache_entries; i++) {
            if (this->stamps[i] < oldest) {
                oldest = this->stamps[i];
                index  = i;
            }
        }

        this->stamps[index] = this->pointer_stamp;
        cache_idx = index;
        this->add_pointer_static(cursor, index);
        return POINTER_TO_SEND;
    }

    bool is_cached(int index) const {
        return this->cached[index];
    }

    void set_cached(int index, bool cached) {
        this->cached[index] = cached;
    }
};  // struct PointerCache

