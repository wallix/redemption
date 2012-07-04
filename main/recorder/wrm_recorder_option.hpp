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

#if !defined(__MAIN_RECORDER_WRM_RECORDER_OPTION__)
#define __MAIN_RECORDER_WRM_RECORDER_OPTION__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "range_time_point.hpp"

struct WrmRecoderOption {
    boost::program_options::options_description desc;
    boost::program_options::variables_map options;

    range_time_point range;
    uint frame;
    time_point time;
    std::string out_filename;
    std::string in_filename;
    std::string idx_start;
    std::string base_path;
    std::string metaname;
    bool screenshot_wrm;
    bool screenshot_start;
    bool no_screenshot_stop;
    bool ignore_dir_for_meta_in_wrm;
    std::string output_type;
    std::string input_type;

    WrmRecoderOption();

    void parse_command_line(int argc, char** argv);

    /**
     * Return 0 if success.
     */
    uint notify_options();
    uint normalize_options();

    const char * version() const
    {
        return "0.1";
    };

private:
    void add_default_options();
};

#endif