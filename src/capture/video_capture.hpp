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

#include "capture/video_params.hpp"
#include "capture/video_recorder.hpp"
#include "capture/notify_next_video.hpp"
#include "gdi/capture_api.hpp"
#include "transport/out_file_transport.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/timestamp_tracer.hpp"

#include <memory>
#include <chrono>

class CaptureParams;
class FullVideoParams;
class RDPDrawable;

struct VideoTransportBase : Transport
{
    VideoTransportBase(const int groupid, ReportMessageApi * report_message);

    void seek(int64_t offset, int whence) override;

    ~VideoTransportBase();

protected:
    void force_open();
    void rename();
    bool is_open() const;
    void do_send(const uint8_t * data, size_t len) override;

private:
    OutFileTransport out_file;
    const int groupid;

    char tmp_filename[1128];

protected:
    char final_filename[1024];
    bool status = true;
};


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
        timeval const & now,
        TraceTimestamp trace_timestamp,
        ImageByInterval image_by_interval,
        unsigned frame_rate,
        RDPDrawable & drawable,
        gdi::ImageFrameApi & imageFrameApi
    );

    void frame_marker_event(video_recorder & /*recorder*/);
    void encoding_video_frame(video_recorder & /*recorder*/);
    gdi::CaptureApi::Microseconds snapshot(
        video_recorder & /*recorder*/, timeval const & now, bool ignore_frame_in_timeval);
    void next_video();

    uint16_t width() const noexcept;

    uint16_t height() const noexcept;

    size_t pix_len() const noexcept;

    const uint8_t * data() const noexcept;

private:
    void preparing_video_frame(video_recorder & /*recorder*/);

    RDPDrawable & drawable;
    timeval start_video_capture;
    std::chrono::microseconds frame_interval;
    std::chrono::microseconds current_video_time;
    uint64_t start_frame_index;

    TraceTimestamp trace_timestamp;
    ImageByInterval image_by_interval;
    time_t previous_second = 0;
    bool has_frame_marker = false;

    gdi::ImageFrameApi & image_frame_api;

public:
    TimestampTracer timestamp_tracer;
};


struct FullVideoCaptureImpl : gdi::CaptureApi
{
    FullVideoCaptureImpl(
        CaptureParams const & capture_params,
        RDPDrawable & drawable, gdi::ImageFrameApi & imageFrameApi,
        VideoParams const & video_params, FullVideoParams const & full_video_params
    );

    ~FullVideoCaptureImpl();

    void frame_marker_event(
        const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override;

    Microseconds periodic_snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override;

    void encoding_video_frame();

private:
    struct TmpFileTransport final : VideoTransportBase
    {
        TmpFileTransport(
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            ReportMessageApi * report_message
        );
    } trans_tmp_file;

    VideoCaptureCtx video_cap_ctx;
    video_recorder recorder;
};


class SequencedVideoCaptureImpl : public gdi::CaptureApi
{
    struct SequenceTransport final : VideoTransportBase
    {
        struct FileGen {
            char path[1024];
            char base[1012];
            char ext[12];
            unsigned num = 0;
        } filegen;

        SequenceTransport(
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            ReportMessageApi * report_message
        );

        bool next() override;

        ~SequenceTransport();

    private:
        void set_final_filename();
        void do_send(const uint8_t * data, size_t len) override;
    };

    bool ic_has_first_img = false;

public:
    void frame_marker_event(
        timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override;

    Microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override;

public:
    // first next_video is ignored
    struct FirstImage
    {
        SequencedVideoCaptureImpl & first_image_impl;

        const timeval first_image_start_capture;

        FirstImage(timeval const & now, SequencedVideoCaptureImpl & impl)
        : first_image_impl(impl)
        , first_image_start_capture(now)
        {}

        Microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        );

        void frame_marker_event(timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval);
    } first_image;

public:
    SequenceTransport vc_trans;

    struct VideoCapture
    {
        VideoCapture(
            const timeval & now,
            SequenceTransport & trans,
            RDPDrawable & drawable,
            gdi::ImageFrameApi & imageFrameApi,
            VideoParams const& video_params
        );

        ~VideoCapture();

        void next_video();

        void encoding_video_frame();

        void frame_marker_event(const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/);

        Microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        );

        void trace_timestamp(const tm & now);

        void clear_timestamp();

        void prepare_video_frame();

    private:
        VideoCaptureCtx video_cap_ctx;
        std::unique_ptr<video_recorder> recorder;
        SequenceTransport & trans;
        const VideoParams video_params;
        gdi::ImageFrameApi & image_frame_api;
    } vc;

    SequenceTransport ic_trans;

    unsigned ic_zoom_factor;
    unsigned ic_scaled_width;
    unsigned ic_scaled_height;

    /* const */ RDPDrawable & ic_drawable;

    gdi::ImageFrameApi & image_frame_api;

    SmartVideoCropping smart_video_cropping;

private:
    std::unique_ptr<uint8_t[]> ic_scaled_buffer;

public:
    void zoom(unsigned percent);

    void ic_flush();

    void dump24();

    void scale_dump24();

    class VideoSequencer
    {
        timeval start_break;
        std::chrono::microseconds break_interval;

    protected:
        SequencedVideoCaptureImpl & impl;

    public:
        VideoSequencer(const timeval & now, std::chrono::microseconds break_interval, SequencedVideoCaptureImpl & impl)
        : start_break(now)
        , break_interval(break_interval)
        , impl(impl)
        {}

        std::chrono::microseconds get_interval() const
        {
            return this->break_interval;
        }

        void reset_now(const timeval& now);

        Microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        );

        void frame_marker_event(
            timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
        );
    } video_sequencer;

    NotifyNextVideo & next_video_notifier;

    void next_video_impl(const timeval& now, NotifyNextVideo::reason reason);

public:
    SequencedVideoCaptureImpl(
        CaptureParams const & capture_params,
        unsigned image_zoom,
        /* const */RDPDrawable & drawable,
        gdi::ImageFrameApi & imageFrameApi,
        VideoParams const& video_params,
        NotifyNextVideo & next_video_notifier);

    void next_video(const timeval& now);

    void encoding_video_frame();
};
