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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once

#include "acl/auth_api.hpp"
#include "capture/video_params.hpp"
#include "capture/video_recorder.hpp"
#include "capture/notify_next_video.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/graphic_api_forwarder.hpp"
#include "transport/file_transport.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/timestamp_tracer.hpp"
#include "utils/monotonic_time_to_real_time.hpp"
#include "utils/scaled_image24.hpp"

#include <chrono>
#include <optional>

class CaptureParams;
class FullVideoParams;
class RDPDrawable;


struct VideoCaptureCtx : noncopyable
{
    struct UpdatableDraw
    {
        template<class... Ts>
        void draw(Ts const&...);

        void set_pointer(
            uint16_t /*cache_idx*/, Pointer const& /*cursor*/,
            gdi::GraphicApi::SetPointerMode /*mode*/)
        {}
        void set_palette(BGRPalette const & /*palette*/) {}
        void sync() {}

        void set_row(std::size_t /*rownum*/, bytes_view /*data*/) {}
        void begin_update() {}
        void end_update() {}

        bool has_draw_event = true;
    };

    enum class ImageByInterval : unsigned char
    {
        OneWithTimestamp,
        OneWithoutTimestamp,
        ZeroOrOneWithTimestamp,
        ZeroOrOneWithoutTimestamp,
    };

    VideoCaptureCtx(
        MonotonicTimePoint monotonic_now,
        RealTimePoint real_time,
        ImageByInterval image_by_interval,
        unsigned frame_rate,
        RDPDrawable & drawable,
        gdi::ImageFrameApi & image_frame
    );

    void frame_marker_event(video_recorder & recorder);
    void encoding_end_frame(video_recorder & recorder, bool & has_draw_event);
    gdi::CaptureApi::WaitingTimeBeforeNextSnapshot snapshot(
        video_recorder& recorder, MonotonicTimePoint now, bool & has_draw_event,
        uint16_t cursor_x, uint16_t cursor_y);
    void next_video(video_recorder & recorder);

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

private:
    void preparing_video_frame(video_recorder & recorder);

    RDPDrawable & drawable;
    MonotonicTimePoint monotonic_last_time_capture;
    MonotonicTimeToRealTime monotonic_to_real;
    MonotonicTimePoint::duration frame_interval;

    MonotonicTimePoint next_trace_time;
    int64_t frame_index = 0;
    const ImageByInterval image_by_interval;
    bool has_frame_marker = false;
    uint16_t cursor_x = 0;
    uint16_t cursor_y = 0;

    gdi::ImageFrameApi & image_frame_api;

public:
    TimestampTracer timestamp_tracer;
};


struct FullVideoCaptureImpl final
  : gdi::CaptureApi
  , public gdi::GraphicApiForwarder<VideoCaptureCtx::UpdatableDraw>
{
    FullVideoCaptureImpl(
        CaptureParams const & capture_params,
        RDPDrawable & drawable, gdi::ImageFrameApi & image_frame,
        VideoParams const & video_params, FullVideoParams const & full_video_params
    );

    ~FullVideoCaptureImpl();

    void draw(RDP::FrameMarker const & cmd) override;

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y
    ) override;

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

private:
    VideoCaptureCtx video_cap_ctx;
    video_recorder recorder;
};


class SequencedVideoCaptureImpl final
  : public gdi::CaptureApi
  , public gdi::GraphicApiForwarder<VideoCaptureCtx::UpdatableDraw>
{
public:
    SequencedVideoCaptureImpl(
        CaptureParams const & capture_params,
        unsigned png_width, unsigned png_height,
        /* const */RDPDrawable & drawable,
        gdi::ImageFrameApi & image_frame,
        VideoParams const& video_params,
        NotifyNextVideo & next_video_notifier);

    ~SequencedVideoCaptureImpl();

    void draw(RDP::FrameMarker const & cmd) override;

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now,
        uint16_t cursor_x, uint16_t cursor_y
    ) override;

    void next_video(MonotonicTimePoint now);

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

private:
    void ic_flush(const tm& now);

    void next_video_impl(MonotonicTimePoint now, NotifyNextVideo::Reason reason);

    // first next_video is ignored
    WaitingTimeBeforeNextSnapshot first_periodic_snapshot(MonotonicTimePoint now);
    WaitingTimeBeforeNextSnapshot video_sequencer_periodic_snapshot(MonotonicTimePoint now);

    void init_recorder();

    struct FilenameGenerator
    {
        FilenameGenerator(
            std::string_view prefix,
            std::string_view filename,
            std::string_view extension
        );

        void next();
        char const* current_filename() const;

    private:
        std::string filename;
        int const num_pos;
        int num = 0;
    };

    bool ic_has_first_img = false;

    const MonotonicTimePoint monotonic_start_capture;
    MonotonicTimeToRealTime monotonic_to_real;

    VideoCaptureCtx video_cap_ctx;
    FilenameGenerator vc_filename_generator;
    std::optional<video_recorder> recorder;
    FilenameGenerator ic_filename_generator;
    const VideoParams video_params;
    int const groupid;
    AclReportApi * const acl_report;

    /* const */ RDPDrawable & ic_drawable;

    gdi::ImageFrameApi & image_frame_api;

    ScaledPng24 ic_scaled_png;

    MonotonicTimePoint start_break;
    const std::chrono::microseconds break_interval;

    NotifyNextVideo & next_video_notifier;
};
