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

#if !defined(__MAIN_RECORDER_TO_WRM__)
#define __MAIN_RECORDER_TO_WRM__

#include "to_wrm.hpp"
#include "wrm_recorder_option.hpp"
#include "wrm_recorder.hpp"
#include "timer_compute.hpp"
#include "capture.hpp"

void to_wrm(WRMRecorder& recorder, WrmRecorderOption& opt, const char* outfile)
{
    Capture capture(
        recorder.meta().width,
                    recorder.meta().height,
                    outfile,
                    opt.metaname.empty() ? 0 : opt.metaname.c_str(),
                    0, 0, true
    );
    recorder.consumer(&capture);
    TimerCompute timercompute(recorder);
    uint64_t msecond = timercompute.start();

    //std::cout << "start: " << msecond << '\n';
    uint64_t mtime = timercompute.advance_second(opt.range.left);

    //std::cout << "mtime: " << mtime << '\n';
    if (opt.range.left && !mtime)
        return /*0*/;
    if (msecond){
        capture.timestamp(msecond);
        msecond += mtime;
        capture.timer().sec()  = msecond / TimerCompute::coeff_sec_to_usec;
        capture.timer().usec() = msecond % TimerCompute::coeff_sec_to_usec;
    }

    if (opt.screenshot_wrm && opt.screenshot_start)
        capture.dump_png();

    //uint64_t chunk_time = 0;
        timercompute.usec() = mtime - opt.range.left;
        uint frame = 0;
        mtime = TimerCompute::coeff_sec_to_usec * opt.time;
        msecond = TimerCompute::coeff_sec_to_usec * (opt.range.right.time - opt.range.left.time);

        while (recorder.selected_next_order() && frame != opt.frame)
        {
            if (timercompute.interpret_is_time_chunk()) {
                uint64_t usec = timercompute.usec();
                if (timercompute.chunk_time_value) {
                    //chunk_time += timercompute.chunk_time_value;
                    //std::cout << "chunk_time: " << chunk_time << '\n';
                    capture.timestamp(timercompute.chunk_time_value);
                    capture.timer() += timercompute.chunk_time_value;
                }

                if (usec >= mtime) {
                    /*if (chunk_time) {
                     *                   std::cout << "timestamp + breakpoint chunk_time: " << chunk_time  << '\n';
                     *                   capture.timestamp(chunk_time);
                     *                   chunk_time = 0;
                    }*/
                    capture.breakpoint();
                    if (opt.screenshot_wrm)
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
            }
            else {
                /*if (chunk_time) {
                 *               std::cout << "timestamp chunk_time: " << chunk_time  << '\n';
                 *               capture.timestamp(chunk_time);
                 *               chunk_time = 0;
                }*/

                recorder.interpret_order();
            }
        }

        /*if (chunk_time) {
         *       capture.timestamp(chunk_time);
        }*/

        //return frame;
}

#endif