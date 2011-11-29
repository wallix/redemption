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
class Bitmap;

class BmpCache {
    Bitmap * cache[3][8192];
    public:
        BmpCache(){
            for (size_t cid = 0; cid < 3 ; cid++){
                for (size_t cidx = 0; cidx < 8192 ; cidx++){
                    cache[cid][cidx] = NULL;
                }
            }
        }
        ~BmpCache(){
            for (uint8_t cid = 0; cid < 3; cid++){
                for (uint16_t cidx = 0 ; cidx < 8192; cidx++){
                    if (cache[cid][cidx]){
                        delete cache[cid][cidx];
                        cache[cid][cidx] = NULL;
                    }
                }
            }
        }
        void put(uint8_t id, uint16_t idx, Bitmap * const bmp){
            if (cache[id][idx]){
                delete cache[id][idx];
            }
            cache[id][idx] = bmp;
        }
        Bitmap * get(uint8_t id, uint16_t idx){
            return cache[id][idx];
        }
};
#endif
