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

#ifndef __EMSCRIPTEN__

#include "core/RDP/nla/credssp.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "utils/hexdump.hpp"
#include "utils/translation.hpp"
#include "system/ssl_sha256.hpp"

#include "core/RDP/nla/kerberos/kerberos.hpp"

#include "transport/transport.hpp"

class rdpCredsspClientKerberos
{
private:
    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;
    TSRequest ts_request;
        uint32_t error_code = 0;

    ClientNonce SavedClientNonce;

    std::vector<uint8_t> PublicKey;
    std::vector<uint8_t> ClientServerHash;
    std::vector<uint8_t> ServerClientHash;
    std::vector<uint8_t> ServicePrincipalName;
    struct SEC_WINNT_AUTH_IDENTITY
    {
        // kerberos only
        //@{
        char princname[256];
        char princpass[256];
        //@}
        // ntlm only
        //@{
        private:
        std::vector<uint8_t> User;
        std::vector<uint8_t> Domain;
        public:
        std::vector<uint8_t> Password;
        //@}

        public:
        SEC_WINNT_AUTH_IDENTITY()
        {
            this->princname[0] = 0;
            this->princpass[0] = 0;
        }

        void user_init_copy(bytes_view av)
        {
            this->User.assign(av.data(), av.data()+av.size());
        }

        void domain_init_copy(bytes_view av)
        {
            this->Domain.assign(av.data(), av.data()+av.size());
        }

        bool is_empty_user_domain(){
            return (this->User.size() == 0) && (this->Domain.size() == 0);
        }

        bytes_view get_password_utf16_av() const
        {
            return this->Password;
        }

        bytes_view get_user_utf16_av() const
        {
            return this->User;
        }

        bytes_view get_domain_utf16_av() const
        {
            return this->Domain;
        }

        void SetUserFromUtf8(const uint8_t * user)
        {
            if (user) {
                size_t user_len = UTF8Len(user);
                this->User = std::vector<uint8_t>(user_len * 2);
                UTF8toUTF16({user, strlen(char_ptr_cast(user))}, this->User.data(), user_len * 2);
            }
            else {
                this->User.clear();
            }
        }

        void SetDomainFromUtf8(const uint8_t * domain)
        {
            if (domain) {
                size_t domain_len = UTF8Len(domain);
                this->Domain = std::vector<uint8_t>(domain_len * 2);
                UTF8toUTF16({domain, strlen(char_ptr_cast(domain))}, this->Domain.data(), domain_len * 2);
            }
            else {
                this->Domain.clear();
            }
        }

        void SetPasswordFromUtf8(const uint8_t * password)
        {
            if (password) {
                size_t password_len = UTF8Len(password);
                this->Password = std::vector<uint8_t>(password_len * 2);
                UTF8toUTF16({password, strlen(char_ptr_cast(password))}, this->Password.data(), password_len * 2);
            }
            else {
                this->Password.clear();
            }
        }

        void SetKrbAuthIdentity(const uint8_t * user, const uint8_t * pass)
        {
            auto copy = [](char (&arr)[256], uint8_t const* data){
                if (data) {
                    const char * p = char_ptr_cast(data);
                    const size_t length = p ? strnlen(p, 255) : 0;
                    memcpy(arr, data, length);
                    arr[length] = 0;
                }
            };

            copy(this->princname, user);
            copy(this->princpass, pass);
        }

        void clear()
        {
            this->User.clear();
            this->Domain.clear();
            this->Password.clear();
        }

        void CopyAuthIdentity(bytes_view user_utf16_av, bytes_view domain_utf16_av, bytes_view password_utf16_av)
        {
            this->User.assign(user_utf16_av.data(),user_utf16_av.data()+user_utf16_av.size());
            this->Domain.assign(domain_utf16_av.data(),domain_utf16_av.data()+domain_utf16_av.size());
            this->Password.assign(password_utf16_av.data(),password_utf16_av.data()+password_utf16_av.size());
        }

    } identity;
        
