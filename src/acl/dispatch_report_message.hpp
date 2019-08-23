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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "core/log_id.hpp"
#include "core/report_message_api.hpp"
#include "core/front_api.hpp"

#include <string>


class DispatchReportMessage : public ReportMessageApi
{
public:
    using LogCategoryFlags = utils::flags_t<LogCategoryId>;

    DispatchReportMessage(
        ReportMessageApi & report_message, FrontAPI& front,
        LogCategoryFlags dont_log) noexcept
    : report_message(report_message)
    , front(front)
    , dont_log(dont_log)
    {}

    void report(const char * reason, const char * message) override
    {
        this->report_message.report(reason, message);
    }

    void log6(LogId id, const timeval time, KVList kv_list) override
    {
        this->report_message.log6(id, time, kv_list);

        if (dont_log.test(detail::log_id_category_map[underlying_cast(id)])) {
            return ;
        }

        std::string s;
        auto& str_id = detail::log_id_string_map[underlying_cast(id)];
        s.insert(s.end(), str_id.begin(), str_id.end());
        for (auto const& kv : kv_list) {
            s += '\x01';
            s.insert(s.end(), kv.value.begin(), kv.value.end());
        }

        this->front.session_update(s);
    }

    void update_inactivity_timeout() override
    {
        this->report_message.update_inactivity_timeout();
    }

    time_t get_inactivity_timeout() override
    {
        return this->report_message.get_inactivity_timeout();
    }

private:
    ReportMessageApi& report_message;
    FrontAPI& front;
    LogCategoryFlags dont_log;
};
