/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/session_events.hpp"

namespace
{
    struct DataEvents
    {
        EventContainer events;

        DataEvents()
        {
            events.set_current_time({10000, 0});
        }

        void execute_timer_events()
        {
            events.execute_events(
                [](int /*fd*/){ return false; },
                false
            );
        }

        void set_time_and_excute_timer_events(time_t t)
        {
            set_time(t);
            execute_timer_events();
        }

        void set_time(time_t t)
        {
            events.time_base.current_time.tv_sec = t;
        }

        time_t time() const
        {
            return events.time_base.current_time.tv_sec;
        }
    };
}


using namespace std::chrono_literals;


RED_AUTO_TEST_CASE(TestKeepAlive)
{
    DataEvents d;
    time_t t = d.time();
    Inifile ini;

    KeepAlive keepalive(ini, d.events, 30s);

    auto set_delay = [&](time_t delay){
        d.set_time_and_excute_timer_events(t + delay);
        return ini.is_asked<cfg::context::keepalive>();
    };

    auto receive_keepalive = [&]{
        keepalive.keep_alive();
        // reset is_asked
        ini.set<cfg::context::keepalive>(true);
    };

    keepalive.start();
    RED_TEST(!ini.is_asked<cfg::context::keepalive>());
    RED_TEST(!set_delay(20));
    RED_TEST(set_delay(30));
    RED_TEST(set_delay(50));
    receive_keepalive();
    RED_TEST(!set_delay(70));
    receive_keepalive();
    RED_TEST(set_delay(100));
    RED_CHECK_EXCEPTION_ERROR_ID(set_delay(130),
        ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED);
}

RED_AUTO_TEST_CASE(TestInactivity)
{
    DataEvents d;
    time_t t = d.time();

    Inactivity inactivity(d.events);


    // min is 30
    inactivity.start(20s);
    d.execute_timer_events();

    d.set_time_and_excute_timer_events(t + 25);
    RED_CHECK_EXCEPTION_ERROR_ID(d.set_time_and_excute_timer_events(t + 30),
        ERR_SESSION_CLOSE_USER_INACTIVITY);


    // activity, activity, event
    //@{
    d.set_time(t + 10000);
    t = d.time();
    inactivity.start(40s);

    d.set_time_and_excute_timer_events(t + 25);
    inactivity.activity();

    d.set_time_and_excute_timer_events(t + 50);
    inactivity.activity();

    d.set_time_and_excute_timer_events(t + 50);

    d.set_time_and_excute_timer_events(t + 75);

    RED_CHECK_EXCEPTION_ERROR_ID(d.set_time_and_excute_timer_events(t + 90),
        ERR_SESSION_CLOSE_USER_INACTIVITY);
    //@}


    // start() then stop()
    d.set_time(t + 10000);
    t = d.time();
    inactivity.start(40s);
    inactivity.stop();
    d.set_time(t + 75);
    d.execute_timer_events();
}

RED_AUTO_TEST_CASE(TestEndSessionWarning)
{
    DataEvents d;
    time_t t = d.time();

    auto sync_time = [&]{
        t += 10000;
        d.set_time(t);
    };


    EndSessionWarning end_session_warning(d.events);

    const auto uncalled = -1min .count();
    auto update_warning = [&]{
        auto m = uncalled;
        end_session_warning.update_warning([&](std::chrono::minutes minutes){
            RED_TEST(minutes.count() >= 0);
            m = minutes.count();
        });
        return m;
    };

    auto set_time_and_update_warning = [&](time_t t){
        end_session_warning.set_time(t);
        d.execute_timer_events();
        return update_warning();
    };

    auto update_time_and_update_warning = [&](time_t t){
        d.set_time_and_excute_timer_events(t);
        return update_warning();
    };


    RED_TEST(update_warning() == uncalled);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 30*60 + 2) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2) == 30);
    RED_TEST(update_time_and_update_warning(t + 3) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 1 + 20*60) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2 + 20*60) == 10);
    RED_TEST(update_time_and_update_warning(t + 1 + 25*60) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2 + 25*60) == 5);
    RED_TEST(update_time_and_update_warning(t + 1 + 29*60) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2 + 29*60) == 1);
    RED_TEST(update_time_and_update_warning(t + 1 + 30*60) == uncalled);
    RED_CHECK_EXCEPTION_ERROR_ID(update_time_and_update_warning(t + 2 + 30*60),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 30*60) == 30);
    RED_TEST(update_time_and_update_warning(t - 1 + 20*60) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 0 + 20*60) == 10);
    RED_TEST(update_time_and_update_warning(t - 1 + 25*60) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 0 + 25*60) == 5);
    RED_TEST(update_time_and_update_warning(t - 1 + 29*60) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 0 + 29*60) == 1);
    RED_TEST(update_time_and_update_warning(t - 1 + 30*60) == uncalled);
    RED_CHECK_EXCEPTION_ERROR_ID(update_time_and_update_warning(t + 0 + 30*60),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 20*60) == 20);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 10*60) == 10);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 20) == 0);

    sync_time();
    RED_TEST(set_time_and_update_warning(0) == uncalled);

    RED_CHECK_EXCEPTION_ERROR_ID(set_time_and_update_warning(t),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);
}