    struct Krb5Creds_deleter
    {
        void operator()(Krb5Creds* credentials) const
        {
            credentials->destroy_credentials(nullptr);
            delete credentials; /*NOLINT*/
        }
    };

    using Krb5CredsPtr = std::unique_ptr<Krb5Creds, Krb5Creds_deleter>;
    Krb5CredsPtr sspi_credentials = nullptr;
    std::unique_ptr<KERBEROSContext> sspi_krb_ctx = nullptr;

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    SEC_STATUS sspi_AcquireCredentialsHandle(
        const char * pszPrincipal, std::vector<uint8_t> * pvLogonID, SEC_WINNT_AUTH_IDENTITY const* pAuthData
    ) {
        if (pszPrincipal && pvLogonID) {
            size_t length = strlen(pszPrincipal);
            pvLogonID->assign(pszPrincipal, pszPrincipal+length);
            pvLogonID->push_back(0);
        }
        this->sspi_credentials = Krb5CredsPtr(new Krb5Creds);

        // set KRB5CCNAME cache name to specific with PID,
        // call kinit to get tgt with identity credentials

        int pid = getpid();
        char cache[256];
        snprintf(cache, 255, "FILE:/tmp/krb_red_%d", pid);
        cache[255] = 0;
        setenv("KRB5CCNAME", cache, 1);
        LOG(LOG_INFO, "set KRB5CCNAME to %s", cache);
        if (pAuthData) {
            int ret = this->sspi_credentials->get_credentials(pAuthData->princname,
                                                         pAuthData->princpass, nullptr);
            if (!ret) {
                return SEC_E_OK;
            }
        }
        return SEC_E_NO_CREDENTIALS;
    }

