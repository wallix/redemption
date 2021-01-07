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


class AsynchronousTaskContainer;

class AsynchronousTask : noncopyable
{
public:
    struct AsynchronousEventContainer
    {
        template<class TimeoutAction>
        void create_event_timeout(
            std::string_view name,
            timeval trigger_time, TimeoutAction&& on_timeout);

        template<class FdAction>
        void create_event_fd_without_timeout(
            std::string_view name,
            int fd, FdAction&& on_fd);

        template<class FdAction, class TimeoutAction>
        void create_event_fd_timeout(
            std::string_view name,
            int fd,
            std::chrono::microseconds grace_delay,
            timeval trigger_time,
            FdAction&& on_fd,
            TimeoutAction&& on_timeout);

    private:
        AsynchronousEventContainer(AsynchronousTaskContainer& async_cont) noexcept
        : async_cont(async_cont)
        {}

        friend class AsynchronousTaskContainer;
        AsynchronousTaskContainer& async_cont;
    };

    virtual ~AsynchronousTask() = default;

    virtual void configure_event(timeval now, AsynchronousEventContainer async_event_container) = 0;

private:
    friend class AsynchronousTaskContainer;
    AsynchronousTask* p_next_task_ = nullptr;
};


struct AsynchronousTaskContainer
{
public:
    explicit AsynchronousTaskContainer(EventContainer & events)
        : events_guard(events)
    {
    }

    void add(std::unique_ptr<AsynchronousTask>&& task)
    {
        if (this->first_task) {
            this->last_task->p_next_task_ = task.release();
            this->last_task = this->last_task->p_next_task_;
        }
        else {
            this->first_task = task.release();
            this->last_task = this->first_task;
            this->first_task->configure_event(
                this->events_guard.get_current_time(),
                AsynchronousTask::AsynchronousEventContainer{*this}
            );
        }
    }

    ~AsynchronousTaskContainer()
    {
        auto* task = this->first_task;
        while (task) {
            delete std::exchange(task, task->p_next_task_);
        }
    }

private:
    void next()
    {
        delete std::exchange(this->first_task, this->first_task->p_next_task_);
        if (this->first_task) {
            this->first_task->configure_event(
                this->events_guard.get_current_time(),
                AsynchronousTask::AsynchronousEventContainer{*this}
            );
        }
        else {
            this->last_task = nullptr;
        }
    }

    AsynchronousTask* first_task = nullptr;
    AsynchronousTask* last_task = nullptr;

    friend class AsynchronousTask::AsynchronousEventContainer;
    EventsGuard events_guard;
};

template<class TimeoutAction>
inline void AsynchronousTask::AsynchronousEventContainer
::create_event_timeout(
    std::string_view name,
    timeval trigger_time, TimeoutAction&& on_timeout)
{
    auto& async_cont = this->async_cont;
    async_cont.events_guard.create_event_timeout(
        name, trigger_time,
        [on_timeout = static_cast<TimeoutAction&&>(on_timeout), &async_cont](Event& e){
            on_timeout(e);
            if (e.garbage) { async_cont.next(); }
        }
    );
}

template<class FdAction>
inline void AsynchronousTask::AsynchronousEventContainer
::create_event_fd_without_timeout(
    std::string_view name,
    int fd, FdAction&& on_fd)
{
    auto& async_cont = this->async_cont;
    async_cont.events_guard.create_event_fd_without_timeout(
        name, fd,
        [on_fd = static_cast<FdAction&&>(on_fd), &async_cont](Event& e){
            on_fd(e);
            if (e.garbage) { async_cont.next(); }
        }
    );
}

template<class FdAction, class TimeoutAction>
inline void AsynchronousTask::AsynchronousEventContainer
::create_event_fd_timeout(
    std::string_view name,
    int fd,
    std::chrono::microseconds grace_delay,
    timeval trigger_time,
    FdAction&& on_fd,
    TimeoutAction&& on_timeout)
{
    auto& async_cont = this->async_cont;
    async_cont.events_guard.create_event_fd_timeout(
        name, fd, grace_delay, trigger_time,
        [on_fd = static_cast<FdAction&&>(on_fd), &async_cont](Event& e){
            on_fd(e);
            if (e.garbage) { async_cont.next(); }
        },
        [on_timeout = static_cast<TimeoutAction&&>(on_timeout), &async_cont](Event& e){
            on_timeout(e);
            if (e.garbage) { async_cont.next(); }
        }
    );
}
