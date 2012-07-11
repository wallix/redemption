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

#include "recorder_run.hpp"
#include "parse_command_line.hpp"

int recorder_app(RecorderOption& opt, int argc, char** argv,
                 RecorderAction* actions, std::size_t n)
{
    opt.accept_output_type<>(RecorderActionStringIterator(actions),
                             RecorderActionStringIterator(actions + n));

    if (!parse_command_line(opt, argc, argv)){
        return 0;
    }

    InputType::enum_t itype;
    if (int error = opt.prepare(itype)){
        return error;
    }

    return recorder_run(opt, actions, n, itype);
}

#endif