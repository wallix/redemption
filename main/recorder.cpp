/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Jonathan Poelen
 *
 * recorder main program
 *
 */

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#define PRINT_LOG
#define LOG_PRINT

#include "range_time_point.hpp"
#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "capture.hpp"
#include "wrm_recorder.hpp"

// class NativeCapture;
// class StaticCapture;

namespace po = boost::program_options;


void validate(boost::any& v,
              const std::vector<std::string>& values,
              range_time_point* range, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    v = boost::any(range_time_point(s));
}

void validate(boost::any& v,
              const std::vector<std::string>& values,
              time_point* time, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    v = boost::any(time_point(s));
}

struct RecorderError{
    enum {
        SUCCESS = 0,
        HELP = 1,
        SYNCHRONISE_ERROR = 2,
        NOT_OUTPUT_FILE = 3,
    };
};

const char * recorder_string_errors[] = {
    "SUCCESS",
    "HELP",
    "SYNCHRONISE_ERROR",
    "NOT_OUTPUT_FILE"
};

struct WrmRecoderOption {
    po::options_description desc;
    po::variables_map options;

    range_time_point range;
    uint frame;
    time_point time;
    std::string out_filename;
    std::string wrm_in_filename;
    bool synchronise_screen;

    WrmRecoderOption()
    : desc("Options")
    , options()
    , range()
    , frame(std::numeric_limits<uint>::max())
    , time(60*2)
    , out_filename()
    , wrm_in_filename()
    , synchronise_screen(false)
    {
    }

    void add_default_options()
    {
        this->desc.add_options()
        // --help, -h
        ("help,h", "produce help message")
        // --version, -v
        ("version,v", "show software version")
        ("range,r", po::value(&this->range),
        "range capture\n"
        "range format:\n"
        "[[[+|-]time[h|m|s][...]][,][[+]time[h|m|s][[+|-]time[h|m|s][...]]]]\n"
        "example:\n"
        "1h30,+10m -> from 1h30 to 1h40\n"
        "20m+2h-50s,3h -> from 2h19m10s to 3h"
        )
        ("frame,f", po::value(&this->frame), "maximum frame for png capture option")
        ("time,t", po::value(&this->time), "time capture for 1 file. format: [+|-]time[h|m|s][...]")
        //("synchronise,s", "")
        ("input-file,i", po::value(&this->wrm_in_filename), "wrm filename")
        ("output-file,o", po::value(&this->out_filename), "png or wrm filename")
        ("screenshot,s", "screenshot when one wrm file is create")
        ("1", "")
        ;
    }

    void parse_command_line(int argc, char** argv)
    {
        po::positional_options_description p;
        p.add("input-file", -1);
        po::store(
            po::command_line_parser(argc, argv).options(
                this->desc
            ).positional(p).run(),
            this->options
        );
    }

    uint notify_options()
    {
        po::notify(this->options);

        /*if (this->options.count("synchronise")){
            this->synchronise = true;
        }*/

        if (!this->range.valid()){
            std::swap<>(this->range.left, this->range.right);
        }

        po::variables_map::iterator it = this->options.find("screenshot");
        po::variables_map::iterator end = this->options.end();
        if (it != end)
            this->synchronise_screen = true;

        return RecorderError::SUCCESS;
    }

    const char * version() const
    {
        return "0.1";
    }
};

uint app_parse_command_line(int argc, char** argv, WrmRecoderOption& app)
{
    app.parse_command_line(argc, argv);

    if (app.options.count("version")) {
        std::cout << "wrm-recorder version " << app.version() << '\n';
    }

    if (app.options.count("help")) {
        return RecorderError::HELP;
    }
    return app.notify_options();
}


struct OutputType {
    enum enum_t {
        NOT_FOUND,
        PNG_TYPE,
        WRM_TYPE
    };

    static OutputType::enum_t get_output_type(const std::string& filename)
    {
        std::size_t p = filename.find_last_of('.');
        return (p == std::string::npos || p + 4 != filename.length())
        ? NOT_FOUND
        : (
            !filename.compare(p+1, 3, "png")
            ? PNG_TYPE
            : (
                !filename.compare(p+1, 3, "wrm")
                ? WRM_TYPE
                : NOT_FOUND
            )
        );
    }
};


class WrmRecorderApp
{
    WrmRecoderOption& opt;

public:
    WrmRecorderApp(WrmRecoderOption& opt)
    : opt(opt)
    {
    }

    OutputType::enum_t get_output_type() const
    {
        return OutputType::get_output_type(this->opt.out_filename);
    }

    std::string get_out_filename() const
    {
        return opt.out_filename.erase(opt.out_filename.length() - 4);
    }

    void run()
    {
        this->run(this->get_out_filename().c_str(), this->get_output_type());
    }

    void run(OutputType::enum_t type)
    {
        this->run(this->get_out_filename().c_str(), type);
    }

    void run(const char* outfile)
    {
        this->run(outfile, this->get_output_type());
    }

