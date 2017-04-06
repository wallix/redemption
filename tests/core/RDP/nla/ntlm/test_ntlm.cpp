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
#define BOOST_TEST_MODULE TestNtlm
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
#include "core/RDP/nla/ntlm/ntlm.hpp"
#include "check_sig.hpp"


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
    CredHandle credentials;
    TimeStamp expiration;

    // status = table.FreeCredentialsHandle(&credentials);
    // RED_CHECK_EQUAL(status, SEC_E_INVALID_HANDLE);
    // If AcquireCredential succeed, do not forget to free credential handle !
    status = table.AcquireCredentialsHandle(nullptr, NTLMSP_NAME, SECPKG_CRED_OUTBOUND, nullptr,
                                            &id, nullptr, nullptr,
                                            &credentials, &expiration);


    RED_CHECK_EQUAL(status, SEC_E_OK);
    CREDENTIALS * creds = reinterpret_cast<CREDENTIALS*>(credentials.SecureHandleGetLowerPointer());
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.User.get_data(), creds->identity.User.size()),
        "\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Domain.get_data(), creds->identity.Domain.size()),
        "\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Password.get_data(), creds->identity.Password.size()),
        "\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00");

    status = table.FreeCredentialsHandle(&credentials);
    RED_CHECK_EQUAL(status, SEC_E_OK);
}

RED_AUTO_TEST_CASE(TestInitialize)
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
    CredHandle credentials;
    TimeStamp expiration;

    // status = table.FreeCredentialsHandle(&credentials);
    // RED_CHECK_EQUAL(status, SEC_E_INVALID_HANDLE);

    // If AcquireCredential succeed, do not forget to free credential handle !
    status = table.AcquireCredentialsHandle(nullptr, NTLMSP_NAME, SECPKG_CRED_OUTBOUND, nullptr,
                                            &id, nullptr, nullptr,
                                            &credentials, &expiration);
    RED_CHECK_EQUAL(status, SEC_E_OK);

    CREDENTIALS * creds = reinterpret_cast<CREDENTIALS*>(credentials.SecureHandleGetLowerPointer());
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.User.get_data(), creds->identity.User.size()),
        "\x4d\x00\xe9\x00\x6e\x00\xe9\x00\x6c\x00\x61\x00\x73\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Domain.get_data(), creds->identity.Domain.size()),
        "\x53\x00\x70\x00\x61\x00\x72\x00\x74\x00\x65\x00");
    RED_CHECK_MEM_C(
        make_array_view(creds->identity.Password.get_data(), creds->identity.Password.size()),
        "\x48\x00\xe9\x00\x6c\x00\xe8\x00\x6e\x00\x65\x00");

    SecPkgInfo packageInfo;
    status = table.QuerySecurityPackageInfo(NTLMSP_NAME, &packageInfo);
    RED_CHECK_EQUAL(status, SEC_E_OK);

    CtxtHandle client_context;
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
    output_buffer.Buffer.init(packageInfo.cbMaxToken);

    unsigned long fContextReq = 0;
    fContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

    // client first call, no input buffer, no context
    status = table.InitializeSecurityContext(&credentials,
                                             nullptr, // context
                                             nullptr, // TargetName
                                             fContextReq, SECURITY_NATIVE_DREP,
                                             nullptr, // input buffer desc
                                             0, &client_context, // context (NTLMContext)
                                             &output_buffer_desc, // output buffer desc
                                             &expiration);

    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);

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
    input_buffer.Buffer.init(packageInfo.cbMaxToken);

    CtxtHandle server_context;
    // server first call, no context
    // got input buffer (output of client): Negotiate message
    status = table.AcceptSecurityContext(&credentials, nullptr,
                                         &output_buffer_desc, fsContextReq,
                                         SECURITY_NATIVE_DREP, &server_context,
                                         &input_buffer_desc, &expiration);

    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(input_buffer.Buffer.size(), 120);
    // hexdump_c(input_buffer.Buffer.get_data(), 120);

    output_buffer_desc.ulVersion = SECBUFFER_VERSION;
    output_buffer_desc.cBuffers = 1;
    output_buffer_desc.pBuffers = &output_buffer;
    output_buffer.BufferType = SECBUFFER_TOKEN;
    output_buffer.Buffer.init(packageInfo.cbMaxToken);

    // client second call, got context
    // got input buffer: challenge message
    status = table.InitializeSecurityContext(&credentials,
                                             &client_context, // context
                                             nullptr, // TargetName
                                             fContextReq, SECURITY_NATIVE_DREP,
                                             &input_buffer_desc, // input buffer desc
                                             0, &client_context, // context (NTLMContext)
                                             &output_buffer_desc, // output buffer desc
                                             &expiration);

    RED_CHECK_EQUAL(status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(output_buffer.Buffer.size(), 266);
    // hexdump_c(output_buffer.Buffer.get_data(), 266);


    input_buffer_desc.ulVersion = SECBUFFER_VERSION;
    input_buffer_desc.cBuffers = 1;
    input_buffer_desc.pBuffers = &input_buffer;
    input_buffer.BufferType = SECBUFFER_TOKEN;
    input_buffer.Buffer.init(packageInfo.cbMaxToken);



    // server second call, got context
    // got input buffer (ouput of client): authenticate message
    status = table.AcceptSecurityContext(&credentials, &server_context,
                                         &output_buffer_desc, fsContextReq,
                                         SECURITY_NATIVE_DREP, &server_context,
                                         &input_buffer_desc, &expiration);

    RED_CHECK_EQUAL(status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(input_buffer.Buffer.size(), 0);

    // Check contexts
    NTLMContext * client = reinterpret_cast<NTLMContext*>(client_context.SecureHandleGetLowerPointer());
    NTLMContext * server = reinterpret_cast<NTLMContext*>(server_context.SecureHandleGetLowerPointer());

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
    table.QueryContextAttributes(nullptr, SECPKG_ATTR_SIZES, &ContextSizes);
    RED_CHECK_EQUAL(ContextSizes.cbMaxToken, 2010);
    RED_CHECK_EQUAL(ContextSizes.cbMaxSignature, 16);
    RED_CHECK_EQUAL(ContextSizes.cbBlockSize, 0);
    RED_CHECK_EQUAL(ContextSizes.cbSecurityTrailer, 16);


    RED_CHECK_EQUAL(server->confidentiality, client->confidentiality);
    RED_CHECK_EQUAL(server->confidentiality, true);
    server->confidentiality = false;
    client->confidentiality = false;
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
    status = table.EncryptMessage(&server_context, 0, &Message, 0);
    RED_CHECK_EQUAL(status, SEC_E_OK);
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
    unsigned long pfQOP = 0;
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
    status = table.DecryptMessage(&client_context, &Message, 0, &pfQOP);

    RED_CHECK_EQUAL(status, SEC_E_OK);

    status = table.ImpersonateSecurityContext(&server_context);
    RED_CHECK_EQUAL(status, SEC_E_OK);
    status = table.RevertSecurityContext(&server_context);
    RED_CHECK_EQUAL(status, SEC_E_OK);

    // clear handles
    status = table.FreeContextBuffer(&server_context);
    RED_CHECK_EQUAL(status, SEC_E_OK);
    status = table.FreeContextBuffer(&client_context);
    RED_CHECK_EQUAL(status, SEC_E_OK);
    status = table.FreeCredentialsHandle(&credentials);
    RED_CHECK_EQUAL(status, SEC_E_OK);

}
