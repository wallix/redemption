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
#include "utils/hexdump.hpp"
#include "utils/log.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/colors.hpp"
#include "utils/pixel_io.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

#include <algorithm> // std::min
#include <cassert>
#include <cstring>

/** @brief the size of a cursor */
struct CursorSize {
    unsigned width;
    unsigned height;
    explicit CursorSize(unsigned w, unsigned h) : width(w), height(h) {}
};

/** @brief the hotspot coordinates of a cursor */
struct Hotspot {
    unsigned x;
    unsigned y;
    explicit Hotspot(unsigned x, unsigned y) : x(x), y(y) {}
};

inline void fix_32_bpp(CursorSize dimensions, uint8_t * data_buffer, uint8_t const * mask_buffer)
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

/** @brief a mouse pointer in the proxy */
struct Pointer
{
    friend class NewPointerUpdate;
    friend class ColorPointerUpdate;
    friend Pointer decode_pointer(BitsPerPixel data_bpp, const BGRPalette & palette,
                           uint16_t width, uint16_t height, uint16_t hsx, uint16_t hsy,
                           uint16_t dlen, const uint8_t * data,
                           uint16_t mlen, const uint8_t * mask,
                           bool clean_up_32_bpp_cursor);
    friend Pointer pointer_loader_vnc(BytesPerPixel Bpp, uint16_t width, uint16_t height, uint16_t hsx, uint16_t hsy, u8_array_view vncdata, u8_array_view vncmask, int red_shift, int red_max, int green_shift, int green_max, int blue_shift, int blue_max);
    friend Pointer pointer_loader_new(BitsPerPixel data_bpp, InStream & stream, const BGRPalette & palette, bool clean_up_32_bpp_cursor);
    friend Pointer predefined_pointer(const unsigned width, const unsigned height,
                                      const char * def,
                                      const unsigned hsx, const unsigned hsy,
                                      const bool inverted);

    friend Pointer harmonize_pointer(Pointer const& src_ptr);

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

    bool haveRdpPointer = false;
    BitsPerPixel original_bpp = BitsPerPixel(24);

private:
    uint8_t data[DATA_SIZE] {};
    uint16_t original_dlen = 0;
    uint8_t original_data[DATA_SIZE] {};

    uint8_t mask[MASK_SIZE] {};
    uint16_t original_mlen = 0;
    uint8_t original_mask[MASK_SIZE] {};

    bool only_black_white = false;

    CursorSize dimensions {32,32};
    Hotspot hotspot {0, 0};
    BitsPerPixel bpp {24};


public:
    explicit Pointer() = default;

    bool operator==(const Pointer & other) const {
        return (other.hotspot.x == this->hotspot.x
             && other.hotspot.y == this->hotspot.y
             && other.dimensions.width == this->dimensions.width
             && other.dimensions.height == this->dimensions.height
             && (0 == memcmp(this->data, other.data, other.xor_data_size()))
             && (0 == memcmp(this->mask, other.mask, this->bit_mask_size())));
    }

    explicit Pointer(CursorSize d, Hotspot hs)
        : dimensions(d)
        , hotspot(hs)
    {
    }

    explicit Pointer(BitsPerPixel bpp, CursorSize d, Hotspot hs, uint16_t dlen, const uint8_t *data,
            uint16_t mlen, const uint8_t *mask)
        : haveRdpPointer(true)
        , original_bpp(bpp)
        , original_dlen(dlen)
        , original_mlen(mlen)
        , dimensions(d)
        , hotspot(hs)
        , bpp(bpp)
    {
        memcpy(original_data, data, dlen);
        memcpy(original_mask, mask, mlen);
    }

    BitsPerPixel get_bpp() const { return bpp; }

    [[nodiscard]] CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    [[nodiscard]] Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    // size is a multiple of 2
    [[nodiscard]] u8_array_view get_monochrome_and_mask() const
    {
        return {this->mask, this->bit_mask_size()};
    }

