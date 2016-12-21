/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/


#pragma once

#include <algorithm>

#include "capture/png_params.hpp"
#include "transport/out_meta_sequence_transport.hpp"

#include "utils/drawable.hpp"


static void scale_data(uint8_t *dest, const uint8_t *src,
                       unsigned int dest_width, unsigned int src_width,
                       unsigned int dest_height, unsigned int src_height,
                       unsigned int src_rowsize) {
    const uint32_t Bpp = 3;
    unsigned int y_pixels = dest_height;
    unsigned int y_int_part = src_height / dest_height * src_rowsize;
    unsigned int y_fract_part = src_height % dest_height;
    unsigned int yE = 0;
    unsigned int x_int_part = src_width / dest_width * Bpp;
    unsigned int x_fract_part = src_width % dest_width;

    while (y_pixels-- > 0) {
        unsigned int xE = 0;
        const uint8_t * x_src = src;
        unsigned int x_pixels = dest_width;
        while (x_pixels-- > 0) {
            dest[0] = x_src[2];
            dest[1] = x_src[1];
            dest[2] = x_src[0];

            dest += Bpp;
            x_src += x_int_part;
            xE += x_fract_part;
            if (xE >= dest_width) {
                xE -= dest_width;
                x_src += Bpp;
            }
        }
        src += y_int_part;
        yE += y_fract_part;
        if (yE >= dest_height) {
            yE -= dest_height;
            src += src_rowsize;
        }
    }
}

class PngCapture : public gdi::UpdateConfigCaptureApi, public gdi::CaptureApi
{
public:
    OutFilenameSequenceTransport trans;
    timeval start_capture;
    std::chrono::microseconds frame_interval;

    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    Drawable & drawable;
    std::unique_ptr<uint8_t[]> scaled_buffer;

    PngCapture(
        const timeval & now, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const PngParams & png_params) 
    : trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_tmp_path, basename, ".png", groupid, authentifier)
    , start_capture(now)
    , frame_interval(png_params.png_interval)
    , zoom_factor(png_params.zoom)
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    {}

private:
    void update_config(Inifile const & ini) override {
    }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        (void)x;
        (void)y;
        (void)ignore_frame_in_timeval;
        using std::chrono::microseconds;
        uint64_t const duration = difftimeval(now, this->start_capture);
        uint64_t const interval = this->frame_interval.count();
        if (duration >= interval) {
            if (this->drawable.logical_frame_ended
                // Force snapshot if diff_time_val >= 1.5 x frame_interval.
                || (duration >= interval * 3 / 2)) {
                this->drawable.trace_mouse();
                tm ptm;
                localtime_r(&now.tv_sec, &ptm);
                this->drawable.trace_timestamp(ptm);
                if (this->zoom_factor == 100) {
                    // TODO we should have a variant of ::transport_dump_png24
                    // taking a Drawable as input
                    ::transport_dump_png24(
                        this->trans, this->drawable.data(),
                        this->drawable.width(), this->drawable.height(),
                        this->drawable.rowsize(), true);
                }
                else {
                    // TODO all the zoom thing could be hidden behind a
                    // special type of Drawable
                    scale_data(
                        this->scaled_buffer.get(), this->drawable.data(),
                        this->scaled_width, this->drawable.width(),
                        this->scaled_height, this->drawable.height(),
                        this->drawable.rowsize());
                    ::transport_dump_png24(
                        this->trans, this->scaled_buffer.get(),
                        this->scaled_width, this->scaled_height,
                        this->scaled_width * 3, false);
                }
                // TODO: showing hiding mouse/timestamp should be hidden
                // behind a special type of Drawable
                this->trans.next();
                this->drawable.clear_timestamp();
                this->start_capture = now;
                this->drawable.clear_mouse();

                return microseconds(interval ? interval - duration % interval : 0u);
            }
            else {
                // Wait 0.3 x frame_interval.
                return this->frame_interval / 3;
            }
        }
        return microseconds(interval - duration);
    }

    void do_pause_capture(timeval const & now) override {
        // Draw Pause message
        time_t rawtime = now.tv_sec;
        tm ptm;
        localtime_r(&rawtime, &ptm);
        this->drawable.trace_pausetimestamp(ptm);
        if (this->zoom_factor == 100) {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }
        else {
            scale_data(
                this->scaled_buffer.get(), this->drawable.data(),
                this->scaled_width, this->drawable.width(),
                this->scaled_height, this->drawable.height(),
                this->drawable.rowsize());
            ::transport_dump_png24(
                this->trans, this->scaled_buffer.get(),
                this->scaled_width, this->scaled_height,
                this->scaled_width * 3, false);
        }
        this->trans.next();
        this->drawable.clear_pausetimestamp();
        this->start_capture = now;
    }

};

class PngCaptureRT : public gdi::UpdateConfigCaptureApi, public gdi::CaptureApi
{
public:
    OutFilenameSequenceTransport trans;
    uint32_t num_start = 0;
    unsigned png_limit;
    
    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    Drawable & drawable;

