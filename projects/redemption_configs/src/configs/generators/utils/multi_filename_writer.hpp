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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_GENERATOR_MULTI_FILENAME_WRITER_HPP
#define REDEMPTION_SRC_GENERATOR_MULTI_FILENAME_WRITER_HPP

#include <fstream>
#include <cstring>
#include <string>


template<class Arg>
struct MultiFilenameWriter
{
    explicit MultiFilenameWriter(Arg & reference_arg, int errnum = 1)
    : arg(reference_arg)
    , errnum(errnum)
    {}

    template<class Fn>
    MultiFilenameWriter & then(std::string const& new_filename, Fn fn)
    {
        return this->then(new_filename.c_str(), fn);
    }

    template<class Fn>
    MultiFilenameWriter & then(const char * new_filename, Fn fn)
    {
        if (!this->err) {
            const char * filename = this->filename;
            if (new_filename && *new_filename && strcmp(this->filename, new_filename)) {
                this->of.close();
                this->of.open(new_filename);
                filename = new_filename;
            }

            fn(this->of, this->arg);
            this->err = !bool(this->of);
            this->filename = filename;
            ++this->errnum;
        }
        return *this;
    }

    char const * filename = "";
    Arg & arg;
    std::ofstream of;
    int errnum;
    int err = 0;
};

#endif
