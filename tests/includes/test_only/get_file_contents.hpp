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

#ifndef REDEMPTION_TESTS_GET_FILE_CONTENTS_HPP
#define REDEMPTION_TESTS_GET_FILE_CONTENTS_HPP

#include <fstream>

template<class String>
int get_file_contents(String& s, const char * name)
{
    typedef typename String::value_type char_type;
    typedef typename String::traits_type traits_type;

    std::basic_filebuf<char_type, traits_type> buf;

    char_type c;
    buf.pubsetbuf(&c, 1);

    if (!buf.open(name, std::ios::in)) {
        return errno;
    }

    const std::streamsize sz = buf.in_avail();
    if (sz == std::streamsize(-1)) {
        return errno;
    }

    s.resize(std::size_t(sz));
    const std::streamsize n = buf.sgetn(&s[0], std::streamsize(s.size()));
    return (sz != n) ? s.resize(std::size_t(n)), errno : 0;
}

template<class String>
String get_file_contents(const char * name)
{
    String s;
    get_file_contents(s, name);
    return s;
}

#endif
