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
   Copyright (C) Wallix 2021
   Author(s): Proxy Team
*/

#pragma once

#include "cxx/cxx.hpp"
#include "utils/timebase.hpp"
#include "utils/monotonic_clock.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/sugar/noncopyable.hpp"

#include "utils/log.hpp"

#include <vector>
#include <chrono>
#include <utility>
#include <string_view>
#include <type_traits>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cinttypes>

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
        MonotonicTimePoint now;
        MonotonicTimePoint trigger_time;
        MonotonicTimePoint::duration grace_delay {};

        // fd is stored to enabled fd events detection
        // if fd event is triggered timeout is incremented by grace delay
        void set_fd(int fd, MonotonicTimePoint::duration grace_delay)
        {
            this->fd = fd;
            this->grace_delay = grace_delay;
        }

        void reset_timeout(MonotonicTimePoint trigger_time)
        {
            this->active_timer = true;
            this->trigger_time = trigger_time;
        }

        void reset_timeout(MonotonicTimePoint::duration delay)
        {
            this->reset_timeout(this->now + delay);
        }

        // void stop_alarm()
        // {
        //     this->active_timer = false;
        // }

        bool is_active() const
        {
            return this->active_timer;
        }

        bool trigger(MonotonicTimePoint now)
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
        SigNothrow* custom_destructor = nullptr;
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

namespace detail
{
    struct ProtectedEventContainer;
}

class EventContainer : noncopyable
{
public:
    class Creator;

    [[nodiscard]] TimeBase const& get_time_base() const noexcept
    {
        return this->creator.time_base;
    }

    [[nodiscard]] MonotonicTimePoint get_monotonic_time() const noexcept
    {
        return this->creator.time_base.monotonic_time;
    }

    void end_of_lifespan(void const* lifespan)
    {
        for (auto* pevent: this->creator.queue){
            if (pevent->lifespan_handle == lifespan){
                pevent->garbage = true;
            }
        }
    }

    Creator& event_creator() noexcept
    {
        return this->creator;
    }

    /// expose create_event_* functions
    struct Creator
    {
        template<class TimeoutAction>
        [[nodiscard]] Event& create_event_timeout(
            std::string_view name, void const* lifespan,
            MonotonicTimePoint::duration delay, TimeoutAction&& on_timeout)
        {
            return this->create_event_timeout(
                name, lifespan,
                this->time_base.monotonic_time + delay,
                static_cast<TimeoutAction&&>(on_timeout));
        }

        template<class TimeoutAction>
        [[nodiscard]] Event& create_event_timeout(
            std::string_view name, void const* lifespan,
            MonotonicTimePoint trigger_time, TimeoutAction&& on_timeout)
        {
            Event* pevent = make_event(
                name, lifespan,
                NilFn(),
                static_cast<TimeoutAction&&>(on_timeout));
            pevent->alarm.reset_timeout(trigger_time);
            this->queue.push_back(pevent);
            return *pevent;
        }

        template<class FdAction>
        [[nodiscard]] Event& create_event_fd_without_timeout(
            std::string_view name, void const* lifespan,
            int fd, FdAction&& on_fd)
        {
            Event* pevent = make_event(
                name, lifespan,
                static_cast<FdAction&&>(on_fd),
                NilFn());
            pevent->alarm.fd = fd;
            this->queue.push_back(pevent);
            return *pevent;
        }

        template<class FdAction, class TimeoutAction>
        [[nodiscard]] Event& create_event_fd_timeout(
            std::string_view name, void const* lifespan,
            int fd,
            MonotonicTimePoint::duration delay,
            FdAction&& on_fd,
            TimeoutAction&& on_timeout)
        {
            return this->create_event_fd_timeout(
                name, lifespan, fd, delay,
                this->time_base.monotonic_time + delay,
                static_cast<FdAction&&>(on_fd),
                static_cast<TimeoutAction&&>(on_timeout));
        }

        template<class FdAction, class TimeoutAction>
        [[nodiscard]] Event& create_event_fd_timeout(
            std::string_view name, void const* lifespan,
            int fd,
            MonotonicTimePoint::duration grace_delay,
            MonotonicTimePoint trigger_time,
            FdAction&& on_fd,
            TimeoutAction&& on_timeout)
        {
            Event* pevent = make_event(
                name, lifespan,
                static_cast<FdAction&&>(on_fd),
                static_cast<TimeoutAction&&>(on_timeout));
            pevent->alarm.set_fd(fd, grace_delay);
            pevent->alarm.reset_timeout(trigger_time);
            this->queue.push_back(pevent);
            return *pevent;
        }

