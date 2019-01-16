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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <iosfwd>

#define REDEMPTION_OSTREAM(out_name, param) \
    template<class Ch, class Tr>            \
    std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out_name, param)

#define REDEMPTION_ISTREAM(in_name, param) \
    template<class Ch, class Tr>           \
    std::basic_istream<Ch, Tr>& operator>>(std::basic_istream<Ch, Tr>& in_name, param)


#define REDEMPTION_FRIEND_OSTREAM(out_name, param) \
    template<class Ch, class Tr>                   \
    friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& out_name, param)

#define REDEMPTION_FRIEND_ISTREAM(in_name, param) \
    template<class Ch, class Tr>                  \
    friend std::basic_istream<Ch, Tr>& operator>>(std::basic_istream<Ch, Tr>& in_name, param)
