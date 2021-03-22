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
#include "test_only/test_framework/check_img.hpp"
#include "test_only/test_framework/file.hpp"

#include "capture/video_capture.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "capture/full_video_params.hpp"
#include "capture/capture_params.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "utils/fileutils.hpp"

#include <chrono>

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/capture/video_capture/"

using namespace std::chrono_literals;

namespace
{
    void simple_movie(
        MonotonicTimePoint now, unsigned duration, RDPDrawable & drawable,
        gdi::CaptureApi & capture, gdi::GraphicApi & video_drawable, bool mouse
    ) {
        Rect screen(0, 0, drawable.width(), drawable.height());
        auto const color_cxt = gdi::ColorCtx::depth24();
        drawable.draw(RDPOpaqueRect(screen, encode_color24()(BLUE)), screen, color_cxt);
        video_drawable.draw(RDPOpaqueRect(screen, encode_color24()(BLUE)), screen, color_cxt);

        Rect r(10, 10, 50, 50);
        int vx = 5;
        int vy = 4;
        for (size_t x = 0; x < duration; x++) {
            drawable.draw(RDPOpaqueRect(r, encode_color24()(BLUE)), screen, color_cxt);
            r.y += vy;
            r.x += vx;
            drawable.draw(RDPOpaqueRect(r, encode_color24()(WABGREEN)), screen, color_cxt);
            video_drawable.draw(RDPOpaqueRect(r, encode_color24()(WABGREEN)), screen, color_cxt);
            now += 40000us;
            //printf("now sec=%u usec=%u\n", (unsigned)now.tv_sec, (unsigned)now.tv_usec);
            uint16_t cursor_x = mouse ? uint16_t(r.x + 10) : 0;
            uint16_t cursor_y = mouse ? uint16_t(r.y + 10) : 0;
            drawable.set_mouse_cursor_pos(cursor_x, cursor_y);
            capture.periodic_snapshot(now, cursor_x, cursor_y);
            capture.periodic_snapshot(now, cursor_x, cursor_y);
            if ((r.x + r.cx >= drawable.width())  || (r.x < 0)) { vx = -vx; }
            if ((r.y + r.cy >= drawable.height()) || (r.y < 0)) { vy = -vy; }
        }
        // last frame (video.encoding_video_frame())
        now += 40000us;
        uint16_t cursor_x = mouse ? uint16_t(r.x + 10) : 0;
        uint16_t cursor_y = mouse ? uint16_t(r.y + 10) : 0;
        capture.periodic_snapshot(now, cursor_x, cursor_y);
    }

    struct notified_on_video_change : public NotifyNextVideo
    {
        void notify_next_video(MonotonicTimePoint /*now*/, Reason /*reason*/) override
        {
        }
    } next_video_notifier;

    struct Codec
    {
        char const* name;
        char const* options;
    };
    constexpr Codec mp4{"mp4", "profile=baseline preset=ultrafast b=100000"};

    void simple_sequenced_video(
        char const* dirname, Codec const& codec, std::chrono::seconds video_interval,
        unsigned loop_duration, bool mouse)
    {
        MonotonicTimePoint monotonic_time{12s + 653432us};
        RealTimePoint real_time{1353055788s + monotonic_time.time_since_epoch()};
        RDPDrawable drawable(800, 600);
        VideoParams video_params{
            25, codec.name, codec.options, false, false, false, video_interval, 0};
        CaptureParams capture_params{
            monotonic_time, real_time, "video", nullptr, dirname, 0 /* groupid */,
            nullptr, SmartVideoCropping::disable, 0};
        SequencedVideoCaptureImpl video_capture(
            capture_params, 0 /* png_width */, 0 /* png_height */,
            drawable, drawable, video_params, next_video_notifier);
        simple_movie(
            monotonic_time, loop_duration, drawable, video_capture, video_capture, mouse);
    }

    void simple_full_video(
        char const* dirname, Codec const& codec, std::chrono::seconds video_interval,
        unsigned loop_duration, bool mouse)
    {
        MonotonicTimePoint monotonic_time{12s + 653432us};
        RealTimePoint real_time{1353055788s + monotonic_time.time_since_epoch()};
        RDPDrawable drawable(800, 600);
        VideoParams video_params{
            25, codec.name, codec.options, false, false, false, video_interval, 0};
        CaptureParams capture_params{
            monotonic_time, real_time, "video", nullptr, dirname, 0 /* groupid */,
            nullptr, SmartVideoCropping::disable, 0};
        FullVideoCaptureImpl video_capture(
            capture_params, drawable, drawable, video_params, FullVideoParams{false});
        simple_movie(monotonic_time, loop_duration, drawable, video_capture, video_capture, mouse);
    }
} // namespace