    private:
        friend class EventContainer;
        friend class detail::ProtectedEventContainer;

        Creator() = default;

        std::vector<Event*> queue;
        TimeBase time_base {};

        static const int action_tag = 0;
        static const int timeout_tag = 1;

        struct NilFn : noncopyable
        {
            operator Event::Actions::Sig* ();
        };

        template<int Tag, class Fn>
        struct Function
        {
            Fn fn;

            template<class RealEvent>
            Event::Actions::Sig* to_ptr_func(Fn const& /*fn*/)
            {
                return [](Event& e){
                    static_cast<Function&>(static_cast<RealEvent&>(e)).fn(e);
                };
            }
        };

        template<int Tag>
        struct Function<Tag, Event::Actions::Sig*>
        {
            Function(Event::Actions::Sig* /*fn*/) {}
            Function(NilFn const& /*fn*/) {}

            template<class RootEvent>
            Event::Actions::Sig* to_ptr_func(Event::Actions::Sig* f)
            {
                assert(f);
                return f;
            }
        };

        template<class ActionFn, class TimeoutFn>
        struct TupleFunctions
        : Function<action_tag, ActionFn>
        , Function<timeout_tag, TimeoutFn>
        {};

        template<class ActionFn, class TimeoutFn>
        struct RootEvent : Event, TupleFunctions<ActionFn, TimeoutFn>
        {};

        template<class Fn>
        using SimplifyFn = std::conditional_t<
            std::is_convertible_v<Fn, Event::Actions::Sig*>,
            Event::Actions::Sig*,
            Fn
        >;

        template<class ActionFn, class TimeoutFn>
        static Event* make_event(
            std::string_view name, void const* lifespan,
            ActionFn&& on_action, TimeoutFn&& on_timeout)
        {
            using DecayActionFn = SimplifyFn<std::decay_t<ActionFn>>;
            using DecayTimeoutFn = SimplifyFn<std::decay_t<TimeoutFn>>;

            using RealEvent = RootEvent<DecayActionFn, DecayTimeoutFn>;

            void* raw = ::operator new(sizeof(RealEvent) + name.size() + 1u); /* NOLINT */

            auto* data_name = static_cast<char*>(raw) + sizeof(RealEvent);
            memcpy(data_name, name.data(), name.size());
            data_name[name.size()] = 0;

            RealEvent* event = new (raw) RealEvent{
                {lifespan},
                {
                    {static_cast<ActionFn&&>(on_action)},
                    {static_cast<TimeoutFn&&>(on_timeout)},
                }
            };
            event->name = data_name;

            if constexpr (!std::is_same_v<NilFn, ActionFn>) {
                event->actions.on_action
                    = static_cast<Function<action_tag, DecayActionFn>&>(*event)
                    .template to_ptr_func<RealEvent>(on_action);
            }

            if constexpr (!std::is_same_v<NilFn, TimeoutFn>) {
                event->actions.on_timeout
                    = static_cast<Function<timeout_tag, DecayTimeoutFn>&>(*event)
                    .template to_ptr_func<RealEvent>(on_timeout);
            }

            if constexpr (!(
                std::is_trivially_destructible_v<DecayActionFn>
            && std::is_trivially_destructible_v<DecayTimeoutFn>)
            ) {
                static_assert(std::is_nothrow_destructible_v<DecayActionFn>);
                static_assert(std::is_nothrow_destructible_v<DecayTimeoutFn>);
                event->actions.custom_destructor = [](Event& e) noexcept {
                    using Tuple = TupleFunctions<DecayActionFn, DecayTimeoutFn>;
                    static_cast<Tuple&>(static_cast<RealEvent&>(e)).~Tuple();
                };
            }

            return event;
        }

        static void delete_event(Event* e)
        {
            if (e->actions.custom_destructor) {
                e->actions.custom_destructor(*e);
            }
            e->~Event();
            ::operator delete(e);
        }

