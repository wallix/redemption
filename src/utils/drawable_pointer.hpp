/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean, Poelen Jonathan
*/

/*
    Drawable uses a special form of "pointer"
    defined as a list of non overlapping contiguous sets of pixels
    each set is defined by
    - x, y coordinates relative to origin of pointer
    - the number of pixels in that zone
    - a pointer to where to find or store the actual pixel values.

    The current implementation first create an empty hull (without actual pointer to data)
    then use one of several initialisation function
    either to perform a copy of the mouse pixel data inside DrawablePointer,
    either to perform a copy of pixels below pointer from the image
        (when we create a DrawablePointer from background data to restore pointer later)
*/

#pragma once

#include "utils/bitfu.hpp"
#include "utils/log.hpp"
#include "utils/pixel_io.hpp"
#include "utils/image_view.hpp"
#include "utils/colors.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include <cstdint>
#include <cstddef>
#include <cstring>


class Array2D
{
    size_t width_in_bytes;
    size_t height;
    const uint8_t * data;

public:
    class Iterator
    {
        const uint8_t * data;
        size_t nbbytes;
    public:
        explicit Iterator (const uint8_t * data, size_t nbbytes) : data(data), nbbytes(nbbytes) {}
        bool operator!= (const Iterator & other) const { return this->data != other.data; }
        const uint8_t * operator* () const { return this->data; }
        const Iterator & operator++ () { this->data += this->nbbytes; return *this; }
    };

public:
    explicit Array2D(size_t width_in_bytes, size_t height, const uint8_t * data) :
    width_in_bytes(width_in_bytes), height(height), data(data)
    {}

    [[nodiscard]] Iterator begin () const { return Iterator(this->data, this->width_in_bytes); }
    [[nodiscard]] Iterator end () const { return Iterator(&this->data[this->width_in_bytes * this->height], this->width_in_bytes); }
};

class PixelArray
{
    uint8_t * data;
    const size_t width_in_bytes;

public:
    class Iterator
    {
        uint8_t * data;
        size_t offset;
    public:
        explicit Iterator (uint8_t * data) : data(data), offset(0) {}
        bool operator!= (const Iterator & other) const { return (this->data != other.data) || (this->offset != other.offset); }
        uint8_t * operator* () const { return &this->data[this->offset]; }
        const Iterator & operator++ () { this->offset+=3; return *this; }
    };

    explicit PixelArray(size_t width_in_pixels, uint8_t * data) : data(data), width_in_bytes(width_in_pixels*3)  {}

    [[nodiscard]] Iterator begin () const { return Iterator(this->data); }
    [[nodiscard]] Iterator end () const { return Iterator(this->data + this->width_in_bytes); }
};


class BitZones
{
    size_t width_in_bits;
    const uint8_t * data;

public:
    class Iterator
    {
        const size_t width_in_bits;
        const uint8_t * data;
        size_t offset;
        struct Zone {
            bool bit;
            size_t length;
            explicit Zone(bool bit, size_t length) : bit(bit), length(length) {}
        } zone;
    public:
        explicit Iterator (const uint8_t * data, size_t width_in_bits, size_t offset) : width_in_bits(width_in_bits), data(data), offset(offset), zone(false,0)
        {
            this->operator++();
        }

        bool operator!= (const Iterator & other) const { return (this->data != other.data) || (this->offset != other.offset); }

        Zone operator* () const { return this->zone;}

        const Iterator & operator++ () {
            if (this->offset < this->width_in_bits){
                this->offset += zone.length;
                if (this->offset < this->width_in_bits){
                    bool bit = this->data[this->offset>>3]&(1<<(7-(offset&0x7)));

                    size_t lg = 1;
                    while ((this->offset+lg) < this->width_in_bits){
                        bool bit2 = this->data[(this->offset+lg)>>3]&(1<<(7-((this->offset+lg)&0x7)));
                        if (bit2 != bit) { break;}
                        lg++;
                    }
//                    printf("%d - zone(%d, %d) - %d | ", this->offset, bit, lg, this->offset+ lg);
                    this->zone = Zone(bit, lg);
                }
            }
            return *this;
        }
    };

    explicit BitZones(size_t width_in_bits, const uint8_t * data) : width_in_bits(width_in_bits), data(data)  {
//        printf("bitzone %.2x %.2x %.2x %.2x\n", data[0], data[1], data[2], data[3]);
    }

    [[nodiscard]] Iterator begin () const { return Iterator(this->data, this->width_in_bits, 0); }
    [[nodiscard]] Iterator end () const { return Iterator(this->data, this->width_in_bits, this->width_in_bits); }
};


class BitArray
{
    size_t width_in_bits;
    const uint8_t * data;

public:
    class Iterator
    {
        const uint8_t * data;
        size_t offset;
    public:
        explicit Iterator (const uint8_t * data, size_t offset) : data(data), offset(offset) {}
        bool operator!= (const Iterator & other) const { return (this->data != other.data) || (this->offset != other.offset); }
        bool operator* () const { return this->data[this->offset>>3]&(1<<(7-(offset&0x7))); }
        const Iterator & operator++ () { ++this->offset; return *this; }
    };

    explicit BitArray(size_t width_in_bits, const uint8_t * data) : width_in_bits(width_in_bits), data(data)  {}

