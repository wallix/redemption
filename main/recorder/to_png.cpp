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

#include "to_png.hpp"
#include "wrm_recorder_option.hpp"
#include "wrm_recorder.hpp"
#include "timer_compute.hpp"
#include "staticcapture.hpp"

void to_png(WRMRecorder& recorder, const char* outfile,
            std::size_t start, std::size_t stop, std::size_t interval,
            uint frame_limit,
            bool screenshot_start, bool no_screenshot_stop)
{
    StaticCapture capture(recorder.meta().width,
                          recorder.meta().height,
                          outfile,
                          0, 0);
    recorder.consumer(&capture);
    TimerCompute timercompute(recorder);
    if (start && !timercompute.advance_second(start))
        return /*0*/;

    if (screenshot_start)
        capture.dump_png();

    uint frame = 0;
    uint64_t mtime = TimerCompute::coeff_sec_to_usec * interval;
    uint64_t msecond = TimerCompute::coeff_sec_to_usec * (stop - start);
    while (recorder.selected_next_order())
    {
        if (timercompute.interpret_is_time_chunk()){
            uint64_t usec = timercompute.usec();
            if (usec >= mtime){
                capture.dump_png();
                timercompute.reset();
                if (++frame == frame_limit)
                    break;
            }
            if (msecond <= usec){
                msecond = 0;
                break;
            } else {
                msecond -= usec;
            }
        } else {
            recorder.interpret_order();
        }
    }
    if (!no_screenshot_stop && msecond && frame != frame_limit){
        capture.dump_png();
        //++frame;
    }
    //return frame;
}
