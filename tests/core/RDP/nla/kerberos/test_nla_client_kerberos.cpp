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
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/nla/nla_client_kerberos.hpp"
#include "utils/sugar/cast.hpp"

RED_AUTO_TEST_CASE(TestSecWinntAuthIdentityConstructor)
{
    SEC_WINNT_AUTH_IDENTITY id;

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(id.get_kerberos_password() == nullptr);

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_password_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));
}

RED_AUTO_TEST_CASE(TestSecWinntAuthIdentitySetNullPassword)
{
    SEC_WINNT_AUTH_IDENTITY id;

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(id.get_kerberos_password() == nullptr);

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_password_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));

    id.set_password_from_utf8(nullptr);

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(id.get_kerberos_password() == nullptr);

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_password_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));
}

RED_AUTO_TEST_CASE(TestSecWinntAuthIdentitySetPasswordFromUTF8)
{
    SEC_WINNT_AUTH_IDENTITY id;

    id.set_password_from_utf8(byte_ptr_cast("Password"));

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(!strcmp(id.get_kerberos_password(), "Password"));

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("Password", 8));

    RED_CHECK(id.get_password_utf16() ==
        bytes_view("P\x0" "a\x0""s\x0" "s\x0" "w\x0" "o\x0" "r\x0" "d\x0", 16));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));
}

RED_AUTO_TEST_CASE(TestSecWinntAuthIdentitySetPasswordFromUTF8_2)
{
    SEC_WINNT_AUTH_IDENTITY id;

    id.set_password_from_utf8(byte_ptr_cast("_123456789_123456789_123456789_123456789_123456789_123456789"));

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(!strcmp(id.get_kerberos_password(), "_123456789_123456789_123456789_123456789_123456789_123456789"));

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("_123456789_123456789_123456789_123456789_123456789_123456789", 60));

    RED_CHECK(id.get_password_utf16() ==
        bytes_view(
            "_\x0" "1\x0" "2\x0" "3\x0" "4\x0" "5\x0" "6\x0" "7\x0" "8\x0" "9\x0"
            "_\x0" "1\x0" "2\x0" "3\x0" "4\x0" "5\x0" "6\x0" "7\x0" "8\x0" "9\x0"
            "_\x0" "1\x0" "2\x0" "3\x0" "4\x0" "5\x0" "6\x0" "7\x0" "8\x0" "9\x0"
            "_\x0" "1\x0" "2\x0" "3\x0" "4\x0" "5\x0" "6\x0" "7\x0" "8\x0" "9\x0"
            "_\x0" "1\x0" "2\x0" "3\x0" "4\x0" "5\x0" "6\x0" "7\x0" "8\x0" "9\x0"
            "_\x0" "1\x0" "2\x0" "3\x0" "4\x0" "5\x0" "6\x0" "7\x0" "8\x0" "9\x0", 120));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));
}

RED_AUTO_TEST_CASE(TestSecWinntAuthIdentitySetPasswordFromUTF8_3)
{
    SEC_WINNT_AUTH_IDENTITY id;

    id.set_password_from_utf8(byte_ptr_cast("Password"));

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(!strcmp(id.get_kerberos_password(), "Password"));

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("Password", 8));

    RED_CHECK(id.get_password_utf16() ==
        bytes_view("P\x0" "a\x0""s\x0" "s\x0" "w\x0" "o\x0" "r\x0" "d\x0", 16));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));

    id.set_password_from_utf8(nullptr);

    RED_CHECK(!id.is_valid());

    RED_CHECK(!strcmp(id.get_kerberos_principal_name(), ""));

    RED_CHECK(!strcmp(id.get_kerberos_password(), ""));

    RED_CHECK(!strcmp(id.get_kerberos_keytab_path(), ""));

//    RED_CHECK(id.get_username_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_username_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_password_utf8() == bytes_view("Password", 0));

    RED_CHECK(id.get_password_utf16() == bytes_view("", 0));

//    RED_CHECK(id.get_domain_utf8() == bytes_view("", 0));

    RED_CHECK(id.get_domain_utf16() == bytes_view("", 0));
}

/*
RED_AUTO_TEST_CASE(TestTemp)
{
    std::vector<uint8_t> v;

    bytes_view bv(v);

    RED_CHECK(bv.data() != nullptr);
    RED_CHECK(bv.size() == 0);
}
*/