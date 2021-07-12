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

#include "core/RDP/nla/credssp.hpp"
#include "utils/hexdump.hpp"
#include "system/ssl_sha256.hpp"
#include "utils/translation.hpp"

#ifndef __EMSCRIPTEN__
#include "core/RDP/nla/kerberos.hpp"
#endif


class rdpCredsspServerKerberos final
{

    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;

    TSRequest ts_request = {6}; // Credssp Version 6 Supported
    uint32_t error_code = 0;
    static const size_t CLIENT_NONCE_LENGTH = 32;
    ClientNonce SavedClientNonce;

    writable_u8_array_view public_key;
    std::string& extra_message;
    Language lang;
    const bool credssp_verbose;
    const bool verbose;

    std::vector<uint8_t> ClientServerHash;
    std::vector<uint8_t> ServerClientHash;

    std::vector<uint8_t> ServicePrincipalName;
    public:
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
            return (this->User.empty() && this->Domain.empty());
        }

        [[nodiscard]] bytes_view get_password_utf16_av() const
        {
            return this->Password;
        }

        [[nodiscard]] bytes_view get_user_utf16_av() const
        {
            return this->User;
        }

        [[nodiscard]] bytes_view get_domain_utf16_av() const
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

    public:
    struct Kerberos_SecurityFunctionTable
    {
    private:
        struct Krb5Creds_deleter
        {
            void operator()(Krb5Creds* credentials) const
            {
                credentials->destroy_credentials(nullptr);
                delete credentials; /*NOLINT*/
            }
        };

        using Krb5CredsPtr = std::unique_ptr<Krb5Creds, Krb5Creds_deleter>;
        Krb5CredsPtr credentials = nullptr;
        std::unique_ptr<KERBEROSContext> krb_ctx = nullptr;

    public:
        ~Kerberos_SecurityFunctionTable()
        {
            this->krb_ctx.reset();
            this->credentials.reset();
            unsetenv("KRB5CCNAME");
        }

        // GSS_Acquire_cred
        // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
        SEC_STATUS AcquireCredentialsHandle(
            const char * pszPrincipal, std::vector<uint8_t> * pvLogonID, SEC_WINNT_AUTH_IDENTITY const* pAuthData
        )
        {
            if (pszPrincipal && pvLogonID) {
                size_t length = strlen(pszPrincipal);
                pvLogonID->assign(pszPrincipal,pszPrincipal+length);
                pvLogonID->push_back(0);
            }
            this->credentials = Krb5CredsPtr(new Krb5Creds);

            // set KRB5CCNAME cache name to specific with PID,
            // call kinit to get tgt with identity credentials

            int pid = getpid();
            char cache[256];
            snprintf(cache, 255, "FILE:/tmp/krb_red_%d", pid);
            cache[255] = 0;
            setenv("KRB5CCNAME", cache, 1);
            LOG(LOG_INFO, "set KRB5CCNAME to %s", cache);
            if (pAuthData) {
                int ret = this->credentials->get_credentials_password(pAuthData->princname,
                                                             pAuthData->princpass,
                                                             nullptr, nullptr);
                if (!ret) {
                    return SEC_E_OK;
                }
            }
            return SEC_E_NO_CREDENTIALS;
        }

        bool get_service_name(chars_view server, gss_name_t * name) {
            gss_buffer_desc output;
            const char* service_name = "TERMSRV";
            gss_OID type = GSS_C_NT_HOSTBASED_SERVICE;
            auto size = (strlen(service_name) + 1 + server.size() + 1);
            auto output_value = std::make_unique<char[]>(size);
            output.value = output_value.get();
            snprintf(static_cast<char*>(output.value), size, "%s@%.*s",
                service_name, int(server.size()), server.data());
            output.length = strlen(static_cast<char*>(output.value)) + 1;
            LOG(LOG_INFO, "GSS IMPORT NAME : %s", static_cast<char*>(output.value));
            OM_uint32 minor_status = 0;
            OM_uint32 major_status = gss_import_name(&minor_status, &output, type, name);
            if (GSS_ERROR(major_status)) {
                LOG(LOG_ERR, "Failed to create service principal name");
                return false;
            }
            return true;
        }


        // GSS_Init_sec_context
        // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
        SEC_STATUS InitializeSecurityContext(
            chars_view pszTargetName, u8_array_view input_buffer, std::vector<uint8_t>& output_buffer
        )
        {
            gss_cred_id_t gss_no_cred = GSS_C_NO_CREDENTIAL;
            if (!this->krb_ctx) {
                // LOG(LOG_INFO, "Initialiaze Sec Ctx: NO CONTEXT");
                this->krb_ctx = std::make_unique<KERBEROSContext>();

                // Target name (server name, ip ...)
                if (!this->get_service_name(pszTargetName, &krb_ctx->target_name)) {
                    return SEC_E_WRONG_PRINCIPAL;
                }
            }
            // else {
            //     LOG(LOG_INFO, "Initialiaze Sec CTX: USE FORMER CONTEXT");
            // }


            // Token Buffer
            gss_buffer_desc input_tok;
            gss_buffer_desc output_tok;
            output_tok.length = 0;
            // LOG(LOG_INFO, "GOT INPUT BUFFER: length %d",
            //     input_buffer->Buffer.size());
            input_tok.length = input_buffer.size();
            input_tok.value = const_cast<uint8_t*>(input_buffer.data()); /*NOLINT*/

            gss_OID_desc desired_mech = _gss_spnego_krb5_mechanism_oid_desc();
            if (!this->mech_available(&desired_mech)) {
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
            OM_uint32 minor_status = 0;

            OM_uint32 major_status = gss_init_sec_context(&minor_status,
                                                gss_no_cred,
                                                &this->krb_ctx->gss_ctx,
                                                this->krb_ctx->target_name,
                                                &desired_mech,
                                                GSS_C_MUTUAL_FLAG,
                                                GSS_C_INDEFINITE,
                                                GSS_C_NO_CHANNEL_BINDINGS,
                                                &input_tok,
                                                &this->krb_ctx->actual_mech,
                                                &output_tok,
                                                &this->krb_ctx->actual_services,
                                                &this->krb_ctx->actual_time);

            if (GSS_ERROR(major_status)) {
                LOG(LOG_INFO, "MAJOR ERROR");
                this->report_error(GSS_C_GSS_CODE, "CredSSP: SPNEGO negotiation failed.",
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
        SEC_STATUS AcceptSecurityContext(
            u8_array_view input_buffer, std::vector<uint8_t>& output_buffer
        )
        {
            gss_cred_id_t gss_no_cred = GSS_C_NO_CREDENTIAL;
            if (!this->krb_ctx) {
                // LOG(LOG_INFO, "Initialiaze Sec Ctx: NO CONTEXT");
                this->krb_ctx = std::make_unique<KERBEROSContext>();
            }
            // else {
            //     LOG(LOG_INFO, "Initialiaze Sec CTX: USE FORMER CONTEXT");
            // }


            // Token Buffer
            gss_buffer_desc input_tok;
            gss_buffer_desc output_tok;
            output_tok.length = 0;

            // LOG(LOG_INFO, "GOT INPUT BUFFER: length %d",
            //     input_buffer->Buffer.size());
            input_tok.length = input_buffer.size();
            input_tok.value = const_cast<uint8_t*>(input_buffer.data()); /*NOLINT*/

            gss_OID_desc desired_mech = _gss_spnego_krb5_mechanism_oid_desc();
            if (!this->mech_available(&desired_mech)) {
                LOG(LOG_ERR, "Desired Mech unavailable");
                return SEC_E_CRYPTO_SYSTEM_INVALID;
            }

            // acquire delegated credential handle if client has tgt with delegation flag
            // should be freed with gss_release_cred()

            // OM_uint32 gss_accept_sec_context
            //               (OM_uint32 ,             /* minor_status */
            //                gss_ctx_id_t ,          /* context_handle */
            //                const gss_cred_id_t,    /* acceptor_cred_handle */
            //                const gss_buffer_t,     /* input_token_buffer */
            //                const gss_channel_bindings_t,
            //                                        /* input_chan_bindings */
            //                gss_name_t ,            /* src_name */
            //                gss_OID ,               /* mech_type */
            //                gss_buffer_t,           /* output_token */
            //                OM_uint32 ,             /* ret_flags */
            //                OM_uint32 ,             /* time_rec */
            //                gss_cred_id_t *         /* delegated_cred_handle */
            //               );
            OM_uint32 minor_status = 0;
            OM_uint32 major_status = gss_accept_sec_context(&minor_status,
                                                  &this->krb_ctx->gss_ctx,
                                                  gss_no_cred,
                                                  &input_tok,
                                                  GSS_C_NO_CHANNEL_BINDINGS,
                                                  nullptr,
                                                  &this->krb_ctx->actual_mech,
                                                  &output_tok,
                                                  &this->krb_ctx->actual_flag,
                                                  &this->krb_ctx->actual_time,
                                                  &this->krb_ctx->deleg_cred);

            if (GSS_ERROR(major_status)) {
                LOG(LOG_INFO, "MAJOR ERROR");
                this->report_error(GSS_C_GSS_CODE, "CredSSP: SPNEGO negotiation failed.",
                                   major_status, minor_status);
                return SEC_E_OUT_OF_SEQUENCE;
            }

            // LOG(LOG_INFO, "output tok length : %d", output_tok.length);
            output_buffer.assign(static_cast<const uint8_t*>(output_tok.value),static_cast<const uint8_t*>(output_tok.value)+ output_tok.length);

            (void) gss_release_buffer(&minor_status, &output_tok);

            if (major_status & GSS_S_CONTINUE_NEEDED) {
                // LOG(LOG_INFO, "MAJOR CONTINUE NEEDED");
                return SEC_I_CONTINUE_NEEDED;
            }
            // LOG(LOG_INFO, "MAJOR COMPLETE NEEDED");
            return SEC_I_COMPLETE_NEEDED;
        }

        // GSS_Wrap
        // ENCRYPT_MESSAGE EncryptMessage;
        SEC_STATUS EncryptMessage(u8_array_view data_in, std::vector<uint8_t>& data_out, unsigned long MessageSeqNo)
        {
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

            int conf_state;
            if (!this->krb_ctx) {
                return SEC_E_NO_CONTEXT;
            }
            gss_buffer_desc inbuf;
            gss_buffer_desc outbuf;

            inbuf.value = const_cast<uint8_t*>(data_in.data()); /*NOLINT*/
            inbuf.length = data_in.size();
            // LOG(LOG_INFO, "GSS_WRAP inbuf length : %d", inbuf.length);
            OM_uint32 minor_status = 0;
            OM_uint32 major_status = gss_wrap(&minor_status, this->krb_ctx->gss_ctx, true,
                    GSS_C_QOP_DEFAULT, &inbuf, &conf_state, &outbuf);
            if (GSS_ERROR(major_status)) {
                LOG(LOG_INFO, "MAJOR ERROR");
                this->report_error(GSS_C_GSS_CODE, "CredSSP: GSS WRAP failed.",
                                   major_status, minor_status);
                return SEC_E_ENCRYPT_FAILURE;
            }
            // LOG(LOG_INFO, "GSS_WRAP outbuf length : %d", outbuf.length);
            data_out.assign(static_cast<uint8_t const*>(outbuf.value),static_cast<uint8_t const*>(outbuf.value)+ outbuf.length);
            gss_release_buffer(&minor_status, &outbuf);

            return SEC_E_OK;
        }

        // GSS_Unwrap
        // DECRYPT_MESSAGE DecryptMessage;
        SEC_STATUS DecryptMessage(u8_array_view data_in, std::vector<uint8_t>& data_out, unsigned long MessageSeqNo)
        {
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
            if (!this->krb_ctx) {
                return SEC_E_NO_CONTEXT;
            }
            gss_buffer_desc inbuf;
            gss_buffer_desc outbuf;
            inbuf.value = const_cast<uint8_t*>(data_in.data()); /*NOLINT*/
            inbuf.length = data_in.size();
            // LOG(LOG_INFO, "GSS_UNWRAP inbuf length : %d", inbuf.length);
            major_status = gss_unwrap(&minor_status, this->krb_ctx->gss_ctx, &inbuf, &outbuf,
                                      &conf_state, &qop_state);
            if (GSS_ERROR(major_status)) {
                LOG(LOG_INFO, "MAJOR ERROR");
                this->report_error(GSS_C_GSS_CODE, "CredSSP: GSS UNWRAP failed.",
                                   major_status, minor_status);
                return SEC_E_DECRYPT_FAILURE;
            }
            // LOG(LOG_INFO, "GSS_UNWRAP outbuf length : %d", outbuf.length);
            data_out.assign(static_cast<uint8_t const*>(outbuf.value), static_cast<uint8_t const*>(outbuf.value)+outbuf.length);
            gss_release_buffer(&minor_status, &outbuf);
            return SEC_E_OK;
        }

        void report_error(OM_uint32 code, const char *str,
                          OM_uint32 major_status, OM_uint32 minor_status)
        {
            OM_uint32 msgctx = 0;
            OM_uint32 ms;
            gss_buffer_desc status_string;

            LOG(LOG_ERR, "GSS error [%u:%u:%u]: %s",
                (major_status & 0xff000000) >> 24,    // Calling error
                (major_status & 0xff0000) >> 16,    // Routine error
                major_status & 0xffff,    // Supplementary info bits
                str);

            LOG(LOG_ERR, "GSS Minor status error [%u:%u:%u]:%u %s",
                (minor_status & 0xff000000) >> 24,    // Calling error
                (minor_status & 0xff0000) >> 16,    // Routine error
                minor_status & 0xffff,    // Supplementary info bits
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

        bool mech_available(gss_OID mech)
        {
            int mech_found;
            OM_uint32 major_status;
            OM_uint32 minor_status;
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
                this->report_error(GSS_C_GSS_CODE, "Failed to get available mechs on system",
                                   major_status, minor_status);
                gss_release_oid_set(&minor_status, &mech_set);
                return false;
            }

            gss_test_oid_set_member(&minor_status, mech, mech_set, &mech_found);

            gss_release_oid_set(&minor_status, &mech_set);
            if (GSS_ERROR(major_status)) {
                this->report_error(GSS_C_GSS_CODE, "Failed to match mechanism in set",
                                      major_status, minor_status);
                return false;
            }

            return mech_found != 0;
        }

    } table;

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        this->ServicePrincipalName.assign(pszTargetName, pszTargetName + length);
        this->ServicePrincipalName.push_back(0);
    }

    void credssp_generate_public_key_hash_client_to_server() {
        LOG(LOG_DEBUG, "rdpCredsspServer::generate credssp public key hash (client->server)");
        auto & SavedHash = this->ClientServerHash;
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
        sha256.update(this->SavedClientNonce.clientNonce);
        sha256.update(this->public_key);
        sha256.final(make_writable_sized_array_view(hash));
        SavedHash = std::vector<uint8_t>(sizeof(hash));
        memcpy(SavedHash.data(), hash, sizeof(hash));
    }

    void credssp_generate_public_key_hash_server_to_client() {
        LOG(LOG_DEBUG, "rdpCredsspServer::generate credssp public key hash (server->client)");
        auto & SavedHash = this->ServerClientHash;
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Server-To-Client Binding Hash\0"_av);
        sha256.update(this->SavedClientNonce.clientNonce);
        sha256.update(this->public_key);
        sha256.final(make_writable_sized_array_view(hash));
        SavedHash = std::vector<uint8_t>(sizeof(hash));
        memcpy(SavedHash.data(), hash, sizeof(hash));
    }


    void credssp_buffer_free() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::buffer_free");
        this->ts_request.negoTokens.clear();
        this->ts_request.pubKeyAuth.clear();
        this->ts_request.authInfo.clear();
        this->ts_request.clientNonce.reset();
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
    rdpCredsspServerKerberos(writable_u8_array_view key,
               std::string& extra_message,
               Language lang,
               const bool credssp_verbose,
               const bool verbose)
        : public_key(key)
        , extra_message(extra_message)
        , lang(lang)
        , credssp_verbose(credssp_verbose)
        , verbose(verbose)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::Initialization");
        this->identity.SetUserFromUtf8(nullptr);
        this->identity.SetDomainFromUtf8(nullptr);
        this->identity.SetPasswordFromUtf8(nullptr);
        this->SetHostnameFromUtf8(nullptr);
        this->identity.SetKrbAuthIdentity(nullptr, nullptr);

        this->server_auth_data.state = ServerAuthenticateData::Start;

        // TODO: see how to correctly support kerberos init error
        // as we are in a constructor this should be done using exception
        // or setting some status field in the object returning a value is
        // not an option
        if (SEC_E_OK != this->table.AcquireCredentialsHandle(
                            /*char* pszPrincipal*/nullptr,
                            /*Array* pvLogonID*/nullptr,
                            /*SEC_WINNT_AUTH_IDENTITY const* pAuthData*/nullptr))
        {
            LOG(LOG_ERR, "InitSecurityInterface status: SEC_E_NO_CREDENTIALS");
            return;
        }

        /*
        * from tspkg.dll: 0x00000112
        * ASC_REQ_MUTUAL_AUTH
        * ASC_REQ_CONFIDENTIALITY
        * ASC_REQ_ALLOCATE_MEMORY
        */

        this->server_auth_data.state = ServerAuthenticateData::Loop;
    }

public:
    credssp::State authenticate_next(bytes_view in_data, OutStream & out_stream)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::authenticate_next");

        switch (this->server_auth_data.state)
        {
            case ServerAuthenticateData::Start:
              LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Start");
              return credssp::State::Err;
            case ServerAuthenticateData::Loop:
                LOG(LOG_INFO, "ServerAuthenticateData::Loop");
                if (Res::Err == this->authenticate_recv(in_data, out_stream)) {
                    LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                    return credssp::State::Err;
                }
                return credssp::State::Cont;
            case ServerAuthenticateData::Final:
               LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final");
               if (Res::Err == this->authenticate_final(in_data)) {
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
            this->public_key = make_writable_array_view(this->ServerClientHash);
        }
        else {
            // if we are server and protocol is 2,3,4
            // then echos the public key +1
            ::ap_integer_increment_le(this->public_key);
        }

        return this->table.EncryptMessage(
            public_key, this->ts_request.pubKeyAuth, this->send_seq_num++);
    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_public_key_echo");

        std::vector<uint8_t> Buffer;

        SEC_STATUS const status = this->table.DecryptMessage(
            this->ts_request.pubKeyAuth, Buffer, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            if (this->ts_request.pubKeyAuth.empty()) {
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
            this->public_key = make_writable_array_view(this->ClientServerHash);
        }

        writable_u8_array_view public_key2 {Buffer};

        if (public_key2.size() != public_key.size()) {
            LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", public_key2.size(), public_key.size());
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
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

    SEC_STATUS credssp_decrypt_ts_credentials() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_ts_credentials");

        if (this->ts_request.authInfo.empty()) {
            LOG(LOG_ERR, "credssp_decrypt_ts_credentials missing ts_request.authInfo buffer");
            return SEC_E_INVALID_TOKEN;
        }

        std::vector<uint8_t> decrypted_creds;

        const SEC_STATUS status = this->table.DecryptMessage(this->ts_request.authInfo, decrypted_creds, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            return status;
        }

        this->ts_credentials = recvTSCredentials(decrypted_creds, this->credssp_verbose);

        // hexdump(this->ts_credentials.passCreds.userName,
        //         this->ts_credentials.passCreds.userName_length);
        // hexdump(this->ts_credentials.passCreds.domainName,
        //         this->ts_credentials.passCreds.domainName_length);
        // hexdump(this->ts_credentials.passCreds.password,
        //         this->ts_credentials.passCreds.password_length);

        return SEC_E_OK;
    }

    Res authenticate_recv(bytes_view in_data, OutStream & out_stream)
    {
        LOG_IF(this->verbose, LOG_INFO,"rdpCredsspServer::authenticate_recv");

        if (this->state_accept_security_context != SEC_I_LOCAL_LOGON) {
            /* receive authentication token */
            this->ts_request = recvTSRequest(in_data, this->credssp_verbose);
            this->error_code = this->ts_request.error_code;
        }

        if (this->ts_request.negoTokens.empty()) {
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
        SEC_STATUS status = this->table.AcceptSecurityContext(
            this->ts_request.negoTokens,
            /*output*/this->ts_request.negoTokens);
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

            this->ts_request.negoTokens.clear();

            this->credssp_encrypt_public_key_echo();
        }

        if ((status != SEC_E_OK) && (status != SEC_I_CONTINUE_NEEDED)) {
            LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X", status);
            return Res::Err;
        }

        this->ts_request.error_code = 0;
        auto v = emitTSRequest(this->ts_request.version,
                               this->ts_request.negoTokens,
                               this->ts_request.authInfo,
                               this->ts_request.pubKeyAuth,
                               this->ts_request.error_code,
                               this->ts_request.clientNonce.clientNonce,
                               this->ts_request.clientNonce.initialized,
                               this->credssp_verbose);
        out_stream.out_copy_bytes(v);

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

    Res authenticate_final(bytes_view in_data)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::sm_credssp_server_authenticate_final");
        /* Receive encrypted credentials */
        this->ts_request = recvTSRequest(in_data, this->credssp_verbose);
        this->error_code = this->ts_request.error_code;

        SEC_STATUS status = this->credssp_decrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X", status);
            return Res::Err;
        }

        return Res::Ok;
    }

};
