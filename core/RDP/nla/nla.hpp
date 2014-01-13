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

#include "transport.hpp"

#define NLA_PKG_NAME NTLMSP_NAME

struct rdpCredssp
{
    bool server;
    int send_seq_num;
    int recv_seq_num;
    // freerdp* instance;
    CtxtHandle context;

    Array SspiModule;

    // rdpSettings* settings;
    Transport& trans;

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

    bool hardcodedtests;

    rdpCredssp(Transport & transport)
        : send_seq_num(0)
        , recv_seq_num(0)
        , trans(transport)
        , negoToken(ts_request.negoTokens)
        , pubKeyAuth(ts_request.pubKeyAuth)
        , authInfo(ts_request.authInfo)
        , table(new SecurityFunctionTable)
        , hardcodedtests(false)
    {
        this->SspiModule.init(0);
    }

    ~rdpCredssp()
    {
        if (this->table) {
            this->table->FreeContextBuffer(&this->context);
            delete this->table;
            this->table = NULL;
        }
    }

    void InitSecurityInterface(SecInterface secInter) {
        if (this->table) {
            delete this->table;
            this->table = NULL;
        }
        if (secInter == NTLM_Interface) {
            this->table = new Ntlm_SecurityFunctionTable;
        }
        else if (this->table == NULL) {
            this->table = new SecurityFunctionTable;
        }
    }

