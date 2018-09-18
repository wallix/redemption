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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/sugar/array_view.hpp"

namespace cpack
{
    struct Uint32_le
    {
        uint8_t array[4];

        Uint32_le(uint16_t v) noexcept
        {
            this->array[0] = static_cast<uint8_t>(v);
            this->array[1] = static_cast<uint8_t>(v >> 8);
            this->array[2] = static_cast<uint8_t>(v >> 16);
            this->array[3] = static_cast<uint8_t>(v << 24);
        }

        array_view_const_u8 av() const noexcept
        {
            return make_array_view(this->array);
        }

        operator array_view_const_u8() const noexcept
        {
            return this->av();
        }
    };

    struct Uint16_le
    {
        uint8_t array[2];

        Uint16_le(uint16_t v) noexcept
        {
            this->array[0] = static_cast<uint8_t>(v);
            this->array[1] = static_cast<uint8_t>(v >> 8);
        }

        array_view_const_u8 av() const noexcept
        {
            return make_array_view(this->array);
        }

        operator array_view_const_u8() const noexcept
        {
            return this->av();
        }
    };
} // namespace cpack
