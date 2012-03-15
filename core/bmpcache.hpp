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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__BMPCACHE_HPP__)
#define __BMPCACHE_HPP__

enum {
    BITMAP_FOUND_IN_CACHE,
    BITMAP_ADDED_TO_CACHE
};

class Bitmap;

struct BmpCache {

    const uint8_t bpp;
    uint32_t small_entries;
    uint32_t small_size;
    uint32_t medium_entries;
    uint32_t medium_size;
    uint32_t big_entries;
    uint32_t big_size;

    const Bitmap * cache[3][8192];
    uint32_t stamps[3][8192];
    //uint32_t crc[3][8192];
    uint8_t sha1[3][8192][20];
    uint32_t stamp;
    public:
        BmpCache(const uint8_t bpp,
                 uint32_t small_entries = 8192, uint32_t small_size = 768,
                 uint32_t medium_entries = 8192, uint32_t medium_size = 3072,
                 uint32_t big_entries = 8192, uint32_t big_size = 12288)
            : bpp(bpp)
            , small_entries(small_entries)
            , small_size(small_size)
            , medium_entries(medium_entries)
            , medium_size(medium_size)
            , big_entries(big_entries)
            , big_size(big_size)
        {
            this->stamp = 0;
            for (size_t cid = 0; cid < 3 ; cid++){
                for (size_t cidx = 0; cidx < 8192 ; cidx++){
                    this->cache[cid][cidx] = NULL;
                    this->stamps[cid][cidx] = 0;
                    //this->crc[cid][cidx] = 0;
                    bzero(this->sha1[cid][cidx], 20);
                }
            }
        }
        ~BmpCache(){
            for (uint8_t cid = 0; cid < 3; cid++){
                for (uint16_t cidx = 0 ; cidx < 8192; cidx++){
                    delete this->cache[cid][cidx];
                }
            }
        }

        void put(uint8_t id, uint16_t idx, const Bitmap * const bmp){
            delete this->cache[id][idx];
            this->cache[id][idx] = bmp;
            this->stamps[id][idx] = ++stamp;
            //this->crc[id][idx] = bmp->compute_crc();
            bmp->compute_sha1(this->sha1[id][idx]);
        }

        void restamp(uint8_t id, uint16_t idx){
            this->stamps[id][idx] = ++stamp;
        }

        const Bitmap * get(uint8_t id, uint16_t idx){
            return this->cache[id][idx];
        }

        unsigned get_stamp(uint8_t id, uint16_t idx){
            return this->stamps[id][idx];
        }


        TODO("palette to use for conversion when we are in 8 bits mode should be passed from memblt.cache_id, not stored in bitmap")
        uint32_t cache_bitmap(const Bitmap & oldbmp){
            uint8_t outbuf[65536];
            oldbmp.convert_data_bitmap(this->bpp, outbuf);
            const Bitmap * bmp = new Bitmap(this->bpp, &oldbmp.original_palette,
                    oldbmp.cx, oldbmp.cy, outbuf,
                    oldbmp.cx * nbbytes(this->bpp) * oldbmp.cy, false, false);

            //const unsigned bmp_crc = bmp->compute_crc();
            uint8_t bmp_sha1[20];
            bmp->compute_sha1(bmp_sha1);

            uint16_t oldest_cidx = 0;
            unsigned oldstamp = this->stamps[0][0];

            uint16_t entries = 0;
            uint8_t id = 0;
            uint32_t bmp_size = bmp->bmp_size;

            if (bmp_size <= this->small_size) {
                entries = this->small_entries;
                id = 0;
            } else if (bmp_size <= this->medium_size) {
                entries = this->medium_entries;
                id = 1;
            } else if (bmp_size <= this->big_size) {
                entries = this->big_entries;
                id = 2;
            }
            else {
                LOG(LOG_ERR, "bitmap size too big %d", bmp_size);
                throw Error(ERR_BITMAP_CACHE_TOO_BIG);
            }

            for (uint16_t cidx = 0 ; cidx < entries; cidx++){
                if (/*bmp_crc == this->crc[id][cidx]
                && */0 == memcmp(bmp_sha1, this->sha1[id][cidx], sizeof(bmp_sha1))
                && this->cache[id][cidx]->cx == bmp->cx
                && this->cache[id][cidx]->cy == bmp->cy){
                    delete bmp;
                    return (BITMAP_FOUND_IN_CACHE << 24)|(id<<16)|cidx;
                }
                if (this->stamps[id][cidx] < oldstamp){
                    oldest_cidx = cidx;
                    oldstamp = this->stamps[id][cidx];
                }
            }
            // find oldest stamp (or 0) and replace bitmap
            delete this->cache[id][oldest_cidx];
            this->cache[id][oldest_cidx] = bmp;
            this->stamps[id][oldest_cidx] = ++stamp;
            //this->crc[id][oldest_cidx] = bmp_crc;
            bmp->compute_sha1(this->sha1[id][oldest_cidx]);
            return (BITMAP_ADDED_TO_CACHE << 24)|(id<<16)|oldest_cidx;
        }
};
#endif
