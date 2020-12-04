/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2020
   Author(s): Christophe Grosjean
*/

#pragma once

#include "cxx/cxx.hpp"
#include "utils/timeval_ops.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/sugar/noncopyable.hpp"

#include "utils/log.hpp"

#include <vector>
#include <chrono>
#include <string_view>
#include <type_traits>

#include <cstdint>
#include <cstring>
#include <cinttypes>
#include <utility>

class EventContainer;
class EventRef;

struct Event
{
    bool garbage = false;

    struct Trigger
    {
    private:
        friend class EventContainer;

        bool active_timer = false;

    public:
        int fd = INVALID_SOCKET;
        timeval now;
        timeval trigger_time;
        timeval start_time;
        std::chrono::microseconds grace_delay = std::chrono::seconds{0};

        // fd is stored to enabled fd events detection
        // if fd event is triggered timeout is incremented by grace delay
        void set_fd(int fd, std::chrono::microseconds grace_delay)
        {
            this->fd = fd;
            this->grace_delay = grace_delay;
        }

        // timeout alarm will call on_timeout once when trigger_time is reached
        // the trigger time must be reset with set_timeout
        // if we want to call the alarm again
        void set_timeout(timeval trigger_time)
        {
            this->active_timer = true;
            this->trigger_time = this->start_time = trigger_time;
        }

        void reset_timeout(timeval trigger_time)
        {
            this->active_timer = true;
            this->trigger_time = trigger_time;
        }

        // void stop_alarm()
        // {
        //     this->active_timer = false;
        // }

        bool trigger(timeval now)
        {
            if (this->active_timer && now >= this->trigger_time) {
                this->now = now;
                this->active_timer = false;
                return true;
            }
            return false;
        }
    } alarm;

    struct Actions
    {
        inline void exec_timeout(Event& e) { this->on_timeout(e); }
        inline void exec_action(Event& e) { this->on_action(e); }

    private:
        friend class EventContainer;

        using Sig = void(Event&);
        using SigNothrow = void(Event&) noexcept;

        Sig* on_timeout = [](Event &){};
        Sig* on_action = [](Event &){};
        SigNothrow* on_delete = nullptr;
    } actions;

    void const* const lifespan_handle;

    char const* name = "No name";

private:
    friend class EventContainer;

    Event(void const* lifespan_handle) noexcept
    : lifespan_handle(lifespan_handle)
    {}

    ~Event();

    Event(Event const&) = delete;
    Event& operator=(Event const&) = delete;

    friend class EventRef;
    EventRef* event_ref = nullptr;
};

class EventsGuard;

// event id, 0 means no event
// used to identify some event in event queue.
struct EventRef
{
    EventRef() = default;

    EventRef(Event& event) noexcept
    : event_(&event)
    {
        assert(event_);
        event_->event_ref = this;
    }

    EventRef(EventRef&& other) noexcept
    : event_(std::exchange(other.event_, nullptr))
    {
        if (event_) {
            event_->event_ref = this;
        }
    }

    EventRef(EventRef const&) = delete;
    EventRef& operator = (EventRef const&) = delete;

    EventRef& operator = (Event& event) noexcept
    {
        assert(&event != event_);

        if (event_) {
            event_->garbage = true;
            event_->event_ref = nullptr;
        }
        event_ = &event;
        if (event_) {
            event_->event_ref = this;
        }

        return *this;
    }

    EventRef& operator = (EventRef&& other) noexcept
    {
        assert(other.event_ != event_);

        if (event_) {
            event_->garbage = true;
            event_->event_ref = nullptr;
        }
        event_ = std::exchange(other.event_, nullptr);
        if (event_) {
            event_->event_ref = this;
        }

        return *this;
    }

    ~EventRef()
    {
        if (event_) {
            event_->garbage = true;
            event_->event_ref = nullptr;
        }
    }

    bool has_event() const { return event_ && !event_->garbage; }
    explicit operator bool() const { return has_event(); }

    bool operator == (EventRef const& other) const { return event_ == other.event_; }
    bool operator != (EventRef const& other) const { return event_ != other.event_; }

    bool operator == (Event const& event) const { return event_ == &event; }
    bool operator != (Event const& event) const { return event_ != &event; }

