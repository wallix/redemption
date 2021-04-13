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

struct CursorSize
{
    uint16_t width;
    uint16_t height;

    constexpr explicit CursorSize(uint16_t width, uint16_t height) noexcept
    : width(width)
    , height(height)
    {}
};

struct Hotspot
{
    uint16_t x;
    uint16_t y;

    constexpr explicit Hotspot(uint16_t x, uint16_t y) noexcept
    : x(x)
    , y(y)
    {}
};

struct RdpPointerView
{
    RdpPointerView() = default;

    constexpr explicit RdpPointerView(
        CursorSize dimensions,
        Hotspot hotspot,
        BitsPerPixel xor_bits_per_pixel,
        bytes_view xor_mask,
        bytes_view and_mask
    ) noexcept
    : dimensions_(dimensions)
    , hotspot_(hotspot)
    , xor_bits_per_pixel_(xor_bits_per_pixel)
    , xor_mask_(xor_mask)
    , and_mask_(and_mask)
    {
        assert(xor_bits_per_pixel == BitsPerPixel(0)
            || compute_mask_line_size(dimensions.width, xor_bits_per_pixel) != 0);
        assert(xor_mask.size() == dimensions.height
                                * compute_mask_line_size(dimensions.width, xor_bits_per_pixel));
        assert(and_mask.size() == dimensions.height
                                * compute_mask_line_size(dimensions.width, BitsPerPixel(1)));
    }

    static constexpr RdpPointerView from_raw_ptr(
        CursorSize dimensions,
        Hotspot hotspot,
        BitsPerPixel xor_bits_per_pixel,
        byte_ptr xor_mask_ptr,
        byte_ptr and_mask_ptr) noexcept
    {
        auto xor_mask_len = dimensions.height
          * RdpPointerView::compute_mask_line_size(dimensions.width, xor_bits_per_pixel);
        auto and_mask_len = dimensions.height
          * RdpPointerView::compute_mask_line_size(dimensions.width, BitsPerPixel{1});

        return RdpPointerView(
            dimensions,
            hotspot,
            BitsPerPixel{24},
            {xor_mask_ptr, xor_mask_len},
            {and_mask_ptr, and_mask_len});
    }

    constexpr CursorSize dimensions() const noexcept
    {
        return this->dimensions_;
    }

    constexpr Hotspot hotspot() const noexcept
    {
        return this->hotspot_;
    }

    constexpr BitsPerPixel xor_bits_per_pixel() const noexcept
    {
        return this->xor_bits_per_pixel_;
    }

    // padded to a 2-byte boundary
    constexpr bytes_view xor_mask() const noexcept
    {
        return this->xor_mask_;
    }

    // padded to a 2-byte boundary
    constexpr bytes_view and_mask() const noexcept
    {
        return this->and_mask_;
    }

    // padded to a 2-byte boundary
    constexpr static uint32_t compute_mask_line_size(
        uint16_t width, BitsPerPixel bits_per_pixel) noexcept
    {
        switch (bits_per_pixel) {
            case BitsPerPixel::BitsPP1:
                return ::even_pad_length(::nbbytes(width));

            case BitsPerPixel::BitsPP4:
                return ::even_pad_length(::nbbytes(width * 4));

            case BitsPerPixel::BitsPP8:
                return ::even_pad_length(width);

            case BitsPerPixel::BitsPP15:
            case BitsPerPixel::BitsPP16:
                return width * 2;

            case BitsPerPixel::BitsPP24:
                return ::even_pad_length(width * 3);

            case BitsPerPixel::BitsPP32:
                return width * 4;

            case BitsPerPixel::Unspecified:
                break;
        }

        return 0;
    }

private:
    CursorSize dimensions_ {0, 0};
    Hotspot hotspot_ {0, 0};
    BitsPerPixel xor_bits_per_pixel_;
    bytes_view xor_mask_;
    bytes_view and_mask_;
};


struct RdpPointer
{
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
    CursorSize dimensions {32,32};
    Hotspot hotspot {0, 0};

