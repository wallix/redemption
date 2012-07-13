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
    GraphicsToFile& caprecorder = capture.recorder;

    if (start && !mtime)
        return /*0*/;
    if (mstart.tv_sec != 0){
        capture.send_time_start(mstart);
    }
    if (mtime){
        caprecorder.timestamp(mtime);
        caprecorder.timer += mtime;
    }

    timercompute.usec() = mtime - start;
    mtime = TimerCompute::coeff_sec_to_usec * (stop - start);
    BStream& stream = recorder.reader.stream;

    while (recorder.selected_next_order())
    {
        if (timercompute.interpret_is_time_chunk()) {
            if (timercompute.chunk_time_value) {
                caprecorder.timestamp(timercompute.chunk_time_value);
                caprecorder.timer += timercompute.chunk_time_value;
            }

            if (mtime <= timercompute.usec()){
                break;
            }
        }
        else {
            switch (recorder.chunk_type()) {
                case WRMChunk::NEXT_FILE_ID:
                    recorder.interpret_order();
                    break;
                case WRMChunk::META_FILE:
                case WRMChunk::TIME_START:
                    recorder.ignore_chunks();
                    break;
                case WRMChunk::BREAKPOINT:
                {
                    stream.p += 2 + 2 + 1 + 8 + 8;
                    uint nb_img = stream.in_uint16_le() * stream.in_uint16_le();

                    --recorder.remaining_order_count();

                    while (nb_img)
                    {
                        recorder.selected_next_order();
                        nb_img -= recorder.remaining_order_count();
                        recorder.ignore_chunks();
                    }

                    recorder.selected_next_order();

                    stream.p = stream.end - 2;
                    recorder.remaining_order_count() = 0;
                    nb_img = stream.in_uint16_le();

                    for (uint ignore = nb_img + 27; ignore; --ignore){
                        recorder.selected_next_order();
                        recorder.ignore_chunks();
                    }
                }
                    break;
                default:
                    caprecorder.trans->send(stream.data,
                                            stream.end - stream.data);
                    recorder.ignore_chunks();
                    break;
            }
        }
    }

    //return frame;
}
