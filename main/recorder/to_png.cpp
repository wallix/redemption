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

#if !defined(__MAIN_RECORDER_TO_PNG__)
#define __MAIN_RECORDER_TO_PNG__

#include "to_png.hpp"
#include "wrm_recorder_option.hpp"
#include "wrm_recorder.hpp"
#include "timer_compute.hpp"
#include "staticcapture.hpp"

void to_png(WRMRecorder& recorder, WrmRecorderOption& opt, const char* outfile)
{
    StaticCapture capture(recorder.meta().width,
                          recorder.meta().height,
                          outfile,
                          0, 0);
    recorder.consumer(&capture);
    TimerCompute timercompute(recorder);
    if (opt.range.left && !timercompute.advance_second(opt.range.left))
        return /*0*/;

    if (opt.screenshot_start)
        capture.dump_png();

    uint frame = 0;
    uint64_t mtime = TimerCompute::coeff_sec_to_usec * opt.time;
    uint64_t msecond = TimerCompute::coeff_sec_to_usec * (opt.range.right.time - opt.range.left.time);
    while (recorder.selected_next_order() && frame != opt.frame)
    {
        if (timercompute.interpret_is_time_chunk()){
            uint64_t usec = timercompute.usec();
            if (usec >= mtime){
                capture.dump_png();
                timercompute.reset();
                ++frame;
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
    if (!opt.no_screenshot_stop && msecond && frame != opt.frame){
        capture.dump_png();
        //++frame;
    }
    //return frame;
}

#endif