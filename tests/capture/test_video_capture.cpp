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

#define RED_TEST_MODULE TestWrmCapture
#include "system/redemption_unit_tests.hpp"


#define LOGNULL

#include "utils/log.hpp"

#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/stream.hpp"

#include "transport/transport.hpp"
#include "test_only/transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/in_file_transport.hpp"

#include "test_only/check_sig.hpp"
#include "test_only/get_file_contents.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"

#include "capture/video_capture.hpp"
#include "core/RDP/RDPDrawable.hpp"

inline void simple_movie(
    timeval now, unsigned duration, RDPDrawable & drawable,
    gdi::CaptureApi & capture, bool ignore_frame_in_timeval, bool mouse
) {
    Rect screen(0, 0, drawable.width(), drawable.height());
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(screen, encode_color24()(BLUE)), screen, color_cxt);

    uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
    Rect r(10, 10, 50, 50);
    int vx = 5;
    int vy = 4;
    for (size_t x = 0; x < duration; x++) {
        drawable.draw(RDPOpaqueRect(r, encode_color24()(BLUE)), screen, color_cxt);
        r.y += vy;
        r.x += vx;
        drawable.draw(RDPOpaqueRect(r, encode_color24()(WABGREEN)), screen, color_cxt);
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
    // last frame (video.encoding_video_frame())
    usec += 40000LL;
    now.tv_sec  = usec / 1000000LL;
    now.tv_usec = (usec % 1000000LL);
    int cursor_x = mouse?r.x + 10:0;
    int cursor_y = mouse?r.y + 10:0;
    capture.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
}

#define CHECK_FILESIZE_AND_CLEAN(filename, size) { \
    size_t fsize = filesize(filename);             \
    RED_CHECK_EQUAL(size, fsize);                \
    ::unlink(filename);                            \
}

#define CHECK_FILESIZE_AND_CLEAN2(filename, size1, size2) { \
    size_t fsize = filesize(filename);                      \
    if (fsize != size2){                                    \
        RED_CHECK_EQUAL(size1, fsize);                    \
    }                                                       \
    ::unlink(filename);                                     \
}

RED_AUTO_TEST_CASE(TestSequencedVideoCapture)
{
    {
        struct notified_on_video_change : public NotifyNextVideo
        {
            void notify_next_video(const timeval& now, reason reason) override
            {
                LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
                    static_cast<unsigned>(now.tv_sec),
                    static_cast<unsigned>(now.tv_usec),
                    static_cast<unsigned>(reason));
            }
        } next_video_notifier;

        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, std::chrono::microseconds{2 * 1000000l}, 0};
        SequencedVideoCaptureImpl video_capture(now,
            "./", "opaquerect_videocapture",
            0 /* groupid */, 100 /* zoom */, drawable, &drawable, flv_params,
            next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000000.png", 3099);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000000.flv", 59101);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000001.png", 3104);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000001.flv", 57209);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000002.png", 3107);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000002.flv", 58613);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000003.png", 3099);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000003.flv", 58182);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000004.png", 3098);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000004.flv", 57363);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000005.png", 3098);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture-000005.flv", 18489);
}

RED_AUTO_TEST_CASE(TestSequencedVideoCaptureMP4)
{
    {
        struct notified_on_video_change : public NotifyNextVideo
        {
            void notify_next_video(const timeval& now, reason reason) override
            {
                LOG(LOG_INFO, "next video: now=%u:%u reason=%u",
                    static_cast<unsigned>(now.tv_sec),
                    static_cast<unsigned>(now.tv_usec),
                    static_cast<unsigned>(reason));
            }
        } next_video_notifier;

        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", false, false, std::chrono::microseconds{2 * 1000000l}, 0};
        SequencedVideoCaptureImpl video_capture(now,
            "./", "opaquerect_videocapture",
            0 /* groupid */, 100 /* zoom */, drawable, &drawable, flv_params, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000000.png", 3099, 3099);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000000.mp4", 25039, 25025);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000001.png", 3104, 3104);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000001.mp4", 24670, 24656);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000002.png", 3107, 3107);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000002.mp4", 24759, 24745);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000003.png", 3099, 3099);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000003.mp4", 24283, 24268);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000004.png", 3098, 3098);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000004.mp4", 23971, 23957);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000005.png", 3098, 3098);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_videocapture-000005.mp4", 262, 262);
}

