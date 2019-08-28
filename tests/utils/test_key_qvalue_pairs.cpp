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

#include "utils/key_qvalue_pairs.hpp"

#include <vector>


RED_AUTO_TEST_CASE(TestQValue)
{
    auto to_string = [s = std::string{}](KVList kv_list) mutable -> std::string const& {
        log_format_set_info(s, LogId::INPUT_LANGUAGE, kv_list);
        return s;
    };

    RED_CHECK_EQUAL("type=\"INPUT_LANGUAGE\"", to_string({}));
    RED_CHECK_EQUAL("type=\"INPUT_LANGUAGE\" data=\"xxx\\\"yyy\" field=\"data\"",
        to_string({
            KVLog{"data"_av, "xxx\"yyy"_av},
            KVLog{"field"_av, "data"_av},
        }));
    RED_CHECK_EQUAL("type=\"INPUT_LANGUAGE\" data=\"a\\\\b\\\"c \\r\\n\"",
        to_string({
            KVLog{"data"_av, "a\\b\"c \r\n"_av},
        }));
}
