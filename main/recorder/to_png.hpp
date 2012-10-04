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

#include <vector>

#include "relative_time_point.hpp"
#include "wrm_recorder_option.hpp"
#include "staticcapture.hpp"

static inline void to_png(WRMRecorder& recorder, const char* outfile,
            std::size_t start, std::size_t stop, std::size_t interval,
            unsigned resize_width, unsigned resize_height,
            uint frame_limit,
            bool screenshot_start, bool no_screenshot_stop,
            bool screenshot_all)
{
    printf("to png -> %s width=%u height=%u\n", outfile, recorder.meta().width, recorder.meta().height);

    StaticCapture capture(recorder.meta().width, recorder.meta().height, outfile, true);

    recorder.consumer(&capture);
    recorder.load_png_context(capture.drawable);
    const uint64_t coeff_sec_to_usec = 1000000;
    uint64_t timercompute_microsec = 0;
    uint64_t timercompute_chunk_time_value = 0;
    if (start > 0){
        uint64_t msec = coeff_sec_to_usec * start;
        if (timercompute_microsec < msec){
            while (recorder.reader.selected_next_order())
            {
                if (recorder.chunk_type() == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                    timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
                    timercompute_microsec += timercompute_chunk_time_value;
                    --recorder.remaining_order_count();    
                    break;
                }
                recorder.interpret_order();
            }
        }
        if (timercompute_microsec == 0){
            return /*0*/;
        }
        timercompute_microsec = 0;
    }
    if (1 && screenshot_start){
        capture.dump_png();
    }

    uint frame = 0;
    uint64_t mtime = coeff_sec_to_usec * interval;
    uint64_t msecond = coeff_sec_to_usec * (stop - start);
    uint64_t minterval = 0;

    while (recorder.reader.selected_next_order())
    {
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP) {
            timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
            timercompute_microsec += timercompute_chunk_time_value;
            --recorder.remaining_order_count();
            uint64_t usec = timercompute_microsec;
            if (usec >= mtime)
            {
                capture.dump_png();
                timercompute_microsec = 0;
                if (++frame == frame_limit){
                    break;
                }
                if (screenshot_all){
                    minterval += usec - mtime;
                    while (minterval >= mtime){
                        capture.dump_png();
                        minterval -= mtime;
                    }
                }
            }

            if (msecond <= usec){
                msecond = 0;
                break;
            }
            else{
                msecond -= usec;
            }
        }
        else{
            recorder.interpret_order();
        }
    }
    if (!no_screenshot_stop && msecond && frame != frame_limit){
        capture.dump_png();
        //++frame;
    }
    LOG(LOG_INFO, "to png -> %s done", outfile);
    //return frame;
}

static inline void to_png_2(WRMRecorder& recorder, const char* outfile,
            const std::vector<relative_time_point>& capture_points,
            unsigned resize_width, unsigned resize_height,
            bool no_screenshot_stop)
{
    if (capture_points.empty()){
        return ;
    }

    StaticCapture capture(recorder.meta().width, recorder.meta().height, outfile, true);
    recorder.consumer(&capture);
    recorder.load_png_context(capture.drawable);

    typedef std::vector<relative_time_point>::const_iterator iterator;
    iterator it = capture_points.begin();

    const uint64_t coeff_sec_to_usec = 1000000;
    uint64_t timercompute_microsec = 0;
    uint64_t timercompute_chunk_time_value = 0;
    uint64_t start = it->point.time;
    if (start){
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
        if (tmp == 0){
            return /*0*/;
        }
    }

    capture.dump_png();
    iterator end = capture_points.end();
    if (++it == end)
        return;
    uint64_t mtime = 0;

    while (recorder.reader.selected_next_order())
    {
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP) {
            timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
            timercompute_microsec += timercompute_chunk_time_value;
            --recorder.remaining_order_count();
            mtime += timercompute_microsec;
            while (mtime >= coeff_sec_to_usec * it->point.time)
            {
                capture.dump_png();
                if (++it == end)
                    return;
            }
            timercompute_microsec = 0;
        }
        else
        {
            recorder.interpret_order();
        }
    }
    if (!no_screenshot_stop){
        capture.dump_png();
    }
}

#endif
