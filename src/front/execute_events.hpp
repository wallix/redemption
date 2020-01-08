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
    std::chrono::milliseconds timeout, SessionReactor& session_reactor,
    CallbackEventContainer & front_events_,
    SessionReactor::EnableGraphics enable_graphics,
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
    session_reactor.fd_events_.for_each(g);
    if (enable_graphics) {
        session_reactor.graphic_fd_events_.for_each(g);
    }

    session_reactor.set_current_time(tvtime());
    timeval timeoutastv = to_timeval(
                            session_reactor.get_next_timeout(front_events_, enable_graphics, timeout)
                          - session_reactor.get_current_time());

    int num = select(max + 1, &rfds, nullptr, nullptr, &timeoutastv);

    if (num < 0) {
        if (errno == EINTR) {
            return ExecuteEventsResult::Continue;
        }
        return ExecuteEventsResult::Error;
    }

    session_reactor.set_current_time(tvtime());
    session_reactor.execute_timers(enable_graphics, [&]() -> gdi::GraphicApi& { return front; });

    if (num) {
        front_events_.exec_action(callback);
        auto fd_isset = [&rfds](int fd, auto& /*e*/){ return io_fd_isset(fd, rfds); };
        session_reactor.execute_graphics(fd_isset, front);
        return ExecuteEventsResult::Success;
    }

    return ExecuteEventsResult::Timeout;
}
