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

#ifndef _REDEMPTION_CORE_RDP_NLA_NLA_HPP_
#define _REDEMPTION_CORE_RDP_NLA_NLA_HPP_

#include "RDP/nla/sspi.hpp"
#include "RDP/nla/credssp.hpp"
#include "RDP/nla/ntlm/ntlm.hpp"

#define NLA_PKG_NAME NTLMSP_NAME

SecurityFunctionTable * InitSecurityInterface(SecInterface secInter) {
    SecurityFunctionTable * table = NULL;
    if (secInter == NTLM_Interface) {
        table = new Ntlm_SecurityFunctionTable;
    }
    return table;
}

struct rdpCredssp
{
    bool server;
    int send_seq_num;
    int recv_seq_num;
    // freerdp* instance;
    CtxtHandle context;

    Array SspiModule;

    // rdpSettings* settings;
    // rdpTransport* transport;

    TSCredentials ts_credentials;
    TSRequest ts_request;
    Array & negoToken;
    Array & pubKeyAuth;
    Array & authInfo;
    Array PublicKey;

    // CryptoRc4 rc4_seal_state;
    Array ServicePrincipalName;
    SEC_WINNT_AUTH_IDENTITY identity;
    PSecurityFunctionTable table;
    SecPkgContext_Sizes ContextSizes;


    rdpCredssp()
        : negoToken(ts_request.negoTokens)
        , pubKeyAuth(ts_request.pubKeyAuth)
        , authInfo(ts_request.authInfo)
    {}

    ~rdpCredssp()
    {
        this->table->FreeContextBuffer(&this->context);
        if (this->table) {
            delete this->table;
            this->table = NULL;
        }
    }

    int credssp_ntlm_client_init() {
        // char* spn;
        // int length;
        // bool PromptPassword;
        // rdpTls* tls = NULL;
        // freerdp* instance;
        // rdpSettings* settings;

//         PromptPassword = false;
//         settings = this->settings;
//         instance = (freerdp*) settings->instance;

//         if ((!settings->Password) || (!settings->Username)
//             || (!strlen(settings->Password)) || (!strlen(settings->Username))) {
//             PromptPassword = true;
//         }

// #ifndef _WIN32
//         if (PromptPassword) {
//             if (settings->RestrictedAdminModeRequired) {
//                 if ((settings->PasswordHash) && (strlen(settings->PasswordHash) > 0))
//                     PromptPassword = false;
//             }
//         }
// #endif

//         if (PromptPassword) {
//             if (instance->Authenticate) {
//                 BOOL proceed = instance->Authenticate(instance,
//                                                       &settings->Username, &settings->Password, &settings->Domain);

//                 if (!proceed)
//                     return 0;
//             }
//         }

        // this->identity.SetAuthIdentity(settings->Username, settings->Domain,
        //                                settings->Password);
        this->identity.SetAuthIdentity(NULL, NULL, NULL);

// #ifndef _WIN32
//         {
//             SEC_WINNT_AUTH_IDENTITY* identity = &(this->identity);

//             if (settings->RestrictedAdminModeRequired) {
//                 if (settings->PasswordHash) {
//                     if (strlen(settings->PasswordHash) == 32) {
//                         if (identity->Password)
//                             free(identity->Password);

//                         identity->PasswordLength = ConvertToUnicode(CP_UTF8, 0,
//                                                                     settings->PasswordHash,
//                                                                     -1, &identity->Password,
//                                                                     0) - 1;

//                         /**
//                          * Multiply password hash length by 64 to obtain a length exceeding
//                          * the maximum (256) and use it this for hash identification in WinPR.
//                          */
//                         identity->PasswordLength = 32 * 64; /* 2048 */
//                     }
//                 }
//             }
//         }
// #endif

// ============================================
/* Get Public Key From TLS Layer and hostname */
// ============================================

//         if (this->transport->layer == TRANSPORT_LAYER_TLS) {
//             tls = this->transport->TlsIn;
//         }
//         else if (this->transport->layer == TRANSPORT_LAYER_TSG_TLS) {
//             tls = this->transport->TsgTls;
//         }
//         else {
//             fprintf(stderr, "Unknown NLA transport layer\n");
//             return 0;
//         }

//         sspi_SecBufferAlloc(&this->PublicKey, tls->PublicKeyLength);
//         CopyMemory(this->PublicKey.pvBuffer, tls->PublicKey, tls->PublicKeyLength);

//         length = sizeof(TERMSRV_SPN_PREFIX) + strlen(settings->ServerHostname);

//         spn = (SEC_CHAR*) malloc(length + 1);
//         sprintf(spn, "%s%s", TERMSRV_SPN_PREFIX, settings->ServerHostname);

// #ifdef UNICODE
//         this->ServicePrincipalName = (LPTSTR) malloc(length * 2 + 2);
//         MultiByteToWideChar(CP_UTF8, 0, spn, length,
//                             (LPWSTR) this->ServicePrincipalName, length);
//         free(spn);
// #else
//         this->ServicePrincipalName = spn;
// #endif

        return 1;

    }

