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


#pragma once

#include "utils/log.hpp"
#include "utils/bitfu.hpp"
#include <cstring>
#include <algorithm> // std:min

struct Pointer {

    enum  {
        POINTER_NULL             ,
        POINTER_NORMAL           ,
        POINTER_EDIT             ,
        POINTER_DRAWABLE_DEFAULT ,
        POINTER_SYSTEM_DEFAULT   ,
        POINTER_SIZENESW         ,  // Double-pointed arrow pointing northeast and southwest
        POINTER_SIZENS           ,  // Double-pointed arrow pointing north and south
        POINTER_SIZENWSE         ,  // Double-pointed arrow pointing northwest and southeast
        POINTER_SIZEWE              // Double-pointed arrow pointing west and east
    };

public:
    // Bitmap sizes (in bytes)
    enum {
          DATA_SIZE = 32 * 32 * 4 // maxHeight x maxWidth x bpp = 32 pixel x 32 pixel x 32 bits
        , MASK_SIZE = 32 * 32 / 8 // maxHeight x maxWidth x bpp = 32 pixel x 32 pixel x  1 bit
    };
    enum {
          MAX_WIDTH  = 32
        , MAX_HEIGHT = 32
    };

    unsigned bpp;
    unsigned width;
    unsigned height;
    uint8_t  pointer_type;

    /* hotspot */
    int x;
    int y;

    uint8_t data[DATA_SIZE];
    uint8_t mask[MASK_SIZE];

public:
    explicit Pointer(uint8_t pointer_type = POINTER_NULL)
    : pointer_type(pointer_type)
    {
        switch (pointer_type) {
            default:
            case POINTER_NULL:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 0;
                    this->y      = 0;
                    ::memset(this->data, 0, DATA_SIZE);
                    ::memset(this->mask, 0xFF, MASK_SIZE);
                }
                break;

            case POINTER_NORMAL:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 0; /* hotspot */
                    this->y      = 0;
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
                    ::memcpy(this->mask, mask_cursor0, this->width * this->height / 8);
                }
                break;  // case POINTER_NORMAL:

