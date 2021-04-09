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

#include "core/RDP/rdp_pointer.hpp"
#include "utils/bitfu.hpp"
#include "utils/log.hpp"
#include "utils/colors.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

#include <algorithm> // std::min
#include <cassert>
#include <cstring>

Pointer Pointer::build_from_native(
    CursorSize d, Hotspot hs, BitsPerPixel xor_bpp, bytes_view xor_mask, bytes_view and_mask)
{
    Pointer pointer;

    pointer.dimensions = d;
    pointer.hotspot = hs;
    pointer.native_xor_bpp = xor_bpp;
    ::memcpy(pointer.data, xor_mask.data(), xor_mask.size());
    pointer.native_length_xor_mask = xor_mask.size();
    ::memcpy(pointer.mask, and_mask.data(), and_mask.size());
    pointer.native_length_and_mask = and_mask.size();
    return pointer;
}

bool Pointer::operator==(const Pointer & other) const
{
    return other.hotspot.x == this->hotspot.x
        && other.hotspot.y == this->hotspot.y
        && other.dimensions.width == this->dimensions.width
        && other.dimensions.height == this->dimensions.height
        && (0 == memcmp(this->data, other.data, other.native_length_xor_mask))
        && (0 == memcmp(this->mask, other.mask, this->bit_mask_size()));
}

//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.

//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).
//
//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in
//      bits-per-pixel of the XOR mask contained in the colorPtrAttr field.
//
//    colorPtrAttr (variable): Encapsulated Color Pointer Update (section
//      2.2.9.1.1.4.4) structure which contains information about the pointer.
//      The Color Pointer Update fields are all used, as specified in section
//      2.2.9.1.1.4.4; Color XOR data is presented in the
//      color depth described in the xorBpp field (for 8 bpp, each byte contains
//      one palette index; for 4 bpp, there are two palette indices per byte).
//
// ==> This part is obsolete (removed from MSRDPBCGR in 2009) : however, the XOR
// mask data alignment packing is slightly different. For monochrome (1 bpp)
// pointers the XOR data is always padded to a 4-byte boundary per scan line,
// while color pointer XOR data is still packed on a 2-byte boundary.

bool emit_native_pointer(OutStream& stream, uint16_t cache_idx, Pointer const& cursor)
{
    const auto dimensions = cursor.get_dimensions();
    const auto hotspot = cursor.get_hotspot();

    const bool new_pointer_update_used = (cursor.get_native_xor_bpp() != BitsPerPixel{24});

    if (new_pointer_update_used)
    {
        // xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in bits-per-pixel of the XOR mask
        //     contained in the colorPtrAttr field.
        stream.out_uint16_le(static_cast<uint16_t>(cursor.get_native_xor_bpp()));
    }

    // cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
    //   entry in the pointer cache in which to store the pointer image. The
    //   number of cache entries is negotiated using the Pointer Capability Set
    //   (section 2.2.7.1.5).

    stream.out_uint16_le(cache_idx);

    // hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
    //   x-coordinates and y-coordinates of the pointer hotspot.
    //         2.2.9.1.1.4.1  Point (TS_POINT16)
    //         ---------------------------------
    //         The TS_POINT16 structure specifies a point relative to the
    //         top-left corner of the server's desktop.
    //
    //         xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
    //           relative to the top-left corner of the server's desktop.

    //LOG(LOG_INFO, "hotspot.x=%u", hotspot.x);
    stream.out_uint16_le(hotspot.x);

    // yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
    //     relative to the top-left corner of the server's desktop.

    //LOG(LOG_INFO, "hotspot.y=%u", hotspot.y);
    stream.out_uint16_le(hotspot.y);

    // width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
    //     pixels (the maximum allowed pointer width is 32 pixels).

    //LOG(LOG_INFO, "dimensions.width=%u", dimensions.width);
    stream.out_uint16_le(dimensions.width);

    // height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
    //     in pixels (the maximum allowed pointer height is 32 pixels).

    //LOG(LOG_INFO, "dimensions.height=%u", dimensions.height);
    stream.out_uint16_le(dimensions.height);

    // lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
    //     the andMaskData field.

    auto av_and_mask = cursor.get_monochrome_and_mask();
    auto av_xor_mask = cursor.get_native_xor_mask();

    stream.out_uint16_le(av_and_mask.size());

    // lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
    //     the xorMaskData field.

    stream.out_uint16_le(av_xor_mask.size());

    // xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
    //     bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
    //     boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
    //     is being sent, then each scan-line will consume 10 bytes (3 pixels per
    //     scan-line multiplied by 3 bpp, rounded up to the next even number of
    //     bytes).
    //LOG(LOG_INFO, "xorMaskData=%zu", av_data.size());
    //hexdump(av_data);
    stream.out_copy_bytes(av_xor_mask);

    // andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
    //     bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
    //     boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
    //     is being sent, then each scan-line will consume 2 bytes (7 pixels per
    //     scan-line multiplied by 1 bpp, rounded up to the next even number of
    //     bytes).
    //LOG(LOG_INFO, "andMaskData=%zu", av_mask.size());
    //hexdump(av_mask);
    stream.out_copy_bytes(av_and_mask); /* mask */

    // colorPointerData (1 byte): Single byte representing unused padding.
    //     The contents of this byte should be ignored.
    if (stream.get_offset() % 2 != 0)
    {
        stream.out_clear_bytes(1);
    }

    return new_pointer_update_used;
}