    // size is a multiple of 2
    [[nodiscard]] u8_array_view get_24bits_xor_mask() const
    {
        return {this->data, this->xor_data_size()};
    }

    const uint8_t *get_original_data() const {
        return this->original_data;
    }

    [[nodiscard]] unsigned bit_mask_size() const {
        return this->dimensions.height * ::even_pad_length(::nbbytes(this->dimensions.width));
    }

    [[nodiscard]] unsigned xor_data_size() const {
        return this->dimensions.height * ::even_pad_length(this->dimensions.width * 3);
    }

    [[nodiscard]] bool is_valid() const {
        return (this->dimensions.width != 0 && this->dimensions.height != 0/* && this->bpp*/);
    }

    void emit_pointer2(OutStream & result) const
    {
        result.out_uint8(this->get_dimensions().width);
        result.out_uint8(this->get_dimensions().height);

        if (haveRdpPointer) {
            result.out_uint8(static_cast<uint8_t>(this->original_bpp));

            result.out_uint8(this->get_hotspot().x);
            result.out_uint8(this->get_hotspot().y);

            result.out_uint16_le(this->original_dlen);
            result.out_uint16_le(this->original_mlen);

            result.out_copy_bytes(this->original_data, this->original_dlen);
            result.out_copy_bytes(this->original_mask, this->original_mlen);

        } else {
            result.out_uint8(24);

            result.out_uint8(this->get_hotspot().x);
            result.out_uint8(this->get_hotspot().y);

            result.out_uint16_le(this->xor_data_size());
            result.out_uint16_le(this->bit_mask_size());

            result.out_copy_bytes(this->get_24bits_xor_mask());
            result.out_copy_bytes(this->get_monochrome_and_mask());
        }
    }

