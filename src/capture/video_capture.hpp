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

#include "gdi/capture_api.hpp"
#include "transport/transport.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "flv_params.hpp"
#include "capture/video_recorder.hpp"
#include "capture/notify_next_video.hpp"

#include <memory>
#include <chrono>


struct timeval;
class video_recorder;
class RDPDrawable;


struct VideoTransportBase : Transport
{
    VideoTransportBase(const int groupid, auth_api * authentifier);

    void seek(int64_t offset, int whence) override;

    ~VideoTransportBase();

protected:
    void force_open();
    void rename();
    bool is_open() const;
    void do_send(const uint8_t * data, size_t len) override;

private:
    int fd;
    const int groupid;

    char tmp_filename[1024];

protected:
    char final_filename[1024];
};


struct VideoCaptureCtx : noncopyable
{
    VideoCaptureCtx(
        timeval const & now,
        bool no_timestamp,
        unsigned frame_rate,
        RDPDrawable & drawable
    );

    void frame_marker_event(video_recorder &);
    void encoding_video_frame(video_recorder &);
    std::chrono::microseconds snapshot(video_recorder &, timeval const & now, bool ignore_frame_in_timeval);

private:
    void preparing_video_frame(video_recorder &);

    RDPDrawable & drawable;
    timeval start_video_capture;
    std::chrono::microseconds frame_interval;
    bool no_timestamp;
    time_t previous_second = 0;
    bool has_frame_marker = false;
};


struct FullVideoCaptureImpl : gdi::CaptureApi
{
    FullVideoCaptureImpl(
        const timeval & now, const char * const record_path, const char * const basename,
        const int groupid, bool no_timestamp, RDPDrawable & drawable, FlvParams flv_params
    );

    ~FullVideoCaptureImpl();

    void frame_marker_event(
        const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override;

    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
    ) override;

    std::chrono::microseconds periodic_snapshot(
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
            auth_api * authentifier
        );
    } trans_tmp_file;

    video_recorder recorder;
    VideoCaptureCtx video_cap_ctx;
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
            auth_api * authentifier
        );

        bool next() override;

        ~SequenceTransport();

    private:
        void set_final_filename();
        void do_send(const uint8_t * data, size_t len) override;
    };

    bool ic_has_first_img = false;

public:
    std::chrono::microseconds do_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override;

    void frame_marker_event(
        timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override;

    std::chrono::microseconds periodic_snapshot(
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

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        );

        void frame_marker_event(timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval);

        std::chrono::microseconds do_snapshot(
            const timeval& now, int x, int y, bool ignore_frame_in_timeval
        );
    } first_image;

public:
    SequenceTransport vc_trans;

    struct VideoCapture
    {
        VideoCapture(
            const timeval & now,
            SequenceTransport & trans,
            RDPDrawable & drawable,
            bool no_timestamp,
            FlvParams flv_params
        );

        ~VideoCapture();

        void next_video();

        void encoding_video_frame();

        void frame_marker_event(const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/);

        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
        );

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        );

    private:
        VideoCaptureCtx video_cap_ctx;
        std::unique_ptr<video_recorder> recorder;
        SequenceTransport & trans;
        FlvParams flv_params;
        RDPDrawable & drawable;
    } vc;

    SequenceTransport ic_trans;

    unsigned ic_zoom_factor;
    unsigned ic_scaled_width;
    unsigned ic_scaled_height;

    /* const */ RDPDrawable & ic_drawable;

private:
    std::unique_ptr<uint8_t[]> ic_scaled_buffer;

public:
    void zoom(unsigned percent);

    void ic_flush();

    void dump24();

    void scale_dump24();

    class VideoSequencer : public gdi::CaptureApi
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

        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
        ) override;

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) override;

        void frame_marker_event(
            timeval const & now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
        ) override;
    } video_sequencer;

    NotifyNextVideo & next_video_notifier;

    void next_video_impl(const timeval& now, NotifyNextVideo::reason reason);

public:
    SequencedVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        unsigned image_zoom,
        /* const */RDPDrawable & drawable,
        FlvParams flv_params,
        std::chrono::microseconds video_interval,
        NotifyNextVideo & next_video_notifier);

    void next_video(const timeval& now);

    void encoding_video_frame();
};
