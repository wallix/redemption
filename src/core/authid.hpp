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


#pragma once

#include "configs/autogen/authid.hpp"

#include <cstring>

static inline authid_t authid_from_string(const char * strauthid) {
    authid_t res = AUTHID_UNKNOWN;
    for (unsigned i = 0; i < MAX_AUTHID; i++) {
        if (0 == strcmp(authstr[i], strauthid)) {
            res = static_cast<authid_t>(i);
            break;
        }
    }
    return res;
}

static inline const char * string_from_authid(authid_t authid) {
    return (authid >= authid_t::MAX_AUTHID)
        ? ""
        : authstr[static_cast<unsigned>(authid)];
}
