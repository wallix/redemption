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

   Unit test to capture interface to video recording to flv or mp4
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


//#define LOGNULL
#define LOGPRINT

#include "utils/log.hpp"

#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/stream.hpp"

#include "transport/transport.hpp"
#include "transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"

#include "check_sig.hpp"
#include "get_file_contents.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"

#include "capture/video_capture.hpp"

void simple_movie(timeval now, unsigned duration, RDPDrawable & drawable, gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse);

void simple_movie(timeval now, unsigned duration, RDPDrawable & drawable, gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse)
{
    Rect screen(0, 0, drawable.width(), drawable.height());
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(screen, BLUE), screen, color_cxt);

    uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
    Rect r(10, 10, 50, 50);
    int vx = 5;
    int vy = 4;
    for (size_t x = 0; x < duration; x++) {
        drawable.draw(RDPOpaqueRect(r, BLUE), screen, color_cxt);
        r.y += vy;
        r.x += vx;
        drawable.draw(RDPOpaqueRect(r, WABGREEN), screen, color_cxt);
        usec += 40000LL;
        now.tv_sec  = usec / 1000000LL;
        now.tv_usec = (usec % 1000000LL);
        //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
        int cursor_x = mouse?r.x + 10:0;
        int cursor_y = mouse?r.y + 10:0;
        drawable.set_mouse_cursor_pos(cursor_x, cursor_y);
        capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if ((r.x + r.cx >= drawable.width())  || (r.x < 0)) { vx = -vx; }
        if ((r.y + r.cy >= drawable.height()) || (r.y < 0)) { vy = -vy; }
    }
}

BOOST_AUTO_TEST_CASE(TestSequencedVideoCapture)
{
    {
        struct notified_on_video_change : public NotifyNextVideo
        {
            void notify_next_video(const timeval& now, reason reason) 
            {
                LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
                    static_cast<unsigned>(now.tv_sec),
                    static_cast<unsigned>(now.tv_usec),
                    static_cast<unsigned>(reason));
            }
        } next_video_notifier;

        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
        SequencedVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_videocapture", 
            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
            std::chrono::microseconds{2 * 1000000l}, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    struct CheckFiles {
        const char * filename;
        size_t size;
    } fileinfo[] = {
        {"./opaquerect_videocapture-000000.png", 3099},
        {"./opaquerect_videocapture-000000.flv", 40677},
        {"./opaquerect_videocapture-000001.png", 3104},
        {"./opaquerect_videocapture-000001.flv", 40011},
        {"./opaquerect_videocapture-000002.png", 3107},
        {"./opaquerect_videocapture-000002.flv", 41172},
        {"./opaquerect_videocapture-000003.png", 3099},
        {"./opaquerect_videocapture-000003.flv", 40610},
        {"./opaquerect_videocapture-000004.png", 3098},
        {"./opaquerect_videocapture-000004.flv", 40173},
        {"./opaquerect_videocapture-000005.png", 3098},
        {"./opaquerect_videocapture-000005.flv", 13539},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        BOOST_CHECK_EQUAL(x.size, fsize);
        ::unlink(x.filename);
    }
}

BOOST_AUTO_TEST_CASE(TestSequencedVideoCaptureMP4)
{
    {
        struct notified_on_video_change : public NotifyNextVideo
        {
            void notify_next_video(const timeval& now, reason reason) 
            {
                LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
                    static_cast<unsigned>(now.tv_sec),
                    static_cast<unsigned>(now.tv_usec),
                    static_cast<unsigned>(reason));
            }
        } next_video_notifier;

        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
        SequencedVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_videocapture", 
            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
            std::chrono::microseconds{2 * 1000000l}, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    struct CheckFiles {
        const char * filename;
        size_t size;
        size_t alternativesize;
    } fileinfo[] = {
        {"./opaquerect_videocapture-000000.png", 3099, 3099},
        {"./opaquerect_videocapture-000000.mp4", 12999, 12985},
        {"./opaquerect_videocapture-000001.png", 3104, 3104},
        {"./opaquerect_videocapture-000001.mp4", 11726, 11712},
        {"./opaquerect_videocapture-000002.png", 3107, 3107},
        {"./opaquerect_videocapture-000002.mp4", 10798, 0},
        {"./opaquerect_videocapture-000003.png", 3099, 3099},
        {"./opaquerect_videocapture-000003.mp4", 11329, 0},
        {"./opaquerect_videocapture-000004.png", 3098, 3098},
        {"./opaquerect_videocapture-000004.mp4", 12331, 9},
        {"./opaquerect_videocapture-000005.png", 3098, 3098},
        {"./opaquerect_videocapture-000005.mp4", 262, 0},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        if (fsize != x.size && fsize != x.alternativesize){
            BOOST_CHECK_EQUAL(x.size, fsize);
        }
        ::unlink(x.filename);
    }
}

