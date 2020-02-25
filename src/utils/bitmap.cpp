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

#include "utils/bitfu.hpp"
#include "utils/colors.hpp"
#include "utils/image_data_view.hpp"
#include "utils/log.hpp"
#include "utils/rect.hpp"
#include "utils/rle.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "system/ssl_sha1.hpp"

#include <cstring>

#include <cassert>
#include <utility>


namespace aux_
{
    BmpMemAlloc bitmap_data_allocator;
} // namespace aux_

Bitmap::Bitmap(Bitmap && bmp) noexcept
: data_bitmap(std::exchange(bmp.data_bitmap, nullptr))
{}

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

// Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
Bitmap::Bitmap(
    BitsPerPixel session_color_depth, BitsPerPixel bpp, const BGRPalette * palette,
    uint16_t cx, uint16_t cy, const uint8_t * data, const size_t size,
    bool compressed, size_t* RM18446_adjusted_size)
: data_bitmap(DataBitmap::construct(bpp, cx, cy))
{
    if (cx <= 0 || cy <= 0){
        LOG(LOG_ERR, "Bogus empty bitmap!!! cx=%u cy=%u size=%zu bpp=%u", cx, cy, size, bpp);
    }
    if (bpp == BitsPerPixel{8}){
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
        MutableImageDataView const image_view{
            this->data_bitmap->get(),
            this->cx(),
            this->cy(),
            this->line_size(),
            this->bpp(),
            ConstImageDataView::Storage::BottomToTop
        };

        if (session_color_depth == BitsPerPixel{32} && (bpp == BitsPerPixel{24} || bpp == BitsPerPixel{32})) {
            rle_decompress60(image_view, cx, cy, data, size);
        }
        else {
            // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
            rle_decompress(image_view, data, cx, cy, size, RM18446_adjusted_size);
        }
    } else {
        uint8_t * dest = this->data_bitmap->get();
        const uint8_t * src = data;
        const size_t data_width = cx * nb_bytes_per_pixel(bpp);
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

Bitmap::Bitmap(const uint8_t *data, size_t stride, const Rect &rect)
    : data_bitmap(DataBitmap::construct(BitsPerPixel{32}, align4(rect.width()), rect.height()))
{
    uint8_t *dest = this->data_bitmap->get();
    const size_t lineSize = this->data_bitmap->line_size();
    const uint8_t *src = data + (rect.height() - 1) * stride;

    for (uint16_t i = 0; i < rect.height(); i++, src-= stride, dest += lineSize) {
        if (stride < lineSize){
            memcpy(dest, src, stride);
            memset(dest + stride, 0, lineSize - stride);
        }
        else {
            memcpy(dest, src, lineSize);
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
    if (this->bpp() == BitsPerPixel{8}) {
        this->data_bitmap->palette() = src_bmp.data_bitmap->palette();
    }
    // bitmapDataStream (variable): A variable-sized array of bytes.
    //  Uncompressed bitmap data represents a bitmap as a bottom-up,
    //  left-to-right series of pixels. Each pixel is a whole
    //  number of bytes. Each row contains a multiple of four bytes
    // (including up to three bytes of padding, as necessary).
    // In redemption we ensure a more constraint restriction to avoid padding
    // bitmap width must always be a multiple of 4
    const uint8_t Bpp = nb_bytes_per_pixel(this->bpp());
    uint8_t *dest = this->data_bitmap->get();
    const size_t dest_line_size = this->line_size();
    const size_t src_line_size = src_bmp.line_size();
    const uint16_t cy = this->cy();
    const uint16_t src_cy = src_bmp.cy();
    const uint8_t *src = src_bmp.data_bitmap->get() + src_line_size * (src_cy - r.y - cy) + r.x * Bpp;
    const unsigned line_to_copy = std::min<unsigned>(r.cx * Bpp, (src_bmp.cx() - r.x) * Bpp);
    for (uint16_t i = 0; i < cy; i++) {
        memcpy(dest, src, line_to_copy);
        if (line_to_copy < dest_line_size){
            memset(dest + line_to_copy, 0, dest_line_size - line_to_copy);
        }
        src += src_line_size;
        dest += dest_line_size;
    }
}

Bitmap::Bitmap(const uint8_t * vnc_raw, uint16_t vnc_cx, uint16_t /*vnc_cy*/, BitsPerPixel vnc_bpp, const Rect tile)
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
    const uint8_t Bpp = nb_bytes_per_pixel(this->bpp());
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

BitsPerPixel Bitmap::bpp() const noexcept
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

// TODO simplify and enhance compression using 1 pixel orders BLACK or WHITE.
void Bitmap::compress(BitsPerPixel session_color_depth, OutStream & outbuffer) const
{
    if (this->data_bitmap->compressed_size()) {
        outbuffer.out_copy_bytes(this->data_bitmap->compressed_data(), this->data_bitmap->compressed_size());
        return;
    }

    uint8_t * tmp_data_compressed = outbuffer.get_current();

    ConstImageDataView const image_view{
        this->data(),
        this->cx(),
        this->cy(),
        this->line_size(),
        this->bpp(),
        ConstImageDataView::Storage::BottomToTop
    };

    if ((session_color_depth == BitsPerPixel{32}) && ((this->bpp() == BitsPerPixel{24}) || (this->bpp() == BitsPerPixel{32}))) {
        rle_compress60(image_view, outbuffer);
    }
    else {
        rle_compress(image_view, outbuffer);
    }

    // Memoize result of compression
    this->data_bitmap->copy_compressed_buffer(
        tmp_data_compressed, outbuffer.get_current() - tmp_data_compressed);
}

void Bitmap::compute_sha1(uint8_t (&sig)[SslSha1::DIGEST_LENGTH]) const
{
    this->data_bitmap->copy_sha1(sig);
}

Bitmap::Bitmap(BitsPerPixel out_bpp, const Bitmap & bmp)
{
    //LOG(LOG_INFO, "Creating bitmap (%p) (copy constructor) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, bpp);
    if (out_bpp != bmp.bpp()) {
        auto bpp2bpp = [this, bmp](auto buf_to_color, auto dec, auto color_to_buf, auto enc) -> void
        {
            uint8_t * dest = this->data_bitmap->get();
            const uint8_t * src = bmp.data_bitmap->get();
            const uint8_t src_nbbytes = nb_bytes_per_pixel(dec.bpp);
            const uint8_t Bpp = nb_bytes_per_pixel(enc.bpp);
            for (size_t y = 0; y < bmp.cy(); y++) {
                for (size_t x = 0; x < bmp.cx(); x++) {
                    BGRColor pixel = dec(buf_to_color(src));
                    constexpr bool enc_8_15_16 = enc.bpp == BitsPerPixel{8}
                                              || enc.bpp == BitsPerPixel{15}
                                              || enc.bpp == BitsPerPixel{16};
                    constexpr bool dec_8_15_16 = dec.bpp == BitsPerPixel{8}
                                              || dec.bpp == BitsPerPixel{15}
                                              || dec.bpp == BitsPerPixel{16};
                    if (enc_8_15_16 != dec_8_15_16) {
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
        auto buf2col_4B = [=](uint8_t const * p) { return RDPColor::from(p[0] | (p[1] << 8) | (p[2] << 16)); };
        auto col2buf_1B = [ ](RDPColor c, uint8_t * p) {                   p[0] = c.as_bgr().red(); };
        auto col2buf_2B = [=](RDPColor c, uint8_t * p) { col2buf_1B(c, p); p[1] = c.as_bgr().green(); };
        auto col2buf_3B = [=](RDPColor c, uint8_t * p) { col2buf_2B(c, p); p[2] = c.as_bgr().blue(); };
        auto col2buf_4B = [=](RDPColor c, uint8_t * p) { col2buf_2B(c, p); p[2] = c.as_bgr().blue(); p[3] = 0xff; };
        using namespace shortcut_encode;
        using namespace shortcut_decode_with_palette;
        switch ((underlying_cast(bmp.bpp()) << 8) + underlying_cast(out_bpp)) {
            case  (8<<8)+15: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_2B, enc15()); break;
            case  (8<<8)+16: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_2B, enc16()); break;
            case  (8<<8)+24: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_3B, enc24()); break;
            case  (8<<8)+32: bpp2bpp(buf2col_1B, dec8{bmp.palette()}, col2buf_4B, enc32()); break;
            case (15<<8)+8 : bpp2bpp(buf2col_2B, dec15(), col2buf_1B, enc8()); break;
            case (15<<8)+16: bpp2bpp(buf2col_2B, dec15(), col2buf_2B, enc16()); break;
            case (15<<8)+24: bpp2bpp(buf2col_2B, dec15(), col2buf_3B, enc24()); break;
            case (15<<8)+32: bpp2bpp(buf2col_2B, dec15(), col2buf_4B, enc32()); break;
            case (16<<8)+8 : bpp2bpp(buf2col_2B, dec16(), col2buf_1B, enc8()); break;
            case (16<<8)+15: bpp2bpp(buf2col_2B, dec16(), col2buf_2B, enc15()); break;
            case (16<<8)+24: bpp2bpp(buf2col_2B, dec16(), col2buf_3B, enc24()); break;
            case (16<<8)+32: bpp2bpp(buf2col_2B, dec16(), col2buf_4B, enc32()); break;
            case (24<<8)+8 : bpp2bpp(buf2col_3B, dec24(), col2buf_1B, enc8()); break;
            case (24<<8)+15: bpp2bpp(buf2col_3B, dec24(), col2buf_2B, enc15()); break;
            case (24<<8)+16: bpp2bpp(buf2col_3B, dec24(), col2buf_2B, enc16()); break;
            case (24<<8)+32: bpp2bpp(buf2col_3B, dec24(), col2buf_4B, enc32()); break;
            case (32<<8)+8 : bpp2bpp(buf2col_4B, dec32(), col2buf_1B, enc8()); break;
            case (32<<8)+15: bpp2bpp(buf2col_4B, dec32(), col2buf_2B, enc15()); break;
            case (32<<8)+16: bpp2bpp(buf2col_4B, dec32(), col2buf_2B, enc16()); break;
            case (32<<8)+24: bpp2bpp(buf2col_4B, dec32(), col2buf_3B, enc24()); break;
            default: assert(!"unknown bpp");
        }
        if (out_bpp == BitsPerPixel{8}) {
            this->data_bitmap->palette() = BGRPalette::classic_332();
        }
    }
    else {
        this->data_bitmap = bmp.data_bitmap;
        this->data_bitmap->inc();
    }
}

Bitmap::operator ConstImageDataView() const
{
    // 8, 15, 16 = BGR
    // 24 = RGB
    return ConstImageDataView{
        this->data(),
        this->cx(), this->cy(),
        this->line_size(),
        this->bpp(),
        ConstImageDataView::Storage::BottomToTop,
        &this->data_bitmap->palette()
    };
}
