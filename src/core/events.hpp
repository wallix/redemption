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

#include "utils/timeval_ops.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/sugar/noncopyable.hpp"
#include <vector>
#include <string>
#include <functional>
#include <cstdint>
#include <cinttypes>

#include "utils/log.hpp"
#include "core/error.hpp"

class EventContainer;

struct Event
{
    std::string name;
    void const* lifespan_handle = nullptr;
    bool garbage = false;

    struct Trigger
    {
        int fd = -1;
        bool active = false;
        timeval now;
        timeval trigger_time;
        timeval start_time;
        std::chrono::microseconds grace_delay = std::chrono::seconds{0};

        // fd is stored to enabled fd events detection
        // if fd event is triggered timeout is incremented by grace delay
        void set_fd(int fd, std::chrono::microseconds grace_delay) {
            this->fd = fd;
            this->grace_delay = grace_delay;
        }

        // timeout alarm will call on_timeout once when trigger_time is reached
        // the trigger time must be reset with set_timeout
        // if we want to call the alarm again
        void set_timeout(timeval trigger_time) {
          this->active = true;
          this->trigger_time = this->start_time = this->now = trigger_time;
        }

        void reset_timeout(timeval trigger_time) {
          this->active = true;
          this->trigger_time = this->now = trigger_time;
        }

        void stop_alarm(){
            this->active = false;
        }

        bool trigger(timeval now) {
            this->now = now;
            if (not this->active) { return false; }
            if (this->now >= this->trigger_time) {
                this->active = false;
                return true;
            }
            return false;
        }
    } alarm;

    struct Actions
    {
        friend class EventContainer;

    private:
        std::function<void(Event &)> on_timeout = [](Event &){};
        std::function<void(Event &)> on_action = [](Event &){};

    public:
        void exec_timeout(Event & event) {
            this->on_timeout(event);
        }

        void exec_action(Event & event) {
            this->on_action(event);
        }
    } actions;

    Event(std::string name, void const* lifespan)
        : name(name)
        , lifespan_handle(lifespan)
        {
        }

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
        if (not this->reset){
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

private:
    std::uintptr_t id_ = 0;
};


struct EventContainer : noncopyable
{
    std::vector<Event*> queue;

    void get_fds(std::function<void(int fd)> fn)
    {
        for (auto & pevent: this->queue){
            Event & event = *pevent;
            if (not event.garbage and event.alarm.fd != INVALID_SOCKET){
                fn(event.alarm.fd);
            }
        }
    }

    void get_fds_timeouts(std::function<void(timeval tv)> const& fn)
    {
        for (auto & pevent: this->queue){
            Event & event = *pevent;
            if (not event.garbage and event.alarm.fd != INVALID_SOCKET){
                fn(event.alarm.trigger_time);
            }
        }
    }

    void get_timeouts(std::function<void(timeval tv)> const& fn)
    {
        for (auto & pevent: this->queue){
            Event & event = *pevent;
            if (not event.garbage and event.alarm.fd == INVALID_SOCKET){
                fn(event.alarm.trigger_time);
            }
        }
    }

    void execute_events(const timeval tv, std::function<bool(int fd)> const& fn, bool verbose)
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
            if (event.garbage) {
                LOG_IF(verbose, LOG_INFO,
                    "GARBAGE EVENT '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d =========",
                    event.name, EventId(&event).id(),
                    int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
            }

            if (not event.garbage) {
                if (event.alarm.fd != -1 && fn(event.alarm.fd)) {
                    LOG_IF(verbose, LOG_INFO,
                        "FD EVENT TRIGGER '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d",
                        event.name, EventId(&event).id(),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                    event.alarm.set_timeout(tv+event.alarm.grace_delay);
                    event.actions.exec_action(event);
                    continue;
                }
            }

            if (not event.garbage) {
                if (!event.alarm.active){
                    LOG_IF(verbose & 0x20, LOG_INFO,
                        "EXPIRED TIMEOUT EVENT '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d",
                        event.name, EventId(&event).id(),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                }
                if (event.alarm.trigger(tv)){
                    LOG_IF(verbose, LOG_INFO,
                        "TIMEOUT EVENT TRIGGER '%s' (%" EVENT_ID_FORMAT ") timeout=%d now=%d",
                        event.name, EventId(&event).id(),
                        int(event.alarm.trigger_time.tv_sec%1000), int(tv.tv_sec%1000));
                    event.actions.exec_timeout(event);
                }
            }
#undef EVENT_ID_FORMAT
        }
        this->garbage_collector();
    }

    void garbage_collector() {
        for (size_t i = 0; i < this->queue.size() ; i++){
            while ((i < this->queue.size()) && this->queue[i]->garbage){
                delete this->queue[i];
                if (i < this->queue.size() -1){
                    this->queue[i] = this->queue.back();
                }
                this->queue.pop_back();
            }
        }
    }

    void end_of_lifespan(void const* lifespan)
    {
        for (auto & pevent: this->queue){
            Event & event = *pevent;
            if (event.lifespan_handle == lifespan){
                event.garbage = true;
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
            if (not event.garbage and event.alarm.active){
                ultimatum = event.alarm.trigger_time;
                break;
            }
        }

        for (; first != last; ++first){
            Event const& event = **first;
            if (not event.garbage and event.alarm.active){
                ultimatum = std::min(event.alarm.trigger_time, ultimatum);
            }
        }

        return ultimatum;
    }


    EventId erase_event(EventId & event_id)
    {
        if (event_id) {
            for(auto & pevent: this->queue){
                if (EventId(pevent) == event_id){
                    pevent->garbage = true;
                    event_id = EventId();
                    return event_id;
                }
            }
        }
        return event_id;
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

    EventId create_event_timeout(std::string name, void const* lifespan,
        timeval trigger_time,
        std::function<void(Event&)> timeout)
    {
        Event * pevent = new Event(name, lifespan);
        Event & event = *pevent;
        event.alarm.set_timeout(trigger_time);
        event.actions.on_timeout = std::move(timeout);
        this->queue.push_back(pevent);
        return EventId(pevent);
    }

    EventId create_event_fd_timeout(std::string name, void const* lifespan,
        int fd, std::chrono::microseconds grace_delay,
        timeval trigger_time,
        std::function<void(Event&)> on_fd,
        std::function<void(Event&)> on_timeout)
    {
        Event * pevent = new Event(name, lifespan);
        Event & event = *pevent;
        event.alarm.set_fd(fd, grace_delay);
        event.alarm.set_timeout(trigger_time);
        event.actions.on_action = std::move(on_fd);
        event.actions.on_timeout = std::move(on_timeout);
        this->queue.push_back(pevent);
        return EventId(pevent);
    }

    ~EventContainer(){
        // clear every remaining event
       for(auto & pevent: this->queue){
            delete pevent;
        }
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

    EventId create_event_timeout(
        std::string name,
        timeval trigger_time,
        std::function<void(Event&)> timeout)
    {
        return this->events.create_event_timeout(
            std::move(name), this,
            trigger_time, std::move(timeout));
    }

    EventId create_event_fd_timeout(
        std::string name,
        int fd,
        std::chrono::microseconds grace_delay,
        timeval trigger_time,
        std::function<void(Event&)> on_fd,
        std::function<void(Event&)> on_timeout)
    {
        return this->events.create_event_fd_timeout(
            std::move(name), this, fd,
            grace_delay, trigger_time,
            std::move(on_fd), std::move(on_timeout));
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
