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
#include "utils/timebase.hpp"
#include "core/events.hpp"


RED_AUTO_TEST_CASE(TestOneShotTimerEvent)
{
    int counter = 0;

    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);

    EventContainer events;
    events.create_event_timeout("test", nullptr, wakeup, [&counter](Event&){ ++counter; });

    RED_REQUIRE(events.queue.size() == 1u);
    Event& e = *events.queue[0];

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(1)));
    RED_CHECK(counter == 0);
    RED_REQUIRE(events.queue.size() == 1u);
    // but only once
    RED_CHECK(!e.alarm.trigger(wakeup+std::chrono::seconds(2)));
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 1);
    RED_REQUIRE(events.queue.size() == 1u);

    // If I set an alarm in the past it will be triggered immediately
    e.alarm.set_timeout(origin);
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(3)));
    RED_CHECK(counter == 1);
    RED_REQUIRE(events.queue.size() == 1u);
}

RED_AUTO_TEST_CASE(TestPeriodicTimerEvent)
{
    int counter = 0;

    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);

    EventContainer events;
    events.create_event_timeout("test", nullptr, wakeup, [&counter](Event& event){
        event.alarm.reset_timeout(event.alarm.now + std::chrono::seconds{1});
        ++counter;
    });
    Event& e = *events.queue[0];

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    RED_CHECK(counter == 0);
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(1)));
    RED_CHECK(counter == 0);
    e.actions.exec_timeout(e);
    // and again after period, because event reset alarm
    RED_CHECK(e.alarm.trigger(wakeup+std::chrono::seconds(2)));
    RED_CHECK(counter == 1);
}

// on each call to sequencer the curent method is called
// then the sequencer is positionned to the next one
// The sequencer position can also be explicitely reset
// to some arbitrary method in the sequence.

RED_AUTO_TEST_CASE(TestNewEmptySequencer)
{
    timeval origin{79, 0};

    EventContainer events;
    events.create_event_timeout("test", nullptr, origin, Sequencer{false, 0, true, {}});
    Event& e = *events.queue[0];
    e.actions.exec_timeout(e);
    RED_CHECK(e.garbage);
}

RED_AUTO_TEST_CASE(TestNewSimpleSequencer)
{
    int counter = 0;

    Sequencer chain = {false, 0, false, {
        { "first",
            [&counter](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                RED_CHECK(counter == 0);
                counter = 1;
            }
        },
        { "second",
            [&counter](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                RED_CHECK(counter == 1);
                counter = 2;
            }
        },
        { "third",
            [&counter](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                RED_CHECK(counter == 2);
                counter = 3;
            }
        },
        { "fourth",
            [&counter](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                RED_CHECK(counter == 3);
                counter = 4;
            }
        }
    }};

    timeval origin{79, 0};

    EventContainer events;
    events.create_event_timeout("test", nullptr, origin, chain);
    Event& e = *events.queue[0];

    RED_CHECK(counter == 0);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 1);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 2);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 3);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 4);
    RED_CHECK(e.garbage);
}

RED_AUTO_TEST_CASE(TestNewSimpleSequencerNonLinear)
{
    int counter = 0;

    Sequencer chain = {false, 0, true, {
        { "first",
            [&counter](Event&/*event*/,Sequencer&sequencer)
            {
                if (counter == 0){
                    counter = 1;
                    sequencer.next_state("third");
                }
                else {
                    RED_CHECK(counter == 2);
                    counter = 10;
                    sequencer.next_state("fourth");
                }
            }
        },
        { "second",
            [&counter](Event&/*event*/,Sequencer&sequencer)
            {
                RED_CHECK(counter == 3);
                counter = 2;
                sequencer.next_state("first");
            }
        },
        { "third",
            [&counter](Event&/*event*/,Sequencer&sequencer)
            {
                RED_CHECK(counter == 1);
                counter = 3;
                sequencer.next_state("second");
            }
        },
        { "fourth",
            [&counter](Event&/*event*/,Sequencer&/*sequencer*/)
            {
                RED_CHECK(counter == 10);
                counter = 4;
            }
        }
    }};

    timeval origin{79, 0};

    EventContainer events;
    events.create_event_timeout("test", nullptr, origin, chain);
    Event& e = *events.queue[0];

    RED_CHECK(counter == 0);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 1);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 3);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 2);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 10);
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 4);
    RED_CHECK(e.garbage);
}


RED_AUTO_TEST_CASE(TestChangeOfRunningAction)
{
    struct Context {
        EventsGuard events_guard;
        TimeBase & time_base;
        int counter1 = 0;
        int counter2 = 0;

        Context(EventContainer & events, TimeBase & time_base)
            : events_guard(events)
            , time_base(time_base)
        {
            this->events_guard.create_event_timeout(
                "Init Event",
                this->time_base.get_current_time(),
                [this](Event&/*event*/)
                {
                    this->events_guard.create_event_fd_timeout(
                        "Fd Event",
                        1, std::chrono::seconds{300},
                        this->time_base.get_current_time(),
                        [this](Event&/*event*/){ ++this->counter1; },
                        [this](Event&/*event*/){ ++this->counter2; }
                    );
                }
            );
        }
    };

    TimeBase time_base({0,0});
    EventContainer events;

    Context context(events, time_base);
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return false; }, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 0);
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return false; }, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    time_base.set_current_time({3,0});
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return false; }, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    events.execute_events(time_base.get_current_time(), [](int /*fd*/){ return true; }, false);
    RED_CHECK(context.counter1 == 1);
    RED_CHECK(context.counter2 == 1);
}
