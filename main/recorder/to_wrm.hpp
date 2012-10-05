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

#include "cipher.hpp"
#include "wrm_recorder.hpp"
#include "capture.hpp"

static inline void to_wrm(WRMRecorder& recorder, const char* outfile,
            std::size_t start, std::size_t stop, std::size_t interval,
            uint frame_limit,
            bool screenshot_start, bool screenshot_wrm,
            const char* metaname,
            CipherMode::enum_t mode,
            const unsigned char * key, const unsigned char * iv
)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    Capture capture(now, recorder.meta().width,
                    recorder.meta().height,
                    outfile, metaname,
                    0, 0, true,
                    mode, key, iv);
    recorder.consumer(&capture);
    uint64_t timercompute_microsec = 0;
    uint64_t timercompute_chunk_time_value = 0;
    
    timeval mstart = {0,0};
    while (recorder.reader.selected_next_order())
    {
        if (recorder.chunk_type() == WRMChunk::TIME_START){
            mstart = recorder.get_start_time_order();
            break;
        }
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
        {
            timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
            timercompute_microsec += timercompute_chunk_time_value;
            --recorder.remaining_order_count();
            mstart.tv_sec = 0;
            mstart.tv_usec = 0;
            break;
        }
        recorder.interpret_order();
    }

    static const uint64_t coeff_sec_to_usec = 1000000;
    uint64_t msec = coeff_sec_to_usec * start;
    uint64_t tmp = 0;
    if (msec > 0){
        tmp = timercompute_microsec;
        if (timercompute_microsec < msec){
            while (recorder.reader.selected_next_order())
            {
                if (recorder.chunk_type() == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                    timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
                    timercompute_microsec += timercompute_chunk_time_value;
                    --recorder.remaining_order_count();    
                    tmp = timercompute_microsec;
                    break;
                }
                recorder.interpret_order();
            }
        }
    }
    timercompute_microsec = 0;
    uint64_t mtime = tmp;

    if (start && !mtime){
        return /*0*/;
    }

    if (mstart.tv_sec != 0)
    {
        if (mtime){
            uint64_t tmp_usec = mstart.tv_usec + mtime;
            mstart.tv_sec += (tmp_usec / 1000000);
            mstart.tv_usec = (tmp_usec % 1000000);
        }
        capture.start(mstart);
    }
    else {
        capture.start_with_invalid_now();
    }

    if (mtime){
        capture.timestamp(mtime);
        capture.timer() += mtime;
    }

    if (screenshot_wrm && screenshot_start)
        capture.dump_png();

    //uint64_t chunk_time = 0;
    timercompute_microsec = mtime - start;
    uint frame = 0;
    uint64_t msecond = coeff_sec_to_usec * (stop - start);
    mtime = coeff_sec_to_usec * interval;

    while (recorder.reader.selected_next_order())
    {
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP) {
            timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
            timercompute_microsec += timercompute_chunk_time_value;
            --recorder.remaining_order_count();

            uint64_t usec = timercompute_microsec;
            if (timercompute_chunk_time_value) {
                //chunk_time += timercompute_chunk_time_value;
                //std::cout << "chunk_time: " << chunk_time << '\n';
                capture.timestamp(timercompute_chunk_time_value);
                capture.timer() += timercompute_chunk_time_value;
            }

            if (usec >= mtime) {
                /*if (chunk_time) {
                    std::cout << "timestamp + breakpoint chunk_time: " <<   chunk_time  << '\n';
                    capture.timestamp(chunk_time);
                    chunk_time = 0;
                }*/
                capture.breakpoint(capture.timer().time());
                if (screenshot_wrm)
                    capture.dump_png();
                timercompute_microsec = 0;
                if (++frame == frame_limit)
                    break;
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
                 std::cout << "timestamp chunk_time: " << chunk_time  << '\n';
                 capture.timestamp(chunk_time);
                 chunk_time = 0;
            }*/

            recorder.interpret_order();
        }
    }

    /*if (chunk_time) {
         capture.timestamp(chunk_time);
    }*/

    //return frame;
}
#endif
