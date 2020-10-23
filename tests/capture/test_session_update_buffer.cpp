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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong

   Unit tests for Acl Serializer
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "capture/session_update_buffer.hpp"


RED_AUTO_TEST_CASE(TestSessionUpdateBuffer)
{
    SessionUpdateBuffer buffer;

    buffer.append(timeval{1, 0}, LogId::TITLE_BAR, KVLogList{
        KVLog{"title"_av, "bloc note"_av},
    });
    buffer.append(timeval{2, 770}, LogId::BUTTON_CLICKED, KVLogList{
        KVLog{"value1"_av, "button"_av},
        KVLog{"value2"_av, "bla"_av},
    });

    SessionUpdateBuffer::Data kv_event {};

    auto it = buffer.begin();
    RED_REQUIRE((it != buffer.end()));
    kv_event = *it;
    RED_CHECK(kv_event.id == LogId::TITLE_BAR);
    RED_CHECK(kv_event.time.tv_sec == 1);
    RED_CHECK(kv_event.time.tv_usec == 0);
    RED_REQUIRE(kv_event.kv_list.size() == 1);
    RED_CHECK(kv_event.kv_list[0].key == "title"_av);
    RED_CHECK(kv_event.kv_list[0].value == "bloc note"_av);

    ++it;
    RED_REQUIRE((it != buffer.end()));
    kv_event = *it;
    RED_CHECK(kv_event.id == LogId::BUTTON_CLICKED);
    RED_CHECK(kv_event.time.tv_sec == 2);
    RED_CHECK(kv_event.time.tv_usec == 770);
    RED_REQUIRE(kv_event.kv_list.size() == 2);
    RED_CHECK(kv_event.kv_list[0].key == "value1"_av);
    RED_CHECK(kv_event.kv_list[0].value == "button"_av);
    RED_CHECK(kv_event.kv_list[1].key == "value2"_av);
    RED_CHECK(kv_event.kv_list[1].value == "bla"_av);

    ++it;
    RED_REQUIRE((it == buffer.end()));

    buffer.clear();
    RED_REQUIRE((buffer.begin() == buffer.end()));
}