    void cleanup_32_bpp_cursor(unsigned width, unsigned height) {
        const unsigned int xor_line_length_in_byte = width * 3;
        const unsigned int xor_padded_line_length_in_byte = ::even_pad_length(xor_line_length_in_byte);
        const unsigned int and_line_length_in_byte = ::nbbytes(width);
        const unsigned int and_padded_line_length_in_byte = ::even_pad_length(and_line_length_in_byte);
        for (unsigned int i0 = 0; i0 < height; ++i0) {
            uint8_t* xorMask = this->data + (height - i0 - 1) * xor_padded_line_length_in_byte;

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

    std::string ascii_mask() const {
        std::string ret;
        const uint8_t *mask_ptr = this->mask;
        uint16_t mask = 0x80;

        for (unsigned y = 0; y < dimensions.height; y++) {
            for (unsigned x = 0; x < dimensions.width; x++) {
                if (*mask_ptr & mask) {
                    ret += " ";
                } else {
                    ret += "#";
                }

                mask >>= 1;
                if (mask == 00) {
                    mask_ptr++;
                    mask = 0x80;
                }
            }
            ret += "\n";
        }
        return ret;
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

        //LOG(LOG_INFO, "hotspot.x=%u", hotspot.x);
        stream.out_uint16_le(hotspot.x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        //LOG(LOG_INFO, "hotspot.y=%u", hotspot.y);
        stream.out_uint16_le(hotspot.y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        //LOG(LOG_INFO, "dimensions.width=%u", dimensions.width);
        stream.out_uint16_le(dimensions.width);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        //LOG(LOG_INFO, "dimensions.height=%u", dimensions.height);
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
        //LOG(LOG_INFO, "xorMaskData=%zu", av_data.size());
        //hexdump(av_data);
        stream.out_copy_bytes(av_data);

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).
        //LOG(LOG_INFO, "andMaskData=%zu", av_mask.size());
        //hexdump(av_mask);
        stream.out_copy_bytes(av_mask); /* mask */

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

        if (cursor.haveRdpPointer) {
            stream.out_uint16_le(static_cast<uint16_t>(this->cursor.original_bpp));
            stream.out_uint16_le(this->cache_idx);
            stream.out_uint16_le(hotspot.x);
            stream.out_uint16_le(hotspot.y);
            stream.out_uint16_le(dimensions.width);
            stream.out_uint16_le(dimensions.height);
            stream.out_uint16_le(cursor.original_mlen);
            stream.out_uint16_le(cursor.original_dlen);
            stream.out_copy_bytes(cursor.original_data, cursor.original_dlen);
            stream.out_copy_bytes(cursor.original_mask, cursor.original_mlen);
            return;
        }

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

            stream.out_copy_bytes(this->cursor.get_monochrome_and_mask()); /* mask */
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

    alignas(16)
    uint8_t data[Pointer::DATA_SIZE];

    explicit ARGB32Pointer(Pointer const & cursor)
        : dimensions(cursor.get_dimensions())
        , hotspot(cursor.get_hotspot())
    {
        assert(this->dimensions.width * this->dimensions.height * 4 <= sizeof(data));

        const uint8_t * cursormask = cursor.get_monochrome_and_mask().data();
        const uint8_t * cursordata = cursor.get_24bits_xor_mask().data();

        for (uint8_t y = 0 ; y < this->dimensions.height ; y++){
            const uint8_t * src_mask = &cursormask[y * ::nbbytes(this->dimensions.width)];
            const uint8_t * src_data = &cursordata[y * 3 * this->dimensions.width];
            uint8_t * target_data    = &this->data[4*(this->dimensions.height-1-y)*this->dimensions.width];
            uint8_t mask_count = 7;
            for(uint8_t x = 0 ; x < this->dimensions.width ; x++){
                uint8_t maskbit = *src_mask & (1 << mask_count);
                uint32_t posdata = 3*x;
                uint32_t postarget = 4*x;
                uint32_t pixel = src_data[posdata] + (src_data[posdata+1] << 8) + (src_data[posdata+2] << 16);
                ::out_bytes_le(&target_data[postarget], 4, (pixel|(maskbit==0)) ? (0xFF000000+pixel) : 0);
                src_mask += (mask_count==0) ? 1 : 0;
                mask_count = (mask_count-1) & 7;
            }
        }
    }

    [[nodiscard]] CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    [[nodiscard]] Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    [[nodiscard]] u8_array_view get_alpha_q() const
    {
        return {this->data, this->dimensions.width * this->dimensions.height * 4};
    }
};


inline Pointer decode_pointer(BitsPerPixel data_bpp, const BGRPalette & palette,
                           uint16_t width, uint16_t height, uint16_t hsx, uint16_t hsy,
                           uint16_t dlen, const uint8_t * data,
                           uint16_t mlen, const uint8_t * mask,
                           bool clean_up_32_bpp_cursor)
{
    Pointer cursor(data_bpp, CursorSize(width, height), Hotspot(hsx, hsy), dlen, data, mlen, mask);

    switch (data_bpp) {
    case BitsPerPixel{1}:
    {
        const unsigned int src_line_bytes = ::even_pad_length(::nbbytes(width));
        const unsigned int src_mask_line_bytes = ::even_pad_length(::nbbytes(width));
        const unsigned int dest_line_bytes = ::even_pad_length(width * 3);
        const unsigned int dest_mask_line_bytes = ::even_pad_length(::nbbytes(width));
        const uint8_t * src_last_line       = data + ((height-1) * src_line_bytes);
        const uint8_t * src_last_mask_line  = mask + ((height-1) * src_mask_line_bytes);

        for (unsigned int i = 0; i < height; ++i) {
            const uint8_t * src  = src_last_line     - i * src_line_bytes;
            const uint8_t* src_mask  = src_last_mask_line - i * src_mask_line_bytes;
            uint8_t *      dest = cursor.data + i * dest_line_bytes;
            uint8_t *      dest_mask = cursor.mask + i * dest_mask_line_bytes;

            unsigned char bit_count = 7;
            unsigned char mask_bit_count = 7;
            for (unsigned int j = 0; j < width ; ++j) {
                unsigned databit = *src      & (1 << bit_count);
                unsigned pixel = databit?0xFFFFFF:0 ;
                ::out_bytes_le(dest, 3, pixel);
                dest += 3;
                src            = src + ((bit_count==0)?1:0);
                bit_count      = (bit_count - 1) & 7;

                if (mask_bit_count == 0){
                    *dest_mask = *src_mask;
                    dest_mask++;
                    src_mask++;
                }
                mask_bit_count = (mask_bit_count - 1) & 7;
            }
        }

//         for (unsigned int y = 0; y < height; ++y) {
//             for (unsigned int x = 0; x < width; ++x) {
//                 printf("%s", (::get_pixel_1bpp(data, src_line_bytes, x, y) ? "#" : "."));
//             }
//             printf("\n");
//         }
//         printf("\n");
    }
    break;
    case BitsPerPixel{4}:
    {
        for (unsigned i = 0; i < dlen ; i++) {
            const uint8_t px = data[i];
            // target cursor will receive 8 bits input at once
            ::out_bytes_le(&(cursor.data[6 * i]),     3, palette[(px >> 4) & 0xF].as_u32());
            ::out_bytes_le(&(cursor.data[6 * i + 3]), 3, palette[ px       & 0xF].as_u32());
        }
        memcpy(cursor.mask, mask, mlen);
    }
    break;
    case BitsPerPixel{8}:
    case BitsPerPixel{15}:
    case BitsPerPixel{16}:
    case BitsPerPixel{24}:
    case BitsPerPixel{32}:
    {
        uint8_t BPP = nb_bytes_per_pixel(data_bpp);
        const unsigned int src_xor_line_length_in_byte = width * BPP;
        const unsigned int src_xor_padded_line_length_in_byte = ::even_pad_length(src_xor_line_length_in_byte);
        const unsigned int dest_xor_line_length_in_byte = width * 3;
        const unsigned int dest_xor_padded_line_length_in_byte = ::even_pad_length(dest_xor_line_length_in_byte);

        for (unsigned int i0 = 0; i0 < height; ++i0) {
            const uint8_t* src  = data + (height - i0 - 1) * src_xor_padded_line_length_in_byte;
            uint8_t* dest = cursor.data + (height - i0 - 1) * dest_xor_padded_line_length_in_byte;

            for (unsigned int i1 = 0; i1 < width; ++i1) {
                RDPColor px = RDPColor::from(in_uint32_from_nb_bytes_le(BPP, src));
                src += BPP;
                ::out_bytes_le(dest, 3, color_decode(px, data_bpp, palette).as_u32());
                dest += 3;
            }
        }
        memcpy(cursor.mask, mask, mlen);
        if ((data_bpp == BitsPerPixel{32}) && (clean_up_32_bpp_cursor)) {
            fix_32_bpp(cursor.dimensions, cursor.data, cursor.mask);
        }
    }
    break;
    default:
        // TODO : force some cursor if that happen
        LOG(LOG_ERR, "Mouse pointer : color depth not supported %d", data_bpp);
    break;
    }

   // LOG(LOG_ERR, "mask=%s", cursor.ascii_mask().c_str());
    return cursor;
}

inline Pointer pointer_loader_new(BitsPerPixel data_bpp, InStream & stream, const BGRPalette & palette, bool clean_up_32_bpp_cursor)
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

    return decode_pointer(data_bpp, palette, width, height, hsx, hsy, dlen, data, mlen, mask, clean_up_32_bpp_cursor);
}

inline Pointer pointer_loader_vnc(BytesPerPixel Bpp, uint16_t width, uint16_t height, uint16_t hsx, uint16_t hsy, u8_array_view vncdata, u8_array_view vncmask, int red_shift, int red_max, int green_shift, int green_max, int blue_shift, int blue_max)
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
// pixel in the cursor is valid.Pointer

    size_t minheight = std::min<size_t>(size_t(height), size_t(32));
    size_t minwidth = 32;

    Pointer cursor(CursorSize(minwidth, minheight), Hotspot(hsx, hsy));

    size_t target_offset_line = 0;
    size_t target_mask_offset_line = 0;
    size_t source_offset_line = (minheight-1) * width * underlying_cast(Bpp);
    size_t source_mask_offset_line = (minheight-1) * ::nbbytes(width);
    memset(cursor.data, 0xAA, sizeof(cursor.data));

    // LOG(LOG_INFO, "r%u rs<<%u g%u gs<<%u b%u bs<<%u", red_max, red_shift, green_max, green_shift, blue_max, blue_shift);
    for (size_t y = 0 ; y < minheight ; y++){
        for (size_t x = 0 ; x < 32 ; x++){
            const size_t target_offset = target_offset_line +x*3;
            const size_t source_offset = source_offset_line + x*underlying_cast(Bpp);
            unsigned pixel = 0;
            if (x < width) {
                for(size_t i = 0 ; i < underlying_cast(Bpp); i++){
                    pixel = (pixel<<8) + vncdata[source_offset+i];
                }
            }
            else {
                pixel = 0;
            }
            const unsigned red = (pixel >> red_shift) & red_max;
            const unsigned green = (pixel >> green_shift) & green_max;
            const unsigned blue = (pixel >> blue_shift) & blue_max;
            // LOG(LOG_INFO, "pixel=%.2X (%.1X, %.1X, %.1X)", pixel, red, green, blue);
            cursor.data[target_offset] = (red << 3) | (red >> 2);
            cursor.data[target_offset+1] = (green << 2) | (green >> 4);
            cursor.data[target_offset+2] = (blue << 3) | (blue >> 2);
        }
        for (size_t xx = 0 ; xx < 4 ; xx++){
            // LOG(LOG_INFO, "y=%u xx=%u source_mask_offset=%u target_mask_offset=%u")";
            if (xx < ::nbbytes(width)){
                cursor.mask[target_mask_offset_line+xx] = 0xFF ^ vncmask[source_mask_offset_line+xx];
            }
            else {
                cursor.mask[target_mask_offset_line+xx] = 0xFF;
            }
        }
        if ((minwidth % 8) != 0){
            cursor.mask[target_mask_offset_line+::nbbytes(minwidth)-1] |= (0xFF>>(minwidth % 8));
        }
        target_offset_line += 32*3;
        target_mask_offset_line += 4;
        source_offset_line -= width*underlying_cast(Bpp);
        source_mask_offset_line -= ::nbbytes(width);
    }
    return cursor;
}



inline Pointer pointer_loader_2(InStream & stream)
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
    const BGRPalette palette = BGRPalette::classic_332();
    return decode_pointer(data_bpp, palette, width, height, hsx, hsy, dlen, data, mlen, mask, true);
}

inline Pointer pointer_loader_32x32(InStream & stream)
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
    const BGRPalette palette = BGRPalette::classic_332();
    return decode_pointer(data_bpp, palette, width, height, hsx, hsy, dlen, data, mlen, mask, true);
}


