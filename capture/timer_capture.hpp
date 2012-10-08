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
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 */

#if !defined(__TIMER_CAPTURE_HPP__)
#define __TIMER_CAPTURE_HPP__

#include <sys/time.h>
#include "difftimeval.hpp"
#include "urt.hpp"

class TimerCapture
: public URT
{
public:
    typedef struct timeval time_type;

private:
    TimerCapture(int)
    : URT(0,0)
    {}

public:
    TimerCapture()
    : URT()
    {}

    TimerCapture(const timeval& now)
    : URT(now)
    {}

    TimerCapture& operator=(const timeval& other)
    {
        this->tv = other;
        return *this;
    }

//    const timeval& time() const
//    {
//        return this->tv;
//    }

//    timeval& impl()
//    { return this->tv; }

//    static TimerCapture invalid_timer()
//    {
//        return TimerCapture(0);
//    }

//    bool valid() const
//    { return this->sec() != 0; }


    TimerCapture& operator += (uint64_t useconds)
    {
        useconds += this->usec();
        this->sec() += useconds / 1000000;
        this->usec() = useconds % 1000000;
        return *this;
    }

    void reset()
    {
        gettimeofday(&(this->tv), NULL);
    }
};

#endif
