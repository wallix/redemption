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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Dominique Lafages

   rdp module mouse cursor definition object

*/

#ifndef _REDEMPTION_MOD_RDP_RDP_CURSOR_HPP_
#define _REDEMPTION_MOD_RDP_RDP_CURSOR_HPP__

#include <string.h>

TODO("CGR: looks like it's a duplicate object of rdp_pointer : unify")

// Bitmap sizes (in bytes)
enum { DATA_BITMAP_SIZE = 4096 // maxHeight x maxWidth x bpp = 32 pixel x 32 pixel x 32 bits
     , MASK_BITMAP_SIZE =  128 // maxHeight x maxWidth x bpp = 32 pixel x 32 pixel x  1 bit
};


struct rdp_cursor {
    int x;
    int y;
    int width;
    int height;
    uint8_t data[DATA_BITMAP_SIZE];
    uint8_t mask[MASK_BITMAP_SIZE];
    rdp_cursor() {
        this->x = 0;
        this->y = 0;
        this->width = 0;
        this->height = 0;
        memset(this->data, 0, DATA_BITMAP_SIZE);
        memset(this->mask, 0, MASK_BITMAP_SIZE);
    }
};


#endif