    int credssp_ntlm_client_init() {
        // char* spn;
        // int length;
        // bool PromptPassword;
        // rdpTls* tls = NULL;
        // freerdp* instance;
        // rdpSettings* settings;

        this->server = false;
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


        // TODO REMOVE HARDCODED DATA
        // this->identity.SetAuthIdentity(settings->Username, settings->Domain,
        //                                settings->Password);
        if (this->hardcodedtests) {
            uint8_t user[] = "Ulysse";
            uint8_t domain[] = "Ithaque";
            uint8_t pass[] = "Pénélope";
            this->identity.SetAuthIdentityFromUtf8(user, domain, pass);
        }

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

        // TODO REMOVE HARDCODED DATA
        if (this->hardcodedtests) {
            this->PublicKey.init(16);
            this->PublicKey.copy((uint8_t*)"1245789652325415", 16);

            uint8_t host[] = "Télémaque";
            this->ServicePrincipalName.init(sizeof(host));
            this->ServicePrincipalName.copy(host, sizeof(host));
        }

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

    int credssp_ntlm_server_init() {
	// freerdp* instance;
	// rdpSettings* settings = credssp->settings;
	// instance = (freerdp*) settings->instance;

        this->server = true;

        // ================================
        /* Get Public Key From TLS Layer */
        // ================================

        // TODO REMOVE HARDCODED DATA
        if (this->hardcodedtests) {
            this->PublicKey.init(16);
            this->PublicKey.copy((uint8_t*)"1245789652325415", 16);
        }
	// sspi_SecBufferAlloc(&credssp->PublicKey, credssp->transport->TlsIn->PublicKeyLength);
	// CopyMemory(credssp->PublicKey.pvBuffer, credssp->transport->TlsIn->PublicKey, credssp->transport->TlsIn->PublicKeyLength);

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

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        // Buffers[0].Buffer.copy(this->pubKeyAuth.get_data(),
        //                        Buffers[0].Buffer.size());

        Buffers[1].Buffer.init(public_key_length);
        Buffers[1].Buffer.copy(this->PublicKey.get_data(),
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

        this->pubKeyAuth.init(this->ContextSizes.cbMaxSignature + public_key_length);

        this->pubKeyAuth.copy(Buffers[0].Buffer.get_data(),
                              Buffers[0].Buffer.size());
        this->pubKeyAuth.copy(Buffers[1].Buffer.get_data(),
                              Buffers[1].Buffer.size(),
                              this->ContextSizes.cbMaxSignature);

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
            LOG(LOG_ERR, "unexpected pubKeyAuth buffer size:%d\n",
                (int) this->pubKeyAuth.size());
            LOG(LOG_ERR, "size expected: %d = %d + %d\n",
                this->PublicKey.size() + this->ContextSizes.cbMaxSignature,
                this->PublicKey.size(), this->ContextSizes.cbMaxSignature);

            return SEC_E_INVALID_TOKEN;
        }
        length = this->pubKeyAuth.size();

        public_key_length = this->PublicKey.size();

        Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
        Buffers[1].BufferType = SECBUFFER_DATA; /* Encrypted TLS Public Key */

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        Buffers[0].Buffer.copy(this->pubKeyAuth.get_data(),
                               this->ContextSizes.cbMaxSignature);

        Buffers[1].Buffer.init(length - this->ContextSizes.cbMaxSignature);
        Buffers[1].Buffer.copy(this->pubKeyAuth.get_data() + this->ContextSizes.cbMaxSignature,
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
        Buffers[1].Buffer.copy(ts_credentials_send.get_data(),
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
        this->authInfo.copy(Buffers[0].Buffer.get_data(),
                            Buffers[0].Buffer.size());
        this->authInfo.copy(Buffers[1].Buffer.get_data(),
                            Buffers[1].Buffer.size(),
                            this->ContextSizes.cbMaxSignature);

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
        Buffers[0].Buffer.copy(this->authInfo.get_data(),
                               Buffers[0].Buffer.size());
        // Buffers[0].cbBuffer = this->ContextSizes.cbMaxSignature;
        // Buffers[0].pvBuffer = buffer;

        Buffers[1].Buffer.init(length - this->ContextSizes.cbMaxSignature);
        Buffers[1].Buffer.copy(this->authInfo.get_data() + this->ContextSizes.cbMaxSignature,
                               Buffers[1].Buffer.size());
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

        // hexdump(this->ts_credentials.passCreds.userName,
        //         this->ts_credentials.passCreds.userName_length);
        // hexdump(this->ts_credentials.passCreds.domainName,
        //         this->ts_credentials.passCreds.domainName_length);
        // hexdump(this->ts_credentials.passCreds.password,
        //         this->ts_credentials.passCreds.password_length);

        return SEC_E_OK;
    }

    void credssp_send() {
        BStream ts_request_emit;
        this->ts_request.emit(ts_request_emit);
        this->trans.send(ts_request_emit);
    }
    int credssp_recv() {
        // ad hoc read of ber encoding size.

        uint8_t head[4];
        uint8_t * point = head;
        size_t length = 0;
        this->trans.recv(&point, 2);
        uint8_t byte = head[1];
        if (byte & 0x80) {
            byte &= ~(0x80);
            this->trans.recv(&point, byte);
            if (byte == 1) {
                length = head[2];
            }
            else if (byte == 2) {
                length = (head[2] << 8) | head[3];
            }
            else {
                return 0;
            }
        }
        else {
            length = byte;
            byte = 0;
        }
        BStream ts_request_received(2 + byte + length);
        ts_request_received.out_copy_bytes(head, 2 + byte);
        this->trans.recv(&ts_request_received.p, length);
        // ts_request_received.out_skip_bytes(length);
        ts_request_received.mark_end();
        ts_request_received.rewind();
        this->ts_request.recv(ts_request_received);

        // LOG(LOG_INFO, "MARK %u", this->pubKeyAuth.size());
        // hexdump_c(this->pubKeyAuth.get_data(), this->pubKeyAuth.size());

        return 1;
    }

    void credssp_buffer_free() {
        this->negoToken.init(0);
        this->pubKeyAuth.init(0);
        this->authInfo.init(0);
    }

    int credssp_client_authenticate() {
        SEC_STATUS status;

        // TODO ?
        // sspi_GlobalInit();

        if (this->credssp_ntlm_client_init() == 0) {
            return 0;
        }
        this->InitSecurityInterface(NTLM_Interface);

        SecPkgInfo packageInfo;
        if (this->table == NULL) {
            return 0;
        }
        status = this->table->QuerySecurityPackageInfo(NLA_PKG_NAME, &packageInfo);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "QuerySecurityPackageInfo status: 0x%08X\n", status);
            return 0;
        }

        unsigned long cbMaxToken = packageInfo.cbMaxToken;
        CredHandle credentials;
        TimeStamp expiration;

        status = this->table->AcquireCredentialsHandle(NULL, NLA_PKG_NAME,
                                                       SECPKG_CRED_OUTBOUND, NULL,
                                                       &this->identity, NULL, NULL,
                                                       &credentials, &expiration);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "AcquireCredentialsHandle status: 0x%08X\n", status);
            return 0;
        }

