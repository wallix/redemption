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

#define RED_TEST_MODULE TestNtlm
#include "system/redemption_unit_tests.hpp"

#include "core/RDP/nla/ntlm/ntlm.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/lcg_random.hpp"

using PasswordCallback = Ntlm_SecurityFunctionTable::PasswordCallback;

RED_AUTO_TEST_CASE(TestInitialize)
{
    LCGRandom rand(0);
    LCGTime timeobj;
    std::function<PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)> set_password_cb
      = [](auto&){ return PasswordCallback::Ok; };

    Ntlm_SecurityFunctionTable server_table(rand, timeobj, set_password_cb);
    Ntlm_SecurityFunctionTable client_table(rand, timeobj, set_password_cb);
    SEC_STATUS server_status;
    SEC_STATUS client_status;
    uint8_t const name[] = "Ménélas";
    uint8_t const dom[] = "Sparte";
    uint8_t const pass[] = "Hélène";
    SEC_WINNT_AUTH_IDENTITY client_server_id;
    client_server_id.SetAuthIdentityFromUtf8(name, dom, pass);

    // status = table.FreeCredentialsHandle(&credentials);
    // RED_CHECK_EQUAL(status, SEC_E_INVALID_HANDLE);

    // If AcquireCredential succeed, do not forget to free credential handle !
    server_status = server_table.AcquireCredentialsHandle(
        "NTLM", SECPKG_CRED_OUTBOUND, nullptr, &client_server_id);
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
    client_status = client_table.AcquireCredentialsHandle(
        "NTLM", SECPKG_CRED_OUTBOUND, nullptr, &client_server_id);
    RED_CHECK_EQUAL(client_status, SEC_E_OK);

    SecBuffer output_buffer;

    unsigned long fContextReq = 0;
    fContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

    // client first call, no input buffer, no context
    client_status = client_table.InitializeSecurityContext(
        nullptr, // TargetName
        fContextReq,
        {}, // input buffer desc
        0,
        output_buffer // output buffer desc
    );

    RED_CHECK_EQUAL(client_status, SEC_I_CONTINUE_NEEDED);

    RED_CHECK_EQUAL(output_buffer.size(), 40);
    // hexdump_c(output_buffer.get_data(), 40);

    unsigned long fsContextReq = 0;
    fsContextReq |= ASC_REQ_MUTUAL_AUTH;
    fsContextReq |= ASC_REQ_CONFIDENTIALITY;

    fsContextReq |= ASC_REQ_CONNECTION;
    fsContextReq |= ASC_REQ_USE_SESSION_KEY;

    fsContextReq |= ASC_REQ_REPLAY_DETECT;
    fsContextReq |= ASC_REQ_SEQUENCE_DETECT;

    fsContextReq |= ASC_REQ_EXTENDED_ERROR;

    SecBuffer input_buffer;

    // server first call, no context
    // got input buffer (output of client): Negotiate message
    server_status = server_table.AcceptSecurityContext(
        output_buffer.av(), fsContextReq, input_buffer);

    RED_CHECK_EQUAL(server_status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(input_buffer.size(), 120);
    // hexdump_c(input_buffer.get_data(), 120);

    // client second call, got context
    // got input buffer: challenge message
    client_status = client_table.InitializeSecurityContext(
        nullptr, // TargetName
        fContextReq,
        input_buffer.av(), // input buffer desc
        0,
        output_buffer // output buffer desc
    );

    RED_CHECK_EQUAL(client_status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(output_buffer.size(), 266);
    // hexdump_c(output_buffer.get_data(), 266);


    // server second call, got context
    // got input buffer (ouput of client): authenticate message
    server_status = server_table.AcceptSecurityContext(
        output_buffer.av(), fsContextReq, input_buffer);

    RED_CHECK_EQUAL(server_status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(input_buffer.size(), 0);

    // Check contexts
    NTLMContext const * client = client_table.getContextHandle();
    NTLMContext const * server = server_table.getContextHandle();

    // CHECK SHARED KEY ARE EQUAL BETWEEN SERVER AND CLIENT
    // LOG(LOG_INFO, "===== SESSION BASE KEY =====");
    // hexdump_c(server->SessionBaseKey, 16);
    // hexdump_c(client->SessionBaseKey, 16);
    RED_CHECK_MEM_AA(server->SessionBaseKey, client->SessionBaseKey);

    // LOG(LOG_INFO, "===== EXPORTED SESSION KEY =====");
    // hexdump_c(server->ExportedSessionKey, 16);
    // hexdump_c(client->ExportedSessionKey, 16);
    RED_CHECK_MEM_AA(server->ExportedSessionKey, client->ExportedSessionKey);
    // LOG(LOG_INFO, "===== CLIENT SIGNING KEY =====");
    // hexdump_c(server->ClientSigningKey, 16);
    // hexdump_c(client->ClientSigningKey, 16);
    RED_CHECK_MEM_AA(server->ClientSigningKey, client->ClientSigningKey);

    // LOG(LOG_INFO, "===== CLIENT SEALING KEY =====");
    // hexdump_c(server->ClientSealingKey, 16);
    // hexdump_c(client->ClientSealingKey, 16);
    RED_CHECK_MEM_AA(server->ClientSealingKey, client->ClientSealingKey);

    // LOG(LOG_INFO, "===== SERVER SIGNING KEY =====");
    // hexdump_c(server->ServerSigningKey, 16);
    // hexdump_c(client->ServerSigningKey, 16);
    RED_CHECK_MEM_AA(server->ServerSigningKey, client->ServerSigningKey);

    // LOG(LOG_INFO, "===== SERVER SEALING KEY =====");
    // hexdump_c(server->ServerSealingKey, 16);
    // hexdump_c(client->ServerSealingKey, 16);
    RED_CHECK_MEM_AA(server->ServerSealingKey, client->ServerSealingKey);

    // LOG(LOG_INFO, "===== Message Integrity Check =====");
    // hexdump_c(client->MessageIntegrityCheck, 16);
    // hexdump_c(server->MessageIntegrityCheck, 16);
    RED_CHECK_MEM_AA(client->MessageIntegrityCheck, server->MessageIntegrityCheck);

    // RED_CHECK_EQUAL(server->confidentiality, client->confidentiality);
    // RED_CHECK_EQUAL(server->confidentiality, true);
//     server->confidentiality = false;
//     client->confidentiality = false;
    // ENCRYPT
    uint8_t message[] = "$ds$qùdù*qsdlçàMessagetobeEncrypted !!!";
    SecBuffer Result;
    server_status = server_table.EncryptMessage(message, Result, 0);
    RED_CHECK_EQUAL(server_status, SEC_E_OK);

    const unsigned cbMaxSignature = 16u;

    // LOG(LOG_INFO, "=== ENCRYPTION RESULT: size: %u, token: %u, data %u",
    //     Result.size(), cbMaxSignature, sizeof(message));
    // hexdump_c(Result.get_data(), Result.size());

    // DECRYPT
    SecBuffer Result2;
    client_status = client_table.DecryptMessage({Result.get_data(), Result.size()}, Result2, 0);

    RED_CHECK_EQUAL(Result.size(), make_array_view(message).size() + cbMaxSignature);
    RED_CHECK(0 != memcmp(Result.get_data(), message, Result.size() - cbMaxSignature));
    RED_CHECK_MEM(Result2.av(), make_array_view(message));

    RED_CHECK_EQUAL(client_status, SEC_E_OK);

    server_status = server_table.ImpersonateSecurityContext();
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
    server_status = server_table.RevertSecurityContext();
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
}