        void garbage_collector()
        {
            for (size_t i = 0; i < this->queue.size(); i++) {
                if (REDEMPTION_UNLIKELY(this->queue[i]->garbage)) {
                    do {
                        this->delete_event(this->queue[i]);
                        this->queue[i] = this->queue.back();
                        this->queue.pop_back();
                    } while (i < this->queue.size() && this->queue[i]->garbage);
                }
            }
        }

        ~Creator()
        {
            // clear every remaining event
            for(auto* pevent: this->queue){
                this->delete_event(pevent);
            }
        }
    };

private:
    friend class detail::ProtectedEventContainer;

    Creator creator;
};

namespace detail
{
    struct ProtectedEventContainer
    {
        static void garbage_collector(EventContainer& event_container) noexcept
        {
            event_container.creator.garbage_collector();
        }

        static std::vector<Event*> const& get_events(EventContainer const& event_container) noexcept
        {
            return event_container.creator.queue;
        }

        static TimeBase& get_writable_time_base(EventContainer& event_container) noexcept
        {
            return event_container.creator.time_base;
        }
    };
}

class EventManager
{
    EventContainer event_container;

public:
    [[nodiscard]] EventContainer& get_events() noexcept
    {
        return event_container;
    }

    void garbage_collector()
    {
        detail::ProtectedEventContainer::garbage_collector(this->event_container);
    }

    void set_time_base(TimeBase const& time_base) noexcept
    {
        this->get_writable_time_base() = time_base;
    }

    [[nodiscard]] TimeBase const& get_time_base() const noexcept
    {
        return this->event_container.get_time_base();
    }

    [[nodiscard]] TimeBase& get_writable_time_base() noexcept
    {
        return detail::ProtectedEventContainer::get_writable_time_base(this->event_container);
    }

    [[nodiscard]] MonotonicTimePoint get_monotonic_time() const noexcept
    {
        return this->event_container.get_monotonic_time();
    }

    template<class Fn>
    void for_each_fd(Fn&& fn)
    {
        for (auto* pevent : detail::ProtectedEventContainer::get_events(this->event_container)) {
            if (pevent->alarm.fd != INVALID_SOCKET && !pevent->garbage){
                fn(pevent->alarm.fd);
            }
        }
    }

    bool is_empty() const noexcept
    {
        return detail::ProtectedEventContainer::get_events(this->event_container).empty();
    }

