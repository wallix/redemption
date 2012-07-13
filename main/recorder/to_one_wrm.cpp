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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#include <iostream>

#include "to_one_wrm.hpp"
#include "timer_compute.hpp"
#include "nativecapture.hpp"

void to_one_wrm(WRMRecorder& recorder, const char* outfile,
            std::size_t start, std::size_t stop, const char* metaname
)
{
    NativeCapture capture(recorder.meta().width,
                          recorder.meta().height,
                          outfile, metaname);
    recorder.consumer(&capture);
    TimerCompute timercompute(recorder);

    timeval mstart = timercompute.start();
    uint64_t mtime = timercompute.advance_second(start);

    if (start && !mtime)
        return /*0*/;
    if (mstart.tv_sec != 0){
        capture.send_time_start(mstart);
    }
    if (mtime){
        capture.recorder.timestamp(mtime);
        capture.recorder.timer += mtime;
    }

    timercompute.usec() = mtime - start;
    mtime = TimerCompute::coeff_sec_to_usec * (stop - start);

    while (recorder.selected_next_order())
    {
        if (timercompute.interpret_is_time_chunk()) {
            if (timercompute.chunk_time_value) {
                capture.recorder.timestamp(timercompute.chunk_time_value);
                capture.recorder.timer += timercompute.chunk_time_value;
            }

            if (mtime >= timercompute.usec()){
                break;
            }
        }
        else {
            recorder.interpret_order();
        }
    }

    //return frame;
}
