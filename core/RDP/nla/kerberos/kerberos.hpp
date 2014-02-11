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

#ifndef _REDEMPTION_CORE_RDP_NLA_KERBEROS_KERBEROS_HPP_
#define _REDEMPTION_CORE_RDP_NLA_KERBEROS_KERBEROS_HPP_

#include <gssapi/gssapi.h>
#include "RDP/nla/sspi.hpp"
#include "RDP/nla/kerberos/credentials.hpp"

const char* KERBEROS_PACKAGE_NAME = "KERBEROS";
const char Kerberos_Name[] = "Kerberos";
const char Kerberos_Comment[] = "Kerberos Security Package";
const SecPkgInfo KERBEROS_SecPkgInfo = {
    0x00082B37,             // fCapabilities
    1,                      // wVersion
    0x000A,                 // wRPCID
    0x00000B48,             // cbMaxToken
    Kerberos_Name,          // Name
    Kerberos_Comment        // Comment
};
static gss_OID_desc _gss_spnego_krb5_mechanism_oid_desc =
	{ 9, (void *) "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02" };

struct KERBEROSContext {
    gss_ctx_id_t * gss_ctx;
    gss_name_t target_name;
    OM_uint32 actual_services;
    OM_uint32 actual_time;
    gss_OID actual_mech;

    KERBEROSContext()
        : gss_ctx(NULL)
    {
    }

    virtual ~KERBEROSContext() {
        OM_uint32 major_status, minor_status;
        if (this->gss_ctx) {
            gss_buffer_t output_token;
            output_token = GSS_C_NO_BUFFER;
            major_status = gss_delete_sec_context(&minor_status, this->gss_ctx,
                                                  output_token);
            (void) major_status;
            LOG(LOG_INFO, "DELETE KERBEROS CONTEXT");
        }
    }
};


struct Kerberos_SecurityFunctionTable : public SecurityFunctionTable {

