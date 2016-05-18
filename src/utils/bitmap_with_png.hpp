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

#ifndef _REDEMPTION_UTILS_BITMAP_WITH_PNG_HPP__
#define _REDEMPTION_UTILS_BITMAP_WITH_PNG_HPP__

//#include "utils/png_bitmap.hpp"
#include "utils/png.hpp"
#include "utils/bitmap_with_png.hpp"

#include "error.h"
#include <png.h>



using std::size_t;

class Bitmap_PNG : public Bitmap
{


public:
    using Bitmap::Bitmap;

    enum openfile_t {
        OPEN_FILE_UNKNOWN,
        OPEN_FILE_BMP,
        OPEN_FILE_PNG
    };

    openfile_t check_file_type(const char * filename) {
        char type1[8];

        int fd_ = open(filename, O_RDONLY);
        if (!fd_) {
            LOG(LOG_ERR, "Widget_load: error loading bitmap from file [%s] %s(%u)\n",
                filename, strerror(errno), errno);
            TODO("see value returned, maybe we should return open error");
            return OPEN_FILE_UNKNOWN;
        }

        class fdbuf
        {
            int fd;
        public:
            explicit fdbuf(int fd = -1) noexcept : fd(fd){}

            ~fdbuf(){::close(this->fd);}
            ssize_t read(void * data, size_t len) const
            {
                size_t remaining_len = len;
                while (remaining_len) {
                    ssize_t ret = ::read(this->fd, static_cast<char*>(data)
                                        +(len - remaining_len), remaining_len);
                    if (ret < 0){
                        if (errno == EINTR){
                            continue;
                        }
                        // Error should still be there next time we try to read
                        if (remaining_len != len){
                            return len - remaining_len;
                        }
                        return ret;
                    }
                    // We must exit loop or we will enter infinite loop
                    if (ret == 0){
                        break;
                    }
                    remaining_len -= ret;
                }
                return len - remaining_len;
            }

            off64_t seek(off64_t offset, int whence) const
            {
                return lseek64(this->fd, offset, whence);
            }
        } file(fd_);

        if (file.read(type1, 2) != 2) {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
            return OPEN_FILE_UNKNOWN;
        }
        if ((type1[0] == 'B') && (type1[1] == 'M')) {
            LOG(LOG_INFO, "Widget_load: image file [%s] is BMP file\n", filename);
            return OPEN_FILE_BMP;
        }
        if (file.read(&type1[2], 6) != 6) {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
            return OPEN_FILE_BMP;
        }
        if (png_check_sig(reinterpret_cast<png_bytep>(type1), 8)) {
            LOG(LOG_INFO, "Widget_load: image file [%s] is PNG file\n", filename);
            return OPEN_FILE_PNG;
        }
        LOG(LOG_ERR, "Widget_load: error bitmap file [%s] not BMP or PNG file\n", filename);
        return OPEN_FILE_UNKNOWN;
    }

