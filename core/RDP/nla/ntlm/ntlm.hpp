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

#ifndef _REDEMPTION_CORE_RDP_NLA_NTLM_NTLM_HPP_
#define _REDEMPTION_CORE_RDP_NLA_NTLM_NTLM_HPP_

#include "RDP/nla/sspi.hpp"
#include "RDP/nla/ntlm/ntlm_context.hpp"

const char* NTLM_PACKAGE_NAME = "NTLM";

struct Ntlm_SecurityFunctionTable : public SecurityFunctionTable {

    virtual ~Ntlm_SecurityFunctionTable() {}

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    virtual SEC_STATUS AcquireCredentialsHandle(char * pszPrincipal, char * pszPackage,
                                                unsigned long fCredentialUse, void* pvLogonID,
                                                void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
                                                void* pvGetKeyArgument, PCredHandle phCredential,
                                                TimeStamp * ptsExpiry) {

	CREDENTIALS* credentials;
	SEC_WINNT_AUTH_IDENTITY* identity;

	if (fCredentialUse == SECPKG_CRED_OUTBOUND) {
            credentials = new CREDENTIALS;

            identity = (SEC_WINNT_AUTH_IDENTITY*) pAuthData;

            if (identity != NULL)
                memcpy(&(credentials->identity), identity, sizeof(SEC_WINNT_AUTH_IDENTITY));

            phCredential->SecureHandleSetLowerPointer((void*) credentials);
            phCredential->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);

            return SEC_E_OK;
        }
	else if (fCredentialUse == SECPKG_CRED_INBOUND) {
            credentials = new CREDENTIALS;

            identity = (SEC_WINNT_AUTH_IDENTITY*) pAuthData;

            if (identity)
                memcpy(&(credentials->identity), identity, sizeof(SEC_WINNT_AUTH_IDENTITY));
            else
                memset(&(credentials->identity), 0x00, sizeof(SEC_WINNT_AUTH_IDENTITY));

            phCredential->SecureHandleSetLowerPointer((void*) credentials);
            phCredential->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);

