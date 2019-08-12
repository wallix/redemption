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

#include "utils/literals/utf16.hpp"

template<char C> struct c {};

int main()
{
    constexpr auto s = "abc"_utf16_le;

    c<'a'>{} = c<s[0]>{};
    c<0>{}   = c<s[1]>{};
    c<'b'>{} = c<s[2]>{};
    c<0>{}   = c<s[3]>{};
    c<'c'>{} = c<s[4]>{};
    c<0>{}   = c<s[5]>{};
    // s[6] -> out of bounds of array_view
    c<0>{}   = c<s.data()[6]>{};
    c<0>{}   = c<s.data()[7]>{};
    // s.data()[8] -> out of bounds of char*
}
