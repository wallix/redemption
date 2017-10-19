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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Martin Potier,
              Meng Tan, Clement Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   This file implement the bitmap items data structure
   including RDP RLE compression and decompression algorithms

   It also features storage and color versionning of the bitmap
   returning a pointer on a table, corresponding to the required
   color model.
*/

#include "utils/bitmap.hpp"
#include "utils/bitmap_private_data.hpp"
#include "utils/bitmap_data_allocator.hpp"

#include "utils/log.hpp"
#include "utils/bitfu.hpp"
#include "utils/colors.hpp"
#include "utils/stream.hpp"
#include "utils/rect.hpp"
#include "utils/bitmap_data_allocator.hpp"
#include "utils/sugar/array_view.hpp"

#include "cxx/cxx.hpp"

#include "system/ssl_sha1.hpp"

#include <cstring>

#include <cassert>
#include <utility>
#include <type_traits> // aligned_storage


namespace aux_
{
    BmpMemAlloc bitmap_data_allocator;
}

Bitmap::Bitmap(Bitmap && bmp) noexcept
: data_bitmap(bmp.data_bitmap)
{
    bmp.data_bitmap = nullptr;
}

Bitmap::Bitmap(const Bitmap & other) noexcept
: data_bitmap(other.data_bitmap)
{
    // TODO used a default DataBitmap instead of nullptr
    if (this->data_bitmap) {
        this->data_bitmap->inc();
    }
}

Bitmap::~Bitmap()
{
    this->reset();
}

Bitmap & Bitmap::operator=(const Bitmap & other) noexcept
{
    other.data_bitmap->inc();
    this->reset();
    this->data_bitmap = other.data_bitmap;
    return *this;
}

Bitmap & Bitmap::operator=(Bitmap && other) noexcept
{
    auto other_data = other.data_bitmap;
    other.data_bitmap = nullptr;
    this->data_bitmap = other_data;
    return *this;
}

bool Bitmap::is_valid() const noexcept
{
    return this->data_bitmap;
}

void Bitmap::reset() noexcept
{
    if (this->data_bitmap) {
        this->data_bitmap->dec();
        if (this->data_bitmap->count() == 0) {
            DataBitmap::destruct(this->data_bitmap);
        }
        this->data_bitmap = nullptr;
    }
}

void Bitmap::swap(Bitmap & other) noexcept
{
    using std::swap;
    swap(this->data_bitmap, other.data_bitmap);
}

// TODO
#include "utils/bitmap_from_rle.hpp"

Bitmap::Bitmap(
    uint8_t session_color_depth, uint8_t bpp, const BGRPalette * palette,
    uint16_t cx, uint16_t cy, const uint8_t * data, const size_t size,
    bool compressed)
: data_bitmap(DataBitmap::construct(bpp, cx, cy))
{
    if (cx <= 0 || cy <= 0){
        LOG(LOG_ERR, "Bogus empty bitmap!!! cx=%u cy=%u size=%zu bpp=%u", cx, cy, size, bpp);
    }
    if (bpp == 8){
        if (palette){
            this->data_bitmap->palette() = *palette;
        }
        else {
            this->data_bitmap->palette() = BGRPalette::classic_332();
        }
    }
    //LOG(LOG_INFO, "Creating bitmap (%p) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, size, bpp);
    if (compressed) {
        this->data_bitmap->copy_compressed_buffer(data, size);
        if ((session_color_depth == 32) && ((bpp == 24) || (bpp == 32))) {
            ::decompress60(*this->data_bitmap, cx, cy, data, size);
        }
        else {
            ::decompress(*this->data_bitmap, data, cx, cy, size);
        }
    } else {
        uint8_t * dest = this->data_bitmap->get();
        const uint8_t * src = data;
        const size_t data_width = cx * nbbytes(bpp);
        const size_t line_size = this->line_size();
        const uint16_t cy = this->cy();
        for (uint16_t i = 0; i < cy ; i++){
            memcpy(dest, src, data_width);
            memset(dest + data_width, 0, line_size - data_width);
            src += data_width;
            dest += line_size;
        }
    }
}

Bitmap::Bitmap(const Bitmap & src_bmp, const Rect r)
: data_bitmap(src_bmp.data_bitmap)
{
    //LOG(LOG_INFO, "Creating bitmap (%p) extracting part cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, bpp);
    if (0 == r.x && 0 == r.y && r.cx == src_bmp.cx() && r.cy == src_bmp.cy()) {
        this->data_bitmap->inc();
        return ;
    }
    this->data_bitmap = DataBitmap::construct(src_bmp.bpp(), r.cx, r.cy);
    if (this->bpp() == 8) {
        this->data_bitmap->palette() = src_bmp.data_bitmap->palette();
    }
    // bitmapDataStream (variable): A variable-sized array of bytes.
    //  Uncompressed bitmap data represents a bitmap as a bottom-up,
    //  left-to-right series of pixels. Each pixel is a whole
    //  number of bytes. Each row contains a multiple of four bytes
    // (including up to three bytes of padding, as necessary).
    // In redemption we ensure a more constraint restriction to avoid padding
    // bitmap width must always be a multiple of 4
    const uint8_t Bpp = nbbytes(this->bpp());
    uint8_t *dest = this->data_bitmap->get();
    const size_t line_size = this->line_size();
    const size_t src_line_size = src_bmp.line_size();
    const uint16_t cy = this->cy();
    const uint16_t src_cy = src_bmp.cy();
    const uint8_t *src = src_bmp.data_bitmap->get() + src_line_size * (src_cy - r.y - cy) + r.x * Bpp;
    const unsigned line_to_copy = r.cx * Bpp;
    for (uint16_t i = 0; i < cy; i++) {
        memcpy(dest, src, line_to_copy);
        if (line_to_copy < line_size){
            memset(dest + line_to_copy, 0, line_size - line_to_copy);
        }
        src += src_line_size;
        dest += line_size;
    }
}

