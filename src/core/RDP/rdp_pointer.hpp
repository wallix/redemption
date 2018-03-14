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
#include <vector>
#include "utils/log.hpp"
#include "utils/hexdump.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/colors.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

struct CursorSize {
    unsigned width;
    unsigned height;
    CursorSize(unsigned w, unsigned h) : width(w), height(h) {}
    CursorSize(const CursorSize & cs) : width(cs.width), height(cs.height) {}
};

struct Hotspot {
    unsigned x;
    unsigned y;
    Hotspot(unsigned x, unsigned y) : x(x), y(y) {}
    Hotspot(const Hotspot & hs) : x(hs.x), y(hs.y) {}
};

struct Pointer {

    friend class NewPointerUpdate;
    friend class ColorPointerUpdate;
    
// TODO: in TS_SYSTEMPOINTERATTRIBUTE, POINTER_NULL and POINTER_NORMAL are attributed specific values
// we could directly provide these to Pointer constructor instead of defining a switch on call site (rdp.hpp)

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
        POINTER_DOT              ,  // Little Dot of 5x5 pixels

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


private:
//    unsigned bpp;
    
    CursorSize dimensions;
    
    Hotspot hotspot;
    
    struct {
        alignas(4) 
        uint8_t data[DATA_SIZE];
    } alpha_q;

    uint8_t data[DATA_SIZE];

    uint8_t mask[MASK_SIZE];

    bool only_black_white = false;

public:

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

