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
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT
#include "core/RDP/nla/sspi.hpp"



RED_AUTO_TEST_CASE(Test_Array)
{
    RED_CHECK(true);
    bool test_equal(false);

    Array array_0;
    test_equal = false;
    RED_CHECK_EQUAL(array_0.size(), 65536);
    if (array_0.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);


    Array array_1(42);
    test_equal = false;
    RED_CHECK_EQUAL(array_1.size(), 42);
    if (array_1.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);


    Array array_2(65536 + 1);
    test_equal = false;
    RED_CHECK_EQUAL(array_2.size(), 65537);
    if (array_2.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);


    Array array_3;
    test_equal = false;
    RED_CHECK_EQUAL(array_3.size(), 65536);
    if (array_3.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);
    array_3.copy(array_1);
    test_equal = false;
    RED_CHECK_EQUAL(array_3.size(), 42);
    if (array_3.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);


    Array array_4;
    test_equal = false;
    RED_CHECK_EQUAL(array_4.size(), 65536);
    if (array_4.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);
    array_4.init(42);
    test_equal = false;
    RED_CHECK_EQUAL(array_4.size(), 42);
    if (array_4.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);


    Array array_5;
    test_equal = false;
    RED_CHECK_EQUAL(array_5.size(), 65536);
    if (array_5.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);
    uint8_t source[] = {0,  1,  2,  3};
    array_5.copy(source, 3, 1);
    test_equal = false;
    RED_CHECK_EQUAL(array_5.size(), 65536);
    if (array_5.get_data() != nullptr) {
        test_equal = true;
    }
    RED_CHECK_EQUAL(test_equal, true);
    RED_CHECK_EQUAL(array_5.get_data()[0], 0);
    RED_CHECK_EQUAL(array_5.get_data()[1], 0);
    RED_CHECK_EQUAL(array_5.get_data()[2], 1);

}

RED_AUTO_TEST_CASE(TestSecBuffer)
{
    SecBuffer a;
    a.setzero();
    RED_CHECK_EQUAL(a.BufferType, 0);
    RED_CHECK_EQUAL(a.Buffer.size(), 0);

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

    RED_CHECK_EQUAL(buff3->Buffer.size(), 333);
    RED_CHECK_EQUAL(buff5->Buffer.size(), 555);
    RED_CHECK_EQUAL(buff7->Buffer.size(), 777);
    RED_CHECK(buffunknown == nullptr);


}

RED_AUTO_TEST_CASE(TestSecIdentity)
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

    RED_CHECK(!memcmp("\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00",
                        id.User.get_data(),
                        id.User.size()));
    RED_CHECK(!memcmp("\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00",
                        id.Domain.get_data(),
                        id.Domain.size()));
    RED_CHECK(!memcmp("\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00",
                        id.Password.get_data(),
                        id.Password.size()));

    SEC_WINNT_AUTH_IDENTITY id2;


    id2.CopyAuthIdentity(id);
    RED_CHECK(!memcmp("\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00",
                       id2.User.get_data(),
                       id2.User.size()));
    RED_CHECK(!memcmp("\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00",
                       id2.Domain.get_data(),
                       id2.Domain.size()));
    RED_CHECK(!memcmp("\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00",
                       id2.Password.get_data(),
                       id2.Password.size()));

    id2.clear();
    RED_CHECK_EQUAL(id2.User.size(), 0);
    RED_CHECK_EQUAL(id2.Domain.size(), 0);
    RED_CHECK_EQUAL(id2.Password.size(), 0);


    id.SetUserFromUtf8(reinterpret_cast<const uint8_t *>("Zeus"));
    RED_CHECK(!memcmp("\x5a\x00\x65\x00\x75\x00\x73\x00",
                        id.User.get_data(),
                        id.User.size()));
    id.SetDomainFromUtf8(reinterpret_cast<const uint8_t *>("Olympe"));
    RED_CHECK(!memcmp("\x4f\x00\x6c\x00\x79\x00\x6d\x00\x70\x00\x65\x00",
                        id.Domain.get_data(),
                        id.Domain.size()));
    id.SetPasswordFromUtf8(reinterpret_cast<const uint8_t *>("Athéna"));
    RED_CHECK(!memcmp("\x41\x00\x74\x00\x68\x00\xe9\x00\x6e\x00\x61\x00",
                        id.Password.get_data(),
                        id.Password.size()));
    // hexdump_c(id.User.get_data(), id.User.size());
    // hexdump_c(id.Domain.get_data(), id.Domain.size());
    // hexdump_c(id.Password.get_data(), id.Password.size());
}

