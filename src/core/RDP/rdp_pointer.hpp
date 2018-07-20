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
    explicit CursorSize(unsigned w, unsigned h) : width(w), height(h) {}
};

struct Hotspot {
    unsigned x;
    unsigned y;
    explicit Hotspot(unsigned x, unsigned y) : x(x), y(y) {}
};

static bool is_black_and_white(const uint8_t * data, const size_t width, const size_t height, const size_t row_length, const unsigned Bpp);
static void to_regular_mask(CursorSize dimensions, uint8_t * mask, const uint8_t * indata, unsigned mlen, uint8_t bpp);
static void to_regular_pointer(CursorSize dimensions, uint8_t * data, const uint8_t * indata, unsigned dlen, uint8_t bpp, const BGRPalette & palette);

static bool is_black_and_white(const uint8_t * data, const size_t width, const size_t height, const size_t row_length, const unsigned Bpp)
{
    for (unsigned int h = 0; h < height; ++h) {
        const uint8_t * row = data + h * row_length;
        for (unsigned int w = 0; w < width; ++w) {
            unsigned pixel = ::in_uint32_from_nb_bytes_le(Bpp, row + w*Bpp);
            if ((pixel != 0) and (pixel != 0xFFFFFF)){
                return false;
            }
        }
    }
    return true;
}


static void to_regular_mask(CursorSize dimensions, uint8_t * mask, const uint8_t * indata, unsigned mlen, uint8_t bpp) {
    /* TODO check code below: why do we revert mask and pointer when pointer is 1 BPP
        * and not with other color depth ? Looks fishy, a mask and pointer should always
        * be encoded in the same way, not depending on color depth difficult to see for
        * symmetrical pointers... check documentation it may be more efficient to revert
        * cursor after creating it instead of doing it on the fly */
    switch (bpp) {
    case 1 :
    {
        const unsigned int and_line_length_in_byte = ::nbbytes(dimensions.width);
        const unsigned int and_padded_line_length_in_byte = ::even_pad_length(and_line_length_in_byte);
        for (unsigned int i = 0; i < dimensions.height; ++i) {
            const uint8_t* src  = indata + (dimensions.height - i - 1) * and_padded_line_length_in_byte;
            uint8_t * dest = mask + i * and_padded_line_length_in_byte;
            ::memcpy(dest, src, and_padded_line_length_in_byte);
        }
    }
    break;
    default:
        memcpy(mask, indata, mlen);
    break;
    }
}


static void to_regular_pointer(CursorSize dimensions, uint8_t * data, const uint8_t * indata, unsigned dlen, uint8_t bpp, const BGRPalette & palette)
{
    switch (bpp) {
    case 1 :
    {
        const unsigned int src_xor_line_length_in_byte = ::nbbytes(dimensions.width);
        const unsigned int src_xor_padded_line_length_in_byte = ::even_pad_length(src_xor_line_length_in_byte);

        const unsigned int dest_xor_line_length_in_byte        = dimensions.width * 3;
        const unsigned int dest_xor_padded_line_length_in_byte = ::even_pad_length(dest_xor_line_length_in_byte);

        for (unsigned int i = 0; i < dimensions.height; ++i) {
            const uint8_t* src  = indata + (dimensions.height - i - 1) * src_xor_padded_line_length_in_byte;
                    uint8_t* dest = data + i * dest_xor_padded_line_length_in_byte;

            unsigned char and_bit_extraction_mask = 7;

            for (unsigned int j = 0; j < dimensions.width; ++j) {
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
            ::out_bytes_le(&(data[6 * i]),     3, palette[(px >> 4) & 0xF].to_u32());
            ::out_bytes_le(&(data[6 * i + 3]), 3, palette[ px       & 0xF].to_u32());
        }
    }
    break;
    case 32: case 24: case 16: case 15: case 8:
    {
        uint8_t BPP = nbbytes(bpp);

        const unsigned int src_xor_line_length_in_byte = dimensions.width * BPP;
        const unsigned int src_xor_padded_line_length_in_byte = ::even_pad_length(src_xor_line_length_in_byte);

        const unsigned int dest_xor_line_length_in_byte = dimensions.width * 3;
        const unsigned int dest_xor_padded_line_length_in_byte = ::even_pad_length(dest_xor_line_length_in_byte);

        for (unsigned int i0 = 0; i0 < dimensions.height; ++i0) {
            const uint8_t* src  = indata + (dimensions.height - i0 - 1) * src_xor_padded_line_length_in_byte;
                    uint8_t* dest = data + (dimensions.height - i0 - 1) * dest_xor_padded_line_length_in_byte;

            for (unsigned int i1 = 0; i1 < dimensions.width; ++i1) {
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
            ::out_bytes_le(data + x *3, 3, GREEN);
        }
        break;
    }
}



struct BasePointer {
protected:
    CursorSize dimensions;
    Hotspot hotspot;
public:
    const CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    const Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    explicit BasePointer(CursorSize dimensions, Hotspot hotspot)
        : dimensions(dimensions)
        , hotspot(hotspot)
    {}
};

struct ConstPointer : public BasePointer {
    const char * data;
    explicit ConstPointer(const CursorSize & d, const Hotspot & hs, const char * data)
        : BasePointer(d, hs)
        , data(data)
    {}
};

struct NormalPointer : public ConstPointer {
    explicit NormalPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{0,0},
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
    )
    {}
};

struct EditPointer : public ConstPointer {
    explicit EditPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{15,16},
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
    )
    {}
};

struct DrawableDefaultPointer : public ConstPointer {
    explicit DrawableDefaultPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{0,0},
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
    )
    {}
};

