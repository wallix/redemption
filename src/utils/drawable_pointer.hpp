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
        Iterator (const uint8_t * data, size_t nbbytes) : data(data), nbbytes(nbbytes) {}
        bool operator!= (const Iterator & other) const { return this->data != other.data; }
        const uint8_t * operator* () const { return this->data; }
        const Iterator & operator++ () { this->data += this->nbbytes; return *this; }
    };
    
    public:
    Array2D(size_t width_in_bytes, size_t height, const uint8_t * data) :
    width_in_bytes(width_in_bytes), height(height), data(data)
    {}
 
    Iterator begin () const { return Iterator(this->data, this->width_in_bytes); }
    Iterator end () const { return Iterator(&this->data[this->width_in_bytes * this->height], this->width_in_bytes); }
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
        Iterator (const uint8_t * data, size_t offset) : data(data), offset(offset) {}
        bool operator!= (const Iterator & other) const { return (this->data != other.data) || (this->offset != other.offset); }
        bool operator* () const { return this->data[this->offset>>3]&(1<<(7-(offset&0x7))); }
        const Iterator & operator++ () { ++this->offset; return *this; }
    };
    
    BitArray(size_t width_in_bits, const uint8_t * data) : width_in_bits(width_in_bits), data(data)  {}
 
    Iterator begin () const { return Iterator(this->data, 0); }
    Iterator end () const { return Iterator(this->data, this->width_in_bits); }
};


struct DrawablePointer {
    enum {
          MAX_WIDTH  = 96
        , MAX_HEIGHT = 96
    };

    struct ContiguousPixels {
        int             x;
        int             y;
        uint8_t         data_size;
        const uint8_t * data;
    };

    ContiguousPixels contiguous_pixels[MAX_WIDTH / 2 * MAX_HEIGHT];    // MAX_WIDTH / 2 contiguous pixels per line * MAX_HEIGHT lines
    uint8_t          number_of_contiguous_pixels;

    uint8_t data[MAX_WIDTH * MAX_HEIGHT * 3];  // 32 pixels per line * 32 lines * 3 bytes per pixel

    DrawablePointer() : contiguous_pixels(), number_of_contiguous_pixels(0), data() {}

    void initialize(unsigned int width, unsigned int height, const uint8_t * pointer_data, const uint8_t * pointer_mask) {
        ::memset(this->contiguous_pixels, 0, sizeof(this->contiguous_pixels));
        this->number_of_contiguous_pixels = 0;
        ::memset(this->data, 0, sizeof(this->data));

        ContiguousPixels * current_contiguous_pixels  = this->contiguous_pixels;

//        Array2D a2d(::nbbytes(width), height, pointer_mask);
//        for (auto x : a2d){
//            BitArray ba(width, x);
//            for (auto bit : ba){
//                printf("%s", bit?"1":"0");
//            }
//            printf("\n");
//        }

        const unsigned int remainder = (width % 8);
        const unsigned int and_line_length_in_byte = width / 8 + (remainder ? 1 : 0);
        const unsigned int and_padded_line_length_in_byte =
            ((and_line_length_in_byte % 2) ?
             and_line_length_in_byte + 1 :
             and_line_length_in_byte);

        for (unsigned int line = 0; line < height; line++) {
            bool in_contiguous_mouse_pixels = false;

            for (unsigned int column = 0; column < width; column++) {
                const div_t        res = div(column, 8);
                const unsigned int rem = 7 - res.rem;

                bool non_transparent_pixel = !(((*(pointer_mask + and_padded_line_length_in_byte * (height - (line + 1)) + res.quot)) & (1 << rem)) >> rem);
                if (non_transparent_pixel && !in_contiguous_mouse_pixels) {
                    this->number_of_contiguous_pixels++;
                    current_contiguous_pixels->x         = column;
                    current_contiguous_pixels->y         = line;
                    current_contiguous_pixels->data_size = 0;
                    current_contiguous_pixels++;
                    in_contiguous_mouse_pixels = true;
                }
                else if (!non_transparent_pixel && in_contiguous_mouse_pixels) {
                    in_contiguous_mouse_pixels = false;
                }

                if (in_contiguous_mouse_pixels) {
                    (current_contiguous_pixels-1)->data_size += 3;
                }
            }
        }

        const unsigned int xor_line_length_in_byte = width * 3;
        const unsigned int xor_padded_line_length_in_byte =
            ((xor_line_length_in_byte % 2) ?
             xor_line_length_in_byte + 1 :
             xor_line_length_in_byte);

        uint8_t * current_data = this->data;
        for (size_t count = 0 ; count < this->number_of_contiguous_pixels ; count++) {
            ContiguousPixels & block = this->contiguous_pixels[count];
              const uint8_t * pixel = pointer_data + xor_padded_line_length_in_byte * (height - (block.y + 1)) + block.x * 3;
            ::memcpy(current_data, pixel, block.data_size);
            this->contiguous_pixels[count].data = current_data;
            current_data += block.data_size;
        }
        //hexdump_c(pointer_mask, 128);
    }

    struct ContiguousPixelsView {
        DrawablePointer::ContiguousPixels const * first;
        DrawablePointer::ContiguousPixels const * last;
        DrawablePointer::ContiguousPixels const * begin() const noexcept { return this->first; }
        DrawablePointer::ContiguousPixels const * end() const noexcept { return this->last; }
    };

    ContiguousPixelsView contiguous_pixels_view() const {
        return {this->contiguous_pixels + 0, this->contiguous_pixels + this->number_of_contiguous_pixels};
    }
};  // struct DrawablePointer

