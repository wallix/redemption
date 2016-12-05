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
   Author(s): Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestNewVideo
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "capture/video_capture.hpp"
#include "capture/sequencer.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "utils/drawable.hpp"

struct VideoSequencerAction
{
    VideoCapture & recorder;

    void operator()(const timeval&) const {
        this->recorder.next_video();
    }
};

using VideoSequencer = SequencerCapture<VideoSequencerAction>;


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
        Drawable drawable(width, height);

        VideoCapture flvgen(now, trans, drawable, false, {width, height, 25, 15, 100000, "flv", 0});
        VideoSequencer flvseq(now, std::chrono::microseconds{2 * 1000000l}, VideoSequencerAction{flvgen});

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 250; x++) {
            drawable.opaquerect(r.intersect(screen), color1);
            r.y += vy;
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
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
        Drawable drawable(width, height);

        VideoCapture flvgen(now, trans, drawable, false, {width, height, 25, 15, 100000, "mp4", 0});
        VideoSequencer flvseq(now, std::chrono::microseconds{2 * 1000000l}, VideoSequencerAction{flvgen});

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 100; x++) {
            drawable.opaquerect(r.intersect(screen), color1);
            r.y += vy;
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
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
    BOOST_CHECK_EQUAL(15663, filesize(filename));
    ::unlink(filename);
    filename = (file_gen.get(1));
    BOOST_CHECK_EQUAL(16090, filesize(filename));
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
        Drawable drawable(width, height);

        VideoCapture flvgen(now, trans, drawable, false, {width, height, 25, 15, 100000, "flv", 0});
        VideoSequencer flvseq(now, std::chrono::microseconds{1000 * 1000000l}, VideoSequencerAction{flvgen});

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 1000 ; x++) {
            r.y += vy;
            drawable.opaquerect(r.intersect(screen), color1);
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
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
        Drawable drawable(width, height);

        VideoCapture flvgen(now, trans, drawable, false, {width, height, 25, 15, 100000, "flv", 0});
        VideoSequencer flvseq(now, std::chrono::microseconds{1000 * 1000000l}, VideoSequencerAction{flvgen});

        auto const color1 = drawable.u32bgr_to_color(BLUE);
        auto const color2 = drawable.u32bgr_to_color(WABGREEN);
        auto const color3 = drawable.u32bgr_to_color(RED);

        Rect screen(0, 0, width, height);
        drawable.opaquerect(screen, color1);
        uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
        Rect r(10, 10, 50, 25);
        Rect r1(10, 10 + 25, 50, 25);
        int vx = 5;
        int vy = 4;
        bool ignore_frame_in_timeval = false;
        for (size_t x = 0; x < 1000 ; x++) {
            drawable.opaquerect(r.intersect(screen), color1);
            drawable.opaquerect(r1.intersect(screen), color1);
            r.y += vy;
            r.x += vx;
            drawable.opaquerect(r.intersect(screen), color2);
            usec += 40000LL;
            now.tv_sec  = usec / 1000000LL;
            now.tv_usec = (usec % 1000000LL);
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
            flvgen.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            flvseq.snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
            r1.y += vy;
            r1.x += vx;
            drawable.opaquerect(r1.intersect(screen), color3);

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
