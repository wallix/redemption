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


#pragma once

#include "utils/timeval_ops.hpp"

#include <chrono>


class TimeObj {
public:
    virtual ~TimeObj() = default;
    virtual timeval get_time() = 0;
};

class TimeSystem : public TimeObj {
public:
    TimeSystem() = default;
    ~TimeSystem() override = default;
    timeval get_time() override {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return tv;
    }
};

class LCGTime : public TimeObj {
    uint32_t seed;
public:
    explicit LCGTime(uint32_t seed = 7984813UL)
        : seed(seed)
    {}
    ~LCGTime() override = default;
    timeval get_time() override {
        timeval tv;
        tv.tv_sec = this->rand32();
        tv.tv_usec = this->rand32();
        return tv;
    }
    uint32_t rand32() {
        return this->seed = 6843513UL * this->seed + 451209UL;
    }
};

static inline std::chrono::milliseconds to_ms(timeval const& tv)
{
    return std::chrono::milliseconds(
        static_cast<uint64_t>(tv.tv_sec)*1000LL + static_cast<uint64_t>(tv.tv_usec)/1000L
    );
}

static inline std::chrono::microseconds ustime(const timeval & now) {
    return std::chrono::microseconds(
        static_cast<uint64_t>(now.tv_sec)*1000000LL + static_cast<uint64_t>(now.tv_usec)
    );
}

static inline timeval tvtime()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv;
}

static inline std::chrono::microseconds ustime() {
    return ustime(tvtime());
}

// As gettimeofday is not monotonic we may get surprising results (overflow). In these case we choose to send 0.
static inline std::chrono::microseconds difftimeval(const timeval& endtime, const timeval& starttime)
{
    auto d = ustime(endtime) - ustime(starttime);
    return (d > std::chrono::microseconds(0x100000000LL)) ? std::chrono::microseconds{} : d;
}

//static inline timeval usectotimeval(const std::chrono::microseconds time) {
//    timeval res;
//    res.tv_sec  = time / 1000000L;
//    res.tv_usec = time - (res.tv_sec * 1000000L);
//    // res.tv_usec = time % 1000000L;
//    return res;
//}

static inline timeval addusectimeval(const std::chrono::microseconds usec, const timeval & tv) {
    timeval res;
    uint64_t sum_usec = tv.tv_usec + usec.count();
    res.tv_sec  = sum_usec / 1000000;
    res.tv_usec = sum_usec - (res.tv_sec * 1000000L);
    res.tv_sec += tv.tv_sec;
    // res.tv_usec = sum_usec % 1000000;
    // res.tv_sec  = sum_usec / 1000000 + tv.tv_sec;
    return res;
}

/**
 * All these operations assume that any timeval.tv_usec is < 1000000L
 * Otherwise, the timeval is not well formated
 * @{
 */
static inline bool lessthantimeval(const timeval & before, const timeval & after) {
    return before < after;
}

//static inline timeval addtimeval(const timeval & time1, const timeval & time2) {
//    // return time1 + time2
//    timeval res;
//    bool carry = (time1.tv_usec + time2.tv_usec) >= 1000000L;
//    res.tv_usec = time1.tv_usec + time2.tv_usec - carry*1000000L;
//    res.tv_sec  = time1.tv_sec  + time2.tv_sec  + carry;
//
//    return res;
//}


static inline timeval how_long_to_wait(const timeval & alarm, const timeval & now) {
    // return number of usec to wait:
    // alarm - now if alarm is in the future
    // or 0 if alarm time is past
    timeval res = {0,0};
    if (!lessthantimeval(alarm, now)) {
        bool carry = alarm.tv_usec < now.tv_usec;
        res.tv_usec = alarm.tv_usec - now.tv_usec + carry*1000000L;
        res.tv_sec  = alarm.tv_sec  - now.tv_sec  - carry;
    }
    return res;
}


//static inline timeval absdifftimeval(const timeval & endtime, const timeval & starttime) {
//    // return | endtime - starttime |
//    timeval res;
//    if (!lessthantimeval(endtime, starttime)) {
//        bool carry = endtime.tv_usec < starttime.tv_usec;
//        res.tv_usec = endtime.tv_usec - starttime.tv_usec + carry*1000000L;
//        res.tv_sec  = endtime.tv_sec  - starttime.tv_sec  - carry;
//    }
//    else {
//        //TODO infinite loop if endtime = starttime
//        res = absdifftimeval(starttime, endtime);
//    }
//    return res;
//}

//static inline timeval multtimeval(uint64_t mult, const timeval & tv) {
//    timeval res;
//    uint64_t total_usec = tv.tv_usec * mult;
//    res.tv_sec  = total_usec / 1000000L;
//    res.tv_usec = total_usec - (res.tv_sec * 1000000L);
//    res.tv_sec += tv.tv_sec * mult;
//    return res;
//}

//static inline timeval mintimeval(const timeval & time1, const timeval & time2) {
//    // return min(time1,time2)
//    return lessthantimeval(time1, time2)?time1:time2;
//}
/**
 * @}
 */

inline timeval & operator+=(timeval& a, std::chrono::microseconds const& usec) /*NOLINT*/
{
    a = addusectimeval(usec, a);
    return a;
}
