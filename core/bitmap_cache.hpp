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

enum {
    BITMAP_FOUND_IN_CACHE,
    BITMAP_ADDED_TO_CACHE
};


struct BitmapCacheItem {
    int stamp;
    #warning crc is a bitmap property, should not be here
    unsigned crc;
    Bitmap * pbmp;

    BitmapCacheItem() : pbmp(0) {
        this->stamp = 0;
        this->crc = 0;
    };

    BitmapCacheItem(int bpp, Bitmap * pbmp) : pbmp(pbmp) {
        this->stamp = 0;
        this->crc = pbmp->get_crc(bpp);
    };

    ~BitmapCacheItem(){
    }
};

struct BitmapCache {
    /* client info */
    unsigned small_entries;
    unsigned small_size;
    unsigned medium_entries;
    unsigned medium_size;
    unsigned big_entries;
    unsigned big_size;

    /* bitmap */
    int bitmap_stamp;
    BitmapCacheItem * small_bitmaps;
    BitmapCacheItem * medium_bitmaps;
    BitmapCacheItem * big_bitmaps;

    BitmapCache(ClientInfo* client_info) {
        this->small_entries = client_info->cache1_entries;
        this->small_size = client_info->cache1_size;
        this->medium_entries = client_info->cache2_entries;
        this->medium_size = client_info->cache2_size;
        this->big_entries = client_info->cache3_entries;
        this->big_size = client_info->cache3_size;

        this->small_bitmaps = new BitmapCacheItem[client_info->cache1_entries];
        this->medium_bitmaps = new BitmapCacheItem[client_info->cache2_entries];
        this->big_bitmaps = new BitmapCacheItem[client_info->cache3_entries];

        this->bitmap_stamp = 0;
    }

    ~BitmapCache()
    {
        for (size_t i = 0 ; i < this->small_entries ; i++){
            if (this->small_bitmaps[i].pbmp){
                delete this->small_bitmaps[i].pbmp;
            }
        }
        for (size_t i = 0 ; i < this->medium_entries ; i++){
            if (this->medium_bitmaps[i].pbmp){
                delete this->medium_bitmaps[i].pbmp;
            }
        }
        for (size_t i = 0 ; i < this->big_entries ; i++){
            if (this->big_bitmaps[i].pbmp){
                delete this->big_bitmaps[i].pbmp;
            }
        }
        delete [] this->small_bitmaps;
        delete [] this->medium_bitmaps;
        delete [] this->big_bitmaps;
    }

    BitmapCacheItem * get_item(const uint8_t cache_id, const uint16_t cache_idx) const
    {
        BitmapCacheItem * item = 0;
        switch (cache_id){
            case 0:
                if (cache_idx < this->small_entries){
                    item = &(this->small_bitmaps[cache_idx]);
                }
            break;
            case 1:
                if (cache_idx < this->medium_entries){
                    item = &(this->medium_bitmaps[cache_idx]);
                }
            break;
            case 2:
                if (cache_idx < this->big_entries){
                    item = &(this->big_bitmaps[cache_idx]);
                }
            break;
        }

        assert(item && item->stamp && item->pbmp);

        return item;
    }

    /* returns cache id, cx, cy, bpp, data_co */
    #warning we should pass in src as a bitmap
    uint32_t add_bitmap(int src_cx, int src_cy, const uint8_t * src_data,
                    const Rect & tile, int src_bpp)
    {
        int cache_idx = 0;
        Bitmap * pbitmap = new Bitmap(src_bpp, tile, src_cx, src_cy, src_data);
        BitmapCacheItem cache_item(src_bpp, pbitmap);
        this->bitmap_stamp++;
        int entries = 0;
        BitmapCacheItem * array = 0;
        int cache_id = 0;

        if (cache_item.pbmp->bmp_size(src_bpp) <= this->small_size) {
            array = this->small_bitmaps;
            entries = this->small_entries;
            cache_id = 0;
        } else if (cache_item.pbmp->bmp_size(src_bpp) <= this->medium_size) {
            array = this->medium_bitmaps;
            entries = this->medium_entries;
            cache_id = 1;
        } else if (cache_item.pbmp->bmp_size(src_bpp) <= this->big_size) {
            array = this->big_bitmaps;
            entries = this->big_entries;
            cache_id = 2;
        }
        else {
            LOG(LOG_ERR, "bitmap size too big %d", cache_item.pbmp->bmp_size(src_bpp));
            assert(false);
        }

        if (array){
            cache_item.stamp = this->bitmap_stamp;
            cache_idx = 0;
            int oldest = 0x7fffffff;
            for (int j = 0; j < entries; j++) {
                if (array[j].stamp < oldest) {
                    // Keep oldest
                    oldest = array[j].stamp;
                    cache_idx = j;
                }

                #warning create a comparizon function in bitmap_cache_item
                if (array[j].pbmp
                && array[j].pbmp->cx == cache_item.pbmp->cx
                && array[j].pbmp->cy == cache_item.pbmp->cy
                && array[j].crc == cache_item.crc)
                {
                    delete pbitmap;
                    array[j].stamp = this->bitmap_stamp;
                    return (BITMAP_FOUND_IN_CACHE << 24)|(cache_id << 16)|j;
                }
            }

            // cache_idx contains oldest
            if (array[cache_idx].pbmp){
                delete array[cache_idx].pbmp;
            }

            array[cache_idx] = cache_item;
            return (BITMAP_ADDED_TO_CACHE<<24)|(cache_id << 16)|cache_idx;
        }
        else {
            #warning bitmap should not be sent through cache if it is too big, should allready have been splitted by sender ?
            LOG(LOG_ERR, "bitmap not added to cache, too big(%d = %d x %d x %d) [%d, %d, %d]\n", cache_item.pbmp->bmp_size(src_bpp), tile.cx, tile.cy, src_bpp, this->small_size, this->medium_size, this->big_size);
        }
        throw Error(ERR_BITMAP_CACHE_TOO_BIG);
    }
};

#endif
