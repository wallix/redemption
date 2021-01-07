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

const auto nofd_fn = [](int /*fd*/) { return false; };

RED_AUTO_TEST_CASE(TestOneShotTimerEvent)
{
    int counter = 0;

    timeval origin{79, 0};
    timeval wakeup = origin+std::chrono::seconds(2);

    EventContainer events;
    (void)events.create_event_timeout("test", nullptr, wakeup, [&counter](Event&){ ++counter; });

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
    (void)events.create_event_timeout("test", nullptr, wakeup, [&counter](Event& event){
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

RED_AUTO_TEST_CASE(TestEventGuard)
{
    EventContainer events;
    {
        EventsGuard events_guard(events);
        events_guard.create_event_timeout("Init Event", {0, 0}, [](Event&/*event*/) {});
        RED_CHECK(events.queue.size() == 1);
        RED_CHECK(!events.queue[0]->garbage);
    }
    RED_CHECK(events.queue.size() == 1);
    RED_CHECK(events.queue[0]->garbage);

    events.garbage_collector();
    RED_CHECK(events.queue.size() == 0);
}

RED_AUTO_TEST_CASE(TestEventRef)
{
    EventContainer events;
    {
        EventRef ref = events.create_event_timeout("Init Event", nullptr, {0, 0}, [](Event&/*event*/) {});
        RED_CHECK(events.queue.size() == 1);
        RED_CHECK(!events.queue[0]->garbage);
    }
    RED_CHECK(events.queue.size() == 1);
    RED_CHECK(events.queue[0]->garbage);

    events.garbage_collector();
    RED_CHECK(events.queue.size() == 0);

    {
        EventRef ref1 = events.create_event_timeout("Init Event", nullptr, {0, 0}, [](Event&/*event*/) {});
        RED_CHECK(ref1.has_event());
        RED_CHECK(events.queue.size() == 1);
        RED_CHECK(!events.queue[0]->garbage);

        ref1 = events.create_event_timeout("Init Event", nullptr, {0, 0}, [](Event&/*event*/) {});
        RED_CHECK(events.queue.size() == 2);
        RED_CHECK(events.queue[0]->garbage);
        RED_CHECK(!events.queue[1]->garbage);

        EventRef ref2 = events.create_event_timeout("Init Event", nullptr, {0, 0}, [](Event&/*event*/) {});
        RED_CHECK(events.queue.size() == 3);
        RED_CHECK(events.queue[0]->garbage);
        RED_CHECK(!events.queue[1]->garbage);
        RED_CHECK(!events.queue[2]->garbage);

        ref2 = std::move(ref1);
        RED_CHECK(ref2.has_event());
        RED_CHECK(!ref1.has_event());
        RED_CHECK(events.queue.size() == 3);
        RED_CHECK(events.queue[0]->garbage);
        RED_CHECK(!events.queue[1]->garbage);
        RED_CHECK(events.queue[2]->garbage);

        EventRef ref3 = events.create_event_timeout("Init Event", nullptr, {0, 0}, [](Event&/*event*/) {});
        RED_CHECK(ref3.has_event());
        RED_CHECK(!events.queue[3]->garbage);
        ref3.garbage();
        RED_CHECK(!ref3.has_event());
        RED_CHECK(events.queue[3]->garbage);
    }
    RED_CHECK(events.queue.size() == 4);
    RED_CHECK(events.queue[0]->garbage);
    RED_CHECK(events.queue[1]->garbage);
    RED_CHECK(events.queue[2]->garbage);
    RED_CHECK(events.queue[3]->garbage);

    events.garbage_collector();
    RED_CHECK(events.queue.size() == 0);

    {
        EventRef ref1 = events.create_event_timeout("Init Event", nullptr, {0, 0}, [](Event& e) { e.garbage = true; });

        RED_CHECK(ref1.has_event());
        events.execute_events({}, nofd_fn, false);
        RED_CHECK(!ref1.has_event());
        RED_CHECK(events.queue.size() == 0);
    }
}

RED_AUTO_TEST_CASE(TestChangeOfRunningAction)
{
    struct Context {
        EventsGuard events_guard;
        int counter1 = 0;
        int counter2 = 0;

        Context(EventContainer & events)
            : events_guard(events)
        {
            this->events_guard.create_event_timeout(
                "Init Event",
                this->events_guard.get_current_time(),
                [this](Event&/*event*/)
                {
                    this->events_guard.create_event_fd_timeout(
                        "Fd Event",
                        1, std::chrono::seconds{300},
                        this->events_guard.get_current_time(),
                        [this](Event&/*event*/){ ++this->counter1; },
                        [this](Event&/*event*/){ ++this->counter2; }
                    );
                }
            );
        }
    };

    EventContainer events;

    Context context(events);
    events.execute_events(events.get_current_time(), nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 0);
    events.execute_events(events.get_current_time(), nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    events.set_current_time({3,0});
    events.execute_events(events.get_current_time(), nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    events.execute_events(events.get_current_time(), [](int /*fd*/){ return true; }, false);
    RED_CHECK(context.counter1 == 1);
    RED_CHECK(context.counter2 == 1);
    events.set_current_time({303,0});
    events.execute_events(events.get_current_time(), nofd_fn, false);
    RED_CHECK(context.counter1 == 1);
    RED_CHECK(context.counter2 == 2);
}

RED_AUTO_TEST_CASE(TestNontrivialEvent)
{
    struct Context {
        EventsGuard events_guard;
        int counter1 = 0;
        int counter2 = 0;

        struct Ref
        {
            int& i;
            Ref(int& i) : i(i) {}
            ~Ref() { i |= 0x10; }
        };

        Context(EventContainer & events)
            : events_guard(events)
        {
            this->events_guard.create_event_fd_timeout(
                "Fd Event",
                1, std::chrono::seconds{300},
                {0, 0},
                [this, r = std::make_unique<Ref>(counter1)](Event&/*event*/){
                    ++this->counter1;
                },
                [this, r = std::make_unique<Ref>(counter2)](Event&/*event*/){
                    ++this->counter2;
                }
            );
        }
    };

    EventContainer events;

    Context context(events);
    events.execute_events({300,0}, nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    context.events_guard.end_of_lifespan();
    events.garbage_collector();
    RED_CHECK(context.counter1 == 0x10);
    RED_CHECK(context.counter2 == 0x11);
}
