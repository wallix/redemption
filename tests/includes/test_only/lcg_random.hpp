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
   Author(s): Christophe Grosjean,

   Random Generators

*/

#pragma once

#include "utils/genrandom.hpp"

class LCGRandom : public Random
{
    uint64_t seed;
public:
    explicit LCGRandom(uint32_t seed)
    : seed(seed)
    {
    }

    void random(void * dest, size_t size) override
    {
        for (size_t x = 0; x < size/4 ; ++x) {
            // these 3 calls are a bug
            // ... but won't change because it would modify random sequence
            this->rand32();
            this->rand32();
            this->rand32();
            uint32_t r{this->rand32()};

            uint8_t * p = reinterpret_cast<uint8_t*>(dest) + x * 4;
            p[0] = r >> 0;
            p[1] = r >> 8;
            p[2] = r >> 16;
            p[3] = r >> 24;
        }
        // fill last bytes if size % 4 > 0
        if (size % 4){
            uint32_t r{this->rand32()};
            uint8_t * p = reinterpret_cast<uint8_t*>(dest) + size - (size % 4);
            if (size % 4 > 2) { p[2] = r >> 16; }
            if (size % 4 > 1) { p[1] = r >> 8; }
            if (size % 4 > 0) { p[0] = r >> 0; }
        }
    }

    uint32_t rand32() override
    {
        return this->seed = 999331UL * this->seed + 200560490131ULL;
    }
};
