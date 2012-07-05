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

#include "wrm_recorder_option.hpp"
#include "input_type.hpp"

/*template<typename _Converter, typename _Format = typename _Converter::format_type>
_Format filename_to_type(const std::string& filename)
{
    const std::size_t pos = filename.find_last_of('.');
    return _Converter::string_to_type(filename.substr(pos + 1));
}

template<typename _Converter, typename _Format = typename _Converter::format_type>
_Format get_output_type(const WrmRecoderOption& opt)
{
    if (!opt.output_type.empty()){
        return _Converter::string_to_type(opt.output_type);
    }
    return filename_to_type<_Converter, _Format>(opt.out_filename);
}*/

inline InputType::enum_t get_input_type(const WrmRecorderOption& opt)
{
    if (!opt.input_type.empty()){
        return InputType::string_to_type(opt.input_type);
    }
    const std::size_t pos = opt.in_filename.find_last_of('.');
    return InputType::string_to_type(opt.in_filename.substr(pos + 1));
}

#endif