    void run(const char* outfile, OutputType::enum_t type)
    {
        WRMRecorder recorder(opt.wrm_in_filename.c_str());
        switch (type) {
            case OutputType::PNG_TYPE:
                this->png_run(recorder, outfile);
                break;
            default:
                this->wrm_run(recorder, outfile);
                break;
        }
    }

private:
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
            //std::cout << "m: " << m;
            this->micro_sec += this->chunk_time_value / (100000000000000l);
            //std::cout << ", micro_sec: " << this->micro_sec << '\n';
            --this->recorder.remaining_order_count();
        }

        bool interpret_is_time_chunk()
        {
            if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
            {
                this->interpret_time();
                return true;
            }
            return false;
        }

        uint64_t start()
        {
            uint64_t time_start = 0;
            if (this->recorder.selected_next_order())
            {
                if (recorder.chunk_type() == WRMChunk::TIMESTAMP)
                {
                    time_start = this->recorder.reader.stream.in_uint64_be();
                }
            }
            else
            {
                recorder.interpret_order();
            }
            return time_start;
        }

        uint64_t advance_usecond(uint msec)
        {
            while (this->recorder.selected_next_order())
            {
                if (this->interpret_is_time_chunk() && this->micro_sec){
                    if (this->micro_sec >= msec) {
                        uint64_t tmp = this->micro_sec;
                        this->reset();
                        return tmp;
                    }
                }
                else {
                    recorder.interpret_order();
                }
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
            recorder.meta.width,
            recorder.meta.height,
            outfile,
            0, 0
        );
        recorder.consumer(&capture);
        TimerCompute timercompute(recorder);
        if (!timercompute.advance_second(this->opt.range.left))
            return 0;

        uint frame = 0;
        uint64_t mtime = TimerCompute::coeff_sec_to_usec * this->opt.time;
        uint64_t msecond = TimerCompute::coeff_sec_to_usec * (this->opt.range.right.time - this->opt.range.left.time);
        while (recorder.selected_next_order() && frame != this->opt.frame)
        {
            if (timercompute.interpret_is_time_chunk()){
                if (timercompute.usec() >= mtime){
                    capture.dump_png();
                    timercompute.reset();
                    ++frame;
                }
                if (msecond <= timercompute.usec()){
                    msecond = 0;
                    break;
                } else {
                    msecond -= timercompute.usec();
                }
            } else {
                recorder.interpret_order();
            }
        }
        if (msecond && frame != this->opt.frame){
            capture.dump_png();
            ++frame;
        }
        return frame;
    }

    uint wrm_run(WRMRecorder& recorder, const char* outfile)
    {
        Capture capture(
            recorder.meta.width,
            recorder.meta.height,
            outfile,
            0, 0
        );
        recorder.consumer(&capture);
        TimerCompute timercompute(recorder);
        uint64_t msecond = timercompute.start();
        std::cout << "start: " << msecond << '\n';
        uint64_t mtime = timercompute.advance_second(this->opt.range.left);
        std::cout << "mtime: " << mtime << '\n';
        if (!mtime)
            return 0;
        if (msecond)
            capture.timestamp(msecond);
        timercompute.usec() = mtime - this->opt.range.left;

        uint frame = 0;
        mtime = TimerCompute::coeff_sec_to_usec * this->opt.time;
        msecond = TimerCompute::coeff_sec_to_usec * (this->opt.range.right.time - this->opt.range.left.time);
        while (recorder.selected_next_order() && frame != this->opt.frame)
        {
            if (timercompute.interpret_is_time_chunk()){
                //std::cout << "timercompute.usec(): " << timercompute.usec()  << ' ' << "mtime: " << mtime << '\n';
                uint64_t usec = timercompute.usec();
                if (timercompute.chunk_time_value)
                    capture.timestamp(timercompute.chunk_time_value);
                if (usec >= mtime){
                    std::cout << "usec: " << usec << ", mtime: " << mtime << std::endl;
                    capture.breakpoint();
                    if (this->opt.synchronise_screen)
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
        std::cout << "msecond: " << msecond << '\n';
        return frame;
    }
};

int main(int argc, char** argv)
{
    WrmRecoderOption opt;
    opt.add_default_options();

    uint error = app_parse_command_line(argc, argv, opt);
    if (error){
        std::cout << "error (" << error << "): " << recorder_string_errors[error] << '\n'
        << opt.desc;
        return error;
    }

    if (!opt.out_filename.empty()){
        std::cout << "output-file: " << opt.out_filename << '\n';
    }
    if (!opt.wrm_in_filename.empty()){
        std::cout << "input-file: " << opt.wrm_in_filename << '\n';
    }

    std::cout
    << "frame: " << opt.frame << '\n'
    << "time: " << opt.time << '\n'
    << "range: " << opt.range << '\n'
    ;

    if (opt.out_filename.empty()){
        std::cerr << "not output-file\n";
        return 80;
    }
    if (opt.wrm_in_filename.empty()){
        std::cerr << "not input-file\n";
        return 90;
    }

    OutputType::enum_t type = OutputType::get_output_type(opt.out_filename);
    if (type == OutputType::NOT_FOUND){
        std::cerr << "incorect output extension, accept png or wrm.\n";
        return 100;
    }

    std::cout << "type: " << type << '\n';

    WrmRecorderApp app(opt);
    app.run();

    return 0;
}