RED_AUTO_TEST_CASE(TestVideoCaptureOneChunkFLV)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& now, reason reason) override
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
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, std::chrono::microseconds{1000 * 1000000l}, 0};
        SequencedVideoCaptureImpl video_capture(now,
            "./", "opaquerect_videocapture_one_chunk_xxx",
            0 /* groupid */, 100 /* zoom */, drawable, &drawable, flv_params, next_video_notifier);
        simple_movie(now, 1000, drawable, video_capture, false, true);
    }

    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000000.png", 3099);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000000.flv", 1187492);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000001.png", -1);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000001.flv", -1);
}

RED_AUTO_TEST_CASE(TestFullVideoCaptureFlv)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, {}, 0};
        FullVideoCaptureImpl video_capture(now,
            "./", "opaquerect_fullvideocapture_timestamp1",
            0 /* groupid */, drawable, &drawable, flv_params);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    CHECK_FILESIZE_AND_CLEAN("./opaquerect_fullvideocapture_timestamp1.flv", 307997);
}

RED_AUTO_TEST_CASE(TestFullVideoCaptureFlv2)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, {}, 0};
        FullVideoCaptureImpl video_capture(now,
            "./", "opaquerect_fullvideocapture_timestamp_mouse0",
            0 /* groupid */, drawable, &drawable, flv_params);
        simple_movie(now, 250, drawable, video_capture, false, false);
    }
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_fullvideocapture_timestamp_mouse0.flv", 297960);
}

RED_AUTO_TEST_CASE(TestFullVideoCaptureX264)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", false, false, {}, 0};
        FullVideoCaptureImpl video_capture(now,
            "./", "opaquerect_fullvideocapture_timestamp2",
            0 /* groupid */, drawable, &drawable, flv_params);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_fullvideocapture_timestamp2.mp4", 118756, 118742);
}

RED_AUTO_TEST_CASE(SequencedVideoCaptureFLV)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& now, reason reason) override
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
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, std::chrono::microseconds{1000000}, 0};
        SequencedVideoCaptureImpl video_capture(now,
            "./", "opaquerect_seqvideocapture",
            0 /* groupid */, 100 /* zoom */, drawable, &drawable, flv_params, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000000.png", 3099);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000000.flv", 29091);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000001.png", 3099);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000001.flv", 30202);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000002.png", 3104);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000002.flv", 28658);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000003.png", 3101);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000003.flv", 28743);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000004.png", 3107);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000004.flv", 28716);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000005.png", 3101);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000005.flv", 30089);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000006.png", 3099);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000006.flv", 28726);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000007.png", 3101);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000007.flv", 29648);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000008.png", 3098);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000008.flv", 28608);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000009.png", 3098);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000009.flv", 28947);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000010.png", 3098);
    CHECK_FILESIZE_AND_CLEAN("./opaquerect_seqvideocapture-000010.flv", 18489);
}


RED_AUTO_TEST_CASE(SequencedVideoCaptureX264)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& now, reason reason) override
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
        FlvParams flv_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", false, false, std::chrono::microseconds{1000000}, 0};
        SequencedVideoCaptureImpl video_capture(now,
            "./", "opaquerect_seqvideocapture_timestamp2",
            0 /* groupid */, 100 /* zoom */, drawable, &drawable, flv_params, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000000.png", 3099, 3099);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000000.mp4", 13265, 13251);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000001.png", 3099, 3099);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000001.mp4", 13042, 13028);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000002.png", 3104, 3104);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000002.mp4", 12964, 12950);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000003.png", 3101, 3101);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000003.mp4", 12850, 12836);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000004.png", 3107, 3107);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000004.mp4", 12991, 12977);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000005.png", 3101, 3101);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000005.mp4", 13024, 13010);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000006.png", 3099, 3099);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000006.mp4", 12843, 12829);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000007.png", 3101, 3101);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000007.mp4", 12707, 12692);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000008.png", 3098, 3098);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000008.mp4", 12667, 12653);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000009.png", 3098, 3098);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000009.mp4", 12545, 12531);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000010.png", 3098, 3098);
    CHECK_FILESIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000010.mp4", 262, 262);
}
