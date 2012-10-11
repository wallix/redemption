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
    timeval now;
    gettimeofday(&now, NULL);

    char buffer[128] = {};
    size_t used = snprintf(buffer, 128, "accept ");
    for (unsigned int i = 0 ; i < n ; i++){
        used += snprintf(&buffer[used], 128-used, "%s'%s'", (i>0?(i!=n-1)?", ":" or ":""), actions[i].key);
    }
    opt.desc.add_options()("output-type,O", po::value(&opt.output_type), buffer);

    try
    {
        if (!opt.parse_command_line(argc, argv)){
            return 0;
        }
    }
    catch(boost::program_options::error& error)
    {
        std::cerr << "parse command line: " << error.what() << std::endl;
        return -1;
    }

    InputType::enum_t itype;
    if (int error = opt.prepare(itype)){
        switch (error) {
            case WrmRecorderOption::INPUT_KEY_OVERLOAD:
                std::cerr << "Size key is " << opt.in_cipher_info.key_len() << " octets" << std::endl;
                break;
            case WrmRecorderOption::INPUT_IV_OVERLOAD:
                std::cerr << "Size IV is " << opt.in_cipher_info.iv_len() << " octets" << std::endl;
                break;
            case WrmRecorderOption::OUTPUT_KEY_OVERLOAD:
                std::cerr << "Size key is " << opt.out_cipher_info.key_len() << " octets" << std::endl;
                break;
            case WrmRecorderOption::OUTPUT_IV_OVERLOAD:
                std::cerr << "Size IV is " << opt.out_cipher_info.iv_len() << " octets" << std::endl;
                break;
            default:
                break;
        }
        return error;
    }

    printf("running recorder\n");
    const std::size_t pos = opt.out_filename.find_last_of('.');
    std::string extension = opt.output_type.empty()
    ? (std::string::npos == pos ? "" : opt.out_filename.substr(pos + 1))
    : opt.output_type;

    unsigned int i = 0;
    for (i = 0 ; i < n ; i++){
        if (0 == strcasecmp(actions[i].key, extension.c_str())){
            break;
        }
    }
    if (i >= n){
        std::cerr
        << "Incorrect output-type, "
        << opt.desc.find("output-type", false).description() << std::endl;
        return 1100;
    }

    RecorderAdapter* adapter = actions[i].action;

    printf("base_path=%s in_filename=%s\n", opt.base_path.c_str(), opt.in_filename.c_str());
    exit(0);

    WRMRecorder recorder(now,
                        opt.in_crypt_mode,
                        opt.in_crypt_key, 
                        opt.in_crypt_iv,
                        itype, 
                        opt.base_path, 
                        opt.ignore_dir_for_meta_in_wrm,
                        opt.times_in_meta_are_false,
                        opt.force_interpret_breakpoint,
                        opt.range,
                        opt.in_filename,
                        opt.idx_start);

    if (std::string::npos != pos){
        opt.out_filename.erase(pos);
    }

    try
    {
        (*adapter)(recorder, opt.out_filename.c_str());
    }
    catch (Error& error)
    {
        std::cerr << "Error " << error.id;
        if (error.errnum)
            std::cerr << ": " << strerror(error.errnum);
        std::cerr << std::endl;
        return 100000 + error.errnum;
    }
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
