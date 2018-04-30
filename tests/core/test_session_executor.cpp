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

#define RED_TEST_MODULE TestSessionExecutor
#include "system/redemption_unit_tests.hpp"

#include <string>

#include "core/session_reactor.hpp"
#include "gdi/graphic_api.hpp"

template<class> struct t_{};
RED_AUTO_TEST_CASE(TestSequencer)
{
    using R = jln2::R;
    struct Ctx {} ctx;
    using Array = std::array<int, 5>;
    Array a{};
    auto sequencer = jln2::sequencer(
        [&a](Ctx){ a[0] = 1; return R::Next; },
        [&a](Ctx){ a[1] = 1; return R::Next; },
        [&a](Ctx){ a[2] = 1; return R::Next; },
        [&a](Ctx){ a[3] = 1; return R::Next; },
        [&a](Ctx){ a[4] = 1; return R::Next; }
    );
    RED_CHECK(sequencer(ctx) == jln2::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 0, 0, 0, 0}}));
    RED_CHECK(sequencer(ctx) == jln2::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 0, 0, 0}}));
    RED_CHECK(sequencer(ctx) == jln2::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 1, 0, 0}}));
    RED_CHECK(sequencer(ctx) == jln2::R::Ready); RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 1, 1, 0}}));
    RED_CHECK(sequencer(ctx) == jln2::R::Next);  RED_CHECK_EQ_RANGES(a, (Array{{1, 1, 1, 1, 1}}));
}

constexpr auto fd_is_set = [](int /*fd*/, auto& /*e*/){ return true; };

