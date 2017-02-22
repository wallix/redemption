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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#pragma once

#include <cstring>
#include <cstdio>

inline bool check_mem(const void * p, std::size_t len, const void * mem, char * message)
{
    if (memcmp(p, mem, len)) {
        unsigned char const * sig = reinterpret_cast<unsigned char const *>(p);
        message += std::sprintf(message, "Expected signature: \"\\x%.2x", unsigned(*sig));
        while (--len) {
            message += std::sprintf(message, "\\x%.2x", unsigned(*++sig));
        }
        message[0] = '"';
        message[1] = 0;
        return false;
    }
    return true;
}

#define CHECK_MEM(p, len, mem)                   \
    {                                            \
        char message[len * 5 + 256];             \
        if (!check_mem(p, len, mem, message)) {  \
            BOOST_CHECK_MESSAGE(false, message); \
        }                                        \
    }