    BitsPerPixel native_xor_bpp { BitsPerPixel{0} };

    uint16_t native_length_and_mask { 0 };
    uint16_t native_length_xor_mask { 0 };

    uint8_t data[DATA_SIZE];
    uint8_t mask[MASK_SIZE];

public:
    RdpPointer() = default;

    struct constexpr_t {};

    template<class Builder>
    constexpr explicit RdpPointer(constexpr_t, CursorSize d, Hotspot hs, BitsPerPixel bits_per_pixel, Builder&& builder) noexcept
    : dimensions(d)
    , hotspot(hs)
    , native_xor_bpp(bits_per_pixel)
    , data{}
    , mask{}
    {
        this->native_length_xor_mask
          = checked_int(d.height * even_pad_length(d.width * nbbytes(underlying_cast(bits_per_pixel))));
        this->native_length_and_mask
          = checked_int(d.height * even_pad_length(nbbytes(d.width)));

        builder(this->data, this->mask);
    }

    explicit RdpPointer(CursorSize d, Hotspot hs, BitsPerPixel xor_bpp, bytes_view xor_mask, bytes_view and_mask) noexcept;

    RdpPointer(RdpPointer const& pointer) noexcept;
    explicit RdpPointer(RdpPointerView const& pointer) noexcept;

    RdpPointer& operator=(RdpPointer const& pointer) noexcept;
    RdpPointer& operator=(RdpPointerView const& pointer) noexcept;

    operator RdpPointerView () const noexcept
    {
        return RdpPointerView(
            get_dimensions(), get_hotspot(), get_native_xor_bpp(),
            get_native_xor_mask(), get_monochrome_and_mask());
    }

    inline RdpPointerView as_view() const noexcept
    {
        return *this;
    }

    [[nodiscard]] CursorSize get_dimensions() const noexcept
    {
        return this->dimensions;
    }

    [[nodiscard]] Hotspot get_hotspot() const noexcept
    {
        return this->hotspot;
    }

    // size is a multiple of 2
    [[nodiscard]] bytes_view get_monochrome_and_mask() const noexcept
    {
        return {this->mask, this->bit_mask_size()};
    }

    [[nodiscard]] unsigned bit_mask_size() const noexcept
    {
        return this->dimensions.height * ::even_pad_length(::nbbytes(this->dimensions.width));
    }

    [[nodiscard]] bool is_valid() const noexcept
    {
        return (this->dimensions.width != 0 && this->dimensions.height != 0/* && this->bpp*/);
    }

    [[nodiscard]] BitsPerPixel get_native_xor_bpp() const noexcept
    {
        return native_xor_bpp;
    }

    [[nodiscard]] bytes_view get_native_xor_mask() const noexcept
    {
        return {this->data, this->native_length_xor_mask};
    }
};


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

bool emit_native_pointer(OutStream& stream, uint16_t cache_idx, RdpPointerView const& cursor);


RdpPointerView pointer_loader_new(BitsPerPixel data_bpp, InStream& stream);

RdpPointerView pointer_loader_2(InStream & stream);

RdpPointerView pointer_loader_32x32(InStream & stream);

RdpPointer const& normal_pointer() noexcept;
RdpPointer const& edit_pointer() noexcept;
RdpPointer const& size_NS_pointer() noexcept;
RdpPointer const& size_NESW_pointer() noexcept;
RdpPointer const& size_NWSE_pointer() noexcept;
RdpPointer const& size_WE_pointer() noexcept;
RdpPointer const& dot_pointer() noexcept;
RdpPointer const& null_pointer() noexcept;
RdpPointer const& system_normal_pointer() noexcept;

enum class PredefinedPointer
{
    Normal,
    Edit,
    SystemNormal,
    Null,
    Dot,
    NS,
    NESW,
    NWSE,
    WE,

    COUNT,
};

RdpPointer const& predefined_pointer_to_pointer(PredefinedPointer pointer) noexcept;
