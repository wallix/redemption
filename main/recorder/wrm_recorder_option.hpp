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
#include "input_type.hpp"
#include "cipher.hpp"

struct WrmRecorderOption
{
    boost::program_options::options_description desc;
    boost::program_options::variables_map options;

    range_time_point range;
    uint frame;
    time_point time;
    std::string in_filename;
    uint idx_start;
    std::string base_path;
    std::string metaname;
    bool ignore_dir_for_meta_in_wrm;
    std::string input_type;
    bool times_in_meta_are_false;
    CipherMode::enum_t in_cipher_mode;
    std::string in_cipher_key;
    std::string in_cipher_iv;

    WrmRecorderOption();

    /*virtual*/ void parse_command_line(int argc, char** argv);

    /**
     * Return 0 if success.
     * @{
     */
    virtual int notify_options();
    virtual int normalize_options();
    //@}

    virtual const char * version() const
    {
        return "0.1";
    };

    enum Error {
        SUCCESS,
        IN_FILENAME_IS_EMPTY,
        KEY_OR_IV_WITHOUT_MODE
    };

    virtual const char * get_cerror(int error)
    {
        if (error == IN_FILENAME_IS_EMPTY)
            return "Not input-file";
        if (error == KEY_OR_IV_WITHOUT_MODE)
            return "Set --cipher-key or --cipher-iv without --cipher-mode";
        if (error == SUCCESS)
            return "Success";
        return "Unknow";
    }

    /**
     * Normalize option and display error
     * @param[out] itype
     * Return 0 if success
     */
    int prepare(InputType::enum_t& itype);

private:
    void add_default_options();
};

#endif