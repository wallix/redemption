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

#define RED_TEST_MODULE TestKerberos
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/nla/kerberos/kerberos.hpp"

RED_AUTO_TEST_CASE(TestAcquireCredentials)
{
    Kerberos_SecurityFunctionTable table;
    uint8_t name[] = "tartempion";
    uint8_t pass[] = "a";
    SEC_WINNT_AUTH_IDENTITY id;
    id.SetKrbAuthIdentity(name, pass);

    // SEC_STATUS status = table.FreeCredentialsHandle(&credentials);
    // RED_CHECK_EQUAL(status, SEC_E_INVALID_HANDLE);
    // If AcquireCredential succeed, do not forget to free credential handle !
    // status = table.AcquireCredentialsHandle("NTLM", nullptr, &id);

    // RED_CHECK_EQUAL(status, SEC_E_NO_CREDENTIALS);
    // CREDENTIALS * creds = (CREDENTIALS*)credentials.SecureHandleGetLowerPointer();
    // RED_CHECK(!memcmp("\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00",
    //                     creds->identity.User.get_data(),
    //                     creds->identity.User.size()));
    // RED_CHECK(!memcmp("\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00",
    //                     creds->identity.Domain.get_data(),
    //                     creds->identity.Domain.size()));
    // RED_CHECK(!memcmp("\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00",
    //                     creds->identity.Password.get_data(),
    //                     creds->identity.Password.size()));
}
