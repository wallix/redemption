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
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Meng Tan
*/

#pragma once

#include "utils/difftimeval.hpp"

class SessionInactivity
{
public:
    SessionInactivity() noexcept;
    std::chrono::seconds get_inactivity_timeout() const noexcept;
    time_t get_last_activity_time() const noexcept;
    bool get_timer_started() const noexcept;
    bool activity(time_t now, bool& has_user_activity) noexcept;
    void update_inactivity_timeout(std::chrono::seconds timeout);
    void start_timer(std::chrono::seconds timeout, time_t start) noexcept;
    void stop_timer() noexcept;
   

private :    
    static constexpr std::chrono::seconds ACCEPTED_TIMEOUT_MIN = 30s;
    static constexpr std::chrono::seconds ACCEPTED_TIMEOUT_MAX = 168h; // one week

    // Inactivity management
    // let t be the timeout of the blocking select in session loop,
    // the effective inactivity timeout detection will be between
    // inactivity_timeout and inactivity_timeout + t.
    // hence we should have t << inactivity_timeout.
    std::chrono::seconds _inactivity_timeout;
    time_t _last_activity_time;
    bool _timer_started;
};
