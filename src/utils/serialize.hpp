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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean

   bunch of functions used to serialized binary data
*/

#pragma once

inline std::array<uint8_t, 2> out_uint16_le(unsigned int v)
{
    return {uint8_t(v), uint8_t(v >> 8)};
}

inline void push_back_array(std::vector<uint8_t> & v, const array_view_u8 a)
{
    v.insert(std::end(v), a.data(), a.data() + a.size());
}