inline Pointer predefined_pointer(const unsigned width, const unsigned height,
                                  const char * def,
                                  const unsigned hsx = 0, const unsigned hsy = 0, /*NOLINT*/
                                  const bool inverted = false) /*NOLINT*/
{
    Pointer cursor(CursorSize(width, height), Hotspot(hsx, hsy));
    uint8_t * dest      = cursor.data;
    uint8_t * dest_mask = cursor.mask;
    const char    * src = def;
    for (size_t y = 0 ; y < height ; y++){
        unsigned bit_count = 7;
        uint8_t res_mask = 0;
        for (size_t x = 0 ; x < width ; x++){
            const char c = *src;
            ::out_bytes_le(dest, 3, ((c == 'X' || c == '-') != inverted) ? 0xFFFFFF : 0);
            dest += 3;
            res_mask |= ((c == '.')||(c == '-'))?(1 << bit_count):0;
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

    return cursor;
}

inline Pointer harmonize_pointer(Pointer const& src_ptr) {
    Pointer dest_ptr(CursorSize(::even_pad_length(src_ptr.dimensions.width), src_ptr.dimensions.height), Hotspot(src_ptr.hotspot.x, src_ptr.hotspot.y));

    if (src_ptr.dimensions.width % 2) {
        const unsigned int src_xor_line_length_in_byte = src_ptr.dimensions.width * 3;
        const unsigned int src_xor_padded_line_length_in_byte = ::even_pad_length(src_xor_line_length_in_byte);
        const unsigned int src_and_line_length_in_byte = ::nbbytes(src_ptr.dimensions.width);
        const unsigned int src_and_padded_line_length_in_byte = ::even_pad_length(src_and_line_length_in_byte);

        const unsigned int dest_xor_line_length_in_byte = dest_ptr.dimensions.width * 3;
        const unsigned int dest_xor_padded_line_length_in_byte = ::even_pad_length(dest_xor_line_length_in_byte);
        const unsigned int dest_and_line_length_in_byte = ::nbbytes(dest_ptr.dimensions.width);
        const unsigned int dest_and_padded_line_length_in_byte = ::even_pad_length(dest_and_line_length_in_byte);

        div_t const div_result = ::div(dest_ptr.dimensions.width, 8);

        uint8_t const* src_xor  = src_ptr.data;
        uint8_t const* src_and  = src_ptr.mask;
        uint8_t* dest_xor = dest_ptr.data;
        uint8_t* dest_and = dest_ptr.mask;
        for (unsigned int i = 0; i < src_ptr.dimensions.height; ++i) {
            memcpy(dest_xor, src_xor, src_xor_padded_line_length_in_byte);
            memcpy(dest_and, src_and, src_and_padded_line_length_in_byte);

            (*(dest_and + div_result.quot)) |= (1 << (8 - div_result.rem));

            src_xor += src_xor_padded_line_length_in_byte;
            src_and += src_and_padded_line_length_in_byte;

            dest_xor += dest_xor_padded_line_length_in_byte;
            dest_and += dest_and_padded_line_length_in_byte;
        }
    }
    else {
        dest_ptr = src_ptr;
    }

    return dest_ptr;
}

inline Pointer normal_pointer()
{
    return predefined_pointer(32, 32,
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
    );
}


inline Pointer edit_pointer()
{
    return predefined_pointer(32, 32,
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
            , 15, 16);
}

inline Pointer drawable_default_pointer(bool inverted = false)
{
    return predefined_pointer(32, 32,
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
            , 0, 0, inverted
    );
}

inline Pointer size_NS_pointer()
{
    return predefined_pointer(32, 32,
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
        , 10, 10);
}


inline Pointer size_NESW_pointer()
{
    return predefined_pointer(32, 32,
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
    , 10, 10);
}


inline Pointer size_NWSE_pointer()
{
    return predefined_pointer(32, 32,
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
    , 10, 10);
}


inline Pointer size_WE_pointer()
{
    return predefined_pointer(32, 32,
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
    , 10, 10);
}

inline Pointer dot_pointer()
{
    return predefined_pointer(32, 32,
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
    , 2, 2);
}

inline Pointer null_pointer()
{
    return predefined_pointer(32, 32,
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
    , 2, 2);
}

inline Pointer system_normal_pointer()
{
    return predefined_pointer(32, 32,
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
    , 10, 10);
}


inline Pointer system_default_pointer()
{
    return predefined_pointer(32, 32,
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
    , 10, 10);
}
