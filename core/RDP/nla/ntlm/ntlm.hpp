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
const char Ntlm_Name[] = "NTLM";
const char Ntlm_Comment[] = "NTLM Security Package";
const SecPkgInfo NTLM_SecPkgInfo = {
    0x00082B37,             // fCapabilities
    1,                      // wVersion
    0x000A,                 // wRPCID
    0x00000B48,             // cbMaxToken
    Ntlm_Name,              // Name
    Ntlm_Comment            // Comment
};

struct Ntlm_SecurityFunctionTable : public SecurityFunctionTable {

    virtual ~Ntlm_SecurityFunctionTable() {}


    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    virtual SEC_STATUS QuerySecurityPackageInfo(const char* pszPackageName,
                                                SecPkgInfo * pPackageInfo) {

        if (strcmp(pszPackageName, NTLM_SecPkgInfo.Name) == 0) {

            pPackageInfo->fCapabilities = NTLM_SecPkgInfo.fCapabilities;
            pPackageInfo->wVersion = NTLM_SecPkgInfo.wVersion;
            pPackageInfo->wRPCID = NTLM_SecPkgInfo.wRPCID;
            pPackageInfo->cbMaxToken = NTLM_SecPkgInfo.cbMaxToken;
            pPackageInfo->Name = NTLM_SecPkgInfo.Name;
            pPackageInfo->Comment = NTLM_SecPkgInfo.Comment;

            return SEC_E_OK;
        }

        return SEC_E_SECPKG_NOT_FOUND;
    }

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    virtual SEC_STATUS QueryContextAttributes(PCtxtHandle phContext, unsigned long ulAttribute,
                                              void* pBuffer) {
        if (!pBuffer) {
            return SEC_E_INSUFFICIENT_MEMORY;
        }
        if (ulAttribute == SECPKG_ATTR_SIZES) {
            SecPkgContext_Sizes* ContextSizes = (SecPkgContext_Sizes*) pBuffer;
            ContextSizes->cbMaxToken = 2010;
            ContextSizes->cbMaxSignature = 16;
            ContextSizes->cbBlockSize = 0;
            ContextSizes->cbSecurityTrailer = 16;
            return SEC_E_OK;
        }

        return SEC_E_UNSUPPORTED_FUNCTION;
    }

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    virtual SEC_STATUS AcquireCredentialsHandle(const char * pszPrincipal, const char * pszPackage,
                                                unsigned long fCredentialUse, void* pvLogonID,
                                                void* pAuthData, SEC_GET_KEY_FN pGetKeyFn,
                                                void* pvGetKeyArgument, PCredHandle phCredential,
                                                TimeStamp * ptsExpiry) {

        CREDENTIALS* credentials = NULL;
        SEC_WINNT_AUTH_IDENTITY* identity = NULL;

        if (fCredentialUse == SECPKG_CRED_OUTBOUND) {
            credentials = new CREDENTIALS;

            identity = (SEC_WINNT_AUTH_IDENTITY*) pAuthData;

            if (identity != NULL) {
                credentials->identity.CopyAuthIdentity(*identity);
            }

            phCredential->SecureHandleSetLowerPointer((void*) credentials);
            phCredential->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);

            return SEC_E_OK;
        }
        else if (fCredentialUse == SECPKG_CRED_INBOUND) {
            credentials = new CREDENTIALS;

            identity = (SEC_WINNT_AUTH_IDENTITY*) pAuthData;

            if (identity) {
                credentials->identity.CopyAuthIdentity(*identity);
            }
            else {
                credentials->identity.clear();
            }
            phCredential->SecureHandleSetLowerPointer((void*) credentials);
            phCredential->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);

