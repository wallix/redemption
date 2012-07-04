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

#if !defined(__MAIN_RECORDER_WRM_RECORDER_APP__)
#define __MAIN_RECORDER_WRM_RECORDER_APP__

#include "wrm_recorder.hpp"

#include "capture.hpp"

#include "wrm_recorder_option.hpp"
#include "get_type.hpp"

class WrmRecorderApp
{
    WrmRecoderOption& opt;

public:
    WrmRecorderApp(WrmRecoderOption& opt)
    : opt(opt)
    {
    }

    std::string get_out_filename() const
    {
        const std::size_t pos = opt.out_filename.find_last_of('.');
        return this->opt.out_filename.substr(0, pos);
    }

    const std::string& get_in_filename() const
    {
        return this->opt.in_filename;
    }

    void run(OutputType::enum_t otype, InputType::enum_t itype)
    {
        this->run(this->get_out_filename().c_str(), otype, itype);
    }

    void run(const char* outfile, OutputType::enum_t otype, InputType::enum_t itype)
    {
        //WRMRecorder recorder(opt.in_filename, opt.base_path);
        WRMRecorder recorder;
        recorder.set_basepath(opt.base_path);
        recorder.only_filename = opt.ignore_dir_for_meta_in_wrm;

        switch (itype) {
            case InputType::WRM_TYPE:
                recorder.open_wrm_followed_meta(opt.in_filename.c_str());
                break;
            case InputType::META_TYPE:
                recorder.open_meta_followed_wrm(opt.in_filename.c_str());
                break;
            default:
                throw std::runtime_error("input type not found");
        }

        if (!recorder.is_meta_chunk())
           throw std::runtime_error("chunk meta not found in output file");

        this->run_otype(recorder, outfile, otype);
    }

protected:
    virtual void run_otype(WRMRecorder& recorder, const char* outfile,
                           OutputType::enum_t otype)
    {
        switch (otype) {
            case OutputType::PNG_TYPE:
                this->png_run(recorder, outfile);
                break;
            case OutputType::WRM_TYPE:
                this->wrm_run(recorder, outfile);
                break;
            default:
                throw std::runtime_error("output type not found");
        }
    }

protected:
    class TimerCompute {
        uint64_t micro_sec;

    public:
        static const uint64_t coeff_sec_to_usec = 1000000;

    public:
        WRMRecorder& recorder;
        uint64_t chunk_time_value;

    public:
        TimerCompute(WRMRecorder& recorder)
        : micro_sec(0)
        , recorder(recorder)
        , chunk_time_value(0)
        {}

        void reset()
        {
            this->micro_sec = 0;
        }

        uint64_t& usec()
        { return this->micro_sec; }

        uint64_t sec()
        { return this->micro_sec / coeff_sec_to_usec; }

        void interpret_time()
        {
            this->chunk_time_value = this->recorder.reader.stream.in_uint64_be();
            this->micro_sec += this->chunk_time_value;
            --this->recorder.remaining_order_count();
        }

        bool interpret_is_time_chunk()
        {
            if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
            {
                //std::cout << this->micro_sec << " -> ";
                this->interpret_time();
                //std::cout << this->micro_sec << " (" << this->chunk_time_value << ")\n";
                return true;
            }
            return false;
        }

        uint64_t start()
        {
            uint64_t time_start = 0;
            if (this->recorder.selected_next_order())
            {
                if (this->recorder.chunk_type() == WRMChunk::TIMESTAMP)
                {
                    this->interpret_time();
                }
                else
                {
                    this->recorder.interpret_order();
                }
            }
            return time_start;
        }

        uint64_t advance_usecond(uint msec)
        {
            if (!msec)
                return 0;

            while (this->recorder.selected_next_order())
            {
                if (this->interpret_is_time_chunk())
                {
                    if (this->micro_sec >= msec)
                    {
                        uint64_t tmp = this->micro_sec;
                        this->reset();
                        return tmp;
                    }
                }
                else
                    this->recorder.interpret_order();
            }
            return 0;
        }

        uint64_t advance_second(uint second)
        {
            return this->advance_usecond(coeff_sec_to_usec * second);
        }
    };

    uint png_run(WRMRecorder& recorder, const char* outfile)
    {
        StaticCapture capture(
            recorder.meta().width,
            recorder.meta().height,
            outfile,
            0, 0
        );
        recorder.consumer(&capture);
        TimerCompute timercompute(recorder);
        if (this->opt.range.left && !timercompute.advance_second(this->opt.range.left))
            return 0;

        if (this->opt.screenshot_start)
            capture.dump_png();

        uint frame = 0;
        uint64_t mtime = TimerCompute::coeff_sec_to_usec * this->opt.time;
        uint64_t msecond = TimerCompute::coeff_sec_to_usec * (this->opt.range.right.time - this->opt.range.left.time);
        while (recorder.selected_next_order() && frame != this->opt.frame)
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
        if (!this->opt.no_screenshot_stop
            && msecond && frame != this->opt.frame){
            capture.dump_png();
            ++frame;
        }
        return frame;
    }

    uint wrm_run(WRMRecorder& recorder, const char* outfile)
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

        std::cout << "start: " << msecond << '\n';
        uint64_t mtime = timercompute.advance_second(this->opt.range.left);

        std::cout << "mtime: " << mtime << '\n';
        if (this->opt.range.left && !mtime)
            return 0;
        if (msecond){
            capture.timestamp(msecond);
            msecond += mtime;
            capture.timer().sec()  = msecond / TimerCompute::coeff_sec_to_usec;
            capture.timer().usec() = msecond % TimerCompute::coeff_sec_to_usec;
        }

        if (this->opt.screenshot_wrm && this->opt.screenshot_start)
            capture.dump_png();

        //uint64_t chunk_time = 0;
        timercompute.usec() = mtime - this->opt.range.left;
        uint frame = 0;
        mtime = TimerCompute::coeff_sec_to_usec * this->opt.time;
        msecond = TimerCompute::coeff_sec_to_usec * (this->opt.range.right.time - this->opt.range.left.time);

        while (recorder.selected_next_order() && frame != this->opt.frame)
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
                        std::cout << "timestamp + breakpoint chunk_time: " << chunk_time  << '\n';
                        capture.timestamp(chunk_time);
                        chunk_time = 0;
                    }*/
                    capture.breakpoint();
                    if (this->opt.screenshot_wrm)
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

        return frame;
    }
};

#endif