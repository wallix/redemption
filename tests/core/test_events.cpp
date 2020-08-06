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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "utils/sugar/unique_fd.hpp"
#include "utils/log.hpp"
#include "utils/timebase.hpp"

#include <string>

#include "core/events.hpp"

RED_AUTO_TEST_CASE(TestOneShotTimerEvent)
{
    std::string s;

    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);

    // the second parameter of event is the event context
    // it should be an object whose lifecycle match event functions lifecycle
    Event e("Event", nullptr);
    e.alarm.set_timeout(wakeup);
    e.actions.set_timeout_function([&s](Event&){ s += "Event Triggered"; });

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(1)));
    // but only once
    RED_CHECK(!e.alarm.trigger(wakeup+std::chrono::seconds(2)));
    e.actions.exec_timeout(e);
    RED_CHECK(s == std::string("Event Triggered"));

    // If I set an alarm in the past it will be triggered immediately
    e.alarm.set_timeout(origin);
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(3)));
}

RED_AUTO_TEST_CASE(TestPeriodicTimerEvent)
{
    std::string s;

    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);

    // the second parameter of event is the event context
    // it should be an object whose lifecycle match event functions lifecycle
    Event e("Event", nullptr);
    e.alarm.set_timeout(wakeup);
    e.actions.set_timeout_function([&s](Event&event){
        event.alarm.reset_timeout(event.alarm.now + std::chrono::seconds{1});
        s += "Event Triggered";
    });

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(1)));
    e.actions.exec_timeout(e);
    // and again after period, because event reset alarm
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(2)));
}


RED_AUTO_TEST_CASE(TestEventContainer)
{
    std::string s;
    EventContainer events;
    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);
    // the second parameter of event is the event context
    // it should be an object whose lifecycle match event functions lifecycle
    Event * pevent = new Event("Event", nullptr);
    Event & event = * pevent;
    event.actions.set_timeout_function([&s](Event&){ s += "Event Triggered"; });
    event.alarm.set_timeout(wakeup);
    events.add(pevent);

    auto t = origin;
    for (auto & pevent: events.queue){
        Event & event = *pevent;
        RED_CHECK(!event.alarm.trigger(t));
    }
    t = t + std::chrono::seconds(1);
    for (auto & pevent: events.queue){
        Event & event = *pevent;
        RED_CHECK(!event.alarm.trigger(t));
    }
    t = t + std::chrono::seconds(1);
    for (auto & pevent: events.queue){
        Event & event = *pevent;
        if (event.alarm.trigger(t)){
            event.actions.exec_timeout(event);
            RED_CHECK(s == std::string("Event Triggered"));
        }
        else {
            RED_CHECK(false);
        }
    }

    t = t + std::chrono::seconds(1);
    for (auto & pevent: events.queue){
        Event & event = *pevent;
        RED_CHECK(!event.alarm.trigger(t));
    }
}


// on each call to sequencer the curent method is called
// then the sequencer is positionned to the next one
// The sequencer position can also be explicitely reset
// to some arbitrary method in the sequence.


RED_AUTO_TEST_CASE(TestNewEmptySequencer)
{
    Sequencer chain = {false, 0, true, {}};
    Event e("Chain", nullptr);
    e.actions.set_timeout_function(chain);
    e.actions.exec_timeout(e);
    RED_CHECK(e.garbage == true);
}


RED_AUTO_TEST_CASE(TestNewSimpleSequencer)
{
    struct Context {
        size_t counter = 0;
    } context;
    Sequencer chain = {false, 0, false, {
        { "first",
            [&context](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                context.counter = 1;
            }
        },
        { "second",
            [&context](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                context.counter = 2;
            }
        },
        { "third",
            [&context](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                context.counter = 3;
            }
        },
        { "fourth",
            [&context](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                context.counter = 4;
            }
        }
    }};
    Event e("Chain", nullptr);
    e.actions.set_timeout_function(chain);
    RED_CHECK(context.counter == 0);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 1);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 2);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 3);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 4);
    RED_CHECK(e.garbage == true);
}

