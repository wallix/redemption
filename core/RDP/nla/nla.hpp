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
#include "RDP/nla/kerberos/kerberos.hpp"
#include "transport.hpp"

#define NLA_PKG_NAME NTLMSP_NAME

struct rdpCredssp
{
    bool server;
    int send_seq_num;
    int recv_seq_num;

    CtxtHandle context;
    CredHandle credentials;

    Array SspiModule;

    Transport& trans;

    TSCredentials ts_credentials;
    TSRequest ts_request;
    Array & negoToken;
    Array & pubKeyAuth;
    Array & authInfo;
    Array PublicKey;

    Array ServicePrincipalName;
    SEC_WINNT_AUTH_IDENTITY identity;
    PSecurityFunctionTable table;
    SecPkgContext_Sizes ContextSizes;
    bool RestrictedAdminMode;
    SecInterface sec_interface;


    TODO("Should not have such variable, but for input/output tests timestamp (and generated nonce) should be static");
    bool hardcodedtests;

    rdpCredssp(Transport & transport,
               uint8_t * user,
               uint8_t * domain,
               uint8_t * pass,
               uint8_t * hostname)
        : send_seq_num(0)
        , recv_seq_num(0)
        , trans(transport)
        , negoToken(ts_request.negoTokens)
        , pubKeyAuth(ts_request.pubKeyAuth)
        , authInfo(ts_request.authInfo)
        , table(new SecurityFunctionTable)
        , RestrictedAdminMode(false)
        , hardcodedtests(false)
    {
        this->SspiModule.init(0);
        this->set_credentials(user, domain, pass, hostname);
        this->sec_interface = NTLM_Interface;
    }

    ~rdpCredssp()
    {
        if (this->table) {
            this->table->FreeContextBuffer(&this->context);
            this->table->FreeCredentialsHandle(&this->credentials);
            delete this->table;
            this->table = NULL;
        }
    }

