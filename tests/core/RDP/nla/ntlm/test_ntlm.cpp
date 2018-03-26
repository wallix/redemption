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


RED_AUTO_TEST_CASE(TestAcquireCredentials)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    Ntlm_SecurityFunctionTable table(rand, timeobj);
    SEC_STATUS status;
    uint8_t name[] = "Ménélas";
    uint8_t dom[] = "Sparte";
    uint8_t pass[] = "Hélène";
    SEC_WINNT_AUTH_IDENTITY id;
    id.SetAuthIdentityFromUtf8(name, dom, pass);

    // status = table.FreeCredentialsHandle(&credentials);
    // RED_CHECK_EQUAL(status, SEC_E_INVALID_HANDLE);
    // If AcquireCredential succeed, do not forget to free credential handle !
    status = table.AcquireCredentialsHandle(NTLMSP_NAME, SECPKG_CRED_OUTBOUND, nullptr, &id);


    RED_CHECK_EQUAL(status, SEC_E_OK);
    CREDENTIALS const * creds = table.getCredentialHandle();
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.User.get_data(), creds->identity.User.size()),
        "\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Domain.get_data(), creds->identity.Domain.size()),
        "\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Password.get_data(), creds->identity.Password.size()),
        "\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00");
}

RED_AUTO_TEST_CASE(TestInitialize)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    Ntlm_SecurityFunctionTable server_table(rand, timeobj);
    Ntlm_SecurityFunctionTable client_table(rand, timeobj);
    SEC_STATUS server_status;
    SEC_STATUS client_status;
    uint8_t const name[] = "Ménélas";
    uint8_t const dom[] = "Sparte";
    uint8_t const pass[] = "Hélène";
    SEC_WINNT_AUTH_IDENTITY server_id;
    server_id.SetAuthIdentityFromUtf8(name, dom, pass);
    SEC_WINNT_AUTH_IDENTITY client_id;
    client_id.SetAuthIdentityFromUtf8(name, dom, pass);

    // status = table.FreeCredentialsHandle(&credentials);
    // RED_CHECK_EQUAL(status, SEC_E_INVALID_HANDLE);

    // If AcquireCredential succeed, do not forget to free credential handle !
    server_status = server_table.AcquireCredentialsHandle(
        NTLMSP_NAME, SECPKG_CRED_OUTBOUND, nullptr, &server_id);
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
    client_status = client_table.AcquireCredentialsHandle(
        NTLMSP_NAME, SECPKG_CRED_OUTBOUND, nullptr, &client_id);
    RED_CHECK_EQUAL(client_status, SEC_E_OK);

    CREDENTIALS const * creds = server_table.getCredentialHandle();
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.User.get_data(), creds->identity.User.size()),
        "\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Domain.get_data(), creds->identity.Domain.size()),
        "\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Password.get_data(), creds->identity.Password.size()),
        "\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00");

    SecPkgInfo server_packageInfo;
    server_status = server_table.QuerySecurityPackageInfo(&server_packageInfo);
    RED_CHECK_EQUAL(server_status, SEC_E_OK);

    SecPkgInfo client_packageInfo;
    client_status = client_table.QuerySecurityPackageInfo(&client_packageInfo);
    RED_CHECK_EQUAL(client_status, SEC_E_OK);

    SecBuffer input_buffer;
    SecBuffer output_buffer;
    SecBufferDesc input_buffer_desc;
    SecBufferDesc output_buffer_desc;
    input_buffer.setzero();
    output_buffer.setzero();
    output_buffer_desc.ulVersion = SECBUFFER_VERSION;
    output_buffer_desc.cBuffers = 1;
    output_buffer_desc.pBuffers = &output_buffer;
    output_buffer.BufferType = SECBUFFER_TOKEN;
    output_buffer.Buffer.init(client_packageInfo.cbMaxToken);

    unsigned long fContextReq = 0;
    fContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

    // client first call, no input buffer, no context
    client_status = client_table.InitializeSecurityContext(
        nullptr, // TargetName
        fContextReq,
        nullptr, // input buffer desc
        0,
        &output_buffer_desc // output buffer desc
    );

    RED_CHECK_EQUAL(client_status, SEC_I_CONTINUE_NEEDED);

    RED_CHECK_EQUAL(output_buffer.Buffer.size(), 40);
    // hexdump_c(output_buffer.Buffer.get_data(), 40);

    unsigned long fsContextReq = 0;
    fsContextReq |= ASC_REQ_MUTUAL_AUTH;
    fsContextReq |= ASC_REQ_CONFIDENTIALITY;

    fsContextReq |= ASC_REQ_CONNECTION;
    fsContextReq |= ASC_REQ_USE_SESSION_KEY;

    fsContextReq |= ASC_REQ_REPLAY_DETECT;
    fsContextReq |= ASC_REQ_SEQUENCE_DETECT;

    fsContextReq |= ASC_REQ_EXTENDED_ERROR;

    input_buffer_desc.ulVersion = SECBUFFER_VERSION;
    input_buffer_desc.cBuffers = 1;
    input_buffer_desc.pBuffers = &input_buffer;
    input_buffer.BufferType = SECBUFFER_TOKEN;
    input_buffer.Buffer.init(client_packageInfo.cbMaxToken);

    // server first call, no context
    // got input buffer (output of client): Negotiate message
    server_status = server_table.AcceptSecurityContext(
        &output_buffer_desc, fsContextReq, &input_buffer_desc);

    RED_CHECK_EQUAL(server_status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(input_buffer.Buffer.size(), 120);
    // hexdump_c(input_buffer.Buffer.get_data(), 120);

    output_buffer_desc.ulVersion = SECBUFFER_VERSION;
    output_buffer_desc.cBuffers = 1;
    output_buffer_desc.pBuffers = &output_buffer;
    output_buffer.BufferType = SECBUFFER_TOKEN;
    output_buffer.Buffer.init(server_packageInfo.cbMaxToken);

    // client second call, got context
    // got input buffer: challenge message
    client_status = client_table.InitializeSecurityContext(
        nullptr, // TargetName
        fContextReq,
        &input_buffer_desc, // input buffer desc
        0,
        &output_buffer_desc // output buffer desc
    );

    RED_CHECK_EQUAL(client_status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(output_buffer.Buffer.size(), 262);
    // hexdump_c(output_buffer.Buffer.get_data(), 266);


    input_buffer_desc.ulVersion = SECBUFFER_VERSION;
    input_buffer_desc.cBuffers = 1;
    input_buffer_desc.pBuffers = &input_buffer;
    input_buffer.BufferType = SECBUFFER_TOKEN;
    input_buffer.Buffer.init(client_packageInfo.cbMaxToken);



    // server second call, got context
    // got input buffer (ouput of client): authenticate message
    server_status = server_table.AcceptSecurityContext(
        &output_buffer_desc, fsContextReq, &input_buffer_desc);

    RED_CHECK_EQUAL(server_status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(input_buffer.Buffer.size(), 0);

    // Check contexts
    NTLMContext const * client = client_table.getContextHandle();
    NTLMContext const * server = server_table.getContextHandle();

    // CHECK SHARED KEY ARE EQUAL BETWEEN SERVER AND CLIENT
    // LOG(LOG_INFO, "===== SESSION BASE KEY =====");
    // hexdump_c(server->SessionBaseKey, 16);
    // hexdump_c(client->SessionBaseKey, 16);
    RED_CHECK(!memcmp(server->SessionBaseKey,
                        client->SessionBaseKey,
                        16));

    // LOG(LOG_INFO, "===== EXPORTED SESSION KEY =====");
    // hexdump_c(server->ExportedSessionKey, 16);
    // hexdump_c(client->ExportedSessionKey, 16);
    RED_CHECK(!memcmp(server->ExportedSessionKey,
                        client->ExportedSessionKey,
                        16));
    // LOG(LOG_INFO, "===== CLIENT SIGNING KEY =====");
    // hexdump_c(server->ClientSigningKey, 16);
    // hexdump_c(client->ClientSigningKey, 16);
    RED_CHECK(!memcmp(server->ClientSigningKey,
                        client->ClientSigningKey,
                        16));

    // LOG(LOG_INFO, "===== CLIENT SEALING KEY =====");
    // hexdump_c(server->ClientSealingKey, 16);
    // hexdump_c(client->ClientSealingKey, 16);
    RED_CHECK(!memcmp(server->ClientSealingKey,
                        client->ClientSealingKey,
                        16));

    // LOG(LOG_INFO, "===== SERVER SIGNING KEY =====");
    // hexdump_c(server->ServerSigningKey, 16);
    // hexdump_c(client->ServerSigningKey, 16);
    RED_CHECK(!memcmp(server->ServerSigningKey,
                        client->ServerSigningKey,
                        16));

    // LOG(LOG_INFO, "===== SERVER SEALING KEY =====");
    // hexdump_c(server->ServerSealingKey, 16);
    // hexdump_c(client->ServerSealingKey, 16);
    RED_CHECK(!memcmp(server->ServerSealingKey,
                        client->ServerSealingKey,
                        16));

    // LOG(LOG_INFO, "===== Message Integrity Check =====");
    // hexdump_c(client->MessageIntegrityCheck, 16);
    // hexdump_c(server->MessageIntegrityCheck, 16);
    RED_CHECK(!memcmp(client->MessageIntegrityCheck,
                        server->MessageIntegrityCheck,
                        16));

    SecPkgContext_Sizes ContextSizes;
    server_table.QueryContextSizes(&ContextSizes);
    RED_CHECK_EQUAL(ContextSizes.cbMaxToken, 2010);
    RED_CHECK_EQUAL(ContextSizes.cbMaxSignature, 16);
    RED_CHECK_EQUAL(ContextSizes.cbBlockSize, 0);
    RED_CHECK_EQUAL(ContextSizes.cbSecurityTrailer, 16);


    RED_CHECK_EQUAL(server->confidentiality, client->confidentiality);
    RED_CHECK_EQUAL(server->confidentiality, true);
//     server->confidentiality = false;
//     client->confidentiality = false;
    // ENCRYPT
    uint8_t message[] = "$ds$qùdù*qsdlçàMessagetobeEncrypted !!!";
    SecBuffer Buffers[2];
    SecBufferDesc Message;
    Array Result;
    Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
    Buffers[1].BufferType = SECBUFFER_DATA;  /* TLS Public Key */
    Buffers[0].Buffer.init(ContextSizes.cbMaxSignature);
    Buffers[1].Buffer.init(sizeof(message));
    Buffers[1].Buffer.copy(message,
                           Buffers[1].Buffer.size());
    Message.cBuffers = 2;
    Message.ulVersion = SECBUFFER_VERSION;
    Message.pBuffers = Buffers;
    server_status = server_table.EncryptMessage(&Message, 0);
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
    Result.init(ContextSizes.cbMaxSignature + sizeof(message));

    Result.copy(Buffers[0].Buffer.get_data(),
                Buffers[0].Buffer.size());
    Result.copy(Buffers[1].Buffer.get_data(),
                Buffers[1].Buffer.size(),
                ContextSizes.cbMaxSignature);

    // LOG(LOG_INFO, "=== ENCRYPTION RESULT: size: %u, token: %u, data %u",
    //     Result.size(), ContextSizes.cbMaxSignature, sizeof(message));
    // hexdump_c(Result.get_data(), Result.size());

    // DECRYPT
    Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
    Buffers[1].BufferType = SECBUFFER_DATA; /* Encrypted TLS Public Key */
    Buffers[0].Buffer.init(ContextSizes.cbMaxSignature);
    Buffers[0].Buffer.copy(Result.get_data(),
                           ContextSizes.cbMaxSignature);
    Buffers[1].Buffer.init(Result.size() - ContextSizes.cbMaxSignature);
    Buffers[1].Buffer.copy(Result.get_data() + ContextSizes.cbMaxSignature,
                           Buffers[1].Buffer.size());
    Message.cBuffers = 2;
    Message.ulVersion = SECBUFFER_VERSION;
    Message.pBuffers = Buffers;
    client_status = client_table.DecryptMessage(&Message, 0);

    RED_CHECK_EQUAL(client_status, SEC_E_OK);

    server_status = server_table.ImpersonateSecurityContext();
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
    server_status = server_table.RevertSecurityContext();
    RED_CHECK_EQUAL(server_status, SEC_E_OK);
}
