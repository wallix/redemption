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
#include "utils/image_data_view.hpp"
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

    unsigned line_bytes { 0 };

    uint8_t data[MAX_WIDTH * MAX_HEIGHT * 3]; // 96 pixels per line * 96 lines * 3 bytes per pixel
    uint8_t mask24[MAX_WIDTH * MAX_HEIGHT * 3]; // 96 pixels per line * 96 lines * 3 bytes per pixel
    uint8_t data32[MAX_WIDTH * MAX_HEIGHT * 4]; // 96x96 RGBA

    ConstImageDataView image_data_view_data;
    ConstImageDataView image_data_view_mask24;
    ConstImageDataView image_data_view_mask32;
    BitsPerPixel bpp {24};

    DrawablePointer()
    : image_data_view_data(create_img(nullptr))
    , image_data_view_mask24(create_img(nullptr))
    , image_data_view_mask32(create_img(nullptr, BytesPerPixel(4)))
    {}

    DrawablePointer(Pointer const& cursor)
    : DrawablePointer()
    {
        this->set_cursor(cursor);
    }

    void set_cursor(Pointer const& cursor)
    {
        const auto dim = cursor.get_dimensions();
        this->width = dim.width;
        this->height = dim.height;
        this->bpp = cursor.get_bpp();

        const uint8_t* pointer_mask = cursor.get_monochrome_and_mask().data();
        const unsigned int mask_line_bytes = ::even_pad_length(::nbbytes(this->width));
        const uint8_t *dataSrc = cursor.get_24bits_xor_mask().data();
        this->line_bytes = ::even_pad_length(this->width * 3);

        ::memcpy(this->data, dataSrc, this->line_bytes * this->height);
        this->image_data_view_data = this->create_img(this->data);

        uint32_t srcSteps = 3;
        uint32_t srcStride = this->width * 3;

        if (cursor.haveRdpPointer && cursor.original_bpp == BitsPerPixel(32)) {
            srcSteps = 4;
            srcStride = this->width * 4;
            dataSrc = cursor.get_original_data();
        }

        for (unsigned int y = 0; y < this->width; ++y) {
            for (unsigned int x = 0; x < this->height; ++x) {
                uint32_t mask = ::get_pixel_1bpp(pointer_mask, mask_line_bytes, x, y) ? 0xFFFFFFFF : 0x00000000;
                uint32_t value = dataSrc[y * srcStride + x * srcSteps];
                ::put_pixel_32bpp(this->data32, this->line_bytes, x, y, value & mask);
            }
        }

        this->image_data_view_mask32 = this->create_img(this->data32, BitsPerPixel(32));
    }

private:
    ConstImageDataView create_img(uint8_t const* data, BitsOrBytePerPixel depth = BitsPerPixel(24)) const
    {
        return ConstImageDataView(
            data,
            this->width,
            this->height,
            this->line_bytes,
            depth,
            ConstImageDataView::Storage::BottomToTop
        );
    }
};  // struct DrawablePointer
