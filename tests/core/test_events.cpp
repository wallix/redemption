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

using namespace std::chrono_literals;

const auto nofd_fn = [](int /*fd*/) { return false; };

static std::vector<Event*> const& get_events(EventContainer const& event_container)
{
    return detail::ProtectedEventContainer::get_events(event_container);
}

static std::vector<Event*> const& get_events(EventManager& event_manager)
{
    return get_events(event_manager.get_events());
}

RED_AUTO_TEST_CASE(TestOneShotTimerEvent)
{
    int counter = 0;

    MonotonicTimePoint origin{79s};
    MonotonicTimePoint wakeup = origin + 2s;

    EventContainer events;
    (void)events.event_creator().create_event_timeout(
        "test", nullptr, wakeup, [&counter](Event&){ ++counter; });

    RED_REQUIRE(get_events(events).size() == 1u);
    Event& e = *get_events(events)[0];

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+1s));
    RED_CHECK(counter == 0);
    RED_REQUIRE(get_events(events).size() == 1u);
    // but only once
    RED_CHECK(!e.alarm.trigger(wakeup+2s));
    e.actions.exec_timeout(e);
    RED_CHECK(counter == 1);
    RED_REQUIRE(get_events(events).size() == 1u);

    // If I set an alarm in the past it will be triggered immediately
    e.alarm.reset_timeout(origin);
    RED_CHECK(e.alarm.trigger(wakeup+3s));
    RED_CHECK(counter == 1);
    RED_REQUIRE(get_events(events).size() == 1u);
}

RED_AUTO_TEST_CASE(TestPeriodicTimerEvent)
{
    int counter = 0;

    MonotonicTimePoint origin{79s};
    MonotonicTimePoint wakeup = origin+2s;

    EventContainer events;
    (void)events.event_creator().create_event_timeout(
        "test", nullptr, wakeup, [&counter](Event& event){
            event.alarm.reset_timeout(event.alarm.now + 1s);
            ++counter;
        });
    Event& e = *get_events(events)[0];

    // before time: nothing happens
    RED_CHECK(!e.alarm.trigger(origin));
    RED_CHECK(counter == 0);
    // when it's time of above alarm is triggered
    RED_CHECK(e.alarm.trigger(wakeup+1s));
    RED_CHECK(counter == 0);
    e.actions.exec_timeout(e);
    // and again after period, because event reset alarm
    RED_CHECK(e.alarm.trigger(wakeup+2s));
    RED_CHECK(counter == 1);
}

RED_AUTO_TEST_CASE(TestEventGuard)
{
    EventManager event_manager;
    {
        EventsGuard events_guard(event_manager.get_events());
        events_guard.create_event_timeout("Init Event", MonotonicTimePoint{}, [](Event&/*event*/) {});
        RED_CHECK(get_events(event_manager).size() == 1);
        RED_CHECK(!get_events(event_manager)[0]->garbage);
    }
    RED_CHECK(get_events(event_manager).size() == 1);
    RED_CHECK(get_events(event_manager)[0]->garbage);

    event_manager.garbage_collector();
    RED_CHECK(get_events(event_manager).size() == 0);
}

