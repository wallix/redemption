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

#include "image_capture.hpp"

#include <stdio.h>
#include <png.h>

#include "bitmap.hpp"
#include "rect.hpp"
#include "difftimeval.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"

#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "png.hpp"
#include "error.hpp"
#include "config.hpp"
#include "colors.hpp"

#include "RDP/RDPDrawable.hpp"
#include "config.hpp"
#include "sequence_generator.hpp"

struct StaticCaptureConfig {
    unsigned png_limit;
    uint64_t png_interval;
    char path[1024];
    bool bgr;

    StaticCaptureConfig()
    : png_limit(3) {}
};

class StaticCapture : public ImageCapture, public RDPCaptureDevice {
public:
    bool clear_png;
    SequenceGenerator const * seq;
    StaticCaptureConfig conf;

    timeval start_static_capture;
    uint64_t inter_frame_interval_static_capture;
    uint64_t time_to_wait;

    bool    first_picture_capture_delayed;
    timeval first_picture_capture_now;
    uint32_t rt_display;

    StaticCapture(const timeval & now, Transport & trans, SequenceGenerator const * seq, unsigned width, unsigned height,
                  bool clear_png, const Inifile & ini, Drawable & drawable)
    : ImageCapture(trans, width, height, drawable)
    , clear_png(clear_png)
    , seq(seq)
    , start_static_capture(now)
    , time_to_wait(0)
    , first_picture_capture_delayed(true)
    , first_picture_capture_now(now)
    , rt_display(0)
    {
        this->conf.png_interval = 3000; // png interval is in 1/10 s, default value, 1 static snapshot every 5 minutes
        this->inter_frame_interval_static_capture = this->conf.png_interval * 100000; // 1 000 000 us is 1 sec
        this->update_config(ini);
    }

    virtual ~StaticCapture() {
        if (this->first_picture_capture_delayed) {
            this->breakpoint(this->first_picture_capture_now);
        }

        // delete all captured files at the end of the RDP client session
        if (this->clear_png){
            this->unlink_filegen(0);
        }
    }

private:
    void unlink_filegen(size_t end)
    {
        for(size_t i = this->conf.png_limit ; i > end ; i--) {
            if (this->trans.get_seqno() >= i){
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->seq->get(this->trans.get_seqno() - i));
            }
        }
    }

public:
    void update_config(const Inifile & ini){
        if (ini.video.png_limit < this->conf.png_limit) {
            this->unlink_filegen(ini.video.png_limit);
        }
        this->conf.png_limit = ini.video.png_limit;

        if (ini.video.png_interval != this->conf.png_interval) {
            // png interval is in 1/10 s, default value, 1 static snapshot every 5 minutes
            this->conf.png_interval = ini.video.png_interval;
            this->inter_frame_interval_static_capture = this->conf.png_interval * 100000; // 1 000 000 us is 1 sec
        }
        this->rt_display = ini.video.rt_display.get();
    }

    virtual void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval) {
        if (!this->rt_display) {
            this->time_to_wait = 0;
            return;
        }
        unsigned diff_time_val = static_cast<unsigned>(difftimeval(now, this->start_static_capture));
        if (diff_time_val >= static_cast<unsigned>(this->inter_frame_interval_static_capture)) {
            if (   this->drawable.logical_frame_ended
                // Force snapshot if diff_time_val >= 1,5 x inter_frame_interval_static_capture.
                || (diff_time_val >= static_cast<unsigned>(this->inter_frame_interval_static_capture) * 3 / 2)) {
                this->drawable.trace_mouse();
                this->breakpoint(now);
                this->start_static_capture = addusectimeval(this->inter_frame_interval_static_capture, this->start_static_capture);
                this->drawable.clear_mouse();
            }
            else {
                if (this->first_picture_capture_delayed) {
                    ::memcpy(&this->first_picture_capture_now, &now, sizeof(this->first_picture_capture_now));
                }
                // Wait 0,3 x inter_frame_interval_static_capture.
                this->time_to_wait = this->inter_frame_interval_static_capture / 3;
                return;
            }
        }
        this->time_to_wait = this->inter_frame_interval_static_capture - difftimeval(now, this->start_static_capture);
    }

private:
    void flush_png()
    {
        if (this->conf.png_limit > 0){
            if (this->trans.get_seqno() >= this->conf.png_limit) {
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->seq->get(this->trans.get_seqno() - this->conf.png_limit));
            }
            this->flush();
            this->trans.next();
        }
    }

public:
    void pause_snapshot(const timeval & now) {
        // Draw Pause message
        time_t rawtime = now.tv_sec;
        tm ptm;
        localtime_r(&rawtime, &ptm);
        this->drawable.trace_pausetimestamp(ptm);
        this->flush_png();
        this->drawable.clear_pausetimestamp();
        this->start_static_capture = now;
    }

    void breakpoint(const timeval & now)
    {
        this->first_picture_capture_delayed = false;

        time_t rawtime = now.tv_sec;
        tm ptm;
        localtime_r(&rawtime, &ptm);
        this->drawable.trace_timestamp(ptm);
        this->flush_png();
        this->drawable.clear_timestamp();
    }
};

#endif