BOOST_AUTO_TEST_CASE(TestVideoCaptureOneChunkFLV)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& now, reason reason) 
        {
            LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
                static_cast<unsigned>(now.tv_sec),
                static_cast<unsigned>(now.tv_usec),
                static_cast<unsigned>(reason));
        }
    } next_video_notifier;

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
        SequencedVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_videocapture_one_chunk_xxx", 
            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
            std::chrono::microseconds{1000 * 1000000l}, next_video_notifier);
        simple_movie(now, 1000, drawable, video_capture, false, true);
    }

    struct CheckFiles {
        const char * filename;
        size_t size;
    } fileinfo[] = {
        {"./opaquerect_videocapture_one_chunk_xxx-000000.png", 3099},
        {"./opaquerect_videocapture_one_chunk_xxx-000000.flv", 645722},
        {"./opaquerect_videocapture_one_chunk_xxx-000001.png", static_cast<long unsigned>(-1)},
        {"./opaquerect_videocapture_one_chunk_xxx-000001.flv", static_cast<long unsigned>(-1)},
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        BOOST_CHECK_EQUAL(x.size, fsize);
        ::unlink(x.filename);
    }
}

BOOST_AUTO_TEST_CASE(TestFullVideoCaptureFlv)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
        FullVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_fullvideocapture_timestamp1", 
            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }
    const char * filename = "./opaquerect_fullvideocapture_timestamp1.flv";
    BOOST_CHECK_EQUAL(164693, filesize(filename));
    ::unlink(filename);

}

BOOST_AUTO_TEST_CASE(TestFullVideoCaptureFlv2)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
        FullVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_fullvideocapture_timestamp_mouse0", 
            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
        simple_movie(now, 250, drawable, video_capture, false, false);
    }
    const char * filename = "./opaquerect_fullvideocapture_timestamp_mouse0.flv";
    BOOST_CHECK_EQUAL(158699, filesize(filename));
    ::unlink(filename);

}

BOOST_AUTO_TEST_CASE(TestFullVideoCaptureX264)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
        FullVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_fullvideocapture_timestamp2",
            0 /* groupid */, false /* no_timestamp */, drawable, flv_params);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }
    const char * filename = "./opaquerect_fullvideocapture_timestamp2.mp4";
    size_t fsize = filesize(filename);
     // size actually depends on the codec version and at least two slightltly different ones exists for h264
    if (fsize != 54190 && fsize != 54176){
        BOOST_CHECK_EQUAL(54190, filesize(filename));
    }
    ::unlink(filename);

}

BOOST_AUTO_TEST_CASE(SequencedVideoCaptureFLV)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& now, reason reason) 
        {
            LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
                static_cast<unsigned>(now.tv_sec),
                static_cast<unsigned>(now.tv_usec),
                static_cast<unsigned>(reason));
        }
    } next_video_notifier;

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", 0};
        SequencedVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_seqvideocapture",
            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
            std::chrono::microseconds{1000000}, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }
    
    struct CheckFiles {
        const char * filename;
        size_t size;
    } fileinfo[] = {
        {"./opaquerect_seqvideocapture-000000.png", 3099},
        {"./opaquerect_seqvideocapture-000000.flv", 29439},
        {"./opaquerect_seqvideocapture-000001.png", 3099},
        {"./opaquerect_seqvideocapture-000001.flv", 30726},
        {"./opaquerect_seqvideocapture-000002.png", 3104},
        {"./opaquerect_seqvideocapture-000002.flv", 29119},
        {"./opaquerect_seqvideocapture-000003.png", 3101},
        {"./opaquerect_seqvideocapture-000003.flv", 29108},
        {"./opaquerect_seqvideocapture-000004.png", 3107},
        {"./opaquerect_seqvideocapture-000004.flv", 29088},
        {"./opaquerect_seqvideocapture-000005.png", 3101},
        {"./opaquerect_seqvideocapture-000005.flv", 30560},
        {"./opaquerect_seqvideocapture-000006.png", 3099},
        {"./opaquerect_seqvideocapture-000006.flv", 29076},
        {"./opaquerect_seqvideocapture-000007.png", 3101},
        {"./opaquerect_seqvideocapture-000007.flv", 30125},
        {"./opaquerect_seqvideocapture-000008.png", 3098},
        {"./opaquerect_seqvideocapture-000008.flv", 28966},
        {"./opaquerect_seqvideocapture-000009.png", 3098},
        {"./opaquerect_seqvideocapture-000009.flv", 29309},
        {"./opaquerect_seqvideocapture-000010.png", 3098},
        {"./opaquerect_seqvideocapture-000010.flv", 13539}
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        BOOST_CHECK_EQUAL(x.size, fsize);
        ::unlink(x.filename);
    }
}


