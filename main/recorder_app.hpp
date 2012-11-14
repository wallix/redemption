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

#if !defined(__MAIN_RECORDER_RECORDER_APP__)
#define __MAIN_RECORDER_RECORDER_APP__

#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <utility>
#include <string>

#include "FileToGraphic.hpp"
#include "wrm_recorder_option.hpp"
#include "wrm_recorder.hpp"

struct RecorderAdapter
{
    virtual void operator()(WRMRecorder& recorder, const char* outfile) = 0;
};

struct RecorderAction {
    const char * key;
    RecorderAdapter * action;

    RecorderAction(const char * key, RecorderAdapter* action)
    : key(key)
    , action(action) 
    {
    }
};

int recorder_app(WrmRecorderOption& opt, int argc, char** argv, RecorderAction* actions, std::size_t n)
{
//    char buffer[128] = {};
//    size_t used = snprintf(buffer, 128, "accept ");
//    for (unsigned int i = 0 ; i < n ; i++){
//        used += snprintf(&buffer[used], 128-used, "%s'%s'", (i>0?(i!=n-1)?", ":" or ":""), actions[i].key);
//    }
//    opt.desc.add_options()("output-type,O", po::value(&opt.output_type), buffer);

    openlog("redrec", LOG_CONS | LOG_PERROR, LOG_USER);

    std::string input_filename;
    std::string output_filename;
    uint32_t begin_cap = 0;
    uint32_t end_cap = 0;

    boost::program_options::options_description desc("Options");
    desc.add_options()
    ("help,h", "produce help message")
    ("version,v", "show software version")
    ("output-file,o", boost::program_options::value(&output_filename), "output base filename (see --output-type)")
    ("input-file,i", boost::program_options::value(&input_filename), "input base filename (see --input-type)")
//    ("in", boost::program_options::value(&input_filename), "alias for --input-file")
//    ("out", boost::program_options::value(&output_filename), "alias for --output-file")
    ("begin,b", boost::program_options::value(&begin_cap), "begin capture")
    ("end,e", boost::program_options::value(&end_cap), "end capture")
    ;

    boost::program_options::variables_map options;
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(desc)
//            .positional(p)
            .run(),
        options
    );
    boost::program_options::notify(options);

    if (input_filename.c_str()[0] == 0){
        cout << desc << endl;
        printf("Missing input filename\n");
        exit(-1);
    }

    if (output_filename.c_str()[0] == 0){
        printf("Missing output filename\n");
        cout << desc << endl;
        exit(-1);
    }

    timeval begin_capture;
    begin_capture.tv_sec = begin_cap; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = end_cap; end_capture.tv_usec = 0;
    
//    InByFilenameTransport in_wrm_trans(input_filename.c_str());
    InByMetaSequenceTransport in_wrm_trans(input_filename.c_str());
    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false);

    Inifile ini;
    ini.globals.debug.primary_orders = 0;
    ini.globals.debug.secondary_orders = 0;
    ini.globals.png_limit = 10;
    ini.globals.png_interval = 10;
    ini.globals.frame_interval = 10;
    ini.globals.break_interval = 10000;

    char path[1024];
    char basename[1024];
    strcpy(path, "./"); // default value, actual one should come from output_filename
    strcpy(basename, "redemption"); // default value actual one should come from output_filename
    const char * end_of_path = strrchr(output_filename.c_str(), '/') + 1;
    if (end_of_path){
        memcpy(path, ini.globals.movie_path, end_of_path - output_filename.c_str());
        path[end_of_path - output_filename.c_str()] = 0;
        const char * start_of_extension = strrchr(end_of_path, '.');
        if (start_of_extension){
            memcpy(basename, end_of_path, start_of_extension - end_of_path);
            basename[start_of_extension - end_of_path] = 0;
        }
        else {
            if (end_of_path[0]){
                strcpy(basename, end_of_path);
            }
        }
    }

    Capture capture(player.record_now, player.screen_rect.cx, player.screen_rect.cy, path, basename, ini);
    player.add_consumer(capture.pnc);
//    player.add_consumer(capture.psc);

    player.play();

    return 0;
}

class ToPngAdapter
: public RecorderAdapter
{
    WrmRecorderOption& _option;

public:
    ToPngAdapter(WrmRecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        printf("to png adapter -> %s width=%u height=%u resize_width=%u resize_height=%u\n", 
            outfile, 800, 600, 
            this->_option.png_scale_width, this->_option.png_scale_height);

        recorder.to_png(outfile,
               this->_option.range.left.time,
               this->_option.range.right.time,
               this->_option.time.time,
               this->_option.png_scale_width,
               this->_option.png_scale_height,
               this->_option.frame,
               this->_option.screenshot_start,
               this->_option.no_screenshot_stop,
               this->_option.screenshot_all
        );
    }
};

class ToPngListAdapter : public RecorderAdapter
{
    WrmRecorderOption& _option;

public:
    ToPngListAdapter(WrmRecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        recorder.to_png_list(outfile,
               this->_option.time_list,
               this->_option.png_scale_width,
               this->_option.png_scale_height,
               this->_option.no_screenshot_stop
        );
    }
};

class ToWrmAdapter : public RecorderAdapter
{
    WrmRecorderOption& _option;

public:
    ToWrmAdapter(WrmRecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        const char * metaname = this->_option.metaname.empty() ? 0 : this->_option.metaname.c_str();

        const unsigned char * key = 0;
        const unsigned char * iv = 0;
        if (this->_option.out_crypt_mode)
        {
            if (this->_option.out_crypt_key.size)
                key = this->_option.out_crypt_key.data;
            if (this->_option.out_crypt_iv.size)
                iv = this->_option.out_crypt_iv.data;
        }

        if (this->_option.cat_wrm) {
            recorder.to_one_wrm(outfile,
                       this->_option.range.left.time,
                       this->_option.range.right.time,
                       metaname,
                       this->_option.out_crypt_mode,
                       key, iv
                      );
        }
        else {
            recorder.to_wrm(outfile,
                   this->_option.range.left.time,
                   this->_option.range.right.time,
                   this->_option.time.time,
                   this->_option.frame,
                   this->_option.screenshot_start,
                   this->_option.screenshot_wrm,
                   metaname,
                   this->_option.out_crypt_mode,
                   key, iv
                  );
        }
    }
};

#endif
