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

#if !defined(__MAIN_RECORDER_RECORDER_RUN__)
#define __MAIN_RECORDER_RECORDER_RUN__

#include <iostream>

#include <boost/program_options/options_description.hpp>

#include "adaptator.hpp"
#include "recorder_option.hpp"
#include "wrm_recorder_init.hpp"

int recorder_run(RecorderOption& opt,
                 RecorderAction* actions, std::size_t n,
                 InputType::enum_t itype)
{
    const std::size_t pos = opt.out_filename.find_last_of('.');
    std::string extension = opt.output_type.empty() ? opt.out_filename.substr(pos + 1) : opt.output_type;

    RecorderAdaptator* adaptator = get_recorder_adaptator(actions, actions + n,
                                                          extension);
    if (!adaptator){
        std::cerr
        << "Incorrect output-type, "
        << opt.desc.find("output-type", false).description() << '\n';
        return 1100;
    }

    WRMRecorder recorder;
    if (int error = wrm_recorder_init(recorder, opt, itype)){
        return error;
    }

    try
    {
        opt.out_filename.erase(pos);
        (*adaptator)(recorder, opt.out_filename.c_str());
    }
    catch (Error& error)
    {
        std::cerr << "Error " << error.id << ": " << strerror(error.errnum) << std::endl;
        return 100000 + error.errnum;
    }
    return 0;
}

#endif