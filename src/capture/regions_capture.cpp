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

#include "capture/regions_capture.hpp"
#include "capture/file_to_graphic.hpp"
#include "capture/rail_screen_computation.hpp"


RegionsCapture RegionsCapture::compute_regions(
    Transport & trans,
    SmartVideoCropping smart_video_cropping,
    MonotonicTimePoint end_capture,
    bool play_video_with_corrupted_bitmap,
    ExplicitCRef<bool> requested_to_shutdown,
    FileToGraphic::Verbose verbose)
{
    RegionsCapture ret;

    FileToGraphic reader(
        trans, MonotonicTimePoint(), end_capture,
        play_video_with_corrupted_bitmap, verbose);

    ret.is_remote_app = reader.get_wrm_info().remote_app;

    auto set_max_dim = [](Dimension& dim, Dimension const& other){
        dim.w = std::max(dim.w, other.w);
        dim.h = std::max(dim.h, other.h);
    };

    if (ret.is_remote_app
     && smart_video_cropping != SmartVideoCropping::v1
    ) {
        RailScreenComputation rail_computation(
            reader.get_wrm_info().width,
            reader.get_wrm_info().height,
            bool(verbose));

        ret.rail_window_rect_start = reader.rail_wrm_window_rect;
        rail_computation.visibility_rect_event(reader.rail_wrm_window_rect);

        reader.add_consumer(
            &rail_computation, nullptr, nullptr, nullptr, nullptr, nullptr,
            &rail_computation);

        while (!requested_to_shutdown && reader.next_order()) {
            reader.interpret_order();
        }

        ret.max_image_frame_rect = rail_computation.get_max_image_frame_rect();
        ret.min_image_frame_dim = rail_computation.get_min_image_frame_dim();

        // IMAGE_FRAME_RECT (obsolete packet, replaced by RAIL_WINDOW_RECT)
        if (!reader.max_image_frame_rect.isempty()) {
            ret.max_image_frame_rect
              = ret.max_image_frame_rect.disjunct(reader.max_image_frame_rect);
            set_max_dim(ret.min_image_frame_dim, reader.min_image_frame_dim);
        }

        ret.max_screen_dim = reader.max_screen_dim;

        auto& info = reader.get_wrm_info();

        switch (smart_video_cropping) {
            case SmartVideoCropping::v1:
                break;

            case SmartVideoCropping::v2:
                ret.crop_rect.cx = std::min(ret.min_image_frame_dim.w, info.width);
                ret.crop_rect.cy = std::min(ret.min_image_frame_dim.h, info.height);
                if (!ret.crop_rect.isempty()) {
                    ret.crop_rect.cx += (ret.crop_rect.cx & 1);

                    ret.crop_rect.x = (info.width  - ret.crop_rect.cx) / 2;
                    ret.crop_rect.y = (info.height - ret.crop_rect.cy) / 2;
                }
                break;

            case SmartVideoCropping::disable:
                ret.crop_rect = ret.max_image_frame_rect.intersect(info.width, info.height);
                if (ret.crop_rect.cx & 1) {
                    if (ret.crop_rect.x + ret.crop_rect.cx < info.width) {
                        ret.crop_rect.cx += 1;
                    }
                    else if (ret.crop_rect.x > 0) {
                        ret.crop_rect.x  -= 1;
                        ret.crop_rect.cx += 1;
                    }
                }
                break;
        }
    }
    else {
        // max_screen_dim is in META_FILE
        ret.max_screen_dim = reader.max_screen_dim;
        try {
            while (!requested_to_shutdown) {
                trans.next();
                FileToGraphic reader(
                    trans, MonotonicTimePoint(), end_capture,
                    play_video_with_corrupted_bitmap, verbose);
                set_max_dim(ret.max_screen_dim, reader.max_screen_dim);
            }
        }
        catch (...) {
            // ERR_NO_MORE_DATA
        }
    }

    return ret;
}
