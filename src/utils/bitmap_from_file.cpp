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

#include "utils/bitmap_from_file.hpp"

#include "utils/bitmap.hpp"
#include "utils/bitmap_private_data.hpp"
#include "utils/file.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "cxx/cxx.hpp"

#ifndef __EMSCRIPTEN__
# include <png.h>
#endif

#include <cerrno>


using std::size_t; /*NOLINT*/

namespace
{
    bool read_all(int fd, void * data_, std::size_t len)
    {
        unsigned char * data = static_cast<unsigned char *>(data_);
        while (len) {
            ssize_t const ret = ::read(fd, data, len);
            if (ret <= 0) {
                if (errno == EINTR) {
                    continue;
                }
                return false;
            }
            len -= ret;
            data += ret;
        }
        return true;
    }

    Bitmap bitmap_from_bmp_without_sig(int fd, const char * filename);
#ifndef __EMSCRIPTEN__
    Bitmap bitmap_from_png_without_sig(int fd, const char * filename);
#endif
} // namespace


Bitmap bitmap_from_file_impl(const char * filename)
{
#ifdef __EMSCRIPTEN__
    using png_byte = uint8_t;
#endif
    png_byte type1[8];

    unique_fd file{filename, O_RDONLY};

    if (!file) {
        LOG(LOG_ERR, "Bitmap: error loading bitmap from file [%s] %s(%d)",
            filename, strerror(errno), errno);
        // TODO see value returned, maybe we should return open error
        return Bitmap{};
    }

    if (not read_all(file.fd(), type1, 2)) {
        LOG(LOG_ERR, "Bitmap: error bitmap file [%s] read error", filename);
        return Bitmap{};
    }
    if (type1[0] == 'B' && type1[1] == 'M') {
        LOG(LOG_INFO, "Bitmap: image file [%s] is BMP file\n", filename);
        return bitmap_from_bmp_without_sig(file.fd(), filename);
    }
    if (not read_all(file.fd(), type1 + 2, 6)) {
        LOG(LOG_ERR, "Bitmap: error bitmap file [%s] read error", filename);
        return Bitmap{};
    }
#ifndef __EMSCRIPTEN__
    if (png_sig_cmp(type1, 0, 8) == 0) {
        //LOG(LOG_INFO, "Bitmap: image file [%s] is PNG file\n", filename);
        return bitmap_from_png_without_sig(file.fd(), filename);
    }
#endif

    LOG(LOG_ERR, "Bitmap: error bitmap file [%s] not BMP or PNG file\n", filename);
    return Bitmap{};
}


Bitmap bitmap_from_file(const char * filename)
{
    Bitmap bitmap = bitmap_from_file_impl(filename);
    if (bitmap.is_valid()) {
        return bitmap;
    }
    return load_error_bitmap();
}

namespace
{
#ifndef __EMSCRIPTEN__
Bitmap bitmap_from_png_without_sig(int fd, const char * /*filename*/)
{
    Bitmap bitmap;

    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        return bitmap;
    }
    png_infop info_ptr = nullptr;
    SCOPE_EXIT(png_destroy_read_struct(&png_ptr, &info_ptr, nullptr));

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        return bitmap;
    }

#if PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4)
    if (setjmp(png_jmpbuf(png_ptr)))
#else
    if (setjmp(png_ptr->jmpbuf))
#endif
    {
        return bitmap;
    }
    // this handle lib png errors for this call

    File file(fdopen(fd, "rb"));
    if (!file) {
        return bitmap;
    }
    png_init_io(png_ptr, file.get());

    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
#if PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
#else
        png_set_gray_1_2_4_to_8(png_ptr);
#endif
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }
    else if (bit_depth < 8) {
        png_set_packing(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }

    if (color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_strip_alpha(png_ptr);
    }
    png_set_bgr(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // TODO Looks like there's a shift when width is not divisible by 4
    png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (static_cast<uint16_t>(width) * 3 != rowbytes) {
        LOG(LOG_ERR, "PNG Image has bad type");
        return bitmap;
    }

    auto u = std::make_unique<png_bytep[]>(height);
    png_bytep * row_pointers = u.get();
    Bitmap::PrivateData::Data & data = Bitmap::PrivateData::initialize_png(bitmap, width, height);
    png_bytep row = data.get() + data.bmp_size() - data.line_size();
    for (uint i = 0; i < height; ++i) {
        row_pointers[i] = row - i * data.line_size();
    }
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);

    return bitmap;
}
#endif

