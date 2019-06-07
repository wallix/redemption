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
#include "core/RDP/tpdu_buffer.hpp"
#include "utils/hexdump.hpp"
#include "utils/translation.hpp"
#include "system/ssl_sha256.hpp"

#include "transport/transport.hpp"

class rdpCredsspServerNTLM final
{
    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;

    TSRequest ts_request = {6}; // Credssp Version 6 Supported
    static const size_t CLIENT_NONCE_LENGTH = 32;
    ClientNonce SavedClientNonce;

    array_view_u8 public_key;
    Random & rand;
    TimeObj & timeobj;
    std::string& extra_message;
    Translation::language_t lang;
    bool restricted_admin_mode;
    const bool verbose;

    Array ClientServerHash;
    Array ServerClientHash;

    Array ServicePrincipalName;
    SEC_WINNT_AUTH_IDENTITY identity;

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        this->ServicePrincipalName.init(length + 1);
        this->ServicePrincipalName.copy({pszTargetName, length});
        this->ServicePrincipalName.get_data()[length] = 0;
    }

    static void ap_integer_increment_le(array_view_u8 number) {
        for (uint8_t& i : number) {
            if (i < 0xFF) {
                i++;
                break;
            }
            i = 0;
        }
    }

    static void ap_integer_decrement_le(array_view_u8 number) {
        for (uint8_t& i : number) {
            if (i > 0) {
                i--;
                break;
            }
            i = 0xFF;
        }
    }

    void credssp_generate_public_key_hash_client_to_server() {
        LOG(LOG_DEBUG, "rdpCredsspServer::generate credssp public key hash (client->server)");
        Array & SavedHash = this->ClientServerHash;
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, CLIENT_NONCE_LENGTH));
        sha256.update(this->public_key);
        sha256.final(hash);
        SavedHash.init(sizeof(hash));
        memcpy(SavedHash.get_data(), hash, sizeof(hash));
    }

    void credssp_generate_public_key_hash_server_to_client() {
        LOG(LOG_DEBUG, "rdpCredsspServer::generate credssp public key hash (server->client)");
        Array & SavedHash = this->ServerClientHash;
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Server-To-Client Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, CLIENT_NONCE_LENGTH));
        sha256.update(this->public_key);
        sha256.final(hash);
        SavedHash.init(sizeof(hash));
        memcpy(SavedHash.get_data(), hash, sizeof(hash));
    }


    void credssp_buffer_free() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::buffer_free");
        this->ts_request.negoTokens.init(0);
        this->ts_request.pubKeyAuth.init(0);
        this->ts_request.authInfo.init(0);
        this->ts_request.clientNonce.reset();
        this->ts_request.error_code = 0;
    }

public:    

    struct ServerAuthenticateData
    {
        enum : uint8_t { Start, Loop, Final } state = Start;
    };

    ServerAuthenticateData server_auth_data;
    
    enum class Res : bool { Err, Ok };

protected:
    SEC_STATUS state_accept_security_context = SEC_I_INCOMPLETE_CREDENTIALS;



