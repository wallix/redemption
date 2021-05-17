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

#pragma once

#include "utils/ref.hpp"
#include "utils/rect.hpp"
#include "utils/monotonic_clock.hpp"
#include "configs/autogen/enums.hpp"
#include "capture/file_to_graphic_verbose.hpp"

#include <memory>

class Transport;

struct RegionsCapture
{
    static RegionsCapture compute_regions(
        Transport & trans,
        SmartVideoCropping smart_video_cropping,
        MonotonicTimePoint::duration interval_time_for_frame_maker_end,
        MonotonicTimePoint begin_capture,
        MonotonicTimePoint end_capture,
        bool play_video_with_corrupted_bitmap,
        ExplicitCRef<bool> requested_to_shutdown,
        FileToGraphicVerbose verbose);

    bool      is_remote_app = false;
    Rect      rail_window_rect_start;
    Rect      max_image_frame_rect;
    Rect      crop_rect;
    Dimension min_image_frame_dim;
    Dimension max_screen_dim;

    struct UpdatableFrameMarkerEnd
    {
        std::unique_ptr<unsigned long long[]> p;
        std::size_t len;
    };

    UpdatableFrameMarkerEnd updatable_frame_marker_end {};
};