    bool sspi_get_service_name(array_view_const_char server, gss_name_t * name) {
        gss_buffer_desc output;
        OM_uint32 major_status, minor_status;
        const char service_name[] = "TERMSRV";
        gss_OID type = GSS_C_NT_HOSTBASED_SERVICE;
        int size = (strlen(service_name) + 1 + server.size() + 1);

        auto output_value = std::make_unique<char[]>(size);
        output.value = output_value.get();
        snprintf(static_cast<char*>(output.value), size, "%s@%.*s",
            service_name, int(server.size()), server.data());
        output.length = strlen(static_cast<char*>(output.value)) + 1;
        LOG(LOG_INFO, "GSS IMPORT NAME : %s", static_cast<char*>(output.value));
        major_status = gss_import_name(&minor_status, &output, type, name);
        if (GSS_ERROR(major_status)) {
            LOG(LOG_ERR, "Failed to create service principal name");
            return false;
        }
        return true;
    }


    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    SEC_STATUS sspi_InitializeSecurityContext(
        array_view_const_char pszTargetName, array_view_const_u8 input_buffer, std::vector<uint8_t> & output_buffer
    ) 
    {
        OM_uint32 major_status, minor_status;

        gss_cred_id_t gss_no_cred = GSS_C_NO_CREDENTIAL;
        if (!this->sspi_krb_ctx) {
            // LOG(LOG_INFO, "Initialiaze Sec Ctx: NO CONTEXT");
            this->sspi_krb_ctx = std::make_unique<KERBEROSContext>();

            // Target name (server name, ip ...)
            if (!this->sspi_get_service_name(pszTargetName, &this->sspi_krb_ctx->target_name)) {
                return SEC_E_WRONG_PRINCIPAL;
            }
        }
        // else {
        //     LOG(LOG_INFO, "Initialiaze Sec CTX: USE FORMER CONTEXT");
        // }

        // Token Buffer
        gss_buffer_desc input_tok, output_tok;
        output_tok.length = 0;
        // LOG(LOG_INFO, "GOT INPUT BUFFER: length %d",
        //     input_buffer->Buffer.size());
        input_tok.length = input_buffer.size();
        input_tok.value = const_cast<uint8_t*>(input_buffer.data()); /*NOLINT*/

        gss_OID_desc desired_mech = _gss_spnego_krb5_mechanism_oid_desc();
        if (!this->sspi_mech_available(&desired_mech)) {
            LOG(LOG_ERR, "Desired Mech unavailable");
            return SEC_E_CRYPTO_SYSTEM_INVALID;
        }
// OM_uint32 gss_init_sec_context
//                  (OM_uint32 ,             /* minor_status */
//                   const gss_cred_id_t,    /* initiator_cred_handle */
//                   gss_ctx_id_t ,          /* context_handle */
//                   const gss_name_t,       /* target_name */
//                   const gss_OID,          /* mech_type */
//                   OM_uint32,              /* req_flags */
//                   OM_uint32,              /* time_req */
//                   const gss_channel_bindings_t,
//                                           /* input_chan_bindings */
//                   const gss_buffer_t,     /* input_token */
//                   gss_OID ,               /* actual_mech_type */
//                   gss_buffer_t,           /* output_token */
//                   OM_uint32 ,             /* ret_flags */
//                   OM_uint32 *             /* time_rec */
//                  );
        major_status = gss_init_sec_context(&minor_status,
                                            gss_no_cred,
                                            &this->sspi_krb_ctx->gss_ctx,
                                            this->sspi_krb_ctx->target_name,
                                            &desired_mech,
                                            GSS_C_MUTUAL_FLAG,
                                            GSS_C_INDEFINITE,
                                            GSS_C_NO_CHANNEL_BINDINGS,
                                            &input_tok,
                                            &this->sspi_krb_ctx->actual_mech,
                                            &output_tok,
                                            &this->sspi_krb_ctx->actual_services,
                                            &this->sspi_krb_ctx->actual_time);

        if (GSS_ERROR(major_status)) {
            LOG(LOG_INFO, "MAJOR ERROR");
            this->sspi_report_error(GSS_C_GSS_CODE, "CredSSP: SPNEGO negotiation failed.",
                               major_status, minor_status);
            return SEC_E_OUT_OF_SEQUENCE;
        }

        // LOG(LOG_INFO, "output tok length : %d", output_tok.length);
        output_buffer.assign(static_cast<uint8_t const*>(output_tok.value), static_cast<uint8_t const*>(output_tok.value)+output_tok.length);

        (void) gss_release_buffer(&minor_status, &output_tok);

        if (major_status & GSS_S_CONTINUE_NEEDED) {
            // LOG(LOG_INFO, "MAJOR CONTINUE NEEDED");
            (void) gss_release_buffer(&minor_status, &input_tok);
            return SEC_I_CONTINUE_NEEDED;
        }
        // LOG(LOG_INFO, "MAJOR COMPLETE NEEDED");
        return SEC_I_COMPLETE_NEEDED;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    // Not used by client

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    SEC_STATUS sspi_EncryptMessage(array_view_const_u8 data_in, std::vector<uint8_t>& data_out, unsigned long MessageSeqNo) {
        (void)MessageSeqNo;
        // OM_uint32 KRB5_CALLCONV
        // gss_wrap(
        //     OM_uint32 *,        /* minor_status */
        //     gss_ctx_id_t,       /* context_handle */
        //     int,                /* conf_req_flag */
        //     gss_qop_t,          /* qop_req */
        //     gss_buffer_t,       /* input_message_buffer */
        //     int *,              /* conf_state */
        //     gss_buffer_t);      /* output_message_buffer */

        OM_uint32 major_status;
        OM_uint32 minor_status;
        int conf_state;
        if (!this->sspi_krb_ctx) {
            return SEC_E_NO_CONTEXT;
        }
        gss_buffer_desc inbuf, outbuf;

        inbuf.value = const_cast<uint8_t*>(data_in.data()); /*NOLINT*/
        inbuf.length = data_in.size();
        // LOG(LOG_INFO, "GSS_WRAP inbuf length : %d", inbuf.length);
        major_status = gss_wrap(&minor_status, this->sspi_krb_ctx->gss_ctx, true,
			    GSS_C_QOP_DEFAULT, &inbuf, &conf_state, &outbuf);
        if (GSS_ERROR(major_status)) {
            LOG(LOG_INFO, "MAJOR ERROR");
            this->sspi_report_error(GSS_C_GSS_CODE, "CredSSP: GSS WRAP failed.",
                               major_status, minor_status);
            return SEC_E_ENCRYPT_FAILURE;
        }
        // LOG(LOG_INFO, "GSS_WRAP outbuf length : %d", outbuf.length);
        data_out.assign(static_cast<uint8_t const*>(outbuf.value), static_cast<uint8_t const*>(outbuf.value)+ outbuf.length);
        gss_release_buffer(&minor_status, &outbuf);

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    SEC_STATUS sspi_DecryptMessage(array_view_const_u8 data_in, std::vector<uint8_t>& data_out, unsigned long MessageSeqNo) {
        (void)MessageSeqNo;

        // OM_uint32 gss_unwrap
        //     (OM_uint32 ,             /* minor_status */
        //      const gss_ctx_id_t,     /* context_handle */
        //      const gss_buffer_t,     /* input_message_buffer */
        //      gss_buffer_t,           /* output_message_buffer */
        //      int ,                   /* conf_state */
        //      gss_qop_t *             /* qop_state */
        //      );

        OM_uint32 major_status;
        OM_uint32 minor_status;
        int conf_state;
        gss_qop_t qop_state;
        if (!this->sspi_krb_ctx) {
            return SEC_E_NO_CONTEXT;
        }
        gss_buffer_desc inbuf, outbuf;
        inbuf.value = const_cast<uint8_t*>(data_in.data()); /*NOLINT*/
        inbuf.length = data_in.size();
        // LOG(LOG_INFO, "GSS_UNWRAP inbuf length : %d", inbuf.length);
        major_status = gss_unwrap(&minor_status, this->sspi_krb_ctx->gss_ctx, &inbuf, &outbuf,
                                  &conf_state, &qop_state);
        if (GSS_ERROR(major_status)) {
            LOG(LOG_INFO, "MAJOR ERROR");
            this->sspi_report_error(GSS_C_GSS_CODE, "CredSSP: GSS UNWRAP failed.",
                               major_status, minor_status);
            return SEC_E_DECRYPT_FAILURE;
        }
        // LOG(LOG_INFO, "GSS_UNWRAP outbuf length : %d", outbuf.length);
        data_out.assign(static_cast<uint8_t const*>(outbuf.value),static_cast<uint8_t const*>(outbuf.value)+ outbuf.length);
        gss_release_buffer(&minor_status, &outbuf);
        return SEC_E_OK;
    }

    void sspi_report_error(OM_uint32 code, const char *str,
                      OM_uint32 major_status, OM_uint32 minor_status)
    {
        OM_uint32 msgctx = 0, ms;
        gss_buffer_desc status_string;

        LOG(LOG_ERR, "GSS error [%u:%u:%u]: %s",
            (major_status & 0xff000000) >> 24,	// Calling error
            (major_status & 0xff0000) >> 16,	// Routine error
            major_status & 0xffff,	// Supplementary info bits
            str);

        LOG(LOG_ERR, "GSS Minor status error [%u:%u:%u]:%u %s",
            (minor_status & 0xff000000) >> 24,	// Calling error
            (minor_status & 0xff0000) >> 16,	// Routine error
            minor_status & 0xffff,	// Supplementary info bits
            minor_status,
            str);

        do {
            ms = gss_display_status(
                &minor_status, major_status,
                code, GSS_C_NULL_OID, &msgctx, &status_string);
        	if (ms != GSS_S_COMPLETE) {
                continue;
            }

            LOG(LOG_ERR," - %s", static_cast<char const*>(status_string.value));
        }
        while (ms == GSS_S_COMPLETE && msgctx);

    }

    bool sspi_mech_available(gss_OID mech)
    {
        int mech_found;
        OM_uint32 major_status, minor_status;
        gss_OID_set mech_set;

        mech_found = 0;

        if (mech == GSS_C_NO_OID) {
            return true;
        }

        major_status = gss_indicate_mechs(&minor_status, &mech_set);
        if (!mech_set) {
            return false;
        }
        if (GSS_ERROR(major_status)) {
            this->sspi_report_error(GSS_C_GSS_CODE, "Failed to get available mechs on system",
                               major_status, minor_status);
            gss_release_oid_set(&minor_status, &mech_set);
            return false;
        }

        gss_test_oid_set_member(&minor_status, mech, mech_set, &mech_found);

        gss_release_oid_set(&minor_status, &mech_set);
        if (GSS_ERROR(major_status)) {
            this->sspi_report_error(GSS_C_GSS_CODE, "Failed to match mechanism in set",
                                  major_status, minor_status);
            return false;
        }

        return mech_found != 0;
    }

    bool restricted_admin_mode;

    const char * target_host;
    Random & rand;
    std::string& extra_message;
    Translation::language_t lang;
    const bool verbose;

    Transport & trans;

    void credssp_send()
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::send");
        StaticOutStream<65536> ts_request_emit;
        auto v = emitTSRequest(this->ts_request);
        this->error_code = this->ts_request.error_code;
        ts_request_emit.out_copy_bytes(v);
        this->trans.send(ts_request_emit.get_bytes());
    }

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        this->ServicePrincipalName.assign(pszTargetName, pszTargetName+length);
        this->ServicePrincipalName.push_back(0);
    }

