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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <fstream>
#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/fileutils.hpp"

template<class String>
void append_file_contents(String& s, const char * name)
{
    using char_type = typename String::value_type;
    using traits_type = typename String::traits_type;

    std::basic_filebuf<char_type, traits_type> buf;

    const std::streamsize sz = filesize(name);
    if (sz == std::streamsize(-1)) {
        LOG(LOG_ERR, "failed to stat file size for %s : %s", name, strerror(errno));
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    char_type c;
    buf.pubsetbuf(&c, 1);

    if (!buf.open(name, std::ios::in)) {
        LOG(LOG_ERR, "failed to open file %s : %s", name, strerror(errno));
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    s.resize(std::size_t(sz));
    const std::streamsize n = buf.sgetn(&s[0], std::streamsize(s.size()));
    if (sz != n) {
        LOG(LOG_ERR, "failed to read file into buffer %s : %s", name, strerror(errno));
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }
}

template<class String = std::string>
String get_file_contents(const char * name)
{
    String s;
    append_file_contents(s, name);
    return s;
}

template<class String = std::string>
String get_file_contents(std::string const& name)
{
    return get_file_contents<String>(name.c_str());
}
