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
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#pragma once

#include "core/events.hpp"


class Inifile;

class KeepAlive
{
public:
    KeepAlive(
        Inifile& ini,
        EventContainer& event_container,
        std::chrono::seconds grace_delay)
    : timer_event(event_container)
    , ini(ini)
    , grace_delay(grace_delay)
    {}

    void keep_alive();

    void start();

    void stop();

private:
    MonotonicTimePoint timeout() const;

    bool wait_answer = false;   // true when we are waiting for a positive response
                                // false when positive response has been received and
                                // timers have been set to new timers.
    EventRef2 timer_event;
    Inifile& ini;
    std::chrono::seconds grace_delay;
};


class Inactivity
{
public:
    Inactivity(EventContainer& event_container)
    : timer_event(event_container)
    {}

    void activity();

    void start(std::chrono::seconds delay);

    void stop();

private:
    EventRef2 timer_event;
    std::chrono::seconds grace_delay;
};


class EndSessionWarning
{
public:
    EndSessionWarning(EventContainer& event_container)
    : timer_event(event_container)
    {}

    /// disable timer with 0 for \c end_date
    void set_time(MonotonicTimePoint end_date);

    /// add delay (can be negative)
    void add_delay(MonotonicTimePoint::duration delay);

    // void reset()
    // {
    //     this->timer_event.garbage();
    //     this->last_delay = this->last_delay.zero();
    // }

    template<class Fn>
    void update_warning(Fn&& fn)
    {
        using namespace std::literals::chrono_literals;

        if (this->last_delay.count()) {
            // ajust minutes
            fn(std::chrono::duration_cast<std::chrono::minutes>(this->last_delay + 30s));
            this->last_delay = this->last_delay.zero();
        }
    }

private:
    MonotonicTimePoint next_timeout(MonotonicTimePoint now);

    MonotonicTimePoint timer_close;
    MonotonicTimePoint::duration last_delay {};
    EventRef2 timer_event;
};
