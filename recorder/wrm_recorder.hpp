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
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 */

#if !defined(__WRM_RECORDER_HPP__)
#define __WRM_RECORDER_HPP__

#include <errno.h>
#include <vector>

#include "transport.hpp"
#include "FileToGraphic.hpp"
#include "RDP/RDPSerializer.hpp"
#include "meta_file.hpp"
#include "RDP/RDPDrawable.hpp"
#include "bitmap.hpp"
#include "stream.hpp"
#include "png.hpp"
#include "error.hpp"
#include "auto_buffer.hpp"
#include "cipher_transport.hpp"
#include "zlib.hpp"
#include "range_time_point.hpp"
#include "relative_time_point.hpp"
#include "capture.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"


class WRMRecorder
{
public:
    InFileTransport & trans;
    FileToGraphic & reader;

    Drawable * redrawable;


public:
    WRMRecorder(const timeval & now, InFileTransport & trans,  FileToGraphic & reader,  range_time_point & range)
    : trans(trans)
    , reader(reader)
    , redrawable(0)
    {
    }

    ~WRMRecorder()
    {
    }

public:

    void interpret_order()
    {
        switch (this->reader.chunk_type)
        {
            default:
                this->reader.interpret_order();
                break;
        }
    }

    bool next_order()
    {
        if (this->reader.next_order()){
            this->interpret_order();
            return true;
        }
        return false;
    }
    
    void to_png(const char* outfile,
                std::size_t start, std::size_t stop, std::size_t interval,
                unsigned resize_width, unsigned resize_height,
                uint frame_limit,
                bool screenshot_start, bool no_screenshot_stop,
                bool screenshot_all)
    {
//        printf("to png -> %s width=%u height=%u\n", outfile, this->reader.data_meta.width, this->reader.data_meta.height);

//        StaticCapture capture(this->reader.data_meta.width, this->reader.data_meta.height, outfile, true);

//        this->reader.consumer = &capture;
//        const uint64_t coeff_sec_to_usec = 1000000;
//        uint64_t timercompute_microsec = 0;
//        uint64_t timercompute_chunk_time_value = 0;
//        if (start > 0){
//            uint64_t msec = coeff_sec_to_usec * start;
//            if (timercompute_microsec < msec){
//                while (this->reader.next_order())
//                {
//                    if (this->reader.chunk_type == TIMESTAMP && timercompute_microsec < msec){
//                        timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                        timercompute_microsec += timercompute_chunk_time_value;
//                        break;
//                    }
//                    this->interpret_order();
//                }
//            }
//            if (timercompute_microsec == 0){
//                return /*0*/;
//            }
//            timercompute_microsec = 0;
//        }
//        if (1 && screenshot_start){
//            capture.dump_png();
//        }

//        uint frame = 0;
//        uint64_t mtime = coeff_sec_to_usec * interval;
//        uint64_t msecond = coeff_sec_to_usec * (stop - start);
//        uint64_t minterval = 0;

//        while (this->reader.next_order())
//        {
//            if (this->reader.chunk_type == TIMESTAMP) {
//                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                timercompute_microsec += timercompute_chunk_time_value;
//                uint64_t usec = timercompute_microsec;
//                if (usec >= mtime)
//                {
//                    capture.dump_png();
//                    timercompute_microsec = 0;
//                    if (++frame == frame_limit){
//                        break;
//                    }
//                    if (screenshot_all){
//                        minterval += usec - mtime;
//                        while (minterval >= mtime){
//                            capture.dump_png();
//                            minterval -= mtime;
//                        }
//                    }
//                }

//                if (msecond <= usec){
//                    msecond = 0;
//                    break;
//                }
//                else{
//                    msecond -= usec;
//                }
//            }
//            else{
//                this->interpret_order();
//            }
//        }
//        if (!no_screenshot_stop && msecond && frame != frame_limit){
//            capture.dump_png();
//            //++frame;
//        }
//        LOG(LOG_INFO, "to png -> %s done", outfile);
//        //return frame;
    }

