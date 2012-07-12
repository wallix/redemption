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

#if !defined(__MAIN_RECORDER_PARSE_COMMAND_LINE_HPP__)
#define __MAIN_RECORDER_PARSE_COMMAND_LINE_HPP__

#include "wrm_recorder_option.hpp"
#include "input_type.hpp"
#include "get_type.hpp"

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

#endif