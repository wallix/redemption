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

#include "gdi/screen_info.hpp"
#include "utils/bitfu.hpp"
#include "utils/sugar/bytes_view.hpp"

class InStream;
class OutStream;
class BGRPalette;

struct CursorSize
{
    uint16_t width;
    uint16_t height;
    explicit constexpr CursorSize(uint16_t w, uint16_t h) : width(w), height(h) {}
};

struct Hotspot
{
    uint16_t x;
    uint16_t y;
    explicit constexpr Hotspot(uint16_t x, uint16_t y) : x(x), y(y) {}
};

struct Pointer
{
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
    // TODO OPTIMIZATION initialize with respect to dimensions
    uint8_t data[DATA_SIZE] {};
    uint8_t mask[MASK_SIZE] {};

    CursorSize dimensions {32,32};
    Hotspot hotspot {0, 0};

    BitsPerPixel native_xor_bpp { BitsPerPixel{0} };

    uint16_t native_length_and_mask { 0 };
    uint16_t native_length_xor_mask { 0 };

public:
    constexpr explicit Pointer() = default;

    template<class Builder>
    constexpr static Pointer build_from(CursorSize d, Hotspot hs, BitsPerPixel bits_per_pixel, Builder&& builder)
    {
        Pointer pointer;

        pointer.dimensions = d;
        pointer.hotspot = hs;
        pointer.native_xor_bpp = bits_per_pixel;
        pointer.native_length_xor_mask
          = checked_int(d.height * even_pad_length(d.width * nbbytes(underlying_cast(bits_per_pixel))));
        pointer.native_length_and_mask
          = checked_int(d.height * even_pad_length(nbbytes(d.width)));

        builder(pointer.data, pointer.mask);

        return pointer;
    }

    static Pointer build_from_native(CursorSize d, Hotspot hs, BitsPerPixel xor_bpp, bytes_view xor_mask, bytes_view and_mask);

    bool operator==(const Pointer & other) const;

    [[nodiscard]] CursorSize get_dimensions() const
    {
        return this->dimensions;
    }

    [[nodiscard]] Hotspot get_hotspot() const
    {
        return this->hotspot;
    }

    // size is a multiple of 2
    [[nodiscard]] bytes_view get_monochrome_and_mask() const
    {
        return {this->mask, this->bit_mask_size()};
    }

    // size is a multiple of 2
    [[nodiscard]] bytes_view get_nbits_xor_mask() const
    {
        return {this->data, this->xor_data_size()};
    }

    [[nodiscard]] unsigned bit_mask_size() const
    {
        return this->dimensions.height * ::even_pad_length(::nbbytes(this->dimensions.width));
    }

    [[nodiscard]] unsigned xor_data_size() const
    {
        uint8_t bytes_per_pixel = (this->native_xor_bpp == BitsPerPixel::BitsPP32) ? 4 : 3;

        return this->dimensions.height * ::even_pad_length(this->dimensions.width * bytes_per_pixel);
    }

    [[nodiscard]] bool is_valid() const
    {
        return (this->dimensions.width != 0 && this->dimensions.height != 0/* && this->bpp*/);
    }

    [[nodiscard]] BitsPerPixel get_native_xor_bpp() const { return native_xor_bpp; }

    [[nodiscard]] bytes_view get_native_xor_mask() const
    {
        return {this->data, this->native_length_xor_mask};
    }

    // TODO move in wrm_capture.hpp
    void emit_pointer32x32(OutStream & result) const;
    void emit_pointer2(OutStream & result) const;
};


//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.
void emit_color_pointer_update(OutStream& stream, uint16_t cache_idx, Pointer const& cursor);

//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).
void emit_new_pointer_update(OutStream& stream, uint16_t cache_idx, Pointer const& cursor);


bool emit_native_pointer(OutStream& stream, uint16_t cache_idx, Pointer const& cursor);


Pointer pointer_loader_new(BitsPerPixel data_bpp, InStream& stream);

Pointer decode_pointer(BitsPerPixel data_bpp,
                       uint16_t width,
                       uint16_t height,
                       uint16_t hsx,
                       uint16_t hsy,
                       uint16_t dlen,
                       const uint8_t * data,
                       uint16_t mlen,
                       const uint8_t * mask);

Pointer pointer_loader_2(InStream & stream);

Pointer pointer_loader_32x32(InStream & stream);

Pointer const& normal_pointer() noexcept;
Pointer const& edit_pointer() noexcept;
Pointer const& drawable_default_pointer() noexcept;
Pointer const& size_NS_pointer() noexcept;
Pointer const& size_NESW_pointer() noexcept;
Pointer const& size_NWSE_pointer() noexcept;
Pointer const& size_WE_pointer() noexcept;
Pointer const& dot_pointer() noexcept;
Pointer const& null_pointer() noexcept;
Pointer const& system_normal_pointer() noexcept;
Pointer const& system_default_pointer() noexcept;
