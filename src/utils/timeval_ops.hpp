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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include <sys/time.h>
#include <stdint.h>

inline bool operator<(const timeval & a, const timeval & b) noexcept {
    // return ustime(a) < ustime(b)
    return a.tv_sec < b.tv_sec
       || (a.tv_sec == b.tv_sec
        && a.tv_usec < b.tv_usec);
}

inline bool operator==(const timeval & a, const timeval & b) noexcept {
    return a.tv_sec == b.tv_sec && a.tv_usec == b.tv_usec;
}

inline bool operator!=(const timeval & a, const timeval & b) noexcept {
    return !(a == b);
}

inline bool operator>(const timeval & a, const timeval & b) noexcept {
    return b < a;
}

inline bool operator<=(const timeval & a, const timeval & b) noexcept {
    return !(b < a);
}

inline bool operator>=(const timeval & a, const timeval & b) noexcept {
    return !(a < b);
}