    std::unique_ptr<uint8_t[]> scaled_buffer;

    timeval start_capture;
    std::chrono::microseconds frame_interval;
    
    bool enable_rt_display = false;

    PngCaptureRT(
        const timeval & now, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const PngParams & png_params) 
    : trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        record_tmp_path, basename, ".png", groupid, authentifier)
    , png_limit(png_params.png_limit)
    , zoom_factor(std::min(png_params.zoom, 100u))
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    , start_capture(now)
    , frame_interval(png_params.png_interval)
    {
        const unsigned zoom_width = (this->drawable.width() * this->zoom_factor) / 100;
        const unsigned zoom_height = (this->drawable.height() * this->zoom_factor) / 100;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    ~PngCaptureRT()
    {
        for(uint32_t until_num = this->trans.get_seqno() + 1; this->num_start < until_num; ++this->num_start){
            // unlink may fail, for instance if file does not exist, just don't care
            ::unlink(this->trans.seqgen()->get(this->num_start));
        }
    }

private:
    void update_config(Inifile const & ini) override {
        auto const old_enable_rt_display = this->enable_rt_display;
        this->enable_rt_display = ini.get<cfg::video::rt_display>();

        if (old_enable_rt_display == this->enable_rt_display) {
            return ;
        }

        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable real time: %d", int(this->enable_rt_display));
        }

        if (!this->enable_rt_display) {
            for(uint32_t until_num = this->trans.get_seqno() + 1; this->num_start < until_num; ++this->num_start){
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->trans.seqgen()->get(this->num_start));
            }
        }
    }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        if (this->enable_rt_display) {
            (void)x;
            (void)y;
            (void)ignore_frame_in_timeval;
            using std::chrono::microseconds;
            uint64_t const duration = difftimeval(now, this->start_capture);
            uint64_t const interval = this->frame_interval.count();
            if (duration >= interval) {
                if (this->drawable.logical_frame_ended
                    // Force snapshot if diff_time_val >= 1.5 x frame_interval.
                    || (duration >= interval * 3 / 2)) {
                    this->drawable.trace_mouse();
                    tm ptm;
                    localtime_r(&now.tv_sec, &ptm);
                    this->drawable.trace_timestamp(ptm);
                    if (this->zoom_factor == 100) {
                        ::transport_dump_png24(
                            this->trans, this->drawable.data(),
                            this->drawable.width(), this->drawable.height(),
                            this->drawable.rowsize(), true);
                    }
                    else {
                        scale_data(
                            this->scaled_buffer.get(), this->drawable.data(),
                            this->scaled_width, this->drawable.width(),
                            this->scaled_height, this->drawable.height(),
                            this->drawable.rowsize());
                        ::transport_dump_png24(
                            this->trans, this->scaled_buffer.get(),
                            this->scaled_width, this->scaled_height,
                            this->scaled_width * 3, false);
                    }
                    if (this->png_limit && this->trans.get_seqno() >= this->png_limit) {
                        // unlink may fail, for instance if file does not exist, just don't care
                        ::unlink(this->trans.seqgen()->get(this->trans.get_seqno() - this->png_limit));
                    }
                    this->trans.next();
                    this->drawable.clear_timestamp();
                    this->start_capture = now;
                    this->drawable.clear_mouse();

                    return microseconds(interval ? interval - duration % interval : 0u);
                }
                else {
                    // Wait 0.3 x frame_interval.
                    return this->frame_interval / 3;
                }
            }
            return microseconds(interval - duration);
        }
        return this->frame_interval;
    }

    void do_pause_capture(timeval const & now) override {
        if (this->enable_rt_display) {
            // Draw Pause message
            time_t rawtime = now.tv_sec;
            tm ptm;
            localtime_r(&rawtime, &ptm);
            this->drawable.trace_pausetimestamp(ptm);
            if (this->zoom_factor == 100) {
                ::transport_dump_png24(
                    this->trans, this->drawable.data(),
                    this->drawable.width(), this->drawable.height(),
                    this->drawable.rowsize(), true);
            }
            else {
                scale_data(
                    this->scaled_buffer.get(), this->drawable.data(),
                    this->scaled_width, this->drawable.width(),
                    this->scaled_height, this->drawable.height(),
                    this->drawable.rowsize());
                ::transport_dump_png24(
                    this->trans, this->scaled_buffer.get(),
                    this->scaled_width, this->scaled_height,
                    this->scaled_width * 3, false);
            }
            if (this->png_limit && this->trans.get_seqno() >= this->png_limit) {
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->trans.seqgen()->get(this->trans.get_seqno() - this->png_limit));
            }
            this->trans.next();
            this->drawable.clear_pausetimestamp();
            this->start_capture = now;
        }
    }

    void do_resume_capture(timeval const & now) override {
    }
};


