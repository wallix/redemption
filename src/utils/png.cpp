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

#include "gdi/graphic_api.hpp"
#include "transport/transport.hpp"
#include "transport/file_transport.hpp"
#include "utils/parse.hpp"
#include "utils/bitfu.hpp"
#include "utils/sugar/array_view.hpp"
#include "capture/wrm_capture.hpp"

#include <cassert>
#include <cstdint>

#include <png.h>

namespace detail
{
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

    struct PngWriteStruct
    {
        png_struct * ppng = nullptr;
        png_info * pinfo = nullptr;

        PngWriteStruct()
        {
            ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            pinfo = png_create_info_struct(ppng);
        }

        ~PngWriteStruct()
        {
            png_destroy_write_struct(&ppng, &pinfo);
        }
    };

    struct PngReadStruct
    {
        png_struct * ppng = nullptr;
        png_info * pinfo = nullptr;

        PngReadStruct()
        {
            ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            pinfo = png_create_info_struct(ppng);
        }

        ~PngReadStruct()
        {
            png_destroy_read_struct(&ppng, &pinfo, nullptr);
        }
    };

    template<class TransportType>
    void transport_dump_png24(
        TransportType & trans, const uint8_t * data,
        const size_t width, const size_t height, const size_t rowsize,
        const bool bgr
    ) {
        struct NoExceptTransport
        {
            TransportType & trans;
            int         error_id;
        };

        auto png_write_data = [](png_structp png_ptr, png_bytep data, png_size_t length) noexcept {
            try {
                static_cast<NoExceptTransport*>(png_ptr->io_ptr)->trans.send(data, length);
            } catch (...) {
                static_cast<NoExceptTransport*>(png_ptr->io_ptr)->error_id = -1;
                png_error(png_ptr, "Exception in Transport::send");
            }
        };

        auto png_flush_data = [](png_structp png_ptr) noexcept {
            try {
                static_cast<NoExceptTransport*>(png_ptr->io_ptr)->trans.flush();
            } catch (...) {
                static_cast<NoExceptTransport*>(png_ptr->io_ptr)->error_id = -1;
                png_error(png_ptr, "Exception in Transport::flush");
            }
        };

        NoExceptTransport no_except_transport = { trans, 0 };

        PngWriteStruct png;
        png_set_write_fn(png.ppng, &no_except_transport, png_write_data, png_flush_data);

        detail::dump_png24_impl(
            png.ppng, png.pinfo, data, width, height, rowsize, bgr,
            [&]() noexcept { return !no_except_transport.error_id; }
        );

        if (!no_except_transport.error_id) {
            png_write_end(png.ppng, png.pinfo);
            trans.flush();
        }

        // commented line below it to create row capture
        // fwrite(this->data, 3, this->width * this->height, fd);
    }
}

void transport_dump_png24(
    Transport & trans, uint8_t const * data,
    const size_t width, const size_t height, const size_t rowsize,
    const bool bgr
) {
    detail::transport_dump_png24(trans, data, width, height, rowsize, bgr);
}

void file_transport_dump_png24(
    FileTransport & trans, uint8_t const * data,
    const size_t width, const size_t height, const size_t rowsize,
    const bool bgr
) {
    detail::transport_dump_png24(trans, data, width, height, rowsize, bgr);
}

void dump_png24(
    std::FILE * fd, uint8_t const * data,
    const size_t width, const size_t height, const size_t rowsize,
    const bool bgr
) {
    detail::PngWriteStruct png;

    // prepare png header
    png_init_io(png.ppng, fd);

    detail::dump_png24_impl(png.ppng, png.pinfo, data, width, height, rowsize, bgr, []{return true;});

    png_write_end(png.ppng, png.pinfo);

    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, fd);
}

void read_png24(
    std::FILE * fd, uint8_t * data,
    const size_t width, const size_t height, const size_t rowsize
) {
    (void)width;

    detail::PngReadStruct png;
    png_init_io(png.ppng, fd);
    png_read_info(png.ppng, png.pinfo);

    for (size_t k = 0 ; k < height ; ++k) {
        png_read_row(png.ppng, const_cast<unsigned char*>(data), nullptr);
        data += rowsize;
    }

    png_read_end(png.ppng, png.pinfo);
}

void transport_read_png24(
    Transport & trans, uint8_t * data,
    const size_t width, const size_t height, const size_t rowsize
) {
    (void)width;

    auto png_read_data_fn = [](png_structp png_ptr, png_bytep data, png_size_t length) {
        // TODO catch exception ?
        static_cast<Transport*>(png_ptr->io_ptr)->recv_new(data, length);
    };

    detail::PngReadStruct png;
    png_set_read_fn(png.ppng, &trans, png_read_data_fn);
    png_read_info(png.ppng, png.pinfo);

    for (size_t k = 0 ; k < height ; ++k) {
        png_read_row(png.ppng, const_cast<unsigned char*>(data), nullptr);
        data += rowsize;
    }

    png_read_end(png.ppng, png.pinfo);
}