BOOST_AUTO_TEST_CASE(SequencedVideoCaptureX264)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& now, reason reason) 
        {
            LOG(LOG_INFO, "next video: now=%u:%u reason=%u", 
                static_cast<unsigned>(now.tv_sec),
                static_cast<unsigned>(now.tv_usec),
                static_cast<unsigned>(reason));
        }
    } next_video_notifier;

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", 0};
        SequencedVideoCaptureImpl video_capture(now, 
            "./", "opaquerect_seqvideocapture_timestamp2",
            0 /* groupid */, false /* no_timestamp */, 100 /* zoom */, drawable, flv_params,
            std::chrono::microseconds{1000000}, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }
    struct CheckFiles {
        const char * filename;
        size_t size;
        // size actually depends on the codec version and at least two slightltly different ones exists for h264
        size_t alternativesize;
    } fileinfo[] = {
        {"./opaquerect_seqvideocapture_timestamp2-000000.png", 3099, 3099},
        {"./opaquerect_seqvideocapture_timestamp2-000000.mp4", 7323, 7309},
        {"./opaquerect_seqvideocapture_timestamp2-000001.png", 3099, 3099},
        {"./opaquerect_seqvideocapture_timestamp2-000001.mp4", 6889, 6875},
        {"./opaquerect_seqvideocapture_timestamp2-000002.png", 3104, 3104},
        {"./opaquerect_seqvideocapture_timestamp2-000002.mp4", 6629, 6615},
        {"./opaquerect_seqvideocapture_timestamp2-000003.png", 3101, 3101},
        {"./opaquerect_seqvideocapture_timestamp2-000003.mp4", 6385, 6371},
        {"./opaquerect_seqvideocapture_timestamp2-000004.png", 3107, 3107},
        {"./opaquerect_seqvideocapture_timestamp2-000004.mp4", 6013, 5999},
        {"./opaquerect_seqvideocapture_timestamp2-000005.png", 3101, 3101},
        {"./opaquerect_seqvideocapture_timestamp2-000005.mp4", 6036, 6022},
        {"./opaquerect_seqvideocapture_timestamp2-000006.png", 3099, 3099},
        {"./opaquerect_seqvideocapture_timestamp2-000006.mp4", 6133, 6119},
        {"./opaquerect_seqvideocapture_timestamp2-000007.png", 3101, 3101},
        {"./opaquerect_seqvideocapture_timestamp2-000007.mp4", 6410, 6396},
        {"./opaquerect_seqvideocapture_timestamp2-000008.png", 3098, 3098},
        {"./opaquerect_seqvideocapture_timestamp2-000008.mp4", 6631, 6617},
        {"./opaquerect_seqvideocapture_timestamp2-000009.png", 3098, 3098},
        {"./opaquerect_seqvideocapture_timestamp2-000009.mp4", 6876, 6862},
        {"./opaquerect_seqvideocapture_timestamp2-000010.png", 3098, 3098},
        {"./opaquerect_seqvideocapture_timestamp2-000010.mp4", 262, 262}
    };
    for (auto x: fileinfo) {
        size_t fsize = filesize(x.filename);
        if ((x.size != fsize) && (x.alternativesize != fsize)){
            BOOST_CHECK_EQUAL(x.size, filesize(x.filename));
        }
        ::unlink(x.filename);
    }
}

