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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSspi
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/nla/sspi.hpp"
#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestSecBuffer)
{
    SecBuffer a;
    a.setzero();
    BOOST_CHECK_EQUAL(a.BufferType, 0);
    BOOST_CHECK_EQUAL(a.Buffer.size(), 0);

    SecBuffer buffs[10];
    for (int i = 0; i < 10; i++) {
        buffs[i].setzero();
    }

    buffs[3].BufferType = SECBUFFER_EXTRA;
    buffs[3].Buffer.init(333);
    buffs[5].BufferType = SECBUFFER_ALERT;
    buffs[5].Buffer.init(555);
    buffs[7].BufferType = SECBUFFER_DATA;
    buffs[7].Buffer.init(777);

    SecBufferDesc buffdesc = {
        SECBUFFER_VERSION,
        10,
        buffs
    };

    PSecBuffer buff3 = buffdesc.FindSecBuffer(SECBUFFER_EXTRA);
    PSecBuffer buff5 = buffdesc.FindSecBuffer(SECBUFFER_ALERT);
    PSecBuffer buff7 = buffdesc.FindSecBuffer(SECBUFFER_DATA);
    PSecBuffer buffunknown = buffdesc.FindSecBuffer(SECBUFFER_CHANGE_PASS_RESPONSE);

    BOOST_CHECK_EQUAL(buff3->Buffer.size(), 333);
    BOOST_CHECK_EQUAL(buff5->Buffer.size(), 555);
    BOOST_CHECK_EQUAL(buff7->Buffer.size(), 777);
    BOOST_CHECK(buffunknown == NULL);
}

BOOST_AUTO_TEST_CASE(TestSecIdentity)
{
    uint8_t name[] = "Ménélas";
    uint8_t dom[] = "Sparte";
    uint8_t pass[] = "Hélène";

    SEC_WINNT_AUTH_IDENTITY id;
    id.SetAuthIdentityFromUtf8(name, dom, pass);

    // hexdump_c(name, sizeof(name));
    // hexdump_c(dom, sizeof(dom));
    // hexdump_c(pass, sizeof(pass));

    // hexdump_c(id.User.get_data(), id.User.size());
    // hexdump_c(id.Domain.get_data(), id.Domain.size());
    // hexdump_c(id.Password.get_data(), id.Password.size());

    BOOST_CHECK(!memcmp("\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00",
                        id.User.get_data(),
                        id.User.size()));
    BOOST_CHECK(!memcmp("\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00",
                        id.Domain.get_data(),
                        id.Domain.size()));
    BOOST_CHECK(!memcmp("\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00",
                        id.Password.get_data(),
                        id.Password.size()));

    SEC_WINNT_AUTH_IDENTITY id2;

    id2.CopyAuthIdentity(id);
    BOOST_CHECK(!memcmp("\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00",
                       id2.User.get_data(),
                       id2.User.size()));
    BOOST_CHECK(!memcmp("\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00",
                       id2.Domain.get_data(),
                       id2.Domain.size()));
    BOOST_CHECK(!memcmp("\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00",
                       id2.Password.get_data(),
                       id2.Password.size()));
}

BOOST_AUTO_TEST_CASE(TestSecFunctionTable)
{
    SecurityFunctionTable table;
    SEC_STATUS status;
    status = table.EnumerateSecurityPackages(NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.QueryCredentialsAttributes(NULL, 0, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.AcquireCredentialsHandle(NULL, NULL, 0, NULL,
                                            NULL, NULL, NULL, NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.FreeCredentialsHandle(NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.InitializeSecurityContext(NULL, NULL, NULL, 0, 0, 0, NULL,
                                             0, NULL, NULL, NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.AcceptSecurityContext(NULL, NULL, NULL, 0, 0, NULL, NULL, NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.CompleteAuthToken(NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.DeleteSecurityContext(NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.ApplyControlToken(NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.QueryContextAttributes(NULL, 0, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.ImpersonateSecurityContext(NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.RevertSecurityContext(NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.MakeSignature(NULL, 0, NULL, 0);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.FreeContextBuffer(NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.ExportSecurityContext(NULL, 0, NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.ImportSecurityContext(NULL, NULL, NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.AddCredentials(NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.QuerySecurityContextToken(NULL, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.EncryptMessage(NULL, 0, NULL, 0);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.DecryptMessage(NULL, NULL, 0, NULL);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.SetContextAttributes(NULL, 0, NULL, 0);
    BOOST_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    SecPkgInfo packageInfo = {};
    status = table.QuerySecurityPackageInfo(NTLMSP_NAME, &packageInfo);
    BOOST_CHECK_EQUAL(status, SEC_E_OK);
    BOOST_CHECK_EQUAL(packageInfo.fCapabilities, NTLM_SecPkgInfo.fCapabilities);
    BOOST_CHECK_EQUAL(packageInfo.wVersion, NTLM_SecPkgInfo.wVersion);
    BOOST_CHECK_EQUAL(packageInfo.wRPCID, NTLM_SecPkgInfo.wRPCID);
    BOOST_CHECK_EQUAL(packageInfo.cbMaxToken, NTLM_SecPkgInfo.cbMaxToken);

    SecPkgInfo packageInfo2 = {};
    status = table.QuerySecurityPackageInfo("KERBEROS", &packageInfo2);
    BOOST_CHECK_EQUAL(status, SEC_E_SECPKG_NOT_FOUND);

}
