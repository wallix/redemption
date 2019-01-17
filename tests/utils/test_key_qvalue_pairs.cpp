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

#define RED_TEST_MODULE TestKeyValuePairs
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/key_qvalue_pairs.hpp"


RED_AUTO_TEST_CASE(Test_KVPairs1)
{
    kv_pair pairs[] = {{"type", "INPUT"}};
    RED_CHECK_EQUAL("type=\"INPUT\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs1av)
{
    kv_pair pairs[] = {{cstr_array_view("type"), make_array_view("INPUT!BEURK", 5)}};
    RED_CHECK_EQUAL("type=\"INPUT\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs1quote)
{
    kv_pair pairs[] = {{"type", "INPUT"}};
    RED_CHECK_EQUAL("type=\"INPUT\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs1quote_bis)
{
    kv_pair pairs[] = {{"type", "IN\"PUT"}};
    RED_CHECK_EQUAL("type=\"IN\\\"PUT\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs2)
{
    kv_pair pairs[] = {{"type", "INPUT"}, {"data", "xxxyyy"}};
    RED_CHECK_EQUAL("type=\"INPUT\" data=\"xxxyyy\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs3)
{
    kv_pair pairs[] = {{"type", "INPUT"}, {"data", "xxxyyy"}, {"field", "data"}};
    RED_CHECK_EQUAL("type=\"INPUT\" data=\"xxxyyy\" field=\"data\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs4)
{
    kv_pair pairs[] = {{"type", "a\\b\"c \r\n"}};
    RED_CHECK_EQUAL("type=\"a\\\\b\\\"c \\r\\n\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs5)
{
    kv_pair pairs[] = {{"type", "\xf5"}};
    RED_CHECK_EQUAL("type=\"\xf5\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs_noarray)
{
    kv_pair pairs[] = {{"type", "INPUT"}, {"data", "xxxyyy"}, {"field", "data"}};
    RED_CHECK_EQUAL("type=\"INPUT\" data=\"xxxyyy\" field=\"data\"", key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KeyQvalueFormatter)
{
    KeyQvalueFormatter message;
    message.assign("INPUT", {{"data", "xxxyyy"}, {"field", "data"}});
    RED_CHECK_EQUAL("type=\"INPUT\" data=\"xxxyyy\" field=\"data\"", message.str());
}
