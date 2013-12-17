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
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Synchronisation objects

*/

#ifndef _REDEMPTION_CORE_WAIT_OBJ_HPP_
#define _REDEMPTION_CORE_WAIT_OBJ_HPP_

#include "error.hpp"
#include <stdint.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include "difftimeval.hpp"
#include "sockettransport.hpp"

enum BackEvent_t {
    BACK_EVENT_NONE = 0,
    BACK_EVENT_NEXT,
    BACK_EVENT_STOP = 4,
    BACK_EVENT_REFRESH,
};


class wait_obj
{
public:
    SocketTransport * st;
    bool              set_state;
    BackEvent_t       signal;
    struct timeval    trigger_time;
    bool              object_and_time;
    bool              waked_up_by_time;

    wait_obj(SocketTransport * socktrans, bool object_and_time = false)
    : st(socktrans)
    , set_state(false)
    , signal(BACK_EVENT_NONE)
    , object_and_time(object_and_time)
    , waked_up_by_time(false)
    {
        this->trigger_time = tvtime();
    }

    ~wait_obj()
    {
        if ((this->st != NULL) && (this->st->sck > 0)){
            close(this->st->sck);
        }
    }

    void add_to_fd_set(fd_set & rfds, unsigned & max, timeval & timeout)
    {
        if ((this->st != NULL) && (this->st->sck > 0)){
            FD_SET(this->st->sck, &rfds);
            max = ((unsigned)this->st->sck > max)?this->st->sck:max;
        }
        if (((this->st == NULL) || (this->st->sck <= 0) || this->object_and_time) && this->set_state) {
            struct timeval now;
            now = tvtime();
            timeval remain = how_long_to_wait(this->trigger_time, now);
            if (lessthantimeval(remain, timeout)){
                timeout = remain;
            }
        }
    }

    void reset()
    {
        this->set_state = false;
    }

    bool is_set(fd_set & rfds)
    {
        this->waked_up_by_time = false;

        if ((this->st != NULL) && (this->st->sck > 0)) {
            bool res = FD_ISSET(this->st->sck, &rfds);

            if (res || !this->object_and_time) {
                return res;
            }
        }

        if (this->set_state) {
            struct timeval now;
            now = tvtime();
            if ((now.tv_sec > this->trigger_time.tv_sec) ||
                ((now.tv_sec  == this->trigger_time.tv_sec) &&
                 (now.tv_usec >= this->trigger_time.tv_usec))) {
                this->waked_up_by_time = true;
                return true;
            }
        }

        return false;
    }

    // Idle time in microsecond
    void set(uint64_t idle_usec = 0)
    {
        this->set_state = true;
        struct timeval now = tvtime();

        // uint64_t sum_usec = (now.tv_usec + idle_usec);
        // this->trigger_time.tv_sec = (sum_usec / 1000000) + now.tv_sec;
        // this->trigger_time.tv_usec = sum_usec % 1000000;
        this->trigger_time = addusectimeval(idle_usec, now);
    }

    // Idle time in microsecond
    void update(uint64_t idle_usec)
    {
        if (this->set_state) {
            struct timeval now = tvtime();
            timeval new_trigger = addusectimeval(idle_usec, now);
            if (lessthantimeval(new_trigger, this->trigger_time)) {
                this->trigger_time = new_trigger;
            }
        }
        else {
            this->set(idle_usec);
        }
    }

    bool can_recv()
    {
        fd_set rfds;
        struct timeval time;
        int rv = false;

        time.tv_sec = 0;
        time.tv_usec = 0;
        FD_ZERO(&rfds);
        if ((this->st != NULL) && (this->st->sck > 0)) {
            FD_SET(this->st->sck, &rfds);
            rv = select(this->st->sck + 1, &rfds, 0, 0, &time); /* don't wait */
            if (rv > 0) {
                int opt;
                unsigned int opt_len = sizeof(opt);

                if (getsockopt(this->st->sck, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&opt), &opt_len) == 0) {
                    rv = (opt == 0);
                }
            }
        }
        return rv;
    }
};

#endif