RED_AUTO_TEST_CASE(TestSecureHandle)
{
    SecHandle handle;
    unsigned long a = 521354;
    SecBuffer buff;
    buff.Buffer.init(462);

    handle.SecureHandleSetLowerPointer(&a);
    handle.SecureHandleSetUpperPointer(&buff);

    unsigned long * b = reinterpret_cast<unsigned long *>(handle.SecureHandleGetLowerPointer());
    PSecBuffer buffimport = reinterpret_cast<PSecBuffer>(handle.SecureHandleGetUpperPointer());

    RED_CHECK_EQUAL(*b, a);
    RED_CHECK_EQUAL(buffimport->Buffer.size(), buff.Buffer.size());

    SecHandle handle2(handle);

    unsigned long * b2 = reinterpret_cast<unsigned long *>(handle2.SecureHandleGetLowerPointer());
    PSecBuffer buffimport2 = reinterpret_cast<PSecBuffer>(handle2.SecureHandleGetUpperPointer());

    RED_CHECK_EQUAL(*b2, a);
    RED_CHECK_EQUAL(buffimport2->Buffer.size(), buff.Buffer.size());

    SecHandle handle3;

    handle3 = handle2;

    unsigned long * b3 = reinterpret_cast<unsigned long *>(handle3.SecureHandleGetLowerPointer());
    PSecBuffer buffimport3 = reinterpret_cast<PSecBuffer>(handle3.SecureHandleGetUpperPointer());

    RED_CHECK_EQUAL(*b3, a);
    RED_CHECK_EQUAL(buffimport3->Buffer.size(), buff.Buffer.size());

}


RED_AUTO_TEST_CASE(TestSecFunctionTable)
{
    SecurityFunctionTable table;
    SEC_STATUS status;
    //status = table.EnumerateSecurityPackages(nullptr, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.QueryCredentialsAttributes(nullptr, 0, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.AcquireCredentialsHandle(nullptr, 0, nullptr, nullptr);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.InitializeSecurityContext(nullptr, 0, nullptr, 0, nullptr);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.AcceptSecurityContext(nullptr, 0, nullptr);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.CompleteAuthToken(nullptr);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.DeleteSecurityContext(nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.ApplyControlToken(nullptr, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.QueryContextAttributes(0, nullptr);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.ImpersonateSecurityContext();
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.RevertSecurityContext();
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.MakeSignature(nullptr, 0, nullptr, 0);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.VerifySignature(nullptr, nullptr, 0, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.ExportSecurityContext(nullptr, 0, nullptr, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.ImportSecurityContext(nullptr, nullptr, nullptr);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.AddCredentials(nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.QuerySecurityContextToken(nullptr, nullptr);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.EncryptMessage(nullptr, 0);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    status = table.DecryptMessage(nullptr, 0);
    RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    //status = table.SetContextAttributes(nullptr, 0, nullptr, 0);
    //RED_CHECK_EQUAL(status, SEC_E_UNSUPPORTED_FUNCTION);

    SecPkgInfo packageInfo;
    status = table.QuerySecurityPackageInfo(NTLMSP_NAME, &packageInfo);
    RED_CHECK_EQUAL(status, SEC_E_SECPKG_NOT_FOUND);
    // RED_CHECK_EQUAL(packageInfo.fCapabilities, NTLM_SecPkgInfo.fCapabilities);
    // RED_CHECK_EQUAL(packageInfo.wVersion, NTLM_SecPkgInfo.wVersion);
    // RED_CHECK_EQUAL(packageInfo.wRPCID, NTLM_SecPkgInfo.wRPCID);
    // RED_CHECK_EQUAL(packageInfo.cbMaxToken, NTLM_SecPkgInfo.cbMaxToken);

    SecPkgInfo packageInfo2;
    status = table.QuerySecurityPackageInfo("KERBEROS", &packageInfo2);
    RED_CHECK_EQUAL(status, SEC_E_SECPKG_NOT_FOUND);

}
