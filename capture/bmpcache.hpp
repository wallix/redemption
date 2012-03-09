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

class BmpCache {
    Bitmap * cache[3][8192];
    unsigned stamps[3][8192];
    unsigned crc[3][8192];
    unsigned stamp;
    public:
        TODO("merge this with BitmapCache")
        BmpCache(){
            this->stamp = 0;
            for (size_t cid = 0; cid < 3 ; cid++){
                for (size_t cidx = 0; cidx < 8192 ; cidx++){
                    this->cache[cid][cidx] = NULL;
                    this->stamps[cid][cidx] = 0;
                    this->crc[cid][cidx] = 0;
                }
            }
        }
        ~BmpCache(){
            for (uint8_t cid = 0; cid < 3; cid++){
                for (uint16_t cidx = 0 ; cidx < 8192; cidx++){
                    if (this->cache[cid][cidx]){
                        delete this->cache[cid][cidx];
                    }
                }
            }
        }

        void put(uint8_t id, uint16_t idx, Bitmap * const bmp){
            if (this->cache[id][idx]){
                delete this->cache[id][idx];
            }
            this->cache[id][idx] = bmp;
            this->stamps[id][idx] = ++stamp;
            this->crc[id][idx] = bmp->get_crc();
        }

        void restamp(uint8_t id, uint16_t idx){
            this->stamps[id][idx] = ++stamp;
        }

        Bitmap * get(uint8_t id, uint16_t idx){
            return this->cache[id][idx];
        }

        unsigned get_stamp(uint8_t id, uint16_t idx){
            return this->stamps[id][idx];
        }


        uint32_t get_by_crc(Bitmap * const bmp){
            const unsigned bmp_crc = bmp->get_crc();
            uint8_t oldest_cid = 0;
            uint16_t oldest_cidx = 0;
            unsigned oldstamp = this->stamps[0][0];
            for (uint8_t cid = 0; cid < 3; cid++){
                for (uint16_t cidx = 0 ; cidx < 8192; cidx++){
                    if (bmp_crc == this->crc[cid][cidx]){
                        return (BITMAP_FOUND_IN_CACHE << 24)|(cid<<16)|cidx;
                    }
                    if (this->stamps[cid][cidx] < oldstamp){
                        oldest_cid = cid;
                        oldest_cidx = cidx;
                        oldstamp = this->stamps[cid][cidx];
                    }
                }
            }
            // find oldest stamp (or 0) and replace bitmap
            this->put(oldest_cid, oldest_cidx, bmp);
            return (BITMAP_ADDED_TO_CACHE << 24)|(oldest_cid<<16)|oldest_cidx;
        }


};
#endif
