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

#include "core/session_reactor.hpp"
#include "gdi/graphic_api.hpp"
#include "core/callback.hpp"
#include "utils/select.hpp"


enum class ExecuteEventsResult
{
    Success,
    Error,
    Continue,
    Timeout,
};

inline ExecuteEventsResult execute_events(
    std::chrono::milliseconds timeout,
    SessionReactor& session_reactor,
    TopFdContainer& fd_events_,
    GraphicFdContainer& graphic_fd_events_,
    TimerContainer& timer_events_,
    GraphicEventContainer & graphic_events_,
    GraphicTimerContainer & graphic_timer_events_,
    CallbackEventContainer & front_events_,
    EnableGraphics enable_graphics,
    Callback& callback, gdi::GraphicApi& front)
{
    unsigned max = 0;
    fd_set   rfds;
    io_fd_zero(rfds);

    auto g = [&rfds,&max](int fd, auto& /*top*/){
        assert(fd != -1);
        io_fd_set(fd, rfds);
        max = std::max(max, unsigned(fd));
    };
    fd_events_.for_each(g);
    if (enable_graphics) {
        graphic_fd_events_.for_each(g);
    }

    session_reactor.set_current_time(tvtime());
    timeval timeoutastv = to_timeval(
                            session_reactor.get_next_timeout(fd_events_, graphic_fd_events_, timer_events_, graphic_events_, graphic_timer_events_, front_events_, enable_graphics, timeout)
                          - session_reactor.get_current_time());

    int num = select(max + 1, &rfds, nullptr, nullptr, &timeoutastv);

    if (num < 0) {
        if (errno == EINTR) {
            return ExecuteEventsResult::Continue;
        }
        return ExecuteEventsResult::Error;
    }

    session_reactor.set_current_time(tvtime());
    auto const end_tv = session_reactor.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    if (enable_graphics) {
        graphic_timer_events_.exec_timer(end_tv, front);
        graphic_fd_events_.exec_timeout(end_tv, front);
    }

    if (num) {
        front_events_.exec_action(callback);
        auto fd_isset = [&rfds](int fd, auto& /*e*/){ return io_fd_isset(fd, rfds); };
        graphic_events_.exec_action(front);
        graphic_fd_events_.exec_action(fd_isset, front);
        return ExecuteEventsResult::Success;
    }

    return ExecuteEventsResult::Timeout;
}
