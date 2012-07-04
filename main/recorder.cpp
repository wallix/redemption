/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 *
 * recorder main program
 *
 */

#include <iostream>

//#define LOGPRINT

#include "recorder/wrm_recorder_app.hpp"
#include "recorder/init.hpp"

int main(int argc, char** argv)
{
    WrmRecoderOption opt;
    InputType::enum_t itype;
    OutputType::enum_t otype;

    {
        int error = init_opt_and_iotype(opt, argc, argv, itype, otype);
        if (error)
            return error;
    }

    std::cout
    << "output-file: " << opt.out_filename << '\n'
    << "input-file: " << opt.in_filename << '\n'
    << "frame limit: " << opt.frame << '\n'
    << "time: " << opt.time << '\n'
    << "range: " << opt.range << '\n'
    ;

    WrmRecorderApp app(opt);

    try {
        app.run(otype, itype);
    } catch (Error e) {
        std::cerr
        << "id: " << e.id
        << ", errnum: " << e.errnum
        << ", strerror: " << strerror(e.errnum)
        << std::endl;
    }

    return 0;
}