void set_rows_from_image_chunk(
    Transport & trans,
    const uint16_t chunk_type,
    const uint32_t chunk_size,
    const size_t width,
    const array_view<gdi::GraphicApi*> graphic_consumers
) {
    struct InChunkedImage
    {
        uint16_t chunk_type;
        uint32_t chunk_size;
        uint16_t chunk_count;
        Transport & trans;
        char buf[65536];
        InStream in_stream;

        InChunkedImage(uint16_t chunk_type, uint32_t chunk_size, Transport & trans)
        : chunk_type(chunk_type)
        , chunk_size(chunk_size)
        , chunk_count(1)
        , trans(trans)
        , in_stream(this->buf, this->chunk_size - 8)
        {
            this->trans.recv_new(this->buf, this->in_stream.get_capacity());
        }
    };

    auto png_read_data_fn = [](png_structp png_ptr, png_bytep buffer, png_size_t len) {
        // TODO catch exception ?
        auto & chunk_trans = *static_cast<InChunkedImage*>(png_ptr->io_ptr);
        size_t total_len = 0;
        while (total_len < len){
            size_t remaining = chunk_trans.in_stream.in_remain();
            if (remaining >= (len - total_len)){
                chunk_trans.in_stream.in_copy_bytes(buffer + total_len, len - total_len);
                //*pbuffer += len;
                return;
            }
            chunk_trans.in_stream.in_copy_bytes(buffer + total_len, remaining);
            total_len += remaining;
            switch (chunk_trans.chunk_type){
            case PARTIAL_IMAGE_CHUNK:
            {
                const size_t header_sz = 8;
                char header_buf[header_sz];
                InStream header(header_buf);
                chunk_trans.trans.recv_new(header_buf, header_sz);
                chunk_trans.chunk_type = header.in_uint16_le();
                chunk_trans.chunk_size = header.in_uint32_le();
                chunk_trans.chunk_count = header.in_uint16_le();
                chunk_trans.in_stream = InStream(chunk_trans.buf, chunk_trans.chunk_size - 8);
                chunk_trans.trans.recv_new(chunk_trans.buf, chunk_trans.chunk_size - 8);
            }
            break;
            case LAST_IMAGE_CHUNK:
                LOG(LOG_ERR, "Failed to read embedded image from WRM (transport closed)");
                png_error(png_ptr, "Failed to read embedded image from WRM (transport closed)");
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            default:
                LOG(LOG_ERR, "Failed to read embedded image from WRM");
                png_error(png_ptr, "Failed to read embedded image from WRM");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
        }
    };

    InChunkedImage chunk_trans(chunk_type, chunk_size, trans);
    detail::PngReadStruct png;
    png_set_read_fn(png.ppng, &chunk_trans, png_read_data_fn);
    png_read_info(png.ppng, png.pinfo);

    size_t height = png_get_image_height(png.ppng, png.pinfo);
    // TODO check png row_size is identical to drawable rowsize

    uint32_t tmp[8192];
    assert(sizeof(tmp) / sizeof(tmp[0]) >= width);
    for (size_t k = 0; k < height; ++k) {
        png_read_row(png.ppng, reinterpret_cast<uint8_t*>(tmp), nullptr);

        uint32_t bgrtmp[8192];
        const uint32_t * s = reinterpret_cast<const uint32_t*>(tmp);
        uint32_t * t = bgrtmp;
        for (size_t n = 0; n < (width / 4); n++){
            unsigned bRGB = *s++;
            unsigned GBrg = *s++;
            unsigned rgbR = *s++;
            *t++ = ((GBrg << 16) & 0xFF000000)
                    | ((bRGB << 16) & 0x00FF0000)
                    | (bRGB         & 0x0000FF00)
                    | ((bRGB >> 16) & 0x000000FF);
            *t++ = (GBrg         & 0xFF000000)
                    | ((rgbR << 16) & 0x00FF0000)
                    | ((bRGB >> 16) & 0x0000FF00)
                    | ( GBrg        & 0x000000FF);
            *t++ = ((rgbR << 16) & 0xFF000000)
                    | (rgbR         & 0x00FF0000)
                    | ((rgbR >> 16) & 0x0000FF00)
                    | ((GBrg >> 16) & 0x000000FF);
        }

        for (gdi::GraphicApi * gd : graphic_consumers){
            gd->set_row(k, reinterpret_cast<uint8_t*>(bgrtmp));
        }
    }
    png_read_end(png.ppng, png.pinfo);
}
