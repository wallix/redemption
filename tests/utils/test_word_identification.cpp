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

#define UNIT_TEST_MODULE TestWordIdentification
#include "system/redemption_unit_tests.hpp"


//#define LOGNULL
#define LOGPRINT
#include "utils/log.hpp"

#include "utils/word_identification.hpp"


RED_AUTO_TEST_CASE(Test_Word)
{
    RED_CHECK_EQUAL(0, in("cmd", {}));

    const char * options1[] = {"one"};
    RED_CHECK_EQUAL(0, in("cmd", options1));
    RED_CHECK_EQUAL(1, in("one", options1));

    const char * options3[] = {"one", "two", "three"};
    RED_CHECK_EQUAL(0, in("cmd", options3));
    RED_CHECK_EQUAL(0, in("cmd", {"one", "two", "three"}));
    RED_CHECK_EQUAL(3, in("three", options3));
    RED_CHECK_EQUAL(3, in("three", {"one", "two", "three"}));
    
    
}

//RED_AUTO_TEST_CASE(Test_end)
//{
//    RED_CHECK_EQUAL(0, ends_with("./tools/verifier.py", {}));

//    const char * options1[] = {"one"};
//    RED_CHECK_EQUAL(0, ends_with("./tools/verifier.py", options1));
//    RED_CHECK_EQUAL(1, ends_with("./tools/verifier.py", {"verifier.py"}));

//    const char * options3[] = {"one", "two", "verifier.py"};
//    RED_CHECK_EQUAL(3, ends_with("./tools/verifier.py", options3));
//    RED_CHECK_EQUAL(0, ends_with("./tools/verifier.py", {"one", "two", "three"}));

//    RED_CHECK_EQUAL(3, ends_with("./tools/verifier.py", options3));
//    RED_CHECK_EQUAL(3, ends_with("./tools/verifier.py", {"one", "two", "verifier.py"}));
//}
