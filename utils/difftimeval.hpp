/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
   Martin Potier, Jonathan Poelen
*/

#ifndef _REDEMPTION_UTILS_DIFFTIMEVAL_HPP_
#define _REDEMPTION_UTILS_DIFFTIMEVAL_HPP_

#include <sys/time.h>
#include <stdint.h>

static inline uint64_t ustime(const timeval & now) {
    return static_cast<uint64_t>(now.tv_sec)*1000000LL + static_cast<uint64_t>(now.tv_usec);
}

static inline const timeval tvtime()
{
    timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}

static inline uint64_t ustime() {
    return ustime(tvtime());
}

static inline uint64_t difftimeval(const timeval& endtime, const timeval& starttime)
REDOC("as gettimeofday is not monotonic we may get surprising results (overflow). In these case we choose to send 0.")
{
    uint64_t d = ustime(endtime) - ustime(starttime);
    return (d > 0x100000000LL)?0:d;
}

#endif
