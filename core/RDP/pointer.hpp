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

#ifndef _REDEMPTION_CORE_RDP_POINTER_HPP_
#define _REDEMPTION_CORE_RDP_POINTER_HPP_

#include <math.h>

#include "drawable.hpp"
#include "client_info.hpp"

struct Pointer {
    enum {
        POINTER_NULL,
        POINTER_CURSOR0,
        POINTER_CURSOR1
    };

    // Bitmap sizes (in bytes)
    enum { DATA_SIZE = 32 * 32 * 4 // maxHeight x maxWidth x bpp = 32 pixel x 32 pixel x 32 bits
         , MASK_SIZE = 32 * 32 / 8 // maxHeight x maxWidth x bpp = 32 pixel x 32 pixel x  1 bit
    };
    enum { 
            MAX_WIDTH = 32
          , MAX_HEIGHT = 32
    };

    unsigned bpp;
    unsigned width;
    unsigned height;
    int x; /* hotspot */
    int y;

    uint8_t data[DATA_SIZE];
    uint8_t mask[MASK_SIZE];

    Pointer(uint8_t pointer_type = POINTER_NULL) {
        switch (pointer_type) {
            default:
            case POINTER_NULL:
            {
                this->x = 0;
                this->y = 0;
                this->width = 32;
                this->height = 32;
                this->bpp = 24;
                memset(this->data, 0, DATA_SIZE);
                memset(this->mask, 0, MASK_SIZE);
            }
            break;

            case POINTER_CURSOR0:
            {
                this->x = 0; /* hotspot */
                this->y = 0;
                this->width = 32;
                this->height = 32;
                this->bpp = 24;
                const char * data_cursor0 =
                /* 0000 */ "................................"
                /* 0060 */ "................................"
                /* 00c0 */ "................................"
                /* 0120 */ "................................"
                /* 0180 */ "................................"
                /* 01e0 */ "................................"
                /* 0240 */ "................................"
                /* 02a0 */ "................................"
                /* 0300 */ "................................"
                /* 0360 */ "................................"
                /* 03c0 */ "................................"
                /* 0420 */ "................................"
                /* 0480 */ "................................"
                /* 04e0 */ ".......XX......................."
                /* 0540 */ "......X..X......................"
                /* 05a0 */ "......X..X......................"
                /* 0600 */ ".....X..X......................."
                /* 0660 */ "X....X..X......................."
                /* 06c0 */ "XX..X..X........................"
                /* 0720 */ "X.X.X..X........................"
                /* 0780 */ "X..X..X........................."
                /* 07e0 */ "X.....XXXXX....................."
                /* 0840 */ "X........X......................"
                /* 08a0 */ "X.......X......................."
                /* 0900 */ "X......X........................"
                /* 0960 */ "X.....X........................."
                /* 09c0 */ "X....X.........................."
                /* 0a20 */ "X...X..........................."
                /* 0a80 */ "X..X............................"
                /* 0ae0 */ "X.X............................."
                /* 0b40 */ "XX.............................."
                /* 0ba0 */ "X..............................."
                ;
                uint8_t * tmp = this->data;
                for (size_t i = 0 ; i < this->width * this->height ; i++) {
                    uint8_t v = (data_cursor0[i] == 'X') ? 0xFF : 0;
                    tmp[0] = tmp[1] = tmp[2] = v;
                    tmp += 3;
                }

                const char * mask_cursor0 =
                /* 0000 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0008 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0010 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0018 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0020 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0028 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0030 */ "\xff\xff\xff\xff"
                           "\xfe\x7f\xff\xff"
                /* 0038 */ "\xfc\x3f\xff\xff"
                           "\xfc\x3f\xff\xff"
                /* 0040 */ "\xf8\x7f\xff\xff"
                           "\x78\x7f\xff\xff"
                /* 0048 */ "\x30\xff\xff\xff"
                           "\x10\xff\xff\xff"
                /* 0050 */ "\x01\xff\xff\xff"
                           "\x00\x1f\xff\xff"
                /* 0058 */ "\x00\x3f\xff\xff"
                           "\x00\x7f\xff\xff"
                /* 0060 */ "\x00\xff\xff\xff"
                           "\x01\xff\xff\xff"
                /* 0068 */ "\x03\xff\xff\xff"
                           "\x07\xff\xff\xff"
                /* 0070 */ "\x0f\xff\xff\xff"
                           "\x1f\xff\xff\xff"
                /* 0078 */ "\x3f\xff\xff\xff"
                           "\x7f\xff\xff\xff"
                ;
                memcpy(this->mask, mask_cursor0, this->width * this->height / 8);
            }
            break;  // case POINTER_CURSOR0:

            case POINTER_CURSOR1:
            {
                this->x = 15; /* hotspot */
                this->y = 16;
                this->width = 32;
                this->height = 32;
                this->bpp = 24;
                const char * data_cursor1 =
                /* 0000 */ "................................"
                /* 0060 */ "................................"
                /* 00c0 */ "................................"
                /* 0120 */ "................................"
                /* 0180 */ "................................"
                /* 01e0 */ "................................"
                /* 0240 */ "................................"
                /* 02a0 */ "...........XXXX.XXXX............"
                /* 0300 */ "...........X...X...X............"
                /* 0360 */ "...........XXXX.XXXX............"
                /* 03c0 */ "..............X.X..............."
                /* 0420 */ "..............X.X..............."
                /* 0480 */ "..............X.X..............."
                /* 04e0 */ "..............X.X..............."
                /* 0540 */ "..............X.X..............."
                /* 05a0 */ "..............X.X..............."
                /* 0600 */ "..............X.X..............."
                /* 0660 */ "..............X.X..............."
                /* 06c0 */ "..............X.X..............."
                /* 0720 */ "..............X.X..............."
                /* 0780 */ "..............X.X..............."
                /* 07e0 */ "..............X.X..............."
                /* 0840 */ "...........XXXX.XXXX............"
                /* 08a0 */ "...........X...X...X............"
                /* 0900 */ "...........XXXX.XXXX............"
                /* 0960 */ "................................"
                /* 09c0 */ "................................"
                /* 0a20 */ "................................"
                /* 0a80 */ "................................"
                /* 0ae0 */ "................................"
                /* 0b40 */ "................................"
                /* 0ba0 */ "................................"
                ;
                uint8_t * tmp = this->data;
                for (size_t i = 0 ; i < this->width * this->height ; i++) {
                    uint8_t v = (data_cursor1[i] == 'X') ? 0xFF : 0;
                    tmp[0] = tmp[1] = tmp[2] = v;
                    tmp += 3;
                }
                const char * mask_cursor1 =
                /* 0000 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0008 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0010 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0018 */ "\xff\xff\xff\xff"
                           "\xff\xe1\x0f\xff"
                /* 0020 */ "\xff\xe0\x0f\xff"
                           "\xff\xe0\x0f\xff"
                /* 0028 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0030 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0038 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0040 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0048 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0050 */ "\xff\xfc\x7f\xff"
                           "\xff\xfc\x7f\xff"
                /* 0058 */ "\xff\xe0\x0f\xff"
                           "\xff\xe0\x0f\xff"
                /* 0060 */ "\xff\xe1\x0f\xff"
                           "\xff\xff\xff\xff"
                /* 0068 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0070 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                /* 0078 */ "\xff\xff\xff\xff"
                           "\xff\xff\xff\xff"
                ;
                memcpy(this->mask, mask_cursor1, this->width * this->height / 8);
            }
            break;  // case POINTER_CURSOR1:
        }   // switch (pointer_type)
    }   // Pointer(uint8_t pointer_type)

    unsigned data_size() const
    {
        switch (this->bpp){
        case 1:
            return (this->width * this->height) / 8;
        case 4:
            return (this->width * this->height) / 2;
        default:
            return (this->width * this->height) * nbbytes(this->bpp);
        }
    }

    unsigned mask_size() const
    {
        return (this->width * this->height) / 8;
    }

    ~Pointer() {}
};

#endif