struct SizeNSPointer : public ConstPointer {
    explicit SizeNSPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{10,10},
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
    )
    {}
};

struct SizeNESWPointer : public ConstPointer {
    explicit SizeNESWPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{10,10},
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
    )
    {}
};


struct SizeNWSEPointer : public ConstPointer {
    explicit SizeNWSEPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{10,10},
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
    )
    {}
};


struct SizeWEPointer : public ConstPointer {
    explicit SizeWEPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{10,10},
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
    )
    {}
};

struct DotPointer : public ConstPointer {
    explicit DotPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{2,2},
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
    )
    {}
};

struct NullPointer : public ConstPointer {
    explicit NullPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{2,2},
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
    /* 0a20 */ "................................"
    /* 0a80 */ "................................"
    /* 0ae0 */ "................................"
    /* 0b40 */ "................................"
    /* 0ba0 */ "................................"
    )
    {}
};

struct SystemDefaultPointer : public ConstPointer {
    explicit SystemDefaultPointer()
        : ConstPointer(CursorSize{32,32}, Hotspot{10,10},
    /* 0000 */ "--------------------------------"
    /* 0060 */ "------------------XX------------"
    /* 00c0 */ "-----------------X++X-----------"
    /* 0120 */ "-----------------X++X-----------"
    /* 0180 */ "----------------X++X------------"
    /* 01e0 */ "----------X-----X++X------------"
    /* 0240 */ "----------XX---X++X-------------"
    /* 02a0 */ "----------X+X--X++X-------------"
    /* 0300 */ "----------X++XX++X--------------"
    /* 0360 */ "----------X+++X++X--------------"
    /* 03c0 */ "----------X++++++XXXXX----------"
    /* 0420 */ "----------X+++++++++X-----------"
    /* 0480 */ "----------X++++++++X------------"
    /* 04e0 */ "----------X+++++++X-------------"
    /* 0540 */ "----------X++++++X--------------"
    /* 05a0 */ "----------X+++++X---------------"
    /* 0600 */ "----------X++++X----------------"
    /* 0660 */ "----------X+++X-----------------"
    /* 06c0 */ "----------X++X------------------"
    /* 0720 */ "----------X+X-------------------"
    /* 0780 */ "----------XX--------------------"
    /* 07e0 */ "----------X---------------------"
    /* 0840 */ "--------------------------------"
    /* 08a0 */ "--------------------------------"
    /* 0900 */ "--------------------------------"
    /* 0960 */ "--------------------------------"
    /* 09c0 */ "--------------------------------"
    /* 0a20 */ "--------------------------------"
    /* 0a80 */ "--------------------------------"
    /* 0ae0 */ "--------------------------------"
    /* 0b40 */ "--------------------------------"
    /* 0ba0 */ "--------------------------------"
    )
    {}
};

