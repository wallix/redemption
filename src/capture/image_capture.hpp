/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Meng Tan
*/

#ifndef _REDEMPTION_CAPTURE_STATICCAPTURE_HPP_
#define _REDEMPTION_CAPTURE_STATICCAPTURE_HPP_

#include "capture/drawable_to_file.hpp"
#include "utils/difftimeval.hpp"
#include "transport/transport.hpp"

#include "gdi/capture_api.hpp"
#include "utils/dummypng.cpp"

class ImageCapture : private DrawableToFile, public gdi::CaptureApi
{
    timeval start_capture;
    std::chrono::microseconds frame_interval;

public:
    ImageCapture (
        const timeval & now, const Drawable & drawable, Transport & trans,
        std::chrono::microseconds png_interval)
    : DrawableToFile(trans, drawable)
    , start_capture(now)
    , frame_interval(png_interval)
    {}

    using DrawableToFile::zoom;

    void breakpoint(const timeval & now)
    {
        tm ptm;
        localtime_r(&now.tv_sec, &ptm);
        const_cast<Drawable&>(this->drawable).trace_timestamp(ptm);
        this->flush_png();
        const_cast<Drawable&>(this->drawable).clear_timestamp();
    }

private:
    void flush_png()
    {
        this->flush();
        this->trans.next();
    }

    std::chrono::microseconds do_snapshot(
        const timeval & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        using std::chrono::microseconds;
        uint64_t const duration = difftimeval(now, this->start_capture);
        uint64_t const interval = this->frame_interval.count();
        if (duration >= interval) {
            if (   this->logical_frame_ended()
                // Force snapshot if diff_time_val >= 1.5 x frame_interval.
                || (duration >= interval * 3 / 2)) {
                const_cast<Drawable&>(this->drawable).trace_mouse();
                this->breakpoint(now);
                this->start_capture = now;
                const_cast<Drawable&>(this->drawable).clear_mouse();

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
        const_cast<Drawable&>(this->drawable).trace_pausetimestamp(ptm);
        this->flush_png();
        const_cast<Drawable&>(this->drawable).clear_pausetimestamp();
        this->start_capture = now;
    }
};

#endif
