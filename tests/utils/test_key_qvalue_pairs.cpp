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
#include "system/redemption_unit_tests.hpp"


//#define LOGNULL
#define LOGPRINT
#include "utils/log.hpp"

#include "utils/key_qvalue_pairs.hpp"


RED_AUTO_TEST_CASE(Test_KVPairs1)
{
    kv_pair pairs[] = {kv_pair{"type", "INPUT"}};
    RED_CHECK_EQUAL(std::string("type=\"INPUT\""), key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs1quote)
{
    kv_pair pairs[] = {kv_pair{"type", "INPUT"}};
    RED_CHECK_EQUAL(std::string("type=\"INPUT\""), key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs1quote_bis)
{
    kv_pair pairs[] = {kv_pair{"type", "IN\"PUT"}};
    RED_CHECK_EQUAL(std::string("type=\"IN\\\"PUT\""), key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs2)
{
    kv_pair pairs[] = {kv_pair{"type", "INPUT"}, kv_pair{"data", "xxxyyy"}};

    RED_CHECK_EQUAL(std::string("type=\"INPUT\" data=\"xxxyyy\""), key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs3)
{
    kv_pair pairs[] = {kv_pair{"type", "INPUT"}, kv_pair{"data", "xxxyyy"}, kv_pair{"field", "data"}};

    RED_CHECK_EQUAL(std::string("type=\"INPUT\" data=\"xxxyyy\" field=\"data\""), key_qvalue_pairs(pairs));
}

RED_AUTO_TEST_CASE(Test_KVPairs_noarray)
{
    RED_CHECK_EQUAL(std::string("type=\"INPUT\" data=\"xxxyyy\" field=\"data\""),
                    key_qvalue_pairs({kv_pair{"type", "INPUT"}, kv_pair{"data", "xxxyyy"}, kv_pair{"field", "data"}}));
}