    void garbage()
    {
        if (event_) {
            event_->garbage = true;
            event_->event_ref = nullptr;
            event_ = nullptr;
        }
    }

    bool reset_timeout(timeval trigger_time)
    {
        if (has_event()) {
            event_->alarm.reset_timeout(trigger_time);
            return true;
        }
        return false;
    }

    bool set_timeout(timeval trigger_time)
    {
        if (has_event()) {
            event_->alarm.set_timeout(trigger_time);
            return true;
        }
        return false;
    }

    char const* name() const
    {
        return has_event() ? event_->name : nullptr;
    }

private:
    friend class Event;
    Event* event_ = nullptr;
};

inline Event::~Event()
{
    if (event_ref) {
        event_ref->event_ = nullptr;
    }
}

struct EventContainer : noncopyable
{
    std::vector<Event*> queue;

    template<class Fn>
    void get_fds(Fn&& fn)
    {
        for (auto* pevent: this->queue){
            if (pevent->alarm.fd != INVALID_SOCKET && !pevent->garbage){
                fn(pevent->alarm.fd);
            }
        }
    }

    template<class Fn>
    void get_fds_timeouts(Fn&& fn)
    {
        for (auto* pevent : this->queue){
            if (pevent->alarm.fd != INVALID_SOCKET && !pevent->garbage){
                fn(pevent->alarm.trigger_time);
            }
        }
    }

