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

   This file implement the cache used for bitmaps item

*/



#if !defined(__BITMAP_CACHE_HPP__)
#define __BITMAP_CACHE_HPP__

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include "altoco.hpp"
#include "log.hpp"
#include "client_info.hpp"
#include "stream.hpp"
#include "colors.hpp"
#include <inttypes.h>
#include "rect.hpp"

#include "bmpcache.hpp"

enum {
    BITMAP_FOUND_IN_CACHE,
    BITMAP_ADDED_TO_CACHE
};


struct BitmapCache : public BmpCache {
    /* client info */
    unsigned small_entries;
    unsigned small_size;
    unsigned medium_entries;
    unsigned medium_size;
    unsigned big_entries;
    unsigned big_size;

    BitmapCache(ClientInfo* client_info) {
        this->small_entries = client_info->cache1_entries;
        this->small_size = client_info->cache1_size;
        this->medium_entries = client_info->cache2_entries;
        this->medium_size = client_info->cache2_size;
        this->big_entries = client_info->cache3_entries;
        this->big_size = client_info->cache3_size;
    }

    ~BitmapCache()
    {
    }

    TODO(" we should pass in src as a bitmap")
    uint32_t add_bitmap(int src_cx, int src_cy, const uint8_t * src_data,
                    const Rect & tile, int src_bpp, BGRPalette & src_palette)
    {
        Bitmap * candidate_bmp = new Bitmap(src_bpp, &src_palette, tile, src_cx, src_cy, src_data);

        unsigned id = 0;
        unsigned entries = 0;
        if (candidate_bmp->bmp_size(src_bpp) <= this->small_size) {
            entries = this->small_entries;
            id = 0;
        } else if (candidate_bmp->bmp_size(src_bpp) <= this->medium_size) {
            entries = this->medium_entries;
            id = 1;
        } else if (candidate_bmp->bmp_size(src_bpp) <= this->big_size) {
            entries = this->big_entries;
            id = 2;
        }
        else {
            LOG(LOG_ERR, "bitmap size too big %d", candidate_bmp->bmp_size(src_bpp));
            throw Error(ERR_BITMAP_CACHE_TOO_BIG);
        }

        unsigned oldest_idx = 0;
        unsigned oldest = 0x7fffffff;
        for (unsigned idx = 0; idx < entries; idx++) {
            unsigned stamp = this->get_stamp(id, idx);
            if (stamp < oldest) {
                // Keep oldest
                oldest = stamp;
                oldest_idx = idx;
            }
            Bitmap * pbmp = this->get(id, idx);

            if (pbmp){
                if (pbmp->cx == candidate_bmp->cx
                && pbmp->cy == candidate_bmp->cy
                && pbmp->get_crc() == candidate_bmp->get_crc())
                {
                    delete candidate_bmp;
                    this->restamp(id, idx); // refresh stamp
                    return (BITMAP_FOUND_IN_CACHE << 24)|(id << 16)|idx;
                }
            }
        }
        this->put(id, oldest_idx, candidate_bmp);
        return (BITMAP_ADDED_TO_CACHE<<24)|(id << 16)|oldest_idx;
    }
};

#endif