    TODO("I could use some data provider lambda instead of filename")
    explicit Bitmap_PNG(const char* filename)
    : Bitmap()
    {
        //LOG(LOG_INFO, "loading bitmap %s", filename);

        openfile_t res = Bitmap_PNG::check_file_type(filename);

        if (res == OPEN_FILE_UNKNOWN) {
            LOG(LOG_ERR, "loading bitmap %s failed, Unknown format type", filename);
            this->load_error_bitmap();
            // throw Error(ERR_BITMAP_LOAD_UNKNOWN_TYPE_FILE);
        }
        else if (res == OPEN_FILE_PNG) {
            bool bres = this->open_png_file(filename);
            if (!bres) {
                LOG(LOG_ERR, "loading bitmap %s failed", filename);
                this->load_error_bitmap();
                // throw Error(ERR_BITMAP_PNG_LOAD_FAILED);
            }
        }
        else {
            BGRPalette palette1{BGRPalette::no_init()};

            /* header for bmp file */
            struct bmp_header {
                size_t size;
                unsigned image_width;
                unsigned image_height;
                short planes;
                short bit_count;
                int compression;
                int image_size;
                int x_pels_per_meter;
                int y_pels_per_meter;
                int clr_used;
                int clr_important;
                bmp_header() {
                    this->size = 0;
                    this->image_width = 0;
                    this->image_height = 0;
                    this->planes = 0;
                    this->bit_count = 0;
                    this->compression = 0;
                    this->image_size = 0;
                    this->x_pels_per_meter = 0;
                    this->y_pels_per_meter = 0;
                    this->clr_used = 0;
                    this->clr_important = 0;
                }
            } header;

            uint8_t stream_data[8192];
            std::unique_ptr<uint8_t[]> stream_dyndata;
            InStream stream(stream_data);

            TODO(" reading of file and bitmap decoding should be kept appart  putting both together makes testing hard. And what if I want to read a bitmap from some network socket instead of a disk file ?");
            {
                int const fd_ = ::open(filename, O_RDONLY);
                if (fd_ == -1) {
                    LOG(LOG_ERR, "Widget_load: error loading bitmap from file [%s] %s(%u)\n", filename, strerror(errno), errno);
                    this->load_error_bitmap();
                    // throw Error(ERR_BITMAP_LOAD_FAILED);
                    return ;
                }

                class fdbuf
                {
                    int fd;
                public:
                    explicit fdbuf(int fd = -1) noexcept : fd(fd){}

                    ~fdbuf(){::close(this->fd);}
                    ssize_t read(void * data, size_t len) const
                    {
                        size_t remaining_len = len;
                        while (remaining_len) {
                            ssize_t ret = ::read(this->fd, static_cast<char*>(data)
                                                +(len - remaining_len), remaining_len);
                            if (ret < 0){
                                if (errno == EINTR){
                                    continue;
                                }
                                // Error should still be there next time we try to read
                                if (remaining_len != len){
                                    return len - remaining_len;
                                }
                                return ret;
                            }
                            // We must exit loop or we will enter infinite loop
                            if (ret == 0){
                                break;
                            }
                            remaining_len -= ret;
                        }
                        return len - remaining_len;
                    }

                    off64_t seek(off64_t offset, int whence) const
                    {
                        return lseek64(this->fd, offset, whence);
                    }
                } file(fd_);

                char type1[2];

                /* read file type */
                if (file.read(type1, 2) != 2) {
                    LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
                    this->load_error_bitmap();
                    // throw Error(ERR_BITMAP_LOAD_FAILED);
                    return ;
                }
                if ((type1[0] != 'B') || (type1[1] != 'M')) {
                    LOG(LOG_ERR, "Widget_load: error bitmap file [%s] not BMP file\n", filename);
                    this->load_error_bitmap();
                    // throw Error(ERR_BITMAP_LOAD_FAILED);
                    return ;
                }

                /* read file size */
                TODO("define some stream aware function to read data from file (to update stream.end by itself). It should probably not be inside stream itself because read primitives are OS dependant, and there is not need to make stream OS dependant.");
                if (file.read(stream_data, 4) < 4){
                    LOG(LOG_ERR, "Widget_load: error read file size");
                    throw Error(ERR_BITMAP_LOAD_FAILED);
                }
                stream = InStream(stream_data, 4);
                {
                    TODO("Check what is this size ? header size ? used as fixed below ?");
                        /* uint32_t size = */ stream.in_uint32_le();
                }

                // skip some bytes to set file pointer to bmp header
                file.seek(14, SEEK_SET);
                if (file.read(stream_data, 40) < 40){
                    LOG(LOG_ERR, "Widget_load: error read file size (2)");
                    throw Error(ERR_BITMAP_LOAD_FAILED);
                }
                stream = InStream(stream_data, 40);
                TODO(" we should read header size and use it to read header instead of using magic constant 40");
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
                file.seek(14 + header.size, SEEK_SET);

                // compute pixel size (in Quartet) and read palette if needed
                int file_Qpp = 1;
                TODO(" add support for loading of 16 bits bmp from file");
                    switch (header.bit_count) {
                        // Qpp = groups of 4 bytes per pixel
                    case 24:
                        file_Qpp = 6;
                        break;
                    case 8:
                        file_Qpp = 2;
                    case 4:
                        REDASSERT(header.clr_used * 4 <= 8192);
                        if (file.read(stream_data, header.clr_used * 4) < header.clr_used * 4){
                            throw Error(ERR_BITMAP_LOAD_FAILED);
                        }
                        stream = InStream(stream_data, header.clr_used * 4);
                        for (int i = 0; i < header.clr_used; i++) {
                            uint8_t r = stream.in_uint8();
                            uint8_t g = stream.in_uint8();
                            uint8_t b = stream.in_uint8();
                            stream.in_skip_bytes(1); // skip alpha channel
                            palette1.set_color(i, (b << 16)|(g << 8)|r);
                        }
                        break;
                    default:
                        LOG(LOG_ERR, "Widget_load: error bitmap file [%s]"
                            " unsupported bpp %d\n", filename,
                            header.bit_count);
                        throw Error(ERR_BITMAP_LOAD_FAILED);
                    }

                LOG(LOG_INFO, "loading file %u x %u x %d", header.image_width, header.image_height, header.bit_count);

                // bitmap loaded from files are always converted to 24 bits
                // this avoid palette problems for 8 bits,
                // and 4 bits is not supported in other parts of code anyway

                // read bitmap data
                {
                    size_t size = (header.image_width * header.image_height * file_Qpp) / 2;
                    auto p = stream_data;
                    if (size > sizeof(stream_data)) {
                        p = new uint8_t[size];
                        stream_dyndata.reset(p);
                    }
                    int row_size = (header.image_width * file_Qpp) / 2;
                    int padding = align4(row_size) - row_size;
                    for (unsigned y = 0; y < header.image_height; y++) {
                        int k = file.read(p + y * row_size, row_size + padding);
                        if (k != (row_size + padding)) {
                            LOG(LOG_ERR, "Widget_load: read error reading bitmap file [%s] read\n", filename);
                            throw Error(ERR_BITMAP_LOAD_FAILED);
                        }
                    }
                    stream = InStream(p, size);
                }
            }

            const uint8_t Bpp = 3;
            this->data_bitmap = DataBitmap::construct(
                24, static_cast<uint16_t>(header.image_width), static_cast<uint16_t>(header.image_height));
            uint8_t * dest = this->data_bitmap->get();

            const size_t line_size = this->line_size();
            int k = 0;
            for (unsigned y = 0; y < header.image_height ; y++) {
                for (unsigned x = 0 ; x < header.image_width; x++) {
                    uint32_t pixel = 0;
                    switch (header.bit_count){
                    case 24:
                        {
                            uint8_t r = stream.in_uint8();
                            uint8_t g = stream.in_uint8();
                            uint8_t b = stream.in_uint8();
                            pixel = (b << 16) | (g << 8) | r;
                        }
                        break;
                    case 8:
                        pixel = stream.in_uint8();
                        break;
                    case 4:
                        if ((x & 1) == 0) {
                            k = stream.in_uint8();
                            pixel = (k >> 4) & 0xf;
                        }
                        else {
                            pixel = k & 0xf;
                        }
                        pixel = palette1[pixel];
                        break;
                    }

                    uint32_t px = color_decode(pixel, static_cast<uint8_t>(header.bit_count), palette1);
                    ::out_bytes_le(dest + y * line_size + x * Bpp, Bpp, px);
                }
                if (line_size > header.image_width * Bpp){
                    memset(dest + y * line_size + header.image_width * Bpp,
                           0,
                           line_size - header.image_width * Bpp);
                }
            }
        }
    }