            return SEC_E_OK;
        }

	return SEC_E_OK;
    }

    SEC_STATUS FreeCredentialsHandle(PCredHandle phCredential) {
	CREDENTIALS* credentials;

	if (!phCredential)
            return SEC_E_INVALID_HANDLE;

	credentials = (CREDENTIALS*) phCredential->SecureHandleGetLowerPointer();

	if (!credentials)
            return SEC_E_INVALID_HANDLE;

	delete credentials;

	return SEC_E_OK;
    }

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    virtual SEC_STATUS InitializeSecurityContext(PCredHandle phCredential, PCtxtHandle phContext,
                                                 char* pszTargetName, unsigned long fContextReq,
                                                 unsigned long Reserved1,
                                                 unsigned long TargetDataRep,
                                                 SecBufferDesc * pInput, unsigned long Reserved2,
                                                 PCtxtHandle phNewContext, SecBufferDesc * pOutput,
                                                 unsigned long * pfContextAttr,
                                                 TimeStamp * ptsExpiry) {
	NTLMContext* context;
	// SEC_STATUS status;
	CREDENTIALS* credentials;
	PSecBuffer input_buffer = NULL;
	PSecBuffer output_buffer = NULL;
	// PSecBuffer channel_bindings = NULL;

	context = (NTLMContext*) phContext->SecureHandleGetLowerPointer();

	if (!context) {
            context = new NTLMContext;
            context->init();
            context->server = false;

            if (!context)
                return SEC_E_INSUFFICIENT_MEMORY;

            if (fContextReq & ISC_REQ_CONFIDENTIALITY)
                context->confidentiality = 1;

            credentials = (CREDENTIALS*) phCredential->SecureHandleGetLowerPointer();

            // if (context->Workstation.size() < 1)
            //     context->ntlm_SetContextWorkstation(NULL);

            context->ntlm_SetContextServicePrincipalName(pszTargetName);
            context->identity.CopyAuthIdentity(credentials->identity);

            phNewContext->SecureHandleSetLowerPointer(context);
            phNewContext->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);
        }

	if ((!pInput) || (context->state == NTLM_STATE_AUTHENTICATE)) {
            if (!pOutput)
                return SEC_E_INVALID_TOKEN;

            if (pOutput->cBuffers < 1)
                return SEC_E_INVALID_TOKEN;

            output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!output_buffer)
                return SEC_E_INVALID_TOKEN;

            if (output_buffer->Buffer.size() < 1)
                return SEC_E_INVALID_TOKEN;

            if (context->state == NTLM_STATE_INITIAL)
                context->state = NTLM_STATE_NEGOTIATE;

            if (context->state == NTLM_STATE_NEGOTIATE) {
                context->ntlm_client_build_negotiate();
                BStream out_stream;
                context->NEGOTIATE_MESSAGE.emit(out_stream);
                output_buffer->Buffer.init(out_stream.size());
                // out_stream.rewind();
                // out_stream.in_copy_bytes(output_buffer.Buffer.get_data(), out_stream.size());
                memcpy(output_buffer->Buffer.get_data(), out_stream.get_data(), out_stream.size());
                //context->ntlm_write_NegotiateMessage(output_buffer);
            }
            return SEC_E_OUT_OF_SEQUENCE;
        }
	else {
            if (pInput->cBuffers < 1)
                return SEC_E_INVALID_TOKEN;

            input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer)
                return SEC_E_INVALID_TOKEN;

            if (input_buffer->Buffer.size() < 1)
                return SEC_E_INVALID_TOKEN;

            // channel_bindings = sspi_FindSecBuffer(pInput, SECBUFFER_CHANNEL_BINDINGS);

            // if (channel_bindings) {
            //     context->Bindings.BindingsLength = channel_bindings->cbBuffer;
            //     context->Bindings.Bindings = (SEC_CHANNEL_BINDINGS*) channel_bindings->pvBuffer;
            // }

            if (context->state == NTLM_STATE_CHALLENGE) {
                BStream input_stream;
                input_stream.out_copy_bytes(input_buffer->Buffer.get_data(),
                                            input_buffer->Buffer.size());
                input_stream.mark_end();
                input_stream.rewind();
                context->CHALLENGE_MESSAGE.recv(input_stream);

                // status = ntlm_read_ChallengeMessage(context, input_buffer);

                if (!pOutput)
                    return SEC_E_INVALID_TOKEN;

                if (pOutput->cBuffers < 1)
                    return SEC_E_INVALID_TOKEN;

                // output_buffer = sspi_FindSecBuffer(pOutput, SECBUFFER_TOKEN);

                if (!output_buffer)
                    return SEC_E_INVALID_TOKEN;

                if (output_buffer->Buffer.size() < 1)
                    return SEC_E_INSUFFICIENT_MEMORY;

                if (context->state == NTLM_STATE_AUTHENTICATE) {
                    SEC_WINNT_AUTH_IDENTITY & id = context->identity;
                    context->ntlm_client_build_authenticate(id.Password.get_data(),
                                                            id.Password.size(),
                                                            id.User.get_data(),
                                                            id.User.size(),
                                                            id.Domain.get_data(),
                                                            id.Domain.size(),
                                                            context->Workstation.get_data(),
                                                            context->Workstation.size());
                    BStream out_stream;
                    context->AUTHENTICATE_MESSAGE.emit(out_stream);
                    output_buffer->Buffer.init(out_stream.size());
                    // out_stream.rewind();
                    // out_stream.in_copy_bytes(output_buffer.Buffer.get_data(),
                    //                          out_stream.size());
                    memcpy(output_buffer->Buffer.get_data(),
                           out_stream.get_data(), out_stream.size());

                    // return ntlm_write_AuthenticateMessage(context, output_buffer);
                }
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

	return SEC_E_OUT_OF_SEQUENCE;
    }
};


#endif
