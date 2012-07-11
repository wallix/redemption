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

    const timeval& time() const
    {
        return this->tv;
    }

    timeval& impl()
    { return this->tv; }

    static TimerCapture invalid_timer()
    {
        return TimerCapture(0);
    }

    bool valid() const
    { return this->sec() != 0; }

    uint64_t elapsed(const struct timeval& now)
    {
        uint64_t diff = difftimeval(now, this->tv);
        this->tv = now;
        return diff;
    }

    uint64_t elapsed()
    {
        struct timeval now;
        gettimeofday(&now, 0);
        return elapsed(now);
    }

    bool elapsed_if_wait(const struct timeval& now, uint64_t elapsed)
    {
        if (difftimeval(now, this->tv) > elapsed)
        {
            this->tv = now;
            return true;
        }
        return false;
    }

    bool elapsed_if_wait(uint64_t elapsed)
    {
        struct timeval now;
        gettimeofday(&now, 0);
        return elapsed_if_wait(now ,elapsed);
    }

    TimerCapture& operator += (uint64_t useconds)
    {
        useconds += this->usec();
        this->sec() += useconds / 1000000;
        this->usec() = useconds % 1000000;
        /*this->now.tv_sec += useconds / 1000000;
        this->now.tv_usec += useconds % 1000000;
        if (this->now.tv_usec >= 1000000)
        {
            this->now.tv_sec += this->now.tv_usec / 1000000;
            this->now.tv_usec %= 1000000;
        }*/
        return *this;
    }

    using URT::reset;
};

struct WaitCapture {
    TimerCapture timer;

private:
    uint64_t time_future;

//public:
    uint64_t time_wait;

public:
    WaitCapture()
    : timer(TimerCapture::invalid_timer())
    , time_future(0)
    {}

    void future(uint64_t micro_sec)
    {
        if (this->timer.valid())
        {
            uint64_t elapsed = this->timer.elapsed() + this->time_future;
            if (elapsed <= micro_sec)
            {
                this->time_wait = micro_sec - elapsed;
                this->time_future = 0;
            }
            else
            {
                this->time_future = elapsed - micro_sec;
                this->time_wait = 0;
            }
        }
        else
        {
            timer.reset();
            this->time_wait = micro_sec;
            this->time_future = 0;
        }
    }

    void sleep()
    {
        if (this->time_wait)
        {
            struct timespec wtime = {
                this->time_wait / 1000000,
                this->time_wait % 1000000 * 1000
            };
            nanosleep(&wtime, NULL);
            this->timer += this->time_wait;
        }
    }

    void fake_sleep()
    {
        if (this->time_wait)
        {
            this->timer += this->time_wait;
        }
    }

    void wait(uint64_t micro_sec)
    {
        this->future(micro_sec);
        this->sleep();
    }

    void fake_wait(uint64_t micro_sec)
    {
        this->future(micro_sec);
        this->fake_sleep();
    }
};

#endif