Bitmap bitmap_from_bmp_without_sig(int fd, const char * filename)
{
    Bitmap bitmap;
    BGRPalette palette1 = BGRPalette::classic_332();

    /* header for bmp file */
    struct bmp_header {
        size_t size = 0;
        unsigned image_width = 0;
        unsigned image_height = 0;
        short planes = 0;
        short bit_count = 0;
        int compression = 0;
        int image_size = 0;
        int x_pels_per_meter = 0;
        int y_pels_per_meter = 0;
        int clr_used = 0;
        int clr_important = 0;
    } header;

    BufMaker<8192> buf_maker;
    InStream stream(buf_maker.static_array());

    // TODO reading of file and bitmap decoding should be kept appart  putting both together makes testing hard. And what if I want to read a bitmap from some network socket instead of a disk file ?
    {
        auto* stream_data = buf_maker.static_array().data();
        /* read file size */
        // TODO define some stream aware function to read data from file (to update stream.end by itself). It should probably not be inside stream itself because read primitives are OS dependant, and there is not need to make stream OS dependant.
        if (not read_all(fd, stream_data, 4)) {
            LOG(LOG_ERR, "Widget_load: error read file size");
            return bitmap;
        }
        stream = InStream(stream_data, 4);
        {
            // TODO Check what is this size ? header size ? used as fixed below ?
                /* uint32_t size = */ stream.in_uint32_le();
        }

        // skip some bytes to set file pointer to bmp header
        lseek(fd, 14, SEEK_SET);
        if (not read_all(fd, stream_data, 40)){
            LOG(LOG_ERR, "Widget_load: error read file size (2)");
            return bitmap;
        }
        stream = InStream(stream_data, 40);
        // TODO we should read header size and use it to read header instead of using magic constant 40
        header.size = stream.in_uint32_le();
        if (header.size != 40){
            LOG(LOG_INFO, "Wrong header size: expected 40, got %zu", header.size);
            assert(header.size == 40);
        }

        header.image_width = stream.in_uint32_le();         // used
        header.image_height = stream.in_uint32_le();        // used
        header.planes = stream.in_uint16_le();
        header.bit_count = stream.in_uint16_le();           // used
        header.compression = stream.in_uint32_le();
        header.image_size = stream.in_uint32_le();
        header.x_pels_per_meter = stream.in_uint32_le();
        header.y_pels_per_meter = stream.in_uint32_le();
        header.clr_used = stream.in_uint32_le();            // used
        header.clr_important = stream.in_uint32_le();

        // skip header (including more fields that we do not read if any)
        lseek(fd, 14 + header.size, SEEK_SET);

        // compute pixel size (in Quartet) and read palette if needed
        unsigned file_Qpp = 1;
        // TODO add support for loading of 16 bits bmp from file
        switch (header.bit_count) {
                // Qpp = groups of 4 bytes per pixel
            case 24:
                file_Qpp = 6;
                break;
            case 8:
                file_Qpp = 2;
                REDEMPTION_CXX_FALLTHROUGH;
            case 4:
                assert(header.clr_used * 4 <= 8192);
                if (not read_all(fd, stream_data, header.clr_used * 4)){
                    return bitmap;
                }
                stream = InStream(stream_data, header.clr_used * 4);
                for (int i = 0; i < header.clr_used; i++) {
                    uint8_t r = stream.in_uint8();
                    uint8_t g = stream.in_uint8();
                    uint8_t b = stream.in_uint8();
                    stream.in_skip_bytes(1); // skip alpha channel
                    palette1.set_color(i, BGRColor(b, g, r));
                }
                break;
            default:
                LOG(LOG_ERR, "Bitmap: error bitmap file [%s] unsupported bpp %d\n",
                    filename, header.bit_count);
                return bitmap;
        }

        LOG(LOG_INFO, "loading file %u x %u x %d", header.image_width, header.image_height, header.bit_count);

        // bitmap loaded from files are always converted to 24 bits
        // this avoid palette problems for 8 bits,
        // and 4 bits is not supported in other parts of code anyway

        // read bitmap data
        {
            unsigned const row_size = (header.image_width * file_Qpp) / 2u;
            unsigned const padding = align4(row_size) - row_size;
            size_t const size = row_size * header.image_height;
            size_t const bufsize = size + padding;
            auto p = buf_maker.dyn_array(bufsize).data();
            for (unsigned y = 0; y < header.image_height; y++) {
                if (not read_all(fd, p + y * row_size, row_size + padding)) {
                    LOG(LOG_ERR, "Bitmap: read error reading bitmap file [%s] read\n", filename);
                    return bitmap;
                }
            }
            stream = InStream(p, size);
        }
    }

    auto initializer_data = [&header, &bitmap](auto init_pixel_at) {
        Bitmap::PrivateData::Data & data
          = Bitmap::PrivateData::initialize(
              bitmap, BitsPerPixel{24}, header.image_width, header.image_height);
        const uint8_t Bpp = 3;
        uint8_t * dest = data.get();
        const size_t line_size = data.line_size();
        for (unsigned y = 0; y < header.image_height ; y++) {
            auto const p = dest + y * line_size;
            for (unsigned x = 0 ; x < header.image_width; x++) {
                init_pixel_at(p + x * Bpp, x);
            }
            if (line_size > header.image_width * Bpp) {
                memset(p + header.image_width * Bpp, 0, line_size - header.image_width * Bpp);
            }
        }
    };

    switch (header.bit_count){
    case 24:
        initializer_data([&stream](uint8_t * d, unsigned /*x*/){
            d[0] = stream.in_uint8();
            d[1] = stream.in_uint8();
            d[2] = stream.in_uint8();
        });
        break;
    case 16:
        initializer_data([&stream](uint8_t * d, unsigned /*x*/){
            BGRColor c = decode_color16()(RDPColor::from(stream.in_uint16_le()));
            d[0] = c.blue();
            d[1] = c.green();
            d[2] = c.red();
        });
        break;
    case 8:
        initializer_data([&stream, &palette1](uint8_t * d, unsigned /*x*/){
            BGRColor c = decode_color8()(RDPColor::from(stream.in_uint8()), palette1);
            d[0] = c.blue();
            d[1] = c.green();
            d[2] = c.red();
        });
        break;
    case 4:
        int k = 0;
        initializer_data([&stream, &palette1, &k](uint8_t * d, unsigned x){
            uint8_t idx_palette;
            if ((x & 1) == 0) {
                k = stream.in_uint8();
                idx_palette = (k >> 4) & 0xf;
            }
            else {
                idx_palette = k & 0xf;
            }
            BGRColor c = palette1[idx_palette];
            d[0] = c.blue();
            d[1] = c.green();
            d[2] = c.red();
        });
        break;
    }

    return bitmap;
}
} // namespace

