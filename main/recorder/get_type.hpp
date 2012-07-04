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

#if !defined(__MAIN_RECORDER_GET_TYPE__)
#define __MAIN_RECORDER_GET_TYPE__

#include "input_type.hpp"
#include "output_type.hpp"
#include "wrm_recorder_option.hpp"

inline OutputType::enum_t get_output_type(const WrmRecoderOption& opt)
{
    if (!opt.output_type.empty()){
        return OutputType::string_type_to_enum(opt.output_type);
    }
    return OutputType::get_output_type(opt.out_filename);
}

inline InputType::enum_t get_input_type(const WrmRecoderOption& opt)
{
    if (!opt.input_type.empty()){
        return InputType::string_type_to_enum(opt.input_type);
    }
    return InputType::get_input_type(opt.in_filename);
}

#endif