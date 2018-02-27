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

#include "utils/bitfu.hpp"

#include <algorithm> // std::min
#include <cassert>
#include <cstring>
#include "utils/log.hpp"
#include "utils/hexdump.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/colors.hpp"

// TODO: in TS_SYSTEMPOINTERATTRIBUTE, POINTER_NULL and POINTER_NORMAL are attributed specific values
// we could directly provide these to Pointer constructor instead of defining a switch on call site (rdp.hpp)

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
        POINTER_SIZEWE           ,  // Double-pointed arrow pointing west and east

        POINTER_CUSTOM
    };

public:
    // Bitmap sizes (in bytes)
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
        , MAX_BPP    = 32
    };
    enum {
          DATA_SIZE = MAX_WIDTH * MAX_HEIGHT * MAX_BPP / 8
        , MASK_SIZE = MAX_WIDTH * MAX_HEIGHT * 1 / 8
    };

   

public:
    struct CursorSize {
        unsigned width;
        unsigned height;
        CursorSize(int w, int h) : width(w), height(h) {}
        CursorSize(const CursorSize & cs) : width(cs.width), height(cs.height) {}
    };

    struct Hotspot {
        unsigned x;
        unsigned y;
        Hotspot(int x, int y) : x(x), y(y) {}
        Hotspot(const Hotspot & hs) : x(hs.x), y(hs.y) {}
    };

private:
//    unsigned bpp;
    
    CursorSize dimensions;
    
    Hotspot hotspot;

public:
    uint8_t data[DATA_SIZE];
private:
    uint8_t mask[MASK_SIZE];
public:
    bool only_black_white = false;

    CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    void set_dimensions(const CursorSize & dimensions)
    {
        this->dimensions = dimensions;
    }

    Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    void set_hotspot(const Hotspot & hotspot)
    {
        this->hotspot = hotspot;
    }

    void store_data_cursor(const char * cursor){ 
            uint8_t * tmp = this->data;
            memset(this->mask, 0, this->dimensions.width * this->dimensions.height / 8);
            for (size_t i = 0 ; i < this->dimensions.width * this->dimensions.height ; i++) {
                uint8_t v = (cursor[i] == 'X') ? 0xFF : 0;
                tmp[0] = tmp[1] = tmp[2] = v;
                tmp += 3;
                this->mask[i/8]|= (cursor[i] == '.')?(0x80 >> (i%8)):0;
            }
    }

