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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/regions_capture.hpp"
#include "transport/file_transport.hpp"


#define RECORDER_TEST_PATH FIXTURES_PATH "/multimon_remoteapp/recorder"

namespace
{
    struct InTestFile : InFileTransport
    {
        using InFileTransport::InFileTransport;

        bool next() override
        {
            return false;
        }
    };

    RegionsCapture compute_regions(char const* filename, SmartVideoCropping svc)
    {
        InTestFile in_file(unique_fd{filename});

        bool program_requested_to_shutdown = false;

        return RegionsCapture::compute_regions(
            in_file,
            svc,
            MonotonicTimePoint::duration(),
            MonotonicTimePoint(),
            MonotonicTimePoint(),
            false,
            ExplicitCRef(program_requested_to_shutdown),
            FileToGraphicVerbose());
    }
} // anonymous namespace

RED_AUTO_TEST_CASE(TestRegionsCaptureDisabled)
{
    auto r = compute_regions(RECORDER_TEST_PATH "/app_4720x1440-000000.wrm", SmartVideoCropping::disable);

    RED_CHECK(r.is_remote_app);
    RED_CHECK(r.rail_window_rect_start == Rect());
    RED_CHECK(r.max_image_frame_rect == Rect());
    RED_CHECK(r.crop_rect == Rect());
    RED_CHECK(r.min_image_frame_dim == Dimension());
    RED_CHECK(r.max_screen_dim == Dimension(4720, 1440));
    RED_CHECK(r.screen_position == (Point{0, 0}));
    RED_CHECK(r.updatable_frame_marker_end.len == 0);
}

RED_AUTO_TEST_CASE(TestRegionsCaptureV1)
{
    auto r = compute_regions(RECORDER_TEST_PATH "/app_4720x1440-000000.wrm", SmartVideoCropping::v1);

    RED_CHECK(r.is_remote_app);
    RED_CHECK(r.rail_window_rect_start == Rect());
    RED_CHECK(r.max_image_frame_rect == Rect(135, 72, 1603, 763));
    RED_CHECK(r.crop_rect == Rect(135, 72, 1603, 763));
    RED_CHECK(r.min_image_frame_dim == Dimension(769, 559));
    RED_CHECK(r.max_screen_dim == Dimension(4720, 1440));
    RED_CHECK(r.screen_position == (Point{0, 0}));
    RED_CHECK(r.updatable_frame_marker_end.len == 0);
}

RED_AUTO_TEST_CASE(TestRegionsCaptureV2)
{
    auto r = compute_regions(RECORDER_TEST_PATH "/app_4720x1440-000000.wrm", SmartVideoCropping::v2);

    RED_CHECK(r.is_remote_app);
    RED_CHECK(r.rail_window_rect_start == Rect());
    RED_CHECK(r.max_image_frame_rect == Rect(135, 72, 1603, 763));
    RED_CHECK(r.crop_rect == Rect(1975, 440, 769, 559));
    RED_CHECK(r.min_image_frame_dim == Dimension(769, 559));
    RED_CHECK(r.max_screen_dim == Dimension(4720, 1440));
    RED_CHECK(r.screen_position == (Point{0, 0}));
    RED_CHECK(r.updatable_frame_marker_end.len == 0);
}

RED_AUTO_TEST_CASE(TestRegionsCaptureDisabledSecondaryScreenToLeft)
{
    auto r = compute_regions(RECORDER_TEST_PATH "/secondary_screen_to_left-000000.wrm", SmartVideoCropping::disable);

    RED_CHECK(r.is_remote_app);
    RED_CHECK(r.rail_window_rect_start == Rect());
    RED_CHECK(r.max_image_frame_rect == Rect());
    RED_CHECK(r.crop_rect == Rect());
    RED_CHECK(r.min_image_frame_dim == Dimension());
    RED_CHECK(r.max_screen_dim == Dimension(4720, 1440));
    RED_CHECK(r.screen_position == (Point{0, 0}));
    RED_CHECK(r.updatable_frame_marker_end.len == 0);
}

RED_AUTO_TEST_CASE(TestRegionsCaptureV1SecondaryScreenToLeft)
{
    auto r = compute_regions(RECORDER_TEST_PATH "/secondary_screen_to_left-000000.wrm", SmartVideoCropping::v1);

    RED_CHECK(r.is_remote_app);
    RED_CHECK(r.rail_window_rect_start == Rect());
    RED_CHECK(r.max_image_frame_rect == Rect(-826, 35, 2865, 924));
    RED_CHECK(r.crop_rect == Rect(454, 35, 2865, 924));
    RED_CHECK(r.min_image_frame_dim == Dimension(1048, 559));
    RED_CHECK(r.max_screen_dim == Dimension(4720, 1440));
    RED_CHECK(r.screen_position == (Point{-1280, 0}));
    RED_CHECK(r.updatable_frame_marker_end.len == 0);
}

RED_AUTO_TEST_CASE(TestRegionsCaptureV2SecondaryScreenToLeft)
{
    auto r = compute_regions(RECORDER_TEST_PATH "/secondary_screen_to_left-000000.wrm", SmartVideoCropping::v2);

    RED_CHECK(r.is_remote_app);
    RED_CHECK(r.rail_window_rect_start == Rect());
    RED_CHECK(r.max_image_frame_rect == Rect(-826, 35, 2865, 924));
    RED_CHECK(r.crop_rect == Rect(1836, 440, 1048, 559));
    RED_CHECK(r.min_image_frame_dim == Dimension(1048, 559));
    RED_CHECK(r.max_screen_dim == Dimension(4720, 1440));
    RED_CHECK(r.screen_position == (Point{-1280, 0}));
    RED_CHECK(r.updatable_frame_marker_end.len == 0);
}