RED_AUTO_TEST_CASE_WD(TestSequencedVideoCaptureMP4, wd)
{
    simple_sequenced_video(wd.dirname(), mp4, 2s, 250, true);

    RED_CHECK_IMG(wd.add_file("video-000000.png"), IMG_TEST_PATH "2bis.png");
    RED_CHECK_IMG(wd.add_file("video-000001.png"), IMG_TEST_PATH "2s.png");
    RED_CHECK_IMG(wd.add_file("video-000002.png"), IMG_TEST_PATH "4s.png");
    RED_CHECK_IMG(wd.add_file("video-000003.png"), IMG_TEST_PATH "6s.png");
    RED_CHECK_IMG(wd.add_file("video-000004.png"), IMG_TEST_PATH "8s.png");
    RED_CHECK_IMG(wd.add_file("video-000005.png"), IMG_TEST_PATH "10s.png");
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.mp4"), 23021 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.mp4"), 22338 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.mp4"), 23267 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.mp4"), 24767 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.mp4"), 23044 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.mp4"), 5315 +- 2000_v);
}

RED_AUTO_TEST_CASE_WD(SequencedVideoCaptureX264, wd)
{
    simple_sequenced_video(wd.dirname(), mp4, 1s, 250, true);

    RED_CHECK_IMG(wd.add_file("video-000000.png"), IMG_TEST_PATH "1s.png");
    RED_CHECK_IMG(wd.add_file("video-000001.png"), IMG_TEST_PATH "1s.png");
    RED_CHECK_IMG(wd.add_file("video-000002.png"), IMG_TEST_PATH "2s.png");
    RED_CHECK_IMG(wd.add_file("video-000003.png"), IMG_TEST_PATH "3s.png");
    RED_CHECK_IMG(wd.add_file("video-000004.png"), IMG_TEST_PATH "4s.png");
    RED_CHECK_IMG(wd.add_file("video-000005.png"), IMG_TEST_PATH "5s.png");
    RED_CHECK_IMG(wd.add_file("video-000006.png"), IMG_TEST_PATH "6s.png");
    RED_CHECK_IMG(wd.add_file("video-000007.png"), IMG_TEST_PATH "7s.png");
    RED_CHECK_IMG(wd.add_file("video-000008.png"), IMG_TEST_PATH "8s.png");
    RED_CHECK_IMG(wd.add_file("video-000009.png"), IMG_TEST_PATH "9s.png");
    RED_CHECK_IMG(wd.add_file("video-000010.png"), IMG_TEST_PATH "10s.png");
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.mp4"), 13584 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.mp4"), 15175 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.mp4"), 15299 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000003.mp4"), 13576 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000004.mp4"), 13587 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000005.mp4"), 14264 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000006.mp4"), 13949 +- 3100_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000007.mp4"), 13385 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000008.mp4"), 13622 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000009.mp4"), 13693 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000010.mp4"), 6080 +- 50_v);
}

RED_AUTO_TEST_CASE_WD(TestSequencedVideoCaptureMP4_3, wd)
{
    simple_sequenced_video(wd.dirname(), mp4, 5s, 250, true);

    RED_CHECK_IMG(wd.add_file("video-000000.png"), IMG_TEST_PATH "2bis.png");
    RED_CHECK_IMG(wd.add_file("video-000001.png"), IMG_TEST_PATH "5s.png");
    RED_CHECK_IMG(wd.add_file("video-000002.png"), IMG_TEST_PATH "10s.png");
    RED_TEST_FILE_SIZE(wd.add_file("video-000000.mp4"), 53021 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000001.mp4"), 54338 +- 2000_v);
    RED_TEST_FILE_SIZE(wd.add_file("video-000002.mp4"), 6080 +- 50_v);
}

RED_AUTO_TEST_CASE_WD(TestFullVideoCaptureX264, wd)
{
    simple_full_video(wd.dirname(), mp4, 0s, 250, true);

    RED_TEST_FILE_SIZE(wd.add_file("video.mp4"), 106930 +- 10000_v);
}

RED_AUTO_TEST_CASE_WD(TestFullVideoCaptureX264_2, wd)
{
    simple_full_video(wd.dirname(), mp4, 0s, 250, false);

    RED_TEST_FILE_SIZE(wd.add_file("video.mp4"), 92693 +- 10000_v);
}
#else
int main() {}
#endif
