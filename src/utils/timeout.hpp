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

#include <sys/time.h>

class Timeout
{
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