RED_AUTO_TEST_CASE(TestEventRef)
{
    EventManager event_manager;
    auto& events = event_manager.get_events();

    {
        EventRef ref = events.event_creator().create_event_timeout(
            "Init Event", nullptr, MonotonicTimePoint{}, [](Event&/*event*/) {});
        RED_CHECK(get_events(events).size() == 1);
        RED_CHECK(!get_events(events)[0]->garbage);
    }
    RED_CHECK(get_events(events).size() == 1);
    RED_CHECK(get_events(events)[0]->garbage);

    event_manager.garbage_collector();
    RED_CHECK(get_events(events).size() == 0);

    {
        EventRef ref1 = events.event_creator().create_event_timeout(
            "Init Event", nullptr, MonotonicTimePoint{}, [](Event&/*event*/) {});
        RED_CHECK(ref1.has_event());
        RED_CHECK(get_events(events).size() == 1);
        RED_CHECK(!get_events(events)[0]->garbage);

        ref1 = events.event_creator().create_event_timeout(
            "Init Event", nullptr, MonotonicTimePoint{}, [](Event&/*event*/) {});
        RED_CHECK(get_events(events).size() == 2);
        RED_CHECK(get_events(events)[0]->garbage);
        RED_CHECK(!get_events(events)[1]->garbage);

        EventRef ref2 = events.event_creator().create_event_timeout(
            "Init Event", nullptr, MonotonicTimePoint{}, [](Event&/*event*/) {});
        RED_CHECK(get_events(events).size() == 3);
        RED_CHECK(get_events(events)[0]->garbage);
        RED_CHECK(!get_events(events)[1]->garbage);
        RED_CHECK(!get_events(events)[2]->garbage);

        ref2 = std::move(ref1);
        RED_CHECK(ref2.has_event());
        RED_CHECK(!ref1.has_event());
        RED_CHECK(get_events(events).size() == 3);
        RED_CHECK(get_events(events)[0]->garbage);
        RED_CHECK(!get_events(events)[1]->garbage);
        RED_CHECK(get_events(events)[2]->garbage);

        EventRef ref3 = events.event_creator().create_event_timeout(
            "Init Event", nullptr, MonotonicTimePoint{}, [](Event&/*event*/) {});
        RED_CHECK(ref3.has_event());
        RED_CHECK(!get_events(events)[3]->garbage);
        ref3.garbage();
        RED_CHECK(!ref3.has_event());
        RED_CHECK(get_events(events)[3]->garbage);
    }
    RED_CHECK(get_events(events).size() == 4);
    RED_CHECK(get_events(events)[0]->garbage);
    RED_CHECK(get_events(events)[1]->garbage);
    RED_CHECK(get_events(events)[2]->garbage);
    RED_CHECK(get_events(events)[3]->garbage);

    event_manager.garbage_collector();
    RED_CHECK(get_events(events).size() == 0);

    {
        EventRef ref1 = events.event_creator().create_event_timeout(
            "Init Event", nullptr, MonotonicTimePoint{}, [](Event& e) { e.garbage = true; });

        RED_CHECK(ref1.has_event());
        event_manager.execute_events(nofd_fn, false);
        RED_CHECK(!ref1.has_event());
        RED_CHECK(get_events(events).size() == 0);
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
                this->events_guard.get_monotonic_time(),
                [this](Event&/*event*/)
                {
                    this->events_guard.create_event_fd_timeout(
                        "Fd Event",
                        1, 300s,
                        this->events_guard.get_monotonic_time(),
                        [this](Event&/*event*/){ ++this->counter1; },
                        [this](Event&/*event*/){ ++this->counter2; }
                    );
                }
            );
        }
    };

    EventManager event_manager;
    auto& events = event_manager.get_events();

    Context context(events);
    event_manager.execute_events(nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 0);
    event_manager.execute_events(nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    event_manager.get_writable_time_base().monotonic_time = MonotonicTimePoint{3s};
    event_manager.execute_events(nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    event_manager.execute_events([](int /*fd*/){ return true; }, false);
    RED_CHECK(context.counter1 == 1);
    RED_CHECK(context.counter2 == 1);
    event_manager.get_writable_time_base().monotonic_time = MonotonicTimePoint{303s};
    event_manager.execute_events(nofd_fn, false);
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
                MonotonicTimePoint{},
                [this, r = std::make_unique<Ref>(counter1)](Event&/*event*/){
                    ++this->counter1;
                },
                [this, r = std::make_unique<Ref>(counter2)](Event&/*event*/){
                    ++this->counter2;
                }
            );
        }
    };

    EventManager event_manager;
    auto& events = event_manager.get_events();

    Context context(events);
    event_manager.get_writable_time_base().monotonic_time = MonotonicTimePoint{300s};
    event_manager.execute_events(nofd_fn, false);
    RED_CHECK(context.counter1 == 0);
    RED_CHECK(context.counter2 == 1);
    context.events_guard.end_of_lifespan();
    event_manager.garbage_collector();
    RED_CHECK(context.counter1 == 0x10);
    RED_CHECK(context.counter2 == 0x11);
}
