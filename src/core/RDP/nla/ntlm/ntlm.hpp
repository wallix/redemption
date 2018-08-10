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
#include "core/RDP/nla/ntlm/ntlm_context.hpp"
#include "utils/sugar/byte.hpp"

#include <memory>
#include <functional>


// TODO: constants below are still globals,
// better to move them in the scope of functions/objects using them
namespace {
    //const char* NTLM_PACKAGE_NAME = "NTLM";
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
} // namespace

struct Ntlm_SecurityFunctionTable : public SecurityFunctionTable
{
    enum class PasswordCallback
    {
        Error,
        Ok,
        Wait,
    };

private:
    Random & rand;
    TimeObj & timeobj;
    std::unique_ptr<SEC_WINNT_AUTH_IDENTITY> identity;
    std::unique_ptr<NTLMContext> context;
    std::function<PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)>& set_password_cb;

public:
    SEC_WINNT_AUTH_IDENTITY const * getIdentityHandle() const
    {
        return this->identity.get();
    }

    NTLMContext const * getContextHandle() const
    {
        return this->context.get();
    }

public:
    explicit Ntlm_SecurityFunctionTable(
        Random & rand, TimeObj & timeobj,
        std::function<PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)> & set_password_cb
    )
        : rand(rand), timeobj(timeobj), set_password_cb(set_password_cb)
    {}

    ~Ntlm_SecurityFunctionTable() = default;

    // QUERY_SECURITY_PACKAGE_INFO QuerySecurityPackageInfo;
    SecPkgInfo QuerySecurityPackageInfo() override {
        return NTLM_SecPkgInfo;
    }

    // QUERY_CONTEXT_ATTRIBUTES QueryContextAttributes;
    SecPkgContext_Sizes QueryContextSizes() override {
        SecPkgContext_Sizes ContextSizes;
        ContextSizes.cbMaxToken = 2010;
        ContextSizes.cbMaxSignature = 16;
        ContextSizes.cbBlockSize = 0;
        ContextSizes.cbSecurityTrailer = 16;
        return ContextSizes;
    }

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    SEC_STATUS AcquireCredentialsHandle(
        const char * pszPrincipal, unsigned long fCredentialUse,
        Array * pvLogonID, SEC_WINNT_AUTH_IDENTITY * pAuthData
    ) override
    {
        (void)pszPrincipal;
        (void)pvLogonID;

        if (fCredentialUse == SECPKG_CRED_OUTBOUND
         || fCredentialUse == SECPKG_CRED_INBOUND)
        {
            this->identity = std::make_unique<SEC_WINNT_AUTH_IDENTITY>();

            if (pAuthData) {
                this->identity->CopyAuthIdentity(*pAuthData);
            }
        }

        return SEC_E_OK;
    }

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    SEC_STATUS InitializeSecurityContext(
        char* pszTargetName, unsigned long fContextReq,
        SecBuffer const* pinput_buffer, unsigned long verbose,
        SecBuffer& output_buffer
    ) override
    {
        if (verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");
        }

        if (!this->context) {
            this->context = std::make_unique<NTLMContext>(this->rand, this->timeobj, verbose);

            assert(fContextReq & ISC_REQ_CONFIDENTIALITY);  // this->context->confidentiality = true;
            // this->context->init();
            this->context->server = false;

            // if (this->context->Workstation.size() < 1)
            //     this->context->ntlm_SetContextWorkstation(nullptr);
            if (!this->identity) {
                return SEC_E_WRONG_CREDENTIAL_HANDLE;
            }
            this->context->ntlm_SetContextWorkstation(byte_ptr_cast(pszTargetName));
            this->context->ntlm_SetContextServicePrincipalName(byte_ptr_cast(pszTargetName));

            this->context->identity.CopyAuthIdentity(*this->identity);
        }

        if ((!pinput_buffer) || (this->context->state == NTLM_STATE_AUTHENTICATE)) {
            if (output_buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            if (this->context->state == NTLM_STATE_INITIAL) {
                this->context->state = NTLM_STATE_NEGOTIATE;
            }
            if (this->context->state == NTLM_STATE_NEGOTIATE) {
                return this->context->write_negotiate(output_buffer);
            }
            return SEC_E_OUT_OF_SEQUENCE;
        }

        if (pinput_buffer->size() < 1) {
            return SEC_E_INVALID_TOKEN;
        }
        // channel_bindings = sspi_FindSecBuffer(pInput, SECBUFFER_CHANNEL_BINDINGS);

        // if (channel_bindings) {
        //     this->context->Bindings.BindingsLength = channel_bindings->cbBuffer;
        //     this->context->Bindings.Bindings = (SEC_CHANNEL_BINDINGS*) channel_bindings->pvBuffer;
        // }

        if (this->context->state == NTLM_STATE_CHALLENGE) {
            this->context->read_challenge(*pinput_buffer);

            if (output_buffer.size() < 1) {
                return SEC_E_INSUFFICIENT_MEMORY;
            }
            if (this->context->state == NTLM_STATE_AUTHENTICATE) {
                return this->context->write_authenticate(output_buffer);
            }
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    SEC_STATUS AcceptSecurityContext(
        SecBuffer const& input_buffer, unsigned long fContextReq, SecBuffer& output_buffer
    ) override {
        if (!this->context) {
            this->context = std::make_unique<NTLMContext>(this->rand, this->timeobj);

            this->context->server = true;
            assert(fContextReq & ASC_REQ_CONFIDENTIALITY);  // this->context->confidentiality = true;
            if (!this->identity) {
                return SEC_E_WRONG_CREDENTIAL_HANDLE;
            }
            this->context->identity.CopyAuthIdentity(*this->identity);

            this->context->ntlm_SetContextServicePrincipalName(nullptr);
        }

        if (this->context->state == NTLM_STATE_INITIAL) {
            this->context->state = NTLM_STATE_NEGOTIATE;
            if (input_buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }
            /*SEC_STATUS status = */this->context->read_negotiate(input_buffer);

            if (this->context->state == NTLM_STATE_CHALLENGE) {
                if (output_buffer.size() < 1) {
                    return SEC_E_INSUFFICIENT_MEMORY;
                }

                return this->context->write_challenge(output_buffer);
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

        if (this->context->state == NTLM_STATE_AUTHENTICATE) {
            if (input_buffer.size() < 1) {
                return SEC_E_INVALID_TOKEN;
            }

            SEC_STATUS status = this->context->read_authenticate(input_buffer);

            if (status == SEC_I_CONTINUE_NEEDED) {
                if (!set_password_cb) {
                    return SEC_E_LOGON_DENIED;
                }
                switch (set_password_cb(this->context->identity)) {
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
        byte_array const& data_buffer, uint32_t SeqNo)
    {
        SslHMAC_Md5 hmac_md5(signing_key, 16);
        StaticOutStream<4> out_stream;
        out_stream.out_uint32_le(SeqNo);
        hmac_md5.update(out_stream.get_data(), out_stream.get_offset());
        hmac_md5.update(data_buffer.data(), data_buffer.size());
        hmac_md5.final(digest);
    }

    static void compute_signature(
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

public:
    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    SEC_STATUS EncryptMessage(SecBuffer& data, unsigned long MessageSeqNo) override {
        if (!this->context) {
            return SEC_E_NO_CONTEXT;
        }
        if (this->context->verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::EncryptMessage");
        }

        // data [signature][data_buffer]

        auto data_buffer = make_array_view(data.get_data(), data.size())
          .subarray(this->QueryContextSizes().cbMaxSignature);

        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->compute_hmac_md5(digest, this->context->SendSigningKey, data_buffer, MessageSeqNo);

        /* Encrypt message using with RC4, result overwrites original buffer */
        // this->context->confidentiality == true
        this->context->SendRc4Seal.crypt(
            data_buffer.size(), data_buffer.data(), data_buffer.data());

        this->compute_signature(
            data.get_data(), this->context->SendRc4Seal, digest, MessageSeqNo);

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    SEC_STATUS DecryptMessage(SecBuffer const& data_in, SecBuffer& data_out, unsigned long MessageSeqNo) override {
        if (!this->context) {
            return SEC_E_NO_CONTEXT;
        }
        if (this->context->verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::DecryptMessage");
        }

        // data [signature][data_buffer]

        auto data_buffer = make_array_view(data_in.get_data(), data_in.size())
          .subarray(this->QueryContextSizes().cbMaxSignature);
        data_out.init(data_buffer.size());

        /* Decrypt message using with RC4, result overwrites original buffer */
        // this->context->confidentiality == true
        this->context->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.get_data());

        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->compute_hmac_md5(digest, this->context->RecvSigningKey, {data_out.get_data(), data_out.size()}, MessageSeqNo);

        uint8_t expected_signature[16] = {};
        this->compute_signature(
            expected_signature, this->context->RecvRc4Seal, digest, MessageSeqNo);

        if (memcmp(data_in.get_data(), expected_signature, 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature, 16);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(data_in.get_data(), 16);

            return SEC_E_MESSAGE_ALTERED;
        }

        return SEC_E_OK;
    }

    // IMPERSONATE_SECURITY_CONTEXT ImpersonateSecurityContext;
    SEC_STATUS ImpersonateSecurityContext() override {
        return SEC_E_OK;
    }
    // REVERT_SECURITY_CONTEXT RevertSecurityContext;
    SEC_STATUS RevertSecurityContext() override {
        return SEC_E_OK;
    }
};

