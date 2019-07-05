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

#ifndef __EMSCRIPTEN__
#include "core/RDP/nla/kerberos/kerberos.hpp"
#endif

#include "transport/transport.hpp"

#include <vector>


class rdpCredsspClientNTLM
{
    static constexpr uint32_t cbMaxSignature = 16;
private:
    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;
    TSRequest ts_request;

    ClientNonce SavedClientNonce;

    std::vector<uint8_t> PublicKey;
    std::vector<uint8_t> ClientServerHash;
    std::vector<uint8_t> ServerClientHash;
    Array ServicePrincipalName;
    SEC_WINNT_AUTH_IDENTITY identity;
    
    SEC_WINNT_AUTH_IDENTITY * sspi_identity = &identity;
    
    bool sspi_context_initialized = false;
    NTLMContext sspi_context;
    
    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    SEC_STATUS sspi_InitializeSecurityContext(
        array_view_const_char pszTargetName, array_view_const_u8 input_buffer, Array& output_buffer
    )
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");

        if (!this->sspi_context_initialized) {

            if (!this->sspi_identity) {
                return SEC_E_WRONG_CREDENTIAL_HANDLE;
            }
            this->sspi_context.ntlm_SetContextWorkstation(pszTargetName);
            this->sspi_context.ntlm_SetContextServicePrincipalName(pszTargetName);

            this->sspi_context.identity.CopyAuthIdentity(this->sspi_identity->get_user_utf16_av(),
                                                    this->sspi_identity->get_domain_utf16_av(),
                                                    this->sspi_identity->get_password_utf16_av());
            this->sspi_context_initialized = true;
        }

        if (this->sspi_context.state == NTLM_STATE_INITIAL) {
            this->sspi_context.state = NTLM_STATE_NEGOTIATE;
        }
        if (this->sspi_context.state == NTLM_STATE_NEGOTIATE) {
            return this->sspi_context.write_negotiate(output_buffer);
        }

        if (this->sspi_context.state == NTLM_STATE_CHALLENGE) {
            this->sspi_context.read_challenge(input_buffer);
        }
        if (this->sspi_context.state == NTLM_STATE_AUTHENTICATE) {
            return this->sspi_context.write_authenticate(output_buffer);
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    /// Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key
    static void sspi_compute_hmac_md5(
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

    static void sspi_compute_signature(
        uint8_t* signature, SslRC4& rc4, uint8_t (&digest)[SslMd5::DIGEST_LENGTH], uint32_t SeqNo)
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

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    SEC_STATUS sspi_EncryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo) {
        if (!this->sspi_context_initialized) {
            return SEC_E_NO_CONTEXT;
        }
        LOG_IF(this->sspi_context.verbose, LOG_INFO, "NTLM_SSPI::EncryptMessage");

        // data_out [signature][data_buffer]

        data_out.init(data_in.size() + cbMaxSignature);
        auto message_out = data_out.av().array_from_offset(cbMaxSignature);

        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->sspi_compute_hmac_md5(digest, *this->sspi_context.SendSigningKey, data_in, MessageSeqNo);

        /* Encrypt message using with RC4, result overwrites original buffer */
        // this->sspi_context.confidentiality == true
        this->sspi_context.SendRc4Seal.crypt(data_in.size(), data_in.data(), message_out.data());

        this->sspi_compute_signature(
            data_out.get_data(), this->sspi_context.SendRc4Seal, digest, MessageSeqNo);

        return SEC_E_OK;
    }


    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    std::pair<SEC_STATUS,std::vector<uint8_t>> sspi_DecryptMessage(array_view_const_u8 data_in, unsigned long MessageSeqNo) 
    {
        std::vector<uint8_t> data_out;
    
        if (!this->sspi_context_initialized) {
            return std::make_pair(SEC_E_NO_CONTEXT, data_out);
        }
        LOG_IF(this->sspi_context.verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

        if (data_in.size() < cbMaxSignature) {
            return std::make_pair(SEC_E_INVALID_TOKEN, data_out);
        }

        // data_in [signature][data_buffer]

        auto data_buffer = data_in.array_from_offset(cbMaxSignature);
        data_out.resize(data_buffer.size(), 0);

        /* Decrypt message using with RC4, result overwrites original buffer */
        // this->sspi_context.confidentiality == true
        this->sspi_context.RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.data());

        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->sspi_compute_hmac_md5(digest, *this->sspi_context.RecvSigningKey, {data_out.data(), data_out.size()}, MessageSeqNo);

        uint8_t expected_signature[16] = {};
        this->sspi_compute_signature(
            expected_signature, this->sspi_context.RecvRc4Seal, digest, MessageSeqNo);

        if (memcmp(data_in.data(), expected_signature, 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature, 16);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(data_in.data(), 16);

            return std::make_pair(SEC_E_MESSAGE_ALTERED, data_out);
        }
        return std::make_pair(SEC_E_OK, data_out);
    }

    bool restricted_admin_mode;

    const char * target_host;
    Random & rand;
    std::string& extra_message;
    Translation::language_t lang;
    const bool verbose;

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        this->ServicePrincipalName.init(length + 1);
        this->ServicePrincipalName.copy({pszTargetName, length});
        this->ServicePrincipalName.get_data()[length] = 0;
    }

