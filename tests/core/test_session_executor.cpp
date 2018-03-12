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

RED_AUTO_TEST_CASE(TestSessionExecutorTimer)
{
    SessionReactor session_reactor;

    session_reactor.set_current_time(timeval{10, 222});
    RED_CHECK_EQ(session_reactor.get_current_time().tv_sec, 10);
    RED_CHECK_EQ(session_reactor.get_current_time().tv_usec, 222);

    std::string s;

    auto timer1 = session_reactor.create_timer(std::ref(s))
    .set_delay(std::chrono::seconds(1))
    .on_action(jln::one_shot([](std::string& s){
        s += "timer1\n";
    }));
    auto timer2 = session_reactor.create_timer(std::ref(s))
    .set_delay(std::chrono::seconds(2))
    .on_action([](auto ctx, std::string& s){
        s += "timer2\n";
        return ctx.ready();
    });

    SessionReactor::EnableGraphics enable_gd{true};
    SessionReactor::EnableGraphics disable_gd{false};

    session_reactor.execute_timers(enable_gd, &gdi::null_gd);
    RED_CHECK_EQ(s, "");

    session_reactor.execute_timers_at(enable_gd, {11, 222}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer1\n");
    RED_CHECK(!timer1);
    RED_CHECK(timer2);

    session_reactor.execute_timers_at(enable_gd, {13, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer1\ntimer2\n");
    RED_CHECK(!timer1);
    RED_CHECK(timer2);

    session_reactor.execute_timers_at(enable_gd, {14, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer1\ntimer2\n");
    RED_CHECK(timer2);

    session_reactor.execute_timers_at(enable_gd, {15, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer1\ntimer2\ntimer2\n");
    RED_CHECK(timer2);

    timer2.reset();
    session_reactor.execute_timers_at(enable_gd, {18, 0}, &gdi::null_gd);
    RED_CHECK_EQ(s, "timer1\ntimer2\ntimer2\n");
    RED_CHECK(!timer2);
}