public:
    void set_credentials(uint8_t const* user, uint8_t const* domain,
                         uint8_t const* pass, uint8_t const* hostname) {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::set_credentials");
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

    public:

    struct Ntlm_SecurityFunctionTable
    {
        enum class PasswordCallback
        {
            Error,
            Ok,
            Wait,
        };

    private:
        TimeObj & timeobj;
        std::unique_ptr<SEC_WINNT_AUTH_IDENTITY> identity;
        std::unique_ptr<NTLMContext> context;
        std::function<PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)> set_password_cb;
        bool verbose;

    public:
        explicit Ntlm_SecurityFunctionTable(TimeObj & timeobj,
            std::function<PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)> & set_password_cb,
            bool verbose = false
        )
            : timeobj(timeobj)
            , set_password_cb(set_password_cb)
            , verbose(verbose)
        {}

        ~Ntlm_SecurityFunctionTable() = default;

        // GSS_Acquire_cred
        // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
        SEC_STATUS AcquireCredentialsHandle(
            const char * /*pszPrincipal*/,
            Array * /*pvLogonID*/,
            SEC_WINNT_AUTH_IDENTITY const* /*pAuthData*/)
        {
            this->identity = std::make_unique<SEC_WINNT_AUTH_IDENTITY>();
            return SEC_E_OK;
        }

        // GSS_Init_sec_context
        // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext 
        // -> only for clients : unused for NTLM server

        // GSS_Accept_sec_context
        // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
        SEC_STATUS AcceptSecurityContext(
                array_view_const_u8 input_buffer
                , Array& output_buffer
                , Random & rand)
        {
            LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::AcceptSecurityContext");
            if (!this->context) {
                this->context = std::make_unique<NTLMContext>(true, rand, this->timeobj);

                if (!this->identity) {
                    return SEC_E_WRONG_CREDENTIAL_HANDLE;
                }
                this->context->identity.CopyAuthIdentity(*this->identity);

                this->context->ntlm_SetContextServicePrincipalName(nullptr);
            }

            if (this->context->state == NTLM_STATE_INITIAL) {
                this->context->state = NTLM_STATE_NEGOTIATE;
                SEC_STATUS status = this->context->read_negotiate(input_buffer);
                if (status != SEC_I_CONTINUE_NEEDED) {
                    return SEC_E_INVALID_TOKEN;
                }

                if (this->context->state == NTLM_STATE_CHALLENGE) {
                    return this->context->write_challenge(output_buffer);
                }

                return SEC_E_OUT_OF_SEQUENCE;
            }

            if (this->context->state == NTLM_STATE_AUTHENTICATE) {
                SEC_STATUS status = this->context->read_authenticate(input_buffer);

                if (status == SEC_I_CONTINUE_NEEDED) {
                    if (!this->set_password_cb) {
                        return SEC_E_LOGON_DENIED;
                    }
                    switch (this->set_password_cb(this->context->identity)) {
                        case PasswordCallback::Error:
                            return SEC_E_LOGON_DENIED;
                        case PasswordCallback::Ok:
                            this->context->state = NTLM_STATE_WAIT_PASSWORD;
                            break;
                        case PasswordCallback::Wait:
                            this->context->state = NTLM_STATE_WAIT_PASSWORD;
                            return SEC_I_LOCAL_LOGON;
                    }
                }
            }

            if (this->context->state == NTLM_STATE_WAIT_PASSWORD) {
                SEC_STATUS status = this->context->check_authenticate();
                if (status != SEC_I_CONTINUE_NEEDED && status != SEC_I_COMPLETE_NEEDED) {
                    return status;
                }

                output_buffer.init(0);

                return status;
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

    private:
        /// Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key
        static void compute_hmac_md5(
            uint8_t (&digest)[SslMd5::DIGEST_LENGTH], uint8_t* signing_key,
            const_bytes_view data_buffer, uint32_t SeqNo)
        {
            // TODO signing_key by array reference
            SslHMAC_Md5 hmac_md5({signing_key, 16});
            StaticOutStream<4> out_stream;
            out_stream.out_uint32_le(SeqNo);
            hmac_md5.update(out_stream.get_bytes());
            hmac_md5.update(data_buffer);
            hmac_md5.final(digest);
        }

        static void compute_signature(uint8_t* signature, SslRC4& rc4, uint8_t (&digest)[SslMd5::DIGEST_LENGTH], uint32_t SeqNo)
        {
            uint8_t checksum[8];
            /* RC4-encrypt first 8 bytes of digest */
            rc4.crypt(8, digest, checksum);

            uint32_t version = 1;
            /* Concatenate version, ciphertext and sequence number to build signature */
            memcpy(signature, &version, 4);
            memcpy(&signature[4], checksum, 8);
            memcpy(&signature[12], &SeqNo, 4);
        }

    public:
        // GSS_Wrap
        // ENCRYPT_MESSAGE EncryptMessage;
        SEC_STATUS EncryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo)
        {
            if (!this->context) {
                return SEC_E_NO_CONTEXT;
            }
            LOG_IF(this->context->verbose, LOG_INFO, "NTLM_SSPI::EncryptMessage");

            // data_out [signature][data_buffer]

            data_out.init(data_in.size() + cbMaxSignature);
            auto message_out = data_out.av().array_from_offset(cbMaxSignature);
            uint8_t digest[SslMd5::DIGEST_LENGTH];
            this->compute_hmac_md5(digest, *this->context->SendSigningKey, data_in, MessageSeqNo);
            // this->context->confidentiality == true
            this->context->SendRc4Seal.crypt(data_in.size(), data_in.data(), message_out.data());
            this->compute_signature(data_out.get_data(), this->context->SendRc4Seal, digest, MessageSeqNo);
            return SEC_E_OK;
        }

        // GSS_Unwrap
        // DECRYPT_MESSAGE DecryptMessage;
        SEC_STATUS DecryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo)
        {
            if (!this->context) {
                return SEC_E_NO_CONTEXT;
            }
            LOG_IF(this->context->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

            if (data_in.size() < cbMaxSignature) {
                return SEC_E_INVALID_TOKEN;
            }

            // data_in [signature][data_buffer]

            auto data_buffer = data_in.array_from_offset(cbMaxSignature);
            data_out.init(data_buffer.size());

            /* Decrypt message using with RC4, result overwrites original buffer */
            // this->context->confidentiality == true
            this->context->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.get_data());

            uint8_t digest[SslMd5::DIGEST_LENGTH];
            this->compute_hmac_md5(digest, *this->context->RecvSigningKey, data_out.av(), MessageSeqNo);

            uint8_t expected_signature[16] = {};
            this->compute_signature(
                expected_signature, this->context->RecvRc4Seal, digest, MessageSeqNo);

            if (memcmp(data_in.data(), expected_signature, 16) != 0) {
                /* signature verification failed! */
                LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
                LOG(LOG_ERR, "Expected Signature:");
                hexdump_c(expected_signature, 16);
                LOG(LOG_ERR, "Actual Signature:");
                hexdump_c(data_in.data(), 16);

                return SEC_E_MESSAGE_ALTERED;
            }

            return SEC_E_OK;
        }
    } sspi;

