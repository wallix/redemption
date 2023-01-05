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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/meminfo.hpp"


RED_AUTO_TEST_CASE(TestCheckMemoryAvailable)
{
    RED_CHECK(check_memory_available(0));
    RED_CHECK(!check_memory_available(0xffff'ffff'ffff'ffff));
}

RED_AUTO_TEST_CASE(TestExtractMemoryAvailable)
{
    auto extract = [](chars_view data) {
        return extract_memory_available(data.data(), checked_int(data.size()));
    };

    RED_CHECK(28614956 == extract(
        "MemTotal:       32786092 kB\n"
        "MemFree:        26590104 kB\n"
        "MemAvailable:   28614956 kB\n"
        ""_av));
    RED_CHECK(-1 == extract(
        "MemTotal:       32786092 kB\n"
        "MemFree:        26590104 kB\n"
        "Buffers:        28614956 kB\n"
        ""_av));
    RED_CHECK(-1 == extract(
        "MemTotal:       32786092 kB\n"
        "MemFree:        26590104 kB\n"
        ""_av));
    RED_CHECK(-1 == extract(
        "MemTotal:       32786092 kB\n"
        ""_av));
    RED_CHECK(-1 == extract(""_av));
}
