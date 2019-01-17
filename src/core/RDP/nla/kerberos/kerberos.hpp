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

#include <gssapi/gssapi.h>
#include "core/RDP/nla/sspi.hpp"
#include "core/RDP/nla/kerberos/credentials.hpp"
#include "cxx/diagnostic.hpp"
#include <cassert>

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif

//const char* KERBEROS_PACKAGE_NAME = "KERBEROS";
// const char Kerberos_Name[] = "Kerberos";
// const char Kerberos_Comment[] = "Kerberos Security Package";
// const SecPkgInfo KERBEROS_SecPkgInfo = {
//     0x00082B37,             // fCapabilities
//     1,                      // wVersion
//     0x000A,                 // wRPCID
//     0x00000B48,             // cbMaxToken
//     Kerberos_Name,          // Name
//     Kerberos_Comment        // Comment
// };


inline gss_OID_desc _gss_spnego_krb5_mechanism_oid_desc()
{
    return { 9, const_cast<void *>(static_cast<const void *>("\x2a\x86\x48\x86\xf7\x12\x01\x02\x02")) }; /*NOLINT*/
}

// SecPkgContext_Sizes ContextSizes;
// ContextSizes.cbMaxToken = 4096;
// ContextSizes.cbMaxSignature = 0;
// ContextSizes.cbBlockSize = 0;
// ContextSizes.cbSecurityTrailer = 16;



struct KERBEROSContext final {
    gss_ctx_id_t gss_ctx;
    gss_name_t target_name;
    OM_uint32 actual_services;
    OM_uint32 actual_time;
    OM_uint32 actual_flag;
    gss_OID actual_mech;
    gss_cred_id_t deleg_cred;
    KERBEROSContext()
        : gss_ctx(GSS_C_NO_CONTEXT)
        , target_name(GSS_C_NO_NAME)
        , deleg_cred(GSS_C_NO_CREDENTIAL)
    {}

    ~KERBEROSContext() {
        OM_uint32 major_status, minor_status;
        if (this->target_name != GSS_C_NO_NAME) {
            major_status = gss_release_name(&minor_status, &this->target_name);
            (void) major_status;
            this->target_name = GSS_C_NO_NAME;
        }
        if (this->gss_ctx != GSS_C_NO_CONTEXT) {
            major_status = gss_delete_sec_context(&minor_status, &this->gss_ctx,
                                                  GSS_C_NO_BUFFER);
            (void) major_status;
            this->gss_ctx = GSS_C_NO_CONTEXT;
        }
        if (this->deleg_cred != GSS_C_NO_CREDENTIAL) {
            major_status = gss_release_cred(&minor_status, &this->deleg_cred);
            (void) major_status;
            this->deleg_cred = GSS_C_NO_CREDENTIAL;
        }
    }
};


struct Kerberos_SecurityFunctionTable : public SecurityFunctionTable
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
        const char * pszPrincipal, Array * pvLogonID, SEC_WINNT_AUTH_IDENTITY const* pAuthData
    ) override {
        if (pszPrincipal && pvLogonID) {
            size_t length = strlen(pszPrincipal);
            pvLogonID->init(length + 1);
            pvLogonID->copy({pszPrincipal, length});
            pvLogonID->get_data()[length] = 0;
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
            int ret = this->credentials->get_credentials(pAuthData->princname,
                                                         pAuthData->princpass, nullptr);
            if (!ret) {
                return SEC_E_OK;
            }
        }
        return SEC_E_NO_CREDENTIALS;
    }

    bool get_service_name(array_view_const_char server, gss_name_t * name) {
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
    SEC_STATUS InitializeSecurityContext(
        array_view_const_char pszTargetName, array_view_const_u8 input_buffer, Array& output_buffer
    ) override
    {
        OM_uint32 major_status, minor_status;

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
        gss_buffer_desc input_tok, output_tok;
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
        major_status = gss_init_sec_context(&minor_status,
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
        output_buffer.init(output_tok.length);
        output_buffer.copy({static_cast<uint8_t const*>(output_tok.value), output_tok.length});

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
        array_view_const_u8 input_buffer, Array& output_buffer
    ) override
    {
        OM_uint32 major_status, minor_status;

        gss_cred_id_t gss_no_cred = GSS_C_NO_CREDENTIAL;
        if (!this->krb_ctx) {
            // LOG(LOG_INFO, "Initialiaze Sec Ctx: NO CONTEXT");
            this->krb_ctx = std::make_unique<KERBEROSContext>();
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

        major_status = gss_accept_sec_context(&minor_status,
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
        output_buffer.init(output_tok.length);
        output_buffer.copy({static_cast<const uint8_t*>(output_tok.value), output_tok.length});

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
    SEC_STATUS EncryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo) override {
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
        if (!this->krb_ctx) {
            return SEC_E_NO_CONTEXT;
        }
        gss_buffer_desc inbuf, outbuf;

        inbuf.value = const_cast<uint8_t*>(data_in.data()); /*NOLINT*/
        inbuf.length = data_in.size();
        // LOG(LOG_INFO, "GSS_WRAP inbuf length : %d", inbuf.length);
        major_status = gss_wrap(&minor_status, this->krb_ctx->gss_ctx, true,
				GSS_C_QOP_DEFAULT, &inbuf, &conf_state, &outbuf);
        if (GSS_ERROR(major_status)) {
            LOG(LOG_INFO, "MAJOR ERROR");
            this->report_error(GSS_C_GSS_CODE, "CredSSP: GSS WRAP failed.",
                               major_status, minor_status);
            return SEC_E_ENCRYPT_FAILURE;
        }
        // LOG(LOG_INFO, "GSS_WRAP outbuf length : %d", outbuf.length);
        data_out.init(outbuf.length);
        data_out.copy({static_cast<uint8_t const*>(outbuf.value), outbuf.length});
        gss_release_buffer(&minor_status, &outbuf);

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    SEC_STATUS DecryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo) override {
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
        gss_buffer_desc inbuf, outbuf;
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
        data_out.init(outbuf.length);
        data_out.copy({static_cast<uint8_t const*>(outbuf.value), outbuf.length});
        gss_release_buffer(&minor_status, &outbuf);
        return SEC_E_OK;
    }

    void report_error(OM_uint32 code, const char *str,
                      OM_uint32 major_status, OM_uint32 minor_status)
    {
        OM_uint32 msgctx = 0, ms;
        gss_buffer_desc status_string;

        LOG(LOG_ERR, "GSS error [%u:%u:%u]: %s\n",
            (major_status & 0xff000000) >> 24,	// Calling error
            (major_status & 0xff0000) >> 16,	// Routine error
            major_status & 0xffff,	// Supplementary info bits
            str);

        LOG(LOG_ERR, "GSS Minor status error [%u:%u:%u]:%u %s\n",
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

            LOG(LOG_ERR," - %s\n", static_cast<char const*>(status_string.value));
        }
        while (ms == GSS_S_COMPLETE && msgctx);

    }

    bool mech_available(gss_OID mech)
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

};

REDEMPTION_DIAGNOSTIC_POP