    void ap_integer_increment_le(uint8_t* number, size_t size) {
        size_t index = 0;

        for (index = 0; index < size; index++) {
            if (number[index] < 0xFF) {
                number[index]++;
                break;
            }
            else {
                number[index] = 0;
                continue;
            }
        }
    }

    void ap_integer_decrement_le(uint8_t* number, size_t size) {
        size_t index = 0;
        for (index = 0; index < size; index++) {
            if (number[index] > 0) {
                number[index]--;
                break;
            }
            else {
                number[index] = 0xFF;
                continue;
            }
        }
    }


    SEC_STATUS credssp_encrypt_public_key_echo() {
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;
        int public_key_length;

        public_key_length = this->PublicKey.size();

        Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
        Buffers[1].BufferType = SECBUFFER_DATA;  /* TLS Public Key */

        this->pubKeyAuth.init(this->ContextSizes.cbMaxSignature + public_key_length);

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        memcpy(Buffers[0].Buffer.get_data(), this->pubKeyAuth.get_data(),
               Buffers[0].Buffer.size());

        Buffers[1].Buffer.init(public_key_length);

        memcpy(Buffers[1].Buffer.get_data(), this->PublicKey.get_data(),
               Buffers[1].Buffer.size());

        if (this->server) {
            /* server echos the public key +1 */
            this->ap_integer_increment_le(Buffers[1].Buffer.get_data(), Buffers[1].Buffer.size());
        }

        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->EncryptMessage(&this->context, 0, &Message, this->send_seq_num++);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "EncryptMessage status: 0x%08X\n", status);
            return status;
        }

        memcpy(this->pubKeyAuth.get_data(),
               Buffers[0].Buffer.get_data(),
               Buffers[0].Buffer.size());
        memcpy(this->pubKeyAuth.get_data() + this->ContextSizes.cbMaxSignature,
               Buffers[1].Buffer.get_data(),
               Buffers[1].Buffer.size());

        return status;

    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        int length;
        unsigned long pfQOP = 0;
        uint8_t* public_key1;
        uint8_t* public_key2;
        int public_key_length;
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;

        if (this->PublicKey.size() + this->ContextSizes.cbMaxSignature != this->pubKeyAuth.size()) {
            fprintf(stderr, "unexpected pubKeyAuth buffer size:%d\n",
                    (int) this->pubKeyAuth.size());
            return SEC_E_INVALID_TOKEN;
        }

        length = this->pubKeyAuth.size();

        public_key_length = this->PublicKey.size();

        Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
        Buffers[1].BufferType = SECBUFFER_DATA; /* Encrypted TLS Public Key */

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        memcpy(Buffers[0].Buffer.get_data(),
               this->pubKeyAuth.get_data(),
               this->ContextSizes.cbMaxSignature);

        Buffers[1].Buffer.init(length - this->ContextSizes.cbMaxSignature);
        memcpy(Buffers[1].Buffer.get_data(),
               this->pubKeyAuth.get_data() + this->ContextSizes.cbMaxSignature,
               Buffers[1].Buffer.size());

        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->DecryptMessage(&this->context, &Message, this->recv_seq_num++, &pfQOP);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "DecryptMessage failure: 0x%08X\n", status);
            return status;
        }

        public_key1 = this->PublicKey.get_data();
        public_key2 = Buffers[1].Buffer.get_data();

        if (!this->server) {
            /* server echos the public key +1 */
            ap_integer_decrement_le(public_key2, public_key_length);
        }

        if (memcmp(public_key1, public_key2, public_key_length) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo\n");

            LOG(LOG_ERR, "Expected (length = %d):\n", public_key_length);
            hexdump_c(public_key1, public_key_length);

            LOG(LOG_ERR, "Actual (length = %d):\n", public_key_length);
            hexdump_c(public_key2, public_key_length);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        return SEC_E_OK;
    }

    void credssp_encode_ts_credentials() {
        this->ts_credentials.set_credentials(this->identity.Domain.get_data(),
                                             this->identity.Domain.size(),
                                             this->identity.User.get_data(),
                                             this->identity.User.size(),
                                             this->identity.Password.get_data(),
                                             this->identity.Password.size());
    }

    SEC_STATUS credssp_encrypt_ts_credentials() {
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;

        this->credssp_encode_ts_credentials();

        BStream ts_credentials_send;
        this->ts_credentials.emit(ts_credentials_send);

        Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
        Buffers[1].BufferType = SECBUFFER_DATA;  /* TSCredentials */

        //     sspi_SecBufferAlloc(&credssp->authInfo, credssp->ContextSizes.cbMaxSignature + credssp->ts_credentials.cbBuffer);
        this->authInfo.init(this->ContextSizes.cbMaxSignature + ts_credentials_send.size());

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        memset(Buffers[0].Buffer.get_data(), 0, Buffers[0].Buffer.size());
        //     Buffers[0].cbBuffer = credssp->ContextSizes.cbMaxSignature;
        //     Buffers[0].pvBuffer = credssp->authInfo.pvBuffer;
        //     ZeroMemory(Buffers[0].pvBuffer, Buffers[0].cbBuffer);

        Buffers[1].Buffer.init(ts_credentials_send.size());
        memcpy(Buffers[1].Buffer.get_data(), ts_credentials_send.get_data(),
               ts_credentials_send.size());

        //     Buffers[1].cbBuffer = credssp->ts_credentials.cbBuffer;
        //     Buffers[1].pvBuffer = &((BYTE*) credssp->authInfo.pvBuffer)[Buffers[0].cbBuffer];
        //     CopyMemory(Buffers[1].pvBuffer, credssp->ts_credentials.pvBuffer, Buffers[1].cbBuffer);

        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->EncryptMessage(&this->context, 0, &Message, this->send_seq_num++);

        if (status != SEC_E_OK)
            return status;

        this->authInfo.init(this->ContextSizes.cbMaxSignature + ts_credentials_send.size());
        memcpy(this->authInfo.get_data(),
               Buffers[0].Buffer.get_data(),
               Buffers[0].Buffer.size());
        memcpy(this->authInfo.get_data() + this->ContextSizes.cbMaxSignature,
               Buffers[1].Buffer.get_data(),
               Buffers[1].Buffer.size());

        return SEC_E_OK;
    }

    SEC_STATUS credssp_decrypt_ts_credentials() {
        int length;
        unsigned long pfQOP = 0;
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;

        Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
        Buffers[1].BufferType = SECBUFFER_DATA; /* TSCredentials */

        if (this->authInfo.size() < 1) {
            LOG(LOG_ERR, "credssp_decrypt_ts_credentials missing authInfo buffer\n");
            return SEC_E_INVALID_TOKEN;
        }

        length = this->authInfo.size();
        // CopyMemory(buffer, this->authInfo.pvBuffer, length);

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        memcpy(Buffers[0].Buffer.get_data(),
               this->authInfo.get_data(),
               Buffers[0].Buffer.size());
        // Buffers[0].cbBuffer = this->ContextSizes.cbMaxSignature;
        // Buffers[0].pvBuffer = buffer;

        Buffers[1].Buffer.init(length - this->ContextSizes.cbMaxSignature);
        memcpy(Buffers[0].Buffer.get_data(),
               this->authInfo.get_data() + this->ContextSizes.cbMaxSignature,
               Buffers[0].Buffer.size());
        // Buffers[1].cbBuffer = length - this->ContextSizes.cbMaxSignature;
        // Buffers[1].pvBuffer = &buffer[this->ContextSizes.cbMaxSignature];

        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->DecryptMessage(&this->context, &Message, this->recv_seq_num++, &pfQOP);

        if (status != SEC_E_OK)
            return status;

        StaticStream decrypted_creds(Buffers[1].Buffer.get_data(), Buffers[1].Buffer.size());
        this->ts_credentials.recv(decrypted_creds);
        // credssp_read_ts_credentials(this, &Buffers[1]);

        return SEC_E_OK;
    }


    void credssp_send() {
        // this->ts_request.emit();
    }
    int credssp_recv() {
        // this->ts_request.recv();
        return 1;
    }

    void credssp_buffer_free() {
        this->negoToken.init(0);
        this->pubKeyAuth.init(0);
        this->authInfo.init(0);
    }

};

