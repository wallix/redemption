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

#include "parse_command_line.hpp"
#include "adapter.hpp"
#include "recorder_option.hpp"
#include "wrm_recorder_init.hpp"

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

#endif
