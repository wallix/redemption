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

#include <utils/timeval_ops.hpp>
#include <string>
#include <functional>

#include "utils/log.hpp"
#include "core/error.hpp"

struct Event {
    // TODO: the management of this counter may be moved to EventContainer later
    // no need to use a static lifespan, EventContainer lifespan would be fine
    // in this cas the id would be attributed when adding event to container
    static int counter;
    // event id, 0 means no event
    // used to identify some event in event queue.
    int id = 0;

    std::string name;
    void * lifespan_handle = nullptr;
    bool garbage = false;
    bool teardown = false;

    struct Trigger {
        int fd = -1;
        bool active = false;
        timeval now;
        timeval trigger_time;
        timeval start_time;
        std::chrono::microseconds period = std::chrono::seconds{0};
        std::chrono::microseconds grace_delay = std::chrono::seconds{0};

        // fd is stored to enabled fd events detection
        // if fd event is triggered timeout is incremented by grace delay
        void set_fd(int fd, std::chrono::microseconds grace_delay) {
            this->fd = fd;
            this->grace_delay = grace_delay;
        }

        // periodic alarm will call on_period every period interval,
        // starting at start_time.
        void set_period(std::chrono::microseconds period) {
            this->period = period;
        }

        // timeout alarm will call on_timeout once when trigger_time is reached
        // the trigger time must be reset with set_timeout
        // if we want to call the alarm again
        void set_timeout(timeval trigger_time) {
          this->active = true;
          this->trigger_time = this->start_time = this->now = trigger_time;
        }

        void stop_alarm(){
            this->active = false;
        }

        bool trigger(timeval now) {
            this->now = now;
//            if (this->garbage){ return false; }
            if (not this->active) { return false; }
            if (this->now >= this->trigger_time) {
                if (this->period.count() == 0){
                    // one time alarm
                    this->active = false;
                }
                // periodic alarm : if some ticks were lost
                // the periodic alarm won't try to catch up
                // it will reset the interval based on current time
                this->trigger_time = now + this->period;
                return true;
            }
            return false;
        }
    } alarm;

    struct Actions {
        std::function<void(Event &)> on_timeout = [](Event &){};
        std::function<void(Event &)> on_action = [](Event &){};
        std::function<void(Event &)> on_teardown = [](Event &){};
    } actions;

    Event(std::string name, void * lifespan)
        : id(Event::counter++)
        , name(name)
        , lifespan_handle(lifespan)
        {
            LOG(LOG_INFO, "Creation of new event (%d): %s", this->id, this->name.c_str());
        }

    void rename(const std::string string)
    {
        this->name = string;
    }

    void exec_timeout() { this->actions.on_timeout(*this);}
    void exec_action() { this->actions.on_action(*this);}
    void exec_teardown() { this->actions.on_teardown(*this);}

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
            LOG_IF(this->verbose, LOG_INFO, "%s :=> on_event: Sequence Terminated",
                event.name);
            event.garbage = true;
            return;
        }
        try {
            LOG_IF(this->verbose, LOG_INFO, "%s :=> on_event: %s",
                event.name, sequence[this->current].label);
            sequence[this->current].action(event,*this);
        }
        catch(Error & error){
            LOG_IF(this->verbose, LOG_INFO, "%s :=> on_event: Sequence terminated on Exception %s",
                event.name, error.errmsg());
            event.garbage = true;
            throw;
        }
        catch(...){
            LOG_IF(this->verbose, LOG_INFO, "%s :=> on_event: Sequence terminated on Exception",
                event.name);
            event.garbage = true;
            throw;
        }
        if (not this->reset){
            this->current++;
            if (this->current >= this->sequence.size()){
                LOG_IF(this->verbose, LOG_INFO, "%s :=> on_event: Sequence Terminated On Last Item",
                    event.name);
                    event.garbage = true;
                return;
            }
        }
        this->reset = false;