    virtual ~Kerberos_SecurityFunctionTable() {}


    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    virtual SEC_STATUS QuerySecurityPackageInfo(const char* pszPackageName,
                                                SecPkgInfo * pPackageInfo) {

        // if (strcmp(pszPackageName, KERBEROS_SecPkgInfo.Name) == 0) {
        if (pPackageInfo) {
            pPackageInfo->fCapabilities = KERBEROS_SecPkgInfo.fCapabilities;
            pPackageInfo->wVersion = KERBEROS_SecPkgInfo.wVersion;
            pPackageInfo->wRPCID = KERBEROS_SecPkgInfo.wRPCID;
            pPackageInfo->cbMaxToken = KERBEROS_SecPkgInfo.cbMaxToken;
            pPackageInfo->Name = KERBEROS_SecPkgInfo.Name;
            pPackageInfo->Comment = KERBEROS_SecPkgInfo.Comment;

            return SEC_E_OK;
        }

        return SEC_E_SECPKG_NOT_FOUND;
    }

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    virtual SEC_STATUS QueryContextAttributes(PCtxtHandle phContext,
                                              unsigned long ulAttribute,
                                              void* pBuffer) {
        if (!pBuffer) {
            return SEC_E_INSUFFICIENT_MEMORY;
        }
        if (ulAttribute == SECPKG_ATTR_SIZES) {
            SecPkgContext_Sizes* ContextSizes = (SecPkgContext_Sizes*) pBuffer;
            ContextSizes->cbMaxToken = 2010;
            ContextSizes->cbMaxSignature = 0;
            ContextSizes->cbBlockSize = 0;
            ContextSizes->cbSecurityTrailer = 16;
            return SEC_E_OK;
        }

        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    virtual SEC_STATUS AcquireCredentialsHandle(const char * pszPrincipal,
                                                const char * pszPackage,
                                                unsigned long fCredentialUse,
                                                void* pvLogonID,
                                                void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
                                                void* pvGetKeyArgument,
                                                PCredHandle phCredential,
                                                TimeStamp * ptsExpiry) {

        if (pszPrincipal && pvLogonID) {
            Array * spn = (Array *) pvLogonID;
            const char * p = pszPrincipal;
            size_t length = 0;
            if (p) {
                length = strlen(p);
            }
            spn->init(length + 1);
            spn->copy((const uint8_t *)pszPrincipal, length);
            spn->get_data()[length] = 0;
        }
        Krb5Creds * credentials = new Krb5Creds;
        phCredential->SecureHandleSetLowerPointer((void*) credentials);
        phCredential->SecureHandleSetUpperPointer((void*) KERBEROS_PACKAGE_NAME);

        SEC_WINNT_AUTH_IDENTITY* identity = NULL;
        if (pAuthData != NULL) {
            identity = (SEC_WINNT_AUTH_IDENTITY*) pAuthData;
        }
        // set KRB5CCNAME cache name to specific with PID,
        // call kinit to get tgt with identity credentials

        int pid = getpid();
        char cache[256];
        snprintf(cache, 255, "FILE:/tmp/krb_red_%d", pid);
        setenv("KRB5CCNAME", cache, 1);
        if (identity) {
            int ret = credentials->get_credentials(identity->princname, identity->princpass, NULL);
            if (!ret) {
                return SEC_E_OK;
            }
        }
        return SEC_E_NO_CREDENTIALS;
    }

    SEC_STATUS FreeCredentialsHandle(PCredHandle phCredential) {

        if (!phCredential) {
            return SEC_E_INVALID_HANDLE;
        }
        Krb5Creds* credentials = (Krb5Creds*) phCredential->SecureHandleGetLowerPointer();
        if (!credentials) {
            return SEC_E_INVALID_HANDLE;
        }
        credentials->destroy_credentials(NULL);
        delete credentials;
        credentials = NULL;

        return SEC_E_OK;
    }

    bool get_service_name(char * server, gss_name_t * name) {
        gss_buffer_desc output;
        OM_uint32 major_status, minor_status;
        const char service_name[] = "TERMSRV";
        gss_OID type = (gss_OID) GSS_C_NT_HOSTBASED_SERVICE;
        int size = (strlen(service_name) + 1 + strlen(server) + 1);

        output.value = malloc(size);
        snprintf((char*)output.value, size, "%s@%s", service_name, server);
        output.length = strlen((const char*)output.value) + 1;
        LOG(LOG_INFO, "GSS IMPORT NAME : %s", output.value);
        major_status = gss_import_name(&minor_status, &output, type, name);

        if (GSS_ERROR(major_status)) {
            LOG(LOG_ERR, "Failed to create service principal name");
            return false;
        }

        gss_release_buffer(&minor_status, &output);

        return true;
    }


    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    virtual SEC_STATUS InitializeSecurityContext(PCredHandle phCredential,
                                                 PCtxtHandle phContext,
                                                 char* pszTargetName,
                                                 unsigned long fContextReq,
                                                 unsigned long Reserved1,
                                                 unsigned long TargetDataRep,
                                                 SecBufferDesc * pInput,
                                                 unsigned long Reserved2,
                                                 PCtxtHandle phNewContext,
                                                 SecBufferDesc * pOutput,
                                                 unsigned long * pfContextAttr,
                                                 TimeStamp * ptsExpiry) {


        OM_uint32 major_status, minor_status;

        gss_cred_id_t gss_no_cred = GSS_C_NO_CREDENTIAL;
        gss_ctx_id_t gss_no_ctx = GSS_C_NO_CONTEXT;
        KERBEROSContext * krb_ctx = NULL;
        if (phContext) {
            krb_ctx = (KERBEROSContext*) phContext->SecureHandleGetLowerPointer();
        }
        if (!krb_ctx) {
            LOG(LOG_INFO, "Initialiaze Sec CTX: NO CONTEXT");
            krb_ctx = new KERBEROSContext;
            if (!krb_ctx) {
                return SEC_E_INSUFFICIENT_MEMORY;
            }
            krb_ctx->gss_ctx = &gss_no_ctx;

            phNewContext->SecureHandleSetLowerPointer(krb_ctx);
            phNewContext->SecureHandleSetUpperPointer((void*) KERBEROS_PACKAGE_NAME);

            // Target name (server name, ip ...)
            if (!this->get_service_name(pszTargetName, &krb_ctx->target_name)) {
                return SEC_E_INVALID_TOKEN;
            }
        }
        else {
            LOG(LOG_INFO, "Initialiaze Sec CTX: USE FORMER CONTEXT");
        }


        // Token Buffer
	gss_buffer_desc input_tok, output_tok;
        output_tok.length = 0;
        if (pInput) {
            PSecBuffer input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);
            if (input_buffer) {
                LOG(LOG_INFO, "GOT INPUT BUFFER: length %d", input_buffer->Buffer.size());
                input_tok.length = input_buffer->Buffer.size();
                input_tok.value = input_buffer->Buffer.get_data();
                hexdump_c((uint8_t *)input_tok.value, input_tok.length);
            }
            else {
                LOG(LOG_INFO, "NO INPUT BUFFER TOKEN");
                input_tok.length = 0;
            }
        }
        else {
            LOG(LOG_INFO, "NO INPUT BUFFER DESC");
            input_tok.length = 0;
        }

        gss_OID desired_mech = &_gss_spnego_krb5_mechanism_oid_desc;
        if (!this->mech_available(desired_mech)) {
            LOG(LOG_ERR, "Desired Mech unavailable");
            return SEC_E_INVALID_TOKEN;
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
                                            krb_ctx->gss_ctx,
                                            krb_ctx->target_name,
                                            desired_mech,
                                            GSS_C_MUTUAL_FLAG | GSS_C_DELEG_FLAG,
                                            GSS_C_INDEFINITE,
                                            GSS_C_NO_CHANNEL_BINDINGS,
                                            &input_tok,
                                            &krb_ctx->actual_mech,
                                            &output_tok,
                                            &krb_ctx->actual_services,
                                            &krb_ctx->actual_time);

        if (GSS_ERROR(major_status)) {
            LOG(LOG_INFO, "MAJOR ERROR");
            this->report_error(GSS_C_GSS_CODE, "CredSSP: SPNEGO negotiation failed.",
                               major_status, minor_status);
            return SEC_E_INVALID_TOKEN;
        }

        PSecBuffer output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

        LOG(LOG_INFO, "output tok length : %d", output_tok.length);
        if (output_tok.length < 1) {
            output_buffer->Buffer.init(0);
        }
        else {
            output_buffer->Buffer.init(output_tok.length);
            output_buffer->Buffer.copy((const uint8_t*)output_tok.value, output_tok.length);
        }

        (void) gss_release_buffer(&minor_status, &output_tok);
        (void) gss_release_buffer(&minor_status, &input_tok);

        // phNewContext->SecureHandleSetLowerPointer(gss_ctx);
        // phNewContext->SecureHandleSetUpperPointer((void*) KERBEROS_PACKAGE_NAME);

        if (GSS_ERROR(major_status)) {
            LOG(LOG_INFO, "MAJOR ERROR");
            this->report_error(GSS_C_GSS_CODE, "CredSSP: SPNEGO negotiation failed.",
                               major_status, minor_status);
            return SEC_E_INVALID_TOKEN;
        }
        if (major_status & GSS_S_CONTINUE_NEEDED) {
            LOG(LOG_INFO, "MAJOR CONTINUE NEEDED");
            return SEC_I_CONTINUE_NEEDED;
        }
        LOG(LOG_INFO, "MAJOR COMPLETE NEEDED");
        return SEC_I_COMPLETE_NEEDED;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    virtual SEC_STATUS AcceptSecurityContext(PCredHandle phCredential,
                                             PCtxtHandle phContext,
                                             SecBufferDesc * pInput,
                                             unsigned long fContextReq,
                                             unsigned long TargetDataRep,
                                             PCtxtHandle phNewContext,
                                             SecBufferDesc * pOutput,
                                             unsigned long * pfContextAttr,
                                             TimeStamp * ptsTimeStamp) {
        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    virtual SEC_STATUS FreeContextBuffer(void* pvContextBuffer) {
        KERBEROSContext* toDelete = (KERBEROSContext*)((PCtxtHandle)pvContextBuffer)->SecureHandleGetLowerPointer();
        if (toDelete) {
            delete toDelete;
            toDelete = NULL;
        }
        return SEC_E_OK;
    }

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    virtual SEC_STATUS EncryptMessage(PCtxtHandle phContext, unsigned long fQOP,
                                      PSecBufferDesc pMessage, unsigned long MessageSeqNo) {
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
        KERBEROSContext* context = NULL;

        if (phContext) {
            context = (KERBEROSContext*) phContext->SecureHandleGetLowerPointer();
        }
        if (!context) {
            return SEC_E_NO_CONTEXT;
        }
        PSecBuffer data_buffer = NULL;
	gss_buffer_desc inbuf, outbuf;
        for (int index = 0; index < (int) pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA) {
                data_buffer = &pMessage->pBuffers[index];
            }
        }
        if (data_buffer) {
            inbuf.value = data_buffer->Buffer.get_data();
            inbuf.length = data_buffer->Buffer.size();
        }
        else {
            return SEC_E_INVALID_TOKEN;
        }
	major_status = gss_wrap(&minor_status, *context->gss_ctx, true,
				GSS_C_QOP_DEFAULT, &inbuf, &conf_state, &outbuf);
        (void) major_status;
        // TODO check status
        data_buffer->Buffer.copy((const uint8_t *)outbuf.value, outbuf.length);
	gss_release_buffer(&minor_status, &outbuf);

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    virtual SEC_STATUS DecryptMessage(PCtxtHandle phContext, PSecBufferDesc pMessage,
                                      unsigned long MessageSeqNo, unsigned long * pfQOP) {


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
        KERBEROSContext* context = NULL;

        if (phContext) {
            context = (KERBEROSContext*) phContext->SecureHandleGetLowerPointer();
        }
        if (!context) {
            return SEC_E_NO_CONTEXT;
        }
        PSecBuffer data_buffer = NULL;
	gss_buffer_desc inbuf, outbuf;
        for (int index = 0; index < (int) pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA) {
                data_buffer = &pMessage->pBuffers[index];
            }
        }
        if (data_buffer) {
            inbuf.value = data_buffer->Buffer.get_data();
            inbuf.length = data_buffer->Buffer.size();
        }
        else {
            return SEC_E_INVALID_TOKEN;
        }
	major_status = gss_unwrap(&minor_status, *context->gss_ctx, &inbuf, &outbuf,
                                  &conf_state, &qop_state);
        (void) major_status;
        // TODO Check Status !!!
        data_buffer->Buffer.copy((const uint8_t *)outbuf.value, outbuf.length);
	gss_release_buffer(&minor_status, &outbuf);

        return SEC_E_OK;
    }

    // IMPERSONATE_SECURITY_CONTEXT ImpersonateSecurityContext;
    virtual SEC_STATUS ImpersonateSecurityContext(PCtxtHandle phContext) {
        return SEC_E_OK;
    }
    // REVERT_SECURITY_CONTEXT RevertSecurityContext;
    virtual SEC_STATUS RevertSecurityContext(PCtxtHandle phContext) {
        return SEC_E_OK;
    }

    void report_error(OM_uint32 code, const char *str,
                      OM_uint32 major_status, OM_uint32 minor_status)
    {
        OM_uint32 msgctx = 0, ms;
        gss_buffer_desc status_string;

        LOG(LOG_ERR, "GSS error [%d:%d:%d]: %s\n",
            (major_status & 0xff000000) >> 24,	// Calling error
            (major_status & 0xff0000) >> 16,	// Routine error
            major_status & 0xffff,	// Supplementary info bits
            str);

        LOG(LOG_ERR, "GSS Minor status error [%d:%d:%d]:%d %s\n",
            (minor_status & 0xff000000) >> 24,	// Calling error
            (minor_status & 0xff0000) >> 16,	// Routine error
            minor_status & 0xffff,	// Supplementary info bits
            minor_status,
            str);

        do {
                ms = gss_display_status(&minor_status, major_status,
                                        code, GSS_C_NULL_OID, &msgctx, &status_string);
        	if (ms != GSS_S_COMPLETE)
                    continue;

                LOG(LOG_ERR," - %s\n", status_string.value);
        }
        while (ms == GSS_S_COMPLETE && msgctx);

    }

    bool mech_available(gss_OID mech)
    {
        int mech_found;
        OM_uint32 major_status, minor_status;
        gss_OID_set mech_set;

        mech_found = 0;

        if (mech == GSS_C_NO_OID)
            return true;

        major_status = gss_indicate_mechs(&minor_status, &mech_set);
        if (!mech_set)
            return false;

        if (GSS_ERROR(major_status)) {
            this->report_error(GSS_C_GSS_CODE, "Failed to get available mechs on system",
                               major_status, minor_status);
            return false;
        }

        gss_test_oid_set_member(&minor_status, mech, mech_set, &mech_found);

        if (GSS_ERROR(major_status)) {
            this->report_error(GSS_C_GSS_CODE, "Failed to match mechanism in set",
                                  major_status, minor_status);
            return false;
        }

        if (!mech_found)
            return false;

        return true;
    }

};

#endif
