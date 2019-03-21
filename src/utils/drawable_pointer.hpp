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

    Iterator begin () const { return Iterator(this->data, this->width_in_bytes); }
    Iterator end () const { return Iterator(&this->data[this->width_in_bytes * this->height], this->width_in_bytes); }
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

    Iterator begin () const { return Iterator(this->data); }
    Iterator end () const { return Iterator(this->data + this->width_in_bytes); }
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

    Iterator begin () const { return Iterator(this->data, this->width_in_bits, 0); }
    Iterator end () const { return Iterator(this->data, this->width_in_bits, this->width_in_bits); }
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

    Iterator begin () const { return Iterator(this->data, 0); }
    Iterator end () const { return Iterator(this->data, this->width_in_bits); }
};


struct DrawablePointer {
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
    };

    explicit DrawablePointer() = default;

    unsigned width { 0 };
    unsigned height { 0 };

    unsigned line_bytes { 0 };

    uint8_t data[MAX_WIDTH * MAX_HEIGHT * 3] {}; // 96 pixels per line * 96 lines * 3 bytes per pixel
    uint8_t mask24[MAX_WIDTH * MAX_HEIGHT * 3] {}; // 96 pixels per line * 96 lines * 3 bytes per pixel

    std::unique_ptr<ConstImageDataView> image_data_view_data;
    std::unique_ptr<ConstImageDataView> image_data_view_mask24;

    void initialize(unsigned int width_, unsigned int height_, unsigned int line_bytes_, unsigned int mask_line_bytes_,
                    const uint8_t * pointer_data, const uint8_t * pointer_mask) {
        this->width  = width_;
        this->height = height_;

        this->line_bytes      = line_bytes_;

        ::memcpy(this->data, pointer_data, this->line_bytes * this->height);

        this->image_data_view_data = std::make_unique<ConstImageDataView>(
                this->data,
                width_,
                height_,
                line_bytes_,
                BytesPerPixel{3},
                ConstImageDataView::Storage::BottomToTop
            );

        for (unsigned int y = 0; y < width_; ++y) {
            for (unsigned int x = 0; x < height_; ++x) {
                ::put_pixel_24bpp(this->mask24, line_bytes_, x, y, (::get_pixel_1bpp(pointer_mask, mask_line_bytes_, x, y) ? 0xFFFFFF : 0));
            }
        }
        this->image_data_view_mask24 = std::make_unique<ConstImageDataView>(
                this->mask24,
                width_,
                height_,
                line_bytes_,
                BytesPerPixel{3},
                ConstImageDataView::Storage::BottomToTop
            );
    }
};  // struct DrawablePointer

