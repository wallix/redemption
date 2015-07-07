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

#ifndef REDEMPTION_SRC_CORE_AUTHID_HPP
#define REDEMPTION_SRC_CORE_AUTHID_HPP

#include "configs/authid.hpp"

#include <cstring>

static inline authid_t authid_from_string(const char * strauthid) {
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID - 1 ; i++) {
        if (0 == strcmp(authstr[i], strauthid)) {
            res = static_cast<authid_t>(i + 1);
            break;
        }
    }
    return res;
}

static inline const char * string_from_authid(authid_t authid) {
    if (authid == AUTHID_UNKNOWN || authid >= MAX_AUTHID)
        return "";
    return authstr[static_cast<unsigned>(authid) - 1];
}

#endif
