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

#include <memory>
#include <chrono>


struct timeval;
class video_recorder;
class RDPDrawable;

struct NotifyNextVideo : private noncopyable
{
    enum class reason { sequenced, external };
    virtual void notify_next_video(const timeval& now, reason) = 0;
    virtual ~NotifyNextVideo() = default;
};


class FullVideoCaptureImpl : public gdi::CaptureApi
{
public:
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

public:
    struct TmpFileTransport : public Transport
    {
        char tmp_filename[1024];
        char final_filename[1024];

        int fd;
        int groupid;

    public:
        TmpFileTransport(
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid
        );

        void seek(int64_t offset, int whence) override;

        ~TmpFileTransport();

    private:
        void do_send(const uint8_t * data, size_t len) override;
    } trans_tmp_file;

    RDPDrawable & drawable;

    FlvParams flv_params;
    std::unique_ptr<video_recorder> recorder;
    timeval start_video_capture;
    std::chrono::microseconds inter_frame_interval;
    bool no_timestamp;
};


class SequencedVideoCaptureImpl : public gdi::CaptureApi
{
    struct SequenceTransport : public Transport
    {
        char tmp_filename[1024];
        char final_filename[1024];

        struct FileGen {
            char path[1024];
            char base[1012];
            char ext[12];
            unsigned num = 0;

            void set_final_filename(char * final_filename, size_t final_filename_size);
        } filegen;

        int fd;
        int groupid;

    public:
        SequenceTransport(
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            auth_api * authentifier);

        void seek(int64_t offset, int whence) override;

        bool next() override;

        ~SequenceTransport();

    private:
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

    class VideoCapture
    {
        Transport & trans;
        FlvParams flv_params;

        RDPDrawable & drawable;
        std::unique_ptr<video_recorder> recorder;

        timeval start_video_capture;
        std::chrono::microseconds inter_frame_interval;
        bool no_timestamp;

    public:
        VideoCapture(
            const timeval & now,
            Transport & trans,
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
