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
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 *
 * recorder main program
 *
 */

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#define LOGPRINT

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
    std::string in_filename;
    std::string idx_start;
    std::string base_path;
    std::string metaname;
    bool screenshot_wrm;
    bool screenshot_start;
    bool no_screenshot_stop;
    bool ignore_dir_for_meta_in_wrm;
    std::string output_type;
    std::string input_type;

    WrmRecoderOption()
    : desc("Options")
    , options()
    , range()
    , frame(std::numeric_limits<uint>::max())
    , time(60*2)
    , out_filename()
    , in_filename()
    , idx_start("0")
    , base_path()
    , screenshot_wrm(false)
    , screenshot_start(false)
    , no_screenshot_stop(false)
    , ignore_dir_for_meta_in_wrm(false)
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
        "interval of capture"
        "\n\nformat:"
        "\n[[[+|-]time[h|m|s][...]][,][[+]time[h|m|s][[+|-]time[h|m|s][...]]]]"
        "\n\nexamples:"
        "\n1h30,+10m -> from 1h30 to 1h40"
        "\n20m+2h-50s,3h -> from 2h19m10s to 3h")
        ("frame,f", po::value(&this->frame), "maximum number of frames in the interval")
        ("time,t", po::value(&this->time), "duration between each capture"
        "\nformat: [+|-]time[h|m|s][...]")
        ("input-file,i", po::value(&this->in_filename), "wrm filename")
        ("output-file,o", po::value(&this->out_filename), "png or wrm filename")
        ("index-start,x", po::value(&this->idx_start), "index file in the meta")
        ("screenshot-wrm,s", "capture the screen when a file wrm is create")
        ("screenshot-start,0", "")
        ("no-screenshot-stop,N", "")
        ("path,p", po::value(&this->base_path), "base path for the files presents in the meta")
        ("ignore-dir,n", "ignore directory for meta in the file wrm")
        ("deduce-dir,d", "use --ignore-dir and set --path with the directory of --input-file")
        ("output-meta-name,m", po::value(&this->metaname), "specified name of meta file")
        ("input-type", po::value(&this->input_type), "accept 'mwrm' or 'wrm'")
        ("output-type", po::value(&this->output_type), "accept 'png' or 'wrm'")
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

        if (!this->range.valid()){
            std::swap<>(this->range.left, this->range.right);
        }

        po::variables_map::iterator end = this->options.end();

        {
            typedef std::pair<const char *, bool&> pair_type;
            pair_type p[] = {
                pair_type("screenshot-wrm", this->screenshot_wrm),
                pair_type("screenshot-start", this->screenshot_start),
                pair_type("no-screenshot-stop", this->no_screenshot_stop),
                pair_type("ignore-dir", this->ignore_dir_for_meta_in_wrm),
            };
            for (std::size_t n = 0; n < sizeof(p)/sizeof(p[0]); ++n) {
                if (this->options.find(p[n].first) != end)
                    p[n].second = true;
            }
        }

        if (this->options.find("deduce-dir") != end)
        {
            this->ignore_dir_for_meta_in_wrm = true;
            std::size_t pos = this->in_filename.find_last_of('/');
            if (std::string::npos != pos)
                this->base_path = this->in_filename.substr(0, pos+1);
        }

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
        std::cout << argv[0] << ' ' << app.version() << '\n';
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

    static OutputType::enum_t string_type_to_enum(const std::string& filename)
    {
        if (filename == "png")
            return PNG_TYPE;
        if (filename == "wrm")
            return WRM_TYPE;
        return NOT_FOUND;
    }
};


struct InputType {
    enum enum_t {
        NOT_FOUND,
        META_TYPE,
        WRM_TYPE
    };

    static InputType::enum_t get_input_type(const std::string& filename)
    {
        std::size_t p = filename.find_last_of('.');
        if (p == std::string::npos)
            return NOT_FOUND;
        ++p;
        return (p + 4 == filename.length()
        && !filename.compare(p, 4, "mwrm"))
        ? META_TYPE
        : (
            (p + 3 == filename.length() && !filename.compare(p, 3, "wrm"))
            ? WRM_TYPE
            : NOT_FOUND
        );
    }

    static InputType::enum_t string_type_to_enum(const std::string& filename)
    {
        if (filename == "mwrm")
            return META_TYPE;
        if (filename == "wrm")
            return WRM_TYPE;
        return NOT_FOUND;
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
        if (!this->opt.output_type.empty()){
            return OutputType::string_type_to_enum(this->opt.output_type);
        }
        return OutputType::get_output_type(this->opt.out_filename);
    }

    InputType::enum_t get_input_type() const
    {
        if (!this->opt.input_type.empty()){
            return InputType::string_type_to_enum(this->opt.input_type);
        }
        return InputType::get_input_type(this->opt.in_filename);
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

    void run()
    {
        this->run(this->get_out_filename().c_str());
    }

    void run(OutputType::enum_t otype)
    {
        this->run(otype, this->get_input_type());
    }

    void run(InputType::enum_t itype)
    {
        this->run(this->get_output_type(), itype);
    }

    void run(OutputType::enum_t otype, InputType::enum_t itype)
    {
        this->run(this->get_out_filename().c_str(), otype, itype);
    }

    void run(const char* outfile, OutputType::enum_t otype)
    {
        this->run(outfile, otype, this->get_input_type());
    }

    void run(const char* outfile, InputType::enum_t itype)
    {
        this->run(outfile, this->get_output_type(), itype);
    }

    void run(const char* outfile)
    {
        this->run(outfile, this->get_output_type());
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
        TimerCapture::time_type tmptime;

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

                if (recorder.chunk_type() == WRMChunk::NEXT_FILE) {
                    tmptime = capture.timer().impl();
                    recorder.interpret_order();
                    capture.timer().impl() = tmptime;
                }
                else {
                    recorder.interpret_order();
                }
            }
        }

        /*if (chunk_time) {
            capture.timestamp(chunk_time);
        }*/

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
    if (!opt.in_filename.empty()){
        std::cout << "input-file: " << opt.in_filename << '\n';
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
    if (opt.in_filename.empty()){
        std::cerr << "not input-file\n";
        return 90;
    }

    WrmRecorderApp app(opt);

    OutputType::enum_t otype = app.get_output_type();
    if (otype == OutputType::NOT_FOUND){
        std::cerr << "incorrect output-type: "
        << opt.desc.find("output-type", false).description() << '\n';
        return 100;
    }

    InputType::enum_t itype = app.get_input_type();
    if (itype == InputType::NOT_FOUND){
        std::cerr << "incorrect input-type: "
        << opt.desc.find("input-type", false).description() << '\n';
        return 110;
    }

    try {
        //app.run();
        app.run(otype, itype);
    } catch (Error e) {
        std::cerr
        << "id: " << e.id
        << ", errnum: " << e.errnum
        << ", strerror: " << strerror(e.errnum)
        << std::endl;
    }

    return 0;
}