Pointer decode_pointer(BitsPerPixel data_bpp,
                       uint16_t width,
                       uint16_t height,
                       uint16_t hsx,
                       uint16_t hsy,
                       uint16_t dlen,
                       const uint8_t * data,
                       uint16_t mlen,
                       const uint8_t * mask)
{
    return Pointer::build_from_native(CursorSize(width, height),
                                      Hotspot(hsx, hsy),
                                      data_bpp,
                                      bytes_view(data, dlen),
                                      bytes_view(mask, mlen));
}

Pointer pointer_loader_new(BitsPerPixel data_bpp, InStream& stream)
{
    auto hsx      = stream.in_uint16_le();
    auto hsy      = stream.in_uint16_le();
    auto width    = stream.in_uint16_le();
    auto height   = stream.in_uint16_le();

    uint16_t mlen = stream.in_uint16_le(); /* mask length */
    uint16_t dlen = stream.in_uint16_le(); /* data length */

    assert(::even_pad_length(::nbbytes(width)) == mlen / height);
    assert(::even_pad_length(::nbbytes_large(width * underlying_cast(data_bpp))) == dlen / height);

    if (!stream.in_check_rem(mlen + dlen)){
        LOG(LOG_ERR, "Not enough data for cursor (dlen=%u mlen=%u need=%u remain=%zu)",
            mlen, dlen, static_cast<uint16_t>(mlen+dlen), stream.in_remain());
        throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
    }

    const uint8_t * data = stream.in_uint8p(dlen);
    const uint8_t * mask = stream.in_uint8p(mlen);

    return decode_pointer(data_bpp,
                          width,
                          height,
                          hsx,
                          hsy,
                          dlen,
                          data,
                          mlen,
                          mask);
}

Pointer pointer_loader_2(InStream & stream)
{
    uint8_t width     = stream.in_uint8();
    uint8_t height    = stream.in_uint8();
    BitsPerPixel data_bpp{stream.in_uint8()};
    uint8_t hsx       = stream.in_uint8();
    uint8_t hsy       = stream.in_uint8();
    uint16_t dlen     = stream.in_uint16_le();
    uint16_t mlen     = stream.in_uint16_le();

    LOG_IF(dlen > Pointer::DATA_SIZE, LOG_ERR, "Corrupted recording: recorded mouse data length too large");
    LOG_IF(mlen > Pointer::MASK_SIZE, LOG_ERR, "Corrupted recording: recorded mouse data mask too large");

    auto data = stream.in_uint8p(dlen);
    auto mask = stream.in_uint8p(mlen);

    return decode_pointer(data_bpp,
                          width,
                          height,
                          hsx,
                          hsy,
                          dlen,
                          data,
                          mlen,
                          mask);
}

Pointer pointer_loader_32x32(InStream & stream)
{
    const uint8_t width     = 32;
    const uint8_t height    = 32;
    const BitsPerPixel data_bpp{24};
    const uint8_t hsx       = stream.in_uint8();
    const uint8_t hsy       = stream.in_uint8();
    const uint16_t dlen     = 32 * 32 * nb_bytes_per_pixel(data_bpp);
    uint16_t mlen           = 32 * ::nbbytes(32);

    LOG_IF(dlen > Pointer::DATA_SIZE, LOG_ERR, "Corrupted recording: recorded mouse data length too large");
    LOG_IF(mlen > Pointer::MASK_SIZE, LOG_ERR, "Corrupted recording: recorded mouse data mask too large");

    auto data = stream.in_uint8p(dlen);
    auto mask = stream.in_uint8p(mlen);

    return decode_pointer(data_bpp,
                          width,
                          height,
                          hsx,
                          hsy,
                          dlen,
                          data,
                          mlen,
                          mask);
}

