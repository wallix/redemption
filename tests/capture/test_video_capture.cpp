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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "utils/log.hpp"

#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/stream.hpp"

#include "transport/transport.hpp"
#include "transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"
#include "transport/out_meta_sequence_transport.hpp"

#include "capture/capture.hpp"
#include "check_sig.hpp"
#include "get_file_contents.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"

class VideoSequencer : public gdi::CaptureApi
{
    timeval start_break;
    std::chrono::microseconds break_interval;

protected:
    VideoCapture & action;

public:
    VideoSequencer(const timeval & now, std::chrono::microseconds break_interval, VideoCapture & action)
    : start_break(now)
    , break_interval(break_interval)
    , action(action)
    {}

    std::chrono::microseconds get_interval() const {
        return this->break_interval;
    }

    void reset_now(const timeval& now) {
        this->start_break = now;
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        assert(this->break_interval.count());
        auto const interval = difftimeval(now, this->start_break);
        if (interval >= uint64_t(this->break_interval.count())) {
            this->action.next_video();
            this->start_break = now;
        }
        return this->break_interval;
    }
};

BOOST_AUTO_TEST_CASE(TestOpaqueRectVideoCapture)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "opaquerect_videocapture", ".flv", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        RDPDrawable drawable(width, height);

        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "flv", 0};
        VideoCapture flvgen(now, trans, drawable, false, flv_params);
        VideoSequencer flvseq(now, std::chrono::microseconds{2 * 1000000l}, flvgen);

        Rect screen(0, 0, width, height);

        auto const color_cxt = gdi::ColorCtx::depth24();
        drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 250; x++) {
            drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
            r.y += vy;
            r.x += vx;
            drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename;
    filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(40677, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(1));
    BOOST_CHECK_EQUAL(40011, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(2));
    BOOST_CHECK_EQUAL(41172, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(3));
    BOOST_CHECK_EQUAL(40610, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(4));
    BOOST_CHECK_EQUAL(40173, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(5));
    BOOST_CHECK_EQUAL(13539, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestOpaqueRectVideoCaptureMP4)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "opaquerect_videocapture", ".mp4", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        RDPDrawable drawable(width, height);

        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "mp4", 0};
        VideoCapture flvgen(now, trans, drawable, false, flv_params);
        VideoSequencer flvseq(now, std::chrono::microseconds{2 * 1000000l}, flvgen);

        Rect screen(0, 0, width, height);
        
        auto const color_cxt = gdi::ColorCtx::depth24();
        drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 100; x++) {
            drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
            r.y += vy;
            r.x += vx;
            drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    // that's why there are two possible values allowed
    const char * filename = (file_gen.get(0));
    int fsize = filesize(filename);
    switch (fsize) {
        case 12999: break;
        case 12985: break;
        default: BOOST_CHECK_EQUAL(-2, fsize);
    }
    ::unlink(filename);
    filename = (file_gen.get(1));
    fsize = filesize(filename);
    switch (fsize) {
        case 11726: break;
        case 11712: break;
        default: BOOST_CHECK_EQUAL(-2, fsize);
    }
    ::unlink(filename);
    filename = (file_gen.get(2));
    BOOST_CHECK_EQUAL(262, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestOpaqueRectVideoCaptureOneChunk)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "opaquerect_videocapture_one_chunk", ".flv", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        RDPDrawable drawable(width, height);

//        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "flv", 0};
        VideoCapture flvgen(now, trans, drawable, false, {Level::high, width, height, 25, 15, 100000, "flv", 0});
        VideoSequencer flvseq(now, std::chrono::microseconds{1000 * 1000000l}, flvgen);

        Rect screen(0, 0, width, height);

        auto const color_cxt = gdi::ColorCtx::depth24();
        drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 1000 ; x++) {
            r.y += vy;
            drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
            r.x += vx;
            drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(1629235, filesize(filename));
    ::unlink(filename);
}


BOOST_AUTO_TEST_CASE(TestFrameMarker)
{
    const int groupid = 0;
    OutFilenameSequenceSeekableTransport trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        "./", "framemarked_opaquerect_videocapture", ".flv", groupid);

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        const int width  = 800;
        const int height = 600;
        RDPDrawable drawable(width, height);

        FlvParams flv_params{Level::high, width, height, 25, 15, 100000, "flv", 0};
        VideoCapture flvgen(now, trans, drawable, false, flv_params);
        VideoSequencer flvseq(now, std::chrono::microseconds{1000 * 1000000l}, flvgen);

        Rect screen(0, 0, width, height);
        
        auto const color_cxt = gdi::ColorCtx::depth24();
        drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 25);
        Rect r1(10, 10 + 25, 50, 25);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 1000 ; x++) {
            drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
            drawable.draw(RDPOpaqueRect(r1, BLUE), screen, color_cxt);
            r.y += vy;
            r.x += vx;
            drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            r1.y += vy;
            r1.x += vx;
            drawable.draw(RDPOpaqueRect(r1, RED), screen, color_cxt);
            flvgen.preparing_video_frame();

            if ((r.x + r.cx >= width ) || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= height) || (r.y < 0)) { vy = -vy; }
        }
    }

    trans.disconnect();
    auto & file_gen = *trans.seqgen();

    // actual generated files depends on ffmpeg version
    // values below depends on current embedded ffmpeg version
    const char * filename = (file_gen.get(0));
    BOOST_CHECK_EQUAL(734469, filesize(filename));
    ::unlink(filename);
}

