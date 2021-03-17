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
#include "transport/file_transport.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/timestamp_tracer.hpp"
#include "utils/monotonic_time_to_real_time.hpp"
#include "utils/scaled_image24.hpp"

#include <memory>
#include <chrono>

class CaptureParams;
class FullVideoParams;
class RDPDrawable;


struct VideoCaptureCtx : noncopyable
{
    enum class ImageByInterval : bool
    {
        One,
        ZeroOrOne,
    };

    enum class TraceTimestamp : bool
    {
        No,
        Yes,
    };

    VideoCaptureCtx(
        MonotonicTimePoint monotonic_now,
        RealTimePoint real_time,
        TraceTimestamp trace_timestamp,
        ImageByInterval image_by_interval,
        unsigned frame_rate,
        RDPDrawable & drawable,
        gdi::ImageFrameApi & image_frame
    );

    void frame_marker_event(video_recorder & recorder);
    void encoding_video_frame(video_recorder & recorder);
    gdi::CaptureApi::WaitingTimeBeforeNextSnapshot snapshot(
        video_recorder& recorder, MonotonicTimePoint now);
    void next_video();

    [[nodiscard]] uint16_t width() const noexcept;

    [[nodiscard]] uint16_t height() const noexcept;

    [[nodiscard]] size_t pix_len() const noexcept;

    [[nodiscard]] const uint8_t * data() const noexcept;

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

private:
    void preparing_video_frame(video_recorder & recorder);

    RDPDrawable & drawable;
    MonotonicTimePoint monotonic_last_time_capture;
    const MonotonicTimePoint monotonic_start_capture;
    MonotonicTimeToRealTime monotonic_to_real;
    const std::chrono::microseconds frame_interval;
    std::chrono::microseconds current_video_time;
    int64_t start_frame_index;

    TraceTimestamp trace_timestamp;
    ImageByInterval image_by_interval;
    time_t previous_second = 0;
    bool has_frame_marker = false;

    gdi::ImageFrameApi & image_frame_api;

public:
    TimestampTracer timestamp_tracer;
};


struct FullVideoCaptureImpl final : gdi::CaptureApi
{
    FullVideoCaptureImpl(
        CaptureParams const & capture_params,
        RDPDrawable & drawable, gdi::ImageFrameApi & image_frame,
        VideoParams const & video_params, FullVideoParams const & full_video_params
    );

    ~FullVideoCaptureImpl();

    void frame_marker_event(
        MonotonicTimePoint /*now*/, uint16_t /*cursor_x*/, uint16_t /*cursor_y*/
    ) override;

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y
    ) override;

    void encoding_video_frame();

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

private:
    VideoCaptureCtx video_cap_ctx;
    video_recorder recorder;
};


class SequencedVideoCaptureImpl final : public gdi::CaptureApi
{
public:
    SequencedVideoCaptureImpl(
        CaptureParams const & capture_params,
        unsigned png_width, unsigned png_height,
        /* const */RDPDrawable & drawable,
        gdi::ImageFrameApi & image_frame,
        VideoParams const& video_params,
        NotifyNextVideo & next_video_notifier);

    void frame_marker_event(
        MonotonicTimePoint now, uint16_t cursor_x, uint16_t cursor_y
    ) override;

    WaitingTimeBeforeNextSnapshot periodic_snapshot(
        MonotonicTimePoint now,
        uint16_t cursor_x, uint16_t cursor_y
    ) override;

    void ic_flush();

    void next_video(MonotonicTimePoint now);

    void encoding_video_frame();

    void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

private:
    void next_video_impl(MonotonicTimePoint now, NotifyNextVideo::Reason reason);

    // first next_video is ignored
    WaitingTimeBeforeNextSnapshot first_periodic_snapshot(MonotonicTimePoint now);
    WaitingTimeBeforeNextSnapshot video_sequencer_periodic_snapshot(MonotonicTimePoint now);

    struct SequenceTransport
    {
        std::string filename;
        int const num_pos;
        int const groupid;
        int num = 0;
        AclReportApi * const acl_report;

        SequenceTransport(
            std::string_view prefix,
            std::string_view filename,
            std::string_view extension,
            const int groupid,
            AclReportApi * acl_report
        );

        void next();
    };

    struct VideoCapture
    {
        VideoCapture(
            CaptureParams const & capture_params,
            RDPDrawable & drawable,
            gdi::ImageFrameApi & image_frame,
            VideoParams const & video_params
        );

        ~VideoCapture();

        void next_video();

        void encoding_video_frame();

        void frame_marker_event();

        WaitingTimeBeforeNextSnapshot periodic_snapshot(MonotonicTimePoint now);

        void trace_timestamp(const tm & now);

        void clear_timestamp();

        void prepare_video_frame();

        void synchronize_times(MonotonicTimePoint monotonic_time, RealTimePoint real_time);

    private:
        VideoCaptureCtx video_cap_ctx;
        SequenceTransport trans;
        std::unique_ptr<video_recorder> recorder;
        const VideoParams video_params;
        gdi::ImageFrameApi & image_frame_api;
    };

    bool ic_has_first_img = false;

    const MonotonicTimePoint monotonic_start_capture;
    MonotonicTimeToRealTime monotonic_to_real;

    VideoCapture vc;
    SequenceTransport ic_trans;

    /* const */ RDPDrawable & ic_drawable;

    gdi::ImageFrameApi & image_frame_api;

    ScaledPng24 ic_scaled_png;

    MonotonicTimePoint start_break;
    const std::chrono::microseconds break_interval;

    NotifyNextVideo & next_video_notifier;
};