Bitmap::Bitmap(const uint8_t * vnc_raw, uint16_t vnc_cx, uint16_t /*vnc_cy*/, uint8_t vnc_bpp, const Rect tile)
: data_bitmap(DataBitmap::construct(vnc_bpp, tile.cx, tile.cy))
{
    //LOG(LOG_INFO, "Creating bitmap (%p) extracting part cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, bpp);
    // raw: vnc data is a bunch of pixels of size cx * cy * nbbytes(bpp)
    // line 0 is the first line (top-up)
    // bitmapDataStream (variable): A variable-sized array of bytes.
    //  Uncompressed bitmap data represents a bitmap as a bottom-up,
    //  left-to-right series of pixels. Each pixel is a whole
    //  number of bytes. Each row contains a multiple of four bytes
    // (including up to three bytes of padding, as necessary).
    const uint8_t Bpp = nbbytes(this->bpp());
    const unsigned src_row_size = vnc_cx * Bpp;
    uint8_t *dest = this->data_bitmap->get();
    const uint8_t *src = vnc_raw + src_row_size * (tile.y + tile.cy - 1) + tile.x * Bpp;
    const uint16_t line_to_copy_size = tile.cx * Bpp;
    const size_t line_size = this->line_size();
    const uint16_t cy = this->cy();
    for (uint16_t i = 0; i < cy; i++) {
        memcpy(dest, src, line_to_copy_size);
        if (line_to_copy_size < line_size){
            memset(dest + line_to_copy_size, 0, line_size - line_to_copy_size);
        }
        src -= src_row_size;
        dest += line_size;
    }
}

const uint8_t* Bitmap::data() const noexcept
{
    return this->data_bitmap->get();
}

const BGRPalette & Bitmap::palette() const noexcept
{
    return this->data_bitmap->palette();
}

uint16_t Bitmap::cx() const noexcept
{
    return this->data_bitmap->cx();
}

uint16_t Bitmap::cy() const noexcept
{
    return this->data_bitmap->cy();
}

size_t Bitmap::line_size() const noexcept
{
    return this->data_bitmap->line_size();
}

uint8_t Bitmap::bpp() const noexcept
{
    return this->data_bitmap->bpp();
}

size_t Bitmap::bmp_size() const noexcept
{
    return this->data_bitmap->bmp_size();
}

array_view<uint8_t const> Bitmap::data_compressed() const noexcept
{
    return {this->data_bitmap->compressed_data(), this->data_bitmap->compressed_size()};
}

bool Bitmap::has_data_compressed() const noexcept
{
    return this->data_bitmap->compressed_size();
}

unsigned Bitmap::get_pixel(const uint8_t Bpp, const uint8_t * const p)
{
    return in_uint32_from_nb_bytes_le(Bpp, p);
}
unsigned Bitmap::get_pixel_above(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * const p) const
{
    return ((p-this->line_size()) < pmin)
    ? 0
    : this->get_pixel(Bpp, p - this->line_size());
}
unsigned Bitmap::get_color_count(const uint8_t Bpp, const uint8_t * pmax, const uint8_t * p, unsigned color) const
{
    unsigned acc = 0;
    while (p < pmax && this->get_pixel(Bpp, p) == color){
        acc++;
        p = p + Bpp;
    }
    return acc;
}
unsigned Bitmap::get_bicolor_count(const uint8_t Bpp, const uint8_t * pmax, const uint8_t * p, unsigned color1, unsigned color2) const
{
    unsigned acc = 0;
    while ((p < pmax)
        && (color1 == this->get_pixel(Bpp, p))
        && (p + Bpp < pmax)
        && (color2 == this->get_pixel(Bpp, p + Bpp))) {
            acc = acc + 2;
            p = p + 2 * Bpp;
    }
    return acc;
}
unsigned Bitmap::get_fill_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p) const
{
    unsigned acc = 0;
    while  (p + Bpp <= pmax) {
        unsigned pixel = this->get_pixel(Bpp, p);
        unsigned ypixel = this->get_pixel_above(Bpp, pmin, p);
        if (ypixel != pixel){
            break;
        }
        p += Bpp;
        acc += 1;
    }
    return acc;
}
unsigned Bitmap::get_mix_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground) const
{
    unsigned acc = 0;
    while (p + Bpp <= pmax){
        if (this->get_pixel_above(Bpp, pmin, p) ^ foreground ^ this->get_pixel(Bpp, p)){
            break;
        }
        p += Bpp;
        acc += 1;
    }
    return acc;
}
unsigned Bitmap::get_fom_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground, bool fill) const
{
    unsigned acc = 0;
    while (true){
        unsigned count = 0;
        while  (p + Bpp <= pmax) {
            unsigned pixel = this->get_pixel(Bpp, p);
            unsigned ypixel = this->get_pixel_above(Bpp, pmin, p);
            if (ypixel ^ pixel ^ (fill?0:foreground)){
                break;
            }
            p += Bpp;
            count += 1;
            if (count >= 9) {
                return acc;
            }
        }
        if (!count){
            break;
        }
        acc += count;
        fill ^= true;
    }
    return acc;
}
void Bitmap::get_fom_masks(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * p, uint8_t * mask, const unsigned count) const
{
    unsigned i = 0;
    for (i = 0; i < count; i += 8)
    {
        mask[i>>3] = 0;
    }
    for (i = 0 ; i < count; i++, p += Bpp)
    {
        if (get_pixel(Bpp, p) != get_pixel_above(Bpp, pmin, p)){
            mask[i>>3] |= static_cast<uint8_t>(0x01 << (i & 7));
        }
    }
}
unsigned Bitmap::get_fom_count_set(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned & foreground, unsigned & flags) const
{
    // flags : 1 = fill, 2 = MIX, 3 = (1+2) = FOM
    flags = FLAG_FILL;
    unsigned fill_count = this->get_fill_count(Bpp, pmin, pmax, p);
    if (fill_count) {
        if (fill_count < 8) {
            unsigned fom_count = this->get_fom_count(Bpp, pmin, pmax, p + fill_count * Bpp, foreground, false);
            if (fom_count){
                flags = FLAG_FOM;
                fill_count += fom_count;
            }
        }
        return fill_count;
    }
    // fill_count and mix_count can't match at the same time.
    // this would mean that foreground is black, and we will never set
    // it to black, as it's useless because fill_count allready does that.
    // Hence it's ok to check them independently.
    if  (p + Bpp <= pmax) {
        flags = FLAG_MIX;
        // if there is a pixel we are always able to mix (at worse we will set foreground ourself)
        foreground = this->get_pixel_above(Bpp, pmin, p) ^ this->get_pixel(Bpp, p);
        unsigned mix_count = 1 + this->get_mix_count(Bpp, pmin, pmax, p + Bpp, foreground);
        if (mix_count < 8) {
            unsigned fom_count = 0;
            fom_count = this->get_fom_count(Bpp, pmin, pmax, p + mix_count * Bpp, foreground, true);
            if (fom_count){
                flags = FLAG_FOM;
                mix_count += fom_count;
            }
        }
        return mix_count;
    }
    flags = FLAG_NONE;
    return 0;
}

