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

#if !defined(__MAIN_RECORDER_TO_ONE_WRM__)
#define __MAIN_RECORDER_TO_ONE_WRM__

#include "cipher.hpp"
#include "wrm_recorder.hpp"
#include "nativecapture.hpp"

class WRMRecorder;

void to_one_wrm(WRMRecorder& recorder, const char* outfile,
                std::size_t start, std::size_t stop, const char* metaname,
                CipherMode::enum_t mode,
                const unsigned char * key, const unsigned char * iv
)
{
    LOG(LOG_INFO, "to one wrm");
    NativeCapture capture(recorder.meta().width,
                          recorder.meta().height,
                          outfile, metaname,
                          mode, key, iv
                         );
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
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP){
            timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
            timercompute_microsec += timercompute_chunk_time_value;
            --recorder.remaining_order_count();
            mstart.tv_sec = 0;
            mstart.tv_usec = 0;
            break;
        }
        recorder.interpret_order();
    }

    const uint64_t coeff_sec_to_usec = 1000000;
    uint64_t msec = coeff_sec_to_usec * start;
    uint64_t mtime = 0;
    if (msec > 0){
        mtime = timercompute_microsec;
        if (timercompute_microsec < msec){
            while (recorder.reader.selected_next_order())
            {
                if (recorder.chunk_type() == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                    timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
                    timercompute_microsec += timercompute_chunk_time_value;
                    --recorder.remaining_order_count();    
                    mtime = timercompute_microsec;
                    break;
                }
                recorder.interpret_order();
            }
        }
    }
    timercompute_microsec = 0;

    GraphicsToFile& caprecorder = capture.recorder;

    if (start && !mtime)
        return /*0*/;

    if (mstart.tv_sec != 0){
        if (mtime){
            uint64_t tmp_usec = mstart.tv_usec + mtime;
            mstart.tv_sec += (tmp_usec / 1000000);
            mstart.tv_usec = (tmp_usec % 1000000);
        }
        capture.send_time_start(mstart);
    }
    else
        capture.write_start_in_meta(mstart);

    if (mtime){
        caprecorder.timestamp(mtime);
        caprecorder.timer += mtime;
    }

    timercompute_microsec = mtime - start;
    mtime = coeff_sec_to_usec * (stop - start);
    BStream& stream = recorder.reader.stream;

    while (recorder.reader.selected_next_order())
    {
        if (recorder.chunk_type() == WRMChunk::TIMESTAMP) {
            timercompute_chunk_time_value = recorder.reader.stream.in_uint64_be();
            timercompute_microsec += timercompute_chunk_time_value;
            --recorder.remaining_order_count();
            if (timercompute_chunk_time_value) {
                caprecorder.timestamp(timercompute_chunk_time_value);
                caprecorder.timer += timercompute_chunk_time_value;
            }

            if (mtime <= timercompute_microsec){
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
                    recorder.reader.stream.p = recorder.reader.stream.end;
                    recorder.reader.remaining_order_count = 0;
                    break;
                case WRMChunk::BREAKPOINT:
                {
                    recorder.reader.stream.p = recorder.reader.stream.end;
                    recorder.reader.remaining_order_count = 0;
                    recorder.reader.selected_next_order();

                    while (1)
                    {
                        recorder.reader.stream.init(14);
                        recorder.reader.trans->recv(&recorder.reader.stream.end, 14);
                        if (recorder.reader.stream.in_uint16_le() == 8192 * 3 + 1){
                            break;
                        }
                        recorder.reader.stream.p += 8;
                        uint32_t buffer_size = recorder.reader.stream.in_uint32_le();
                        recorder.reader.stream.init(buffer_size);
                        recorder.reader.trans->recv(&recorder.reader.stream.end, buffer_size);
                    }

                    recorder.reader.stream.p = recorder.reader.stream.end;
                    recorder.reader.remaining_order_count = 0;
                }
                    break;
                default:
                    caprecorder.trans->send(stream.data,
                                            stream.size());
                    recorder.reader.stream.p = recorder.reader.stream.end;
                    recorder.reader.remaining_order_count = 0;
                    break;
            }
        }
    }

    //return frame;
}
#endif
