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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <sys/select.h> // for FD_xxx macros as of POSIX.1-2001

#include "utils/invalid_socket.hpp"
#include "core/wait_obj.hpp"

// NOTE: old-style-cast is ignored because of FD_xxx macros using it behind the hood
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

//t?INVALID_SOCKET:t->sck

inline
void add_to_fd_set(wait_obj const & w, int fd, fd_set & rfds, unsigned & max, timeval & timeout)
{
    if (fd > INVALID_SOCKET) {
        FD_SET(fd, &rfds);
        max = (static_cast<unsigned>(fd) > max) ? fd : max;
    }
    if ((fd <= INVALID_SOCKET || w.object_and_time) && w.set_state) {
        struct timeval now;
        now = tvtime();
        timeval remain = how_long_to_wait(w.trigger_time, now);
        if (lessthantimeval(remain, timeout)) {
            timeout = remain;
        }
    }
}

inline
bool is_set(wait_obj & w, int fd, fd_set & rfds)
{
    w.waked_up_by_time = false;

    if (fd > INVALID_SOCKET) {
        bool res = FD_ISSET(fd, &rfds);

        if (res || !w.object_and_time) {
            return res;
        }
    }

    if (w.set_state) {
        if (tvtime() >= w.trigger_time) {
            w.waked_up_by_time = true;
            return true;
        }
    }

    return false;
}

#pragma GCC diagnostic pop

