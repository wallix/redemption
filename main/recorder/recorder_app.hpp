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

#include "wrm_recorder_option.hpp"
#include <utility>
#include <string>

#include "wrm_recorder.hpp"
#include "recorder/to_png.hpp"
#include "recorder/to_wrm.hpp"
#include "recorder/to_one_wrm.hpp"

struct RecorderAdapter
{
    virtual void operator()(WRMRecorder& recorder, const char* outfile) = 0;
};

typedef std::pair<std::string, RecorderAdapter*> RecorderAction;

RecorderAdapter* get_recorder_adapter(RecorderAction* first,
                                          RecorderAction* last,
                                          const std::string& extension)
{
    for (; first != last; ++first)
    {
        if (first->first == extension)
            return first->second;
    }
    return 0;
}

struct RecorderActionStringIterator
{
private:
    RecorderAction* _base;

public:
    RecorderActionStringIterator(RecorderAction* it)
    : _base(it)
    {}

    RecorderActionStringIterator& operator++()
    {
        ++this->_base;
        return *this;
    }

    const std::string& operator*() const
    {
        return this->_base->first;
    }

    bool operator==(const RecorderActionStringIterator& other)
    {
        return this->_base == other._base;
    }

    bool operator!=(const RecorderActionStringIterator& other)
    {
        return !(*this == other);
    }
};

bool parse_command_line(WrmRecorderOption& opt,
                       int argc, char** argv)
{
    opt.parse_command_line(argc, argv);

    if (opt.options.count("version")) {
        std::cout << argv[0] << ' ' << opt.version() << std::endl;
        return false;
    }

    if (opt.options.count("help")) {
        std::cout << opt.desc;
        return false;
    }

    return true;
}

int recorder_run(RecorderOption& opt,
                 RecorderAction* actions, std::size_t n,
                 InputType::enum_t itype)
{
    const std::size_t pos = opt.out_filename.find_last_of('.');
    std::string extension = opt.output_type.empty()
    ? (std::string::npos == pos ? "" : opt.out_filename.substr(pos + 1))
    : opt.output_type;

    RecorderAdapter* adapter = get_recorder_adapter(actions, actions + n,
                                                    extension);
    if (!adapter){
        std::cerr
        << "Incorrect output-type, "
        << opt.desc.find("output-type", false).description() << std::endl;
        return 1100;
    }

    WRMRecorder recorder;
    if (int error = wrm_recorder_init(recorder, opt, itype)){
        return error;
    }

    if (std::string::npos != pos)
        opt.out_filename.erase(pos);
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


int recorder_app(RecorderOption& opt, int argc, char** argv,
                 RecorderAction* actions, std::size_t n)
{
    opt.accept_output_type<>(RecorderActionStringIterator(actions),
                             RecorderActionStringIterator(actions + n));

    try
    {
        if (!parse_command_line(opt, argc, argv)){
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
            case RecorderOption::OUTPUT_KEY_OVERLOAD:
                std::cerr << "Size key is " << opt.out_cipher_info.key_len() << " octets" << std::endl;
                break;
            case RecorderOption::OUTPUT_IV_OVERLOAD:
                std::cerr << "Size IV is " << opt.out_cipher_info.iv_len() << " octets" << std::endl;
                break;
            default:
                break;
        }
        return error;
    }

    return recorder_run(opt, actions, n, itype);
}

class ToPngAdapter
: public RecorderAdapter
{
    RecorderOption& _option;

public:
    ToPngAdapter(RecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        to_png(recorder, outfile,
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

class ToPngListAdapter
: public RecorderAdapter
{
    RecorderOption& _option;

public:
    ToPngListAdapter(RecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        to_png_2(recorder, outfile,
               this->_option.time_list,
               this->_option.png_scale_width,
               this->_option.png_scale_height,
               this->_option.no_screenshot_stop
        );
    }
};

class ToWrmAdapter
: public RecorderAdapter
{
    RecorderOption& _option;

public:
    ToWrmAdapter(RecorderOption& option)
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
            to_one_wrm(recorder, outfile,
                       this->_option.range.left.time,
                       this->_option.range.right.time,
                       metaname,
                       this->_option.out_crypt_mode,
                       key, iv
                      );
        }
        if (!this->_option.cat_wrm) {
            to_wrm(recorder, outfile,
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
