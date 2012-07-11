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

#if !defined(__MAIN_RECORDER_RECORDER_OPTION__)
#define __MAIN_RECORDER_RECORDER_OPTION__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "wrm_recorder_option.hpp"
#include "range_time_point.hpp"

struct RecorderOption
: WrmRecorderOption
{
    std::string out_filename;
    std::string output_type;
    bool screenshot_wrm;
    bool screenshot_start;
    bool no_screenshot_stop;

    RecorderOption();

    template<typename _ForwardIterator>
    void accept_output_type(_ForwardIterator first, _ForwardIterator last)
    {
        if (first == last){
            throw std::runtime_error("output type is empty");
        }
        std::string output_type_desc = "accept ";
        for (; first != last; ++first){
            output_type_desc += '\'';
            output_type_desc += *first;
            output_type_desc += "', ";
        }
        output_type_desc.erase(output_type_desc.size() - 2);
        std::size_t pos = output_type_desc.find_last_of(',');
        if (pos != std::string::npos){
            output_type_desc[pos] = ' ';
            output_type_desc.insert(pos + 1, "or");
        }
        this->add_output_type(output_type_desc);
    }

    /**
     * Return 0 if success.
     */
    virtual int notify_options();
    virtual int normalize_options();

    virtual const char * version() const
    {
        return "0.1";
    };

    enum Error {
        SUCCESS              = WrmRecorderOption::SUCCESS,
        IN_FILENAME_IS_EMPTY = WrmRecorderOption::IN_FILENAME_IS_EMPTY,
        OUT_FILENAME_IS_EMPTY
    };

    virtual const char * get_cerror(int error)
    {
        if (error == OUT_FILENAME_IS_EMPTY)
            return "Not output-file";
        return WrmRecorderOption::get_cerror(error);
    }

private:
    void add_default_options();
    void add_output_type(const std::string& desc);
};

#endif