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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Meng Tan
*/

#ifndef _REDEMPTION_CAPTURE_NATIVECAPTURE_HPP_
#define _REDEMPTION_CAPTURE_NATIVECAPTURE_HPP_

#include "utils/difftimeval.hpp"
#include "capture/GraphicToFile.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/dump_png24.hpp"

class NativeCapture
: public gdi::CaptureApi
, public gdi::ExternalEventApi
, public gdi::ConfigUpdaterApi
{
    uint64_t frame_interval;
    timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    GraphicToFile & recorder;
    uint64_t time_to_wait;

public:
    NativeCapture( GraphicToFile & recorder, const timeval & now, const Inifile & ini)
    : recorder(recorder)
    , time_to_wait(0)
    {
        // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
        this->start_native_capture = now;
        this->frame_interval = 40;
        this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec

        this->start_break_capture = now;
        this->break_interval = 60 * 10; // break interval is in s, default value 1 break every 10 minutes
        this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec

        this->update_config(ini);
    }

    ~NativeCapture() override {
        this->recorder.sync();
    }

    void update_config(const Inifile & ini) override
    {
        if (ini.get<cfg::video::frame_interval>() != this->frame_interval){
            // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
            this->frame_interval = ini.get<cfg::video::frame_interval>();
            this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec
        }

        if (ini.get<cfg::video::break_interval>() != this->break_interval){
            this->break_interval = ini.get<cfg::video::break_interval>(); // break interval is in s, default value 1 break every 10 minutes
            this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec
        }
    }

    std::chrono::microseconds snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval) override {
        if (difftimeval(now, this->start_native_capture)
                >= this->inter_frame_interval_native_capture) {
            this->recorder.timestamp(now);
            this->time_to_wait = this->inter_frame_interval_native_capture;
            this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
            this->start_native_capture = now;
            if ((difftimeval(now, this->start_break_capture) >=
                 this->inter_frame_interval_start_break_capture)) {
                this->recorder.breakpoint();
                this->start_break_capture = now;
            }
        }
        else {
            this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
        }
        return std::chrono::microseconds{this->time_to_wait};
    }

    // toggles externally genareted breakpoint.
    void external_breakpoint() override {
        this->recorder.breakpoint();
    }

    void external_time(const timeval & now) override {
        this->recorder.sync();
        this->recorder.timestamp(now);
    }
};

#endif