Bitmap load_error_bitmap()
{
    static constexpr uint8_t errorbmp[] = {
/* 0000 */ 0x00, 0x00, 0x00, 0x2d, 0x2d, 0xb6, 0x30, 0x30, 0xb8, 0x20, 0x20, 0x80, 0x07, 0x07, 0x33, 0x00,  // ...--.00.  ...3.
/* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 0020 */ 0x00, 0x30, 0x30, 0xc8, 0x2c, 0x2c, 0xb8, 0x18, 0x18, 0x7f, 0x0c, 0x0c, 0x4d, 0x00, 0x00, 0x00,  // .00.,,......M...
/* 0030 */ 0x26, 0x26, 0xc3, 0x4c, 0x4c, 0xdd, 0x7a, 0x7a, 0xf2, 0x6f, 0x6f, 0xde, 0x43, 0x43, 0xa2, 0x00,  // &&.LL.zz.oo.CC..
/* 0040 */ 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x28,  // ..............((
/* 0050 */ 0xb4, 0x58, 0x58, 0xe1, 0x68, 0x68, 0xf4, 0x4d, 0x4d, 0xde, 0x26, 0x26, 0x9e, 0x07, 0x07, 0x5b,  // .XX.hh.MM.&&...[
/* 0060 */ 0x31, 0x31, 0xdb, 0x60, 0x60, 0xf5, 0x7c, 0x7c, 0xfe, 0x8e, 0x8e, 0xf9, 0x76, 0x76, 0xdd, 0x43,  // 11.``.||....vv.C
/* 0070 */ 0x43, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2d, 0xbb, 0x63, 0x63,  // C..........--.cc
/* 0080 */ 0xe0, 0x70, 0x70, 0xf6, 0x59, 0x59, 0xff, 0x42, 0x42, 0xf7, 0x36, 0x36, 0xde, 0x10, 0x10, 0x85,  // .pp.YY.BB.66....
/* 0090 */ 0x39, 0x39, 0xf3, 0x59, 0x59, 0xfc, 0x6f, 0x6f, 0xfe, 0x8c, 0x8c, 0xff, 0x97, 0x97, 0xf9, 0x78,  // 99.YY.oo.......x
/* 00a0 */ 0x78, 0xdd, 0x3c, 0x3c, 0xa0, 0x00, 0x00, 0x00, 0x2c, 0x2c, 0xc1, 0x5f, 0x5f, 0xdf, 0x79, 0x79,  // x.<<....,,.__.yy
/* 00b0 */ 0xf5, 0x6a, 0x6a, 0xfe, 0x47, 0x47, 0xff, 0x36, 0x36, 0xfd, 0x36, 0x36, 0xf6, 0x15, 0x15, 0xd0,  // .jj.GG.66.66....
/* 00c0 */ 0x37, 0x37, 0xfc, 0x46, 0x46, 0xf8, 0x60, 0x60, 0xfb, 0x7a, 0x7a, 0xfe, 0x92, 0x92, 0xfe, 0x8f,  // 77.FF.``.zz.....
/* 00d0 */ 0x8f, 0xf7, 0x6d, 0x6d, 0xda, 0x39, 0x39, 0xaf, 0x57, 0x57, 0xd5, 0x78, 0x78, 0xf5, 0x74, 0x74,  // ..mm.99.WW.xx.tt
/* 00e0 */ 0xfd, 0x59, 0x59, 0xfe, 0x43, 0x43, 0xfd, 0x3a, 0x3a, 0xf7, 0x2b, 0x2b, 0xf1, 0x14, 0x14, 0xf8,  // .YY.CC.::.++....
/* 00f0 */ 0x00, 0x00, 0x00, 0x34, 0x34, 0xf1, 0x4c, 0x4c, 0xf5, 0x62, 0x62, 0xfb, 0x78, 0x78, 0xfe, 0x87,  // ...44.LL.bb.xx..
/* 0100 */ 0x87, 0xfe, 0x7b, 0x7b, 0xf5, 0x65, 0x65, 0xe7, 0x6b, 0x6b, 0xef, 0x74, 0x74, 0xfe, 0x5f, 0x5f,  // ..{{.ee.kk.tt.__
/* 0110 */ 0xff, 0x4b, 0x4b, 0xfc, 0x3f, 0x3f, 0xf6, 0x34, 0x34, 0xea, 0x12, 0x12, 0xb4, 0x00, 0x00, 0x00,  // .KK.??.44.......
/* 0120 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x31, 0xed, 0x46, 0x46, 0xf3, 0x5a, 0x5a, 0xfb, 0x68,  // ......11.FF.ZZ.h
/* 0130 */ 0x68, 0xff, 0x6c, 0x6c, 0xfe, 0x6a, 0x6a, 0xfd, 0x64, 0x64, 0xfd, 0x59, 0x59, 0xff, 0x4a, 0x4a,  // h.ll.jj.dd.YY.JJ
/* 0140 */ 0xfd, 0x3f, 0x3f, 0xf6, 0x30, 0x30, 0xe8, 0x00, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .??.00..........
/* 0150 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x2c, 0xe7, 0x39, 0x39, 0xea, 0x48,  // .........,,.99.H
/* 0160 */ 0x48, 0xf7, 0x54, 0x54, 0xff, 0x52, 0x52, 0xff, 0x47, 0x47, 0xfe, 0x3a, 0x3a, 0xfb, 0x33, 0x33,  // H.TT.RR.GG.::.33
/* 0170 */ 0xec, 0x2d, 0x2d, 0xd6, 0x06, 0x06, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .--.............
/* 0180 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x15, 0x15, 0xdc, 0x37,  // ...............7
/* 0190 */ 0x37, 0xf5, 0x4d, 0x4d, 0xff, 0x4e, 0x4e, 0xff, 0x40, 0x40, 0xfe, 0x2b, 0x2b, 0xfa, 0x14, 0x14,  // 7.MM.NN.@@.++...
/* 01a0 */ 0xe0, 0x00, 0x00, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 01b0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9d, 0x04, 0x04, 0xd0, 0x28, 0x28, 0xee, 0x4c,  // ............((.L
/* 01c0 */ 0x4c, 0xfc, 0x55, 0x55, 0xff, 0x48, 0x48, 0xfe, 0x47, 0x47, 0xff, 0x42, 0x42, 0xfe, 0x25, 0x25,  // L.UU.HH.GG.BB.%%
/* 01d0 */ 0xf2, 0x0e, 0x0e, 0xd2, 0x00, 0x00, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 01e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x04, 0x04, 0xd0, 0x31, 0x31, 0xef, 0x61, 0x61, 0xfc, 0x70,  // .........11.aa.p
/* 01f0 */ 0x70, 0xff, 0x49, 0x49, 0xfa, 0x1f, 0x1f, 0xf3, 0x32, 0x32, 0xf7, 0x5f, 0x5f, 0xfe, 0x68, 0x68,  // p.II....22.__.hh
/* 0200 */ 0xff, 0x44, 0x44, 0xf4, 0x16, 0x16, 0xd3, 0x00, 0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .DD.............
/* 0210 */ 0x02, 0x02, 0x9a, 0x11, 0x11, 0xcb, 0x3b, 0x3b, 0xef, 0x7e, 0x7e, 0xfd, 0x94, 0x94, 0xfe, 0x65,  // ......;;.~~....e
/* 0220 */ 0x65, 0xfa, 0x27, 0x27, 0xee, 0x03, 0x03, 0xdb, 0x13, 0x13, 0xe6, 0x49, 0x49, 0xf7, 0x86, 0x86,  // e.''.......II...
/* 0230 */ 0xfe, 0x8f, 0x8f, 0xfe, 0x5c, 0x5c, 0xf5, 0x1c, 0x1c, 0xd0, 0x03, 0x03, 0x90, 0x00, 0x00, 0x36,  // ...............6
/* 0240 */ 0x06, 0x06, 0xbe, 0x40, 0x40, 0xef, 0x9e, 0x9e, 0xfe, 0xbb, 0xbb, 0xff, 0x83, 0x83, 0xfb, 0x33,  // ...@@..........3
/* 0250 */ 0x33, 0xee, 0x00, 0x00, 0xd5, 0x00, 0x00, 0x57, 0x00, 0x00, 0xce, 0x18, 0x18, 0xe7, 0x5f, 0x5f,  // 3......W......__
/* 0260 */ 0xf7, 0xaa, 0xaa, 0xfe, 0xb7, 0xb7, 0xff, 0x6d, 0x6d, 0xf5, 0x13, 0x13, 0xcf, 0x00, 0x00, 0x68,  // .......mm......h
/* 0270 */ 0x09, 0x09, 0xd7, 0x60, 0x60, 0xf4, 0xbe, 0xbe, 0xff, 0x9e, 0x9e, 0xfb, 0x3a, 0x3a, 0xed, 0x00,  // ...``.......::..
/* 0280 */ 0x00, 0xd3, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xce, 0x20, 0x20,  // ..............
/* 0290 */ 0xe6, 0x73, 0x73, 0xf6, 0xc6, 0xc6, 0xff, 0x94, 0x94, 0xfa, 0x22, 0x22, 0xe7, 0x01, 0x01, 0xa5,  // .ss.......""....
/* 02a0 */ 0x04, 0x04, 0xcd, 0x25, 0x25, 0xe1, 0x60, 0x60, 0xf3, 0x37, 0x37, 0xeb, 0x0b, 0x0b, 0xcf, 0x00,  // ...%%.``.77.....
/* 02b0 */ 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .y..............
/* 02c0 */ 0xc8, 0x32, 0x32, 0xe4, 0x61, 0x61, 0xf6, 0x37, 0x37, 0xeb, 0x11, 0x11, 0xce, 0x00, 0x00, 0xad,  // .22.aa.77.......
/* 02d0 */ 0x00, 0x00, 0x00, 0x05, 0x05, 0xc6, 0x0a, 0x0a, 0xd0, 0x06, 0x06, 0xbf, 0x00, 0x00, 0x8c, 0x00,  // ................
/* 02e0 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 02f0 */ 0x00, 0x08, 0x08, 0xcf, 0x0a, 0x0a, 0xcf, 0x06, 0x06, 0xbe, 0x02, 0x02, 0xa9, 0x00, 0x00, 0x00  // ................
    };

    Bitmap bitmap;
    Bitmap::PrivateData::Data & data = Bitmap::PrivateData::initialize(bitmap, BitsPerPixel{24}, 16, 16);
    memcpy(data.get(), errorbmp, sizeof(errorbmp));
    return bitmap;
}
