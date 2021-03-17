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

#include "cxx/cxx.hpp"
#include "gdi/graphic_api.hpp"
#include "transport/transport.hpp"
#include "utils/bitfu.hpp"
#include "utils/file.hpp"
#include "utils/image_view.hpp"
#include "utils/log.hpp"
#include "utils/png.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <cassert>
#include <cstdint>
#include <memory>

#include <png.h>

namespace
{
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

    struct DumpPng24Error
    {
        [[noreturn]] void operator()() const
        {
            LOG(LOG_ERR, "dump_png24_impl error");
            throw Error(ERR_RECORDER_SNAPSHOT_FAILED);
        }
    };

    template<class IsOk, class ThrowError>
    static void dump_png24_impl(
        png_struct * ppng, png_info * pinfo,
        const uint8_t * data, const size_t width, const size_t height, const size_t rowsize,
        const bool bgr, IsOk is_ok, ThrowError throw_error
    ) {
        std::unique_ptr<uint8_t[]> dynline;

        if (setjmp(png_jmpbuf(ppng))) {
            dynline.reset();
            throw_error();
        }

        png_set_IHDR(
            ppng, pinfo, width, height, 8,
            PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE,
            PNG_FILTER_TYPE_BASE);
        png_write_info(ppng, pinfo);

        // send image buffer to file, one pixel row at once
        const uint8_t * row = data;

        if (bgr) {
            BufMaker<8192*4> buf_maker;
            uint8_t * bgrtmp = buf_maker.dyn_array(rowsize).data();

            for (size_t k = 0 ; k < height && is_ok(); ++k) {
                const uint8_t * s = row;
                uint8_t * t = bgrtmp;

                switch (width % 4) {
                    case 3:
                        t[0] = s[2];
                        t[1] = s[1];
                        t[2] = s[0];
                        t += 3;
                        s += 3;
                        [[fallthrough]];
                    case 2:
                        t[0] = s[2];
                        t[1] = s[1];
                        t[2] = s[0];
                        t += 3;
                        s += 3;
                        [[fallthrough]];
                    case 1:
                        t[0] = s[2];
                        t[1] = s[1];
                        t[2] = s[0];
                        t += 3;
                        s += 3;
                }

                uint8_t * e = t + (width / 4u) * 12u;
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
                png_write_row(ppng, bgrtmp);
                row += rowsize;
            }
        }
        else {
            for (size_t k = 0 ; k < height && is_ok(); ++k) {
                png_write_row(ppng, const_cast<uint8_t*>(row)); /* NOLINT */
                row += rowsize;
            }
        }
    }

    void read_png24_impl(
        PngReadStruct & png, uint8_t * data,
        const size_t width, const size_t height, const size_t rowsize)
    {
        png_read_info(png.ppng, png.pinfo);

        assert(height == png_get_image_height(png.ppng, png.pinfo));
        assert(width == png_get_image_width(png.ppng, png.pinfo));
        assert(width <= rowsize);

        (void)width;

        for (size_t k = 0; k < height; ++k) {
            png_read_row(png.ppng, data, nullptr);
            data += rowsize;
        }

        png_read_end(png.ppng, png.pinfo);
    }
} // namespace

struct NoExceptTransport
{
    Transport & trans;
    bool        has_error;
    Error       err;

    template<class F>
    static void exec(png_structp png_ptr, char const* name, F f) noexcept
    {
        auto& self = *static_cast<NoExceptTransport*>(png_get_io_ptr(png_ptr));
        try {
            f(self.trans);
        } catch (Error const& err) {
            self.fail(png_ptr, "Exception in dump_png24", name, &err);
        } catch (...) {
            self.fail(png_ptr, "Exception in dump_png24", name, nullptr);
        }
    }

    [[noreturn]]
    void fail(png_structp png_ptr, char const* msg, char const* name, Error const* err) noexcept
    {
        this->has_error = true;
        if (err) {
            this->err = *err;
            if (this->err.id == NO_ERROR) {
                this->err.id = ERR_RECORDER_SNAPSHOT_FAILED;
            }
            LOG(LOG_ERR, "%s (%s): %s", name, msg, err->errmsg());
        }
        else {
            LOG(LOG_ERR, "%s (%s)", name, msg);
        }
        png_error(png_ptr, msg);
        REDEMPTION_UNREACHABLE();
    }
};

