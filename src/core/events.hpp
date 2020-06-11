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
        bool active = false;
        timeval now;
        timeval trigger_time;
        
        void set_timeout_alarm(timeval trigger_time) {
          this->active = true;
          this->trigger_time = this->now = trigger_time;
        }

        bool trigger(timeval now) {
            this->now = now;
            if (not active) { return false; }
            if (this->now >= this->trigger_time) {
                // one time alarm
                this->active = false;
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