template<class TBpp>
void Bitmap::compress_(TBpp Bpp, OutStream & outbuffer) const
{
    struct RLE_OutStream {
        OutStream & stream;
        explicit RLE_OutStream(OutStream & outbuffer)
        : stream(outbuffer)
        {}
        // =========================================================================
        // Helper methods for RDP RLE bitmap compression support
        // =========================================================================
        void out_count(const int in_count, const int mask){
            if (in_count < 32) {
                this->stream.out_uint8(static_cast<uint8_t>((mask << 5) | in_count));
            }
            else if (in_count < 256 + 32){
                this->stream.out_uint8(static_cast<uint8_t>(mask << 5));
                this->stream.out_uint8(static_cast<uint8_t>(in_count - 32));
            }
            else {
                this->stream.out_uint8(static_cast<uint8_t>(0xf0 | mask));
                this->stream.out_uint16_le(in_count);
            }
        }
        // Background Run Orders
        // ~~~~~~~~~~~~~~~~~~~~~
        // A Background Run Order encodes a run of pixels where each pixel in the
        // run matches the uncompressed pixel on the previous scanline. If there is
        // no previous scanline then each pixel in the run MUST be black.
        // When encountering back-to-back background runs, the decompressor MUST
        // write a one-pixel foreground run to the destination buffer before
        // processing the second background run if both runs occur on the first
        // scanline or after the first scanline (if the first run is on the first
        // scanline, and the second run is on the second scanline, then a one-pixel
        // foreground run MUST NOT be written to the destination buffer). This
        // one-pixel foreground run is counted in the length of the run.
        // The run length encodes the number of pixels in the run. There is no data
        // associated with Background Run Orders.
        // +-----------------------+-----------------------------------------------+
        // | 0x0 REGULAR_BG_RUN    | The compression order encodes a regular-form  |
        // |                       | background run. The run length is stored in   |
        // |                       | the five low-order bits of  the order header  |
        // |                       | byte. If this value is zero, then the run     |
        // |                       | length is encoded in the byte following the   |
        // |                       | order header and MUST be incremented by 32 to |
        // |                       | give the final value.                         |
        // +-----------------------+-----------------------------------------------+
        // | 0xF0 MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA     |
        // |                       | background run. The run length is stored in   |
        // |                       | the two bytes following the order header      |
        // |                       | (in little-endian format).                    |
        // +-----------------------+-----------------------------------------------+
        void out_fill_count(const int in_count)
        {
            this->out_count(in_count, 0x00);
        }
        // Foreground Run Orders
        // ~~~~~~~~~~~~~~~~~~~~~
        // A Foreground Run Order encodes a run of pixels where each pixel in the
        // run matches the uncompressed pixel on the previous scanline XOR’ed with
        // the current foreground color. If there is no previous scanline, then
        // each pixel in the run MUST be set to the current foreground color (the
        // initial foreground color is white).
        // The run length encodes the number of pixels in the run.
        // If the order is a "set" variant, then in addition to encoding a run of
        // pixels, the order also encodes a new foreground color (in little-endian
        // format) in the bytes following the optional run length. The current
        // foreground color MUST be updated with the new value before writing
        // the run to the destination buffer.
        // +---------------------------+-------------------------------------------+
        // | 0x1 REGULAR_FG_RUN        | The compression order encodes a           |
        // |                           | regular-form foreground run. The run      |
        // |                           | length is stored in the five low-order    |
        // |                           | bits of the order header byte. If this    |
        // |                           | value is zero, then the run length is     |
        // |                           | encoded in the byte following the order   |
        // |                           | header and MUST be incremented by 32 to   |
        // |                           | give the final value.                     |
        // +---------------------------+-------------------------------------------+
        // | 0xF1 MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA |
        // |                           | foreground run. The run length is stored  |
        // |                           | in the two bytes following the order      |
        // |                           | header (in little-endian format).         |
        // +---------------------------+-------------------------------------------+
        // | 0xC LITE_SET_FG_FG_RUN    | The compression order encodes a "set"     |
        // |                           | variant lite-form foreground run. The run |
        // |                           | length is stored in the four low-order    |
        // |                           | bits of the order header byte. If this    |
        // |                           | value is zero, then the run length is     |
        // |                           | encoded in the byte following the order   |
        // |                           | header and MUST be incremented by 16 to   |
        // |                           | give the final value.                     |
        // +---------------------------+-------------------------------------------+
        // | 0xF6 MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set"     |
        // |                           | variant MEGA_MEGA foreground run. The run |
        // |                           | length is stored in the two bytes         |
        // |                           | following the order header (in            |
        // |                           | little-endian format).                    |
        // +---------------------------+-------------------------------------------+
        void out_mix_count(const int in_count)
        {
            this->out_count(in_count, 0x01);
        }
        void out_mix_count_set(const int in_count, const TBpp Bpp, unsigned new_foreground)
        {
            const uint8_t mask = 0x06;
            if (in_count < 16) {
                this->stream.out_uint8(static_cast<uint8_t>(0xc0 | in_count));
            }
            else if (in_count < 256 + 16){
                this->stream.out_uint8(0xc0);
                this->stream.out_uint8(static_cast<uint8_t>(in_count - 16));
            }
            else {
                this->stream.out_uint8(0xf0 | mask);
                this->stream.out_uint16_le(in_count);
            }
            this->stream.out_bytes_le(Bpp, new_foreground);
        }
        // Foreground / Background Image Orders
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // A Foreground/Background Image Order encodes a binary image where each
        // pixel in the image that is not on the first scanline fulfils exactly one
        // of the following two properties:
        // (a) The pixel matches the uncompressed pixel on the previous scanline
        // XOR'ed with the current foreground color.
        // (b) The pixel matches the uncompressed pixel on the previous scanline.
        // If the pixel is on the first scanline then it fulfils exactly one of the
        // following two properties:
        // (c) The pixel is the current foreground color.
        // (d) The pixel is black.
        // The binary image is encoded as a sequence of byte-sized bitmasks which
        // follow the optional run length (the last bitmask in the sequence can be
        // smaller than one byte in size). If the order is a "set" variant then the
        // bitmasks MUST follow the bytes which specify the new foreground color.
        // Each bit in the encoded bitmask sequence represents one pixel in the
        // image. A bit that has a value of 1 represents a pixel that fulfils
        // either property (a) or (c), while a bit that has a value of 0 represents
        // a pixel that fulfils either property (b) or (d). The individual bitmasks
        // MUST each be processed from the low-order bit to the high-order bit.
        // The run length encodes the number of pixels in the run.
        // If the order is a "set" variant, then in addition to encoding a binary
        // image, the order also encodes a new foreground color (in little-endian
        // format) in the bytes following the optional run length. The current
        // foreground color MUST be updated with the new value before writing
        // the run to the destination buffer.
        // +--------------------------------+--------------------------------------+
        // | 0x2 REGULAR_FGBG_IMAGE         | The compression order encodes a      |
        // |                                | regular-form foreground/background   |
        // |                                | image. The run length is encoded in  |
        // |                                | the five low-order bits of the order |
        // |                                | header byte and MUST be multiplied   |
        // |                                | by 8 to give the final value. If     |
        // |                                | this value is zero, then the run     |
        // |                                | length is encoded in the byte        |
        // |                                | following the order header and MUST  |
        // |                                | be incremented by 1 to give the      |
        // |                                | final value.                         |
        // +--------------------------------+--------------------------------------+
        // | 0xF2 MEGA_MEGA_FGBG_IMAGE      | The compression order encodes a      |
        // |                                | MEGA_MEGA foreground/background      |
        // |                                | image. The run length is stored in   |
        // |                                | the two bytes following the order    |
        // |                                | header (in little-endian format).    |
        // +--------------------------------+--------------------------------------+
        // | 0xD LITE_SET_FG_FGBG_IMAGE     | The compression order encodes a      |
        // |                                | "set" variant lite-form              |
        // |                                | foreground/background image. The run |
        // |                                | length is encoded in the four        |
        // |                                | low-order bits of the order header   |
        // |                                | byte and MUST be multiplied by 8 to  |
        // |                                | give the final value. If this value  |
        // |                                | is zero, then the run length is      |
        // |                                | encoded in the byte following the    |
        // |                                | order header and MUST be incremented |
        // |                                | by 1 to give the final value.        |
        // +--------------------------------+--------------------------------------+
        // | 0xF7 MEGA_MEGA_SET_FGBG_IMAGE  | The compression order encodes a      |
        // |                                | "set" variant MEGA_MEGA              |
        // |                                | foreground/background image. The run |
        // |                                | length is stored in the two bytes    |
        // |                                | following the order header (in       |
        // |                                | little-endian format).               |
        // +-----------------------------------------------------------------------+
        void out_fom_count(const int in_count)
        {
            if (in_count < 256){
                if (in_count & 7){
                    this->stream.out_uint8(0x40);
                    this->stream.out_uint8(static_cast<uint8_t>(in_count - 1));
                }
                else{
                    this->stream.out_uint8(static_cast<uint8_t>(0x40 | (in_count >> 3)));
                }
            }
            else{
                this->stream.out_uint8(0xf2);
                this->stream.out_uint16_le(in_count);
            }
        }
        void out_fom_sequence(const int count, const uint8_t * masks) {
            this->out_fom_count(count);
            this->stream.out_copy_bytes(masks, nbbytes_large(count));
        }
        void out_fom_count_set(const int in_count)
        {
            if (in_count < 256){
                if (in_count & 0x87){
                    this->stream.out_uint8(0xD0);
                    this->stream.out_uint8(static_cast<uint8_t>(in_count - 1));
                }
                else{
                    this->stream.out_uint8(static_cast<uint8_t>(0xD0 | (in_count >> 3)));
                }
            }
            else{
                this->stream.out_uint8(0xf7);
                this->stream.out_uint16_le(in_count);
            }
        }
        void out_fom_sequence_set(const TBpp Bpp, const int count,
                                  const unsigned foreground, const uint8_t * masks) {
            this->out_fom_count_set(count);
            this->stream.out_bytes_le(Bpp, foreground);
            this->stream.out_copy_bytes(masks, nbbytes_large(count));
        }
        // Color Run Orders
        // ~~~~~~~~~~~~~~~~
        // A Color Run Order encodes a run of pixels where each pixel is the same
        // color. The color is encoded (in little-endian format) in the bytes
        // following the optional run length.
        // The run length encodes the number of pixels in the run.
        // +--------------------------+--------------------------------------------+
        // | 0x3 REGULAR_COLOR_RUN    | The compression order encodes a            |
        // |                          | regular-form color run. The run length is  |
        // |                          | stored in the five low-order bits of the   |
        // |                          | order header byte. If this value is zero,  |
        // |                          | then the run length is encoded in the byte |
        // |                          | following the order header and MUST be     |
        // |                          | incremented by 32 to give the final value. |
        // +--------------------------+--------------------------------------------+
        // | 0xF3 MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA  |
        // |                          | color run. The run length is stored in the |
        // |                          | two bytes following the order header (in   |
        // |                          | little-endian format).                     |
        // +--------------------------+--------------------------------------------+
        void out_color_sequence(const TBpp Bpp, const int count, const uint32_t color)
        {
            this->out_color_count(count);
            this->stream.out_bytes_le(Bpp, color);
        }
        void out_color_count(const int in_count)
        {
            this->out_count(in_count, 0x03);
        }
        // Color Image Orders
        // ~~~~~~~~~~~~~~~~~~
        // A Color Image Order encodes a run of uncompressed pixels.
        // The run length encodes the number of pixels in the run. So, to compute
        // the actual number of bytes which follow the optional run length, the run
        // length MUST be multiplied by the color depth (in bits-per-pixel) of the
        // bitmap data.
        // +-----------------------------+-----------------------------------------+
        // | 0x4 REGULAR_COLOR_IMAGE     | The compression order encodes a         |
        // |                             | regular-form color image. The run       |
        // |                             | length is stored in the five low-order  |
        // |                             | bits of the order header byte. If this  |
        // |                             | value is zero, then the run length is   |
        // |                             | encoded in the byte following the order |
        // |                             | header and MUST be incremented by 32 to |
        // |                             | give the final value.                   |
        // +-----------------------------+-----------------------------------------+
        // | 0xF4 MEGA_MEGA_COLOR_IMAGE  | The compression order encodes a         |
        // |                             | MEGA_MEGA color image. The run length   |
        // |                             | is stored in the two bytes following    |
        // |                             | the order header (in little-endian      |
        // |                             | format).                                |
        // +-----------------------------+-----------------------------------------+
        void out_copy_sequence(const TBpp Bpp, const int count, const uint8_t * data)
        {
            this->out_copy_count(count);
            this->stream.out_copy_bytes(data, count * Bpp);
        }
        void out_copy_count(const int in_count)
        {
            this->out_count(in_count, 0x04);
        }
        // Dithered Run Orders
        // ~~~~~~~~~~~~~~~~~~~
        // A Dithered Run Order encodes a run of pixels which is composed of two
        // alternating colors. The two colors are encoded (in little-endian format)
        // in the bytes following the optional run length.
        // The run length encodes the number of pixel-pairs in the run (not pixels).
        // +-----------------------------+-----------------------------------------+
        // | 0xE LITE_DITHERED_RUN       | The compression order encodes a         |
        // |                             | lite-form dithered run. The run length  |
        // |                             | is stored in the four low-order bits of |
        // |                             | the order header byte. If this value is |
        // |                             | zero, then the run length is encoded in |
        // |                             | the byte following the order header and |
        // |                             | MUST be incremented by 16 to give the   |
        // |                             | final value.                            |
        // +-----------------------------+-----------------------------------------+
        // | 0xF8 MEGA_MEGA_DITHERED_RUN | The compression order encodes a         |
        // |                             | MEGA_MEGA dithered run. The run length  |
        // |                             | is stored in the two bytes following    |
        // |                             | the order header (in little-endian      |
        // |                             | format).                                |
        // +-----------------------------+-----------------------------------------+
        void out_bicolor_sequence(const TBpp Bpp, const int count,
                                  const unsigned color1, const unsigned color2)
        {
            this->out_bicolor_count(count);
            this->stream.out_bytes_le(Bpp, color1);
            this->stream.out_bytes_le(Bpp, color2);
        }
        void out_bicolor_count(const int in_count)
        {
            const uint8_t mask = 0x08;
            if (in_count / 2 < 16){
                this->stream.out_uint8(static_cast<uint8_t>(0xe0 | (in_count / 2)));
            }
            else if (in_count / 2 < 256 + 16){
                this->stream.out_uint8(static_cast<uint8_t>(0xe0));
                this->stream.out_uint8(static_cast<uint8_t>(in_count / 2 - 16));
            }
            else{
                this->stream.out_uint8(0xf0 | mask);
                this->stream.out_uint16_le(in_count / 2);
            }
        }
    } out(outbuffer);
    uint8_t * tmp_data_compressed = out.stream.get_current();
    const uint8_t * pmin = this->data_bitmap->get();
    const uint8_t * p = pmin;
    // white with the right length : either 0xFF or 0xFFFF or 0xFFFFFF
    unsigned foreground = ~(-1u << (Bpp*8));
    unsigned new_foreground = foreground;
    unsigned flags = 0;
    uint8_t masks[512];
    unsigned copy_count = 0;
    const uint8_t * pmax = nullptr;
    uint32_t color = 0;
    uint32_t color2 = 0;
    const size_t bmp_size = this->bmp_size();
    const size_t align4_cx_bpp = align4(this->cx() * Bpp);
    for (int part = 0 ; part < 2 ; part++){
        // As far as I can see the specs of bitmap RLE compressor is crap here
        // Fill orders between first scanline and all others must be splitted
        // (or on windows RDP clients black pixels are inserted at beginning of line,
        // on rdesktop this corner case works just fine)...
        // but if the first scanline contains two successive FILL or
        // if all the remaining scanlines contains two consecutive fill
        // orders, a magic MIX pixel is inserted between fills.
        // This explains the surprising loop above and the test below.pp
        if (part){
            pmax = pmin + bmp_size;
        }
        else {
            pmax = pmin + align4_cx_bpp;
        }
        while (p < pmax)
        {
            uint32_t fom_count = this->get_fom_count_set(Bpp, pmin, pmax, p, new_foreground, flags);
            if (nbbytes_large(fom_count) > sizeof(masks)) {
                fom_count = sizeof(masks) * 8;
            }
            uint32_t color_count = 0;
            uint32_t bicolor_count = 0;
            if (p + Bpp < pmax){
                color = this->get_pixel(Bpp, p);
                color2 = this->get_pixel(Bpp, p + Bpp);
                if (color == color2){
                    color_count = this->get_color_count(Bpp, pmax, p, color);
                }
                else {
                    bicolor_count = this->get_bicolor_count(Bpp, pmax, p, color, color2);
                }
            }
            const unsigned fom_cost = 1                            // header
                + (foreground != new_foreground) * Bpp             // set
                + (flags == FLAG_FOM) * nbbytes_large(fom_count);  // mask
            const unsigned copy_fom_cost = 1 * (copy_count == 0) + fom_count * Bpp;     // pixels
            const unsigned color_cost = 1 + Bpp;
            const unsigned bicolor_cost = 1 + 2*Bpp;
            if ((fom_count >= color_count || (color_count == 0))
            && ((fom_count >= bicolor_count) || (bicolor_count < 4))
            && fom_cost < copy_fom_cost) {
                switch (flags){
                    case FLAG_FOM:
                        this->get_fom_masks(Bpp, pmin, p, masks, fom_count);
                        if (new_foreground != foreground){
                            flags = FLAG_FOM_SET;
                        }
                    break;
                    case FLAG_MIX:
                        if (new_foreground != foreground){
                            flags = FLAG_MIX_SET;
                        }
                    break;
                    default:
                    break;
                }
            }
            else {
                unsigned copy_color_cost = (copy_count == 0) + color_count * Bpp;       // copy + pixels
                unsigned copy_bicolor_cost = (copy_count == 0) + bicolor_count * Bpp;   // copy + pixels
                if ((color_cost < copy_color_cost) && (color_count > 0)){
                    flags = FLAG_COLOR;
                }
                else if ((bicolor_cost < copy_bicolor_cost) && (bicolor_count > 0)){
                    flags = FLAG_BICOLOR;
                }
                else {
                    flags = FLAG_NONE;
                    copy_count++;
                }
            }
            if (flags && copy_count > 0){
                out.out_copy_sequence(Bpp, copy_count, p - copy_count * Bpp);
                copy_count = 0;
            }
            switch (flags){
                case FLAG_BICOLOR:
                    out.out_bicolor_sequence(Bpp, bicolor_count, color, color2);
                    p+= bicolor_count * Bpp;
                break;
                case FLAG_COLOR:
                    out.out_color_sequence(Bpp, color_count, color);
                    p+= color_count * Bpp;
                break;
                case FLAG_FOM_SET:
                    out.out_fom_sequence_set(Bpp, fom_count, new_foreground, masks);
                    foreground = new_foreground;
                    p+= fom_count * Bpp;
                break;
                case FLAG_MIX_SET:
                    out.out_mix_count_set(fom_count, Bpp, new_foreground);
                    foreground = new_foreground;
                    p+= fom_count * Bpp;
                break;
                case FLAG_FOM:
                    out.out_fom_sequence(fom_count, masks);
                    p+= fom_count * Bpp;
                break;
                case FLAG_MIX:
                    out.out_mix_count(fom_count);
                    p+= fom_count * Bpp;
                break;
                case FLAG_FILL:
                    out.out_fill_count(fom_count);
                    p+= fom_count * Bpp;
                break;
                default: // copy, but wait until next good sequence before actual sending
                    p += Bpp;
                break;
            }
        }
        if (copy_count > 0){
            out.out_copy_sequence(Bpp, copy_count, p - copy_count * Bpp);
            copy_count = 0;
        }
    }
    // Memoize result of compression
    this->data_bitmap->copy_compressed_buffer(tmp_data_compressed, out.stream.get_current() - tmp_data_compressed);
}

