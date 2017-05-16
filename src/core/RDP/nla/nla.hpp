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


#pragma once

#include "core/RDP/nla/sspi.hpp"
#include "core/RDP/nla/credssp.hpp"
#include "core/RDP/nla/ntlm/ntlm.hpp"

#ifndef __EMSCRIPTEN__
#include "core/RDP/nla/kerberos/kerberos.hpp"
#endif

#include "transport/transport.hpp"

#define NLA_PKG_NAME NTLMSP_NAME

class rdpCredssp
{
    bool server;
    int send_seq_num;
    int recv_seq_num;

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

    const char * target_host;
    Random & rand;
    TimeObj & timeobj;
    const bool verbose;

public:
    bool hardcoded_tests = false;

public:
    rdpCredssp(Transport & transport,
               uint8_t * user,
               uint8_t * domain,
               uint8_t * pass,
               uint8_t * hostname,
               const char * target_host,
               const bool krb,
               const bool restricted_admin_mode,
               Random & rand,
               TimeObj & timeobj,
               const bool verbose = false)
        : server(false)
        , send_seq_num(0)
        , recv_seq_num(0)
        , trans(transport)
        , negoToken(ts_request.negoTokens)
        , pubKeyAuth(ts_request.pubKeyAuth)
        , authInfo(ts_request.authInfo)
        , table(new SecurityFunctionTable)
        , RestrictedAdminMode(restricted_admin_mode)
        , sec_interface(krb ? Kerberos_Interface : NTLM_Interface)
        , target_host(target_host)
        , rand(rand)
        , timeobj(timeobj)
        , verbose(verbose)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp:: Initialization");
        }
        this->set_credentials(user, domain, pass, hostname);
    }

    ~rdpCredssp()
    {
        if (this->table) {
            delete this->table;
            this->table = nullptr;
        }
    }

private:
    rdpCredssp(const rdpCredssp &) /*= delete*/;
    rdpCredssp&operator=(const rdpCredssp &) /*= delete*/;

public:
    void set_credentials(uint8_t * user, uint8_t * domain,
                         uint8_t * pass, uint8_t * hostname) {
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::set_credentials");
        }
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
        const char * p = reinterpret_cast<const char *>(pszTargetName);
        size_t length = 0;
        if (p) {
            length = strlen(p);
        }
        this->ServicePrincipalName.init(length + 1);
        this->ServicePrincipalName.copy(pszTargetName, length);
        this->ServicePrincipalName.get_data()[length] = 0;
    }


    void InitSecurityInterface(SecInterface secInter) {
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::InitSecurityInterface");
        }
        if (this->table) {
            delete this->table;
            this->table = nullptr;
        }
        if (secInter == NTLM_Interface) {
            LOG(LOG_INFO, "Credssp: NTLM Authentication");
            auto table = new Ntlm_SecurityFunctionTable(this->rand, this->timeobj);
            if (this->hardcoded_tests) {
                table->hardcoded_tests = true;
            }
            this->table = table;
        }
        if (secInter == Kerberos_Interface) {
            LOG(LOG_INFO, "Credssp: KERBEROS Authentication");

            #ifndef __EMSCRIPTEN__
            this->table = new Kerberos_SecurityFunctionTable;
            #else
            assert(false && "Unsupported Kerberos");
            #endif
        }
        else if (this->table == nullptr) {
            this->table = new SecurityFunctionTable;
        }
    }

    int credssp_ntlm_client_init() {
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::client_init");
        }

        this->server = false;

