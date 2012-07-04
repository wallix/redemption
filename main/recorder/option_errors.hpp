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

#if !defined(__MAIN_RECORDER_OPTION_ERRORS__)
#define __MAIN_RECORDER_OPTION_ERRORS__

struct RecorderOptionError
{
    static const uint SUCCESS = 0;
    static const uint OUT_FILENAME_IS_EMPTY = 1;
    static const uint IN_FILENAME_IS_EMPTY = 2;

    static const char * get_cstr(uint error)
    {
        if (error == 1)
            return "Not output-file";
        if (error == 2)
            return "Not input-file";
        return "Success";
    }
};

#endif