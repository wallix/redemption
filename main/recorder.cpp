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

#include <boost/program_options/options_description.hpp>
#include <utility>
#include <string>

#define LOGPRINT
#include "version.hpp"

#include "capture.hpp"
#include "FileToGraphic.hpp"


int main(int argc, char** argv)
{
    openlog("redrec", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
    "\n"
    "ReDemPtion RECorder" VERSION ": An RDP movie converter.\n"
    "Copyright (C) Wallix 2010-2012.\n"
    "Christophe Grosjean, Jonathan Poelen\n"
    "\n"
    ;

    std::string input_filename;
    std::string output_filename;

    uint32_t verbose = 0;
    uint32_t clear = 1; // default on
    uint32_t begin_cap = 0;
    uint32_t end_cap = 0;
    uint32_t png_limit = 10;
    uint32_t png_interval = 60;
    uint32_t wrm_frame_interval = 100;
    uint32_t wrm_break_interval = 86400;
    uint32_t order_count = 0;
    unsigned zoom = 100;

    boost::program_options::options_description desc("Options");
    desc.add_options()
    ("help,h", "produce help message")
    ("version,v", "show software version")
    ("output-file,o", boost::program_options::value(&output_filename), "output base filename (see --output-type)")
    ("input-file,i", boost::program_options::value(&input_filename), "input base filename (see --input-type)")

    ("begin,b", boost::program_options::value<uint32_t>(&begin_cap), "begin capture time (in seconds), default=none")
    ("end,e", boost::program_options::value<uint32_t>(&end_cap), "end capture time (in seconds), default=none")
    ("count,c", boost::program_options::value<uint32_t>(&order_count), "Number of orders to execute before stopping, default=0 execute all orders")

    ("png_limit,l", boost::program_options::value<uint32_t>(&png_limit), "maximum number of png files to create (remove older), default=10, 0 will disable png capture")
    ("png_interval,n", boost::program_options::value<uint32_t>(&png_interval), "time interval between png captures, default=60 seconds")

    ("frameinterval,r", boost::program_options::value<uint32_t>(&wrm_frame_interval), "time between consecutive capture frames (in 100/th of seconds), default=100 one frame per second")

    ("breakinterval,k", boost::program_options::value<uint32_t>(&wrm_break_interval), "number of seconds between splitting wrm files in seconds(default, one wrm every day)")

    ("png,p", "enable png capture")
    ("wrm,w", "enable wrm capture")
    ("clear", boost::program_options::value<uint32_t>(&clear), "Clear old capture files with same prefix (default on)")
    ("verbose", boost::program_options::value<uint32_t>(&verbose), "more logs")
    ("zoom", boost::program_options::value<uint32_t>(&zoom), "scaling factor for png capture (default 100%)")
    ;

    Inifile ini;

    try {
        boost::program_options::variables_map options;
        boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv).options(desc)
    //            .positional(p)
                .run(),
            options
        );
        boost::program_options::notify(options);

        if (input_filename.c_str()[0] == 0){
            cout << "Missing input filename\n\n";
            cout << copyright_notice;
            cout << "Usage: redrec [options]\n\n";
            cout << desc << endl;
            exit(-1);
        }

        if (output_filename.c_str()[0] == 0){
            cout << "Missing output filename\n\n";
            cout << copyright_notice;
            cout << "Usage: redrec [options]\n\n";
            cout << desc << endl;
            exit(-1);
        }

        ini.globals.png_limit = png_limit;
        ini.globals.png_interval = png_interval;
        ini.globals.frame_interval = wrm_frame_interval;
        ini.globals.break_interval = wrm_break_interval;
        ini.globals.capture_wrm = options.count("wrm") > 0;
        ini.globals.capture_png = (options.count("png") > 0);

    }
    catch(boost::program_options::error& error) {
        cout << error.what() << endl;
        cout << copyright_notice;
        cout << "Usage: redrec [options]\n\n";
        cout << desc << endl;
        exit(-1);
    };

    timeval begin_capture;
    begin_capture.tv_sec = begin_cap; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = end_cap; end_capture.tv_usec = 0;

    TODO("before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases")

    TODO("also check if it contains any wrm at all and at wich one we should start depending on input time")
    TODO("if start and stop time are outside wrm, users should also be warned")

    InByMetaSequenceTransport in_wrm_trans(input_filename.c_str());
//    InByFilenameTransport in_wrm_trans(input_filename.c_str());

    try {
        in_wrm_trans.next_chunk_info();
        TODO("a negative time should be a time relative to end of movie")
        REDOC("less than 1 year means we are given a time relatve to beginning of movie")
        if (begin_cap < 31536000){ // less than 1 year, it is relative not absolute timestamp
            // begin_capture.tv_usec is 0
            begin_cap += in_wrm_trans.begin_chunk_time;
        }
        if (end_cap < 31536000){ // less than 1 year, it is relative not absolute timestamp
            // begin_capture.tv_usec is 0
            end_cap += in_wrm_trans.begin_chunk_time;
        }
        while (begin_cap >= in_wrm_trans.end_chunk_time){
            in_wrm_trans.next_chunk_info();    
        }
        unsigned count = in_wrm_trans.chunk_num-1;
        in_wrm_trans.reset_meta();
        for (; count > 0 ; count--){
            in_wrm_trans.next_chunk_info();
        }
    }
    catch (const Error & e) {
        if (e.id == (unsigned)ERR_TRANSPORT_READ_FAILED){
            printf("Asked time not found in mwrm file\n");
        };
        exit(-1);
    };

    
    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false, verbose);
    player.max_order_count = order_count;

    const char * fullpath = output_filename.c_str();
    char path[1024];
    char basename[1024];
    strcpy(path, "./"); // default value, actual one should come from output_filename
    strcpy(basename, "redemption"); // default value actual one should come from output_filename
    canonical_path(fullpath, path, sizeof(path), basename, sizeof(basename));

    if (clear == 1) {
        clear_files_flv_meta_png(path, basename);
    }

    Capture capture(player.record_now, player.screen_rect.cx, player.screen_rect.cy, path, basename, ini);
    if (capture.capture_png){
        capture.psc->zoom(zoom);
    }
    player.add_consumer(&capture);

    player.play();
    
    return 0;
}