public:
    rdpCredsspServerNTLM(array_view_u8 key,
               const bool restricted_admin_mode,
               Random & rand,
               TimeObj & timeobj,
               std::string& extra_message,
               Translation::language_t lang,
               std::function<Ntlm_SecurityFunctionTable::PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)> set_password_cb,
               const bool verbose = false)
        : public_key(key)
        , rand(rand)
        , timeobj(timeobj)
        , extra_message(extra_message)
        , lang(lang)
        , restricted_admin_mode(restricted_admin_mode)
        , verbose(verbose)
        , sspi(timeobj, set_password_cb, verbose)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::Initialization: NTLM Authentication");
        this->set_credentials(nullptr, nullptr, nullptr, nullptr);

        this->server_auth_data.state = ServerAuthenticateData::Start;
        // TODO: sspi_GlobalInit();

        // Note: NTLMAcquireCredentialHandle never fails
        this->sspi.AcquireCredentialsHandle(nullptr, nullptr, nullptr);

        /*
        * from tspkg.dll: 0x00000112
        * ASC_REQ_MUTUAL_AUTH
        * ASC_REQ_CONFIDENTIALITY
        * ASC_REQ_ALLOCATE_MEMORY
        */
        this->server_auth_data.state = ServerAuthenticateData::Loop;
    }

public:
    credssp::State credssp_server_authenticate_next(InStream & in_stream, OutStream & out_stream)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::credssp_server_authenticate_next");
    
        switch (this->server_auth_data.state)
        {
            case ServerAuthenticateData::Start:
              LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Start");
              return credssp::State::Err;
            case ServerAuthenticateData::Loop:
                LOG(LOG_INFO, "ServerAuthenticateData::Loop");
                if (Res::Err == this->sm_credssp_server_authenticate_recv(in_stream, out_stream)) {
                    LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                    return credssp::State::Err;
                }
                return credssp::State::Cont;
            case ServerAuthenticateData::Final:
               LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final");
               if (Res::Err == this->sm_credssp_server_authenticate_final(in_stream)) {
                   LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final::Err");
                    return credssp::State::Err;
                }
                this->server_auth_data.state = ServerAuthenticateData::Start;
                return credssp::State::Finish;
        }

        return credssp::State::Err;
    }    