void dump_png24(
    Transport & trans, uint8_t const * data,
    const size_t width, const size_t height, const size_t rowsize,
    const bool bgr
) {
    auto png_write_data = [](png_structp png_ptr, png_bytep data, png_size_t length) noexcept {
        NoExceptTransport::exec(png_ptr, "send", [&](Transport& trans){
            trans.send(data, length);
        });
    };

    auto png_flush_data = [](png_structp png_ptr) noexcept {
        NoExceptTransport::exec(png_ptr, "send", [](Transport& trans){
            trans.flush();
        });
    };

    NoExceptTransport no_except_transport = { trans, false, Error(NO_ERROR) };

    PngWriteStruct png;
    png_set_write_fn(png.ppng, &no_except_transport, png_write_data, png_flush_data);

    dump_png24_impl(
        png.ppng, png.pinfo, data, width, height, rowsize, bgr,
        [&]() noexcept { return !no_except_transport.has_error; },
        [&]() { throw no_except_transport.err; }
    );

    if (!no_except_transport.has_error) {
        png_write_end(png.ppng, png.pinfo);
        trans.flush();
    }
}

void dump_png24(
    std::FILE * file, uint8_t const * data,
    const size_t width, const size_t height, const size_t rowsize,
    const bool bgr
) {
    PngWriteStruct png;

    // prepare png header
    png_init_io(png.ppng, file);

    dump_png24_impl(
        png.ppng, png.pinfo, data, width, height, rowsize, bgr,
        []{return true;}, DumpPng24Error{});

    png_write_end(png.ppng, png.pinfo);

    // commented line below it to create row capture
    // fwrite(this->data, 3, this->width * this->height, file);
}

void dump_png24(
    char const* filename, uint8_t const * data,
    const size_t width, const size_t height, const size_t rowsize,
    const bool bgr
) {
    if (File f{filename, "wb"}) {
        dump_png24(f.get(), data, width, height, rowsize, bgr);
    }
}

void dump_png24(Transport & trans, ImageView const & image_view, bool bgr)
{
    ::dump_png24(
        trans, image_view.data(),
        image_view.width(), image_view.height(),
        image_view.line_size(),
        bgr);
}

void dump_png24(std::FILE * file, ImageView const & image_view, bool bgr)
{
    assert(BytesPerPixel{3} == image_view.bytes_per_pixel());

    PngWriteStruct png;

    // prepare png header
    png_init_io(png.ppng, file);

    dump_png24_impl(
        png.ppng, png.pinfo,
        image_view.data(),
        image_view.width(), image_view.height(),
        image_view.line_size(),
        bgr, []{return true;}, DumpPng24Error{});

    png_write_end(png.ppng, png.pinfo);
}

void dump_png24(const char * filename, ImageView const & image_view, bool bgr)
{
    if (File f{filename, "wb"}) {
        dump_png24(f.get(), image_view, bgr);
    }
}


void read_png24(const char * filename, WritableImageView const & mutable_image_view)
{
    if (File f{filename, "r"}) {
        read_png24(f.get(), mutable_image_view);
    }
}

void read_png24(std::FILE * file, WritableImageView const & mutable_image_view)
{
    PngReadStruct png;
    png_init_io(png.ppng, file);
    read_png24_impl(png, mutable_image_view.mutable_data(),
        mutable_image_view.width(), mutable_image_view.height(),
        mutable_image_view.line_size());
}

void read_png24(Transport & trans, WritableImageView const & mutable_image_view)
{
    assert(BytesPerPixel{3} == mutable_image_view.bytes_per_pixel());

    auto png_read_data_fn = [](png_structp png_ptr, png_bytep data, png_size_t length) {
       // TODO catch exception ?
       // static_cast<Transport*>(png_ptr->io_ptr)->recv_boom(data, length);
        static_cast<Transport*>(png_get_io_ptr(png_ptr))->recv_boom(data, length);
    };

    PngReadStruct png;
    png_set_read_fn(png.ppng, &trans, png_read_data_fn);
    read_png24_impl(png, mutable_image_view.mutable_data(),
        mutable_image_view.width(), mutable_image_view.height(),
        mutable_image_view.line_size());
}

