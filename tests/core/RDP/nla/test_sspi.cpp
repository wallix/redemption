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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#define RED_TEST_MODULE TestSspi
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/nla/sspi.hpp"


RED_AUTO_TEST_CASE(Test_Array)
{
    {
        Array array;
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 65536);

        Array array2(42);
        array.copy(array2);
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 42);
    }

    {
        Array array(42);
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 42);
    }

    {
        Array array(65536 + 1);
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 65536 + 1);

        array.init(42);
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 42);
    }

    {
        Array array;
        array.get_data()[0] = 42;
        array.get_data()[4] = 42;
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 65536);
        uint8_t source[] = {0,  1,  2,  3};
        array.copy({source, 3}, 1);
        RED_CHECK(array.get_data());
        RED_CHECK_EQUAL(array.size(), 65536);
        RED_CHECK_EQUAL(array.get_data()[0], 42);
        RED_CHECK_EQUAL(array.get_data()[1], 0);
        RED_CHECK_EQUAL(array.get_data()[2], 1);
        RED_CHECK_EQUAL(array.get_data()[3], 2);
        RED_CHECK_EQUAL(array.get_data()[4], 42);
    }
}

RED_AUTO_TEST_CASE(TestSecIdentity)
{
    SEC_WINNT_AUTH_IDENTITY id;

    id.SetUserFromUtf8(byte_ptr_cast("Ménélas"));
    id.SetDomainFromUtf8(byte_ptr_cast("Sparte"));
    id.SetPasswordFromUtf8(byte_ptr_cast("Hélène"));
    RED_CHECK_MEM_C(id.User.av(), "M\0\xe9\0n\0\xe9\0l\0a\0s\0");
    RED_CHECK_MEM_C(id.Domain.av(), "S\0p\0a\0r\0t\0e\0");
    RED_CHECK_MEM_C(id.Password.av(), "H\0\xe9\0l\0\xe8\0n\0e\0");


    SEC_WINNT_AUTH_IDENTITY id2;

    id2.CopyAuthIdentity(id);
    RED_CHECK_MEM_C(id.User.av(), "M\0\xe9\0n\0\xe9\0l\0a\0s\0");
    RED_CHECK_MEM_C(id.Domain.av(), "S\0p\0a\0r\0t\0e\0");
    RED_CHECK_MEM_C(id.Password.av(), "H\0\xe9\0l\0\xe8\0n\0e\0");

    id2.clear();
    RED_CHECK_EQUAL(id2.User.size(), 0);
    RED_CHECK_EQUAL(id2.Domain.size(), 0);
    RED_CHECK_EQUAL(id2.Password.size(), 0);

    id.SetUserFromUtf8(byte_ptr_cast("Zeus"));
    RED_CHECK_MEM_C(id.User.av(), "Z\0e\0u\0s\0");
    id.SetDomainFromUtf8(byte_ptr_cast("Olympe"));
    RED_CHECK_MEM_C(id.Domain.av(), "O\0l\0y\0m\0p\0e\0");
    id.SetPasswordFromUtf8(byte_ptr_cast("Athéna"));
    RED_CHECK_MEM_C(id.Password.av(), "A\0t\0h\0\xe9\0n\0a\0");
}
