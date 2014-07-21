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

#ifndef _REDEMPTION_CORE_RDP_CACHES_POINTERCACHE_HPP_
#define _REDEMPTION_CORE_RDP_CACHES_POINTERCACHE_HPP_

#include <math.h>

#include "RDP/pointer.hpp"
#include "drawable.hpp"
#include "client_info.hpp"


enum {
    POINTER_TO_SEND         = 0,
    POINTER_ALLREADY_SENT
};

/* difference caches */
class PointerCache {
    int pointer_cache_entries;

    /* pointer */
    int pointer_stamp;

public:
    Pointer Pointers[32];

private:
    int stamps[32];

public:
    PointerCache() {
        this->pointer_cache_entries = 0;
        this->pointer_stamp = 0;
    }

    ~PointerCache() {}

    TODO(" much duplicated code with constructor and destructor  create some intermediate functions or object")
    int reset(struct ClientInfo & client_info) {
        memset(this, 0, sizeof(struct PointerCache));
        this->pointer_cache_entries = client_info.pointer_cache_entries;
        return 0;
    }

    void add_pointer_static(const Pointer & cursor, int index) {
        this->Pointers[index].x = cursor.x;
        this->Pointers[index].y = cursor.y;
        this->Pointers[index].width = cursor.width;
        this->Pointers[index].height = cursor.height;
        this->Pointers[index].bpp = cursor.bpp;
        memcpy(this->Pointers[index].data, cursor.data, cursor.data_size());
        memcpy(this->Pointers[index].mask, cursor.mask, cursor.mask_size());
        this->stamps[index] = this->pointer_stamp;
    }

    /* check if the pointer is in the cache or not and if it should be sent      */
    int add_pointer(const Pointer & cursor, int & cache_idx)
    {
        int i;
        int oldest = 0x7fffffff;
        int index = 2;

        this->pointer_stamp++;
        /* look for match */
        for (i = 2; i < this->pointer_cache_entries; i++) {
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
        for (i = 2; i < this->pointer_cache_entries; i++) {
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

};  // struct PointerCache

#endif  // #ifndef _REDEMPTION_CORE_RDP_CACHES_POINTERCACHE_HPP_