    bool open_png_file(const char * filename) {
        this->reset();

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     nullptr, nullptr, nullptr);
        if (!png_ptr) {
            return false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            return false;
        }
        // this handle lib png errors for this call
        if (setjmp(png_ptr->jmpbuf)) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return false;
        }

        FILE * fd = fopen(filename, "rb");
        if (!fd) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return false;
        }
        png_init_io(png_ptr, fd);

        png_read_info(png_ptr, info_ptr);

        png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
        png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_gray_1_2_4_to_8(png_ptr);

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png_ptr);

        if (bit_depth == 16)
            png_set_strip_16(png_ptr);
        else if (bit_depth < 8)
            png_set_packing(png_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);

        if (color_type & PNG_COLOR_MASK_ALPHA) {
            png_set_strip_alpha(png_ptr);
        }
        png_set_bgr(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        TODO("Looks like there's a shift when width is not divisible by 4");
        png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        if (static_cast<uint16_t>(width) * 3 != rowbytes) {
            LOG(LOG_ERR, "PNG Image has bad type");
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return false;
        }

        this->data_bitmap = DataBitmap::construct_png(static_cast<uint16_t>(width), static_cast<uint16_t>(height));
        png_bytep row = this->data_bitmap->get() + rowbytes * height - rowbytes;
        png_bytep * row_pointers = new png_bytep[height];
        for (uint i = 0; i < height; ++i) {
            row_pointers[i] = row - i * rowbytes;
        }
        png_read_image(png_ptr, row_pointers);
        png_read_end(png_ptr, info_ptr);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fd);
        delete [] row_pointers;

        // hexdump_d(this->data_bitmap->get(), this->bmp_size);

        return true;
    } // bool open_png_file(const char * filename)
};



#endif
