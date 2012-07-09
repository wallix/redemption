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

// #define LOGPRINT

// #include "recorder/wrm_recorder_app.hpp"
// #include "recorder/init.hpp"


// #include "recorder/get_recorder_action.hpp"
// #include "recorder/wrm_recorder_option.hpp"

#include "recorder/wrm_recorder_option.hpp"
#include "recorder/recorder_run.hpp"
#include "recorder/get_type.hpp"
#include "recorder/to_png.hpp"
#include "recorder/to_wrm.hpp"

int main(int argc, char** argv)
{
    WrmRecorderOption opt;
    opt.parse_command_line(argc, argv);

    if (opt.options.count("version")) {
        std::cout << argv[0] << ' ' << opt.version() << '\n';
        return 0;
    }

    if (opt.options.count("help")) {
        std::cout << opt.desc << std::endl;
        return 0;
    }

    int error = opt.notify_options();
    if (error){
        std::cerr
        << WrmRecorderOption::get_cerror(error) << '\n'
        << opt.desc << std::endl
        ;
        return error;
    }

    InputType::enum_t itype = get_input_type(opt);
    if (itype == InputType::NOT_FOUND){
        std::cerr
        << "Incorrect input-type, "
        << opt.desc.find("input-type", false).description() << '\n';
        return 1000;
    }

    error = opt.normalize_options();
    if (error){
        std::cerr << WrmRecorderOption::get_cerror(error) << std::endl;
        return error;
    }

    typedef recorder_item_traits<WrmRecorderOption> item_traits;
    typedef item_traits::recorder_item recorder_item;
    recorder_item recorder_actions[] = {
        recorder_item("png", &to_png),
        recorder_item("wrm", &to_wrm),
    };

    return recorder_run<>(opt, recorder_actions, itype);
}