void Bitmap::get_run(
    const uint8_t * data, uint16_t data_size, uint8_t last_raw, uint32_t & run_length,
    uint32_t & raw_bytes)
{
    const uint8_t * data_save = data;
    run_length = 0;
    raw_bytes  = 0;
    while (data_size) {
        raw_bytes++;
        data_size--;
        //LOG(LOG_INFO, "row_value=%c", *data);
        uint8_t last_raw_value = *(data++);
        for (; data_size && (*data == last_raw_value); run_length++, data_size--, data++)
            /*LOG(LOG_INFO, "run=%c", *data)*/;
        if (run_length >= 3) {
            break;
        }
        raw_bytes += run_length;
        run_length = 0;
    }
    if ((raw_bytes == 1) && run_length && (*data_save == last_raw)) {
        // [MS-RDPEGDI] Previous base value assumed to be 0.
        raw_bytes = 0;
        run_length++;
    }
    //LOG(LOG_INFO, "");
}

void Bitmap::compress_color_plane(uint16_t cx, uint16_t cy, OutStream & outbuffer, uint8_t * color_plane)
{
    //LOG(LOG_INFO, "compress_color_plane: cx=%u cy=%u", cx, cy);
    //hexdump_d(color_plane, cx * cy);
    uint16_t plane_line_size = cx * sizeof(uint8_t);
    // Converts to delta values.
    for (uint8_t * ypos_rbegin = color_plane + (cy - 1) * plane_line_size, * ypos_rend = color_plane;
         ypos_rbegin != ypos_rend; ypos_rbegin -= plane_line_size) {
        //LOG(LOG_INFO, "Line");
        for (uint8_t * xpos_begin = ypos_rbegin, * xpos_end = xpos_begin + plane_line_size;
             xpos_begin != xpos_end; xpos_begin += sizeof(uint8_t)) {
            //LOG(LOG_INFO, "delta=%d", *xpos_begin);
            int8_t delta = (
                  (  static_cast<int16_t>(*xpos_begin                    )
                   - static_cast<int16_t>(*(xpos_begin - plane_line_size)))
                & 0xFF
            );
            //LOG(LOG_INFO, "delta(1)=%d", delta);
            if (delta >= 0) {
                delta <<= 1;
            }
            else {
                delta = (((~delta + 1) << 1) - 1) & 0xFF;
                //LOG(LOG_INFO, "delta(2)=%d", delta);
            }
            *xpos_begin = static_cast<uint8_t>(delta);
        }
    }
    //LOG(LOG_INFO, "After delta conversion");
    //hexdump_d(color_plane, cx * cy);
    for (const uint8_t * ypos_begin = color_plane, * ypos_end = color_plane + cy * plane_line_size;
         ypos_begin != ypos_end; ypos_begin += plane_line_size) {
        uint16_t data_size = plane_line_size;
        //LOG(LOG_INFO, "Line");
        uint8_t  last_raw  = 0;
        for (const uint8_t * xpos = ypos_begin; data_size; ) {
            uint32_t run_length;
            uint32_t raw_bytes;
            get_run(xpos, data_size, last_raw, run_length, raw_bytes);
            //LOG(LOG_INFO, "run_length=%u raw_bytes=%u", run_length, raw_bytes);
            while (run_length || raw_bytes) {
                if ((run_length > 0) && (run_length < 3)) {
                    break;
                }
                if (!raw_bytes) {
                    if (run_length > 47) {
                        outbuffer.out_uint8((15                << 4) | 2         ); // Control byte
                        //LOG(LOG_INFO, "controlByte: (15, 2); rawValues: <none>");
                        xpos        += 47;
                        data_size   -= 47;
                        run_length  -= 47;
                    }
                    else if (run_length > 31) {
                        outbuffer.out_uint8(((run_length - 32) << 4) | 2         ); // Control byte
                        //LOG(LOG_INFO, "controlByte(1): (%d, 2); rawValues: <none>", run_length - 32);
                        xpos        += run_length;
                        data_size   -= run_length;
                        run_length  =  0;
                    }
                    else if (run_length > 15) {
                        outbuffer.out_uint8(((run_length - 16) << 4) | 1         ); // Control byte
                        //LOG(LOG_INFO, "controlByte(2): (%d, 1); rawValues: <none>", run_length - 16);
                        xpos        += run_length;
                        data_size   -= run_length;
                        run_length  =  0;
                    }
                    else {
                        outbuffer.out_uint8((0                 << 4) | run_length); // Control byte
                        //LOG(LOG_INFO, "controlByte(3): (0, %d); rawValues: <none>", run_length);
                        REDASSERT(!run_length || (run_length > 2));
                        xpos        += run_length;
                        data_size   -= run_length;
                        run_length  =  0;
                    }
                }
                else if (raw_bytes > 15) {
                    uint8_t rb[16];
                    memset(rb, 0, sizeof(rb));
                    memcpy(rb, xpos, 15);
                    outbuffer.out_uint8((15 << 4) | 0); // Control byte
                    //LOG(LOG_INFO, "controlByte(6): (15, 0); rawValues: %s", rb);
                    //hexdump_d(rb, 15);
                    outbuffer.out_copy_bytes(xpos, 15);
                    xpos        += 15;
                    data_size   -= 15;
                    raw_bytes   -= 15;
                }
                else/* if (raw_bytes < 16)*/ {
                    uint8_t rb[16];
                    memset(rb, 0, sizeof(rb));
                    memcpy(rb, xpos, raw_bytes);
                    if (run_length > 15) {
                        outbuffer.out_uint8((raw_bytes << 4) | 15        ); // Control byte
                        //LOG(LOG_INFO, "controlByte(4): (%d, 15); rawValues: %s", raw_bytes, rb);
                        //hexdump_d(rb, raw_bytes);
                        outbuffer.out_copy_bytes(xpos, raw_bytes);
                        xpos        += raw_bytes + 15;
                        data_size   -= raw_bytes + 15;
                        run_length  -= 15;
                        raw_bytes   =  0;
                    }
                    else {
                        outbuffer.out_uint8((raw_bytes << 4) | run_length); // Control byte
                        //LOG(LOG_INFO, "controlByte(5): (%d, %d); rawValues: %s", raw_bytes, run_length, rb);
                        //hexdump_d(rb, raw_bytes);
                        REDASSERT(!run_length || (run_length > 2));
                        outbuffer.out_copy_bytes(xpos, raw_bytes);
                        xpos        += raw_bytes + run_length;
                        data_size   -= raw_bytes + run_length;
                        run_length  = 0;
                        raw_bytes   = 0;
                    }
                }
            }
            last_raw = *(xpos - 1);
        }
    }
    //LOG(LOG_INFO, "compress_color_plane: exit");
}

