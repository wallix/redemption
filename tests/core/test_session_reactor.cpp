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

#include "core/session_reactor.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/log.hpp"

#include <string>

RED_TEST_DELEGATE_PRINT_ENUM(jln::R);

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

RED_AUTO_TEST_CASE(TestSimpleTimer)
{
    LOG(LOG_INFO, "TestSimpleTimer");
    TimeBase time_base(timeval{1591190078, 222});
    TimerContainer timer_events_;
    /* global */ side_effect = "";

    auto && a = timer_events_.create_timer_executor(time_base)
    .set_notify_delete(
        [](jln::NotifyDeleteType){
            /* How can I get the timestamp of call to delete */
            side_effect += "Delete Timer at ???\n";
        })
    .set_delay(std::chrono::seconds(1))
    .on_action(
        [](jln::TimerContext ctx){
            side_effect += "Action Timer at "+std::to_string(ctx.get_current_time().tv_sec)+"\n";
            return ctx.terminate();
        });
    // ctx.ready()
    // ctx.ready_to(fn)

    RED_CHECK(side_effect == "");
    timer_events_.exec_timer(time_base.get_current_time());
    RED_CHECK(side_effect == "");

    time_base.increment_sec(1);
    timer_events_.exec_timer(time_base.get_current_time());
    RED_CHECK(side_effect == std::string("Action Timer at 1591190079\nDelete Timer at ???\n"));

    time_base.increment_sec(1);
    timer_events_.exec_timer(time_base.get_current_time());
    RED_CHECK(side_effect == std::string("Action Timer at 1591190079\nDelete Timer at ???\n"));
}

RED_AUTO_TEST_CASE(TestSimpleTimerLambdaCaptureContext)
{
    LOG(LOG_INFO, "TestSimpleTimer");
    TimeBase time_base(timeval{1591190078, 222});
    TimerContainer timer_events_;
    std::string lambda_captured = "";

    auto && a = timer_events_.create_timer_executor(time_base)
    .set_notify_delete(
        [](jln::NotifyDeleteType){
            /* I can get the time_stamp through lambda capture */
//            lambda_captured += "Delete Timer at "+std::to_string(time_base.get_current_time().tv_sec)+"\n";
        })
    .set_delay(std::chrono::seconds(1))
    .on_action(
        [&lambda_captured](jln::TimerContext ctx){
            lambda_captured += "Action Timer at "+std::to_string(ctx.get_current_time().tv_sec)+"\n";
            return ctx.terminate();
        });
    // ctx.ready()
    // ctx.ready_to(fn)

    RED_CHECK(lambda_captured == "");
    timer_events_.exec_timer(time_base.get_current_time());
    RED_CHECK(lambda_captured == "");

    time_base.increment_sec(1);
    timer_events_.exec_timer(time_base.get_current_time());
    RED_CHECK(lambda_captured == std::string("Action Timer at 1591190079\n"));

    time_base.increment_sec(1);
    timer_events_.exec_timer(time_base.get_current_time());
    RED_CHECK(lambda_captured == std::string("Action Timer at 1591190079\n"));
}


RED_AUTO_TEST_CASE(TestSimpleTimer1)
{
    LOG(LOG_INFO, "TestSimpleTimer");
    TimeBase time_base(timeval{1591190078, 222});
    TimerContainer timer_events_;
    std::string s;

    auto && a = timer_events_.create_timer_executor(time_base)
    .set_notify_delete(
        [](jln::NotifyDeleteType){
            LOG(LOG_INFO, "Delete timer TestSimpleTimerOneShot");
        })
    .set_delay(std::chrono::seconds(1))
    .on_action(
        [](jln::TimerContext ctx){
            LOG(LOG_INFO, "Callback timer TestSimpleTimerOneShot");
            return ctx.terminate();
        });
    // ctx.ready()
    // ctx.ready_to(fn)

    LOG(LOG_INFO, "TestSimpleTimerOneShot exec_timer1 %s", s);
    timer_events_.exec_timer(time_base.get_current_time());
    time_base.increment_sec(1);
    LOG(LOG_INFO, "TestSimpleTimerOneShot exec_timer2 %s", s);
    timer_events_.exec_timer(time_base.get_current_time());
    time_base.increment_sec(1);
    LOG(LOG_INFO, "TestSimpleTimerOneShot exec_timer3 %s", s);
    timer_events_.exec_timer(time_base.get_current_time());
}


