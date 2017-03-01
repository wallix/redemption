/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/


#pragma once

#include <stdint.h>
#include <png.h>

#include <cassert>

#include "transport/transport.hpp"
#include "utils/bitfu.hpp"

namespace detail {

    struct NoExceptTransport {
        Transport * trans;
        int         error_id;
    };

    static inline void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length) noexcept {
        try {
            static_cast<NoExceptTransport *>(png_ptr->io_ptr)->trans->send(data, length);
        } catch (...) {
            static_cast<NoExceptTransport *>(png_ptr->io_ptr)->error_id = -1;
        }
    }

    static inline void png_flush_data(png_structp png_ptr) noexcept {
        try {
            static_cast<NoExceptTransport *>(png_ptr->io_ptr)->trans->flush();
        } catch (...) {
            static_cast<NoExceptTransport *>(png_ptr->io_ptr)->error_id = -1;
        }
    }

    template<class IsOk>
    static void dump_png24_impl(
        png_struct * ppng, png_info * pinfo,
        const uint8_t * data, const size_t width, const size_t height, const size_t rowsize,
        const bool bgr, IsOk is_ok
    ) {
        assert(align4(rowsize) == rowsize);

        png_set_IHDR(ppng, pinfo, width, height, 8,
                    PNG_COLOR_TYPE_RGB,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_BASE,
                    PNG_FILTER_TYPE_BASE);
        png_write_info(ppng, pinfo);

        // send image buffer to file, one pixel row at once
        const uint8_t * row = data;

        if (bgr) {
            uint8_t bgrtmp[8192*4];
            for (size_t k = 0 ; k < height && is_ok(); ++k) {
                const uint8_t * s = row;
                uint8_t * t = bgrtmp;
                uint8_t * e = t + (width / 4) * 12;
                for (; t < e; s += 12, t += 12){
                    t[0] = s[2];
                    t[1] = s[1];
                    t[2] = s[0];

                    t[3] = s[5];
                    t[4] = s[4];
                    t[5] = s[3];

                    t[6] = s[8];
                    t[7] = s[7];
                    t[8] = s[6];

                    t[9] = s[11];
                    t[10] = s[10];
                    t[11] = s[9];
                }
                png_write_row(ppng, const_cast<unsigned char*>(bgrtmp));
                row += rowsize;
            }
        }
        else {
            for (size_t k = 0 ; k < height && is_ok(); ++k) {
                png_write_row(ppng, const_cast<unsigned char*>(row));
                row += rowsize;
            }
        }
    }
}

static inline void transport_dump_png24(Transport & trans, const uint8_t * data,
                            const size_t width,
                            const size_t height,
                            const size_t rowsize,
                            const bool bgr)
{
    detail::NoExceptTransport no_except_transport = { &trans, 0 };

    png_struct * ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_set_write_fn(ppng, &no_except_transport, &detail::png_write_data, &detail::png_flush_data);

    png_info * pinfo = png_create_info_struct(ppng);

    detail::dump_png24_impl(
        ppng, pinfo, data, width, height, rowsize, bgr,
        [&]() noexcept {return !no_except_transport.error_id;}
    );

    if (!no_except_transport.error_id) {
        png_write_end(ppng, pinfo);
        trans.flush();
    }

    png_destroy_write_struct(&ppng, &pinfo);
    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, fd);
}

static inline void dump_png24(FILE * fd, const uint8_t * data,
                            const size_t width,
                            const size_t height,
                            const size_t rowsize,
                            const bool bgr)
{
    png_struct * ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_info * pinfo = png_create_info_struct(ppng);

    // prepare png header
    png_init_io(ppng, fd);

    detail::dump_png24_impl(ppng, pinfo, data, width, height, rowsize, bgr, []{return true;});

    png_write_end(ppng, pinfo);
    png_destroy_write_struct(&ppng, &pinfo);
    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, fd);
}

inline void read_png24(FILE * fd, const uint8_t * data,
                      const size_t width,
                      const size_t height,
                      const size_t rowsize)
{
    (void)width;
    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_info * pinfo = png_create_info_struct(ppng);
    png_init_io(ppng, fd);
    png_read_info(ppng, pinfo);

    for (size_t k = 0 ; k < height ; ++k) {
        png_read_row(ppng, const_cast<unsigned char*>(data), nullptr);
        data += rowsize;
    }
    png_read_end(ppng, pinfo);
    png_destroy_read_struct(&ppng, &pinfo, nullptr);
}

static inline void png_read_data_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
    uint8_t * tmp_data = data;
    static_cast<Transport*>(png_ptr->io_ptr)->recv_new(tmp_data, length);
}


inline void transport_read_png24(Transport * trans, const uint8_t * data,
                      const size_t width,
                      const size_t height,
                      const size_t rowsize)
{
    (void)width;
    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_set_read_fn(ppng, trans, &png_read_data_fn);
    png_info * pinfo = png_create_info_struct(ppng);
    png_read_info(ppng, pinfo);

    for (size_t k = 0 ; k < height ; ++k) {
        png_read_row(ppng, const_cast<unsigned char*>(data), nullptr);
        data += rowsize;
    }
    png_read_end(ppng, pinfo);
    png_destroy_read_struct(&ppng, &pinfo, nullptr);
}

