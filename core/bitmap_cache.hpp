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
//#include "NewRDPOrders.hpp"

enum {
    BITMAP_FOUND_IN_CACHE,
    BITMAP_ADDED_TO_CACHE
};


struct BitmapCacheItem {
    int stamp;
    #warning crc is a bitmap property, should not be here
    unsigned crc;
    Bitmap bmp;

    BitmapCacheItem() : bmp() {
        this->stamp = 0;
        this->crc = 0;
    };

    BitmapCacheItem(int src_cx, int src_cy, const uint8_t * src_data, int x, int y, int cx, int cy, int bpp)
    : bmp(bpp, Rect(x, y, cx, cy), src_cx, src_cy, src_data)
    {
        this->stamp = 0;
        this->crc = this->bmp.get_crc();
    }

    ~BitmapCacheItem(){
        #warning allocating and desallocation bitmaps instead of their data parts would be more natural and less error prone
        if (this->bmp.data_co){
            free((uint8_t*)this->bmp.data_co);
            this->bmp.data_co = 0;
        }
    }
};

struct BitmapCache {
    /* client info */
    int use_bitmap_comp;
    int bitmap_cache_persist_enable;
    int bitmap_cache_version;

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

        this->use_bitmap_comp = client_info->use_bitmap_comp;
        this->bitmap_cache_persist_enable = client_info->bitmap_cache_persist_enable;
        this->bitmap_cache_version = client_info->bitmap_cache_version;

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
        delete [] this->small_bitmaps;
        delete [] this->medium_bitmaps;
        delete [] this->big_bitmaps;
    }

    BitmapCacheItem * get_item(unsigned cache_id, unsigned cache_idx)
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

        assert(item && item->stamp && item->bmp.data_co);

        return item;
    }

    /* returns cache id, cx, cy, bpp, data_co */
    #warning we should use some kind of cache item object at least for passing result
    int add_bitmap(int src_cx, int src_cy, const uint8_t * src_data,
                    int x, int y, int w, int h,
                    int src_bpp,
                    uint8_t & cacheid, uint16_t & cacheidx)
    {
        int cache_idx = 0;
        BitmapCacheItem cache_item(src_cx, src_cy, src_data, x, y, w, h, src_bpp);
        this->bitmap_stamp++;
        int entries = 0;
        BitmapCacheItem * array = 0;
        int cache_id = 0;

        if (cache_item.bmp.bmp_size <= this->small_size) {
            array = this->small_bitmaps;
            entries = this->small_entries;
            cache_id = 0;
        } else if (cache_item.bmp.bmp_size <= this->medium_size) {
            array = this->medium_bitmaps;
            entries = this->medium_entries;
            cache_id = 1;
        } else if (cache_item.bmp.bmp_size <= this->big_size) {
            array = this->big_bitmaps;
            entries = this->big_entries;
            cache_id = 2;
        }
        else {
            LOG(LOG_ERR, "bitmap size too big %d", cache_item.bmp.bmp_size);
            assert(false);
        }

        if (array){
            cache_item.stamp = this->bitmap_stamp;
            /* look for oldest */
            cache_idx = 0;
            int oldest = 0x7fffffff;
            for (int j = 0; j < entries; j++) {
                // look for the oldest
                if (array[j].stamp < oldest) {
                    oldest = array[j].stamp;
                    cache_idx = j;
                }
                // look if cache item found
                if (array[j].bmp.data_co
                && array[j].bmp.bpp == cache_item.bmp.bpp
                && array[j].bmp.cx == cache_item.bmp.cx
                && array[j].bmp.cy == cache_item.bmp.cy
                && array[j].crc == cache_item.crc)
                {
                    array[j].stamp = this->bitmap_stamp;

                    // returned value
                    cacheid = cache_id;
                    cacheidx = j;

                    if (cache_item.bmp.data_co){
                        free(cache_item.bmp.data_co);
                    }
                    #warning this one is tricky, we always must set data_co to 0 or it will be desallocated when cache_item goes out of scope... see warning below as a way to avoid this kind of unwanted effects
                    #warning allocating and desallocation bitmaps instead of their data parts would be more natural and less error prone
                    cache_item.bmp.data_co  = 0;
                    return BITMAP_FOUND_IN_CACHE;
                }
            }

            // cache_idx contains oldest
            if (array[cache_idx].bmp.data_co){
                #warning allocating and desallocation bitmaps instead of their data parts would be more natural and less error prone                free(array[cache_idx].bmp.data_co);
                array[cache_idx].bmp.data_co = 0;
            }

            // returned value
            array[cache_idx].bmp = cache_item.bmp;
            array[cache_idx].crc = cache_item.crc;
            array[cache_idx].stamp = cache_item.stamp;

            assert(cache_item.bmp.data_co == array[cache_idx].bmp.data_co);
            assert(cache_item.bmp.bpp == src_bpp);

            cache_item.bmp.data_co = 0;

            cacheid = cache_id;
            cacheidx = cache_idx;
            return BITMAP_ADDED_TO_CACHE;

        }
        else {
            #warning bitmap should not be sent through cache if it is too big, should allready have been splitted by sender ?
            LOG(LOG_ERR, "bitmap not added to cache, too big(%d = %d x %d x %d) [%d, %d, %d]\n", cache_item.bmp.bmp_size, w, h, cache_item.bmp.bpp, this->small_size, this->medium_size, this->big_size);
        }
        #warning should throw an error
        assert(false);
        return BITMAP_FOUND_IN_CACHE;
    }
};

#endif
