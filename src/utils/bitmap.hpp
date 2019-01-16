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

#include "utils/image_data_view.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/rect.hpp"

#include "system/ssl_sha1.hpp"


using std::size_t; /*NOLINT*/
class BGRPalette;
class OutStream;

class Bitmap
{
    class DataBitmap;

    DataBitmap *data_bitmap = nullptr;

public:
    void *operator new(size_t n) = delete;

    struct PrivateData;

    friend class PrivateData;

    Bitmap() noexcept = default;

    Bitmap(Bitmap && bmp) noexcept;
    Bitmap(const Bitmap & other) noexcept;

    // Detect TS_BITMAP_DATA(Uncompressed bitmap data) + (Compressed)bitmapDataStream
    Bitmap(BitsPerPixel session_color_depth, BitsPerPixel bpp, const BGRPalette * palette,
           uint16_t cx, uint16_t cy, const uint8_t * data, size_t size,
           bool compressed = false, size_t* RM18446_adjusted_size = nullptr);

    Bitmap(const Bitmap & src_bmp, Rect r);

    // TODO add palette support
    Bitmap(const uint8_t * vnc_raw, uint16_t vnc_cx, uint16_t vnc_cy, BitsPerPixel vnc_bpp, Rect tile);

    Bitmap(BitsPerPixel out_bpp, const Bitmap &bmp);

    ~Bitmap();

    Bitmap & operator=(const Bitmap & other) noexcept;
    Bitmap & operator=(Bitmap && other) noexcept;

    void compress(BitsPerPixel session_color_depth, OutStream & outbuffer) const;

    void swap(Bitmap & other) noexcept;

    bool is_valid() const noexcept;

    void reset() noexcept;

    const uint8_t * data() const noexcept;

    const BGRPalette & palette() const noexcept;

    uint16_t cx() const noexcept;
    uint16_t cy() const noexcept;

    size_t line_size() const noexcept;

    size_t bmp_size() const noexcept;

    BitsPerPixel bpp() const noexcept;

    bool has_data_compressed() const noexcept;

    array_view<uint8_t const> data_compressed() const noexcept;

    void compute_sha1(uint8_t (&sig)[SslSha1::DIGEST_LENGTH]) const;

    operator ConstImageDataView() const;
};

inline void swap(Bitmap & a, Bitmap & b) noexcept
{
    a.swap(b);
}
