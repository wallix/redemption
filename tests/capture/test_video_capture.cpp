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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/video_capture.hpp"

#ifndef REDEMPTION_NO_FFMPEG

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "capture/full_video_params.hpp"
#include "capture/capture_params.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "utils/fileutils.hpp"


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

RED_AUTO_TEST_CASE(TestSequencedVideoCapture)
{
    {
        struct notified_on_video_change : public NotifyNextVideo
        {
            void notify_next_video(const timeval& /*now*/, reason /*reason*/) override
            {
                // TEST test
            }
        } next_video_notifier;

        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, false, std::chrono::microseconds{2 * 1000000l}, 0};
        CaptureParams capture_params{
            now, "opaquerect_videocapture", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 100 /* zoom */, drawable, drawable, video_params,
            next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000000.png", 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000000.flv", 77742);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000001.png", 3104);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000001.flv", 75511);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000002.png", 3107);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000002.flv", 77343);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000003.png", 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000003.flv", 76664);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000004.png", 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000004.flv", 75660);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000005.png", 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture-000005.flv", 18752);
}

RED_AUTO_TEST_CASE(TestSequencedVideoCaptureMP4)
{
    {
        struct notified_on_video_change : public NotifyNextVideo
        {
            void notify_next_video(const timeval& /*now*/, reason /*reason*/) override
            {
                // TEST test
            }
        } next_video_notifier;

        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", false, false, false, std::chrono::microseconds{2 * 1000000l}, 0};
        CaptureParams capture_params{
            now, "opaquerect_videocapture", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 100 /* zoom */, drawable, drawable, video_params, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000000.png", 3099, 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000000.mp4", 25467, 25453);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000001.png", 3104, 3104);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000001.mp4", 25016, 25002);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000002.png", 3107, 3107);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000002.mp4", 25192, 25178);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000003.png", 3099, 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000003.mp4", 24640, 24626);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000004.png", 3098, 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000004.mp4", 24409, 24395);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000005.png", 3098, 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_videocapture-000005.mp4", 6618, 6604);
}

RED_AUTO_TEST_CASE(TestVideoCaptureOneChunkFLV)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& /*now*/, reason /*reason*/) override
        {
            // TEST
        }
    } next_video_notifier;

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, false, std::chrono::microseconds{1000 * 1000000l}, 0};
        CaptureParams capture_params{
            now, "opaquerect_videocapture_one_chunk_xxx", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 100 /* zoom */, drawable, drawable, video_params, next_video_notifier);
        simple_movie(now, 1000, drawable, video_capture, false, true);
    }

    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000000.png", 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000000.flv", 1187755);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000001.png", -1);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_videocapture_one_chunk_xxx-000001.flv", -1);
}

RED_AUTO_TEST_CASE(TestFullVideoCaptureFlv)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, false, {}, 0};
        CaptureParams capture_params{
            now, "opaquerect_fullvideocapture_timestamp1", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        FullVideoCaptureImpl video_capture(
            capture_params, drawable, drawable, video_params, FullVideoParams{false});
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_fullvideocapture_timestamp1.flv", 308260);
}

RED_AUTO_TEST_CASE(TestFullVideoCaptureFlv2)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, false, {}, 0};
        CaptureParams capture_params{
            now, "opaquerect_fullvideocapture_timestamp_mouse0", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        FullVideoCaptureImpl video_capture(
            capture_params, drawable, drawable, video_params, FullVideoParams{false});
        simple_movie(now, 250, drawable, video_capture, false, false);
    }
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_fullvideocapture_timestamp_mouse0.flv", 298223);
}

RED_AUTO_TEST_CASE(TestFullVideoCaptureX264)
{
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", false, false, false, {}, 0};
        CaptureParams capture_params{
            now, "opaquerect_fullvideocapture_timestamp2", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        FullVideoCaptureImpl video_capture(
            capture_params, drawable, drawable, video_params, FullVideoParams{false});
        simple_movie(now, 250, drawable, video_capture, false, true);
    }
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_fullvideocapture_timestamp2.mp4", 123987, 123973);
}

RED_AUTO_TEST_CASE(SequencedVideoCaptureFLV)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& /*now*/, reason /*reason*/) override
        {
            // TEST check notification
        }
    } next_video_notifier;

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "flv", false, false, false, std::chrono::microseconds{1000000}, 0};
        CaptureParams capture_params{
            now, "opaquerect_seqvideocapture", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 100 /* zoom */, drawable, drawable, video_params, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000000.png", 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000000.flv", 47289);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000001.png", 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000001.flv", 48843);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000002.png", 3104);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000002.flv", 47007);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000003.png", 3101);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000003.flv", 47045);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000004.png", 3107);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000004.flv", 47028);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000005.png", 3101);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000005.flv", 48819);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000006.png", 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000006.flv", 46963);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000007.png", 3101);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000007.flv", 48130);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000008.png", 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000008.flv", 46803);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000009.png", 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000009.flv", 47244);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000010.png", 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN("./opaquerect_seqvideocapture-000010.flv", 18752);
}


RED_AUTO_TEST_CASE(SequencedVideoCaptureX264)
{
    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& /*now*/, reason /*reason*/) override
        {
            // TEST check notification
        }
    } next_video_notifier;

    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{Level::high, drawable.width(), drawable.height(), 25, 15, 100000, "mp4", false, false, false, std::chrono::microseconds{1000000}, 0};
        CaptureParams capture_params{
            now, "opaquerect_seqvideocapture_timestamp2", nullptr, "./", 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 100 /* zoom */, drawable, drawable, video_params, next_video_notifier);
        simple_movie(now, 250, drawable, video_capture, false, true);
    }

    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000000.png", 3099, 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000000.mp4", 13477, 13463);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000001.png", 3099, 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000001.mp4", 13470, 13456);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000002.png", 3104, 3104);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000002.mp4", 13299, 13285);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000003.png", 3101, 3101);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000003.mp4", 13196, 13182);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000004.png", 3107, 3107);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000004.mp4", 13212, 13198);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000005.png", 3101, 3101);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000005.mp4", 13457, 13443);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000006.png", 3099, 3099);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000006.mp4", 13054, 13040);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000007.png", 3101, 3101);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000007.mp4", 13064, 13050);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000008.png", 3098, 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000008.mp4", 12903, 12889);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000009.png", 3098, 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000009.mp4", 12983, 12969);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000010.png", 3098, 3098);
    RED_CHECK_FILE_SIZE_AND_CLEAN2("./opaquerect_seqvideocapture_timestamp2-000010.mp4", 6618, 6604);
}
#else
RED_AUTO_TEST_CASE(NoTest)
{}
#endif
