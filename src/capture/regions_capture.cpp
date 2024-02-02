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
#include "gdi/updatable_graphics.hpp"
#include "gdi/relayout_api.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "utils/bitset_stream.hpp"

namespace
{

constexpr std::size_t updatable_frame_end_buffer_size = 16*1024*1024 / sizeof(unsigned long long);

struct UpdatableFrameMarkerEndGraphics final : gdi::UpdatableGraphics, gdi::RelayoutApi
{
    UpdatableFrameMarkerEndGraphics(
        MonotonicTimePoint::duration interval,
        CRef<FileToGraphic> ftg)
    : ftg(ftg)
    , next_time(this->ftg.get_monotonic_time())
    , interval(interval)
    {}

    using gdi::UpdatableGraphics::draw;

    void draw(RDP::FrameMarker const & cmd) override
    {
        if (cmd.action == RDP::FrameMarker::FrameEnd) {
            if (is_updatable_frame()) {
                if (next_time <= ftg.get_monotonic_time()) {
                    updatable_frame_end_bitset_stream.write(previous_drawing_event);
                    previous_drawing_event = false;
                    auto elapsed = ftg.get_monotonic_time() - next_time;
                    next_time += elapsed / interval * interval + interval;
                }
                else {
                    updatable_frame_end_bitset_stream.write(false);
                }
            }
            first_frame = false;
            previous_drawing_event = previous_drawing_event || has_drawing_event();
            set_drawing_event(false);
        }
    }

    void last_frame()
    {
        if (is_updatable_frame()) {
            updatable_frame_end_bitset_stream.write(previous_drawing_event);
        }
    }

    bool is_updatable_frame() const
    {
        return !first_frame
            && updatable_frame_end_bitset_stream.current() != updatable_frame_last_it
            ;
    }

    void relayout(MonitorLayoutPDU const& monitor_layout_pdu) override
    {
        auto rect = monitor_layout_pdu.get_rect();
        screen_position_x = std::min(rect.x, screen_position_x);
        screen_position_y = std::min(rect.y, screen_position_y);
    }

    FileToGraphic const& ftg;
    MonotonicTimePoint next_time;
    MonotonicTimePoint::duration interval;
    BitsetOutStream updatable_frame_end_bitset_stream;
    unsigned long long* updatable_frame_last_it = nullptr;
    int16_t screen_position_x = 0;
    int16_t screen_position_y = 0;
    bool previous_drawing_event = true;
    bool first_frame = true;
};

} // anonymous namespace

RegionsCapture RegionsCapture::compute_regions(
    SequencedTransport & trans,
    SmartVideoCropping smart_video_cropping,
    MonotonicTimePoint::duration interval_time_for_frame_maker_end,
    MonotonicTimePoint begin_capture,
    MonotonicTimePoint end_capture,
    bool play_video_with_corrupted_bitmap,
    ExplicitCRef<bool> requested_to_shutdown,
    FileToGraphicVerbose verbose)
{
    RegionsCapture ret;

    FileToGraphic reader(trans, play_video_with_corrupted_bitmap, verbose);

    UpdatableFrameMarkerEndGraphics updatable_frame_end{
        interval_time_for_frame_maker_end, reader};

    ret.is_remote_app = reader.get_wrm_info().remote_app;

    auto set_max_dim = [](Dimension& dim, Dimension const& other){
        dim.w = std::max(dim.w, other.w);
        dim.h = std::max(dim.h, other.h);
    };

    auto consume_all_orders = [&]{
        if (interval_time_for_frame_maker_end.count()) {
            if (begin_capture > reader.get_monotonic_time()) {
                while (!requested_to_shutdown && reader.next_order()) {
                    reader.interpret_order();

                    if (begin_capture <= reader.get_monotonic_time()) {
                        break;
                    }
                }
            }

            ret.updatable_frame_marker_end.len = updatable_frame_end_buffer_size;
            ret.updatable_frame_marker_end.p
                = std::make_unique<unsigned long long[]>(updatable_frame_end_buffer_size);

            updatable_frame_end.updatable_frame_end_bitset_stream
                = BitsetOutStream(ret.updatable_frame_marker_end.p.get());
            updatable_frame_end.updatable_frame_last_it
                = ret.updatable_frame_marker_end.p.get() + updatable_frame_end_buffer_size;

            reader.add_consumer(
                &updatable_frame_end, nullptr, nullptr, nullptr,
                nullptr, nullptr, nullptr
            );
        }

        try {
            while (!requested_to_shutdown && reader.next_order()) {
                reader.interpret_order();
            }
        }
        catch (...) {
            // corrupted wrm
        }
    };

    if (ret.is_remote_app
     && smart_video_cropping != SmartVideoCropping::disable
    ) {
        RailScreenComputation rail_computation(
            reader.get_wrm_info().width,
            reader.get_wrm_info().height,
            true);

        ret.rail_window_rect_start = reader.rail_wrm_window_rect;
        rail_computation.visibility_rect_event(reader.rail_wrm_window_rect);

        reader.add_consumer(
            &rail_computation, nullptr, nullptr, nullptr, nullptr,
            &updatable_frame_end, &rail_computation);

        consume_all_orders();

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
            case SmartVideoCropping::disable:
                break;

            case SmartVideoCropping::v2:
                ret.crop_rect.cx = std::min(ret.min_image_frame_dim.w, info.width);
                ret.crop_rect.cy = std::min(ret.min_image_frame_dim.h, info.height);
                ret.crop_rect.x = (info.width  - ret.crop_rect.cx) / 2;
                ret.crop_rect.y = (info.height - ret.crop_rect.cy) / 2;
                break;

            case SmartVideoCropping::v1:
                ret.crop_rect = ret.max_image_frame_rect
                  // screen position should be negative or 0
                  .offset(-updatable_frame_end.screen_position_x,
                          -updatable_frame_end.screen_position_y)
                  .intersect(info.width, info.height);
                break;
        }

        LOG_IF(bool(verbose), LOG_INFO, "RegionsCapture::crop=%s", ret.crop_rect);
    }
    else if (interval_time_for_frame_maker_end.count()) {
        consume_all_orders();
        ret.max_screen_dim = reader.max_screen_dim;
    }
    else {
        // max_screen_dim is in META_FILE
        ret.max_screen_dim = reader.max_screen_dim;
        try {
            auto t = reader.get_monotonic_time();
            while (!requested_to_shutdown && t <= end_capture && trans.next()) {
                FileToGraphic reader(trans, play_video_with_corrupted_bitmap, verbose);
                set_max_dim(ret.max_screen_dim, reader.max_screen_dim);
                t = reader.get_monotonic_time();
            }
        }
        catch (...) {
            // ERR_TRANSPORT_NO_MORE_DATA
        }
    }

    ret.screen_position = {
        updatable_frame_end.screen_position_x,
        updatable_frame_end.screen_position_y,
    };

    updatable_frame_end.last_frame();

    // update ret.updatable_frame_marker_end.len
    if (auto* p = ret.updatable_frame_marker_end.p.get()) {
        auto* curr = updatable_frame_end.updatable_frame_end_bitset_stream.current();
        if (p != curr) {
            auto dist = curr - p;
            dist += updatable_frame_end.updatable_frame_end_bitset_stream.is_partial();
            ret.updatable_frame_marker_end.len = std::size_t(dist);
        }
        else {
            ret.updatable_frame_marker_end.p.reset();
            ret.updatable_frame_marker_end.len = 0;
        }
    }

    return ret;
}