        SecBuffer input_buffer;
        SecBuffer output_buffer;
        SecBufferDesc input_buffer_desc = {};
        SecBufferDesc output_buffer_desc = {};
        bool have_context;
        bool have_input_buffer;
        have_context = false;
        have_input_buffer = false;
        input_buffer.setzero();
        output_buffer.setzero();
        memset(&this->ContextSizes, 0x00, sizeof(SecPkgContext_Sizes));

        /*
         * from tspkg.dll: 0x00000132
         * ISC_REQ_MUTUAL_AUTH
         * ISC_REQ_CONFIDENTIALITY
         * ISC_REQ_USE_SESSION_KEY
         * ISC_REQ_ALLOCATE_MEMORY
         */

        unsigned long pfContextAttr;
        unsigned long fContextReq = 0;
        fContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

        while (true) {
            output_buffer_desc.ulVersion = SECBUFFER_VERSION;
            output_buffer_desc.cBuffers = 1;
            output_buffer_desc.pBuffers = &output_buffer;
            output_buffer.BufferType = SECBUFFER_TOKEN;
            output_buffer.Buffer.init(cbMaxToken);

            status = this->table->InitializeSecurityContext(&credentials,
                                                            (have_context) ?
                                                            &this->context : NULL,
                                                            (char*) this->ServicePrincipalName.get_data(),
                                                            fContextReq, this->hardcodedtests?1:0,
                                                            SECURITY_NATIVE_DREP,
                                                            (have_input_buffer) ?
                                                            &input_buffer_desc : NULL,
                                                            0, &this->context,
                                                            &output_buffer_desc, &pfContextAttr,
                                                            &expiration);

            if (have_input_buffer && (input_buffer.Buffer.size() > 0)) {
                input_buffer.Buffer.init(0);
            }

            if ((status == SEC_I_COMPLETE_AND_CONTINUE) ||
                (status == SEC_I_COMPLETE_NEEDED) ||
                (status == SEC_E_OK)) {

                this->table->CompleteAuthToken(&this->context, &output_buffer_desc);

                // have_pub_key_auth = true;

                if (this->table->QueryContextAttributes(&this->context, SECPKG_ATTR_SIZES,
                                                        &this->ContextSizes) != SEC_E_OK) {
                    LOG(LOG_ERR, "QueryContextAttributes SECPKG_ATTR_SIZES failure\n");
                    return 0;
                }

                this->credssp_encrypt_public_key_echo();

                if (status == SEC_I_COMPLETE_NEEDED)
                    status = SEC_E_OK;
                else if (status == SEC_I_COMPLETE_AND_CONTINUE)
                    status = SEC_I_CONTINUE_NEEDED;
            }

            /* send authentication token to server */

            if (output_buffer.Buffer.size() > 0) {
                // copy or set reference ? BStream
                this->negoToken.init(output_buffer.Buffer.size());
                this->negoToken.copy(output_buffer.Buffer.get_data(),
                                     output_buffer.Buffer.size());

                // #ifdef WITH_DEBUG_CREDSSP
                //             LOG(LOG_ERR, "Sending Authentication Token\n");
                //             hexdump_c(this->negoToken.pvBuffer, this->negoToken.cbBuffer);
                // #endif

                this->credssp_send();
                this->credssp_buffer_free();
            }

            if (status != SEC_I_CONTINUE_NEEDED)
                break;

            /* receive server response and place in input buffer */

            input_buffer_desc.ulVersion = SECBUFFER_VERSION;
            input_buffer_desc.cBuffers = 1;
            input_buffer_desc.pBuffers = &input_buffer;
            input_buffer.BufferType = SECBUFFER_TOKEN;

            if (this->credssp_recv() < 0)
                return -1;

            // #ifdef WITH_DEBUG_CREDSSP
            //         LOG(LOG_ERR, "Receiving Authentication Token (%d)\n", (int) this->negoToken.cbBuffer);
            //         hexdump_c(this->negoToken.pvBuffer, this->negoToken.cbBuffer);
            // #endif

            input_buffer.Buffer.init(this->negoToken.size());
            input_buffer.Buffer.copy(this->negoToken.get_data(),
                                     this->negoToken.size());

            have_input_buffer = true;
            have_context = true;
        }