namespace
{

constexpr Pointer predefined_pointer(
    const uint16_t width, const uint16_t height, const char * def,
    const uint16_t hsx, const uint16_t hsy)
{
    return Pointer::build_from(
        CursorSize(width, height),
        Hotspot(hsx, hsy),
        BitsPerPixel(24),
        [&](uint8_t * dest, uint8_t * dest_mask)
        {
            const char * src = def;
            for (size_t y = 0 ; y < height ; y++){
                unsigned bit_count = 7;
                uint8_t res_mask = 0;
                for (size_t x = 0 ; x < width ; x++){
                    const char c = *src;
                    const uint8_t v = (c == 'X' || c == '-') ? 0xFF : 0;
                    *dest++ = v;
                    *dest++ = v;
                    *dest++ = v;

                    res_mask |= (c == '.'|| c == '-') ? (1 << bit_count) : 0;
                    if (bit_count == 0){
                        *dest_mask =  res_mask;
                        dest_mask++;
                        res_mask = 0;
                        bit_count = 8;
                    }
                    bit_count--;
                    src++;
                }
                if (bit_count != 7){
                    *dest_mask = res_mask;
                }
            }
        }
    );
}

constexpr auto normal_pointer_v = predefined_pointer(32, 32,
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
    , 0, 0
);

constexpr auto edit_pointer_v = predefined_pointer(32, 32,
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
    , 15, 16
);

constexpr auto drawable_default_pointer_v = predefined_pointer(32, 32,
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
    , 0, 0
);

constexpr auto size_NS_pointer_v = predefined_pointer(32, 32,
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
    , 10, 10
);

constexpr auto size_NESW_pointer_v = predefined_pointer(32, 32,
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
    , 10, 10
);

constexpr auto size_NWSE_pointer_v = predefined_pointer(32, 32,
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
    , 10, 10
);

constexpr auto size_WE_pointer_v = predefined_pointer(32, 32,
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
    , 10, 10
);

constexpr auto dot_pointer_v = predefined_pointer(32, 32,
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
    , 2, 2
);

constexpr auto null_pointer_v = predefined_pointer(32, 32,
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
    , 2, 2
);

constexpr auto system_normal_pointer_v = predefined_pointer(32, 32,
    /* 0000 */ "................................"
    /* 0060 */ "..................XX............"
    /* 00c0 */ ".................X++X..........."
    /* 0120 */ ".................X++X..........."
    /* 0180 */ "................X++X............"
    /* 01e0 */ "..........X.....X++X............"
    /* 0240 */ "..........XX...X++X............."
    /* 02a0 */ "..........X+X..X++X............."
    /* 0300 */ "..........X++XX++X.............."
    /* 0360 */ "..........X+++X++X.............."
    /* 03c0 */ "..........X++++++XXXXX.........."
    /* 0420 */ "..........X+++++++++X..........."
    /* 0480 */ "..........X++++++++X............"
    /* 04e0 */ "..........X+++++++X............."
    /* 0540 */ "..........X++++++X.............."
    /* 05a0 */ "..........X+++++X..............."
    /* 0600 */ "..........X++++X................"
    /* 0660 */ "..........X+++X................."
    /* 06c0 */ "..........X++X.................."
    /* 0720 */ "..........X+X..................."
    /* 0780 */ "..........XX...................."
    /* 07e0 */ "..........X....................."
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
    , 10, 10
);

constexpr auto system_default_pointer_v = predefined_pointer(32, 32,
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
    , 10, 10
);

}

Pointer const& normal_pointer() noexcept { return normal_pointer_v; }
Pointer const& edit_pointer() noexcept { return edit_pointer_v; }
Pointer const& drawable_default_pointer() noexcept { return drawable_default_pointer_v; }
Pointer const& size_NS_pointer() noexcept { return size_NS_pointer_v; }
Pointer const& size_NESW_pointer() noexcept { return size_NESW_pointer_v; }
Pointer const& size_NWSE_pointer() noexcept { return size_NWSE_pointer_v; }
Pointer const& size_WE_pointer() noexcept { return size_WE_pointer_v; }
Pointer const& dot_pointer() noexcept { return dot_pointer_v; }
Pointer const& null_pointer() noexcept { return null_pointer_v; }
Pointer const& system_normal_pointer() noexcept { return system_normal_pointer_v; }
Pointer const& system_default_pointer() noexcept { return system_default_pointer_v; }
