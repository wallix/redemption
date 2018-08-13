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

#include "core/RDP/rdp_pointer.hpp"


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

    void reset() {
        this->pointer_stamp = 0;

        for (auto & stamp : this->stamps) {
            stamp = 0;
        }

        for (auto & cached_ : this->cached) {
            cached_ = false;
        }

        for (auto & pointer : this->Pointers) {
            pointer = Pointer();
        }
    }

    void add_pointer_static(const Pointer & cursor, int index) {
        assert((index >= 0) && (index < MAX_POINTER_COUNT));
        this->Pointers[index] = cursor;
        this->stamps[index] = this->pointer_stamp;
        this->cached[index] = true;
    }

    /* check if the pointer is in the cache or not and if it should be sent      */
    int add_pointer(const Pointer & cursor, int & cache_idx)
    {
        int oldest = 0x7fffffff;
        int index = 0;

        this->pointer_stamp++;
        /* look for match */
        for (int i = 0; i < this->pointer_cache_entries; i++) {
            if (this->Pointers[i] == cursor) {
                this->stamps[i] = this->pointer_stamp;
                cache_idx = i;
                return POINTER_ALLREADY_SENT;
            }
        }
        /* look for oldest */
        for (int i = 0; i < this->pointer_cache_entries; i++) {
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

