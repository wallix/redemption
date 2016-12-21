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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFileToGraphic
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "utils/dump_png24_from_rdp_drawable_adapter.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/in_file_transport.hpp"
#include "capture/capture.hpp"
#include "capture/FileToGraphic.hpp"

class DrawableToFile
{
protected:
    Transport & trans;
    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    const Drawable & drawable;

private:
    std::unique_ptr<uint8_t[]> scaled_buffer;

public:
    DrawableToFile(Transport & trans, const Drawable & drawable, unsigned zoom)
    : trans(trans)
    , zoom_factor(std::min(zoom, 100u))
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    {
        const unsigned zoom_width = (this->drawable.width() * this->zoom_factor) / 100;
        const unsigned zoom_height = (this->drawable.height() * this->zoom_factor) / 100;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    ~DrawableToFile() = default;

    /// \param  percent  0 to 100 or 100 if greater
    void zoom(unsigned percent) {
        percent = std::min(percent, 100u);
        const unsigned zoom_width = (this->drawable.width() * percent) / 100;
        const unsigned zoom_height = (this->drawable.height() * percent) / 100;
        this->zoom_factor = percent;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    bool logical_frame_ended() const {
        return this->drawable.logical_frame_ended;
    }

    void flush() {
        if (this->zoom_factor == 100) {
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

private:
    void dump24() const {
        ::transport_dump_png24(
            this->trans, this->drawable.data(),
            this->drawable.width(), this->drawable.height(),
            this->drawable.rowsize(), true);
    }

    void scale_dump24() const {
        scale_data(
            this->scaled_buffer.get(), this->drawable.data(),
            this->scaled_width, this->drawable.width(),
            this->scaled_height, this->drawable.height(),
            this->drawable.rowsize());
        ::transport_dump_png24(
            this->trans, this->scaled_buffer.get(),
            this->scaled_width, this->scaled_height,
            this->scaled_width * 3, false);
    }

    static void scale_data(uint8_t *dest, const uint8_t *src,
                           unsigned int dest_width, unsigned int src_width,
                           unsigned int dest_height, unsigned int src_height,
                           unsigned int src_rowsize) {
        const uint32_t Bpp = 3;
        unsigned int y_pixels = dest_height;
        unsigned int y_int_part = src_height / dest_height * src_rowsize;
        unsigned int y_fract_part = src_height % dest_height;
        unsigned int yE = 0;
        unsigned int x_int_part = src_width / dest_width * Bpp;
        unsigned int x_fract_part = src_width % dest_width;

        while (y_pixels-- > 0) {
            unsigned int xE = 0;
            const uint8_t * x_src = src;
            unsigned int x_pixels = dest_width;
            while (x_pixels-- > 0) {
                dest[0] = x_src[2];
                dest[1] = x_src[1];
                dest[2] = x_src[0];

                dest += Bpp;
                x_src += x_int_part;
                xE += x_fract_part;
                if (xE >= dest_width) {
                    xE -= dest_width;
                    x_src += Bpp;
                }
            }
            src += y_int_part;
            yE += y_fract_part;
            if (yE >= dest_height) {
                yE -= dest_height;
                src += src_rowsize;
            }
        }
    }
};


BOOST_AUTO_TEST_CASE(TestSample0WRM)
{
    const char * input_filename = FIXTURES_PATH "/sample0.wrm";

    int fd = ::open(input_filename, O_RDONLY);
    if (fd == -1){
        LOG(LOG_INFO, "open '%s' failed with error : %s", input_filename, strerror(errno));
        BOOST_CHECK(false);
        return;
    }

    InFileTransport in_wrm_trans(fd);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(0));

    const int groupid = 0;
    OutFilenameSequenceTransport out_png_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "first", ".png", groupid);
    RDPDrawable drawable1(player.screen_rect.cx, player.screen_rect.cy, 24);
    DrawableToFile png_recorder(out_png_trans, drawable1.impl(), 100);

//    png_recorder.update_config(ini);
    player.add_consumer(&drawable1, nullptr, nullptr, nullptr, nullptr);

    OutFilenameSequenceTransport out_wrm_trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "first", ".wrm", groupid);

    const struct ToCacheOption {
        ToCacheOption(){}
        BmpCache::CacheOption operator()(const BmpCache::cache_ & cache) const {
            return BmpCache::CacheOption(cache.entries(), cache.bmp_size(), cache.persistent());
        }
    } to_cache_option;

    BmpCache bmp_cache(
        BmpCache::Recorder,
        player.bmp_cache->bpp,
        player.bmp_cache->number_of_cache,
        player.bmp_cache->use_waiting_list,
        to_cache_option(player.bmp_cache->get_cache(0)),
        to_cache_option(player.bmp_cache->get_cache(1)),
        to_cache_option(player.bmp_cache->get_cache(2)),
        to_cache_option(player.bmp_cache->get_cache(3)),
        to_cache_option(player.bmp_cache->get_cache(4))
    );
    GlyphCache gly_cache;
    PointerCache ptr_cache;

    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy, 24);
    DumpPng24FromRDPDrawableAdapter dump_png{drawable};
    GraphicToFile graphic_to_file(
        player.record_now,
        out_wrm_trans,
        player.screen_rect.cx,
        player.screen_rect.cy,
        24,
        bmp_cache, gly_cache, ptr_cache, dump_png, WrmCompressionAlgorithm::no_compression
    );
    NativeCapture wrm_recorder(graphic_to_file, player.record_now, std::chrono::seconds{1}, std::chrono::seconds{20});

    player.add_consumer(&drawable, nullptr, nullptr, nullptr, nullptr);
    player.add_consumer(&graphic_to_file, &wrm_recorder, nullptr, nullptr, &wrm_recorder);

    bool requested_to_stop = false;

    BOOST_CHECK_EQUAL(1352304810u, static_cast<unsigned>(player.record_now.tv_sec));
    player.play(requested_to_stop);

    png_recorder.flush();
    BOOST_CHECK_EQUAL(1352304870u, static_cast<unsigned>(player.record_now.tv_sec));

    graphic_to_file.sync();
    const char * filename;

    out_png_trans.disconnect();
    out_wrm_trans.disconnect();

    filename = out_png_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(21280, ::filesize(filename));
    ::unlink(filename);

    filename = out_wrm_trans.seqgen()->get(0);
    BOOST_CHECK_EQUAL(490454, ::filesize(filename));
    ::unlink(filename);
    filename = out_wrm_trans.seqgen()->get(1);
    BOOST_CHECK_EQUAL(1008253, ::filesize(filename));
    ::unlink(filename);
    filename = out_wrm_trans.seqgen()->get(2);
    BOOST_CHECK_EQUAL(195756, ::filesize(filename));
    ::unlink(filename);
}


