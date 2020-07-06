/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2018
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/timebase.hpp"
#include "utils/select.hpp"
#include "core/events.hpp"


enum class ExecuteEventsResult
{
    Success,
    Error,
    Continue,
    Timeout,
};

inline ExecuteEventsResult execute_events(std::chrono::milliseconds timeout, EventContainer& events)
{
    timeval now = tvtime();
    unsigned max = 0;
    fd_set   rfds;
    io_fd_zero(rfds);

    timeval ultimatum =  now + timeout;

    events.get_fds([&rfds,&max](int fd){ io_fd_set(fd, rfds); max = std::max(max, unsigned(fd));});
    events.get_fds_timeouts([&ultimatum](timeval tv){ultimatum = std::min(tv,ultimatum);};);
    if (ultimatum < now){
        ultimatum = now;
    }
    timeval timeoutastv = ultimatum - now;

    int num = select(max + 1, &rfds, nullptr, nullptr, &timeoutastv);

    if (num < 0) {
        if (errno == EINTR) {
            return ExecuteEventsResult::Continue;
        }
        return ExecuteEventsResult::Error;
    }

    timeval now_after_select = tvtime();
    events.execute_events(now_after_select, [](int /*fd*/){ return false; });
    if (num) {
        events.execute_events(now_after_select, [&rfds](int fd){ return io_fd_isset(fd, rfds); });
        return ExecuteEventsResult::Success;
    }
    return ExecuteEventsResult::Timeout;
}