// TODO void read_png24_by_line(read_fn:size_t(writable_bytes_view), f:void(bytes_view))
void set_rows_from_image_chunk(
    Transport & trans,
    const WrmChunkType chunk_type,
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

        InChunkedImage(WrmChunkType chunk_type, uint32_t chunk_size, Transport & trans)
        : chunk_type(safe_cast<uint16_t>(chunk_type))
        , chunk_size(chunk_size)
        , chunk_count(1)
        , trans(trans)
        , in_stream({this->buf, this->chunk_size - 8})
        {
            this->trans.recv_boom(this->buf, this->in_stream.get_capacity());
        }
    };

    auto png_read_data_fn = [](png_structp png_ptr, png_bytep buffer, png_size_t len) {
        // TODO catch exception ?
        //        auto & chunk_trans = *static_cast<InChunkedImage*>(png_ptr->io_ptr);
        auto & chunk_trans = *static_cast<InChunkedImage*>(png_get_io_ptr(png_ptr));
        size_t total_len = 0;
        char const * msg_error = "Failed to read embedded image from WRM";
        while (total_len < len){
            size_t remaining = chunk_trans.in_stream.in_remain();
            if (remaining >= (len - total_len)){
                chunk_trans.in_stream.in_copy_bytes(buffer + total_len, len - total_len);
                //*pbuffer += len;
                return;
            }
            chunk_trans.in_stream.in_copy_bytes(buffer + total_len, remaining);
            total_len += remaining;
            REDEMPTION_DIAGNOSTIC_PUSH()
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
            switch (safe_cast<WrmChunkType>(chunk_trans.chunk_type)) {
            case WrmChunkType::PARTIAL_IMAGE_CHUNK:
            {
                const size_t header_sz = 8;
                char header_buf[header_sz];
                InStream header(header_buf);
                chunk_trans.trans.recv_boom(header_buf, header_sz);
                chunk_trans.chunk_type = header.in_uint16_le();
                chunk_trans.chunk_size = header.in_uint32_le();
                chunk_trans.chunk_count = header.in_uint16_le();
                auto av = chunk_trans.trans.recv_boom(chunk_trans.buf, chunk_trans.chunk_size - 8);
                chunk_trans.in_stream = InStream(av);
            }
            break;
            case WrmChunkType::LAST_IMAGE_CHUNK:
                msg_error = "Failed to read embedded image from WRM (transport closed)";
                REDEMPTION_CXX_FALLTHROUGH;
            default:
                LOG(LOG_ERR, "%s", msg_error);
                png_error(png_ptr, msg_error);
            }
            REDEMPTION_DIAGNOSTIC_POP()
        }
    };

    InChunkedImage chunk_trans(chunk_type, chunk_size, trans);
    PngReadStruct png;
    png_set_read_fn(png.ppng, &chunk_trans, png_read_data_fn);

#if PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 4)
    if (setjmp(png_jmpbuf(png.ppng)))
#else
    if (setjmp(png.ppng->jmpbuf))
#endif
    {
        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

    png_read_info(png.ppng, png.pinfo);

    size_t height = png_get_image_height(png.ppng, png.pinfo);
    size_t rowsize = png_get_rowbytes(png.ppng, png.pinfo);
    // TODO check png row_size is identical to drawable rowsize

    uint8_t tmp[8192*4];
    assert(sizeof(tmp) / sizeof(tmp[0]) / 4 >= width);
    for (size_t k = 0; k < height; ++k) {
        png_read_row(png.ppng, tmp, nullptr);

        uint8_t * t = tmp;
        const uint8_t * e = t + (width / 4) * 12;

        for (; t < e; t += 12){
            using std::swap;
            swap(t[0], t[2]);
            swap(t[3], t[5]);
            swap(t[6], t[8]);
            swap(t[9], t[11]);
        }

        for (gdi::GraphicApi * gd : graphic_consumers){
            gd->set_row(k, {tmp, rowsize});
        }
    }
    png_read_end(png.ppng, png.pinfo);
}