    void to_png_list(const char* outfile,
                const std::vector<relative_time_point>& capture_points,
                unsigned resize_width, unsigned resize_height,
                bool no_screenshot_stop)
    {
//        if (capture_points.empty()){
//            return ;
//        }

//        StaticCapture capture(this->reader.data_meta.width, this->reader.data_meta.height, outfile, true);
//        this->reader.consumer = &capture;

//        typedef std::vector<relative_time_point>::const_iterator iterator;
//        iterator it = capture_points.begin();

//        const uint64_t coeff_sec_to_usec = 1000000;
//        uint64_t timercompute_microsec = 0;
//        uint64_t timercompute_chunk_time_value = 0;
//        uint64_t start = it->point.time;
//        if (start){
//            uint64_t msec = coeff_sec_to_usec * start;
//            uint64_t tmp = 0;
//            if (msec > 0){
//                tmp = timercompute_microsec;
//                if (timercompute_microsec < msec){
//                    while (this->reader.next_order())
//                    {
//                        if (this->reader.chunk_type == TIMESTAMP && timercompute_microsec < msec){
//                            timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                            timercompute_microsec += timercompute_chunk_time_value;
//                            tmp = timercompute_microsec;
//                            break;
//                        }
//                        this->interpret_order();
//                    }
//                }
//            }
//            timercompute_microsec = 0;
//            if (tmp == 0){
//                return /*0*/;
//            }
//        }

//        capture.dump_png();
//        iterator end = capture_points.end();
//        if (++it == end)
//            return;
//        uint64_t mtime = 0;

//        while (this->reader.next_order())
//        {
//            if (this->reader.chunk_type == TIMESTAMP) {
//                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                timercompute_microsec += timercompute_chunk_time_value;
//                mtime += timercompute_microsec;
//                while (mtime >= coeff_sec_to_usec * it->point.time)
//                {
//                    capture.dump_png();
//                    if (++it == end)
//                        return;
//                }
//                timercompute_microsec = 0;
//            }
//            else
//            {
//                this->interpret_order();
//            }
//        }
//        if (!no_screenshot_stop){
//            capture.dump_png();
//        }
    }
    
