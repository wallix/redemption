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
Copyright (C) Wallix 2017
Author(s): Jonathan Poelen
*/

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "core/session_reactor.hpp"
#include "mod/mod_api.hpp"
#include "mod/mod_api.hpp"

inline void execute_negociate_mod(
    TimeBase& time_base, TopFdContainer & fd_events_, GraphicFdContainer & graphic_fd_events_, TimerContainer& timer_events_, GraphicEventContainer & graphic_events_, GraphicTimerContainer & graphic_timer_events_, mod_api& mod, gdi::GraphicApi& gd)
{
    auto end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    // also gd enabled
    graphic_timer_events_.exec_timer(end_tv, gd);
    graphic_fd_events_.exec_timeout(end_tv, gd);

    int n = 0;
    int const limit = 1000;
    while (!mod.is_up_and_running()
        && (!graphic_events_.is_empty() || !graphic_fd_events_.is_empty())
        && (++n < limit)
    ) {
        auto is_set = [](int /*fd*/, auto& /*e*/){ return true; };
        graphic_events_.exec_action(gd);
        graphic_fd_events_.exec_action(is_set, gd);
    }
    RED_REQUIRE_LT(n, limit);
}

inline void execute_mod(TimeBase& time_base, TopFdContainer & fd_events_, GraphicFdContainer & graphic_fd_events_, TimerContainer& timer_events_, GraphicEventContainer & graphic_events_, GraphicTimerContainer & graphic_timer_events_, mod_api& mod, gdi::GraphicApi& gd, int n)
{
    execute_negociate_mod(time_base, fd_events_, graphic_fd_events_, timer_events_, graphic_events_, graphic_timer_events_, mod, gd);
    int count = 0;
    for (; count < n && (!graphic_events_.is_empty() || !graphic_fd_events_.is_empty()); ++count) {
        // LOG(LOG_INFO, "===================> count = %u", count);
        auto is_set = [](int /*fd*/, auto& /*e*/){ return true; };
        graphic_events_.exec_action(gd);
        graphic_fd_events_.exec_action(is_set, gd);
    }
    RED_CHECK_EQ(count, n);
}
