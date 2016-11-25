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


#pragma once

#include "utils/difftimeval.hpp"
#include "capture/GraphicToFile.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/dump_png24.hpp"

class NativeCapture
: public gdi::CaptureApi
, public gdi::ExternalCaptureApi
{
    timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    GraphicToFile & recorder;
    uint64_t time_to_wait;

public:
    NativeCapture(
        GraphicToFile & recorder,
        const timeval & now,
        std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
        std::chrono::seconds break_interval
    )
    : start_native_capture(now)
    , inter_frame_interval_native_capture(
        std::chrono::duration_cast<std::chrono::microseconds>(frame_interval).count())
    , start_break_capture(now)
    , inter_frame_interval_start_break_capture(
        std::chrono::duration_cast<std::chrono::microseconds>(break_interval).count())
    , recorder(recorder)
    , time_to_wait(0)
    {}

    ~NativeCapture() override {
        this->recorder.sync();
    }

    // toggles externally genareted breakpoint.
    void external_breakpoint() override {
        this->recorder.breakpoint();
    }

    void external_time(const timeval & now) override {
        this->recorder.sync();
        this->recorder.timestamp(now);
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        (void)ignore_frame_in_timeval;
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
};