public:
    explicit Pointer(CursorSize d, Hotspot hs, array_view_const_u8 & av_xor, array_view_const_u8 & av_and)
        : dimensions(d)
        , hotspot(hs)
    {
        memcpy(this->mask, av_and.data(), av_and.size());
        memcpy(this->data, av_xor.data(), av_xor.size());

        if ((av_xor.size() > this->bit_mask_size()) || (av_and.size() > sizeof(this->xor_mask_size()))) {
            LOG(LOG_ERR, "mod_rdp::process_color_pointer_pdu: "
                "bad length for color pointer mask_len=%zu data_len=%zu",
                av_and.size(), av_and.size());
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }

    }

    explicit Pointer(const Pointer & other)
    : dimensions(other.dimensions)
    , hotspot(other.hotspot)
    {
        auto & av_and = other.get_monochrome_and_mask();
        auto & av_xor = other.get_24bits_xor_mask();
        memcpy(this->mask, av_and.data(), av_and.size());
        memcpy(this->data, av_xor.data(), av_xor.size());
    }

    bool operator==(const Pointer & other) const {
        return (other.hotspot.x == this->hotspot.x
             && other.hotspot.y == this->hotspot.y
             && other.dimensions.width == this->dimensions.width
             && other.dimensions.height == this->dimensions.height
             && (0 == memcmp(this->data, other.data, other.data_size()))
             && (0 == memcmp(this->mask, other.mask, this->bit_mask_size())));
    }


    explicit Pointer(uint8_t pointer_type = POINTER_NULL)
    :   dimensions{0, 0}
    ,   hotspot{0, 0}
    {
        switch (pointer_type) {
            default:
            case POINTER_NULL:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 0;
                    this->hotspot.y                = 0;
                    this->only_black_white = true;
                    ::memset(this->data, 0, DATA_SIZE);
                    ::memset(this->mask, 0xFF, MASK_SIZE);
                }
                break;

            case POINTER_NORMAL:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 0; /* hotspot */
                    this->hotspot.y                = 0;
                    this->only_black_white = true;
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
                        /* 0540 */ "......X++X......................"
                        /* 05a0 */ "......X++X......................"
                        /* 0600 */ ".....X++X......................."
                        /* 0660 */ "X....X++X......................."
                        /* 06c0 */ "XX..X++X........................"
                        /* 0720 */ "X+X.X++X........................"
                        /* 0780 */ "X++X++X........................."
                        /* 07e0 */ "X+++++XXXXX....................."
                        /* 0840 */ "X++++++++X......................"
                        /* 08a0 */ "X+++++++X......................."
                        /* 0900 */ "X++++++X........................"
                        /* 0960 */ "X+++++X........................."
                        /* 09c0 */ "X++++X.........................."
                        /* 0a20 */ "X+++X..........................."
                        /* 0a80 */ "X++X............................"
                        /* 0ae0 */ "X+X............................."
                        /* 0b40 */ "XX.............................."
                        /* 0ba0 */ "X..............................."
                        ;
                    this->store_data_cursor(data_cursor0);
                }
                break;  // case POINTER_NORMAL:

            case POINTER_EDIT:
                {
//                    this->bpp                = 24;
                    this->dimensions.width              = 32;
                    this->dimensions.height             = 32;
                    this->hotspot.x                  = 15; /* hotspot */
                    this->hotspot.y                  = 16;
                    this->only_black_white = true;
                    const char * data_cursor1 =
                        /* 0000 */ "................................"
                        /* 0060 */ "................................"
                        /* 00c0 */ "................................"
                        /* 0120 */ "................................"
                        /* 0180 */ "................................"
                        /* 01e0 */ "................................"
                        /* 0240 */ "................................"
                        /* 02a0 */ "...........XXXX.XXXX............"
                        /* 0300 */ "...........X+++X+++X............"
                        /* 0360 */ "...........XXXX+XXXX............"
                        /* 03c0 */ "..............X+X..............."
                        /* 0420 */ "..............X+X..............."
                        /* 0480 */ "..............X+X..............."
                        /* 04e0 */ "..............X+X..............."
                        /* 0540 */ "..............X+X..............."
                        /* 05a0 */ "..............X+X..............."
                        /* 0600 */ "..............X+X..............."
                        /* 0660 */ "..............X+X..............."
                        /* 06c0 */ "..............X+X..............."
                        /* 0720 */ "..............X+X..............."
                        /* 0780 */ "..............X+X..............."
                        /* 07e0 */ "..............X+X..............."
                        /* 0840 */ "...........XXXX+XXXX............"
                        /* 08a0 */ "...........X+++X+++X............"
                        /* 0900 */ "...........XXXX.XXXX............"
                        /* 0960 */ "................................"
                        /* 09c0 */ "................................"
                        /* 0a20 */ "................................"
                        /* 0a80 */ "................................"
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    this->store_data_cursor(data_cursor1);
                }
                break;  // case POINTER_EDIT:

            case POINTER_DRAWABLE_DEFAULT:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 0; /* hotspot */
                    this->hotspot.y                = 0;
                    this->only_black_white = true;
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
                        /* 05a0 */ ".X....X++X......................"
                        /* 0600 */ "X+X..X++X......................."
                        /* 0660 */ "X++X.X++X......................."
                        /* 06c0 */ "X+++X++X........................"
                        /* 0720 */ "X++++++XXXXX...................."
                        /* 0780 */ "X++++++++++X...................."
                        /* 07e0 */ "X+++++++++X....................."
                        /* 0840 */ "X++++++++X......................"
                        /* 08a0 */ "X+++++++X......................."
                        /* 0900 */ "X++++++X........................"
                        /* 0960 */ "X+++++X........................."
                        /* 09c0 */ "X++++X.........................."
                        /* 0a20 */ "X+++X..........................."
                        /* 0a80 */ "X++X............................"
                        /* 0ae0 */ "X+X............................."
                        /* 0b40 */ "XX.............................."
                        /* 0ba0 */ "X..............................."
                        ;
                    this->store_data_cursor(data_cursor2);

                }
                break;  // case POINTER_DRAWABLE_DEFAULT:

            case POINTER_SYSTEM_DEFAULT:
                {
//                    this->bpp              = 24;
                    this->dimensions.width = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 10; /* hotspot */
                    this->hotspot.y                = 10;
                    this->only_black_white = true;
                    const char * data_cursor3 =
                        /* 0000 */ "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
                        /* 0060 */ "XXXXXXXXXXXXXXXXXX++XXXXXXXXXXXX"
                        /* 00c0 */ "XXXXXXXXXXXXXXXXX+..+XXXXXXXXXXX"
                        /* 0120 */ "XXXXXXXXXXXXXXXXX+..+XXXXXXXXXXX"
                        /* 0180 */ "XXXXXXXXXXXXXXXX+..+XXXXXXXXXXXX"
                        /* 01e0 */ "XXXXXXXXXX+XXXXX+..+XXXXXXXXXXXX"
                        /* 0240 */ "XXXXXXXXXX++XXX+..+XXXXXXXXXXXXX"
                        /* 02a0 */ "XXXXXXXXXX+.+XX+..+XXXXXXXXXXXXX"
                        /* 0300 */ "XXXXXXXXXX+..++..+XXXXXXXXXXXXXX"
                        /* 0360 */ "XXXXXXXXXX+...+..+XXXXXXXXXXXXXX"
                        /* 03c0 */ "XXXXXXXXXX+......+++++XXXXXXXXXX"
                        /* 0420 */ "XXXXXXXXXX+.........+XXXXXXXXXXX"
                        /* 0480 */ "XXXXXXXXXX+........+XXXXXXXXXXXX"
                        /* 04e0 */ "XXXXXXXXXX+.......+XXXXXXXXXXXXX"
                        /* 0540 */ "XXXXXXXXXX+......+XXXXXXXXXXXXXX"
                        /* 05a0 */ "XXXXXXXXXX+.....+XXXXXXXXXXXXXXX"
                        /* 0600 */ "XXXXXXXXXX+....+XXXXXXXXXXXXXXXX"
                        /* 0660 */ "XXXXXXXXXX+...+XXXXXXXXXXXXXXXXX"
                        /* 06c0 */ "XXXXXXXXXX+..+XXXXXXXXXXXXXXXXXX"
                        /* 0720 */ "XXXXXXXXXX+.+XXXXXXXXXXXXXXXXXXX"
                        /* 0780 */ "XXXXXXXXXX++XXXXXXXXXXXXXXXXXXXX"
                        /* 07e0 */ "XXXXXXXXXX+XXXXXXXXXXXXXXXXXXXXX"
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
                        const char * cursor = data_cursor3;
                        uint8_t * tmp = this->data;
                        memset(this->mask, 0, this->dimensions.width * this->dimensions.height / 8);
                        for (size_t i = 0 ; i < this->dimensions.width * this->dimensions.height ; i++) {
                            uint8_t v = (cursor[i] != '.') ? 0xFF : 0;
                            tmp[0] = tmp[1] = tmp[2] = v;
                            tmp += 3;
                            this->mask[i/8]|= (cursor[i] == 'X')?(0x80 >> (i%8)):0;
                        }
                }
                break;  // case POINTER_SYSTEM_DEFAULT:

            case POINTER_SIZENS:
                {
//                    this->bpp              = 24;
                    this->dimensions.width = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 10; /* hotspot */
                    this->hotspot.y                = 10;
                    this->only_black_white = true;
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
                        /* 0480 */ ".........X+X...................."
                        /* 04e0 */ "........X+++X..................."
                        /* 0540 */ ".......X+++++X.................."
                        /* 05a0 */ "......X+++++++X................."
                        /* 0600 */ "......XXXX+XXXX................."
                        /* 0660 */ ".........X+X...................."
                        /* 06c0 */ ".........X+X...................."
                        /* 0720 */ ".........X+X...................."
                        /* 0780 */ ".........X+X...................."
                        /* 07e0 */ ".........X+X...................."
                        /* 0840 */ ".........X+X...................."
                        /* 08a0 */ ".........X+X...................."
                        /* 0900 */ ".........X+X...................."
                        /* 0960 */ ".........X+X...................."
                        /* 09c0 */ "......XXXX+XXXX................."
                        /* 0a20 */ "......X+++++++X................."
                        /* 0a80 */ ".......X+++++X.................."
                        /* 0ae0 */ "........X+++X..................."
                        /* 0b40 */ ".........X+X...................."
                        /* 0ba0 */ "..........X....................."
                        ;
                    this->store_data_cursor(data_cursor4);
                }
                break;  // case POINTER_SIZENS:

            case POINTER_SIZENESW:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 10; /* hotspot */
                    this->hotspot.y                = 10;
                    this->only_black_white = true;
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
                        /* 05a0 */ "...X+++++X......................"
                        /* 0600 */ "...X++++X......................."
                        /* 0660 */ "...X+++X........................"
                        /* 06c0 */ "...X++X+X......................."
                        /* 0720 */ "...X+X.X+X......................"
                        /* 0780 */ "...XX...X+X....................."
                        /* 07e0 */ ".........X+X...................."
                        /* 0840 */ "..........X+X...XX.............."
                        /* 08a0 */ "...........X+X.X+X.............."
                        /* 0900 */ "............X+X++X.............."
                        /* 0960 */ ".............X+++X.............."
                        /* 09c0 */ "............X++++X.............."
                        /* 0a20 */ "...........X+++++X.............."
                        /* 0a80 */ "...........XXXXXXX.............."
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    this->store_data_cursor(data_cursor5);
                }
                break;  // case POINTER_SIZENESW:

            case POINTER_SIZENWSE:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 10; /* hotspot */
                    this->hotspot.y                = 10;
                    this->only_black_white = true;
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
                        /* 05a0 */ "...........X+++++X.............."
                        /* 0600 */ "............X++++X.............."
                        /* 0660 */ ".............X+++X.............."
                        /* 06c0 */ "............X+X++X.............."
                        /* 0720 */ "...........X+X.X+X.............."
                        /* 0780 */ "..........X+X...XX.............."
                        /* 07e0 */ ".........X+X...................."
                        /* 0840 */ "...XX...X+X....................."
                        /* 08a0 */ "...X+X.X+X......................"
                        /* 0900 */ "...X++X+X......................."
                        /* 0960 */ "...X+++X........................"
                        /* 09c0 */ "...X++++X......................."
                        /* 0a20 */ "...X+++++X......................"
                        /* 0a80 */ "...XXXXXXX......................"
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    this->store_data_cursor(data_cursor6);
                }
                break;  // case POINTER_SIZENWSE:

            case POINTER_SIZEWE:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 10; /* hotspot */
                    this->hotspot.y                = 10;
                    this->only_black_white = true;
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
                        /* 06c0 */ "...X+X.........X+X.............."
                        /* 0720 */ "..X++X.........X++X............."
                        /* 0780 */ ".X+++XXXXXXXXXXX+++X............"
                        /* 07e0 */ "X+++++++++++++++++++X..........."
                        /* 0840 */ ".X+++XXXXXXXXXXX+++X............"
                        /* 08a0 */ "..X++X.........X++X............."
                        /* 0900 */ "...X+X.........X+X.............."
                        /* 0960 */ "....XX.........XX..............."
                        /* 09c0 */ "................................"
                        /* 0a20 */ "................................"
                        /* 0a80 */ "................................"
                        /* 0ae0 */ "................................"
                        /* 0b40 */ "................................"
                        /* 0ba0 */ "................................"
                        ;
                    this->store_data_cursor(data_cursor7);
                }
                break;  // case POINTER_SIZEWE:
        }   // switch (pointer_type)
    }   // Pointer(uint8_t pointer_type)

    void initialize(/*unsigned bpp, */unsigned width, unsigned height, int x, int y, uint8_t * data, size_t data_size,
        uint8_t * mask, size_t mask_size) {
//        this->bpp    = bpp;
        this->dimensions.width  = width;
        this->dimensions.height = height;
        this->hotspot.x      = x;
        this->hotspot.y      = y;

        assert(data_size == sizeof(this->data));
        ::memcpy(this->data, data, std::min(data_size, sizeof(this->data)));
        assert(mask_size == sizeof(this->mask));
        ::memcpy(this->mask, mask, std::min(mask_size, sizeof(this->mask)));
    }

    //bool is_same(const Pointer & other) {
    //  return (
    //         (this->bpp    == other.bpp)
    //      && (this->dimensions.width  == other.dimensions.width)
    //      && (this->dimensions.height == other.dimensions.height)
    //      && (this->hotspot.x      == other.x)
    //      && (this->hotspot.y      == other.y)
    //      && (memcmp(this->data, other.data, this->data_size()) == 0)
    //      && (memcmp(this->mask, other.mask, this->mask_size()) == 0)
    //      );
    //}

    const array_view_const_u8 get_monochrome_and_mask() const
    {
        return {this->mask, this->bit_mask_size()};
    }

    const array_view_const_u8 get_24bits_xor_mask() const
    {
        return {this->data, this->xor_mask_size()};
    }


    unsigned data_size() const {
        const unsigned int xor_line_length_in_byte = this->dimensions.width * 3;
        const unsigned int xor_padded_line_length_in_byte =
            ((xor_line_length_in_byte % 2) ?
             xor_line_length_in_byte + 1 :
             xor_line_length_in_byte);

        return (xor_padded_line_length_in_byte * this->dimensions.height);
    }

    unsigned mask_size() const {
        const unsigned int remainder = (this->dimensions.width % 8);
        const unsigned int and_line_length_in_byte = this->dimensions.width / 8 + (remainder ? 1 : 0);
        const unsigned int and_padded_line_length_in_byte =
            ((and_line_length_in_byte % 2) ?
             and_line_length_in_byte + 1 :
             and_line_length_in_byte);

        return (and_padded_line_length_in_byte * this->dimensions.height);
    }

    unsigned bit_mask_size() const {
        return ::nbbytes(this->dimensions.width) * this->dimensions.height;
    }

    unsigned xor_mask_size() const {
        size_t l_width = (this->dimensions.width * 3);
        return this->dimensions.height * ((l_width&1) ?l_width+1:l_width);
    }

    bool is_valid() const {
        return (this->dimensions.width && this->dimensions.height/* && this->bpp*/);
    }

    void update_bw() {
        const unsigned int xor_line_length_in_byte = this->dimensions.width * 3;
        const unsigned int xor_padded_line_length_in_byte =
            ((xor_line_length_in_byte % 2) ?
             xor_line_length_in_byte + 1 :
             xor_line_length_in_byte);
        for (unsigned int h = 0; h < this->dimensions.height; ++h) {
            const uint8_t* xorMask = this->data + (this->dimensions.height - h - 1) * xor_padded_line_length_in_byte;
            for (unsigned int w = 0; w < this->dimensions.width; ++w) {
                if (((*xorMask) > 0) && ((*xorMask) < 255)) { this->only_black_white = false; return; }
                xorMask++;
                if (((*xorMask) > 0) && ((*xorMask) < 255)) { this->only_black_white = false; return; }
                xorMask++;
                if (((*xorMask) > 0) && ((*xorMask) < 255)) { this->only_black_white = false; return; }
                xorMask++;
            }
        }

        this->only_black_white = true;
    }
    
    void to_regular_pointer(const uint8_t * indata, unsigned dlen, unsigned width, unsigned height, uint8_t bpp, const BGRPalette & global_palette) 
    {
        switch (bpp) {
        case 1 :
        {
            const unsigned int remainder = (width % 8);
            const unsigned int src_xor_line_length_in_byte = width / 8 + (remainder ? 1 : 0);
            const unsigned int src_xor_padded_line_length_in_byte =
                ((src_xor_line_length_in_byte % 2) ?
                 src_xor_line_length_in_byte + 1 :
                 src_xor_line_length_in_byte);

            const unsigned int dest_xor_line_length_in_byte        = width * 3;
            const unsigned int dest_xor_padded_line_length_in_byte =
                dest_xor_line_length_in_byte + ((dest_xor_line_length_in_byte % 2) ? 1 : 0);

            for (unsigned int i = 0; i < height; ++i) {
                const uint8_t* src  = indata + (height - i - 1) * src_xor_padded_line_length_in_byte;
                      uint8_t* dest = this->data + i * dest_xor_padded_line_length_in_byte;

                unsigned char and_bit_extraction_mask = 7;

                for (unsigned int j = 0; j < width; ++j) {
                    ::out_bytes_le(dest, 3, (((*src) & (1 << and_bit_extraction_mask)) ? 0xFFFFFF : 0));

                    dest += 3;

                    if (and_bit_extraction_mask) {
                        and_bit_extraction_mask--;
                    }
                    else {
                        src++;
                        and_bit_extraction_mask = 7;
                    }
                }
            }
        }
        break;
        case 4 :
        {
            for (unsigned i = 0; i < dlen ; i++) {
                const uint8_t px = indata[i];
                // target cursor will receive 8 bits input at once
                ::out_bytes_le(&(this->data[6 * i]),     3, global_palette[(px >> 4) & 0xF].to_u32());
                ::out_bytes_le(&(this->data[6 * i + 3]), 3, global_palette[ px       & 0xF].to_u32());
            }
        }
        break;
        case 32: case 24: case 16: case 15: case 8:
        {
            uint8_t BPP = nbbytes(bpp);

            const unsigned int src_xor_line_length_in_byte = width * BPP;
            const unsigned int src_xor_padded_line_length_in_byte =
                ((src_xor_line_length_in_byte % 2) ?
                 src_xor_line_length_in_byte + 1 :
                 src_xor_line_length_in_byte);

            const unsigned int dest_xor_line_length_in_byte = width * 3;
            const unsigned int dest_xor_padded_line_length_in_byte =
                ((dest_xor_line_length_in_byte % 2) ?
                 dest_xor_line_length_in_byte + 1 :
                 dest_xor_line_length_in_byte);

            for (unsigned int i0 = 0; i0 < height; ++i0) {
                const uint8_t* src  = indata + (height - i0 - 1) * src_xor_padded_line_length_in_byte;
                      uint8_t* dest = this->data + (height - i0 - 1) * dest_xor_padded_line_length_in_byte;

                for (unsigned int i1 = 0; i1 < width; ++i1) {
                    RDPColor px = RDPColor::from(in_uint32_from_nb_bytes_le(BPP, src));
                    src += BPP;
                    ::out_bytes_le(dest, 3, color_decode(px, bpp, global_palette).to_u32());
                    dest += 3;
                }
            }
        }
        break;
        default:
            LOG(LOG_ERR, "Mouse pointer : color depth not supported %d, forcing green mouse (running in the grass ?)", bpp);
            for (size_t x = 0 ; x < 1024 ; x++) {
                ::out_bytes_le(this->data + x *3, 3, GREEN);
            }
            break;
        }
    }

    void to_regular_mask(const uint8_t * indata, unsigned mlen, unsigned width, unsigned height, uint8_t bpp) {
        /* TODO check code below: why do we revert mask and pointer when pointer is 1 BPP
         * and not with other color depth ? Looks fishy, a mask and pointer should always
         * be encoded in the same way, not depending on color depth difficult to see for
         * symmetrical pointers... check documentation it may be more efficient to revert
         * cursor after creating it instead of doing it on the fly */
        switch (bpp) {
        case 1 :
        {
            const unsigned int remainder = (width % 8);
            const unsigned int and_line_length_in_byte = width / 8 + (remainder ? 1 : 0);
            const unsigned int and_padded_line_length_in_byte =
                ((and_line_length_in_byte % 2) ?
                 and_line_length_in_byte + 1 :
                 and_line_length_in_byte);
            for (unsigned int i = 0; i < height; ++i) {
                const uint8_t* src  = indata + (height - i - 1) * and_padded_line_length_in_byte;
                      uint8_t* dest = this->mask + i * and_padded_line_length_in_byte;
                ::memcpy(dest, src, and_padded_line_length_in_byte);
            }
        }
        break;
        default:
            memcpy(this->mask, indata, mlen);
        break;
        }
    }
    
    void cleanup_32_bpp_cursor(unsigned width, unsigned height) {
        const unsigned int xor_line_length_in_byte = width * 3;
        const unsigned int xor_padded_line_length_in_byte =
            ((xor_line_length_in_byte % 2) ?
             xor_line_length_in_byte + 1 :
             xor_line_length_in_byte);
        const unsigned int remainder = (width % 8);
        const unsigned int and_line_length_in_byte = width / 8 + (remainder ? 1 : 0);
        const unsigned int and_padded_line_length_in_byte =
            ((and_line_length_in_byte % 2) ?
             and_line_length_in_byte + 1 :
             and_line_length_in_byte);
        for (unsigned int i0 = 0; i0 < height; ++i0) {
            uint8_t* xorMask = const_cast<uint8_t*>(this->data) + (height - i0 - 1) * xor_padded_line_length_in_byte;

            const uint8_t* andMask = this->mask + (height - i0 - 1) * and_padded_line_length_in_byte;
            unsigned char and_bit_extraction_mask = 7;

            for (unsigned int i1 = 0; i1 < width; ++i1) {
                if ((*andMask) & (1 << and_bit_extraction_mask)) {
                    *xorMask         = 0;
                    *(xorMask + 1)   = 0;
                    *(xorMask + 2)   = 0;
                }

                xorMask += 3;
                if (and_bit_extraction_mask) {
                    and_bit_extraction_mask--;
                }
                else {
                    and_bit_extraction_mask = 7;
                    andMask++;
                }
            }
        }
    }
    
};
