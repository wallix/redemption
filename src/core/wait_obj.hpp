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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan, Raphaël ZHOU
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Synchronisation objects
*/

#pragma once

#include "utils/difftimeval.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/select.hpp"

enum BackEvent_t {
    BACK_EVENT_NONE = 0,
    BACK_EVENT_NEXT,
    BACK_EVENT_STOP = 4,
    BACK_EVENT_REFRESH,

    BACK_EVENT_RETRY_CURRENT
};

class wait_obj
{
public:
    BackEvent_t signal = BACK_EVENT_NONE;

private:
    timeval trigger_time = { 0, 0 };

    bool    waked_up_by_time = false;

public:
    static constexpr const uint64_t NOW = 0;

    wait_obj() = default;

private:
    wait_obj(wait_obj&&) = delete;
    wait_obj(const wait_obj&) = delete;
    wait_obj& operator=(const wait_obj&) = delete;
    wait_obj& operator=(wait_obj&&) = default; // for full_reset()

public:
    bool is_trigger_time_set() const {
        return this->trigger_time != timeval{0, 0};
    }

    bool is_waked_up_by_time() const {
        return this->waked_up_by_time;
    }

    timeval get_trigger_time() {
        return this->trigger_time;
    }

    void set_waked_up_by_time(bool waked_up_by_time) {
        this->waked_up_by_time = waked_up_by_time;
    }

    void full_reset()
    {
        *this = wait_obj();
    }

    void reset_trigger_time()
    {
        this->waked_up_by_time = false;

        this->trigger_time = timeval{0, 0};
    }

    void set_trigger_time(std::chrono::microseconds idle_usec)
    {
        this->waked_up_by_time = false;

        struct timeval now = tvtime();

        this->trigger_time = addusectimeval(idle_usec, now);
    }

    // Idle time in microsecond
    void set_trigger_time(uint64_t idle_usec)
    {
        this->set_trigger_time(std::chrono::microseconds(idle_usec));
    }

    void update_trigger_time(std::chrono::microseconds idle_usec)
    {
        if (!idle_usec.count()) {
            return;
        }

        if (this->is_trigger_time_set()) {
            timeval now = tvtime();
            timeval new_trigger = addusectimeval(idle_usec, now);
            if (lessthantimeval(new_trigger, this->trigger_time)) {
                this->trigger_time = new_trigger;
            }
        }
        else {
            this->set_trigger_time(idle_usec);
        }
    }

    // Idle time in microsecond
    void update_trigger_time(uint64_t idle_usec)
    {
        this->update_trigger_time(std::chrono::microseconds(idle_usec));
    }

    void wait_on_timeout(timeval & timeout) const
    {
        if (this->is_trigger_time_set()) {
            timeval now = tvtime();
            timeval remain = how_long_to_wait(this->trigger_time, now);
            if (lessthantimeval(remain, timeout)) {
                timeout = remain;
            }
        }
    }

    void wait_on_fd(int fd, fd_set & rfds, unsigned & max, timeval & timeout) const
    {
        // TODO: shouldn't we *always* have a timeout ?
        if (fd > INVALID_SOCKET) {
            io_fd_set(fd, rfds);
            max = (static_cast<unsigned>(fd) > max) ? fd : max;
        }

        this->wait_on_timeout(timeout);
    }

    bool is_set(int fd, fd_set & rfds)
    {
        this->waked_up_by_time = false;

        if (fd > INVALID_SOCKET) {
            bool res = io_fd_isset(fd, rfds);

            if (res || !this->is_trigger_time_set()) {
                return res;
            }
        }

        if (this->is_trigger_time_set()) {
            if (tvtime() >= this->trigger_time) {
                this->waked_up_by_time = true;
                return true;
            }
        }

        return false;
    }

    bool is_set()
    {
        this->waked_up_by_time = false;

        if (this->is_trigger_time_set()) {
            if (tvtime() >= this->trigger_time) {
                this->waked_up_by_time = true;
                return true;
            }
        }

        return false;
    }
};