//        LOG_IF(this->verbose, LOG_INFO, "on_event: %s - Next sequence item: %s",
//                event.name, this->sequence[this->current].label);
    }

    void next_state(std::string_view label){
        LOG(LOG_ERR, "Moving to out of Sequence item %.*s", int(label.size()), label.data());
        for(size_t i = 0 ; i < this->sequence.size() ; i++){
            if (this->sequence[i].label == label){
                this->reset = true;
                this->current = i;
                return;
            }
        }
        LOG(LOG_ERR, "Sequence item %.*s not found", int(label.size()), label.data());
    }
};


using EventContainer = std::vector<Event>;

inline void end_of_lifespan(EventContainer & events, void * lifespan)
{
    for (auto & e: events){
        if (e.lifespan_handle == lifespan){
            e.garbage = true;
        }
    }
}


inline void exec_teardowns(EventContainer & events) {
        for (size_t i = 0; i < events.size() ; i++){
            if(events[i].teardown){
                events[i].exec_teardown();
                events[i].teardown = false;
                events[i].garbage = true;
            }
        }
}


inline void garbage_collector(EventContainer & events) {
        for (size_t i = 0; i < events.size() ; i++){
            while ((i < events.size()) && events[i].garbage){
                LOG(LOG_INFO, "Removing Event: %s", events[i].name);
                if (i < events.size() -1){
                    events[i] = std::move(events.back());
                }
                events.pop_back();
            }
        }
}

// returns timeval{0, 0} if no alarm is set
inline timeval next_timeout(EventContainer & events)
{
   timeval ultimatum = {0, 0};
   for(auto &event: events){
        if (not (event.garbage or event.teardown) and event.alarm.active){
            ultimatum = event.alarm.trigger_time;
            break;
        }
   }
   for(auto &event: events){
        if (not (event.garbage or event.teardown) and event.alarm.active){
            ultimatum = std::min(event.alarm.trigger_time, ultimatum);
        }
    }
    return ultimatum;
}

inline void execute_events(EventContainer & events, const timeval tv, const std::function<bool(int fd)> & fn)
{
    if (events.size() > 0){
//        LOG(LOG_INFO, "=== Execute Events Loop ===");
    }

    for (size_t i = 0 ; i < events.size(); i++){
        auto & event = events[i];
//        if (event.alarm.fd != -1){
//            LOG(LOG_INFO, "now=%d:%d Examining Fd Event (%d): %s (%d:%d) fd=%d TriggerTime=%d:%d grace_delay=%ld %s%s%s",
//                int(tv.tv_sec%1000),int(tv.tv_usec)/1000,
//                event.id, event.name.c_str(),
//                int(event.alarm.start_time.tv_sec%1000),int(event.alarm.start_time.tv_usec)/1000,
//                event.alarm.fd,
//                int(event.alarm.trigger_time.tv_sec%1000),int(event.alarm.trigger_time.tv_usec)/1000,
//                event.alarm.grace_delay.count(),
//                event.alarm.active?"active ":"",
//                event.teardown?"teardown ":"",
//                event.garbage?"garbage":""
//                );
//        }
//        else {
//            LOG(LOG_INFO, "now=%d:%d Examining Timeout Event (%d): %s (%d:%d) TriggerTime=%d:%d %s%s%s",
//                int(tv.tv_sec%1000),int(tv.tv_usec)/1000,
//                event.id, event.name.c_str(),
//                int(event.alarm.start_time.tv_sec%1000),int(event.alarm.start_time.tv_usec)/1000,
//                int(event.alarm.trigger_time.tv_sec%1000),int(event.alarm.trigger_time.tv_usec)/1000,
//                event.alarm.active?"active ":"",
//                event.teardown?"teardown ":"",
//                event.garbage?"garbage":""
//                );
//        }

        if (not (event.garbage or event.teardown)) {
            if (event.alarm.fd != -1 && fn(event.alarm.fd)) {
                event.alarm.set_timeout(tv+event.alarm.grace_delay);
//                LOG(LOG_INFO, "Triggering Fd Event: %s", event.name);
                event.exec_action();
                // Not testing timeout now as fd event was triggered
                continue;
            }
        }
        if (not (event.garbage or event.teardown)) {
            if (event.alarm.trigger(tv)){
//                LOG(LOG_INFO, "Triggering Timeout Event: %s", event.name);
                event.exec_timeout();
            }
        }
    }
    exec_teardowns(events);
    garbage_collector(events);
}