    template<class Fn>
    void execute_events(Fn&& fn, bool verbose)
    {
        auto& events = detail::ProtectedEventContainer::get_events(this->event_container);
        auto const tv = this->get_monotonic_time();
        size_t iend = events.size();
        // ignore events created in the loop
        for (size_t i = 0 ; i < iend; ++i){ /*NOLINT*/
            assert(iend <= events.size());
            auto & event = *events[i];

            auto log = [&](char const* cat){
                if (REDEMPTION_UNLIKELY(verbose)) {
                    this->log_event(event, cat);
                }
            };

            if (REDEMPTION_LIKELY(!event.garbage)) {
                if (event.alarm.fd != INVALID_SOCKET && fn(event.alarm.fd)) {
                    log("FD EVENT TRIGGER");
                    event.alarm.reset_timeout(tv+event.alarm.grace_delay);
                    event.actions.exec_action(event);
                }
                else if (event.alarm.trigger(tv)){
                    log("TIMEOUT EVENT TRIGGER");
                    event.actions.exec_timeout(event);
                }
            }
            else {
                log("GARBAGE EVENT");
            }
        }

        this->garbage_collector();
    }

private:
    static void __attribute__((noinline)) log_event(Event const& event, char const* cat) noexcept
    {
        using std::chrono::duration_cast;
        const auto duration = event.alarm.trigger_time.time_since_epoch();
        const auto milliseconds = duration_cast<std::chrono::milliseconds>(duration);
        const auto seconds = duration_cast<std::chrono::seconds>(milliseconds);
        const long long i_seconds = seconds.count();
        const long long i_milliseconds = (milliseconds - seconds).count();
        LOG(LOG_INFO, "%s '%s' (%p) timeout=%lld now=%lld",
            cat, event.name, static_cast<void const*>(&event),
            i_seconds, i_milliseconds);
    }

public:
    // returns MonotonicTimePoint{0, 0} if no alarm is set
    MonotonicTimePoint next_timeout() const
    {
        MonotonicTimePoint ultimatum {};
        auto& events = detail::ProtectedEventContainer::get_events(this->event_container);
        auto first = events.begin();
        auto last = events.end();
        for (; first != last; ++first){
            Event const& event = **first;
            if (event.alarm.is_active() && !event.garbage){
                ultimatum = event.alarm.trigger_time;
                while (++first != last){
                    Event const& event = **first;
                    if (event.alarm.is_active() && !event.garbage){
                        ultimatum = std::min(event.alarm.trigger_time, ultimatum);
                    }
                }
                break;
            }
        }

        return ultimatum;
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

    TimeBase const& get_time_base() const noexcept
    {
        return this->events.get_time_base();
    }

    [[nodiscard]] MonotonicTimePoint get_monotonic_time() const noexcept
    {
        return this->events.get_monotonic_time();
    }

    template<class TimeoutAction>
    Event& create_event_timeout(
        std::string_view name,
        MonotonicTimePoint::duration delay, TimeoutAction&& on_timeout)
    {
        return this->events.event_creator().create_event_timeout(
            name, this, delay,
            static_cast<TimeoutAction&&>(on_timeout)
        );
    }

    template<class TimeoutAction>
    Event& create_event_timeout(
        std::string_view name,
        MonotonicTimePoint trigger_time, TimeoutAction&& on_timeout)
    {
        return this->events.event_creator().create_event_timeout(
            name, this, trigger_time,
            static_cast<TimeoutAction&&>(on_timeout)
        );
    }

    template<class FdAction>
    Event& create_event_fd_without_timeout(
        std::string_view name,
        int fd, FdAction&& on_fd)
    {
        return this->events.event_creator().create_event_fd_without_timeout(
            name, this, fd,
            static_cast<FdAction&&>(on_fd)
        );
    }

    template<class FdAction, class TimeoutAction>
    Event& create_event_fd_timeout(
        std::string_view name,
        int fd,
        MonotonicTimePoint::duration delay,
        FdAction&& on_fd,
        TimeoutAction&& on_timeout)
    {
        return this->events.event_creator().create_event_fd_timeout(
            name, this, fd, delay,
            static_cast<FdAction&&>(on_fd),
            static_cast<TimeoutAction&&>(on_timeout)
        );
    }

    template<class FdAction, class TimeoutAction>
    Event& create_event_fd_timeout(
        std::string_view name,
        int fd,
        MonotonicTimePoint::duration grace_delay,
        MonotonicTimePoint trigger_time,
        FdAction&& on_fd,
        TimeoutAction&& on_timeout)
    {
        return this->events.event_creator().create_event_fd_timeout(
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

    EventContainer& event_container() noexcept
    {
        return this->events;
    }

private:
    EventContainer& events;
};


/**
 * EventRef wrapper that provides a convenient RAII-style mechanism
 * to release events when control leave the scope.
 */
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
        event_->event_ref = this;

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

    bool has_event() const noexcept { return event_ && !event_->garbage; }
    explicit operator bool() const noexcept { return has_event(); }

    Event* get_optional_event() noexcept { return has_event() ? event_ : nullptr; }

    bool operator == (EventRef const& other) const { return event_ == other.event_; }
    bool operator != (EventRef const& other) const { return event_ != other.event_; }

    bool operator == (Event const& event) const { return event_ == &event; }
    bool operator != (Event const& event) const { return event_ != &event; }

    void garbage() noexcept
    {
        if (event_) {
            event_->garbage = true;
            event_->event_ref = nullptr;
            event_ = nullptr;
        }
    }

    bool reset_timeout(MonotonicTimePoint trigger_time) noexcept
    {
        if (has_event()) {
            event_->alarm.reset_timeout(trigger_time);
            return true;
        }
        return false;
    }


    template<class TimeoutFn>
    void reset_timeout_or_create_event(
        MonotonicTimePoint trigger_time,
        EventsGuard& events_guard,
        std::string_view name,
        TimeoutFn&& fn)
    {
        reset_timeout_or_create_event(
            trigger_time,
            events_guard.event_container(),
            name,
            &events_guard,
            static_cast<TimeoutFn&&>(fn));
    }

    template<class TimeoutFn>
    void reset_timeout_or_create_event(
        MonotonicTimePoint::duration delay,
        EventsGuard& events_guard,
        std::string_view name,
        TimeoutFn&& fn)
    {
        reset_timeout_or_create_event(
            events_guard.get_monotonic_time() + delay,
            events_guard.event_container(),
            name,
            &events_guard,
            static_cast<TimeoutFn&&>(fn));
    }

    template<class TimeoutFn>
    void reset_timeout_or_create_event(
        MonotonicTimePoint::duration delay,
        EventContainer& event_container,
        std::string_view name,
        void const* lifespan,
        TimeoutFn&& fn)
    {
        reset_timeout_or_create_event(
            event_container.get_monotonic_time() + delay,
            event_container,
            name,
            lifespan,
            static_cast<TimeoutFn&&>(fn));
    }

    template<class TimeoutFn>
    void reset_timeout_or_create_event(
        MonotonicTimePoint trigger_time,
        EventContainer& event_container,
        std::string_view name,
        void const* lifespan,
        TimeoutFn&& fn)
    {
        if (has_event()) {
            event_->alarm.reset_timeout(trigger_time);
        }
        else {
            auto& new_event = event_container.event_creator().create_event_timeout(
                name,
                lifespan,
                trigger_time,
                static_cast<TimeoutFn&&>(fn)
            );
            if (event_) {
                event_->garbage = true;
                event_->event_ref = nullptr;
            }
            event_ = &new_event;
            event_->event_ref = this;
        }
    }

    char const* name() const
    {
        return has_event() ? event_->name : nullptr;
    }

private:
    friend class Event;
    Event* event_ = nullptr;
};


/**
 * EventRef2 is a EventRef which contains a EventContainer.
 */
struct EventRef2
{
    EventRef2(EventContainer& event_container) noexcept
    : event_container_(event_container)
    {}

    EventRef2(EventContainer& event_container, Event& event) noexcept
    : event_ref_(event)
    , event_container_(event_container)
    {}

    EventRef2(EventRef2&& other) noexcept
    : event_ref_(std::move(other.event_ref_))
    , event_container_(other.event_container_)
    {}

    EventRef2(EventRef2 const&) = delete;
    EventRef2& operator = (EventRef2 const&) = delete;

    EventRef2& operator = (Event& event) noexcept
    {
        event_ref_ = event;
        return *this;
    }

    EventRef2& operator = (EventRef&& other) noexcept
    {
        event_ref_ = std::move(other);
        return *this;
    }

    bool has_event() const noexcept { return event_ref_.has_event(); }
    explicit operator bool() const noexcept { return has_event(); }

    Event* get_optional_event() noexcept { return event_ref_.get_optional_event(); }

    bool operator == (EventRef2 const& other) const { return event_ref_ == other.event_ref_; }
    bool operator != (EventRef2 const& other) const { return event_ref_ != other.event_ref_; }

    bool operator == (Event const& event) const { return event_ref_ == event; }
    bool operator != (Event const& event) const { return event_ref_ != event; }

    void garbage() noexcept
    {
        event_ref_.garbage();
    }

    bool reset_timeout(MonotonicTimePoint trigger_time) noexcept
    {
        return event_ref_.reset_timeout(trigger_time);
    }

    bool reset_timeout(MonotonicTimePoint::duration delay) noexcept
    {
        return event_ref_.reset_timeout(event_container_.get_monotonic_time() + delay);
    }

    template<class TimeoutFn>
    void reset_timeout_or_create_event(
        MonotonicTimePoint trigger_time,
        std::string_view name,
        TimeoutFn&& fn)
    {
        event_ref_.reset_timeout_or_create_event(
            trigger_time,
            event_container_,
            name,
            this,
            static_cast<TimeoutFn&&>(fn));
    }

    template<class TimeoutFn>
    void reset_timeout_or_create_event(
        MonotonicTimePoint::duration delay,
        std::string_view name,
        TimeoutFn&& fn)
    {
        event_ref_.reset_timeout_or_create_event(
            event_container_.get_monotonic_time() + delay,
            event_container_,
            name,
            this,
            static_cast<TimeoutFn&&>(fn));
    }

    char const* name() const
    {
        return event_ref_.name();
    }

    EventContainer const& event_container() const noexcept
    {
        return this->event_container_;
    }

private:
    EventRef event_ref_;
    EventContainer& event_container_;
};

inline Event::~Event()
{
    if (event_ref) {
        event_ref->event_ = nullptr;
    }
}
