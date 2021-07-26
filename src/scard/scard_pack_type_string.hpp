/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <cstring>

#include "scard/scard_pack_type_array.hpp"
#include "scard/scard_pack_type_internal.hpp"


///////////////////////////////////////////////////////////////////////////////


class scard_pack_string : public scard_pack_array_primitive<
    char, scard_pack_array_size_auto,
    scard_pack_array_type_conformant_varying,
    scard_pack_pointer_type_full,
    SCARD_PACK_DWORD
>
{
public:
    ///
    scard_pack_string() = default;
    
    ///
    scard_pack_string(const char *data)
        : scard_pack_array_primitive<
            char, scard_pack_array_size_auto,
            scard_pack_array_type_conformant_varying,
            scard_pack_pointer_type_full,
            SCARD_PACK_DWORD
        >(data, (data ? (std::strlen(data) + 1) : 0))
    {
    }
};