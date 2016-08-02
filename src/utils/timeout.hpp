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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

class Timeout {
    time_t timeout;

public:
    typedef enum {
        TIMEOUT_REACHED,
        TIMEOUT_NOT_REACHED,
        TIMEOUT_INACTIVE
    } timeout_result_t;

    explicit Timeout(time_t now, time_t length = 0)
        : timeout(length ? (now + length) : static_cast<time_t>(0)) {}

    timeout_result_t check(time_t now) const {
        if (this->timeout) {
            if (now > this->timeout) {
                return TIMEOUT_REACHED;
            }
            else {
                return TIMEOUT_NOT_REACHED;
            }
        }
        return TIMEOUT_INACTIVE;
    }

    //bool is_cancelled() const {
    //    return (this->timeout == 0);
    //}

    time_t timeleft(time_t now) const {
        return (this->timeout - now);
    }

    void cancel_timeout() {
        this->timeout = 0;
    }

    void restart_timeout(time_t now, time_t length) {
        this->timeout = now + length;
    }
};



class TimeVal : public timeval {
public:
    TimeVal() {
        ::gettimeofday(this, nullptr);
    }

    explicit TimeVal(uint64_t usec) {
        this->tv_sec  = usec / 1000000L;
        this->tv_usec = usec % 1000000L;
    }

    explicit TimeVal(timeval const & tv) {
        this->tv_sec  = tv.tv_sec;
        this->tv_usec = tv.tv_usec;
    }

    inline TimeVal & operator+(timeval const & other) {
        this->tv_sec  += other.tv_sec;
        this->tv_usec += other.tv_usec;
        if (this->tv_usec >= 1000000LL) {
            this->tv_sec++;

            this->tv_usec -= 1000000LL;
        }

        return *this;
    }

    inline TimeVal & operator=(uint64_t usec) {
        this->tv_sec  = usec / 1000000L;
        this->tv_usec = usec % 1000000L;

        return *this;
    }

    inline TimeVal & operator=(timeval const & tv) {
        this->tv_sec  = tv.tv_sec;
        this->tv_usec = tv.tv_usec;

        return *this;
    }

    inline operator bool() const {
        return (this->tv_sec && this->tv_usec);
    }
};

static inline timeval operator-(timeval const & endtime, timeval const & starttime) {
    assert((endtime.tv_sec  > starttime.tv_sec ) ||
           (endtime.tv_usec > starttime.tv_usec));

    timeval result;

    result.tv_sec = endtime.tv_sec - starttime.tv_sec;

    if (endtime.tv_usec >= starttime.tv_usec) {
        result.tv_usec = endtime.tv_usec - starttime.tv_usec;
    }
    else {
        result.tv_sec--;

        result.tv_usec = 1000000LL - starttime.tv_usec + endtime.tv_usec;
    }

    return result;
}

static inline timeval operator+(timeval const & a, timeval const & b) {
    timeval result;

    result.tv_sec  = a.tv_sec  + b.tv_sec;
    result.tv_usec = a.tv_usec + b.tv_usec;
    if (result.tv_usec >= 1000000LL) {
        result.tv_sec++;

        result.tv_usec -= 1000000LL;
    }

    return result;
}

