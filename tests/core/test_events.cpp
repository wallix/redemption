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

#include <string>

std::string side_effect;

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
    e.actions.on_timeout = [&s](Event&){ s += "Event Triggered"; };

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(1)));
    // but only once
    RED_CHECK(!e.alarm.trigger(wakeup+std::chrono::seconds(2)));
    e.exec_timeout();
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
    e.alarm.set_period(std::chrono::seconds{1});
    e.actions.on_timeout = [&s](Event&){ s += "Event Triggered"; };

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(1)));
    // but only once
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(2)));
}


RED_AUTO_TEST_CASE(TestEventContainer)
{
    std::string s;
    EventContainer event_container;
    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);
    // the second parameter of event is the event context
    // it should be an object whose lifecycle match event functions lifecycle
    Event e("Event", nullptr);
    e.actions.on_timeout = [&s](Event&){ s += "Event Triggered"; };
    e.alarm.set_timeout(wakeup);
    event_container.push_back(std::move(e));

    auto t = origin;
    for (auto & event: event_container){
        RED_CHECK(!event.alarm.trigger(t));
    }
    t = t + std::chrono::seconds(1);
    for (auto & event: event_container){
        RED_CHECK(!event.alarm.trigger(t));
    }
    t = t + std::chrono::seconds(1);
    for (auto & event: event_container){
        if (event.alarm.trigger(t)){
            event.exec_timeout();
            RED_CHECK(s == std::string("Event Triggered"));
        }
        else {
            RED_CHECK(false);
        }
    }

    t = t + std::chrono::seconds(1);
    for (auto & event: event_container){
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
    e.actions.on_timeout = std::move(chain);
    e.exec_timeout();
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
    e.actions.on_timeout = std::move(chain);
    RED_CHECK(context.counter == 0);
    e.exec_timeout();
    RED_CHECK(context.counter == 1);
    e.exec_timeout();
    RED_CHECK(context.counter == 2);
    e.exec_timeout();
    RED_CHECK(context.counter == 3);
    e.exec_timeout();
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
    e.actions.on_timeout = std::move(chain);
    RED_CHECK(context.counter == 0);
    e.exec_timeout();
    RED_CHECK(context.counter == 1);
    e.exec_timeout();
    RED_CHECK(context.counter == 3);
    e.exec_timeout();
    RED_CHECK(context.counter == 2);
    e.exec_timeout();
    RED_CHECK(context.counter == 10);
    e.exec_timeout();
    RED_CHECK(context.counter == 4);
    RED_CHECK(e.garbage == true);
}