// ============================================
/* Get Public Key From TLS Layer and hostname */
// ============================================


        this->PublicKey.init(this->trans.get_public_key_length());
        this->PublicKey.copy(this->trans.get_public_key(), this->trans.get_public_key_length());


        return 1;

    }

    int credssp_ntlm_server_init() {
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::server_init");
        }

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
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::encrypt_public_key_echo");
        }
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

        status = this->table->EncryptMessage(&Message, this->send_seq_num++);

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
        uint8_t* public_key1 = nullptr;
        uint8_t* public_key2 = nullptr;
        unsigned int public_key_length = 0;
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::decrypt_public_key_echo");
        }

        if (this->pubKeyAuth.size() < this->ContextSizes.cbMaxSignature) {
            LOG(LOG_ERR, "unexpected pubKeyAuth buffer size:%zu\n",
                this->pubKeyAuth.size());
            if (this->pubKeyAuth.size() == 0) {
                LOG(LOG_INFO, "Provided password is probably incorrect.\n");
            }
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

        status = this->table->DecryptMessage(&Message, this->recv_seq_num++);

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
            LOG(LOG_INFO, "Restricted Admin Mode");
            this->ts_credentials.set_credentials(nullptr, 0,
                                                 nullptr, 0,
                                                 nullptr, 0);
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
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::encrypt_ts_credentials");
        }
        this->credssp_encode_ts_credentials();

        StaticOutStream<65536> ts_credentials_send;
        this->ts_credentials.emit(ts_credentials_send);

        Buffers[0].BufferType = SECBUFFER_TOKEN; /* Signature */
        Buffers[1].BufferType = SECBUFFER_DATA;  /* TSCredentials */

        this->authInfo.init(this->ContextSizes.cbMaxSignature + ts_credentials_send.get_offset());

        Buffers[0].Buffer.init(this->ContextSizes.cbMaxSignature);
        memset(Buffers[0].Buffer.get_data(), 0, Buffers[0].Buffer.size());

        Buffers[1].Buffer.init(ts_credentials_send.get_offset());
        Buffers[1].Buffer.copy(ts_credentials_send.get_data(),
                               ts_credentials_send.get_offset());


        Message.cBuffers = 2;
        Message.ulVersion = SECBUFFER_VERSION;
        Message.pBuffers = Buffers;

        status = this->table->EncryptMessage(&Message, this->send_seq_num++);

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
        SecBuffer Buffers[2];
        SecBufferDesc Message;
        SEC_STATUS status;
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::decrypt_ts_credentials");
        }
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

        status = this->table->DecryptMessage(&Message, this->recv_seq_num++);

        if (status != SEC_E_OK)
            return status;

        InStream decrypted_creds(Buffers[1].Buffer.get_data(), Buffers[1].Buffer.size());
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
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::send");
        }
        StaticOutStream<65536> ts_request_emit;
        this->ts_request.emit(ts_request_emit);
        this->trans.send(ts_request_emit.get_data(), ts_request_emit.get_offset());
    }
    int credssp_recv() {
        // ad hoc read of ber encoding size.
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::recv");
        }
        uint8_t head[4] = {};
        uint8_t * point = head;
        size_t length = 0;
        this->trans.recv_boom(point, 2);
        point += 2;
        uint8_t byte = head[1];
        if (byte & 0x80) {
            byte &= ~(0x80);
            this->trans.recv_boom(point, byte);

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
        StreamBufMaker<65536> ts_request_received_maker;
        OutStream ts_request_received = ts_request_received_maker.reserve_out_stream(2 + byte + length);
        ts_request_received.out_copy_bytes(head, 2 + byte);
        this->trans.recv_boom(ts_request_received.get_current(), length);
        InStream in_stream(ts_request_received.get_data(), ts_request_received.get_capacity());
        this->ts_request.recv(in_stream);

        // hexdump_c(this->pubKeyAuth.get_data(), this->pubKeyAuth.size());

        return 1;
    }

    void credssp_buffer_free() {
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::buffer_free");
        }
        this->negoToken.init(0);
        this->pubKeyAuth.init(0);
        this->authInfo.init(0);
    }

    int credssp_client_authenticate() {
        SEC_STATUS status;
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp::client_authenticate");
        }

        if (this->credssp_ntlm_client_init() == 0) {
            return 0;
        }
        SecPkgInfo packageInfo;
        bool interface_changed = false;
        do {
            interface_changed = false;
            this->InitSecurityInterface(this->sec_interface);

            if (this->table == nullptr) {
                LOG(LOG_ERR, "Could not Initiate %d Security Interface!", this->sec_interface);
                return 0;
            }
            status = this->table->QuerySecurityPackageInfo(NLA_PKG_NAME, &packageInfo);

            if (status != SEC_E_OK) {
                LOG(LOG_ERR, "QuerySecurityPackageInfo status: 0x%08X\n", status);
                return 0;
            }


            status = this->table->AcquireCredentialsHandle(this->target_host,
                                                           SECPKG_CRED_OUTBOUND,
                                                           &this->ServicePrincipalName,
                                                           &this->identity);
            if (status == SEC_E_NO_CREDENTIALS) {
                if (this->sec_interface != NTLM_Interface) {
                    this->sec_interface = NTLM_Interface;
                    interface_changed = true;
                    LOG(LOG_INFO, "Credssp: No Kerberos Credentials, fallback to NTLM");
                }
            }
        } while (interface_changed);

        unsigned long cbMaxToken = packageInfo.cbMaxToken;

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "AcquireCredentialsHandle status: 0x%08X\n", status);
            return 0;
        }

        SecBuffer input_buffer;
        SecBuffer output_buffer;
        SecBufferDesc input_buffer_desc = {0,0,nullptr};
        SecBufferDesc output_buffer_desc;
        bool have_input_buffer = false;
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

        unsigned long fContextReq = 0;
        fContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

        while (true) {
            output_buffer_desc.ulVersion = SECBUFFER_VERSION;
            output_buffer_desc.cBuffers = 1;
            output_buffer_desc.pBuffers = &output_buffer;
            output_buffer.BufferType = SECBUFFER_TOKEN;
            output_buffer.Buffer.init(cbMaxToken);
            status = this->table->InitializeSecurityContext(
                reinterpret_cast<char*>(this->ServicePrincipalName.get_data()),
                fContextReq,
                have_input_buffer ? &input_buffer_desc : nullptr,
                this->verbose,
                &output_buffer_desc);
            if ((status != SEC_I_COMPLETE_AND_CONTINUE) &&
                (status != SEC_I_COMPLETE_NEEDED) &&
                (status != SEC_E_OK) &&
                (status != SEC_I_CONTINUE_NEEDED)) {
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
                this->table->CompleteAuthToken(&output_buffer_desc);

                // have_pub_key_auth = true;
                if (this->table->QueryContextAttributes(SECPKG_ATTR_SIZES,
                                                        &this->ContextSizes) != SEC_E_OK) {
                    LOG(LOG_ERR, "QueryContextAttributes SECPKG_ATTR_SIZES failure");
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
                if (this->verbose) {
                    LOG(LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
                }

                this->credssp_send();
                this->credssp_buffer_free();
            }
            else if (encrypted == SEC_E_OK) {
                this->negoToken.init(0);
                this->credssp_send();
                this->credssp_buffer_free();
            }

            if (status != SEC_I_CONTINUE_NEEDED) {
                if (this->verbose) {
                    LOG(LOG_INFO, "rdpCredssp Token loop: CONTINUE_NEEDED");
                }
                break;
            }
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
            if (this->verbose) {
                LOG(LOG_INFO, "rdpCredssp - Client Authentication : Receiving Authentication Token");
            }
            input_buffer.Buffer.init(this->negoToken.size());
            input_buffer.Buffer.copy(this->negoToken.get_data(),
                                     this->negoToken.size());

            have_input_buffer = true;
        }

        /* Encrypted Public Key +1 */
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp - Client Authentication : Receiving Encrypted PubKey + 1");
        }
        if (this->credssp_recv() < 0) {
            LOG(LOG_ERR, "Encrypted Public Key Expected!");
            return -1;
        }

        /* Verify Server Public Key Echo */

        status = this->credssp_decrypt_public_key_echo();
        this->credssp_buffer_free();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!");
            this->credssp_buffer_free();
            return -1;
        }

        /* Send encrypted credentials */

        status = this->credssp_encrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "credssp_encrypt_ts_credentials status: 0x%08X\n", status);
            return 0;
        }
        if (this->verbose) {
            LOG(LOG_INFO, "rdpCredssp - Client Authentication : Sending Credentials");
        }
        this->credssp_send();

        /* Free resources */
        this->credssp_buffer_free();


        return 1;
    }

    int credssp_server_authenticate() {
       SEC_STATUS status;
       if (this->verbose) {
           LOG(LOG_INFO, "rdpCredssp::server_authenticate");
       }
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

       status = this->table->AcquireCredentialsHandle(nullptr,
                                                      SECPKG_CRED_INBOUND,
                                                      nullptr,
                                                      nullptr);

       if (status != SEC_E_OK) {
           LOG(LOG_ERR, "AcquireCredentialsHandle status: 0x%08X\n", status);
           return 0;
       }


       SecBuffer input_buffer;
       SecBuffer output_buffer;
       SecBufferDesc input_buffer_desc;
       SecBufferDesc output_buffer_desc;

       input_buffer.setzero();
       output_buffer.setzero();
       memset(&this->ContextSizes, 0x00, sizeof(SecPkgContext_Sizes));
       /*
        * from tspkg.dll: 0x00000112
        * ASC_REQ_MUTUAL_AUTH
        * ASC_REQ_CONFIDENTIALITY
        * ASC_REQ_ALLOCATE_MEMORY
        */

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

           input_buffer.Buffer.init(this->negoToken.size());
           input_buffer.Buffer.copy(this->negoToken.get_data(),
                                    input_buffer.Buffer.size());

           if (this->negoToken.size() < 1) {
               LOG(LOG_ERR, "CredSSP: invalid negoToken!");
               return -1;
           }

           output_buffer_desc.ulVersion = SECBUFFER_VERSION;
           output_buffer_desc.cBuffers = 1;
           output_buffer_desc.pBuffers = &output_buffer;
           output_buffer.BufferType = SECBUFFER_TOKEN;
           output_buffer.Buffer.init(cbMaxToken);

           status = this->table->AcceptSecurityContext(&input_buffer_desc, fContextReq,
                                                       &output_buffer_desc);

           this->negoToken.init(output_buffer.Buffer.size());
           this->negoToken.copy(output_buffer.Buffer.get_data(),
                                output_buffer.Buffer.size());

           if ((status == SEC_I_COMPLETE_AND_CONTINUE) || (status == SEC_I_COMPLETE_NEEDED)) {
               this->table->CompleteAuthToken(&output_buffer_desc);

               if (status == SEC_I_COMPLETE_NEEDED)
                   status = SEC_E_OK;
               else if (status == SEC_I_COMPLETE_AND_CONTINUE)
                   status = SEC_I_CONTINUE_NEEDED;
           }

           if (status == SEC_E_OK) {

               if (this->table->QueryContextAttributes(SECPKG_ATTR_SIZES,
                                                       &this->ContextSizes) != SEC_E_OK) {
                   LOG(LOG_ERR, "QueryContextAttributes SECPKG_ATTR_SIZES failure");
                   return 0;
               }

               if (this->credssp_decrypt_public_key_echo() != SEC_E_OK) {
                   LOG(LOG_ERR, "Error: could not verify client's public key echo");
                   return -1;
               }

               this->negoToken.init(0);

               this->credssp_encrypt_public_key_echo();
           }

           if ((status != SEC_E_OK) && (status != SEC_I_CONTINUE_NEEDED)) {
               LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X\n", status);
               return -1;
           }


           this->credssp_send();
           this->credssp_buffer_free();

           if (status != SEC_I_CONTINUE_NEEDED)
               break;
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

       status = this->table->ImpersonateSecurityContext();

       if (status != SEC_E_OK) {
           LOG(LOG_ERR, "ImpersonateSecurityContext status: 0x%08X\n", status);
           return 0;
       }
       else {
           status = this->table->RevertSecurityContext();

           if (status != SEC_E_OK) {
               LOG(LOG_ERR, "RevertSecurityContext status: 0x%08X\n", status);
               return 0;
           }
       }

       return 1;
    }
};
