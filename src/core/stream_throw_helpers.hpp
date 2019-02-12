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


#include "core/error.hpp"
#include "utils/stream.hpp"
#include "utils/log.hpp"

#pragma once

inline void check_throw(InStream & stream, size_t expected, const char * message, error_type eid)
{
    if (!stream.in_check_rem(expected)) {
        LOG(LOG_ERR, "Truncated %s: expected=%zu remains=%zu", message, expected, stream.in_remain());
        throw Error(eid);
    }
}

inline uint16_t in_uint16_le_throw(InStream & stream, const char * message, error_type eid)
{
    ::check_throw(stream, 2, message, eid);
    return stream.in_uint16_le();
}