    void credssp_generate_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp generate client nonce");
        this->rand.random(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH);
        this->SavedClientNonce.initialized = true;
        this->credssp_set_client_nonce();
    }

    void credssp_get_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp get client nonce");
        if (this->ts_request.clientNonce.isset()){
            this->SavedClientNonce = this->ts_request.clientNonce;
        }
    }
    void credssp_set_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp set client nonce");
        if (!this->ts_request.clientNonce.isset()) {
            this->ts_request.clientNonce = this->SavedClientNonce;
        }
    }

    void credssp_generate_public_key_hash_client_to_server() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::generate credssp public key hash (client->server)");
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH));

        sha256.update({this->PublicKey.data(),this->PublicKey.size()});
        sha256.final(hash);
        this->ClientServerHash.assign(hash, hash+sizeof(hash));
    }

    void credssp_generate_public_key_hash_server_to_client() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::generate credssp public key hash (server->client)");
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Server-To-Client Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH));
        sha256.update({this->PublicKey.data(),this->PublicKey.size()});
        sha256.final(hash);
        this->ServerClientHash.assign(hash, hash + sizeof(hash));
    }

    SEC_STATUS credssp_encrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::encrypt_public_key_echo");
        uint32_t version = this->ts_request.use_version;

        array_view_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};
        if (version >= 5) {
            this->credssp_generate_client_nonce();
            this->credssp_generate_public_key_hash_client_to_server();
            public_key = {this->ClientServerHash.data(), this->ClientServerHash.size()};
        }

        return this->sspi_EncryptMessage(
            public_key, this->ts_request.pubKeyAuth, this->send_seq_num++);
    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::decrypt_public_key_echo");

        auto [status, Buffer] = this->sspi_DecryptMessage(this->ts_request.pubKeyAuth.av(), this->recv_seq_num++);

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

        array_view_const_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};
        if (version >= 5) {
            this->credssp_get_client_nonce();
            this->credssp_generate_public_key_hash_server_to_client();
            public_key = {this->ServerClientHash.data(), this->ServerClientHash.size()};
        }

        array_view_u8 public_key2 = {Buffer.data(), Buffer.size()};

        if (public_key2.size() != public_key.size()) {
            LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", public_key2.size(), public_key.size());
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        if (version < 5) {
            // if we are client and protocol is 2,3,4
            // then get the public key minus one
            ::ap_integer_decrement_le(public_key2);
        }

        if (memcmp(public_key.data(), public_key2.data(), public_key.size()) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo");

            LOG(LOG_ERR, "Expected (length = %zu):", public_key.size());
            hexdump_av_c(public_key);

            LOG(LOG_ERR, "Actual (length = %zu):", public_key.size());
            hexdump_av_c(public_key2);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        return SEC_E_OK;
    }

    void credssp_buffer_free() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::buffer_free");
        this->ts_request.negoTokens.init(0);
        this->ts_request.pubKeyAuth.init(0);
        this->ts_request.authInfo.init(0);
        this->ts_request.clientNonce.reset();
        this->ts_request.error_code = 0;
    }

    enum class Res : bool { Err, Ok };

    struct ClientAuthenticateData
    {
        enum : uint8_t { Start, Loop, Final } state = Start;
        Array input_buffer;
    };
    ClientAuthenticateData client_auth_data;

    Res sm_credssp_client_authenticate_stop(InStream & in_stream, OutTransport transport)
    {
        /* Encrypted Public Key +1 */
        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Encrypted PubKey + 1");

        this->ts_request.recv(in_stream);

        /* Verify Server Public Key Echo */

        SEC_STATUS status = this->credssp_decrypt_public_key_echo();
        this->credssp_buffer_free();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!");
            this->credssp_buffer_free();
            return Res::Err;
        }

        /* Send encrypted credentials */

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::encrypt_ts_credentials");
        if (this->restricted_admin_mode) {
            LOG(LOG_INFO, "Restricted Admin Mode");
            this->ts_credentials.set_credentials_from_av({},{},{});
        }
        else {
            this->ts_credentials.set_credentials_from_av(this->identity.get_domain_utf16_av(),
                                                        this->identity.get_user_utf16_av(),
                                                        this->identity.get_password_utf16_av());
        }

        StaticOutStream<65536> ts_credentials_send;
        this->ts_credentials.emit(ts_credentials_send);

        if (SEC_E_OK != this->sspi_EncryptMessage(
            {ts_credentials_send.get_data(), ts_credentials_send.get_offset()},
            this->ts_request.authInfo, this->send_seq_num++)){
            LOG(LOG_ERR, "credssp_encrypt_ts_credentials status: 0x%08X", status);
            return Res::Err;
        }

        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Credentials");

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::send");
        StaticOutStream<65536> ts_request_emit;
        this->ts_request.emit(ts_request_emit);
        transport.get_transport().send(ts_request_emit.get_bytes());
        this->credssp_buffer_free();

        return Res::Ok;
    }

