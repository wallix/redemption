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

using namespace std::chrono_literals;

namespace
{
    template<class Duration>
    long to_int_us(Duration const& duration)
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    }
}

RED_AUTO_TEST_CASE(TestSessionUpdateBuffer)
{
    SessionUpdateBuffer buffer;

    buffer.append(MonotonicTimePoint(1s), LogId::TITLE_BAR, KVLogList{
        KVLog{"title"_av, "bloc note"_av},
    });
    buffer.append(MonotonicTimePoint(2s + 770us), LogId::BUTTON_CLICKED, KVLogList{
        KVLog{"value1"_av, "button"_av},
        KVLog{"value2"_av, "bla"_av},
    });

    SessionUpdateBuffer::Data kv_event {};

    auto it = buffer.begin();
    RED_REQUIRE((it != buffer.end()));
    kv_event = *it;
    RED_CHECK(kv_event.id == LogId::TITLE_BAR);
    RED_CHECK(to_int_us(kv_event.time.time_since_epoch()) == to_int_us(1s));
    RED_REQUIRE(kv_event.kv_list.size() == 1);
    RED_CHECK(kv_event.kv_list[0].key == "title"_av);
    RED_CHECK(kv_event.kv_list[0].value == "bloc note"_av);

    ++it;
    RED_REQUIRE((it != buffer.end()));
    kv_event = *it;
    RED_CHECK(kv_event.id == LogId::BUTTON_CLICKED);
    RED_CHECK(to_int_us(kv_event.time.time_since_epoch()) == to_int_us(2s + 770us));
    RED_REQUIRE(kv_event.kv_list.size() == 2);
    RED_CHECK(kv_event.kv_list[0].key == "value1"_av);
    RED_CHECK(kv_event.kv_list[0].value == "button"_av);
    RED_CHECK(kv_event.kv_list[1].key == "value2"_av);
    RED_CHECK(kv_event.kv_list[1].value == "bla"_av);

    ++it;
    RED_REQUIRE((it == buffer.end()));

    buffer.clear();
    RED_REQUIRE((buffer.begin() == buffer.end()));

    buffer.append(MonotonicTimePoint(1s + 3us), LogId::TITLE_BAR, KVLogList{
        KVLog{"k0"_av, "v0"_av},
        KVLog{"k1"_av, "v1"_av},
        KVLog{"k2"_av, "v2"_av},
        KVLog{"k3"_av, "v3"_av},
        KVLog{"k4"_av, "v4"_av},
        KVLog{"k5"_av, "v5"_av},
        KVLog{"k6"_av, "v6"_av},
        KVLog{"k7"_av, "v7"_av},
        KVLog{"k8"_av, "v8"_av},
        KVLog{"k9"_av, "v9"_av},
        KVLog{"k10"_av, "v10"_av},
        KVLog{"k11"_av, "v11"_av},
        KVLog{"k12"_av, "v12"_av},
        KVLog{"k13"_av, "v13"_av},
        KVLog{"k14"_av, "v14"_av},
        KVLog{"k15"_av, "v15"_av},
        KVLog{"k16"_av, "v16"_av},
        KVLog{"k17"_av, "v17"_av},
        KVLog{"k18"_av, "v18"_av},
        KVLog{"k19"_av, "v19"_av},
        KVLog{"k20"_av, "v20"_av},
        KVLog{"k21"_av, "v21"_av},
    });

    it = buffer.begin();
    RED_REQUIRE((it != buffer.end()));
    kv_event = *it;
    RED_CHECK(kv_event.id == LogId::TITLE_BAR);
    RED_CHECK(to_int_us(kv_event.time.time_since_epoch()) == to_int_us(1s + 3us));
    RED_REQUIRE(kv_event.kv_list.size() == 22);
    RED_CHECK(kv_event.kv_list[0].key == "k0"_av);
    RED_CHECK(kv_event.kv_list[0].value == "v0"_av);
    RED_CHECK(kv_event.kv_list[21].key == "k21"_av);
    RED_CHECK(kv_event.kv_list[21].value == "v21"_av);
}