RED_AUTO_TEST_CASE(TestSimpleTimerOneShot)
{
    LOG(LOG_INFO, "TestSimpleTimer");
    using Dt = jln::NotifyDeleteType;
    TimeBase time_base(timeval{1591190078, 222});
    TimerContainer timer_events_;
    std::string s;

    auto && a = timer_events_.create_timer_executor(time_base, std::ref(s))
    .set_notify_delete([](Dt, std::string& s){ s += "d1\n"; })
    .set_delay(std::chrono::seconds(1))
    .on_action(jln::one_shot([](std::string& s){LOG(LOG_INFO, "Callback timer"); s += "timer1\n";}));

    LOG(LOG_INFO, "TestSimpleTimer exec_timer1");
    timer_events_.exec_timer(time_base.get_current_time());
    time_base.increment_sec(1);
    LOG(LOG_INFO, "TestSimpleTimer exec_timer2");
    timer_events_.exec_timer(time_base.get_current_time());
    time_base.increment_sec(1);
    LOG(LOG_INFO, "TestSimpleTimer exec_timer3");
    timer_events_.exec_timer(time_base.get_current_time());
}

RED_AUTO_TEST_CASE(TestSequencer)
{
    using R = jln::R;
    struct Ctx {} ctx;
    using Array = std::array<int, 5>;
    Array a{};
    auto sequencer = jln::sequencer(
        [&a](Ctx){ a[0] = 1; return R::Next; },
        [&a](Ctx){ a[1] = 1; return R::Next; },
        [&a](Ctx){ a[2] = 1; return R::Next; },
        [&a](Ctx){ a[3] = 1; return R::Next; },
        [&a](Ctx){ a[4] = 1; return R::Next; }
    );
    RED_CHECK(sequencer(ctx) == jln::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 0, 0, 0, 0}}));
    RED_CHECK(sequencer(ctx) == jln::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 0, 0, 0}}));
    RED_CHECK(sequencer(ctx) == jln::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 1, 0, 0}}));
    RED_CHECK(sequencer(ctx) == jln::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 1, 1, 0}}));
    RED_CHECK(sequencer(ctx) == jln::R::Next);  RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 1, 1, 1}}));
}

constexpr auto fd_is_set = [](int /*fd*/, auto& /*e*/){ return true; };

