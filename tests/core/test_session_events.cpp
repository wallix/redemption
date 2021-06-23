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
#include "core/error.hpp"
#include "configs/config.hpp"

using namespace std::literals::chrono_literals;

namespace
{
    struct DataEvents
    {
        EventManager event_manager;
        EventContainer& events = event_manager.get_events();

        DataEvents()
        {
            event_manager.get_writable_time_base().monotonic_time = MonotonicTimePoint{10000s};
        }

        void execute_timer_events()
        {
            event_manager.execute_events(
                [](int /*fd*/){ return false; },
                false
            );
        }

        void set_time_and_execute_timer_events(MonotonicTimePoint t)
        {
            set_time(t);
            execute_timer_events();
        }

        void set_time(MonotonicTimePoint t)
        {
            event_manager.get_writable_time_base().monotonic_time = t;
        }

        MonotonicTimePoint time() const
        {
            return event_manager.get_monotonic_time();
        }
    };
} // namespace anonymous


using namespace std::chrono_literals;


RED_AUTO_TEST_CASE(TestKeepAlive)
{
    DataEvents d;
    auto t = d.time();
    Inifile ini;

    KeepAlive keepalive(ini, d.events, 30s);

    auto set_delay = [&](MonotonicTimePoint::duration delay){
        d.set_time_and_execute_timer_events(t + delay);
        return ini.is_asked<cfg::context::keepalive>();
    };

    auto receive_keepalive = [&]{
        keepalive.keep_alive();
        // reset is_asked
        ini.set<cfg::context::keepalive>(true);
    };

    keepalive.start();
    RED_TEST(!ini.is_asked<cfg::context::keepalive>());
    RED_TEST(!set_delay(20s));
    RED_TEST(set_delay(30s));
    RED_TEST(set_delay(50s));
    receive_keepalive();
    RED_TEST(!set_delay(70s));
    receive_keepalive();
    RED_TEST(set_delay(100s));
    RED_CHECK_EXCEPTION_ERROR_ID(set_delay(130s),
        ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED);
}

RED_AUTO_TEST_CASE(TestInactivity)
{
    DataEvents d;
    auto t = d.time();

    Inactivity inactivity(d.events);


    // min is 30
    inactivity.start(20s);
    d.execute_timer_events();

    d.set_time_and_execute_timer_events(t + 25s);
    RED_CHECK_EXCEPTION_ERROR_ID(d.set_time_and_execute_timer_events(t + 30s),
        ERR_SESSION_CLOSE_USER_INACTIVITY);


    // activity, activity, event
    //@{
    d.set_time(t + 10000s);
    t = d.time();
    inactivity.start(40s);

    d.set_time_and_execute_timer_events(t + 25s);
    inactivity.activity();

    d.set_time_and_execute_timer_events(t + 50s);
    inactivity.activity();

    d.set_time_and_execute_timer_events(t + 50s);

    d.set_time_and_execute_timer_events(t + 75s);

    RED_CHECK_EXCEPTION_ERROR_ID(d.set_time_and_execute_timer_events(t + 90s),
        ERR_SESSION_CLOSE_USER_INACTIVITY);
    //@}


    // start() then stop()
    d.set_time(t + 10000s);
    t = d.time();
    inactivity.start(40s);
    inactivity.stop();
    d.set_time(t + 75s);
    d.execute_timer_events();
}

RED_AUTO_TEST_CASE(TestEndSessionWarning)
{
    DataEvents d;
    auto t = d.time();

    auto sync_time = [&]{
        t += 10000s;
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

    auto set_time_and_update_warning = [&](MonotonicTimePoint t){
        end_session_warning.set_time(t);
        d.execute_timer_events();
        return update_warning();
    };

    auto update_time_and_update_warning = [&](MonotonicTimePoint t){
        d.set_time_and_execute_timer_events(t);
        return update_warning();
    };


    RED_TEST(update_warning() == uncalled);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 30min + 2s) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2s) == 30);
    RED_TEST(update_time_and_update_warning(t + 3s) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 1s + 20min) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2s + 20min) == 10);
    RED_TEST(update_time_and_update_warning(t + 1s + 25min) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2s + 25min) == 5);
    RED_TEST(update_time_and_update_warning(t + 1s + 29min) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 2s + 29min) == 1);
    RED_TEST(update_time_and_update_warning(t + 1s + 30min) == uncalled);
    RED_CHECK_EXCEPTION_ERROR_ID(update_time_and_update_warning(t + 2s + 30min),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 30min) == 30);
    RED_TEST(update_time_and_update_warning(t - 1s + 20min) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 0s + 20min) == 10);
    RED_TEST(update_time_and_update_warning(t - 1s + 25min) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 0s + 25min) == 5);
    RED_TEST(update_time_and_update_warning(t - 1s + 29min) == uncalled);
    RED_TEST(update_time_and_update_warning(t + 0s + 29min) == 1);
    RED_TEST(update_time_and_update_warning(t - 1s + 30min) == uncalled);
    RED_CHECK_EXCEPTION_ERROR_ID(update_time_and_update_warning(t + 0s + 30min),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 20min) == 20);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 10min) == 10);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 20s) == 0);

    RED_CHECK_EXCEPTION_ERROR_ID(set_time_and_update_warning(t),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);

    sync_time();
    RED_TEST(set_time_and_update_warning(t + 20min) == 20);
    end_session_warning.add_delay(+10min);
    RED_TEST(update_time_and_update_warning(t) == 30);
    end_session_warning.add_delay(-50min);
    RED_CHECK_EXCEPTION_ERROR_ID(update_time_and_update_warning(t),
        ERR_SESSION_CLOSE_ENDDATE_REACHED);
}