public:
    rdpCredsspClientNTLM(OutTransport transport,
               uint8_t * user,
               uint8_t * domain,
               uint8_t * pass,
               uint8_t * hostname,
               const char * target_host,
               const bool restricted_admin_mode,
               Random & rand,
               TimeObj & timeobj,
               std::string& extra_message,
               Translation::language_t lang,
               const bool verbose = false)
        : ts_request(6) // Credssp Version 6 Supported
        , SavedClientNonce()
        , sspi_context(false, rand, timeobj, verbose)
        , restricted_admin_mode(restricted_admin_mode)
        , target_host(target_host)
        , rand(rand)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::Initialization");
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::set_credentials");
        this->identity.SetUserFromUtf8(user);
        this->identity.SetDomainFromUtf8(domain);
        this->identity.SetPasswordFromUtf8(pass);
        this->SetHostnameFromUtf8(hostname);
        this->identity.SetKrbAuthIdentity(user, pass);

        this->client_auth_data.state = ClientAuthenticateData::Start;

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::client_authenticate");

        // ============================================
        /* Get Public Key From TLS Layer and hostname */
        // ============================================

        auto const key = transport.get_transport().get_public_key();
        this->PublicKey.assign(key.data(), key.data()+key.size());

        LOG(LOG_INFO, "Credssp: NTLM Authentication");
        
//        SEC_STATUS status0 = this->sspi_AcquireCredentialsHandle(&this->identity);

        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::AcquireCredentialsHandle");
//        this->sspi_identity = std::make_unique<SEC_WINNT_AUTH_IDENTITY>();