void Bitmap::compress60(OutStream & outbuffer) const
{
    //LOG(LOG_INFO, "bmp compress60");
    REDASSERT((this->bpp() == 24) || (this->bpp() == 32));
    uint8_t * tmp_data_compressed = outbuffer.get_current();
    const uint16_t cx = this->cx();
    const uint16_t cy = this->cy();
    const uint32_t color_plane_size = sizeof(uint8_t) * cx * cy;
    struct Mem {
        void * p; ~Mem() { aux_::bitmap_data_allocator.dealloc(p); }
    } mem { aux_::bitmap_data_allocator.alloc(color_plane_size * 3) };
    uint8_t * mem_color   = static_cast<uint8_t *>(mem.p);
    uint8_t * red_plane   = mem_color + color_plane_size * 0;
    uint8_t * green_plane = mem_color + color_plane_size * 1;
    uint8_t * blue_plane  = mem_color + color_plane_size * 2;
    const uint8_t   byte_per_color = nbbytes(this->bpp());
    const uint8_t * data = this->data_bitmap->get();
    uint8_t * pixel_over_red_plane   = red_plane;
    uint8_t * pixel_over_green_plane = green_plane;
    uint8_t * pixel_over_blue_plane  = blue_plane;
    for (uint16_t y = 0; y < cy; y++) {
        for (uint16_t x = 0; x < cx; x++) {
            uint32_t pixel = in_uint32_from_nb_bytes_le(byte_per_color, data);
            uint8_t b =  ( pixel        & 0xFF);
            uint8_t g =  ((pixel >> 8 ) & 0xFF);
            uint8_t r =  ((pixel >> 16) & 0xFF);
            uint8_t a =  ((pixel >> 24) & 0xFF);
            (void)a;
            *(pixel_over_red_plane++)   = r;
            *(pixel_over_green_plane++) = g;
            *(pixel_over_blue_plane++)  = b;
            data += byte_per_color;
        }
    }
    /*
    REDASSERT(outbuffer.has_room(1 + color_plane_size * 3));
    outbuffer.out_uint8(
          (1 << 5)  // No alpha plane
        );
    outbuffer.out_copy_bytes(red_plane,   color_plane_size);
    outbuffer.out_copy_bytes(green_plane, color_plane_size);
    outbuffer.out_copy_bytes(blue_plane,  color_plane_size);
    outbuffer.out_uint8(0);
    */
    outbuffer.out_uint8(
          (1 << 5)  // No alpha plane
        | (1 << 4)  // RLE
        );
    this->compress_color_plane(cx, cy, outbuffer, red_plane);
    this->compress_color_plane(cx, cy, outbuffer, green_plane);
    this->compress_color_plane(cx, cy, outbuffer, blue_plane);
    // Memoize result of compression
    this->data_bitmap->copy_compressed_buffer(tmp_data_compressed, outbuffer.get_current() - tmp_data_compressed);
    //LOG(LOG_INFO, "data_compressedsize=%u", this->data_compressedsize);
    //LOG(LOG_INFO, "bmp compress60: done");
}

