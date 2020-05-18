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

#include <string>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "core/session_reactor.hpp"
#include "mod/mod_api.hpp"

RED_TEST_DELEGATE_PRINT_ENUM(jln::R);

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
    TimeBase time_base;
    TopFdContainer fd_events_;
    TimerContainer timer_events_;

    time_base.set_current_time(timeval{10, 222});
    RED_CHECK_EQ(time_base.get_current_time().tv_sec, 10);
    RED_CHECK_EQ(time_base.get_current_time().tv_usec, 222);

    std::string s;

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
    .on_action([](JLN_TIMER_CTX ctx, std::string& s, char& c){
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

    fd_events_.exec_timeout(end_tv);
    RED_CHECK_EQ(s, "");

    // set_current_time + execute timers, to simulate times flying
    time_base.set_current_time({11, 222});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\n");
    RED_CHECK(!timer1);
    RED_CHECK(bool(timer2));

    time_base.set_current_time({13, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\n");
    RED_CHECK(!timer1);
    RED_CHECK(bool(timer2));

    time_base.set_current_time({14, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\n");
    RED_CHECK(bool(timer2));

    time_base.set_current_time({15, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\n");
    RED_CHECK(bool(timer2));

    timer2.reset();
    RED_CHECK(!timer2);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\n");

    time_base.set_current_time({16, 0});
    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);

    time_base.set_current_time({16, 0});

    end_tv = time_base.get_current_time();
    timer_events_.exec_timer(end_tv);
    fd_events_.exec_timeout(end_tv);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\ntimer4\n");
}


RED_AUTO_TEST_CASE_WF(TestTimeBaseFd, wf)
{
    TimeBase time_base;
    TopFdContainer fd_events_;

    std::string s;

    unique_fd ufd(wf.c_str(), O_CREAT | O_RDONLY, 0777);
    int const fd1 = ufd.fd();
    RED_REQUIRE_GT(fd1, 0);

    TopFdPtr fd_event = fd_events_.create_top_executor(time_base, fd1, std::ref(s))
    .on_action([](JLN_TOP_CTX ctx, std::string& s){
        s += "fd1:";
        return ctx.next();
    })
    .on_exit(jln::propagate_exit([](std::string& s){
        s += "~fd1:";
    }))
    .set_timeout({})
    .on_timeout([](JLN_TOP_TIMER_CTX ctx, std::string&){ return ctx.ready(); });

    TopFdPtr fd_event_2 = fd_events_.create_top_executor(time_base, fd1)
    .on_action([&s](JLN_TOP_CTX ctx){
        s += "fd2:";
        return ctx.next();
    })
    .on_exit(jln::propagate_exit([&s](){
        s += "~fd2:";
    }))
    .set_timeout({})
    .on_timeout([](JLN_TOP_TIMER_CTX ctx){ return ctx.ready(); });

    fd_events_.exec_action(fd_is_set);
    RED_CHECK_EQ(s, "fd2:~fd2:fd1:~fd1:");
}

RED_AUTO_TEST_CASE(TestTimeBaseSequence)
{
    TimeBase time_base;
    TimerContainer events_;
    time_base.set_current_time(timeval{0, 0});

    std::string s;

    auto end_tv = time_base.get_current_time();

    using namespace jln::literals;
    using jln::value;

    auto trace = [](auto name){
        return [](JLN_FUNCSEQUENCER_CTX ctx, std::string& s){
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
//        return [](JLN_FUNCSEQUENCER_CTX ctx, std::string& s){
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
//            .on_action([](JLN_ACTION_CTX ctx, S&, F){
//                return ctx.terminate();
//            });
//        }
//    };

//    TimeBase time_base;
//    TopFdContainer fd_events_;

//    v.emplace_back(std::make_unique<S>());
//    v.back()->foo(time_base, events_, f);
//    RED_CHECK_EQ(v.size(), 1);
//    v.clear();

//    v.emplace_back(std::make_unique<S>());
//    v.back()->foo(time_base, events_, f);
//    events_.exec_action();
//    fd_events_.exec_action([]([[maybe_unused]] auto&&... xs){return false;});
//    RED_CHECK_EQ(v.size(), 0);
//}