    void to_one_wrm(const char* outfile,
                    std::size_t start, std::size_t stop, const char* metaname,
                    CipherMode::enum_t mode,
                    const unsigned char * key, const unsigned char * iv)
    {
//        LOG(LOG_INFO, "to one wrm");
//        timeval now;
//        gettimeofday(&now, NULL);

//        NativeCapture capture(now,
//                              this->reader.data_meta.width,
//                              this->reader.data_meta.height,
//                              outfile, metaname
//                             );
//                             
//        this->reader.consumer = &capture;
//        uint64_t timercompute_microsec = 0;
//        uint64_t timercompute_chunk_time_value = 0;

//        timeval mstart = {0,0};
//        while (this->reader.next_order())
//        {
//            if (this->reader.chunk_type == TIMESTAMP){
//                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                timercompute_microsec += timercompute_chunk_time_value;
//                mstart.tv_sec = 0;
//                mstart.tv_usec = 0;
//                break;
//            }
//            this->interpret_order();
//        }

//        const uint64_t coeff_sec_to_usec = 1000000;
//        uint64_t msec = coeff_sec_to_usec * start;
//        uint64_t mtime = 0;
//        if (msec > 0){
//            mtime = timercompute_microsec;
//            if (timercompute_microsec < msec){
//                while (this->reader.next_order())
//                {
//                    if (this->reader.chunk_type == TIMESTAMP && timercompute_microsec < msec){
//                        timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                        timercompute_microsec += timercompute_chunk_time_value;
//                        mtime = timercompute_microsec;
//                        break;
//                    }
//                    this->interpret_order();
//                }
//            }
//        }
//        timercompute_microsec = 0;

//        if (start && !mtime){
//            return /*0*/;
//        }

//        if (mstart.tv_sec != 0){
//            if (mtime){
//                uint64_t tmp_usec = mstart.tv_usec + mtime;
//                mstart.tv_sec += (tmp_usec / 1000000);
//                mstart.tv_usec = (tmp_usec % 1000000);
//            }
////            capture.send_time_start(mstart);
//        }
//        else {
////            capture.write_start_in_meta(mstart);
//        }

//        if (mtime){
//            capture.recorder.timestamp(mtime);
//            mtime += capture.recorder.timer.tv_usec;
//            capture.recorder.timer.tv_sec += mtime / 1000000;
//            capture.recorder.timer.tv_usec = mtime % 1000000;
//        }

//        timercompute_microsec = mtime - start;
//        mtime = coeff_sec_to_usec * (stop - start);
//        BStream& stream = this->reader.stream;

//        while (this->reader.next_order())
//        {
//            if (this->reader.chunk_type == TIMESTAMP) {
//                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                timercompute_microsec += timercompute_chunk_time_value;
//                if (timercompute_chunk_time_value) {
//                    capture.recorder.timestamp(timercompute_chunk_time_value);
//                    timercompute_chunk_time_value += capture.recorder.timer.tv_usec;
//                    capture.recorder.timer.tv_sec += timercompute_chunk_time_value / 1000000;
//                    capture.recorder.timer.tv_usec = timercompute_chunk_time_value % 1000000;
//                }

//                if (mtime <= timercompute_microsec){
//                    break;
//                }
//            }
//            else {
//                switch (this->reader.chunk_type) {
//                    case NEXT_FILE_ID:
//                        this->interpret_order();
//                        break;
//                    case META_FILE:
//                    case TIME_START:
//                        this->reader.stream.p = this->reader.stream.end;
//                        break;
//                    case BREAKPOINT:
//                    {
//                        this->reader.stream.p = this->reader.stream.end;
//                        this->reader.next_order();

//                        while (1)
//                        {
//                            this->reader.stream.init(14);
//                            this->reader.trans->recv(&this->reader.stream.end, 14);
//                            if (this->reader.stream.in_uint16_le() == 8192 * 3 + 1){
//                                break;
//                            }
//                            this->reader.stream.p += 8;
//                            uint32_t buffer_size = this->reader.stream.in_uint32_le();
//                            this->reader.stream.init(buffer_size);
//                            this->reader.trans->recv(&this->reader.stream.end, buffer_size);
//                        }

//                        this->reader.stream.p = this->reader.stream.end;
//                    }
//                        break;
//                    default:
//                        capture.recorder.trans->send(stream.data, stream.size());
//                        this->reader.stream.p = this->reader.stream.end;
//                        break;
//                }
//            }
//        }
    } 
    
    
    void to_wrm(const char* outfile,
                std::size_t start, std::size_t stop, std::size_t interval,
                uint frame_limit,
                bool screenshot_start, bool screenshot_wrm,
                const char* metaname,
                CipherMode::enum_t mode,
                const unsigned char * key, const unsigned char * iv
    )
    {
//        struct timeval now;
//        gettimeofday(&now, NULL);
//        Capture capture(now, this->reader.data_meta.width,
//                        this->reader.data_meta.height,
//                        outfile, metaname,
//                        0, 0, true,
//                        mode, key, iv);

//        static const uint64_t coeff_sec_to_usec = 1000000;

//        this->reader.consumer = &capture;

//        uint64_t usec_start_rec = 0;
//        while (this->reader.next_order())
//        {
//            if (this->reader.chunk_type == TIMESTAMP)
//            {
//                uint64_t usec_start = this->reader.stream.in_uint64_be();
//                if (usec_start_rec == 0){
//                    usec_start_rec = usec_start + start * coeff_sec_to_usec;
//                }
//                if (usec_start >= usec_start_rec){
//                    break;
//                }
//            }
//            this->interpret_order();
//        }

//        if (mtime){
//            this->reader.consumer->timestamp(mtime);
//            timeval & timer = this->reader.consumer->timer();
//            mtime += timer.tv_usec;
//            timer.tv_sec += mtime / 1000000;
//            timer.tv_usec = mtime % 1000000;
//        }

//        if (screenshot_wrm && screenshot_start)
//            this->reader.consumer->dump_png();

//        //uint64_t chunk_time = 0;
//        timercompute_microsec = mtime - start;
//        uint frame = 0;
//        uint64_t msecond = coeff_sec_to_usec * (stop - start);
//        mtime = coeff_sec_to_usec * interval;

//        while (this->reader.next_order())
//        {
//            if (this->reader.chunk_type == TIMESTAMP) {
//                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
//                timercompute_microsec += timercompute_chunk_time_value;

//                uint64_t usec = timercompute_microsec;
//                if (timercompute_chunk_time_value) {
//                    //chunk_time += timercompute_chunk_time_value;
//                    //std::cout << "chunk_time: " << chunk_time << '\n';
//                    capture.timestamp(timercompute_chunk_time_value);
//                    timeval now;
//                    gettimeofday(&now, NULL);
//                    timeval & timer = capture.timer();
//                    timercompute_chunk_time_value += timer.tv_usec;
//                    timer.tv_sec += timercompute_chunk_time_value / 1000000;
//                    timer.tv_usec = timercompute_chunk_time_value % 1000000;
//                }

//                if (usec >= mtime) {
//                    /*if (chunk_time) {
//                        std::cout << "timestamp + breakpoint chunk_time: " <<   chunk_time  << '\n';
//                        capture.timestamp(chunk_time);
//                        chunk_time = 0;
//                    }*/
//                    capture.breakpoint(capture.timer());
//                    if (screenshot_wrm)
//                        capture.dump_png();
//                    timercompute_microsec = 0;
//                    if (++frame == frame_limit)
//                        break;
//                }

//                if (msecond <= usec){
//                    msecond = 0;
//                    break;
//                } else {
//                    msecond -= usec;
//                }
//            }
//            else {
//                /*if (chunk_time) {
//                     std::cout << "timestamp chunk_time: " << chunk_time  << '\n';
//                     capture.timestamp(chunk_time);
//                     chunk_time = 0;
//                }*/

//                this->interpret_order();
//            }
//        }

//        /*if (chunk_time) {
//             capture.timestamp(chunk_time);
//        }*/

//        //return frame;
    }
    
       
};

#endif
