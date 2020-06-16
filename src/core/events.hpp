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

struct Event {
    struct Trigger {
        bool garbage = false;
        void * lifespan_handle = nullptr;
        bool active = false;
        timeval now;
        timeval trigger_time;
        timeval start_time;
        std::chrono::microseconds period = std::chrono::seconds{0};

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
            if (this->garbage){ return false; }
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
        // default action is do nothing
        std::function<void(Event &)> on_timeout = [](Event &){};
    } actions;

    Event() {}

    void exec_timeout() { this->actions.on_timeout(*this);}
};


using EventContainer = std::vector<Event>;

inline void end_of_lifespan(EventContainer & events, void * lifespan)
{
    for (auto & e: events){
        if (e.alarm.lifespan_handle == lifespan){
            e.alarm.garbage = true;
        }
    }
}