RED_AUTO_TEST_CASE(TestTimeBaseTimer)
{
    using Ptr = jln::SharedPtr;
    using Dt = jln::NotifyDeleteType;
    TimeBase time_base(timeval{10, 222});
    TimerContainer timer_events_;

    RED_CHECK_EQ(time_base.get_current_time().tv_sec, 10);
    RED_CHECK_EQ(time_base.get_current_time().tv_usec, 222);

    std::string s;

    RED_CHECK(true);

    Ptr timer1 = timer_events_
    .create_timer_executor(time_base, std::ref(s))
    .set_notify_delete([](Dt, std::string& s){
        s += "d1\n";
    })
    .set_delay(std::chrono::seconds(1))
    .on_action(jln::one_shot([](std::string& s){
        s += "timer1\n";
    }));

    Ptr timer2 = timer_events_
    .create_timer_executor(time_base, std::ref(s))
    .set_notify_delete([](Dt, std::string& s){
        s += "d2\n";
    })
    .set_delay(std::chrono::seconds(2))
    .on_action(jln::always_ready([](std::string& s){
        s += "timer2\n";
    }));

    Ptr timer3 = timer_events_
    .create_timer_executor(time_base, std::ref(s), 'a')
    .set_notify_delete([](Dt, std::string& s, char){
        s += "d3\n";
    })
    .set_delay(std::chrono::seconds(1))
    .on_action([](auto ctx, std::string& s, char& c){
        s += "timer3\n";
        return c++ == 'd' ? ctx.terminate() : ctx.ready();
    });

    Ptr timer4 = timer_events_
        .create_timer_executor(time_base,
    std::ref(s))
        .set_time({16, 0})
        .on_action(jln::one_shot([](std::string& s){
            s += "timer4\n";
        }));

    auto end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);

    RED_CHECK_EQ(s, "");

    // set_current_time + execute timers, to simulate times flying
    time_base.set_current_time({11, 222});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\n");
    RED_CHECK(!timer1);
    RED_CHECK(bool(timer2));

    time_base.set_current_time({13, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\n");
    RED_CHECK(!timer1);
    RED_CHECK(bool(timer2));

    time_base.set_current_time({14, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\n");
    RED_CHECK(bool(timer2));

    time_base.set_current_time({15, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\n");
    RED_CHECK(bool(timer2));

    timer2.reset();
    RED_CHECK(!timer2);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\n");

    time_base.set_current_time({16, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);

    time_base.set_current_time({16, 0});

    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\ntimer4\n");
}


RED_AUTO_TEST_CASE(TestTimeBaseSequence)
{
    TimeBase time_base({0,0});
    TimerContainer events_;

    std::string s;

    auto end_tv = time_base.get_current_time();

    using namespace jln::literals;
    using jln::value;

    auto trace = [](auto name){
        return [](auto ctx, std::string& s){
            s += decltype(name){}.c_str();
            // or
            // if constexpr (ctx.is_final_sequence()) return ctx.ready();
            // else return ctx.sequence_next();
            return ctx.next();
        };
    };

    TimerContainer::Ptr event = events_.create_timer_executor(time_base, std::ref(s))
    .set_delay(1ms)
    .on_action(jln::sequencer(
        trace("a"_s),
        trace("b"_s),
        trace("c"_s),
        trace("d"_s)
    ));


    time_base.set_current_time(timeval{1, 2000});
    end_tv = time_base.get_current_time();
    events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "a");
    time_base.set_current_time(timeval{1, 3000});
    end_tv = time_base.get_current_time();
    events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "ab");
    time_base.set_current_time(timeval{1, 4000});
    end_tv = time_base.get_current_time();
    events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "abc");
    time_base.set_current_time(timeval{1, 5000});
    end_tv = time_base.get_current_time();
    events_.exec_timer(end_tv);
    RED_CHECK_EQ(s, "abcd");
    s.clear();

//    auto trace2 = [](auto f){
//        return [](auto ctx, std::string& s){
//            s += ctx.sequence_name();
//            return jln::make_lambda<decltype(f)>()(ctx);
//        };
//    };

//    time_base.set_current_time(timeval{1, 6000});
//    end_tv = time_base.get_current_time();
//    events_.exec_timer(end_tv);
//    RED_CHECK_EQ(s, "a");
//    time_base.set_current_time(timeval{1, 7000});
//    end_tv = time_base.get_current_time();
//    events_.exec_timer(end_tv);
//    RED_CHECK_EQ(s, "ab");
//    time_base.set_current_time(timeval{1, 8000});
//    end_tv = time_base.get_current_time();
//    events_.exec_timer(end_tv);
//    RED_CHECK_EQ(s, "abd");
//    time_base.set_current_time(timeval{1, 9000});
//    end_tv = time_base.get_current_time();
//    events_.exec_timer(end_tv);
//    RED_CHECK_EQ(s, "abdce");
}

//RED_AUTO_TEST_CASE(TestTimeBaseDeleter)
//{
//    class S;

//    std::vector<std::unique_ptr<S>> v;
//    auto f = [&v](S&, jln::NotifyDeleteType t){
//        RED_CHECK_EQ(v.size(), 1);
//        if (t == jln::NotifyDeleteType::DeleteByAction) {
//            v.clear();
//        }
//    };

//    using F = decltype(f);

//    struct S
//    {
//        EventPtr ptr;

//        void foo(TimeBase& time_base, TopEventContainer & events_, F f)
//        {
//            this->ptr = events_.create_action_executor(time_base, std::ref(*this), f)
//            .set_notify_delete([](jln::NotifyDeleteType d, S& self, F f){ f(self, d); })
//            .on_action([](auto ctx, S&, F){
//                return ctx.terminate();
//            });
//        }
//    };

//    TimeBase time_base;

//    v.emplace_back(std::make_unique<S>());
//    v.back()->foo(time_base, events_, f);
//    RED_CHECK_EQ(v.size(), 1);
//    v.clear();

//    v.emplace_back(std::make_unique<S>());
//    v.back()->foo(time_base, events_, f);
//    events_.exec_action();
//    RED_CHECK_EQ(v.size(), 0);
//}
