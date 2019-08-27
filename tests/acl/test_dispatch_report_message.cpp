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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/dispatch_report_message.hpp"
#include "test_only/front/fake_front.hpp"

#include <vector>

namespace
{
    void push(std::string& s, array_view_const_char av)
    {
        s.insert(s.end(), av.begin(), av.end());
        s += '\n';
    }
}

RED_AUTO_TEST_CASE(TestDispatchReportMessage)
{
    ScreenInfo screen_info{1, 1, BitsPerPixel::BitsPP16};
    struct : FakeFront {
        using FakeFront::FakeFront;

        void session_update(LogId id, KVList /*kv_list*/) override
        {
            push(s, detail::log_id_string_map[underlying_cast(id)]);
        }

        std::string s;
    } front(screen_info);
    struct : NullReportMessage {
        void log6(LogId id, const timeval /*time*/, KVList /*kv_list*/) override
        {
            push(s, detail::log_id_string_map[underlying_cast(id)]);
        }

        std::string s;
    } report_message;

    DispatchReportMessage dispatch_report_message(report_message, front, LogCategoryId::Clipboard);

    dispatch_report_message.log6(LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION, {}, {});
    dispatch_report_message.log6(LogId::BUTTON_CLICKED, {}, {});

    RED_CHECK(front.s == "BUTTON_CLICKED\n");
    RED_CHECK(report_message.s == "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION\nBUTTON_CLICKED\n");
}