//        this->sspi_identity->CopyAuthIdentity(this->identity.get_user_utf16_av(), this->identity.get_domain_utf16_av(), this->identity.get_password_utf16_av());

        this->client_auth_data.input_buffer.init(0);
        this->client_auth_data.state = ClientAuthenticateData::Loop;

        /*
         * from tspkg.dll: 0x00000132
         * ISC_REQ_MUTUAL_AUTH
         * ISC_REQ_CONFIDENTIALITY
         * ISC_REQ_USE_SESSION_KEY
         * ISC_REQ_ALLOCATE_MEMORY
         */
        //unsigned long const fContextReq
        //  = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

        /* receive server response and place in input buffer */
        SEC_STATUS status1 = this->sspi_InitializeSecurityContext(
            bytes_view(this->ServicePrincipalName.av()).as_chars(),
            this->client_auth_data.input_buffer.av(),
            /*output*/this->ts_request.negoTokens);
        SEC_STATUS encrypted = SEC_E_INVALID_TOKEN;

        if ((status1 != SEC_I_COMPLETE_AND_CONTINUE) &&
            (status1 != SEC_I_COMPLETE_NEEDED) &&
            (status1 != SEC_E_OK) &&
            (status1 != SEC_I_CONTINUE_NEEDED)) {
            LOG(LOG_ERR, "Initialize Security Context Error !");
            throw ERR_CREDSSP_NTLM_INIT_FAILED;
        }

        this->client_auth_data.input_buffer.init(0);

        if ((status1 == SEC_I_COMPLETE_AND_CONTINUE) ||
            (status1 == SEC_I_COMPLETE_NEEDED) ||
            (status1 == SEC_E_OK)) {
            // have_pub_key_auth = true;
            encrypted = this->credssp_encrypt_public_key_echo();
            if (status1 == SEC_I_COMPLETE_NEEDED) {
                status1 = SEC_E_OK;
            }
            else if (status1 == SEC_I_COMPLETE_AND_CONTINUE) {
                status1 = SEC_I_CONTINUE_NEEDED;
            }
        }

        /* send authentication token to server */
        if ((this->ts_request.negoTokens.size() > 0)||(encrypted == SEC_E_OK)) {
            // #ifdef WITH_DEBUG_CREDSSP
            //             LOG(LOG_ERR, "Sending Authentication Token");
            //             hexdump_c(this->ts_request.negoTokens.pvBuffer, this->ts_request.negoTokens.cbBuffer);
            // #endif
            if (this->ts_request.negoTokens.size() > 0){
                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
            }

            LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::send");
            StaticOutStream<65536> ts_request_emit;
            this->ts_request.emit(ts_request_emit);
            transport.get_transport().send(ts_request_emit.get_bytes());

            this->credssp_buffer_free();
        }

        if (status1 != SEC_I_CONTINUE_NEEDED) {
            LOG_IF(this->verbose, LOG_INFO, "rdpCredssp Token loop: CONTINUE_NEEDED");

            this->client_auth_data.state = ClientAuthenticateData::Final;
        }
    }

    credssp::State credssp_client_authenticate_next(InStream & in_stream, OutTransport transport)
    {
        switch (this->client_auth_data.state)
        {
            case ClientAuthenticateData::Start:
                return credssp::State::Err;

            case ClientAuthenticateData::Loop:
            {
                this->ts_request.recv(in_stream);

                // #ifdef WITH_DEBUG_CREDSSP
                // LOG(LOG_ERR, "Receiving Authentication Token (%d)", (int) this->ts_request.negoTokens.cbBuffer);
                // hexdump_c(this->ts_request.negoTokens.pvBuffer, this->ts_request.negoTokens.cbBuffer);
                // #endif
                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Authentication Token");
                this->client_auth_data.input_buffer.copy(this->ts_request.negoTokens);

                /*
                 * from tspkg.dll: 0x00000132
                 * ISC_REQ_MUTUAL_AUTH
                 * ISC_REQ_CONFIDENTIALITY
                 * ISC_REQ_USE_SESSION_KEY
                 * ISC_REQ_ALLOCATE_MEMORY
                 */
                //unsigned long const fContextReq
                //  = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

                SEC_STATUS status = this->sspi_InitializeSecurityContext(
                    bytes_view(this->ServicePrincipalName.av()).as_chars(),
                    this->client_auth_data.input_buffer.av(),
                    /*output*/this->ts_request.negoTokens);

                if ((status != SEC_I_COMPLETE_AND_CONTINUE) &&
                    (status != SEC_I_COMPLETE_NEEDED) &&
                    (status != SEC_E_OK) &&
                    (status != SEC_I_CONTINUE_NEEDED)) {
                    LOG(LOG_ERR, "Initialize Security Context Error !");
                    return credssp::State::Err;
                }

                this->client_auth_data.input_buffer.init(0);

                SEC_STATUS encrypted = SEC_E_INVALID_TOKEN;

                if ((status == SEC_I_COMPLETE_AND_CONTINUE) ||
                    (status == SEC_I_COMPLETE_NEEDED) ||
                    (status == SEC_E_OK)) {
                    // have_pub_key_auth = true;
                    encrypted = this->credssp_encrypt_public_key_echo();
                    if (status == SEC_I_COMPLETE_NEEDED) {
                        status = SEC_E_OK;
                    }
                    else if (status == SEC_I_COMPLETE_AND_CONTINUE) {
                        status = SEC_I_CONTINUE_NEEDED;
                    }
                }

                /* send authentication token to server */
                if ((this->ts_request.negoTokens.size() > 0)||(encrypted == SEC_E_OK)) {
                    // #ifdef WITH_DEBUG_CREDSSP
                    //             LOG(LOG_ERR, "Sending Authentication Token");
                    //             hexdump_c(this->ts_request.negoTokens.pvBuffer, this->ts_request.negoTokens.cbBuffer);
                    // #endif
                    if (this->ts_request.negoTokens.size() > 0){
                        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
                    }

                    LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::send");
                    StaticOutStream<65536> ts_request_emit;
                    this->ts_request.emit(ts_request_emit);
                    transport.get_transport().send(ts_request_emit.get_bytes());

                    this->credssp_buffer_free();
                }

                if (status != SEC_I_CONTINUE_NEEDED) {
                    LOG_IF(this->verbose, LOG_INFO, "rdpCredssp Token loop: CONTINUE_NEEDED");

                    this->client_auth_data.state = ClientAuthenticateData::Final;
                }
                return credssp::State::Cont;
            }
            case ClientAuthenticateData::Final:
                if (Res::Err == this->sm_credssp_client_authenticate_stop(in_stream, transport)) {
                    return credssp::State::Err;
                }
                this->client_auth_data.state = ClientAuthenticateData::Start;
                return credssp::State::Finish;
        }

        return credssp::State::Err;
    }
};

