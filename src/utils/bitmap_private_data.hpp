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

#pragma once

#include "gdi/screen_info.hpp"
#include "utils/cpack.hpp"
#include "utils/bitmap.hpp"
#include "utils/bitmap_data_allocator.hpp"
#include "utils/bitfu.hpp"
#include "utils/colors.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <cstring>
#include <cassert>

#include <type_traits> // aligned_storage


struct DataBitmapBase
{
    const uint16_t cx_;
    const uint16_t cy_;
    const BitsPerPixel bpp_;
    uint_fast8_t counter_;
    const size_t line_size_;
    const size_t bmp_size_;
    uint8_t * const ptr_;
    // Memoize compressed bitmap
    /*mutable*/ uint8_t * data_compressed_;
    size_t size_compressed_;
    mutable uint8_t sha1_[SslSha1::DIGEST_LENGTH];
    mutable bool sha1_is_init_;
    DataBitmapBase(BitsPerPixel bpp, uint16_t cx, uint16_t cy, uint8_t * ptr) noexcept
    : cx_(cx)
    , cy_(cy)
    , bpp_(bpp)
    , counter_(1)
    , line_size_(this->cx_ * nb_bytes_per_pixel(this->bpp_))
    , bmp_size_(this->line_size_ * cy)
    , ptr_(ptr)
    , data_compressed_(nullptr)
    , size_compressed_(0)
    , sha1_is_init_(false)
    {}
};

class Bitmap::DataBitmap : DataBitmapBase
{
    DataBitmap(uint16_t cx, uint16_t cy, uint8_t * ptr) noexcept
    : DataBitmapBase(BitsPerPixel{24}, cx, cy, ptr)
    {}

    DataBitmap(BitsPerPixel bpp, uint16_t cx, uint16_t cy, uint8_t * ptr) noexcept
    : DataBitmapBase(bpp, cx, cy, ptr)
    {}

    ~DataBitmap()
    {
        aux_::bitmap_data_allocator.dealloc(this->data_compressed_);
    }

    static const size_t palette_index = sizeof(typename std::aligned_storage<sizeof(DataBitmapBase), alignof(BGRColor)>::type);

public:
    DataBitmap(DataBitmap const &) = delete;
    DataBitmap & operator=(DataBitmap const &) = delete;

    static DataBitmap * construct(BitsPerPixel bpp, uint16_t cx, uint16_t cy)
    {
        const size_t sz = align4(cx) * nb_bytes_per_pixel(bpp) * cy;
        const size_t sz_struct = (bpp == BitsPerPixel{8}) ? palette_index + sizeof(BGRPalette) : sizeof(DataBitmap);
        uint8_t * p = static_cast<uint8_t*>(aux_::bitmap_data_allocator.alloc(sz_struct + sz));
        return new (p) DataBitmap(bpp, cx, cy, p + sz_struct); /*NOLINT*/
    }

    static DataBitmap * construct_png(uint16_t cx, uint16_t cy)
    {
        const size_t sz = cx * cy * 3;
        const size_t sz_struct = sizeof(DataBitmap);
        uint8_t * p = static_cast<uint8_t*>(aux_::bitmap_data_allocator.alloc(sz_struct + sz));
        return new (p) DataBitmap(cx, cy, p + sz_struct); /*NOLINT*/
    }

    static void destruct(DataBitmap * cdata) noexcept
    {
        cdata->~DataBitmap();
        aux_::bitmap_data_allocator.dealloc(cdata);
    }

    void copy_sha1(uint8_t (&sig)[SslSha1::DIGEST_LENGTH]) const
    {
        if (!this->sha1_is_init_) {
            this->sha1_is_init_ = true;
            SslSha1 sha1;
            if (this->bpp_ == BitsPerPixel{8}) {
                sha1.update({this->data_palette(), sizeof(BGRPalette)});
            }
            StaticOutStream<5> out_stream;
            out_stream.out_uint8(safe_int(this->bpp_));
            out_stream.out_uint16_le(this->cx_);
            out_stream.out_uint16_le(this->cy_);
            sha1.update(out_stream.get_bytes());
            const uint8_t * first = this->get();
            const uint8_t * last = first + this->cy_ * this->line_size_;
            for (; first != last; first += this->line_size_) {
                sha1.update({first, this->line_size_});
            }
            sha1.final(this->sha1_);
        }
        memcpy(sig, this->sha1_, sizeof(this->sha1_));
    }

    uint8_t * get() const noexcept
    {
        return this->ptr_;
    }

protected:
    uint8_t const * data_palette() const noexcept
    {
        //assert(this->bpp() == 8);
        return reinterpret_cast<uint8_t const*>(this) + palette_index; /*NOLINT*/
    }

public:
    BGRPalette & palette() noexcept
    {
        //assert(this->bpp() == 8);
        return reinterpret_cast<BGRPalette &>(reinterpret_cast<uint8_t*>(this)[palette_index]); /*NOLINT*/
    }

    const BGRPalette & palette() const noexcept
    {
        //assert(this->bpp() == 8);
        return reinterpret_cast<const BGRPalette &>(reinterpret_cast<const uint8_t*>(this)[palette_index]); /*NOLINT*/
    }

    uint16_t cx() const noexcept
    {
        return this->cx_;
    }

    uint16_t cy() const noexcept
    {
        return this->cy_;
    }

    size_t line_size() const noexcept
    {
        return this->line_size_;
    }

    BitsPerPixel bpp() const noexcept
    {
        return this->bpp_;
    }

    size_t bmp_size() const noexcept
    {
        return this->bmp_size_;
    }

    void copy_compressed_buffer(void const * data, size_t n)
    {
        assert(this->compressed_size() == 0);
        uint8_t * p = static_cast<uint8_t*>(aux_::bitmap_data_allocator.alloc(n));
        this->data_compressed_ = static_cast<uint8_t*>(memcpy(p, data, n));
        this->size_compressed_ = n;
    }

    const uint8_t * compressed_data() const noexcept
    {
        return this->data_compressed_;
    }

    size_t compressed_size() const noexcept
    {
        return this->size_compressed_;
    }

    void inc() noexcept
    {
        ++this->counter_;
    }

    void dec() noexcept
    {
        --this->counter_;
    }

    uint_fast8_t count() const noexcept
    {
        return this->counter_;
    }
};

struct Bitmap::PrivateData
{
    using Data = Bitmap::DataBitmap;
    static Data & initialize(Bitmap & bmp, BitsPerPixel bpp, uint16_t cx, uint16_t cy)
    { return *(bmp.data_bitmap = DataBitmap::construct(bpp, cx, cy)); }
    static Data & initialize_png(Bitmap & bmp, uint16_t cx, uint16_t cy)
    { return *(bmp.data_bitmap = DataBitmap::construct_png(cx, cy)); }
};