RED_AUTO_TEST_CASE(TestNewSimpleSequencerNonLinear)
{
    struct Context {
        size_t counter = 0;
    } context;

    Sequencer chain = {false, 0, true, {
        { "first",
            [&context](Event&/*event*/,Sequencer&sequencer)
            {
                if (context.counter == 0){
                    context.counter = 1;
                    sequencer.next_state("third");
                }
                else {
                    context.counter = 10;
                    sequencer.next_state("fourth");
                }
            }
        },
        { "second",
            [&context](Event&/*event*/,Sequencer&sequencer)
            {
                context.counter = 2;
                sequencer.next_state("first");
            }
        },
        { "third",
            [&context](Event&/*event*/,Sequencer&sequencer)
            {
                context.counter = 3;
                sequencer.next_state("second");
            }
        },
        { "fourth",
            [&context](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                context.counter = 4;
            }
        }
    }};
    Event e("Chain", nullptr);
    e.actions.set_timeout_function(chain);
    RED_CHECK(context.counter == 0);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 1);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 3);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 2);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 10);
    e.actions.exec_timeout(e);
    RED_CHECK(context.counter == 4);
    RED_CHECK(e.garbage == true);
}


RED_AUTO_TEST_CASE(TestChangeOfRunningAction)
{

    struct Base {
        virtual void action() = 0;
        virtual ~Base() {}
    };

    struct Context : public Base {
        EventContainer & events;
        TimeBase & time_base;

        Context(EventContainer & events, TimeBase & time_base) : events(events), time_base(time_base)
        {
            this->events.create_event_timeout(
                "Init Event", this,
                this->time_base.get_current_time(),
                [this](Event&event)
                {
                    LOG(LOG_INFO, "Execute Event");
                    // Following fd timeouts
                    event.rename("VNC Fd Event");
                    event.alarm.set_fd(1, std::chrono::seconds{300});
                    event.alarm.set_timeout(this->time_base.get_current_time());
                    event.actions.set_timeout_function([](Event&/*event*/){LOG(LOG_INFO, "Timeout");});
                    event.actions.set_action_function([this](Event&/*event*/){ this->action();});
                });
        }

        ~Context() {
            this->events.end_of_lifespan(this);
        }

        void action() override {
            LOG(LOG_INFO, "Action");
            return;
        }
    };

    TimeBase time_base({0,0});
    EventContainer events;

    Context context(events, time_base);
    LOG(LOG_INFO, "Will Execute Event");
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return false; }, 2);
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return false; }, 2);
    time_base.set_current_time({3,0});
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return false; }, 2);
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return true; }, 2);
}

RED_AUTO_TEST_CASE(TestChangeOfRunningAction2)
{

    struct Event {
        struct Data {
            int val = 0;
            void set_data(int val) {this->val = val; }
        } data;

        struct Actions {
            std::function<void(Event &)> action1 = [](Event &){};
            std::function<void(Event &)> future_action1 = [](Event &){};
            std::function<void(Event &)> action2 = [](Event &){};
        } actions;
    };

    struct Object {
        int val = 0;
    };

    struct Base {
        virtual void action() = 0;
        virtual ~Base() {}
    };

    struct Context : public Base {
        Event & event;
        Object & val;

        Context(Event & event, Object & val) : event(event), val(val)
        {
            event.data.set_data(this->val.val);
            event.actions.action1 = [this](Event&event)
            {
                event.data.set_data(this->val.val);
                event.actions.future_action1 = [](Event&/*event*/){puts("action1");};
                event.actions.action2 = [this](Event&/*event*/){ this->action();};
            };
        }
        void action() override { puts("action2"); return; }
    };

    Object val;
    Event event;
    Context context(event, val);
    event.actions.action1(event);
    event.actions.action1 = std::move(event.actions.future_action1);
    event.actions.action2(event);
}