            case POINTER_EDIT:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 15; /* hotspot */
                    this->y      = 16;
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
                    ::memcpy(this->mask, mask_cursor1, this->width * this->height / 8);
                }
                break;  // case POINTER_EDIT:

            case POINTER_DRAWABLE_DEFAULT:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 0; /* hotspot */
                    this->y      = 0;
                    const char * data_cursor2 =
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
                        /* 04e0 */ "................................"
                        /* 0540 */ "................................"
                        /* 05a0 */ ".X....X..X......................"
                        /* 0600 */ "X.X..X..X......................."
                        /* 0660 */ "X..X.X..X......................."
                        /* 06c0 */ "X...X..X........................"
                        /* 0720 */ "X......XXXXX...................."
                        /* 0780 */ "X..........X...................."
                        /* 07e0 */ "X.........X....................."
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
                        uint8_t v = (data_cursor2[i] == 'X') ? 0 : 0xFF;
                        tmp[0] = tmp[1] = tmp[2] = v;
                        tmp += 3;
                    }

                    const char * mask_cursor2 =
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
                                   "\xff\xff\xff\xff"
                        /* 0038 */ "\xff\xff\xff\xff"
                                   "\xbc\x3f\xff\xff"
                        /* 0040 */ "\x18\x7f\xff\xff"
                                   "\x08\x7f\xff\xff"
                        /* 0048 */ "\x00\xff\xff\xff"
                                   "\x00\x0f\xff\xff"
                        /* 0050 */ "\x00\x0f\xff\xff"
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
                    ::memcpy(this->mask, mask_cursor2, this->width * this->height / 8);
                }
                break;  // case POINTER_DRAWABLE_DEFAULT:

            case POINTER_SYSTEM_DEFAULT:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 10; /* hotspot */
                    this->y      = 10;
                    const char * data_cursor3 =
                        /* 0000 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0060 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 00c0 */ "XXXXXXXXXXXXXXXXXX..XXXXXXXXXXXX"
                        /* 0120 */ "XXXXXXXXXXXXXXXXXX..XXXXXXXXXXXX"
                        /* 0180 */ "XXXXXXXXXXXXXXXXX..XXXXXXXXXXXXX"
                        /* 01e0 */ "XXXXXXXXXXXXXXXXX..XXXXXXXXXXXXX"
                        /* 0240 */ "XXXXXXXXXXXXXXXX..XXXXXXXXXXXXXX"
                        /* 02a0 */ "XXXXXXXXXXX.XXXX..XXXXXXXXXXXXXX"
                        /* 0300 */ "XXXXXXXXXXX..XX..XXXXXXXXXXXXXXX"
                        /* 0360 */ "XXXXXXXXXXX...X..XXXXXXXXXXXXXXX"
                        /* 03c0 */ "XXXXXXXXXXX......XXXXXXXXXXXXXXX"
                        /* 0420 */ "XXXXXXXXXXX.........XXXXXXXXXXXX"
                        /* 0480 */ "XXXXXXXXXXX........XXXXXXXXXXXXX"
                        /* 04e0 */ "XXXXXXXXXXX.......XXXXXXXXXXXXXX"
                        /* 0540 */ "XXXXXXXXXXX......XXXXXXXXXXXXXXX"
                        /* 05a0 */ "XXXXXXXXXXX.....XXXXXXXXXXXXXXXX"
                        /* 0600 */ "XXXXXXXXXXX....XXXXXXXXXXXXXXXXX"
                        /* 0660 */ "XXXXXXXXXXX...XXXXXXXXXXXXXXXXXX"
                        /* 06c0 */ "XXXXXXXXXXX..XXXXXXXXXXXXXXXXXXX"
                        /* 0720 */ "XXXXXXXXXXX.XXXXXXXXXXXXXXXXXXXX"
                        /* 0780 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 07e0 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0840 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 08a0 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0900 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0960 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 09c0 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0a20 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0a80 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0ae0 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0b40 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0ba0 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        ;
                    uint8_t * tmp = this->data;
                    for (size_t i = 0 ; i < this->width * this->height ; i++) {
                        uint8_t v = (data_cursor3[i] == 'X') ? 0 : 0xFF;
                        tmp[0] = tmp[1] = tmp[2] = v;
                        tmp += 3;
                    }

                    const char * mask_cursor3 =
                        /* 0000 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xcf\xff"
                        /* 0008 */ "\xff\xff\x87\xff"
                                   "\xff\xff\x87\xff"
                        /* 0010 */ "\xff\xff\x0f\xff"
                                   "\xff\xdf\x0f\xff"
                        /* 0018 */ "\xff\xce\x1f\xff"
                                   "\xff\xc6\x1f\xff"
                        /* 0020 */ "\xff\xc0\x3f\xff"
                                   "\xff\xc0\x3f\xff"
                        /* 0028 */ "\xff\xc0\x03\xff"
                                   "\xff\xc0\x07\xff"
                        /* 0030 */ "\xff\xc0\x0f\xff"
                                   "\xff\xc0\x1f\xff"
                        /* 0038 */ "\xff\xc0\x3f\xff"
                                   "\xff\xc0\x7f\xff"
                        /* 0040 */ "\xff\xc0\xff\xff"
                                   "\xff\xc1\xff\xff"
                        /* 0048 */ "\xff\xc3\xff\xff"
                                   "\xff\xc7\xff\xff"
                        /* 0050 */ "\xff\xcf\xff\xff"
                                   "\xff\xdf\xff\xff"
                        /* 0058 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0060 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0068 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0070 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0078 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        ;
                    ::memcpy(this->mask, mask_cursor3, this->width * this->height / 8);
                }
                break;  // case POINTER_SYSTEM_DEFAULT:

            case POINTER_SIZENS:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 10; /* hotspot */
                    this->y      = 10;
                    const char * data_cursor4 =
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
                        /* 0420 */ "..........X....................."
                        /* 0480 */ ".........X.X...................."
                        /* 04e0 */ "........X...X..................."
                        /* 0540 */ ".......X.....X.................."
                        /* 05a0 */ "......X.......X................."
                        /* 0600 */ "......XXXX.XXXX................."
                        /* 0660 */ ".........X.X...................."
                        /* 06c0 */ ".........X.X...................."
                        /* 0720 */ ".........X.X...................."
                        /* 0780 */ ".........X.X...................."
                        /* 07e0 */ ".........X.X...................."
                        /* 0840 */ ".........X.X...................."
                        /* 08a0 */ ".........X.X...................."
                        /* 0900 */ ".........X.X...................."
                        /* 0960 */ ".........X.X...................."
                        /* 09c0 */ "......XXXX.XXXX................."
                        /* 0a20 */ "......X.......X................."
                        /* 0a80 */ ".......X.....X.................."
                        /* 0ae0 */ "........X...X..................."
                        /* 0b40 */ ".........X.X...................."
                        /* 0ba0 */ "..........X....................."
                        ;
                    uint8_t * tmp = this->data;
                    for (size_t i = 0 ; i < this->width * this->height ; i++) {
                        uint8_t v = (data_cursor4[i] == 'X') ? 0xFF : 0;
                        tmp[0] = tmp[1] = tmp[2] = v;
                        tmp += 3;
                    }

                    const char * mask_cursor4 =
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
                                   "\xff\xdf\xff\xff"
                        /* 0030 */ "\xff\x8f\xff\xff"
                                   "\xff\x07\xff\xff"
                        /* 0038 */ "\xfe\x03\xff\xff"
                                   "\xfc\x01\xff\xff"
                        /* 0040 */ "\xfc\x01\xff\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0048 */ "\xff\x8f\xff\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0050 */ "\xff\x8f\xff\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0058 */ "\xff\x8f\xff\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0060 */ "\xff\x8f\xff\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0068 */ "\xfc\x01\xff\xff"
                                   "\xfc\x01\xff\xff"
                        /* 0070 */ "\xfe\x03\xff\xff"
                                   "\xff\x07\xff\xff"
                        /* 0078 */ "\xff\x8f\xff\xff"
                                   "\xff\xdf\xff\xff"
                        ;
                    ::memcpy(this->mask, mask_cursor4, this->width * this->height / 8);
                }
                break;  // case POINTER_SIZENS:

            case POINTER_SIZENESW:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 10; /* hotspot */
                    this->y      = 10;
                    const char * data_cursor5 =
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
                        /* 04e0 */ "................................"
                        /* 0540 */ "...XXXXXXX......................"
                        /* 05a0 */ "...X.....X......................"
                        /* 0600 */ "...X....X......................."
                        /* 0660 */ "...X...X........................"
                        /* 06c0 */ "...X..X.X.................. ...."
                        /* 0720 */ "...X.X.X.X......................"
                        /* 0780 */ "...XX...X.X....................."
                        /* 07e0 */ ".........X.X...................."
                        /* 0840 */ "..........X.X...XX.............."
                        /* 08a0 */ "...........X.X.X.X.............."
                        /* 0900 */ "............X.X..X.............."
                        /* 0960 */ ".............X...X.............."
                        /* 09c0 */ "............X....X.............."
                        /* 0a20 */ "...........X.....X.............."
                        /* 0a80 */ "...........XXXXXXX.............."
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    uint8_t * tmp = this->data;
                    for (size_t i = 0 ; i < this->width * this->height ; i++) {
                        uint8_t v = (data_cursor5[i] == 'X') ? 0xFF : 0;
                        tmp[0] = tmp[1] = tmp[2] = v;
                        tmp += 3;
                    }

                    const char * mask_cursor5 =
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
                                   "\xff\xff\xff\xff"
                        /* 0038 */ "\xe0\x3f\xff\xff"
                                   "\xe0\x3f\xff\xff"
                        /* 0040 */ "\xe0\x7f\xff\xff"
                                   "\xe0\xff\xff\xff"
                        /* 0048 */ "\xe0\x7f\xff\xff"
                                   "\xe2\x3f\xff\xff"
                        /* 0050 */ "\xe7\x1f\xff\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0058 */ "\xff\xc7\x3f\xff"
                                   "\xff\xe2\x3f\xff"
                        /* 0060 */ "\xff\xf0\x3f\xff"
                                   "\xff\xf8\x3f\xff"
                        /* 0068 */ "\xff\xf0\x3f\xff"
                                   "\xff\xe0\x3f\xff"
                        /* 0070 */ "\xff\xe0\x3f\xff"
                                   "\xff\xff\xff\xff"
                        /* 0078 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        ;
                    ::memcpy(this->mask, mask_cursor5, this->width * this->height / 8);
                }
                break;  // case POINTER_SIZENESW:

            case POINTER_SIZENWSE:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 10; /* hotspot */
                    this->y      = 10;
                    const char * data_cursor6 =
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
                        /* 04e0 */ "................................"
                        /* 0540 */ "...........XXXXXXX.............."
                        /* 05a0 */ "...........X.....X.............."
                        /* 0600 */ "............X....X.............."
                        /* 0660 */ ".............X...X.............."
                        /* 06c0 */ "............X.X..X.............."
                        /* 0720 */ "...........X.X.X.X.............."
                        /* 0780 */ "..........X.X...XX.............."
                        /* 07e0 */ ".........X.X...................."
                        /* 0840 */ "...XX...X.X....................."
                        /* 08a0 */ "...X.X.X.X......................"
                        /* 0900 */ "...X..X.X......................."
                        /* 0960 */ "...X...X........................"
                        /* 09c0 */ "...X....X......................."
                        /* 0a20 */ "...X.....X......................"
                        /* 0a80 */ "...XXXXXXX......................"
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    uint8_t * tmp = this->data;
                    for (size_t i = 0 ; i < this->width * this->height ; i++) {
                        uint8_t v = (data_cursor6[i] == 'X') ? 0xFF : 0;
                        tmp[0] = tmp[1] = tmp[2] = v;
                        tmp += 3;
                    }

                    const char * mask_cursor6 =
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
                                   "\xff\xff\xff\xff"
                        /* 0038 */ "\xff\xe0\x3f\xff"
                                   "\xff\xe0\x3f\xff"
                        /* 0040 */ "\xff\xf0\x3f\xff"
                                   "\xff\xf8\x3f\xff"
                        /* 0048 */ "\xff\xf0\x3f\xff"
                                   "\xff\xe2\x3f\xff"
                        /* 0050 */ "\xff\xc7\x3f\xff"
                                   "\xff\x8f\xff\xff"
                        /* 0058 */ "\xe7\x1f\xff\xff"
                                   "\xe2\x3f\xff\xff"
                        /* 0060 */ "\xe0\x7f\xff\xff"
                                   "\xe0\xff\xff\xff"
                        /* 0068 */ "\xe0\x7f\xff\xff"
                                   "\xe0\x3f\xff\xff"
                        /* 0070 */ "\xe0\x3f\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0078 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        ;
                    ::memcpy(this->mask, mask_cursor6, this->width * this->height / 8);
                }
                break;  // case POINTER_SIZENWSE:

            case POINTER_SIZEWE:
                {
                    this->bpp    = 24;
                    this->width  = 32;
                    this->height = 32;
                    this->x      = 10; /* hotspot */
                    this->y      = 10;
                    const char * data_cursor7 =
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
                        /* 04e0 */ "................................"
                        /* 0540 */ "................................"
                        /* 05a0 */ "................................"
                        /* 0600 */ "................................"
                        /* 0660 */ "....XX.........XX..............."
                        /* 06c0 */ "...X.X.........X.X.............."
                        /* 0720 */ "..X..X.........X..X............."
                        /* 0780 */ ".X...XXXXXXXXXXX...X............"
                        /* 07e0 */ "X...................X..........."
                        /* 0840 */ ".X...XXXXXXXXXXX...X............"
                        /* 08a0 */ "..X..X.........X..X............."
                        /* 0900 */ "...X.X.........X.X.............."
                        /* 0960 */ "....XX.........XX..............."
                        /* 09c0 */ "................................"
                        /* 0a20 */ "................................"
                        /* 0a80 */ "................................"
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    uint8_t * tmp = this->data;
                    for (size_t i = 0 ; i < this->width * this->height ; i++) {
                        uint8_t v = (data_cursor7[i] == 'X') ? 0xFF : 0;
                        tmp[0] = tmp[1] = tmp[2] = v;
                        tmp += 3;
                    }

                    const char * mask_cursor7 =
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
                                   "\xff\xff\xff\xff"
                        /* 0038 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0040 */ "\xff\xff\xff\xff"
                                   "\xf3\xfe\x7f\xff"
                        /* 0048 */ "\xe3\xfe\x3f\xff"
                                   "\xc3\xfe\x1f\xff"
                        /* 0050 */ "\x80\x00\x0f\xff"
                                   "\x00\x00\x07\xff"
                        /* 0058 */ "\x80\x00\x0f\xff"
                                   "\xc3\xfe\x1f\xff"
                        /* 0060 */ "\xe3\xfe\x3f\xff"
                                   "\xf3\xfe\x7f\xff"
                        /* 0068 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0070 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        /* 0078 */ "\xff\xff\xff\xff"
                                   "\xff\xff\xff\xff"
                        ;
                    ::memcpy(this->mask, mask_cursor7, this->width * this->height / 8);
                }
                break;  // case POINTER_SIZEWE:
        }   // switch (pointer_type)
    }   // Pointer(uint8_t pointer_type)

    ~Pointer() = default;

    void initialize(unsigned bpp, unsigned width, unsigned height, int x, int y, uint8_t * data, size_t data_size,
        uint8_t * mask, size_t mask_size) {
        this->bpp    = bpp;
        this->width  = width;
        this->height = height;
        this->x      = x;
        this->y      = y;

        REDASSERT(data_size == sizeof(this->data));
        ::memcpy(this->data, data, std::min(data_size, sizeof(this->data)));
        REDASSERT(mask_size == sizeof(this->mask));
        ::memcpy(this->mask, mask, std::min(mask_size, sizeof(this->mask)));
    }

    //bool is_same(const Pointer & other) {
    //  return (
    //         (this->bpp    == other.bpp)
    //      && (this->width  == other.width)
    //      && (this->height == other.height)
    //      && (this->x      == other.x)
    //      && (this->y      == other.y)
    //      && (memcmp(this->data, other.data, this->data_size()) == 0)
    //      && (memcmp(this->mask, other.mask, this->mask_size()) == 0)
    //      );
    //}

    unsigned data_size() const {
        switch (this->bpp){
        case 1:
            return (this->width * this->height) / 8;
        case 4:
            return (this->width * this->height) / 2;
        default:
            return (this->width * this->height) * nbbytes(this->bpp);
        }
    }

    unsigned mask_size() const {
        return (this->width * this->height) / 8;
    }

    bool is_valid() const {
        return (this->width && this->height && this->bpp);
    }
};