    explicit Pointer(CursorSize d, Hotspot hs, array_view_const_u8 av_xor, array_view_const_u8 av_and, uint8_t data_bpp, const BGRPalette & palette, bool clean_up_32_bpp_cursor, BogusLinuxCursor bogus_linux_cursor)
        : dimensions(d)
        , hotspot(hs)
    {
        auto mlen = av_and.size();
        auto dlen = av_xor.size();
        auto data = av_xor.data();
        auto mask = av_and.data();
        
        if (data_bpp == 1) {
            uint8_t data_data[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT / 8];
            uint8_t mask_data[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT / 8];
            ::memcpy(data_data, data, dlen);
            ::memcpy(mask_data, mask, mlen);

            if (bogus_linux_cursor == BogusLinuxCursor::enable) {
                for (unsigned i = 0 ; i < mlen; i++) {
                    uint8_t new_mask_data = (mask_data[i] & (data_data[i] ^ 0xFF));
                    uint8_t new_data_data = (data_data[i] ^ mask_data[i] ^ new_mask_data);
                    data_data[i]    = new_data_data;
                    mask_data[i]    = new_mask_data;
                }
            }

            // TODO move that into cursor
            this->to_regular_pointer(data_data, dlen, 1, palette);
            this->to_regular_mask(mask_data, mlen, 1);
            this->only_black_white = true;
        }
        else {
            this->only_black_white = false;
            // TODO move that into cursor
            this->to_regular_pointer(data, dlen, data_bpp, palette);
            this->to_regular_mask(mask, mlen, data_bpp);

            if ((data_bpp == 32) && clean_up_32_bpp_cursor) {
                const unsigned int xor_line_length_in_byte = this->dimensions.width * 3;
                const unsigned int xor_padded_line_length_in_byte =
                    ((xor_line_length_in_byte % 2) ?
                     xor_line_length_in_byte + 1 :
                     xor_line_length_in_byte);
                const unsigned int remainder = (this->dimensions.width % 8);
                const unsigned int and_line_length_in_byte = this->dimensions.width / 8 + (remainder ? 1 : 0);
                const unsigned int and_padded_line_length_in_byte =
                    ((and_line_length_in_byte % 2) ?
                     and_line_length_in_byte + 1 :
                     and_line_length_in_byte);
                for (unsigned int i0 = 0; i0 < this->dimensions.height; ++i0) {
                    uint8_t* xorMask = const_cast<uint8_t*>(this->data) + (this->dimensions.height - i0 - 1) * xor_padded_line_length_in_byte;

                    const uint8_t* andMask = this->mask + (this->dimensions.height - i0 - 1) * and_padded_line_length_in_byte;
                    unsigned char and_bit_extraction_mask = 7;

                    for (unsigned int i1 = 0; i1 < this->dimensions.width; ++i1) {
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

        }
    }

    Pointer(uint8_t Bpp, CursorSize d, Hotspot hs, const std::vector<uint8_t> & vncdata, const std::vector<uint8_t> & vncmask, 
                   int red_shift, int red_max, int green_shift, int green_max, int blue_shift, int blue_max)
        : dimensions(d)
        , hotspot(hs)
    {
    // VNC Pointer format
    // ==================
    
    // The data consists of width * height pixel values followed by
    // a bitmask.

    // PIXEL array : width * height * bytesPerPixel
    // bitmask     : floor((width + 7) / 8) * height

    // The bitmask consists of left-to-right, top-to-bottom
    // scanlines, where each scanline is padded to a whole number of
    // bytes. Within each byte the most significant bit represents
    // the leftmost pixel, with a 1-bit meaning the corresponding
    // pixel in the cursor is valid.

       size_t minheight = std::min<size_t>(size_t(d.height), size_t(32));
       size_t minwidth = std::min<size_t>(size_t(d.width), size_t(32));

       size_t target_offset_line = 0;
       size_t target_mask_offset_line = 0;
       size_t source_offset_line = (minheight-1) * d.width * Bpp;
       size_t source_mask_offset_line = (minheight-1) * ::nbbytes(d.width);
       memset(this->data, 0xAA, sizeof(this->data));
       
//       LOG(LOG_INFO, "r%u rs<<%u g%u gs<<%u b%u bs<<%u", red_max, red_shift, green_max, green_shift, blue_max, blue_shift);
       for (size_t y = 0 ; y < minheight ; y++){
            for (size_t x = 0 ; x < minwidth ; x++){
                const size_t target_offset = target_offset_line +x*3;
                const size_t source_offset = source_offset_line + x*Bpp;
                unsigned pixel = 0;
                for(size_t i = 0 ; i < Bpp ; i++){
//                    pixel = (pixel<<8) + vncdata[source_offset+Bpp-i-1];
                    pixel = (pixel<<8) + vncdata[source_offset+i];
                }
                const unsigned red = (pixel >> red_shift) & red_max; 
                const unsigned green = (pixel >> green_shift) & green_max; 
                const unsigned blue = (pixel >> blue_shift) & blue_max;
//               LOG(LOG_INFO, "pixel=%.2X (%.1X, %.1X, %.1X)", pixel, red, green, blue);
                this->data[target_offset] = (red << 3) | (red >> 2);
                this->data[target_offset+1] = (green << 2) | (green >> 4);
                this->data[target_offset+2] = (blue << 3) | (blue >> 2);
            }
            for (size_t xx = 0 ; xx*8 < minwidth ; xx++){
//                LOG(LOG_INFO, "y=%u xx=%u source_mask_offset=%u target_mask_offset=%u")";
                this->mask[target_mask_offset_line+xx] = 0xFF ^ vncmask[source_mask_offset_line+xx];
            }
            if ((minwidth % 8) != 0){
                this->mask[target_mask_offset_line+::nbbytes(minwidth)-1] |= (0xFF>>(minwidth % 8));
            }

            target_offset_line += (minwidth + (minwidth & 1))*3;
            target_mask_offset_line += ::nbbytes(minwidth);
            source_offset_line -= d.width*Bpp;
            source_mask_offset_line -= ::nbbytes(d.width);
       }
       if (minwidth & 1){
        this->dimensions.width++;
       }
        
    }


    explicit Pointer(CursorSize d, Hotspot hs, array_view_const_u8 av_xor, array_view_const_u8 av_and)
        : dimensions(d)
        , hotspot(hs)
    {
        if ((av_and.size() > this->bit_mask_size()) || (av_xor.size() > this->xor_mask_size())) {
            LOG(LOG_ERR, "mod_rdp::process_color_pointer_pdu: "
                "bad length for color pointer mask_len=%zu data_len=%zu",
                av_and.size(), av_and.size());
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }

        memcpy(this->mask, av_and.data(), av_and.size());
        memcpy(this->data, av_xor.data(), av_xor.size());
        this->update_bw();
    }

    explicit Pointer(uint8_t data_bpp, CursorSize d, Hotspot hs, array_view_const_u8 av_xor, array_view_const_u8 av_and)
        : dimensions(d)
        , hotspot(hs)
    {
        (void)data_bpp;
        if ((av_and.size() > this->bit_mask_size()) || (av_xor.size() > this->xor_mask_size())) {
            LOG(LOG_ERR, "mod_rdp::process_color_pointer_pdu: "
                "bad length for color pointer mask_len=%zu data_len=%zu",
                av_and.size(), av_and.size());
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }

        memcpy(this->mask, av_and.data(), av_and.size());
        memcpy(this->data, av_xor.data(), av_xor.size());
    }

    explicit Pointer(const Pointer & other)
    : dimensions(other.dimensions)
    , hotspot(other.hotspot)
    , only_black_white(other.only_black_white)
    {
        auto & av_and = other.get_monochrome_and_mask();
        auto & av_xor = other.get_24bits_xor_mask();
        memset(this->mask, 0, sizeof(this->mask));
        memcpy(this->mask, av_and.data(), av_and.size());
        memset(this->data, 0, sizeof(this->data));
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
            case POINTER_DOT:
                {
//                    this->bpp              = 24;
                    this->dimensions.width            = 32;
                    this->dimensions.height           = 32;
                    this->hotspot.x                = 2; /* hotspot */
                    this->hotspot.y                = 2;
                    this->only_black_white = true;
                    const char * data_cursor8 =
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
                        /* 0660 */ "................................"
                        /* 06c0 */ "................................"
                        /* 0720 */ "................................"
                        /* 0780 */ "................................"
                        /* 07e0 */ "................................"
                        /* 0840 */ "................................"
                        /* 08a0 */ "................................"
                        /* 0900 */ "................................"
                        /* 0960 */ "................................"
                        /* 09c0 */ "................................"
                        /* 0a20 */ "XXXXX..........................."
                        /* 0a80 */ "X+++X..........................."
                        /* 0ae0 */ "X+++X..........................."
                        /* 0b40 */ "X+++X..........................."
                        /* 0ba0 */ "XXXXX..........................."
                        ;
                    this->store_data_cursor(data_cursor8);
                }
                break;  // case POINTER_DOT:

        }   // switch (pointer_type)

    }   // Pointer(uint8_t pointer_type)


    void set_mask_to_FF(void){
        ::memset(this->mask, 0xFF, sizeof(this->mask));
    }

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
        return this->dimensions.height * (l_width+(l_width&1));
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
    
    void to_regular_pointer(const uint8_t * indata, unsigned dlen, uint8_t bpp, const BGRPalette & palette) 
    {
        switch (bpp) {
        case 1 :
        {
            const unsigned int remainder = (this->dimensions.width % 8);
            const unsigned int src_xor_line_length_in_byte = this->dimensions.width / 8 + (remainder ? 1 : 0);
            const unsigned int src_xor_padded_line_length_in_byte =
                ((src_xor_line_length_in_byte % 2) ?
                 src_xor_line_length_in_byte + 1 :
                 src_xor_line_length_in_byte);

            const unsigned int dest_xor_line_length_in_byte        = this->dimensions.width * 3;
            const unsigned int dest_xor_padded_line_length_in_byte =
                dest_xor_line_length_in_byte + ((dest_xor_line_length_in_byte % 2) ? 1 : 0);

            for (unsigned int i = 0; i < this->dimensions.height; ++i) {
                const uint8_t* src  = indata + (this->dimensions.height - i - 1) * src_xor_padded_line_length_in_byte;
                      uint8_t* dest = this->data + i * dest_xor_padded_line_length_in_byte;

                unsigned char and_bit_extraction_mask = 7;

                for (unsigned int j = 0; j < this->dimensions.width; ++j) {
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
                ::out_bytes_le(&(this->data[6 * i]),     3, palette[(px >> 4) & 0xF].to_u32());
                ::out_bytes_le(&(this->data[6 * i + 3]), 3, palette[ px       & 0xF].to_u32());
            }
        }
        break;
        case 32: case 24: case 16: case 15: case 8:
        {
            uint8_t BPP = nbbytes(bpp);

            const unsigned int src_xor_line_length_in_byte = this->dimensions.width * BPP;
            const unsigned int src_xor_padded_line_length_in_byte =
                ((src_xor_line_length_in_byte % 2) ?
                 src_xor_line_length_in_byte + 1 :
                 src_xor_line_length_in_byte);

            const unsigned int dest_xor_line_length_in_byte = this->dimensions.width * 3;
            const unsigned int dest_xor_padded_line_length_in_byte =
                ((dest_xor_line_length_in_byte % 2) ?
                 dest_xor_line_length_in_byte + 1 :
                 dest_xor_line_length_in_byte);

            for (unsigned int i0 = 0; i0 < this->dimensions.height; ++i0) {
                const uint8_t* src  = indata + (this->dimensions.height - i0 - 1) * src_xor_padded_line_length_in_byte;
                      uint8_t* dest = this->data + (this->dimensions.height - i0 - 1) * dest_xor_padded_line_length_in_byte;

                for (unsigned int i1 = 0; i1 < this->dimensions.width; ++i1) {
                    RDPColor px = RDPColor::from(in_uint32_from_nb_bytes_le(BPP, src));
                    src += BPP;
                    ::out_bytes_le(dest, 3, color_decode(px, bpp, palette).to_u32());
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

    void to_regular_mask(const uint8_t * indata, unsigned mlen, uint8_t bpp) {
        /* TODO check code below: why do we revert mask and pointer when pointer is 1 BPP
         * and not with other color depth ? Looks fishy, a mask and pointer should always
         * be encoded in the same way, not depending on color depth difficult to see for
         * symmetrical pointers... check documentation it may be more efficient to revert
         * cursor after creating it instead of doing it on the fly */
        switch (bpp) {
        case 1 :
        {
            const unsigned int remainder = (this->dimensions.width % 8);
            const unsigned int and_line_length_in_byte = this->dimensions.width / 8 + (remainder ? 1 : 0);
            const unsigned int and_padded_line_length_in_byte =
                ((and_line_length_in_byte % 2) ?
                 and_line_length_in_byte + 1 :
                 and_line_length_in_byte);
            for (unsigned int i = 0; i < this->dimensions.height; ++i) {
                const uint8_t* src  = indata + (this->dimensions.height - i - 1) * and_padded_line_length_in_byte;
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

//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.

// cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache entry in the pointer cache in which
// to store the pointer image. The number of cache entries is specified using the Pointer Capability Set (section 2.2.7.1.5).

// hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the x-coordinates and y-coordinates of the pointer hotspot.

// width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in pixels. The maximum allowed pointer width
// is 96 pixels if the client indicated support for large pointers by setting the LARGE_POINTER_FLAG (0x00000001) in the 
// Large Pointer Capability Set (section 2.2.7.2.7). 
// If the LARGE_POINTER_FLAG was not set, the maximum allowed pointer width is 32 pixels.

// height (2 bytes): A 16-bit, unsigned integer. The height of the pointer in pixels. The maximum allowed pointer height
// is 96 pixels if the client indicated support for large pointers by setting the LARGE_POINTER_FLAG (0x00000001) in the 
// Large Pointer Capability Set (section 2.2.7.2.7). 
// If the LARGE_POINTER_FLAG was not set, the maximum allowed pointer height is 32 pixels.

// lengthAndMask (2 bytes):  A 16-bit, unsigned integer. The size in bytes of the andMaskData field.

// lengthXorMask (2 bytes):  A 16-bit, unsigned integer. The size in bytes of the xorMaskData field.

// xorMaskData (variable): A variable-length array of bytes. Contains the 24-bpp, bottom-up XOR mask scan-line data. 
// The XOR mask is padded to a 2-byte boundary for each encoded scan-line. For example, if a 3x3 pixel cursor is being sent, 
// then each scan-line will consume 10 bytes (3 pixels per scan-line multiplied by 3 bytes per pixel, rounded up to the next even 
// number of bytes).

// andMaskData (variable): A variable-length array of bytes. Contains the 1-bpp, bottom-up AND mask scan-line data. 
// The AND mask is padded to a 2-byte boundary for each encoded scan-line. For example, if a 7x7 pixel cursor is being sent, 
// then each scan-line will consume 2 bytes (7 pixels per scan-line multiplied by 1 bpp, rounded up to the next even number of bytes).

// pad (1 byte): An optional 8-bit, unsigned integer. Padding. Values in this field MUST be ignored.

class ColorPointerUpdate {
    int cache_idx;
    const Pointer & cursor;

public:
    ColorPointerUpdate(int cache_idx, const Pointer & cursor)
        : cache_idx(cache_idx)
        , cursor(cursor)
    {
    }
    
    void emit(OutStream & stream)
    {

        const auto dimensions = cursor.get_dimensions();
        const auto hotspot = cursor.get_hotspot();
    
//    cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
//      entry in the pointer cache in which to store the pointer image. The
//      number of cache entries is negotiated using the Pointer Capability Set
//      (section 2.2.7.1.5).

        stream.out_uint16_le(cache_idx);

//    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
//      x-coordinates and y-coordinates of the pointer hotspot.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.


        stream.out_uint16_le(hotspot.x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(hotspot.y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(dimensions.width);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(dimensions.height);

//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        auto av_mask = cursor.get_monochrome_and_mask();
        auto av_data = cursor.get_24bits_xor_mask();

        stream.out_uint16_le(av_mask.size());

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(av_data.size());

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).
        stream.out_copy_bytes(av_data.data(), av_data.size());

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).
        stream.out_copy_bytes(av_mask.data(), av_mask.size()); /* mask */

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.
        // TODO: why isn't this padding byte sent ?
    }
};

//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).

//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in
//      bits-per-pixel of the XOR mask contained in the colorPtrAttr field.

//    colorPtrAttr (variable): Encapsulated Color Pointer Update (section
//      2.2.9.1.1.4.4) structure which contains information about the pointer.
//      The Color Pointer Update fields are all used, as specified in section
//      2.2.9.1.1.4.4; however, the XOR mask data alignment packing is slightly
//      different. For monochrome (1 bpp) pointers the XOR data is always padded
//      to a 4-byte boundary per scan line, while color pointer XOR data is
//      still packed on a 2-byte boundary. Color XOR data is presented in the
///     color depth described in the xorBpp field (for 8 bpp, each byte contains
//      one palette index; for 4 bpp, there are two palette indices per byte).

class NewPointerUpdate
{
    int cache_idx;
    const Pointer & cursor;

public:
    NewPointerUpdate(int cache_idx, const Pointer & cursor)
        : cache_idx(cache_idx)
        , cursor(cursor)
    {
    }

    void emit(OutStream & stream)
    {
        const auto dimensions = this->cursor.get_dimensions();
        const auto hotspot = this->cursor.get_hotspot();

//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in bits-per-pixel of the XOR mask
//      contained in the colorPtrAttr field.
        stream.out_uint16_le(this->cursor.only_black_white ? 1 : 32);

        stream.out_uint16_le(this->cache_idx);

//    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
//      x-coordinates and y-coordinates of the pointer hotspot.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(hotspot.x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(hotspot.y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(dimensions.width);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(dimensions.height);


        const unsigned int remainder = (dimensions.width % 8);
        const unsigned int and_line_length_in_byte = dimensions.width / 8 + (remainder ? 1 : 0);
        const unsigned int and_padded_line_length_in_byte =
            ((and_line_length_in_byte % 2) ?
             and_line_length_in_byte + 1 :
             and_line_length_in_byte);

        const unsigned int xor_padded_line_length_in_byte = (this->cursor.only_black_white ? and_padded_line_length_in_byte : dimensions.width * 4);


//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        stream.out_uint16_le(and_padded_line_length_in_byte * dimensions.height);

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(xor_padded_line_length_in_byte * dimensions.height);

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).

        const unsigned int source_xor_line_length_in_byte = dimensions.width * 3;
        const unsigned int source_xor_padded_line_length_in_byte =
            ((source_xor_line_length_in_byte % 2) ?
             source_xor_line_length_in_byte + 1 :
             source_xor_line_length_in_byte);

        if (this->cursor.only_black_white) {
            uint8_t xorMaskData[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT * 1 / 8] = { 0 };
            auto av_xor = this->cursor.get_24bits_xor_mask();

            for (unsigned int h = 0; h < dimensions.height; ++h) {
                const uint8_t * psource = av_xor.data()
                    + (dimensions.height - h - 1) * source_xor_padded_line_length_in_byte;
                uint8_t * pdest   = xorMaskData + h * xor_padded_line_length_in_byte;
                uint8_t xor_bit_mask_generation = 7;
                uint8_t xor_byte = 0;

                for (unsigned int w = 0; w < dimensions.width; ++w) {
                    if ((*psource) || (*(psource + 1)) || (*(psource + 2))) {
                        xor_byte |= (1 << xor_bit_mask_generation);
                    }

                    if (!xor_bit_mask_generation) {
                        xor_bit_mask_generation = 8;
                        *pdest = xor_byte;
                        xor_byte = 0;
                        pdest++;
                    }
                    xor_bit_mask_generation--;
                    psource += 3;
                }
                if (xor_bit_mask_generation != 7) {
                    *pdest = xor_byte;
                }
            }

            stream.out_copy_bytes(xorMaskData, xor_padded_line_length_in_byte * dimensions.height);
            

            auto av_and = this->cursor.get_monochrome_and_mask();
            uint8_t andMaskData[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT / 8] = { 0 };

            for (unsigned int h = 0; h < dimensions.height; ++h) {
                const uint8_t* psource = &av_and.data()[(dimensions.height - h - 1) * and_padded_line_length_in_byte];
                      uint8_t* pdest   = andMaskData + h * and_padded_line_length_in_byte;

                memcpy(pdest, psource, and_padded_line_length_in_byte);
            }

            stream.out_copy_bytes(andMaskData, and_padded_line_length_in_byte * dimensions.height); /* mask */
            
        }
        else {
            auto av_and = this->cursor.get_monochrome_and_mask();
            
            uint8_t xorMaskData[Pointer::MAX_WIDTH * Pointer::MAX_HEIGHT * 4] = { 0 };
            auto av_xor = this->cursor.get_24bits_xor_mask();

            for (unsigned int h = 0; h < dimensions.height; ++h) {
                const uint8_t* psource = av_xor.data() + (dimensions.height - h - 1) * source_xor_padded_line_length_in_byte;
                      uint8_t* pdest   = xorMaskData + (dimensions.height - h - 1) * xor_padded_line_length_in_byte;
                const uint8_t* andMask = &(av_and.data()[(dimensions.height - h - 1) * and_padded_line_length_in_byte]);
                unsigned char and_bit_extraction_mask = 7;


                for (unsigned int w = 0; w < dimensions.width; ++w) {
                    * pdest      = * psource;
                    *(pdest + 1) = *(psource + 1);
                    *(pdest + 2) = *(psource + 2);
                    if ((*andMask) & (1 << and_bit_extraction_mask)) {
                        *(pdest + 3) = 0x00;
                    }
                    else {
                        *(pdest + 3) = 0xFF;
                    }

                    pdest   += 4;
                    psource += 3;

                    if (and_bit_extraction_mask) {
                        and_bit_extraction_mask--;
                    }
                    else {
                        and_bit_extraction_mask = 7;
                        andMask++;
                    }
                }
            }

            stream.out_copy_bytes(xorMaskData, xor_padded_line_length_in_byte * dimensions.height);

            auto av = this->cursor.get_monochrome_and_mask();
            stream.out_copy_bytes(av.data(), av.size()); /* mask */
        }

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.
    }
};

struct ARGB32Pointer {
    CursorSize dimensions;
    Hotspot hotspot;

    alignas(4) 
    uint8_t data[Pointer::DATA_SIZE];

    ARGB32Pointer(Pointer const & cursor)
        : dimensions(cursor.get_dimensions())
        , hotspot(cursor.get_hotspot())
    {
        const uint8_t * cursormask = cursor.get_monochrome_and_mask().data();
        const uint8_t * cursordata = cursor.get_24bits_xor_mask().data();
        size_t mask_offset_line = 0;
        size_t data_offset_line = 0;
        size_t target_data_offset_line = ((this->dimensions.height - 1) * this->dimensions.width*4);
        for (uint8_t y = 0 ; y < this->dimensions.height ; y++){
            for(uint8_t x = 0 ; x < this->dimensions.width ; x++){
                const size_t mask_offset = mask_offset_line +::nbbytes(x+1)-1; 
                const size_t data_offset = data_offset_line + x*3; 
                const size_t target_data_offset = target_data_offset_line + x*4;
                //LOG(LOG_INFO, "(x=%d/%u, y=%d/%u) mw=%zu mx=%zu, mask_offset=%zu data_offset=%zu target_offset%zu",x, this->dimensions.width, y, this->dimensions.height, 
                //            size_t(::nbbytes(this->dimensions.width)), size_t(::nbbytes(x+1)), mask_offset, data_offset, target_data_offset);
                uint8_t mask_value = (cursormask[mask_offset]&(0x80>>(x&7)))?0x00:0xFF;
                for (uint8_t i = 0 ; i < 3 ; i++){
                    uint8_t value = cursordata[data_offset+i];
                    this->data[target_data_offset+i] = (mask_value == 0)?0:value;
                }
                this->data[target_data_offset+3] = mask_value;
            }
            mask_offset_line += ::nbbytes(this->dimensions.width);
            data_offset_line += 3 * this->dimensions.width;
            target_data_offset_line -= this->dimensions.width*4;
        }
    }

    CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    const array_view_const_u8 get_alpha_q() const
    {
        return {this->data, this->dimensions.width * this->dimensions.height * 4};
    }
};