RED_AUTO_TEST_CASE(TestSessionExecutorTimer)
{
    using Ptr = jln2::SharedPtr;
    SessionReactor session_reactor;

    session_reactor.set_current_time(timeval{10, 222});
    RED_CHECK_EQ(session_reactor.get_current_time().tv_sec, 10);
    RED_CHECK_EQ(session_reactor.get_current_time().tv_usec, 222);

    std::string s;

    Ptr timer1 = session_reactor.create_timer(std::ref(s))
    .set_notify_delete([](std::string& s){
        s += "d1\n";
    })
    .set_delay(std::chrono::seconds(1))
    .on_action(jln2::one_shot([](std::string& s){
        s += "timer1\n";
    }));

    Ptr timer2 = session_reactor.create_timer(std::ref(s))
    .set_notify_delete([](std::string& s){
        s += "d2\n";
    })
    .set_delay(std::chrono::seconds(2))
    .on_action(jln2::always_ready([](std::string& s){
        s += "timer2\n";
    }));

    Ptr timer3 = session_reactor.create_timer(std::ref(s), 'a')
    .set_notify_delete([](std::string& s, char){
        s += "d3\n";
    })
    .set_delay(std::chrono::seconds(1))
    .on_action([](auto ctx, std::string& s, char& c){
        s += "timer3\n";
        return c++ == 'd' ? ctx.terminate() : ctx.ready();
    });

    Ptr timer4 = session_reactor.create_graphic_timer(std::ref(s))
    .set_time({16, 0})
    .on_action(jln2::one_shot([](gdi::GraphicApi&, std::string& s){
        s += "timer4\n";
    }));

    SessionReactor::EnableGraphics enable_gd{true};
    SessionReactor::EnableGraphics disable_gd{false};

    session_reactor.execute_timers(disable_gd, &gdi::null_gd);
    RED_CHECK_EQ(s, "");

    // execute_timers_at or set_current_time + execute_timers
    session_reactor.execute_timers_at(disable_gd, {11, 222}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\n");
    RED_CHECK(!timer1);
    RED_CHECK(timer2);

    session_reactor.execute_timers_at(disable_gd, {13, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\n");
    RED_CHECK(!timer1);
    RED_CHECK(timer2);

    session_reactor.execute_timers_at(disable_gd, {14, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\n");
    RED_CHECK(timer2);

    session_reactor.execute_timers_at(disable_gd, {15, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\n");
    RED_CHECK(timer2);

    timer2.reset();
    RED_CHECK(!timer2);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\n");

    session_reactor.execute_timers_at(disable_gd, {16, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\n");

    session_reactor.execute_timers_at(enable_gd, {16, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer3\ntimer1\nd1\ntimer3\ntimer2\ntimer3\ntimer3\nd3\ntimer2\nd2\ntimer4\n");
}

RED_AUTO_TEST_CASE(TestSessionExecutorSimpleEvent)
{
    SessionReactor session_reactor;

    std::string s;

    auto gd = session_reactor.create_graphic_event(std::ref(s))
    .set_notify_delete([](std::string& s){
        s += "~gd\n";
    })
    .on_action(jln2::one_shot([](gdi::GraphicApi&, std::string& s){
        s += "gd\n";
    }));

    auto callback = session_reactor.create_callback_event(std::ref(s))
    .set_notify_delete([](std::string& s){
        s += "~callback\n";
    })
    .on_action(jln2::one_shot([](Callback&, std::string& s){
        s += "callback\n";
    }));

    auto ini = session_reactor.create_sesman_event(std::ref(s))
    .on_action([](auto ctx, Inifile&, std::string& s){
        s += "ini\n";
        return ctx.terminate();
    });

    session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
    RED_CHECK_EQ(s, "gd\n~gd\n");

    char dummy;

    session_reactor.execute_sesman(*reinterpret_cast<Inifile*>(&dummy));
    RED_CHECK_EQ(s, "gd\n~gd\nini\n");

    session_reactor.execute_callbacks(*reinterpret_cast<Callback*>(&dummy));
    RED_CHECK_EQ(s, "gd\n~gd\nini\ncallback\n~callback\n");

    RED_CHECK(!gd);
    RED_CHECK(!ini);
    RED_CHECK(!callback);
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils/select.hpp"
#include "utils/sugar/scope_exit.hpp"

RED_AUTO_TEST_CASE(TestSessionExecutorFd)
{
    SessionReactor session_reactor;

    std::string s;

    int fd1 = ::open("/tmp/execute_graphics.test", O_CREAT | O_RDONLY, 0777);
    RED_REQUIRE_GT(fd1, 0);
    SCOPE_EXIT(::close(fd1));

    SessionReactor::TopFdPtr fd_event = session_reactor.create_fd_event(fd1, std::ref(s))
    .on_action([](auto ctx, std::string& s){
        s += "fd1\n";
        return ctx.next();
    })
    .on_exit(jln2::propagate_exit([](std::string& s){
        s += "~fd1\n";
    }))
    .set_timeout({})
    .on_timeout([](auto ctx, std::string&){ return ctx.ready(); });

    SessionReactor::GraphicFdPtr fd_gd_event = session_reactor.create_graphic_fd_event(fd1)
    .on_action([&s](auto ctx, gdi::GraphicApi&){
        s += "fd2\n";
        return ctx.next();
    })
    .on_exit(jln2::propagate_exit([&s](gdi::GraphicApi&){
        s += "~fd2\n";
    }))
    .set_timeout({})
    .on_timeout([](auto ctx, gdi::GraphicApi&){ return ctx.ready(); });

    session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
    RED_CHECK_EQ(s, "fd2\n~fd2\n");

    session_reactor.execute_events(fd_is_set);
    RED_CHECK_EQ(s, "fd2\n~fd2\nfd1\n~fd1\n");
}

// RED_AUTO_TEST_CASE(TestSessionExecutorSequence)
// {
//     SessionReactor session_reactor;
//
//     std::string s;
//
//     using namespace jln::literals;
//     using jln::value;
//
//     auto trace = [](auto name){
//         return [](auto ctx, gdi::GraphicApi&, std::string& s){
//             s += decltype(name){}.c_str();
//             // or
//             // if constexpr (ctx.is_final_sequence()) return ctx.ready();
//             // else return ctx.sequence_next();
//             return ctx.sequence_next_or_terminate();
//         };
//     };
//
//     SessionReactor::GraphicEventPtr event = session_reactor.create_graphic_event(std::ref(s))
//     .on_action(jln::funcsequencer(
//         trace("a"_s),
//         trace("b"_s),
//         trace("c"_s),
//         trace("d"_s)
//     ));
//
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(event);
//     RED_CHECK_EQ(s, "a");
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(event);
//     RED_CHECK_EQ(s, "ab");
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(event);
//     RED_CHECK_EQ(s, "abc");
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(!event);
//     RED_CHECK_EQ(s, "abcd");
//     s.clear();
//
//     auto trace2 = [](auto f){
//         return [](auto ctx, gdi::GraphicApi&, std::string& s){
//             s += ctx.sequence_name().c_str();
//             return jln::make_lambda<decltype(f)>()(ctx);
//         };
//     };
//
//     event = session_reactor.create_graphic_event(std::ref(s))
//     .on_action(jln::funcsequencer(
//         "a"_f = trace2([](auto ctx){ return ctx.sequence_next(); }),
//         "b"_f = trace2([](auto ctx){ return ctx.sequence_at("d"_s); }),
//         "c"_f = [](auto ctx, gdi::GraphicApi& gd, std::string& s){
//             s += ctx.sequence_name().c_str();
//             return ctx.exec_sequence_at("e"_s, gd);
//         },
//         "d"_f = trace2([](auto ctx){ return ctx.sequence_previous(); }),
//         "e"_f = trace2([](auto ctx){ return ctx.terminate(); })
//     ));
//
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(event);
//     RED_CHECK_EQ(s, "a");
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(event);
//     RED_CHECK_EQ(s, "ab");
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(event);
//     RED_CHECK_EQ(s, "abd");
//     session_reactor.execute_graphics(fd_is_set, gdi::null_gd());
//     RED_CHECK(!event);
//     RED_CHECK_EQ(s, "abdce");
// }
