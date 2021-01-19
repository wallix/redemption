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

#include "core/error.hpp"
#include "core/events.hpp"
#include "configs/config.hpp"
#include "utils/timebase.hpp"


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

    void keep_alive()
    {
        if (auto* event = this->timer_event.get_optional_event()) {
            this->wait_answer = false;
            event->alarm.reset_timeout(this->timeout());
        }
    }

    void start()
    {
        this->wait_answer = false;
        this->timer_event.reset_timeout_or_create_event(
            this->timeout(), "Keepalive",
            [this](Event& e) {
                if (!this->wait_answer) {
                    this->ini.ask<cfg::context::keepalive>();
                    this->wait_answer = true;
                    e.alarm.reset_timeout(this->timeout());
                }
                else {
                    e.garbage = true;
                    LOG(LOG_INFO, "Close because of missed ACL keepalive");
                    throw Error(ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED);
                }
            }
        );
    }

    void stop()
    {
        this->timer_event.garbage();
    }

private:
    MonotonicTimePoint timeout() const
    {
        return this->timer_event.event_container().get_current_time() + this->grace_delay;
    }

    bool wait_answer = false;   // true when we are waiting for a positive response
                                // false when positive response has been received and
                                // timers have been set to new timers.
    EventRef2 timer_event;
    Inifile& ini;
    std::chrono::seconds grace_delay;
};


class Inactivity
{
    static constexpr std::chrono::seconds ACCEPTED_TIMEOUT_MIN = 30s;

public:
    Inactivity(EventContainer& event_container)
    : timer_event(event_container)
    {}

    void activity()
    {
        this->timer_event.reset_timeout(this->grace_delay);
    }

    void start(std::chrono::seconds delay)
    {
        if (delay == delay.zero()) {
            LOG(LOG_INFO, "User session inactivity : unlimited");
            this->timer_event.garbage();
            return ;
        }

        if (delay < ACCEPTED_TIMEOUT_MIN) {
            LOG(LOG_INFO,
                "Timeout wanted %ld is lower than minimum allowed, setting it to minimum",
                delay.count());
            delay = ACCEPTED_TIMEOUT_MIN;
        }

        LOG(LOG_INFO, "User session inactivity : set to %ld seconds", delay.count());

        this->grace_delay = delay;
        this->timer_event.reset_timeout_or_create_event(
            this->grace_delay, "Inactivity",
            [](Event& e) {
                e.garbage = true;
                LOG(LOG_INFO, "Close because of user Inactivity");
                throw Error(ERR_SESSION_CLOSE_USER_INACTIVITY);
            }
        );
    }

    void stop()
    {
        if (this->timer_event) {
            this->timer_event.garbage();
            LOG(LOG_INFO, "User session inactivity : timer is stopped !");
        }
    }

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
    void set_time(MonotonicTimePoint end_date)
    {
        this->timer_close = end_date;

        auto now = this->timer_event.event_container().get_current_time();
        if (this->timer_close <= now) {
            this->last_delay = this->last_delay.zero();
        }
        else if (this->timer_close - now <= timers.back()) {
            now = this->next_timeout(now);
        }
        else {
            this->last_delay = this->last_delay.zero();
            now = this->timer_close - timers.back();
        }

        this->timer_event.reset_timeout_or_create_event(
            now, "EndSessionWarning",
            [this](Event& event) {
                if (event.alarm.now >= this->timer_close) {
                    event.garbage = true;
                    throw Error(ERR_SESSION_CLOSE_ENDDATE_REACHED);
                }

                // now+1 for next timer
                auto now = event.alarm.now + 1s;
                event.alarm.reset_timeout(this->next_timeout(now));
            }
        );
    }

    // void reset()
    // {
    //     this->timer_event.garbage();
    //     this->last_delay = this->last_delay.zero();
    // }

    template<class Fn>
    void update_warning(Fn&& fn)
    {
        if (this->last_delay.count()) {
            // ajust minutes
            fn(std::chrono::duration_cast<std::chrono::minutes>(this->last_delay + 30s));
            this->last_delay = this->last_delay.zero();
        }
    }

private:
    static constexpr std::array<MonotonicTimePoint::duration, 4> timers{{
        1min, 5min, 10min, 30min,
    }};

    MonotonicTimePoint next_timeout(MonotonicTimePoint now)
    {
        auto elapsed = this->timer_close - now;
        this->last_delay = elapsed;

        auto timepos = timers.begin();
        if (elapsed <= *timepos) {
            return this->timer_close;
        }

        while (++timepos < timers.end()) {
            if (elapsed <= *timepos) {
                break;
            }
        }
        return this->timer_close - *(timepos-1);
    }

    MonotonicTimePoint timer_close;
    MonotonicTimePoint::duration last_delay;
    EventRef2 timer_event;
};
