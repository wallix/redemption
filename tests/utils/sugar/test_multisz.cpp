/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#define RED_TEST_MODULE TestMultiSZ
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/sugar/multisz.hpp"


RED_AUTO_TEST_CASE(TestSOHSeparatedStringsToMultiSZ)
{
    char dest[16];

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "");
    RED_CHECK(!memcmp(dest, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234");
    RED_CHECK(!memcmp(dest, "1234\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "12345678901234567890");
    RED_CHECK(!memcmp(dest, "12345678901234\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234\x01OPQR");
    RED_CHECK(!memcmp(dest, "1234\x00OPQR\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "12345678\x01OPQRSTUV");
    RED_CHECK(!memcmp(dest, "12345678\x00OPQRS\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234\x01\x01");
    RED_CHECK(!memcmp(dest, "1234\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    SOHSeparatedStringsToMultiSZ(dest, sizeof(dest), "1234\x01\x01OPQR");
    RED_CHECK(!memcmp(dest, "1234\x00\x00OPQR\x00\x00\x00\x00\x00\x00", sizeof(dest)));
}

RED_AUTO_TEST_CASE(TestMultiSZCopy)
{
    char dest[16];

    MultiSZCopy(dest, sizeof(dest), "\x00");
    RED_CHECK(!memcmp(dest, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "1234\x00");
    RED_CHECK(!memcmp(dest, "1234\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "12345678901234567890\x00");
    RED_CHECK(!memcmp(dest, "12345678901234\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "1234\x00OPQR\x00");
    RED_CHECK(!memcmp(dest, "1234\x00OPQR\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "12345678\x00OPQRSTUV\x00");
    RED_CHECK(!memcmp(dest, "12345678\x00OPQRS\x00\x00", sizeof(dest)));

    MultiSZCopy(dest, sizeof(dest), "12345678\x00\x00OPQRSTUV\x00");
    RED_CHECK(!memcmp(dest, "12345678\x00\x00\x00\x00\x00\x00\x00\x00", sizeof(dest)));
}
