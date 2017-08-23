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

#include "utils/sugar/array_view.hpp"
#include "utils/rect.hpp"

#include "system/ssl_sha1.hpp"


using std::size_t;
class Rect;
class BGRPalette;
class OutStream;

class Bitmap
{
    class DataBitmap;

    DataBitmap *data_bitmap = nullptr;

    void *operator new(size_t n) = delete;

public:
    struct PrivateData;

    friend class PrivateData;

    Bitmap() noexcept {}

    Bitmap(Bitmap && bmp) noexcept;
    Bitmap(const Bitmap & other) noexcept;

    Bitmap(uint8_t session_color_depth, uint8_t bpp, const BGRPalette * palette,
           uint16_t cx, uint16_t cy, const uint8_t * data, size_t size,
           bool compressed = false);

    Bitmap(const Bitmap & src_bmp, Rect r);

    // TODO add palette support
    Bitmap(const uint8_t * vnc_raw, uint16_t vnc_cx, uint16_t vnc_cy, uint8_t vnc_bpp, Rect tile);

    Bitmap(uint8_t out_bpp, const Bitmap &bmp);

    ~Bitmap();

    Bitmap & operator=(const Bitmap & other) noexcept;
    Bitmap & operator=(Bitmap && other) noexcept;

    void compress(uint8_t session_color_depth, OutStream & outbuffer) const;

    void swap(Bitmap & other) noexcept;

    bool is_valid() const noexcept;

    void reset() noexcept;

    const uint8_t * data() const noexcept;

    const BGRPalette & palette() const noexcept;

    uint16_t cx() const noexcept;
    uint16_t cy() const noexcept;

    size_t line_size() const noexcept;

    size_t bmp_size() const noexcept;

    uint8_t bpp() const noexcept;

    bool has_data_compressed() const noexcept;

    array_view<uint8_t const> data_compressed() const noexcept;

    void compute_sha1(uint8_t (&sig)[SslSha1::DIGEST_LENGTH]) const;


// TODO move to rdp_bitmap_compressed.hpp
// rdp_compress_bitmap(Bitmap, array_view)
// Bitmap rdp_decompress_bitmap(array_view)

    unsigned get_color_count(const uint8_t Bpp, const uint8_t *pmax, const uint8_t *p, unsigned color) const;
    unsigned get_fill_count(const uint8_t Bpp, const uint8_t *pmin, const uint8_t *pmax, const uint8_t *p) const;
    unsigned get_mix_count(const uint8_t Bpp, const uint8_t *pmin, const uint8_t *pmax, const uint8_t *p,
                           unsigned foreground) const;

    unsigned
    get_bicolor_count(const uint8_t Bpp, const uint8_t *pmax, const uint8_t *p, unsigned color1, unsigned color2) const;

    unsigned get_fom_count_set(const uint8_t Bpp, const uint8_t *pmin, const uint8_t *pmax, const uint8_t *p,
                               unsigned &foreground, unsigned &flags) const;

    static unsigned get_pixel(const uint8_t Bpp, const uint8_t *const p);

    void
    get_fom_masks(const uint8_t Bpp, const uint8_t *pmin, const uint8_t *p, uint8_t *mask, const unsigned count) const;

    static void get_run(const uint8_t *data, uint16_t data_size, uint8_t last_raw, uint32_t &run_length,
                        uint32_t &raw_bytes);

    static void compress_color_plane(uint16_t cx, uint16_t cy, OutStream &outbuffer, uint8_t *color_plane);

    unsigned get_pixel_above(const uint8_t Bpp, const uint8_t *pmin, const uint8_t *const p) const;
    static void decompress_color_plane(uint16_t src_cx, uint16_t src_cy, const uint8_t *&data,
                                       size_t &data_size, uint16_t cx, uint8_t *color_plane);

    enum {
        FLAG_NONE = 0,
        FLAG_FILL = 1,
        FLAG_MIX  = 2,
        FLAG_FOM  = 3,
        FLAG_MIX_SET = 6,
        FLAG_FOM_SET = 7,
        FLAG_COLOR = 8,
        FLAG_BICOLOR = 9
    };

private:
    void decompress(const uint8_t *input, uint16_t src_cx, uint16_t src_cy, size_t size) const;

    template<class TBpp>
    void decompress_(TBpp Bpp, const uint8_t *input, uint16_t src_cx, size_t size) const;

    static void in_copy_color_plan(uint16_t src_cx, uint16_t src_cy, const uint8_t *&data,
                                          size_t &data_size, uint16_t cx, uint8_t *color_plane);

    void decompress60(uint16_t src_cx, uint16_t src_cy, const uint8_t *data, size_t data_size) const;

    unsigned
    get_fom_count(const uint8_t Bpp, const uint8_t *pmin, const uint8_t *pmax, const uint8_t *p, unsigned foreground,
                  bool fill) const;

    template<class TBpp>
    void compress_(TBpp Bpp, OutStream &outbuffer) const;

    void compress60(OutStream &outbuffer) const;
};

inline void swap(Bitmap & a, Bitmap & b) noexcept
{
    a.swap(b);
}
