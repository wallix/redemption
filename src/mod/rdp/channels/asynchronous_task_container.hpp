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
  Author(s): Wallix Team
*/

#pragma once

#include "utils/timebase.hpp"
#include "core/events.hpp"

#include <memory>
#include <deque>


class AsynchronousTaskContainer;

class AsynchronousTask : noncopyable
{
public:
    struct AsyncEventData
    {
        struct TimerEvent
        {
            timeval trigger_time;
            std::function<void(Event&)> on_timeout;
        };

        struct FdEvent
        {
            int fd;
            std::chrono::microseconds grace_delay;
            std::function<void(Event&)> on_fd;
        };

        std::string name;
        TimerEvent timer_event;
        FdEvent fd_event{-1, {}, {}};
    };

    virtual ~AsynchronousTask() = default;
    virtual AsyncEventData configure_event(timeval now) = 0;
};


struct AsynchronousTaskContainer
{
public:
    explicit AsynchronousTaskContainer(TimeBase& time_base, EventContainer & events)
        : time_base(time_base)
        , events_guard(events)
    {
    }

    void add(std::unique_ptr<AsynchronousTask>&& task)
    {
        this->tasks.emplace_back(std::move(task));

        if (this->tasks.size() == 1u) {
            this->next();
        }
    }

private:
    std::function<void(Event&)> to_sync_func(std::function<void(Event&)>& f)
    {
        return [f = std::move(f), this](Event & e){
            f(e);
            if (e.garbage) {
                this->tasks.pop_front();
                this->next();
            }
        };
    }

    void next()
    {
        if (!this->tasks.empty()) {
            auto async_ev = this->tasks.front()->configure_event(this->time_base.get_current_time());
            if (async_ev.fd_event.fd == -1) {
                this->events_guard.create_event_timeout(
                    std::move(async_ev.name),
                    async_ev.timer_event.trigger_time,
                    this->to_sync_func(async_ev.timer_event.on_timeout));
            }
            else {
                this->events_guard.create_event_fd_timeout(
                    std::move(async_ev.name),
                    async_ev.fd_event.fd,
                    async_ev.fd_event.grace_delay,
                    async_ev.timer_event.trigger_time,
                    this->to_sync_func(async_ev.fd_event.on_fd),
                    this->to_sync_func(async_ev.timer_event.on_timeout));
            }
        }
    }

    std::deque<std::unique_ptr<AsynchronousTask>> tasks;

    TimeBase& time_base;
    EventsGuard events_guard;
};