            return SEC_E_OK;
        }

        return SEC_E_OK;
    }

    SEC_STATUS FreeCredentialsHandle(PCredHandle phCredential) {
        CREDENTIALS* credentials;

        if (!phCredential) {
            return SEC_E_INVALID_HANDLE;
        }
        credentials = (CREDENTIALS*) phCredential->SecureHandleGetLowerPointer();

        if (!credentials) {
            return SEC_E_INVALID_HANDLE;
        }
        delete credentials;
        credentials = NULL;

        return SEC_E_OK;
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
        NTLMContext* context = NULL;
        CREDENTIALS* credentials = NULL;
        PSecBuffer input_buffer = NULL;
        PSecBuffer output_buffer = NULL;

        if (phContext) {
            context = (NTLMContext*) phContext->SecureHandleGetLowerPointer();
        }

        if (!context) {
            context = new NTLMContext;

            if (!context) {
                return SEC_E_INSUFFICIENT_MEMORY;
            }

            // context->init();
            context->server = false;
            if (Reserved1 == 1) {
                context->set_tests();
            }
            if (fContextReq & ISC_REQ_CONFIDENTIALITY) {
                context->confidentiality = true;
            }
            credentials = (CREDENTIALS*) phCredential->SecureHandleGetLowerPointer();

            // if (context->Workstation.size() < 1)
            //     context->ntlm_SetContextWorkstation(NULL);

            context->ntlm_SetContextWorkstation((uint8_t*)pszTargetName);
            context->ntlm_SetContextServicePrincipalName((uint8_t*)pszTargetName);

            context->identity.CopyAuthIdentity(credentials->identity);

            phNewContext->SecureHandleSetLowerPointer(context);
            phNewContext->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);
        }

        if ((!pInput) || (context->state == NTLM_STATE_AUTHENTICATE)) {
            if (!pOutput) {
                return SEC_E_INVALID_TOKEN;
            }
            if (pOutput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);
            if (!output_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (output_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            if (context->state == NTLM_STATE_INITIAL) {
                context->state = NTLM_STATE_NEGOTIATE;
            }
            if (context->state == NTLM_STATE_NEGOTIATE) {
                return context->write_negotiate(output_buffer);
            }
            return SEC_E_OUT_OF_SEQUENCE;
        }
        else {
            if (pInput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (input_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            // channel_bindings = sspi_FindSecBuffer(pInput, SECBUFFER_CHANNEL_BINDINGS);

            // if (channel_bindings) {
            //     context->Bindings.BindingsLength = channel_bindings->cbBuffer;
            //     context->Bindings.Bindings = (SEC_CHANNEL_BINDINGS*) channel_bindings->pvBuffer;
            // }

            if (context->state == NTLM_STATE_CHALLENGE) {
                context->read_challenge(input_buffer);

                if (!pOutput) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (pOutput->cBuffers < 1) {
                    return SEC_E_INVALID_TOKEN;
                }
                output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

                if (!output_buffer) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (output_buffer->Buffer.size() < 1) {
                    return SEC_E_INSUFFICIENT_MEMORY;
                }
                if (context->state == NTLM_STATE_AUTHENTICATE) {
                    return context->write_authenticate(output_buffer);
                }
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    virtual SEC_STATUS AcceptSecurityContext(PCredHandle phCredential, PCtxtHandle phContext,
                                             SecBufferDesc * pInput, unsigned long fContextReq,
                                             unsigned long TargetDataRep, PCtxtHandle phNewContext,
                                             SecBufferDesc * pOutput,
                                             unsigned long * pfContextAttr,
                                             TimeStamp * ptsTimeStamp) {
        NTLMContext* context = NULL;
        SEC_STATUS status;
        CREDENTIALS* credentials = NULL;
        PSecBuffer input_buffer = NULL;
        PSecBuffer output_buffer = NULL;

        if (phContext) {
            context = (NTLMContext*) phContext->SecureHandleGetLowerPointer();
        }

        if (!context) {
            context = new NTLMContext;

            if (!context) {
                return SEC_E_INSUFFICIENT_MEMORY;
            }

            context->server = true;
            if (*pfContextAttr == 1) {
                context->set_tests();
            }
            if (fContextReq & ASC_REQ_CONFIDENTIALITY) {
                context->confidentiality = true;
            }
            credentials = (CREDENTIALS*)phCredential->SecureHandleGetLowerPointer();
            context->identity.CopyAuthIdentity(credentials->identity);

            context->ntlm_SetContextServicePrincipalName(NULL);

            phNewContext->SecureHandleSetLowerPointer(context);
            phNewContext->SecureHandleSetUpperPointer((void*) NTLM_PACKAGE_NAME);
        }

        if (context->state == NTLM_STATE_INITIAL) {
            context->state = NTLM_STATE_NEGOTIATE;

            if (!pInput) {
                return SEC_E_INVALID_TOKEN;
            }
            if (pInput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (input_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            status = context->read_negotiate(input_buffer);

            if (context->state == NTLM_STATE_CHALLENGE) {
                if (!pOutput) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (pOutput->cBuffers < 1) {
                    return SEC_E_INVALID_TOKEN;
                }
                output_buffer = pOutput->FindSecBuffer(SECBUFFER_TOKEN);

                if (!output_buffer->BufferType) {
                    return SEC_E_INVALID_TOKEN;
                }
                if (output_buffer->Buffer.size() < 1) {
                    return SEC_E_INSUFFICIENT_MEMORY;
                }
                return context->write_challenge(output_buffer);
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }
        else if (context->state == NTLM_STATE_AUTHENTICATE) {
            if (!pInput) {
                return SEC_E_INVALID_TOKEN;
            }
            if (pInput->cBuffers < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            input_buffer = pInput->FindSecBuffer(SECBUFFER_TOKEN);

            if (!input_buffer) {
                return SEC_E_INVALID_TOKEN;
            }
            if (input_buffer->Buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            status = context->read_authenticate(input_buffer);

            if (pOutput) {
                size_t i;
                for (i = 0; i < pOutput->cBuffers; i++) {
                    pOutput->pBuffers[i].Buffer.init(0);
                    pOutput->pBuffers[i].BufferType = SECBUFFER_TOKEN;
                }
            }
            return status;
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    virtual SEC_STATUS FreeContextBuffer(void* pvContextBuffer) {
        NTLMContext * toDelete = (NTLMContext*) ((PSecHandle)pvContextBuffer)->SecureHandleGetLowerPointer();
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
        int index;
        int length;
        uint8_t* data;
        uint32_t SeqNo;
        uint8_t digest[16];
        uint8_t checksum[8];
        uint8_t* signature;
        uint32_t version = 1;
        NTLMContext* context = NULL;
        PSecBuffer data_buffer = NULL;
        PSecBuffer signature_buffer = NULL;

        SeqNo = MessageSeqNo;
        if (phContext) {
            context = (NTLMContext*) phContext->SecureHandleGetLowerPointer();
        }
        if (!context) {
            return SEC_E_NO_CONTEXT;
        }

        for (index = 0; index < (int) pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA) {
                data_buffer = &pMessage->pBuffers[index];
            }
            else if (pMessage->pBuffers[index].BufferType == SECBUFFER_TOKEN) {
                signature_buffer = &pMessage->pBuffers[index];
            }
        }

        if (!data_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        if (!signature_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        /* Copy original data buffer */
        length = data_buffer->Buffer.size();
        data = new uint8_t[length];
        memcpy(data, data_buffer->Buffer.get_data(), length);

        /* Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key */
        SslHMAC_Md5 hmac_md5(context->SendSigningKey, 16);
        hmac_md5.update((uint8_t*) &(SeqNo), 4);
        hmac_md5.update(data, length);
        hmac_md5.final(digest, sizeof(digest));

        /* Encrypt message using with RC4, result overwrites original buffer */

        if (context->confidentiality) {
            context->SendRc4Seal.crypt(length, data, data_buffer->Buffer.get_data());
        }
        else {
            data_buffer->Buffer.copy(data, length);
        }


// #ifdef WITH_DEBUG_NTLM
        // LOG(LOG_ERR, "======== ENCRYPT ==========");
        // LOG(LOG_ERR, "signing key (length = %d)\n", 16);
        // hexdump_c(context->SendSigningKey, 16);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Digest (length = %d)\n", sizeof(digest));
        // hexdump_c(digest, sizeof(digest));
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Data Buffer (length = %d)\n", length);
        // hexdump_c(data, length);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Encrypted Data Buffer (length = %d)\n", data_buffer->Buffer.size());
        // hexdump_c(data_buffer->Buffer.get_data(), data_buffer->Buffer.size());
        // LOG(LOG_ERR, "\n");
// #endif

        delete [] data;

        /* RC4-encrypt first 8 bytes of digest */
        context->SendRc4Seal.crypt(8, digest, checksum);

        signature = signature_buffer->Buffer.get_data();

        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(signature, (void*) & version, 4);
        memcpy(&signature[4], checksum, 8);
        memcpy(&signature[12], (void*) &(SeqNo), 4);
        context->SendSeqNum++;

// #ifdef WITH_DEBUG_NTLM
        // LOG(LOG_ERR, "Signature (length = %d)\n", signature_buffer->Buffer.size());
        // hexdump_c(signature_buffer->Buffer.get_data(), signature_buffer->Buffer.size());
        // LOG(LOG_ERR, "\n");
// #endif

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    virtual SEC_STATUS DecryptMessage(PCtxtHandle phContext, PSecBufferDesc pMessage,
                                      unsigned long MessageSeqNo, unsigned long * pfQOP) {
        int index = 0;
        int length = 0;
        uint8_t* data = NULL;
        uint32_t SeqNo = 0;
        uint8_t digest[16] = {};
        uint8_t checksum[8] = {};
        uint32_t version = 1;
        NTLMContext* context = NULL;
        uint8_t expected_signature[16] = {};
        PSecBuffer data_buffer = NULL;
        PSecBuffer signature_buffer = NULL;

        SeqNo = (uint32_t) MessageSeqNo;
        if (phContext) {
            context = (NTLMContext*) phContext->SecureHandleGetLowerPointer();
        }
        if (!context) {
            return SEC_E_NO_CONTEXT;
        }

        for (index = 0; index < (int) pMessage->cBuffers; index++) {
            if (pMessage->pBuffers[index].BufferType == SECBUFFER_DATA) {
                data_buffer = &pMessage->pBuffers[index];
            }
            else if (pMessage->pBuffers[index].BufferType == SECBUFFER_TOKEN) {
                signature_buffer = &pMessage->pBuffers[index];
            }
        }

        if (!data_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        if (!signature_buffer) {
            return SEC_E_INVALID_TOKEN;
        }
        /* Copy original data buffer */
        length = data_buffer->Buffer.size();
        data = new uint8_t[length];
        memcpy(data, data_buffer->Buffer.get_data(), length);

        /* Decrypt message using with RC4, result overwrites original buffer */

        if (context->confidentiality) {
            context->RecvRc4Seal.crypt(length, data, data_buffer->Buffer.get_data());
        }
        else {
            data_buffer->Buffer.copy(data, length);
        }

        /* Compute the HMAC-MD5 hash of ConcatenationOf(seq_num,data) using the client signing key */
        SslHMAC_Md5 hmac_md5(context->RecvSigningKey, 16);
        hmac_md5.update((uint8_t*) &(SeqNo), 4);
        hmac_md5.update(data_buffer->Buffer.get_data(), data_buffer->Buffer.size());
        hmac_md5.final(digest, sizeof(digest));

// #ifdef WITH_DEBUG_NTLM
        // LOG(LOG_ERR, "======== DECRYPT ==========");
        // LOG(LOG_ERR, "signing key (length = %d)\n", 16);
        // hexdump_c(context->RecvSigningKey, 16);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Digest (length = %d)\n", sizeof(digest));
        // hexdump_c(digest, sizeof(digest));
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Encrypted Data Buffer (length = %d)\n", length);
        // hexdump_c(data, length);
        // LOG(LOG_ERR, "\n");

        // LOG(LOG_ERR, "Data Buffer (length = %d)\n", data_buffer->Buffer.size());
        // hexdump_c(data_buffer->Buffer.get_data(), data_buffer->Buffer.size());
        // LOG(LOG_ERR, "\n");
// #endif

        delete [] data;

        /* RC4-encrypt first 8 bytes of digest */
        context->RecvRc4Seal.crypt(8, digest, checksum);

        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(expected_signature, (void*) &version, 4);
        memcpy(&expected_signature[4], checksum, 8);
        memcpy(&expected_signature[12], (void*) &(SeqNo), 4);
        context->RecvSeqNum++;

        if (memcmp(signature_buffer->Buffer.get_data(), expected_signature, 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!\n");
            LOG(LOG_ERR, "Expected Signature:\n");
            hexdump_c(expected_signature, 16);
            LOG(LOG_ERR, "Actual Signature:\n");
            hexdump_c(signature_buffer->Buffer.get_data(), 16);

            return SEC_E_MESSAGE_ALTERED;
        }

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
};

#endif
