/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include <iostream>
#include <sstream>

namespace tu
{

struct ostream_buffered
{
    ostream_buffered(std::ostream& ostream = std::cout)
    : ostream(ostream)
    , oldbuf(ostream.rdbuf(&sbuf))
    {}

    ~ostream_buffered()
    {
        ostream.rdbuf(oldbuf);
    }

    std::string str() const
    {
        ostream.rdbuf(oldbuf);
        return sbuf.str();
    }

private:
    std::ostream& ostream;
    std::stringbuf sbuf;
    std::streambuf * oldbuf;
};

} // namespace tu
