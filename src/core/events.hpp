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
#include "core/error.hpp"

#include <vector>
#include <chrono>
#include <functional>
#include <string_view>
#include <type_traits>

#include <cstdint>
#include <cinttypes>

class EventContainer;

struct Event
{
    Event(void const* lifespan_handle) noexcept
    : lifespan_handle(lifespan_handle)
    {}

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
};

struct Sequencer {
    struct Item {
        std::string label;
        std::function<void(Event&event,Sequencer&sequencer)> action;
    };
    bool reset = false;
    size_t current = 0;
    int verbose = true;
    std::vector<Item> sequence;
    void operator()(Event & event){
        this->reset = false;
        if (this->current >= this->sequence.size()){
            LOG_IF(this->verbose, LOG_INFO, "%s :=> sequencer_event: Sequence Terminated",
                event.name);
            event.garbage = true;
            return;
        }
        try {
            LOG_IF(this->verbose, LOG_INFO, "%s :=> sequencer_event: %s", event.name, sequence[this->current].label);
            sequence[this->current].action(event,*this);
        }
        catch(Error & error){
            LOG_IF(this->verbose, LOG_INFO, "%s :=> sequencer_event: Sequence terminated on Exception %s",
                event.name, error.errmsg());
            event.garbage = true;
            throw;
        }
        catch(...){
            LOG_IF(this->verbose, LOG_INFO, "%s :=> sequencer_event: Sequence terminated on Exception",
                event.name);
            event.garbage = true;
            throw;
        }
        if (!this->reset){
            this->current++;
            if (this->current >= this->sequence.size()){
                LOG_IF(this->verbose, LOG_INFO, "%s :=> sequencer_event: Sequence Terminated On Last Item",
                    event.name);
                    event.garbage = true;
                return;
            }
        }
        this->reset = false;
    }

    void next_state(std::string_view label){
        for(size_t i = 0 ; i < this->sequence.size() ; i++){
            if (this->sequence[i].label == label){
                this->reset = true;
                this->current = i;
                return;
            }
        }
        LOG(LOG_ERR, "Sequence item %.*s not found : ending sequencer", int(label.size()), label.data());
        assert(false);
        this->current = this->sequence.size();
    }
};

class EventsGuard;

// event id, 0 means no event
// used to identify some event in event queue.
struct EventId /*: noncopyable_but_movable*/
{
    EventId() = default;

    explicit EventId(Event* p) : id_(reinterpret_cast<std::uintptr_t>(p)) {}

    std::uintptr_t id() const { return id_; }

    explicit operator bool () const { return id_; }

    bool operator == (EventId const& other) const { return id_ == other.id_; }
    bool operator != (EventId const& other) const { return id_ != other.id_; }

    bool erase_from(EventContainer& events);
    bool erase_from(EventsGuard& event_guard);

private:
    std::uintptr_t id_ = 0;
};


struct EventContainer : noncopyable
{
    friend class EventId;

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
#ifdef __EMSCRIPTEN__
# define EVENT_ID_FORMAT "lX"
#else
# define EVENT_ID_FORMAT PRIXPTR
#endif
            if (!event.garbage) {
                if (event.alarm.fd != INVALID_SOCKET && fn(event.alarm.fd)) {
                    LOG_IF(verbose, LOG_INFO,
                        "FD EVENT TRIGGER '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d",
                        event.name, EventId(&event).id(),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                    event.alarm.set_timeout(tv+event.alarm.grace_delay);
                    event.actions.on_action(event);
                }
                else {
                    LOG_IF(event.alarm.active_timer && verbose, LOG_INFO,
                        "EXPIRED TIMEOUT EVENT '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d",
                        event.name, EventId(&event).id(),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));

                    if (event.alarm.trigger(tv)){
                        LOG_IF(verbose, LOG_INFO,
                            "TIMEOUT EVENT TRIGGER '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d",
                            event.name, EventId(&event).id(),
                            int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                        event.actions.on_timeout(event);
                    }
                }
            }
            else {
                LOG_IF(verbose, LOG_INFO,
                    "GARBAGE EVENT '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d =========",
                    event.name, EventId(&event).id(),
                    int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
            }
#undef EVENT_ID_FORMAT
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

    void reset_timeout(const timeval trigger_time, const EventId event_id)
    {
        for(auto & pevent: this->queue){
            if (EventId(pevent) == event_id){
                pevent->alarm.set_timeout(trigger_time);
                return ;
            }
        }
    }

    template<class TimeoutAction>
    EventId create_event_timeout(
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
        return EventId(pevent);
    }

    template<class FdAction>
    EventId create_event_fd_without_timeout(
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
        return EventId(pevent);
    }

    template<class FdAction, class TimeoutAction>
    EventId create_event_fd_timeout(
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
                Event{lifespan},
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
        return EventId(pevent);
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

        auto* event = make_event<ActionEvent>(name, Event{lifespan}, static_cast<Action&&>(act));

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
        static_assert(std::is_trivially_destructible_v<Event>);
        ::operator delete(e);
    }
};

inline bool EventId::erase_from(EventContainer& events)
{
    if (id_) {
        for(auto* pevent : events.queue){
            if (EventId(pevent).id_ == id_){
                pevent->garbage = true;
                id_ = 0;
                return true;
            }
        }
    }

    return false;
}

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
    EventId create_event_timeout(
        std::string_view name,
        timeval trigger_time, TimeoutAction&& on_timeout)
    {
        return this->events.create_event_timeout(
            name, this, trigger_time,
            static_cast<TimeoutAction&&>(on_timeout)
        );
    }

    template<class FdAction>
    EventId create_event_fd_without_timeout(
        std::string_view name,
        int fd, FdAction&& on_fd)
    {
        return this->events.create_event_fd_without_timeout(
            name, this, fd,
            static_cast<FdAction&&>(on_fd)
        );
    }

    template<class FdAction, class TimeoutAction>
    EventId create_event_fd_timeout(
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

inline bool EventId::erase_from(EventsGuard& event_guard)
{
    return this->erase_from(event_guard.event_container());
}
