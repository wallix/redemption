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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/sesman.hpp"
#include "configs/config.hpp"

RED_AUTO_TEST_CASE(TestWithoutExistingLicense)
{
    using namespace std::string_view_literals;

    TimeBase time_base({0,0});
    Inifile ini;
    Sesman sesman(ini, time_base);

    sesman.report("reason1", "msg1");
    sesman.report("reason2", "msg2");

    sesman.log6(LogId::BUTTON_CLICKED, KVList{KVLog{"k1"_av, "v1"_av}});
    sesman.log6(LogId::CHECKBOX_CLICKED, KVList{KVLog{"k2"_av, "v2"_av}});

    int i = 0;
    sesman.flush_acl_report([&](chars_view reason, chars_view msg){
        switch (i++) {
            case 0:
                RED_CHECK(reason == "reason1"sv);
                RED_CHECK(msg == "msg1"sv);
                break;
            case 1:
                RED_CHECK(reason == "reason2"sv);
                RED_CHECK(msg == "msg2"sv);
                break;
            default:
                RED_CHECK(false);
        }
    });
    RED_CHECK(i == 2);

    i = 0;
    sesman.flush_acl_log6([&](LogId log_id, KVList kv_list){
        switch (i++) {
            case 0:
                RED_CHECK(log_id == LogId::BUTTON_CLICKED);
                RED_CHECK(kv_list.size() == 1);
                RED_CHECK(kv_list[0].key == "k1"_av);
                RED_CHECK(kv_list[0].value == "v1"_av);
                break;
            case 1:
                RED_CHECK(log_id == LogId::CHECKBOX_CLICKED);
                RED_CHECK(kv_list.size() == 1);
                RED_CHECK(kv_list[0].key == "k2"_av);
                RED_CHECK(kv_list[0].value == "v2"_av);
                break;
            default:
                RED_CHECK(false);
        }
    });
    RED_CHECK(i == 2);
}