    [[nodiscard]] Iterator begin () const { return Iterator(this->data, 0); }
    [[nodiscard]] Iterator end () const { return Iterator(this->data, this->width_in_bits); }
};


struct DrawablePointer
{
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
    };

    unsigned width { 0 };
    unsigned height { 0 };

    uint8_t data[MAX_WIDTH * MAX_HEIGHT * 4]; // 96 pixels per line * 96 lines * 4 bytes per pixel
    uint8_t mask[MAX_WIDTH * MAX_HEIGHT * 3]; // 96 pixels per line * 96 lines * 3 bytes per pixel

    ImageView image_data_view_data;
    ImageView image_data_view_mask;

    DrawablePointer() :
        image_data_view_data(create_img(nullptr, 0)),
        image_data_view_mask(create_img(nullptr, 0))
    {}

    DrawablePointer(Pointer const& cursor) :
        DrawablePointer()
    {
        this->set_cursor(cursor);
    }

    void set_cursor(Pointer const& cursor)
    {
        const auto dim = cursor.get_dimensions();

        this->width = dim.width;
        this->height = dim.height;

        const BitsPerPixel bits_per_pixel = cursor.get_native_xor_bpp();
        auto pointer_data = cursor.get_native_xor_mask();

        switch (bits_per_pixel) {
            case BitsPerPixel::BitsPP1: {
                const unsigned line_bytes = ::even_pad_length(::nbbytes(dim.width));
                const unsigned h = dim.height;
                auto* dest = this->data;
                for (unsigned y = 0; y < h; ++y) {
                    const uint8_t* src = pointer_data.data() + y * line_bytes;

                    unsigned char bit_count = 7;
                    const uint8_t* enddest = dest + dim.width*3;
                    while (dest < enddest) {
                        uint8_t pixel = (*src & (1 << bit_count)) ? 0xFF : 0;
                        *dest++ = pixel;
                        *dest++ = pixel;
                        *dest++ = pixel;

                        if (bit_count == 0) {
                            ++src;
                        }

                        bit_count = (bit_count - 1) & 7;
                    }
                }

                this->image_data_view_data = this->create_img(this->data,
                                                              this->width * 3,
                                                              BytesPerPixel(3));
                break;
            }

            case BitsPerPixel::BitsPP4: {
                const unsigned line_bytes = ::even_pad_length(::nbbytes(dim.width * 4));
                const unsigned h = dim.height;
                auto& palette = BGRPalette::classic_332();
                auto* dest = this->data;
                for (unsigned y = 0; y < h; ++y) {
                    const uint8_t* src = pointer_data.data() + y * line_bytes;

                    unsigned char bit_count = 7;
                    const uint8_t* enddest = dest + dim.width*3;
                    while (dest < enddest) {
                        BGRColor pixel1 = palette[*src >> 4];

                        *dest++ = pixel1.red();
                        *dest++ = pixel1.green();
                        *dest++ = pixel1.blue();

                        bit_count = (bit_count - 1) & 7;

                        BGRColor pixel2 = palette[*src & 0xf];

                        *dest++ = pixel2.red();
                        *dest++ = pixel2.green();
                        *dest++ = pixel2.blue();

                        bit_count = (bit_count - 1) & 7;

                        ++src;
                    }
                }

                this->image_data_view_data = this->create_img(this->data,
                                                              this->width * 3,
                                                              BytesPerPixel(3));
                break;
            }

            case BitsPerPixel::BitsPP8:
            case BitsPerPixel::BitsPP15:
            case BitsPerPixel::BitsPP16:
            case BitsPerPixel::BitsPP24:
            case BitsPerPixel::BitsPP32: {
                auto bytes_per_pixel = BytesPerPixel(nbbytes(underlying_cast(bits_per_pixel)));
                unsigned line_bytes = ::even_pad_length(
                    this->width * underlying_cast(bytes_per_pixel));

                ::memcpy(this->data, pointer_data.data(), pointer_data.size());
                this->image_data_view_data = this->create_img(this->data,
                                                              line_bytes,
                                                              bytes_per_pixel);
                break;
            }

            case BitsPerPixel::Unspecified:
                break;
        }

        const uint8_t* pointer_mask = cursor.get_monochrome_and_mask().data();
        const unsigned int mask_line_bytes = ::even_pad_length(::nbbytes(this->width));

        for (unsigned int y = 0; y < this->height; ++y)
        {
            for (unsigned int x = 0; x < this->width; ++x)
            {
                ::put_pixel_24bpp(this->mask,
                                  line_bytes,
                                  x,
                                  y,
                                  ::get_pixel_1bpp(pointer_mask,
                                                   mask_line_bytes,
                                                   x,
                                                   y) ? 0xFFFFFF : 0);
            }
        }

        /* xorMask doesn't contain alpha channel info,
           so we will skip the 4th byte on each pixel
           on reading with BytesPerPixel{3} rather than BytesPerPixel{4} */
        this->image_data_view_mask = this->create_img(this->mask, line_bytes);
    }

private:
    ImageView create_img(uint8_t const* data,
                         unsigned line_bytes,
                         BytesPerPixel bytes_per_pixel = BytesPerPixel{3}) const noexcept
    {
        return ImageView(
            data,
            this->width,
            this->height,
            line_bytes,
            bytes_per_pixel,
            ImageView::Storage::BottomToTop,
            &BGRPalette::classic_332()
        );
    }
};  // struct DrawablePointer
