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
   Martin Potier, Jonathan Poelen, Meng Tan
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

static inline timeval usectotimeval(const uint64_t time) {
    timeval res;
    res.tv_sec  = time / 1000000L;
    res.tv_usec = time % 1000000L;
    return res;
}

static inline timeval addusectimeval(const timeval & tv, const uint64_t usec) {
    timeval res;
    uint64_t sum_usec = tv.tv_usec + usec;
    res.tv_usec = sum_usec % 1000000;
    res.tv_sec  = sum_usec / 1000000 + tv.tv_sec;
    return res;
}

// All these operations assume that any timeval.tv_usec is < 1000000L
// Otherwise, the timeval is not well formated
static inline bool lessthantimeval(const timeval & before, const timeval & after) {
    // return before < after
    return (    (after.tv_sec > before.tv_sec)
             || (   (after.tv_sec == before.tv_sec)
                 && (after.tv_usec > before.tv_usec)));
}

static inline timeval addtimeval(const timeval & time1, const timeval & time2) {
    // return time1 + time2
    timeval res;
    bool carry = (time1.tv_usec + time2.tv_usec >= 1000000L);
    res.tv_usec = time1.tv_usec + time2.tv_usec - carry*1000000L;
    res.tv_sec = time1.tv_sec + time2.tv_sec + carry;

    return res;
}


static inline timeval how_long_to_wait(const timeval & alarm, const timeval & now) {
    // return number of usec to wait:
    // alarm - now if alarm is in the future
    // or 0 if alarm time is past
    timeval res = {};
    if (!lessthantimeval(alarm, now)) {
        bool carry = alarm.tv_usec < now.tv_usec;
        res.tv_usec = alarm.tv_usec - now.tv_usec + carry*1000000L;
        res.tv_sec  = alarm.tv_sec  - now.tv_sec  - carry;
    }
    return res;
}


static inline timeval absdifftimeval(const timeval & endtime, const timeval & starttime) {
    // return | endtime - starttime |
    timeval res;
    if (!lessthantimeval(endtime, starttime)) {
        bool carry = endtime.tv_usec < starttime.tv_usec;
        res.tv_usec = endtime.tv_usec - starttime.tv_usec + carry*1000000L;
        res.tv_sec  = endtime.tv_sec  - starttime.tv_sec  - carry;
    }
    else {
        res = absdifftimeval(starttime, endtime);
    }
    return res;
}

static inline timeval mintimeval(const timeval & time1, const timeval & time2) {
    // return min(time1,time2)
    return lessthantimeval(time1,time2)?time1:time2;
}

#endif
