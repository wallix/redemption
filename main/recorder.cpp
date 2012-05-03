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

#include "range_time_point.hpp"
// #include "wrm_recorder.hpp"

class NativeCapture;
class StaticCapture;

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

bool load_option(int argc, char** argv, po::variables_map options,
                 range_time_point range,
                 uint frame, time_point time,
                 std::string wrm_filename,
                 std::string png_filename,
                 std::vector<std::string> wrm_files,
                 bool synchronise
                )
{
    po::options_description desc("Options");
    desc.add_options()
    // --help, -h
    ("help,h", "produce help message")
    // --version, -v
    ("version,v", "show software version")
    ("png,p", po::value(&png_filename), "wrm to png")
    ("wrm,w", po::value(&wrm_filename), "wrm to wrm (cut file)")
    ("range,r", po::value(&range),
     "range capture\n"
     "range format:\n"
     "[[[+|-]time[h|m|s][...]][,][[+]time[h|m|s][[+|-]time[h|m|s][...]]]]\n"
     "example:\n"
     "1h30,+10m -> from 1h30 to 1h40\n"
     "20m+2h-50s,3h -> from 2h19m10s to 3h"
    )
    ("frame,f", po::value(&frame), "maximum frame for png capture option")
    ("time,t", po::value(&time), "time capture for 1 file, with wrm capture option. format: [+|-]time[h|m|s][...]")
    ("synchronise,s", "")
    ("input-file,i", po::value(wrm_files), "")
    ("output-file,o", po::value<std::string>(), "alias -w ... -p ... -s")
    ("1", "")
    ;

    po::positional_options_description p;
    p.add("input-file", -1);
    po::store(
        po::command_line_parser(argc, argv).
        options(desc).positional(p).run(), options
    );
    po::notify(options);

    if (options.count("help")) {
        std::cout << desc;
        return 0;
    }

    po::variables_map::iterator it = options.find("output-file");
    bool png_option = options.count("png");
    bool wrm_option = options.count("wrm");
    if (it != options.end()){
        std::string& filename = it->second.as<std::string>();
        if (!png_option){
            png_filename = filename;
        }
        if (!wrm_option){
            wrm_filename = filename;
        }
    }

    ///TODO right ?
    synchronise = png_option && wrm_option && options.count("synchronise");


    return 1;
}

int main(int argc, char** argv)
{
    range_time_point range;
    uint frame = std::numeric_limits<uint>::max();
    time_point time(3600);
    std::string wrm_filename;
    std::string png_filename;
    std::vector<std::string> wrm_files;
    bool synchronise = false;

    po::variables_map options;
    if (!load_option(argc, argv, options,
        range, frame, time,
        wrm_filename, png_filename,
        wrm_files,
        synchronise))
        return 1;

    po::variables_map::iterator it = options.find("wrm");
    if (it != options.end()){
        std::cout << "wrm: " << it->second.as<std::string>() << '\n';
    }
    it = options.find("png");
    if (it != options.end()){
        std::cout << "png: " << it->second.as<std::string>() << '\n';
    }
    it = options.find("input-file");
    if (it != options.end()){
        std::vector<std::string>& v = it->second.as<std::vector<std::string> >();
        for (std::size_t i = 0, last = v.size(); i != last; ++i)
            std::cout << "input-file (" << i << "): " << v[i] << '\n';
    }
    std::cout
    << "frame: " << frame << '\n'
    << "time: " << time << '\n'
    << "range: " << range << '\n'
    ;
    if (!range.valid()){
        std::swap<>(range.left, range.right);
        std::cout << "revalide range: " << range << '\n';
    }


    //     NativeCapture *native_capture = 0;
    //     StaticCapture *static_capture = 0;
    //WRMRecorder recorder(filename);



    return 0;
}