        /* Encrypted Public Key +1 */
        if (this->credssp_recv() < 0)
            return -1;

        /* Verify Server Public Key Echo */

        status = this->credssp_decrypt_public_key_echo();
        this->credssp_buffer_free();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!\n");
            return -1;
        }

        /* Send encrypted credentials */

        status = this->credssp_encrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "credssp_encrypt_ts_credentials status: 0x%08X\n", status);
            return 0;
        }

        this->credssp_send();
        this->credssp_buffer_free();

        /* Free resources */

        this->table->FreeCredentialsHandle(&credentials);

        return 1;
    }



    int credssp_server_authenticate() {
        SEC_STATUS status;

        // TODO
        // sspi_GlobalInit();

        if (credssp_ntlm_server_init() == 0)
            return 0;

        if (this->SspiModule.size() > 0) {
            // HMODULE hSSPI;
            // INIT_SECURITY_INTERFACE pInitSecurityInterface;

            // hSSPI = LoadLibrary(this->SspiModule);

            // if (!hSSPI) {
            //     _tprintf(_T("Failed to load SSPI module: %s\n"), this->SspiModule);
            //     return 0;
            // }
            // pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(hSSPI, "InitSecurityInterfaceA");

            // this->table = (*pInitSecurityInterface)();
        }
        else {

        }

        this->InitSecurityInterface(NTLM_Interface);
        SecPkgInfo packageInfo;
        status = this->table->QuerySecurityPackageInfo(NLA_PKG_NAME, &packageInfo);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "QuerySecurityPackageInfo status: 0x%08X\n", status);
            return 0;
        }

        unsigned long cbMaxToken = packageInfo.cbMaxToken;
        CredHandle credentials;
        TimeStamp expiration;

        status = this->table->AcquireCredentialsHandle(NULL, NLA_PKG_NAME,
                                                       SECPKG_CRED_INBOUND,
                                                       NULL, NULL, NULL, NULL,
                                                       &credentials, &expiration);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "AcquireCredentialsHandle status: 0x%08X\n", status);
            return 0;
        }


        SecBuffer input_buffer;
        SecBuffer output_buffer;
        SecBufferDesc input_buffer_desc = {};
        SecBufferDesc output_buffer_desc = {};
        bool have_context;

        have_context = false;

        input_buffer.setzero();
        output_buffer.setzero();
        memset(&this->ContextSizes, 0x00, sizeof(SecPkgContext_Sizes));
        /*
         * from tspkg.dll: 0x00000112
         * ASC_REQ_MUTUAL_AUTH
         * ASC_REQ_CONFIDENTIALITY
         * ASC_REQ_ALLOCATE_MEMORY
         */

        unsigned long pfContextAttr = this->hardcodedtests?1:0;
        unsigned long fContextReq = 0;
        fContextReq |= ASC_REQ_MUTUAL_AUTH;
        fContextReq |= ASC_REQ_CONFIDENTIALITY;

        fContextReq |= ASC_REQ_CONNECTION;
        fContextReq |= ASC_REQ_USE_SESSION_KEY;

        fContextReq |= ASC_REQ_REPLAY_DETECT;
        fContextReq |= ASC_REQ_SEQUENCE_DETECT;

        fContextReq |= ASC_REQ_EXTENDED_ERROR;

        while (true) {
            /* receive authentication token */

            input_buffer_desc.ulVersion = SECBUFFER_VERSION;
            input_buffer_desc.cBuffers = 1;
            input_buffer_desc.pBuffers = &input_buffer;
            input_buffer.BufferType = SECBUFFER_TOKEN;

            if (this->credssp_recv() < 0)
                return -1;
            // #ifdef WITH_DEBUG_CREDSSP
            //         LOG(LOG_ERR, "Receiving Authentication Token\n");
            //         credssp_buffer_print(this);
            // #endif

            input_buffer.Buffer.init(this->negoToken.size());
            input_buffer.Buffer.copy(this->negoToken.get_data(),
                                     input_buffer.Buffer.size());

            if (this->negoToken.size() < 1) {
                LOG(LOG_ERR, "CredSSP: invalid negoToken!\n");
                return -1;
            }

            output_buffer_desc.ulVersion = SECBUFFER_VERSION;
            output_buffer_desc.cBuffers = 1;
            output_buffer_desc.pBuffers = &output_buffer;
            output_buffer.BufferType = SECBUFFER_TOKEN;
            output_buffer.Buffer.init(cbMaxToken);

            status = this->table->AcceptSecurityContext(&credentials,
                                                        have_context? &this->context: NULL,
                                                        &input_buffer_desc, fContextReq,
                                                        SECURITY_NATIVE_DREP, &this->context,
                                                        &output_buffer_desc, &pfContextAttr,
                                                        &expiration);

            this->negoToken.init(output_buffer.Buffer.size());
            this->negoToken.copy(output_buffer.Buffer.get_data(),
                                 output_buffer.Buffer.size());

            if ((status == SEC_I_COMPLETE_AND_CONTINUE) || (status == SEC_I_COMPLETE_NEEDED)) {
                this->table->CompleteAuthToken(&this->context, &output_buffer_desc);

                if (status == SEC_I_COMPLETE_NEEDED)
                    status = SEC_E_OK;
                else if (status == SEC_I_COMPLETE_AND_CONTINUE)
                    status = SEC_I_CONTINUE_NEEDED;
            }

            if (status == SEC_E_OK) {

                if (this->table->QueryContextAttributes(&this->context,
                                                        SECPKG_ATTR_SIZES,
                                                        &this->ContextSizes) != SEC_E_OK) {
                    LOG(LOG_ERR, "QueryContextAttributes SECPKG_ATTR_SIZES failure\n");
                    return 0;
                }

                if (this->credssp_decrypt_public_key_echo() != SEC_E_OK) {
                    LOG(LOG_ERR, "Error: could not verify client's public key echo\n");
                    return -1;
                }

                this->negoToken.init(0);

                this->credssp_encrypt_public_key_echo();
            }

            if ((status != SEC_E_OK) && (status != SEC_I_CONTINUE_NEEDED)) {
                LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X\n", status);
                return -1;
            }

            /* send authentication token */
            // #ifdef WITH_DEBUG_CREDSSP
            //         LOG(LOG_ERR, "Sending Authentication Token\n");
            //         credssp_buffer_print(this);
            // #endif

            this->credssp_send();
            this->credssp_buffer_free();

            if (status != SEC_I_CONTINUE_NEEDED)
                break;

            have_context = true;
        }

        /* Receive encrypted credentials */

        if (this->credssp_recv() < 0)
            return -1;

        if (this->credssp_decrypt_ts_credentials() != SEC_E_OK) {
            LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X\n", status);
            return 0;
        }

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X\n", status);
            return 0;
        }

        status = this->table->ImpersonateSecurityContext(&this->context);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "ImpersonateSecurityContext status: 0x%08X\n", status);
            return 0;
        }
        else {
            status = this->table->RevertSecurityContext(&this->context);

            if (status != SEC_E_OK) {
                LOG(LOG_ERR, "RevertSecurityContext status: 0x%08X\n", status);
                return 0;
            }
        }

        return 1;
    }

};



#endif
