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

#if !defined(__MAIN_RECORDER_IOTYPE_BASIC_OUTPUT_TYPE_HPP__)
#define __MAIN_RECORDER_IOTYPE_BASIC_OUTPUT_TYPE_HPP__

#include <string>

struct BasicOutputType {
    enum enum_t {
        NOT_FOUND,
        PNG_TYPE,
        WRM_TYPE
    };

    static BasicOutputType::enum_t get_output_type(const std::string& filename)
    {
        std::size_t p = filename.find_last_of('.');
        if (p == std::string::npos || p + 4 != filename.length())
            return NOT_FOUND;
        if (!filename.compare(p+1, 3, "png"))
            return PNG_TYPE;
        if (!filename.compare(p+1, 3, "wrm"))
            return WRM_TYPE;
        return NOT_FOUND;
    }

    static BasicOutputType::enum_t string_type_to_enum(const std::string& filename)
    {
        if (filename == "png")
            return PNG_TYPE;
        if (filename == "wrm")
            return WRM_TYPE;
        return NOT_FOUND;
    }
};

#endif