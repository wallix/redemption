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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_UTILS_TIMEUTILS_HPP_
#define _REDEMPTION_UTILS_TIMEUTILS_HPP_

#include <ctime>

inline bool operator ==(const tm & a, const tm & b) {
    return (   (a.tm_sec   == b.tm_sec  )
            && (a.tm_min   == b.tm_min  )
            && (a.tm_hour  == b.tm_hour )
            && (a.tm_mday  == b.tm_mday )
            && (a.tm_mon   == b.tm_mon  )
            && (a.tm_year  == b.tm_year )
            && (a.tm_wday  == b.tm_wday )
            && (a.tm_yday  == b.tm_yday )
            && (a.tm_isdst == b.tm_isdst)
           );
}

inline bool operator !=(const tm & a, const tm & b) {
    return (!(a == b));
}

#endif  // #ifndef _REDEMPTION_UTILS_TIMEUTILS_HPP_
