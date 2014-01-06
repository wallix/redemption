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
    virtual SEC_STATUS AcquireCredentialsHandle(const char * pszPrincipal, const char * pszPackage,
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
                return SEC_I_CONTINUE_NEEDED;
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
                context->state = NTLM_STATE_AUTHENTICATE;

                if (!pOutput)
                    return SEC_E_INVALID_TOKEN;

                if (pOutput->cBuffers < 1)
                    return SEC_E_INVALID_TOKEN;

                output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

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
                    return SEC_I_COMPLETE_NEEDED;
                }
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

	return SEC_E_OUT_OF_SEQUENCE;
    }

    virtual SEC_STATUS FreeContextBuffer(void* pvContextBuffer) {
        NTLMContext * toDelete = (NTLMContext*) ((PSecHandle)pvContextBuffer)->SecureHandleGetLowerPointer();
        if (!toDelete) {
            delete toDelete;
        }
        return SEC_E_OK;
    }

    virtual SEC_STATUS EncryptMessage(PCtxtHandle phContext, unsigned long fQOP,
                                      PSecBufferDesc pMessage, unsigned long MessageSeqNo) {
        int index;
        int length;
        uint8_t* data;
        uint32_t SeqNo;
        uint8_t digest[16];
        uint8_t checksum[8];
        uint8_t* signature;
        unsigned long version = 1;
        NTLMContext* context;
        PSecBuffer data_buffer = NULL;
        PSecBuffer signature_buffer = NULL;

        SeqNo = MessageSeqNo;
        context = (NTLMContext*) phContext->SecureHandleGetLowerPointer();

        for (index = 0; index < (int) pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA)
                data_buffer = &pMessage->pBuffers[index];
            else if (pMessage->pBuffers[index].BufferType == SECBUFFER_TOKEN)
                signature_buffer = &pMessage->pBuffers[index];
        }

        if (!data_buffer)
            return SEC_E_INVALID_TOKEN;

        if (!signature_buffer)
            return SEC_E_INVALID_TOKEN;

        /* Copy original data buffer */
        length = data_buffer->Buffer.size();
        data = new uint8_t[length];
        memcpy(data, data_buffer->Buffer.get_data(), length);

        /* Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key */
//         HMAC_CTX_init(&hmac);

        SslHMAC_Md5 hmac_md5(context->SendSigningKey, 16);
        hmac_md5.update((uint8_t*) &(SeqNo), 4);
        hmac_md5.update(data, 4);
        hmac_md5.final(digest, sizeof(digest));
        // HMAC_Init_ex(&hmac, context->SendSigningKey, 16, EVP_md5(), NULL);
        // HMAC_Update(&hmac, (void*) &(SeqNo), 4);
        // HMAC_Update(&hmac, data, length);
        // HMAC_Final(&hmac, digest, NULL);
        // HMAC_CTX_cleanup(&hmac);

        /* Encrypt message using with RC4, result overwrites original buffer */

        if (context->confidentiality) {
            context->SendRc4Seal.crypt(length, data, data_buffer->Buffer.get_data());
            // RC4(&context->SendRc4Seal, length, data, data_buffer->pvBuffer);
        }
        else {
            memcpy(data_buffer->Buffer.get_data(), data, length);
        }
// #ifdef WITH_DEBUG_NTLM
//         LOG(LOG_ERR, "Data Buffer (length = %d)\n", length);
//         hexdump_c(data, length);
//         LOG(LOG_ERR, "\n");

//         LOG(LOG_ERR, "Encrypted Data Buffer (length = %d)\n", (int) data_buffer->cbBuffer);
//         hexdump_c(data_buffer->pvBuffer, data_buffer->cbBuffer);
//         LOG(LOG_ERR, "\n");
// #endif

        delete [] data;
        // free(data);

        /* RC4-encrypt first 8 bytes of digest */
        context->SendRc4Seal.crypt(8, digest, checksum);

        signature = signature_buffer->Buffer.get_data();

        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(signature, (void*) & version, 4);
        memcpy(&signature[4], (void*) checksum, 8);
        memcpy(&signature[12], (void*) &(SeqNo), 4);
        context->SendSeqNum++;

// #ifdef WITH_DEBUG_NTLM
//         LOG(LOG_ERR, "Signature (length = %d)\n", (int) signature_buffer->cbBuffer);
//         hexdump_c(signature_buffer->pvBuffer, signature_buffer->cbBuffer);
//         LOG(LOG_ERR, "\n");
// #endif

        return SEC_E_OK;
    }
};


#endif