    void credssp_generate_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientKerberos::credssp generate client nonce");
        this->rand.random(this->SavedClientNonce.clientNonce.data(), CLIENT_NONCE_LENGTH);
        this->SavedClientNonce.initialized = true;
        this->credssp_set_client_nonce();
    }

    void credssp_get_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientKerberos::credssp get client nonce");
        if (this->ts_request.clientNonce.isset()){
            this->SavedClientNonce = this->ts_request.clientNonce;
        }
    }
    void credssp_set_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientKerberos::credssp set client nonce");
        if (!this->ts_request.clientNonce.isset()) {
            this->ts_request.clientNonce = this->SavedClientNonce;
        }
    }

    void credssp_generate_public_key_hash_client_to_server() {
        LOG(LOG_INFO, "rdpCredsspClientKerberos::generate credssp public key hash (client->server)");
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
        sha256.update(this->SavedClientNonce.clientNonce);
        sha256.update({this->PublicKey.data(),this->PublicKey.size()});
        sha256.final(hash);
        
        this->ClientServerHash.assign(hash, hash+sizeof(hash));
    }

    void credssp_generate_public_key_hash_server_to_client() {
        LOG(LOG_INFO, "rdpCredsspClientKerberos::generate credssp public key hash (server->client)");
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Server-To-Client Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.clientNonce.data(), CLIENT_NONCE_LENGTH));
        sha256.update({this->PublicKey.data(),this->PublicKey.size()});
        sha256.final(hash);
        this->ServerClientHash.assign(hash, hash + sizeof(hash));
    }

    SEC_STATUS credssp_encrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::encrypt_public_key_echo");
        uint32_t version = this->ts_request.use_version;

        array_view_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};;
        if (version >= 5) {
            this->credssp_generate_client_nonce();
            this->credssp_generate_public_key_hash_client_to_server();
            public_key = {this->ClientServerHash.data(),this->ClientServerHash.size()};
        }

        return this->sspi_EncryptMessage(public_key, this->ts_request.pubKeyAuth, this->send_seq_num++);
    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::decrypt_public_key_echo");

        std::vector<uint8_t> Buffer;

        SEC_STATUS const status = this->sspi_DecryptMessage(
            this->ts_request.pubKeyAuth, Buffer, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "sspi_DecryptMessage failure: 0x%08X", status);
            return status;
        }

        const uint32_t version = this->ts_request.use_version;

        array_view_const_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};
        if (version >= 5) {
            this->credssp_get_client_nonce();
            this->credssp_generate_public_key_hash_server_to_client();
            public_key = {this->ServerClientHash.data(), this->ServerClientHash.size()};
        }

        array_view_u8 public_key2 = Buffer;

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
            hexdump_c(public_key);

            LOG(LOG_ERR, "Actual (length = %zu):", public_key.size());
            hexdump_c(public_key2);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        return SEC_E_OK;
    }

    void credssp_buffer_free() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::buffer_free");
        this->ts_request.negoTokens.clear();
        this->ts_request.pubKeyAuth.clear();
        this->ts_request.authInfo.clear();
        this->ts_request.clientNonce.reset();
    }

    enum class Res : bool { Err, Ok };

    struct ClientAuthenticateData
    {
        enum : uint8_t { Start, Loop, Final } state = Start;
        std::vector<uint8_t> input_buffer;
    };
    ClientAuthenticateData client_auth_data;

    Res sm_credssp_client_authenticate_send()
    {
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
            bytes_view(this->ServicePrincipalName).as_chars(),
            this->client_auth_data.input_buffer, /*output*/this->ts_request.negoTokens);
        if ((status != SEC_I_COMPLETE_AND_CONTINUE) &&
            (status != SEC_I_COMPLETE_NEEDED) &&
            (status != SEC_E_OK) &&
            (status != SEC_I_CONTINUE_NEEDED)) {
            LOG(LOG_ERR, "Initialize Security Context Error !");
            return Res::Err;
        }

        this->client_auth_data.input_buffer.clear();

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

        if (this->ts_request.negoTokens.size() > 0) {
            // #ifdef WITH_DEBUG_CREDSSP
            //             LOG(LOG_ERR, "Sending Authentication Token");
            //             hexdump_c(this->ts_request.negoTokens.pvBuffer, this->ts_request.negoTokens.cbBuffer);
            // #endif
            LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");

            this->credssp_send();
            this->credssp_buffer_free();
        }
        else if (encrypted == SEC_E_OK) {
            this->credssp_send();
            this->credssp_buffer_free();
        }

        if (status != SEC_I_CONTINUE_NEEDED) {
            LOG_IF(this->verbose, LOG_INFO, "rdpCredssp Token loop: CONTINUE_NEEDED");

            this->client_auth_data.state = ClientAuthenticateData::Final;
            return Res::Ok;
        }
        /* receive server response and place in input buffer */

        return Res::Ok;
    }

    Res sm_credssp_client_authenticate_stop(bytes_view in_data)
    {
        /* Encrypted Public Key +1 */
        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Encrypted PubKey + 1");

        this->ts_request = recvTSRequest(in_data);

        /* Verify Server Public Key Echo */

        SEC_STATUS status = this->credssp_decrypt_public_key_echo();
        this->credssp_buffer_free();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!");
            this->credssp_buffer_free();
            return Res::Err;
        }

        /* Send encrypted credentials */

        status = this->credssp_encrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "credssp_encrypt_ts_credentials status: 0x%08X", status);
            return Res::Err;
        }

        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Credentials");

        this->credssp_send();

        /* Free resources */
        this->credssp_buffer_free();


        return Res::Ok;
    }

    SEC_STATUS credssp_encrypt_ts_credentials() {

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::encrypt_ts_credentials");

        StaticOutStream<65536> ts_credentials_send;
        std::vector<uint8_t> result;
        if (this->ts_credentials.credType == 1){
            if (this->restricted_admin_mode) {
                LOG(LOG_INFO, "Restricted Admin Mode");
                result = emitTSCredentialsPassword({},{}, {});
            }
            else {
                result = emitTSCredentialsPassword(this->identity.get_domain_utf16_av(),this->identity.get_user_utf16_av(),this->identity.get_password_utf16_av());
            }
        }
        else {
            // Card Reader Not Supported Yet
            bytes_view pin;
            bytes_view userHint;
            bytes_view domainHint; 
            uint32_t keySpec = 0;
            bytes_view cardName;
            bytes_view readerName;
            bytes_view containerName;
            bytes_view cspName;
            result = emitTSCredentialsSmartCard(pin,userHint,domainHint,keySpec,cardName,readerName,containerName, cspName);
        }
        ts_credentials_send.out_copy_bytes(result);

        return this->sspi_EncryptMessage(
            {ts_credentials_send.get_data(), ts_credentials_send.get_offset()},
            this->ts_request.authInfo, this->send_seq_num++);
    }

    void set_credentials(uint8_t const* user, uint8_t const* domain,
                         uint8_t const* pass, uint8_t const* hostname) {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::set_credentials");
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
    rdpCredsspClientKerberos(OutTransport transport,
               uint8_t * user,
               uint8_t * domain,
               uint8_t * pass,
               uint8_t * hostname,
               const char * target_host,
               const bool restricted_admin_mode,
               Random & rand,
               std::string& extra_message,
               Translation::language_t lang,
               const bool verbose = false)
        : ts_request(6) // Credssp Version 6 Supported
        , SavedClientNonce()
        , restricted_admin_mode(restricted_admin_mode)
        , target_host(target_host)
        , rand(rand)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
        , trans(transport.get_transport())
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::Initialization");
        this->set_credentials(user, domain, pass, hostname);
        this->client_auth_data.state = ClientAuthenticateData::Start;

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::client_authenticate");
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientKerberos::ntlm_init");

        // ============================================
        /* Get Public Key From TLS Layer and hostname */
        // ============================================

        auto const key = this->trans.get_public_key();
        this->PublicKey.assign(key.data(), key.data()+key.size());

        LOG(LOG_INFO, "Credssp: KERBEROS Authentication");

        SEC_STATUS status = this->sspi_AcquireCredentialsHandle(this->target_host, &this->ServicePrincipalName, &this->identity);

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Kerberos InitSecurityInterface status:%s0x%08X, fallback to NTLM",
                (status == SEC_E_NO_CREDENTIALS)?" No Credentials ":" ", status);
            throw Error(ERR_CREDSSP_KERBEROS_INIT_FAILED);
        }

        this->client_auth_data.input_buffer.clear();

        this->client_auth_data.state = ClientAuthenticateData::Loop;
        if (Res::Err == this->sm_credssp_client_authenticate_send()){
            throw Error(ERR_CREDSSP_KERBEROS_INIT_FAILED);
        }
    }

    ~rdpCredsspClientKerberos()
    {
        this->sspi_krb_ctx.reset();
        this->sspi_credentials.reset();
        unsetenv("KRB5CCNAME");
    }

    credssp::State credssp_client_authenticate_next(bytes_view in_data)
    {
        switch (this->client_auth_data.state)
        {
            case ClientAuthenticateData::Start:
                return credssp::State::Err;
            case ClientAuthenticateData::Loop:
            
                this->ts_request = recvTSRequest(in_data);
                this->error_code = this->ts_request.error_code;

                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Authentication Token");
                this->client_auth_data.input_buffer.assign(this->ts_request.negoTokens.data(),this->ts_request.negoTokens.data()+this->ts_request.negoTokens.size());
            
                if (Res::Err == this->sm_credssp_client_authenticate_send()) {
                    return credssp::State::Err;
                }
                return credssp::State::Cont;
            case ClientAuthenticateData::Final:
                if (Res::Err == this->sm_credssp_client_authenticate_stop(in_data)) {
                    return credssp::State::Err;
                }
                this->client_auth_data.state = ClientAuthenticateData::Start;
                return credssp::State::Finish;
        }

        return credssp::State::Err;
    }
};

#endif
