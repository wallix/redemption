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

#include "capture/video_recorder.hpp"

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
        FlvParams flv_params)
    : trans(trans)
    , flv_params(std::move(flv_params))
    , drawable(drawable)
    , start_video_capture(now)
    , inter_frame_interval(1000000L / this->flv_params.frame_rate)
    , no_timestamp(no_timestamp)
    {
        if (this->flv_params.verbosity) {
            LOG(LOG_INFO, "Video recording %d x %d, rate: %d, qscale: %d, brate: %d, codec: %s",
                this->flv_params.target_width, this->flv_params.target_height,
                this->flv_params.frame_rate, this->flv_params.qscale, this->flv_params.bitrate,
                this->flv_params.codec.c_str());
        }

        this->next_video();
    }

    void next_video() {
        if (this->recorder) {
            this->recorder.reset();
            this->trans.next();
        }

        io_video_recorder_with_transport io{this->trans};
        this->recorder.reset(new video_recorder(
            io.write_fn(), io.seek_fn(), io.params(),
            drawable.width(), drawable.height(),
            drawable.pix_len(),
            drawable.data(),
            this->flv_params.bitrate,
            this->flv_params.frame_rate,
            this->flv_params.qscale,
            this->flv_params.codec.c_str(),
            this->flv_params.target_width,
            this->flv_params.target_height,
            this->flv_params.verbosity
        ));
    }

    void preparing_video_frame() {
        this->drawable.trace_mouse();
        if (!this->no_timestamp) {
            time_t rawtime = this->start_video_capture.tv_sec;
            tm tm_result;
            localtime_r(&rawtime, &tm_result);
            this->drawable.trace_timestamp(tm_result);
        }
        this->recorder->preparing_video_frame(true);
        if (!this->no_timestamp) { this->drawable.clear_timestamp(); }
        this->drawable.clear_mouse();
    }

    void encoding_video_frame() {
        this->recorder->encoding_video_frame();
    }

    std::chrono::microseconds frame_marker_event(
        const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) {
        this->preparing_video_frame();
        return std::chrono::microseconds{};
    }

    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
    ) {
        uint64_t tick = difftimeval(now, this->start_video_capture);
        uint64_t const inter_frame_interval = this->inter_frame_interval.count();
        if (tick >= inter_frame_interval) {
            auto encoding_video_frame = [this](time_t rawtime){
                this->drawable.trace_mouse();
                if (!this->no_timestamp) {
                    tm tm_result;
                    localtime_r(&rawtime, &tm_result);
                    this->drawable.trace_timestamp(tm_result);
                    this->recorder->encoding_video_frame();
                    this->drawable.clear_timestamp();
                }
                else {
                    this->recorder->encoding_video_frame();
                }
                this->drawable.clear_mouse();
            };

            if (ignore_frame_in_timeval) {
                auto const nframe = tick / inter_frame_interval;
                encoding_video_frame(this->start_video_capture.tv_sec);
                auto const usec = inter_frame_interval * nframe;
                auto sec = usec / 1000000LL;
                this->start_video_capture.tv_usec += usec - sec * inter_frame_interval;
                if (this->start_video_capture.tv_usec >= 1000000LL){
                    this->start_video_capture.tv_usec -= 1000000LL;
                    ++sec;
                }
                this->start_video_capture.tv_sec += sec;
                tick -= inter_frame_interval * nframe;
            }
            else {
                do {
                    encoding_video_frame(this->start_video_capture.tv_sec);
                    this->start_video_capture.tv_usec += inter_frame_interval;
                    if (this->start_video_capture.tv_usec >= 1000000LL){
                        this->start_video_capture.tv_sec += 1;
                        this->start_video_capture.tv_usec -= 1000000LL;
                    }
                    tick -= inter_frame_interval;
                } while (tick >= inter_frame_interval);
            }
        }

        return std::chrono::microseconds(inter_frame_interval - tick);
    }
    
    std::chrono::microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) {
        // assert(now >= previous);
        auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        assert(next_duration.count() >= 0);
        return next_duration;
    }

};