    template<class Fn>
    void execute_events(const timeval tv, Fn&& fn, bool verbose)
    {
        size_t iend = this->queue.size();
        // ignore events created in the loop
        for (size_t i = 0 ; i < iend; ++i){ /*NOLINT*/
            assert(iend <= this->queue.size());
            auto & event = *this->queue[i];
            using VoidP = void const*;
            if (!event.garbage) {
                if (event.alarm.fd != INVALID_SOCKET && fn(event.alarm.fd)) {
                    LOG_IF(verbose, LOG_INFO,
                        "FD EVENT TRIGGER '%s' (%p) timeout=%d now=%d",
                        event.name, VoidP(&event),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                    event.alarm.set_timeout(tv+event.alarm.grace_delay);
                    event.actions.on_action(event);
                }
                else {
                    LOG_IF(event.alarm.active_timer && verbose, LOG_INFO,
                        "EXPIRED TIMEOUT EVENT '%s' (%p) timeout=%d now=%d",
                        event.name, VoidP(&event),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));

                    if (event.alarm.trigger(tv)){
                        LOG_IF(verbose, LOG_INFO,
                            "TIMEOUT EVENT TRIGGER '%s' (%p) timeout=%d now=%d",
                            event.name, VoidP(&event),
                            int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                        event.actions.on_timeout(event);
                    }
                }
            }
            else {
                LOG_IF(verbose, LOG_INFO,
                    "GARBAGE EVENT '%s' (%p) timeout=%d now=%d =========",
                    event.name, VoidP(&event),
                    int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
            }
        }

        this->garbage_collector();
    }

    void garbage_collector()
    {
        for (size_t i = 0; i < this->queue.size(); i++){
            if (REDEMPTION_UNLIKELY(this->queue[i]->garbage)) {
                do {
                    this->delete_event(this->queue[i]);
                    this->queue[i] = this->queue.back();
                    this->queue.pop_back();
                } while (i < this->queue.size() && this->queue[i]->garbage);
            }
        }
    }

    void end_of_lifespan(void const* lifespan)
    {
        for (auto* pevent: this->queue){
            if (pevent->lifespan_handle == lifespan){
                pevent->garbage = true;
            }
        }
    }

    // returns timeval{0, 0} if no alarm is set
    timeval next_timeout() const
    {
        timeval ultimatum = {0, 0};
        auto first = this->queue.begin();
        auto last = this->queue.end();
        for (; first != last; ++first){
            Event const& event = **first;
            if (event.alarm.active_timer && !event.garbage){
                ultimatum = event.alarm.trigger_time;
                break;
            }
        }

        for (; first != last; ++first){
            Event const& event = **first;
            if (event.alarm.active_timer && !event.garbage){
                ultimatum = std::min(event.alarm.trigger_time, ultimatum);
            }
        }

        return ultimatum;
    }

    template<class TimeoutAction>
    [[nodiscard]] Event& create_event_timeout(
        std::string_view name, void const* lifespan,
        timeval trigger_time, TimeoutAction&& on_timeout)
    {
        Event * pevent;

        if constexpr (std::is_convertible_v<TimeoutAction, Event::Actions::Sig*>) {
            pevent = make_event<Event>(name, lifespan);
            pevent->actions.on_timeout = on_timeout;
        }
        else {
            auto* timer_event = this->create_nontrivial_event(
                name, lifespan, static_cast<TimeoutAction&&>(on_timeout));
            timer_event->actions.on_timeout = [](Event& e){
                static_cast<decltype(*timer_event)&>(e).event_action(e);
            };
            pevent = timer_event;
        }

        pevent->alarm.set_timeout(trigger_time);
        this->queue.push_back(pevent);
        return *pevent;
    }

    template<class FdAction>
    [[nodiscard]] Event& create_event_fd_without_timeout(
        std::string_view name, void const* lifespan,
        int fd, FdAction&& on_fd)
    {
        Event * pevent;

        if constexpr (std::is_convertible_v<FdAction, Event::Actions::Sig*>) {
            pevent = make_event<Event>(name, lifespan);
            pevent->actions.on_action = on_fd;
        }
        else {
            auto* fd_event = this->create_nontrivial_event(
                name, lifespan, static_cast<FdAction&&>(on_fd));
            fd_event->actions.on_timeout = [](Event& e){
                static_cast<decltype(*fd_event)&>(e).event_action(e);
            };
            pevent = fd_event;
        }

        pevent->alarm.fd = fd;
        this->queue.push_back(pevent);
        return *pevent;
    }

    template<class FdAction, class TimeoutAction>
    [[nodiscard]] Event& create_event_fd_timeout(
        std::string_view name, void const* lifespan,
        int fd,
        std::chrono::microseconds grace_delay,
        timeval trigger_time,
        FdAction&& on_fd,
        TimeoutAction&& on_timeout)
    {
        Event * pevent;

        using is_on_fd_ptr = std::is_convertible<FdAction, Event::Actions::Sig*>;
        using is_on_timeout_ptr = std::is_convertible<TimeoutAction, Event::Actions::Sig*>;

        if constexpr (is_on_fd_ptr::value && is_on_timeout_ptr::value) {
            pevent = make_event<Event>(name, lifespan);
            pevent->actions.on_action = on_fd;
            pevent->actions.on_timeout = on_timeout;
        }
        else if constexpr (is_on_fd_ptr::value) {
            auto* event = this->create_nontrivial_event(
                name, lifespan, static_cast<TimeoutAction&&>(on_timeout));
            event->actions.on_timeout = [](Event& e){
                static_cast<decltype(*event)&>(e).event_action(e);
            };
            pevent = event;
            pevent->actions.on_action = on_fd;
        }
        else if constexpr (is_on_timeout_ptr::value) {
            auto* event = this->create_nontrivial_event(
                name, lifespan, static_cast<FdAction&&>(on_fd));
            event->actions.on_action = [](Event& e){
                static_cast<decltype(*event)&>(e).event_action(e);
            };
            pevent = event;
            pevent->actions.on_timeout = on_timeout;
        }
        else {
            using UnrefFdAction = std::remove_reference_t<FdAction>;
            using UnrefTimeoutAction = std::remove_reference_t<TimeoutAction>;

            struct FdTimeoutEvent : Event
            {
                UnrefFdAction fd_action;
                UnrefTimeoutAction timeout_action;
            };

            auto* event = make_event<FdTimeoutEvent>(
                name,
                lifespan,
                static_cast<UnrefFdAction&&>(on_fd),
                static_cast<UnrefTimeoutAction&&>(on_timeout)
            );

            using ActEvent = decltype(*event);
            event->actions.on_action = [](Event& e){
                static_cast<ActEvent&>(e).fd_action(e);
            };
            event->actions.on_timeout = [](Event& e){
                static_cast<ActEvent&>(e).timeout_action(e);
            };

            static_assert(std::is_nothrow_destructible_v<UnrefFdAction>);
            static_assert(std::is_nothrow_destructible_v<UnrefTimeoutAction>);

            if constexpr (std::is_trivially_destructible_v<UnrefFdAction>) {
                if constexpr (!std::is_trivially_destructible_v<UnrefTimeoutAction>) {
                    event->actions.on_delete = [](Event& e) noexcept {
                        static_cast<ActEvent&>(e).timeout_action.~UnrefTimeoutAction();
                    };
                }
            }
            else if constexpr (std::is_trivially_destructible_v<UnrefTimeoutAction>) {
                event->actions.on_delete = [](Event& e) noexcept {
                    static_cast<ActEvent&>(e).fd_action.~UnrefFdAction();
                };
            }
            else {
                event->actions.on_delete = [](Event& e) noexcept {
                    static_cast<ActEvent&>(e).fd_action.~UnrefFdAction();
                    static_cast<ActEvent&>(e).timeout_action.~UnrefTimeoutAction();
                };
            }

            pevent = event;
        }

        pevent->alarm.set_fd(fd, grace_delay);
        pevent->alarm.set_timeout(trigger_time);
        this->queue.push_back(pevent);
        return *pevent;
    }

    ~EventContainer()
    {
        // clear every remaining event
        for(auto* pevent: this->queue){
            this->delete_event(pevent);
        }
    }

private:
    template<class Action>
    auto* create_nontrivial_event(std::string_view name, void const* lifespan, Action&& act)
    {
        using UnrefAction = std::remove_reference_t<Action>;

        struct ActionEvent : Event
        {
            UnrefAction event_action;
        };

        auto* event = make_event<ActionEvent>(name, lifespan, static_cast<Action&&>(act));

        if constexpr (!std::is_trivially_destructible_v<UnrefAction>) {
            static_assert(std::is_nothrow_destructible_v<UnrefAction>);
            event->actions.on_delete = [](Event& e) noexcept {
                static_cast<ActionEvent&>(e).event_action.~UnrefAction();
            };
        }

        return event;
    }

    template<class Event, class... Ts>
    static Event* make_event(std::string_view name, Ts&&... xs)
    {
        void* raw = ::operator new(sizeof(Event) + name.size()); /* NOLINT */
        auto* data_name = static_cast<char*>(raw) + sizeof(Event);
        memcpy(data_name, name.data(), name.size());
        Event* event = new (raw) Event{static_cast<Ts&&>(xs)...};
        event->name = data_name;
        return event;
    }

    void delete_event(Event* e)
    {
        if (e->actions.on_delete) {
            e->actions.on_delete(*e);
        }
        e->~Event();
        ::operator delete(e);
    }
};

/**
 * EventContainer wrapper that provides a convenient RAII-style mechanism
 * to release events when control leave the scope.
 */
struct EventsGuard : private noncopyable
{
    EventsGuard(EventContainer& events) noexcept
    : events(events)
    {}

    template<class TimeoutAction>
    Event& create_event_timeout(
        std::string_view name,
        timeval trigger_time, TimeoutAction&& on_timeout)
    {
        return this->events.create_event_timeout(
            name, this, trigger_time,
            static_cast<TimeoutAction&&>(on_timeout)
        );
    }

    template<class FdAction>
    Event& create_event_fd_without_timeout(
        std::string_view name,
        int fd, FdAction&& on_fd)
    {
        return this->events.create_event_fd_without_timeout(
            name, this, fd,
            static_cast<FdAction&&>(on_fd)
        );
    }

    template<class FdAction, class TimeoutAction>
    Event& create_event_fd_timeout(
        std::string_view name,
        int fd,
        std::chrono::microseconds grace_delay,
        timeval trigger_time,
        FdAction&& on_fd,
        TimeoutAction&& on_timeout)
    {
        return this->events.create_event_fd_timeout(
            name, this, fd,
            grace_delay, trigger_time,
            static_cast<FdAction&&>(on_fd),
            static_cast<TimeoutAction&&>(on_timeout)
        );
    }

    void end_of_lifespan()
    {
        this->events.end_of_lifespan(this);
    }

    ~EventsGuard()
    {
        this->end_of_lifespan();
    }

    EventContainer& event_container()
    {
        return this->events;
    }

private:
    EventContainer& events;
};
