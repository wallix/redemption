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

#if !defined(__MAIN_RECORDER_WRM_RECORDER_INIT_HPP__)
#define __MAIN_RECORDER_WRM_RECORDER_INIT_HPP__

#include <iostream>

#include <error.hpp>
#include "wrm_recorder.hpp"
#include "wrm_recorder_option.hpp"

int wrm_recorder_init(WRMRecorder& recorder, WrmRecorderOption& opt, InputType::enum_t itype)
{
    recorder.set_basepath(opt.base_path);
    recorder.only_filename = opt.ignore_dir_for_meta_in_wrm;

    try
    {
        switch (itype) {
            case InputType::WRM_TYPE:
                recorder.open_wrm_followed_meta(opt.in_filename.c_str());
                break;
            case InputType::META_TYPE:
                recorder.open_meta_followed_wrm(opt.in_filename.c_str());
                break;
            default:
                std::cerr << "Input type not found" << std::endl;
                return 2000;
        }

        if (!recorder.is_meta_chunk()){
            std::cerr << recorder.chunk_type() << '\n';
            std::cerr << recorder.meta() << '\n';
            std::cerr << "Chunk META not found in " << opt.in_filename << std::endl;
            return 2001;
        }

        //std::cout
        //<< "input-file: " << opt.in_filename << '\n'
        //<< "frame limit: " << opt.frame << '\n'
        //<< "time: " << opt.time << '\n'
        //<< "range: " << opt.range << '\n'
        //;
    }
    catch (Error& error)
    {
        std::cerr << "Error " << error.id << ": " << strerror(error.errnum) << std::endl;
        return 100000 + error.errnum;
    }

    return 0;
}

#endif