    void set_credentials(uint8_t * user, uint8_t * domain, uint8_t * pass, uint8_t * hostname) {
        this->identity.SetUserFromUtf8(user);
        this->identity.SetDomainFromUtf8(domain);
        this->identity.SetPasswordFromUtf8(pass);
        this->SetHostnameFromUtf8(hostname);
        // hexdump_c(user, strlen((char*)user));
        // hexdump_c(domain, strlen((char*)domain));
        // hexdump_c(pass, strlen((char*)pass));
        // hexdump_c(hostname, strlen((char*)hostname));
        this->identity.SetKrbAuthIdentity(user, pass);
    }

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        const char * p = (char *)pszTargetName;
        size_t length = 0;
        if (p) {
            length = strlen(p);
        }
        this->ServicePrincipalName.init(length + 1);
        this->ServicePrincipalName.copy(pszTargetName, length);
        this->ServicePrincipalName.get_data()[length] = 0;
    }


    void InitSecurityInterface(SecInterface secInter) {
        if (this->table) {
            delete this->table;
            this->table = NULL;
        }
        if (secInter == NTLM_Interface) {
            this->table = new Ntlm_SecurityFunctionTable;
        }
        if (secInter == Kerberos_Interface) {
            this->table = new Kerberos_SecurityFunctionTable;
        }
        else if (this->table == NULL) {
            this->table = new SecurityFunctionTable;
        }
    }

    int credssp_ntlm_client_init() {
        this->server = false;

// ============================================
/* Get Public Key From TLS Layer and hostname */
// ============================================


        this->PublicKey.init(this->trans.get_public_key_length());
        this->PublicKey.copy(this->trans.get_public_key(), this->trans.get_public_key_length());


        return 1;

    }

    int credssp_ntlm_server_init() {

        this->server = true;

        // ================================
        /* Get Public Key From TLS Layer */
        // ================================

        this->PublicKey.init(this->trans.get_public_key_length());
        this->PublicKey.copy(this->trans.get_public_key(), this->trans.get_public_key_length());

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

        // this->pubKeyAuth.init(this->ContextSizes.cbMaxSignature + public_key_length);
        this->pubKeyAuth.init(Buffers[0].Buffer.size() + Buffers[1].Buffer.size());
        this->pubKeyAuth.copy(Buffers[0].Buffer.get_data(),
                              Buffers[0].Buffer.size());
        this->pubKeyAuth.copy(Buffers[1].Buffer.get_data(),
                              Buffers[1].Buffer.size(),
                              this->ContextSizes.cbMaxSignature);
        return status;

    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        int length = 0;
        unsigned long pfQOP = 0;
        uint8_t* public_key1 = NULL;
        uint8_t* public_key2 = NULL;
        unsigned int public_key_length = 0;
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;

        if (this->pubKeyAuth.size() < this->ContextSizes.cbMaxSignature) {
            LOG(LOG_ERR, "unexpected pubKeyAuth buffer size:%d\n",
                (int) this->pubKeyAuth.size());
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

        if (Buffers[1].Buffer.size() != public_key_length) {
            LOG(LOG_ERR, "Decrypted Pub Key length does not match !");
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        if (!this->server) {
            /* server echos the public key +1 */
            ap_integer_decrement_le(public_key2, public_key_length);
        }
        if (memcmp(public_key1, public_key2, public_key_length) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo");

            LOG(LOG_ERR, "Expected (length = %d):", public_key_length);
            hexdump_c(public_key1, public_key_length);

            LOG(LOG_ERR, "Actual (length = %d):", public_key_length);
            hexdump_c(public_key2, public_key_length);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        return SEC_E_OK;
    }

    void credssp_encode_ts_credentials() {
        if (this->RestrictedAdminMode) {
            this->ts_credentials.set_credentials(NULL, 0,
                                                 NULL, 0,
                                                 NULL, 0);
        }
        else {
            this->ts_credentials.set_credentials(this->identity.Domain.get_data(),
                                                 this->identity.Domain.size(),
                                                 this->identity.User.get_data(),
                                                 this->identity.User.size(),
                                                 this->identity.Password.get_data(),
                                                 this->identity.Password.size());
        }
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

        this->authInfo.init(this->ContextSizes.cbMaxSignature + ts_credentials_send.size());

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        memset(Buffers[0].Buffer.get_data(), 0, Buffers[0].Buffer.size());

        Buffers[1].Buffer.init(ts_credentials_send.size());
        Buffers[1].Buffer.copy(ts_credentials_send.get_data(),
                               ts_credentials_send.size());


        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->EncryptMessage(&this->context, 0, &Message, this->send_seq_num++);

        if (status != SEC_E_OK)
            return status;

        // this->authInfo.init(this->ContextSizes.cbMaxSignature + ts_credentials_send.size());
        this->authInfo.init(Buffers[0].Buffer.size() + Buffers[1].Buffer.size());

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

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        Buffers[0].Buffer.copy(this->authInfo.get_data(),
                               Buffers[0].Buffer.size());

        Buffers[1].Buffer.init(length - this->ContextSizes.cbMaxSignature);
        Buffers[1].Buffer.copy(this->authInfo.get_data() + this->ContextSizes.cbMaxSignature,
                               Buffers[1].Buffer.size());

        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->DecryptMessage(&this->context, &Message, this->recv_seq_num++, &pfQOP);

        if (status != SEC_E_OK)
            return status;

        StaticStream decrypted_creds(Buffers[1].Buffer.get_data(), Buffers[1].Buffer.size());
        this->ts_credentials.recv(decrypted_creds);

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
        ts_request_received.mark_end();
        ts_request_received.rewind();
        this->ts_request.recv(ts_request_received);

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


        if (this->credssp_ntlm_client_init() == 0) {
            return 0;
        }
        this->InitSecurityInterface(this->sec_interface);

        SecPkgInfo packageInfo;
        if (this->table == NULL) {
            LOG(LOG_ERR, "Could not Initiate %s Security Interface!", NTLM_Interface);
            return 0;
        }
        status = this->table->QuerySecurityPackageInfo(NLA_PKG_NAME, &packageInfo);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "QuerySecurityPackageInfo status: 0x%08X\n", status);
            return 0;
        }

        unsigned long cbMaxToken = packageInfo.cbMaxToken;
        TimeStamp expiration;

        status = this->table->AcquireCredentialsHandle("10.10.47.128", NLA_PKG_NAME,
                                                       SECPKG_CRED_OUTBOUND,
                                                       &this->ServicePrincipalName,
                                                       &this->identity, NULL, NULL,
                                                       &this->credentials, &expiration);

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
            status = this->table->InitializeSecurityContext(&this->credentials,
                                                            (have_context) ?
                                                            &this->context : NULL,
                                                            (char*)this->ServicePrincipalName.get_data(),
                                                            fContextReq,
                                                            this->hardcodedtests?1:0,
                                                            SECURITY_NATIVE_DREP,
                                                            (have_input_buffer) ?
                                                            &input_buffer_desc : NULL,
                                                            0, &this->context,
                                                            &output_buffer_desc,
                                                            &pfContextAttr,
                                                            &expiration);
            if (status == SEC_E_INVALID_TOKEN) {
                LOG(LOG_ERR, "Initialize Security Context Error !");
                return -1;
            }

            if (have_input_buffer && (input_buffer.Buffer.size() > 0)) {
                input_buffer.Buffer.init(0);
            }
            SEC_STATUS encrypted = SEC_E_INVALID_TOKEN;
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
                encrypted = this->credssp_encrypt_public_key_echo();
                if (status == SEC_I_COMPLETE_NEEDED) {
                    status = SEC_E_OK;
                }
                else if (status == SEC_I_COMPLETE_AND_CONTINUE) {
                    status = SEC_I_CONTINUE_NEEDED;
                }
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
            else if (encrypted == SEC_E_OK) {
                this->negoToken.init(0);
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

            if (this->credssp_recv() < 0) {
                LOG(LOG_ERR, "NEGO Token Expected!");
                return -1;
            }
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
        if (this->credssp_recv() < 0) {
            LOG(LOG_ERR, "Encrypted Public Key Expected!");
            return -1;
        }

        /* Verify Server Public Key Echo */

        status = this->credssp_decrypt_public_key_echo();
        this->credssp_buffer_free();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!\n");
            this->credssp_buffer_free();
            return -1;
        }

        /* Send encrypted credentials */

        status = this->credssp_encrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "credssp_encrypt_ts_credentials status: 0x%08X\n", status);
            return 0;
        }

        this->credssp_send();

        /* Free resources */
        this->credssp_buffer_free();


        return 1;
    }

    int credssp_server_authenticate() {
        SEC_STATUS status;

        // TODO
        // sspi_GlobalInit();

        if (credssp_ntlm_server_init() == 0)
            return 0;

        this->InitSecurityInterface(NTLM_Interface);

        SecPkgInfo packageInfo;
        status = this->table->QuerySecurityPackageInfo(NLA_PKG_NAME, &packageInfo);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "QuerySecurityPackageInfo status: 0x%08X\n", status);
            return 0;
        }

        unsigned long cbMaxToken = packageInfo.cbMaxToken;
        TimeStamp expiration;

        status = this->table->AcquireCredentialsHandle(NULL, NLA_PKG_NAME,
                                                       SECPKG_CRED_INBOUND, NULL,
                                                       NULL, NULL, NULL,
                                                       &this->credentials, &expiration);

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

            status = this->table->AcceptSecurityContext(&this->credentials,
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