private:

    SEC_STATUS credssp_encrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::encrypt_public_key_echo");
        uint32_t version = this->ts_request.use_version;

        if (version >= 5) {
            if (this->ts_request.clientNonce.isset()){
                this->SavedClientNonce = this->ts_request.clientNonce;
            }
            this->credssp_generate_public_key_hash_server_to_client();
            this->public_key = this->ServerClientHash.av();
        }
        else {
            // if we are server and protocol is 2,3,4
            // then echos the public key +1
            this->ap_integer_increment_le(this->public_key);
        }

        return this->sspi.EncryptMessage(
            this->public_key, this->ts_request.pubKeyAuth, this->send_seq_num++);
    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_public_key_echo");

        Array Buffer;

        SEC_STATUS const status = this->sspi.DecryptMessage(
            this->ts_request.pubKeyAuth.av(), Buffer, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "DecryptMessage failure: 0x%08X", status);
            return status;
        }

        const uint32_t version = this->ts_request.use_version;

        if (version >= 5) {
            if (this->ts_request.clientNonce.isset()){
                this->SavedClientNonce = this->ts_request.clientNonce;
            }
            this->credssp_generate_public_key_hash_client_to_server();
            this->public_key = this->ClientServerHash.av();
        }

        array_view_u8 public_key2 = Buffer.av();

        if (public_key2.size() != this->public_key.size()) {
            LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", public_key2.size(), this->public_key.size());
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        if (memcmp(this->public_key.data(), public_key2.data(), public_key.size()) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo");

            LOG(LOG_ERR, "Expected (length = %zu):", this->public_key.size());
            hexdump_av_c(this->public_key);

            LOG(LOG_ERR, "Actual (length = %zu):", this->public_key.size());
            hexdump_av_c(public_key2);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        return SEC_E_OK;
    }

    SEC_STATUS credssp_decrypt_ts_credentials() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_ts_credentials");

        if (this->ts_request.authInfo.size() < 1) {
            LOG(LOG_ERR, "credssp_decrypt_ts_credentials missing ts_request.authInfo buffer");
            return SEC_E_INVALID_TOKEN;
        }

        Array Buffer;

        const SEC_STATUS status = this->sspi.DecryptMessage(
            this->ts_request.authInfo.av(), Buffer, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            return status;
        }

        InStream decrypted_creds(Buffer.get_data(), Buffer.size());
        this->ts_credentials.recv(decrypted_creds);

        // hexdump(this->ts_credentials.passCreds.userName,
        //         this->ts_credentials.passCreds.userName_length);
        // hexdump(this->ts_credentials.passCreds.domainName,
        //         this->ts_credentials.passCreds.domainName_length);
        // hexdump(this->ts_credentials.passCreds.password,
        //         this->ts_credentials.passCreds.password_length);

        return SEC_E_OK;
    }

    Res sm_credssp_server_authenticate_recv(InStream & in_stream, OutStream & out_stream)
    {
        LOG_IF(this->verbose, LOG_INFO,"rdpCredsspServer::sm_credssp_server_authenticate_recv");

        if (this->state_accept_security_context != SEC_I_LOCAL_LOGON) {
            /* receive authentication token */
            this->ts_request.recv(in_stream);
        }

        if (this->ts_request.negoTokens.size() < 1) {
            LOG(LOG_ERR, "CredSSP: invalid ts_request.negoToken!");
            return Res::Err;
        }

        // unsigned long const fContextReq = 0
        //     | ASC_REQ_MUTUAL_AUTH
        //     | ASC_REQ_CONFIDENTIALITY
        //     | ASC_REQ_CONNECTION
        //     | ASC_REQ_USE_SESSION_KEY
        //     | ASC_REQ_REPLAY_DETECT
        //     | ASC_REQ_SEQUENCE_DETECT
        //     | ASC_REQ_EXTENDED_ERROR;
        SEC_STATUS status = this->sspi.AcceptSecurityContext(
            this->ts_request.negoTokens.av(),
            /*output*/this->ts_request.negoTokens, this->rand);
        this->state_accept_security_context = status;
        if (status == SEC_I_LOCAL_LOGON) {
            return Res::Ok;
        }

        if ((status == SEC_I_COMPLETE_AND_CONTINUE) || (status == SEC_I_COMPLETE_NEEDED)) {
            if (status == SEC_I_COMPLETE_NEEDED) {
                status = SEC_E_OK;
            }
            else if (status == SEC_I_COMPLETE_AND_CONTINUE) {
                status = SEC_I_CONTINUE_NEEDED;
            }
        }

        if (status == SEC_E_OK) {
            if (this->credssp_decrypt_public_key_echo() != SEC_E_OK) {
                LOG(LOG_ERR, "Error: could not verify client's public key echo");
                return Res::Err;
            }

            this->ts_request.negoTokens.init(0);

            this->credssp_encrypt_public_key_echo();
        }

        if ((status != SEC_E_OK) && (status != SEC_I_CONTINUE_NEEDED)) {
            LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X", status);
            return Res::Err;
        }

        this->ts_request.emit(out_stream);
        this->credssp_buffer_free();

        if (status != SEC_I_CONTINUE_NEEDED) {
            if (status != SEC_E_OK) {
                LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X", status);
                return Res::Err;
            }
            this->server_auth_data.state = ServerAuthenticateData::Final;
        }

        return Res::Ok;
    }

    Res sm_credssp_server_authenticate_final(InStream & in_stream)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::sm_credssp_server_authenticate_final");
        /* Receive encrypted credentials */
        this->ts_request.recv(in_stream);

        SEC_STATUS status = this->credssp_decrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X", status);
            return Res::Err;
        }

        return Res::Ok;
    }
};



