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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

*/


#pragma once

#include <string.h>
#include "utils/log.hpp"

struct brush_item {
    int stamp;
    /* expand this to a structure to handle more complicated brushes
       for now its 8x8 1bpp brushes only */
    // TODO use RDPBrush
    uint8_t pattern[8];
    brush_item() {
        this->stamp = 0;
        memset(this->pattern, 0, 8);
    }
};

enum {
    BRUSH_TO_SEND = 0,
    BRUSH_ALLREADY_SENT
};

/* difference caches */
struct BrushCache {
    /* brush */
    int brush_stamp = 0;
    struct brush_item brush_items[64];

    BrushCache() = default;
    ~BrushCache() = default;

    /*****************************************************************************/
    /* this does not take owership of brush_item_data, it makes a copy */
    int add_brush(uint8_t* brush_item_data, int & cache_idx)
    {
        int i;
        int oldest;
        int index;

        this->brush_stamp++;
        /* look for match */
        for (i = 0; i < 64; i++) {
            if (memcmp(this->brush_items[i].pattern, brush_item_data, 8) == 0) {
                this->brush_items[i].stamp = this->brush_stamp;
                cache_idx = i;
                return BRUSH_ALLREADY_SENT;
            }
        }
        /* look for oldest */
        index = 0;
        oldest = 0x7fffffff;
        for (i = 0; i < 64; i++) {
            if (this->brush_items[i].stamp < oldest) {
                oldest = this->brush_items[i].stamp;
                index = i;
            }
        }
        memcpy(this->brush_items[index].pattern, brush_item_data, 8);
        this->brush_items[index].stamp = this->brush_stamp;
        cache_idx = index;
        return BRUSH_TO_SEND;
    }
};