// TODO simplify and enhance compression using 1 pixel orders BLACK or WHITE.
void Bitmap::compress(uint8_t session_color_depth, OutStream & outbuffer) const
{
    if (this->data_bitmap->compressed_size()) {
        outbuffer.out_copy_bytes(this->data_bitmap->compressed_data(), this->data_bitmap->compressed_size());
        return;
    }
    if ((session_color_depth == 32) && ((this->bpp() == 24) || (this->bpp() == 32))) {
        return this->compress60(outbuffer);
    }
    switch (this->bpp()) {
        case 8 : return this->compress_(std::integral_constant<uint8_t, nbbytes(8)>{}, outbuffer);
        case 15: return this->compress_(std::integral_constant<uint8_t, nbbytes(15)>{}, outbuffer);
        case 16: return this->compress_(std::integral_constant<uint8_t, nbbytes(16)>{}, outbuffer);
        default: return this->compress_(std::integral_constant<uint8_t, nbbytes(24)>{}, outbuffer);
    }
}

void Bitmap::compute_sha1(uint8_t (&sig)[SslSha1::DIGEST_LENGTH]) const
{
    this->data_bitmap->copy_sha1(sig);
}

Bitmap::Bitmap(uint8_t out_bpp, const Bitmap & bmp)
{
    //LOG(LOG_INFO, "Creating bitmap (%p) (copy constructor) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, bpp);
    if (out_bpp != bmp.bpp()) {
        auto bpp2bpp = [this, bmp](auto buf_to_color, auto dec, auto color_to_buf, auto enc) -> void
        {
            uint8_t * dest = this->data_bitmap->get();
            const uint8_t * src = bmp.data_bitmap->get();
            const uint8_t src_nbbytes = nbbytes(dec.bpp);
            const uint8_t Bpp = nbbytes(enc.bpp);
            for (size_t y = 0; y < bmp.cy() ; y++) {
                for (size_t x = 0; x < bmp.cx() ; x++) {
                    BGRColor pixel = dec(buf_to_color(src));
                    constexpr bool enc_15_16 = enc.bpp == 15 || enc.bpp == 16;
                    constexpr bool dec_15_16 = dec.bpp == 15 || dec.bpp == 16;
                    if (enc_15_16 ^ dec_15_16) {
                        pixel = BGRasRGBColor(pixel);
                    }
                    color_to_buf(enc(pixel), dest);
                    dest += Bpp;
                    src += src_nbbytes;
                }
                src += bmp.line_size() - bmp.cx() * src_nbbytes;
                dest += this->line_size() - bmp.cx() * Bpp;
            }
        };

        this->data_bitmap = DataBitmap::construct(out_bpp, bmp.cx(), bmp.cy());
        auto buf2col_1B = [ ](uint8_t const * p) { return RDPColor::from(p[0]); };
        auto buf2col_2B = [=](uint8_t const * p) { return RDPColor::from(p[0] | (p[1] << 8)); };
        auto buf2col_3B = [=](uint8_t const * p) { return RDPColor::from(p[0] | (p[1] << 8) | (p[2] << 16)); };
        auto col2buf_1B = [ ](RDPColor c, uint8_t * p) {                   p[0] = c.as_bgr().red(); };
        auto col2buf_2B = [=](RDPColor c, uint8_t * p) { col2buf_1B(c, p); p[1] = c.as_bgr().green(); };
        auto col2buf_3B = [=](RDPColor c, uint8_t * p) { col2buf_2B(c, p); p[2] = c.as_bgr().blue(); };
        using namespace shortcut_encode;
        using namespace shortcut_decode_with_palette;
        switch ((bmp.bpp() << 8) + out_bpp) {
            case  (8<<8)+15: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_2B, enc15()); break;
            case  (8<<8)+16: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_2B, enc16()); break;
            case  (8<<8)+24: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_3B, enc24()); break;
            case (15<<8)+8 : bpp2bpp(buf2col_2B, dec15(), col2buf_1B, enc8()); break;
            case (15<<8)+16: bpp2bpp(buf2col_2B, dec15(), col2buf_2B, enc16()); break;
            case (15<<8)+24: bpp2bpp(buf2col_2B, dec15(), col2buf_3B, enc24()); break;
            case (16<<8)+8 : bpp2bpp(buf2col_2B, dec16(), col2buf_1B, enc8()); break;
            case (16<<8)+15: bpp2bpp(buf2col_2B, dec16(), col2buf_2B, enc15()); break;
            case (16<<8)+24: bpp2bpp(buf2col_2B, dec16(), col2buf_3B, enc24()); break;
            case (24<<8)+8 : bpp2bpp(buf2col_3B, dec24(), col2buf_1B, enc8()); break;
            case (24<<8)+15: bpp2bpp(buf2col_3B, dec24(), col2buf_2B, enc15()); break;
            case (24<<8)+16: bpp2bpp(buf2col_3B, dec24(), col2buf_2B, enc16()); break;
            default: assert(!"unknown bpp");
        }
        if (out_bpp == 8) {
            this->data_bitmap->palette() = BGRPalette::classic_332();
        }
    }
    else {
        this->data_bitmap = bmp.data_bitmap;
        this->data_bitmap->inc();
    }
}
