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

#ifndef REDEMPTION_NO_FFMPEG
#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "capture/video_capture.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "capture/full_video_params.hpp"
#include "capture/capture_params.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "utils/fileutils.hpp"

using std::chrono_literals::operator""s;

namespace
{
    void simple_movie(
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

    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(const timeval& /*now*/, reason /*reason*/) override
        {
        }
    } next_video_notifier;

    struct Codec
    {
        char const* name;
        char const* options;
    };
    constexpr Codec flv{"flv", "flags=+qscale b=100000"};
    constexpr Codec mp4{"mp4", "profile=baseline preset=ultrafast b=100000"};

    void simple_sequenced_video(
        char const* dirname, Codec const& codec, std::chrono::seconds video_interval,
        unsigned loop_duration, bool ignore_frame_in_timeval, bool mouse)
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{
            25, codec.name, codec.options, false, false, false, video_interval, 0};
        CaptureParams capture_params{
            now, "video", nullptr, dirname, 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 100 /* zoom */, drawable, drawable, video_params,
            next_video_notifier);
        simple_movie(now, loop_duration, drawable, video_capture, ignore_frame_in_timeval, mouse);
    }

    void simple_full_video(
        char const* dirname, Codec const& codec, std::chrono::seconds video_interval,
        unsigned loop_duration, bool ignore_frame_in_timeval, bool mouse)
    {
        timeval now; now.tv_sec = 1353055800; now.tv_usec = 0;
        RDPDrawable drawable(800, 600);
        VideoParams video_params{
            25, codec.name, codec.options, false, false, false, video_interval, 0};
        CaptureParams capture_params{
            now, "video", nullptr, dirname, 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
        FullVideoCaptureImpl video_capture(
            capture_params, drawable, drawable, video_params, FullVideoParams{false});
        simple_movie(now, loop_duration, drawable, video_capture, ignore_frame_in_timeval, mouse);
    }
} // namespace


RED_AUTO_TEST_CASE_WD(TestSequencedVideoCapture, wd)
{
    simple_sequenced_video(wd.dirname(), flv, 2s, 250, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video-000000.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.flv"), 77155);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.png"), 3104);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.flv"), 75432);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.png"), 3107);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.flv"), 77291);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.flv"), 76735);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.flv"), 75604);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.flv"), 18741);
}

RED_AUTO_TEST_CASE_WD(TestSequencedVideoCaptureMP4, wd)
{
    simple_sequenced_video(wd.dirname(), mp4, 2s, 250, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video-000000.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.mp4"), 23021 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.png"), 3104);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.mp4"), 22338 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.png"), 3107);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.mp4"), 23267 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.mp4"), 24767 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.mp4"), 23044 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.mp4"), 5315 +- 2000_v);
}

RED_AUTO_TEST_CASE_WD(TestVideoCaptureOneChunkFLV, wd)
{
    simple_sequenced_video(wd.dirname(), flv, 1000s, 1000, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video-000000.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.flv"), 1185483);
}

RED_AUTO_TEST_CASE_WD(SequencedVideoCaptureFLV, wd)
{
    simple_sequenced_video(wd.dirname(), flv, 1s, 250, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video-000000.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.flv"), 47215);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.flv"), 48300);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.png"), 3104);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.flv"), 46964);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.png"), 3101);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.flv"), 46982);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.png"), 3107);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.flv"), 47031);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.png"), 3101);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.flv"), 48723);
    RED_TEST_FILE_SIZE(wd.add_file("video-000006.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000006.flv"), 46847);
    RED_TEST_FILE_SIZE(wd.add_file("video-000007.png"), 3101);
    RED_TEST_FILE_SIZE(wd.add_file("video-000007.flv"), 48287);
    RED_TEST_FILE_SIZE(wd.add_file("video-000008.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000008.flv"), 46747);
    RED_TEST_FILE_SIZE(wd.add_file("video-000009.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000009.flv"), 47214);
    RED_TEST_FILE_SIZE(wd.add_file("video-000010.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000010.flv"), 18741);
}

RED_AUTO_TEST_CASE_WD(SequencedVideoCaptureX264, wd)
{
    simple_sequenced_video(wd.dirname(), mp4, 1s, 250, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video-000000.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.mp4"), 13584 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.mp4"), 15175 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.png"), 3104);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.mp4"), 15299 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.png"), 3101);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.mp4"), 13576 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.png"), 3107);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.mp4"), 13587 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.png"), 3101);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.mp4"), 14264 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000006.png"), 3099);
    RED_TEST_FILE_SIZE(wd.add_file("video-000006.mp4"), 13949 +- 3100_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000007.png"), 3101);
    RED_TEST_FILE_SIZE(wd.add_file("video-000007.mp4"), 13385 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000008.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000008.mp4"), 13622 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000009.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000009.mp4"), 13693 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000010.png"), 3098);
    RED_TEST_FILE_SIZE(wd.add_file("video-000010.mp4"), 5315 +- 50_v);
}

RED_AUTO_TEST_CASE_WD(TestFullVideoCaptureFlv, wd)
{
    simple_full_video(wd.dirname(), flv, 0s, 250, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video.flv"), 307698 +- 15000_v);
}

RED_AUTO_TEST_CASE_WD(TestFullVideoCaptureFlv2, wd)
{
    simple_full_video(wd.dirname(), flv, 0s, 250, false, false);

    RED_TEST_FILE_SIZE(wd.add_file("video.flv"), 298467 +- 5000_v);
}

RED_AUTO_TEST_CASE_WD(TestFullVideoCaptureX264, wd)
{
    simple_full_video(wd.dirname(), mp4, 0s, 250, false, true);

    RED_TEST_FILE_SIZE(wd.add_file("video.mp4"), 106930 +- 6000_v);
}
#else
int main() {}
#endif
