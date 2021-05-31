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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "mod/metrics_hmac.hpp"
#include "mod/metrics.hpp"
#include "core/events.hpp"
#include "configs/config.hpp"

#include <memory>

template<class ProtocolMetrics>
struct ModMetrics : Metrics
{
    using Metrics::Metrics;

    ProtocolMetrics protocol_metrics{*this};

    static std::unique_ptr<ModMetrics>
    make_unique(EventContainer& events, Inifile& ini, ScreenInfo const& screen_info)
    {
        chars_view auth_user(ini.get<cfg::globals::auth_user>());
        chars_view sign_key(ini.get<cfg::metrics::sign_key>());
        chars_view target_user(ini.get<cfg::globals::target_user>());
        chars_view target_device(ini.get<cfg::globals::target_device>());

        return std::make_unique<ModMetrics>(
            ini.get<cfg::metrics::log_dir_path>().as_string(),
            ini.get<cfg::context::session_id>(),
            hmac_user(auth_user, sign_key),
            hmac_account(target_user, sign_key),
            hmac_device_service(
                target_device,
                ini.get<cfg::context::target_service>(),
                sign_key),
            hmac_client_info(
                ini.get<cfg::globals::host>(),
                screen_info,
                sign_key),
            events.get_monotonic_time(),
            events.get_time_base().real_time,
            ini.get<cfg::metrics::log_file_turnover_interval>(),
            ini.get<cfg::metrics::log_interval>());
    }

    void timed_log(
        EventsGuard& events_guard, std::chrono::seconds log_interval,
        std::string_view name)
    {
        assert(this->time_base);
        this->time_base = &events_guard.get_time_base();
        this->log_interval = log_interval;
        events_guard.create_event_timeout(
            name,
            log_interval,
            [this](Event& event) {
                event.alarm.reset_timeout(this->log_interval);
                this->log(event.alarm.now, this->time_base->real_time);
            });
    }

private:
    std::chrono::seconds log_interval;
    TimeBase const * time_base = nullptr;
};