static void fix_32_bpp(CursorSize dimensions, uint8_t * data_buffer, uint8_t * mask_buffer);
static void fix_32_bpp(CursorSize dimensions, uint8_t * data_buffer, uint8_t * mask_buffer)
{
    const unsigned int xor_line_length_in_byte = dimensions.width * 3;
    const unsigned int xor_padded_line_length_in_byte = ::even_pad_length(xor_line_length_in_byte);
    const unsigned int and_line_length_in_byte = ::nbbytes(dimensions.width);
    const unsigned int and_padded_line_length_in_byte = ::even_pad_length(and_line_length_in_byte);
    for (unsigned int i0 = 0; i0 < dimensions.height; ++i0) {
        uint8_t* xorMask = data_buffer + (dimensions.height - i0 - 1) * xor_padded_line_length_in_byte;

        const uint8_t* andMask = mask_buffer + (dimensions.height - i0 - 1) * and_padded_line_length_in_byte;
        unsigned char and_bit_extraction_mask = 7;

        // TODO: iterating on width... check scanline padding is OK
        for (unsigned int i1 = 0; i1 < dimensions.width; ++i1) {
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




struct PointerLoaderNew
{
    CursorSize dimensions;
    unsigned maskline_bytes;
    unsigned xorline_bytes;
    uint8_t data_bpp;
    Hotspot hotspot;
    array_view_const_u8 data;
    array_view_const_u8 mask;
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
        , MAX_BPP    = 32
    };
    enum {
          DATA_SIZE = MAX_WIDTH * MAX_HEIGHT * MAX_BPP / 8
        , MASK_SIZE = MAX_WIDTH * MAX_HEIGHT * 1 / 8
    };

    uint8_t data_buffer[DATA_SIZE];
    uint8_t mask_buffer[MASK_SIZE];

    explicit PointerLoaderNew(uint8_t data_bpp, InStream & stream, const BGRPalette & palette, bool clean_up_32_bpp_cursor, BogusLinuxCursor bogus_linux_cursor)
        : dimensions(0, 0)
        , data_bpp{data_bpp}
        , hotspot(0, 0)
    {
        auto hotspot_x      = stream.in_uint16_le();
        auto hotspot_y      = stream.in_uint16_le();
        this->hotspot       = Hotspot(hotspot_x, hotspot_y);
        auto width          = stream.in_uint16_le();
        auto height         = stream.in_uint16_le();
        this->dimensions    = CursorSize(width, height);

        uint16_t mlen = stream.in_uint16_le(); /* mask length */
        uint16_t dlen = stream.in_uint16_le(); /* data length */

        assert(::even_pad_length(::nbbytes(width)) == mlen / height);
        assert(::even_pad_length(::nbbytes(width * data_bpp)) == dlen / height);

        if (!stream.in_check_rem(mlen + dlen)){
            LOG(LOG_ERR, "Not enough data for cursor (dlen=%u mlen=%u need=%u remain=%zu)",
                mlen, dlen, static_cast<uint16_t>(mlen+dlen), stream.in_remain());
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
        }

        const uint8_t * data = stream.in_uint8p(dlen);
        const uint8_t * mask = stream.in_uint8p(mlen);

        switch (data_bpp) {
        case 1:
        {
            uint8_t data_data[DATA_SIZE];
            uint8_t mask_data[MASK_SIZE];
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
            ::to_regular_pointer(this->dimensions, this->data_buffer, data_data, dlen, 1, palette);
            ::to_regular_mask(this->dimensions, this->mask_buffer, mask_data, mlen, 1);
        }
        break;
        default:
        case 4:
        case 8:
        case 15:
        case 16:
        case 24:
            ::to_regular_pointer(this->dimensions, this->data_buffer, data, dlen, data_bpp, palette);
            ::to_regular_mask(this->dimensions, this->mask_buffer, mask, mlen, data_bpp);
        break;
        case 32:
        {
            ::to_regular_pointer(this->dimensions, this->data_buffer, data, dlen, data_bpp, palette);
            ::to_regular_mask(this->dimensions, this->mask_buffer, mask, mlen, data_bpp);
            if (clean_up_32_bpp_cursor) {
                fix_32_bpp(this->dimensions, this->data_buffer, this->mask_buffer);
            }
        }
        break;
        }

        this->data = make_array_view(this->data_buffer, dlen);
        this->mask = make_array_view(this->mask_buffer, mlen);
        this->maskline_bytes = mlen / height;
        this->xorline_bytes = dlen / height;
    }
};



struct PointerLoader2
{
    CursorSize dimensions;
    unsigned maskline_bytes;
    unsigned xorline_bytes;
    uint8_t data_bpp;
    Hotspot hotspot;
    array_view_const_u8 data;
    array_view_const_u8 mask;

    explicit PointerLoader2(InStream & stream)
        : dimensions(0, 0)
        , data_bpp{0}
        , hotspot(0, 0)
    {
        uint8_t width    = stream.in_uint8();
        uint8_t height   = stream.in_uint8();
        this->dimensions = CursorSize(width, height);
        this->data_bpp = stream.in_uint8();
        uint8_t hotspot_x = stream.in_uint8();
        uint8_t hotspot_y = stream.in_uint8();
        this->hotspot = Hotspot(hotspot_x, hotspot_y);
        uint16_t dlen = stream.in_uint16_le();
        uint16_t mlen = stream.in_uint16_le();
        // TODO: assert(dlen <= MAX_WIDTH * MAX_HEIGHT * 3);
        // TODO: assert(mlen <= MAX_WIDTH * MAX_HEIGHT * 1 / 8);
        auto data = stream.in_uint8p(dlen);
        auto mask = stream.in_uint8p(mlen);
        this->data = make_array_view(data, dlen);
        this->mask = make_array_view(mask, mlen);
        this->maskline_bytes = mlen / height;
        this->xorline_bytes = dlen / height;

    }
};

struct PointerLoader32x32
{
    CursorSize dimensions;
    unsigned maskline_bytes;
    unsigned xorline_bytes;
    uint8_t data_bpp;
    Hotspot hotspot;
    array_view_const_u8 data;
    array_view_const_u8 mask;

    explicit PointerLoader32x32(InStream & stream)
        : dimensions(32, 32)
        , maskline_bytes(4)
        , xorline_bytes(96)
        , data_bpp{24}
        , hotspot(0, 0)
    {
        uint8_t hotspot_x = stream.in_uint8();
        uint8_t hotspot_y = stream.in_uint8();
        this->hotspot = Hotspot(hotspot_x, hotspot_y);
        uint16_t dlen = 32*32*::nbbytes(24);
        uint16_t mlen = 32*::nbbytes(32);
        // TODO: assert(dlen <= MAX_WIDTH * MAX_HEIGHT * 3);
        // TODO: assert(mlen <= MAX_WIDTH * MAX_HEIGHT / 8);
        auto data = stream.in_uint8p(dlen);
        auto mask = stream.in_uint8p(mlen);
        this->data = make_array_view(data, dlen);
        this->mask = make_array_view(mask, mlen);
    }
};

struct Pointer : public BasePointer {

    friend class NewPointerUpdate;
    friend class ColorPointerUpdate;

    unsigned maskline_bytes = 0;
    unsigned xorline_bytes = 0;


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

    uint8_t data[DATA_SIZE];

    uint8_t mask[MASK_SIZE];

    bool only_black_white = false;

public:

    explicit Pointer(uint8_t Bpp, CursorSize d, Hotspot hs, const std::vector<uint8_t> & vncdata, const std::vector<uint8_t> & vncmask,
                   int red_shift, int red_max, int green_shift, int green_max, int blue_shift, int blue_max, unsigned maskline_bytes, unsigned xorline_bytes)
        : BasePointer(CursorSize(32,d.height), hs)
        , maskline_bytes(maskline_bytes)
        , xorline_bytes(xorline_bytes)
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
       size_t minwidth = 32;

       size_t target_offset_line = 0;
       size_t target_mask_offset_line = 0;
       size_t source_offset_line = (minheight-1) * d.width * Bpp;
       size_t source_mask_offset_line = (minheight-1) * ::nbbytes(d.width);
       memset(this->data, 0xAA, sizeof(this->data));

//       LOG(LOG_INFO, "r%u rs<<%u g%u gs<<%u b%u bs<<%u", red_max, red_shift, green_max, green_shift, blue_max, blue_shift);
       for (size_t y = 0 ; y < minheight ; y++){
            for (size_t x = 0 ; x < 32 ; x++){
                const size_t target_offset = target_offset_line +x*3;
                const size_t source_offset = source_offset_line + x*Bpp;
                unsigned pixel = 0;
                if (x < d.width) {
                    for(size_t i = 0 ; i < Bpp ; i++){
    //                    pixel = (pixel<<8) + vncdata[source_offset+Bpp-i-1];
                        pixel = (pixel<<8) + vncdata[source_offset+i];
                    }
                }
                else {
                    pixel = 0;
                }
                const unsigned red = (pixel >> red_shift) & red_max;
                const unsigned green = (pixel >> green_shift) & green_max;
                const unsigned blue = (pixel >> blue_shift) & blue_max;
//               LOG(LOG_INFO, "pixel=%.2X (%.1X, %.1X, %.1X)", pixel, red, green, blue);
                this->data[target_offset] = (red << 3) | (red >> 2);
                this->data[target_offset+1] = (green << 2) | (green >> 4);
                this->data[target_offset+2] = (blue << 3) | (blue >> 2);
            }
            for (size_t xx = 0 ; xx < 4 ; xx++){
//                LOG(LOG_INFO, "y=%u xx=%u source_mask_offset=%u target_mask_offset=%u")";
                if (xx < ::nbbytes(d.width)){
                    this->mask[target_mask_offset_line+xx] = 0xFF ^ vncmask[source_mask_offset_line+xx];
                }
                else {
                    this->mask[target_mask_offset_line+xx] = 0xFF;
                }
            }
            if ((minwidth % 8) != 0){
                this->mask[target_mask_offset_line+::nbbytes(minwidth)-1] |= (0xFF>>(minwidth % 8));
            }
            target_offset_line += 32*3;
            target_mask_offset_line += 4;
            source_offset_line -= d.width*Bpp;
            source_mask_offset_line -= ::nbbytes(d.width);
       }
       this->dimensions.width = 32;
       LOG(LOG_INFO, "width=%u height=%u", d.width, d.height);
    }

    explicit Pointer(const PointerLoader2 pl)
     : Pointer(pl.data_bpp, pl.dimensions, pl.hotspot, pl.data, pl.mask, pl.maskline_bytes, pl.xorline_bytes)
    {
    }

    explicit Pointer(const PointerLoader32x32 pl)
     : Pointer(pl.data_bpp, pl.dimensions, pl.hotspot, pl.data, pl.mask, pl.maskline_bytes, pl.xorline_bytes)
    {
    }

    explicit Pointer(const PointerLoaderNew pl)
     : Pointer(pl.data_bpp, pl.dimensions, pl.hotspot, pl.data, pl.mask, pl.maskline_bytes, pl.xorline_bytes)
    {
        unsigned Bpp = 3;
        this->only_black_white = ::is_black_and_white(
                pl.data.data(),
                this->dimensions.width,
                this->dimensions.height,
                ::even_pad_length(this->dimensions.width * Bpp),
                Bpp);
    }


    explicit Pointer(uint8_t data_bpp, CursorSize d, Hotspot hs, array_view_const_u8 av_xor, array_view_const_u8 av_and, unsigned maskline_bytes, unsigned xorline_bytes)
    : BasePointer(d, hs)
    , maskline_bytes(maskline_bytes)
    , xorline_bytes(xorline_bytes)
    {
        (void)data_bpp;
        if ((av_and.size() > this->bit_mask_size()) || (av_xor.size() > this->xor_data_size())) {
            LOG(LOG_ERR, "mod_rdp::process_color_pointer_pdu: "
                "bad length for color pointer mask_len=%zu data_len=%zu",
                av_and.size(), av_and.size());
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK);
        }

        memcpy(this->mask, av_and.data(), av_and.size());
        memcpy(this->data, av_xor.data(), av_xor.size());
    }

    bool operator==(const Pointer & other) const {
        return (other.hotspot.x == this->hotspot.x
             && other.hotspot.y == this->hotspot.y
             && other.dimensions.width == this->dimensions.width
             && other.dimensions.height == this->dimensions.height
             && (0 == memcmp(this->data, other.data, other.xor_data_size()))
             && (0 == memcmp(this->mask, other.mask, this->bit_mask_size())));
    }


    explicit Pointer(const ConstPointer & p = NullPointer{}, bool inverted  = false)
    :   BasePointer(p.get_dimensions(), p.get_hotspot())
    , maskline_bytes(::nbbytes(p.get_dimensions().width))
    , xorline_bytes(p.get_dimensions().width*3)
    {
        this->only_black_white = true;
        const char * cursor = p.data;
        uint8_t * tmp = this->data;
        memset(this->mask, 0, this->dimensions.width * this->dimensions.height / 8);
        for (size_t i = 0 ; i < this->dimensions.width * this->dimensions.height ; i++) {
            // COLOR: X:1 .:0 +:0 -:1
            // MASK:  X:0 .:1 +:0 -:1
            uint8_t v = (((cursor[i] == 'X')||(cursor[i] == '-'))^inverted) ? 0xFF : 0;
            tmp[0] = tmp[1] = tmp[2] = v;
            tmp += 3;
            this->mask[i/8]|= ((cursor[i] == '.')||(cursor[i] == '-'))?(0x80 >> (i%8)):0;
        }
    }

    void set_mask_to_FF(){
        ::memset(this->mask, 0xFF, sizeof(this->mask));
    }

    const array_view_const_u8 get_monochrome_and_mask() const
    {
        return {this->mask, this->bit_mask_size()};
    }

    const array_view_const_u8 get_24bits_xor_mask() const
    {
        return {this->data, this->xor_data_size()};
    }


    unsigned bit_mask_size() const {
        return ::nbbytes(this->dimensions.width) * this->dimensions.height;
    }

    unsigned xor_data_size() const {
        return this->dimensions.height * ::even_pad_length(this->dimensions.width * 3);
    }

    bool is_valid() const {
        return (this->dimensions.width != 0 && this->dimensions.height != 0/* && this->bpp*/);
    }

    void emit_pointer32x32(OutStream & result) const
    {
        result.out_uint8(this->get_hotspot().x);
        result.out_uint8(this->get_hotspot().y);

        result.out_copy_bytes(this->get_24bits_xor_mask());
        result.out_copy_bytes(this->get_monochrome_and_mask());
    }

    void emit_pointer2(OutStream & result) const
    {
        result.out_uint8(this->get_dimensions().width);
        result.out_uint8(this->get_dimensions().height);
        result.out_uint8(24);

        result.out_uint8(this->get_hotspot().x);
        result.out_uint8(this->get_hotspot().y);

        result.out_uint16_le(this->xor_data_size());
        result.out_uint16_le(this->bit_mask_size());

        result.out_copy_bytes(this->get_24bits_xor_mask());
        result.out_copy_bytes(this->get_monochrome_and_mask());
    }

    void cleanup_32_bpp_cursor(unsigned width, unsigned height) {
        const unsigned int xor_line_length_in_byte = width * 3;
        const unsigned int xor_padded_line_length_in_byte = ::even_pad_length(xor_line_length_in_byte);
        const unsigned int and_line_length_in_byte = ::nbbytes(width);
        const unsigned int and_padded_line_length_in_byte = ::even_pad_length(and_line_length_in_byte);
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
    explicit ColorPointerUpdate(int cache_idx, const Pointer & cursor)
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
//      2.2.9.1.1.4.4; Color XOR data is presented in the
///     color depth described in the xorBpp field (for 8 bpp, each byte contains
//      one palette index; for 4 bpp, there are two palette indices per byte).

// ==> This part is obsolete (removed from MSRDPBCGR in 2009) : however, the XOR
// mask data alignment packing is slightly different. For monochrome (1 bpp)
// pointers the XOR data is always padded to a 4-byte boundary per scan line,
// while color pointer XOR data is still packed on a 2-byte boundary.

class NewPointerUpdate
{
    int cache_idx;
    const Pointer & cursor;

public:
    explicit NewPointerUpdate(int cache_idx, const Pointer & cursor)
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


        const unsigned int and_line_length_in_byte = ::nbbytes(dimensions.width);
        const unsigned int and_padded_line_length_in_byte = ::even_pad_length(and_line_length_in_byte);
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
        const unsigned int source_xor_padded_line_length_in_byte = ::even_pad_length(source_xor_line_length_in_byte);

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

    explicit ARGB32Pointer(Pointer const & cursor)
        : dimensions(cursor.get_dimensions())
        , hotspot(cursor.get_hotspot())
    {
        assert(this->dimensions.width * this->dimensions.height * 4 <= sizeof(data));

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