int credssp_client_authenticate(rdpCredssp* credssp) {
    unsigned long cbMaxToken;
    unsigned long fContextReq;
    unsigned long pfContextAttr;
    uint32_t status;
    CredHandle credentials;
    TimeStamp expiration;
    PSecPkgInfo pPackageInfo;
    SecBuffer input_buffer;
    SecBuffer output_buffer;
    SecBufferDesc input_buffer_desc;
    SecBufferDesc output_buffer_desc;
    bool have_context;
    bool have_input_buffer;

    // TODO ?
    // sspi_GlobalInit();

    if (credssp->credssp_ntlm_client_init() == 0)
        return 0;

    credssp->table = InitSecurityInterface(NTLM_Interface);

    pPackageInfo = NULL;
    if (credssp->table == NULL)
        return 0;
    status = credssp->table->QuerySecurityPackageInfo(NLA_PKG_NAME, pPackageInfo);

    if (status != SEC_E_OK) {
        LOG(LOG_ERR, "QuerySecurityPackageInfo status: 0x%08X\n", status);
        return 0;
    }

    cbMaxToken = pPackageInfo->cbMaxToken;

    status = credssp->table->AcquireCredentialsHandle(NULL, NLA_PKG_NAME,
                                                      SECPKG_CRED_OUTBOUND, NULL,
                                                      &credssp->identity, NULL, NULL,
                                                      &credentials, &expiration);

    if (status != SEC_E_OK) {
        LOG(LOG_ERR, "AcquireCredentialsHandle status: 0x%08X\n", status);
        return 0;
    }

    have_context = false;
    have_input_buffer = false;
    // have_pub_key_auth = false;
    memset(&input_buffer, 0x00, sizeof(SecBuffer));
    memset(&output_buffer, 0x00, sizeof(SecBuffer));
    memset(&credssp->ContextSizes, 0x00, sizeof(SecPkgContext_Sizes));

    /*
     * from tspkg.dll: 0x00000132
     * ISC_REQ_MUTUAL_AUTH
     * ISC_REQ_CONFIDENTIALITY
     * ISC_REQ_USE_SESSION_KEY
     * ISC_REQ_ALLOCATE_MEMORY
     */

    fContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

    while (true) {
        output_buffer_desc.ulVersion = SECBUFFER_VERSION;
        output_buffer_desc.cBuffers = 1;
        output_buffer_desc.pBuffers = &output_buffer;
        output_buffer.BufferType = SECBUFFER_TOKEN;
        output_buffer.Buffer.init(cbMaxToken);
        // output_buffer.cbBuffer = cbMaxToken;
        // output_buffer.pvBuffer = malloc(output_buffer.cbBuffer);

        status = credssp->table->InitializeSecurityContext(&credentials,
                                                           (have_context) ?
                                                           &credssp->context : NULL,
                                                           (char*) credssp->ServicePrincipalName.get_data(),
                                                           fContextReq, 0, SECURITY_NATIVE_DREP,
                                                           (have_input_buffer) ?
                                                           &input_buffer_desc : NULL,
                                                           0, &credssp->context,
                                                           &output_buffer_desc, &pfContextAttr,
                                                           &expiration);

        if (have_input_buffer && (input_buffer.Buffer.size() > 0)) {
            input_buffer.Buffer.init(0);
        }

        if ((status == SEC_I_COMPLETE_AND_CONTINUE) ||
            (status == SEC_I_COMPLETE_NEEDED) ||
            (status == SEC_E_OK)) {

            credssp->table->CompleteAuthToken(&credssp->context, &output_buffer_desc);

            // have_pub_key_auth = true;

            if (credssp->table->QueryContextAttributes(&credssp->context, SECPKG_ATTR_SIZES, &credssp->ContextSizes) != SEC_E_OK) {
                LOG(LOG_ERR, "QueryContextAttributes SECPKG_ATTR_SIZES failure\n");
                return 0;
            }

            credssp->credssp_encrypt_public_key_echo();

            if (status == SEC_I_COMPLETE_NEEDED)
                status = SEC_E_OK;
            else if (status == SEC_I_COMPLETE_AND_CONTINUE)
                status = SEC_I_CONTINUE_NEEDED;
        }

        /* send authentication token to server */

        if (output_buffer.Buffer.size() > 0) {
            // copy or set reference ? BStream
            credssp->negoToken.init(output_buffer.Buffer.size());
            memcpy(credssp->negoToken.get_data(), output_buffer.Buffer.get_data(),
                   output_buffer.Buffer.size());

            // credssp->negoToken.reset();
            // credssp->negoToken.out_copy_bytes(output_buffer.Buffer.get_data());
            // credssp->negoToken.mark_end();
            // credssp->negoToken.rewind();
            // credssp->negoToken = output_buffer.Buffer;

// #ifdef WITH_DEBUG_CREDSSP
//             LOG(LOG_ERR, "Sending Authentication Token\n");
//             hexdump_c(credssp->negoToken.pvBuffer, credssp->negoToken.cbBuffer);
// #endif

            credssp->credssp_send();
            credssp->credssp_buffer_free();
        }

        if (status != SEC_I_CONTINUE_NEEDED)
            break;

        /* receive server response and place in input buffer */

        input_buffer_desc.ulVersion = SECBUFFER_VERSION;
        input_buffer_desc.cBuffers = 1;
        input_buffer_desc.pBuffers = &input_buffer;
        input_buffer.BufferType = SECBUFFER_TOKEN;

        if (credssp->credssp_recv() < 0)
            return -1;

// #ifdef WITH_DEBUG_CREDSSP
//         LOG(LOG_ERR, "Receiving Authentication Token (%d)\n", (int) credssp->negoToken.cbBuffer);
//         hexdump_c(credssp->negoToken.pvBuffer, credssp->negoToken.cbBuffer);
// #endif

        input_buffer.Buffer.init(credssp->negoToken.size());
        memcpy(input_buffer.Buffer.get_data(), credssp->negoToken.get_data(),
               credssp->negoToken.size());

        have_input_buffer = true;
        have_context = true;
    }

    /* Encrypted Public Key +1 */
    if (credssp->credssp_recv() < 0)
        return -1;

    /* Verify Server Public Key Echo */

    status = credssp->credssp_decrypt_public_key_echo();
    credssp->credssp_buffer_free();

    if (status != SEC_E_OK) {
        LOG(LOG_ERR, "Could not verify public key echo!\n");
        return -1;
    }

    /* Send encrypted credentials */

    status = credssp->credssp_encrypt_ts_credentials();

    if (status != SEC_E_OK) {
        LOG(LOG_ERR, "credssp_encrypt_ts_credentials status: 0x%08X\n", status);
        return 0;
    }

    credssp->credssp_send();
    credssp->credssp_buffer_free();

    /* Free resources */

    credssp->table->FreeCredentialsHandle(&credentials);
    // credssp->table->FreeContextBuffer(pPackageInfo);

    return 1;
}



#endif
