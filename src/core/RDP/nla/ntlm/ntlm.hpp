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
#include "utils/sugar/byte_ptr.hpp"

#include <memory>
#include <functional>


// TODO: constants below are still globals,
// better to move them in the scope of functions/objects using them
//const char* NTLM_PACKAGE_NAME = "NTLM";
// const char Ntlm_Name[] = "NTLM";
// const char Ntlm_Comment[] = "NTLM Security Package";
// const SecPkgInfo NTLM_SecPkgInfo = {
//     0x00082B37,             // fCapabilities
//     1,                      // wVersion
//     0x000A,                 // wRPCID
//     0x00000B48,             // cbMaxToken
//     Ntlm_Name,              // Name
//     Ntlm_Comment            // Comment
// };
constexpr uint32_t cbMaxSignature = 16;
// SecPkgContext_Sizes ContextSizes;
// ContextSizes.cbMaxToken = 2010;
// ContextSizes.cbMaxSignature = 16;
// ContextSizes.cbBlockSize = 0;
// ContextSizes.cbSecurityTrailer = 16;


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
    bool verbose;

public:
    explicit Ntlm_SecurityFunctionTable(
        Random & rand, TimeObj & timeobj,
        std::function<PasswordCallback(SEC_WINNT_AUTH_IDENTITY&)> & set_password_cb,
        bool verbose = false
    )
        : rand(rand)
        , timeobj(timeobj)
        , set_password_cb(set_password_cb)
        , verbose(verbose)
    {}

    ~Ntlm_SecurityFunctionTable() = default;

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    SEC_STATUS AcquireCredentialsHandle(
        const char * pszPrincipal, Array * pvLogonID, SEC_WINNT_AUTH_IDENTITY const* pAuthData
    ) override
    {
        (void)pszPrincipal;
        (void)pvLogonID;

        this->identity = std::make_unique<SEC_WINNT_AUTH_IDENTITY>();

        if (pAuthData) {
            this->identity->CopyAuthIdentity(*pAuthData);
        }

        return SEC_E_OK;
    }

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    SEC_STATUS InitializeSecurityContext(
        array_view_const_char pszTargetName, array_view_const_u8 input_buffer, Array& output_buffer
    ) override
    {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");
        }

        if (!this->context) {
            this->context = std::make_unique<NTLMContext>(
                false, this->rand, this->timeobj, this->verbose);

            if (!this->identity) {
                return SEC_E_WRONG_CREDENTIAL_HANDLE;
            }
            this->context->ntlm_SetContextWorkstation(pszTargetName);
            this->context->ntlm_SetContextServicePrincipalName(pszTargetName);

            this->context->identity.CopyAuthIdentity(*this->identity);
        }

        if (this->context->state == NTLM_STATE_INITIAL) {
            this->context->state = NTLM_STATE_NEGOTIATE;
        }
        if (this->context->state == NTLM_STATE_NEGOTIATE) {
            return this->context->write_negotiate(output_buffer);
        }

        if (this->context->state == NTLM_STATE_CHALLENGE) {
            this->context->read_challenge(input_buffer);
        }
        if (this->context->state == NTLM_STATE_AUTHENTICATE) {
            return this->context->write_authenticate(output_buffer);
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    SEC_STATUS AcceptSecurityContext(
        array_view_const_u8 input_buffer, Array& output_buffer
    ) override {
        if (!this->context) {
            this->context = std::make_unique<NTLMContext>(true, this->rand, this->timeobj);

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
    SEC_STATUS EncryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo) override {
        if (!this->context) {
            return SEC_E_NO_CONTEXT;
        }
        if (this->context->verbose) {
            LOG(LOG_INFO, "NTLM_SSPI::EncryptMessage");
        }

        // data_out [signature][data_buffer]

        data_out.init(data_in.size() + cbMaxSignature);
        auto message_out = data_out.av().array_from_offset(cbMaxSignature);

        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->compute_hmac_md5(digest, *this->context->SendSigningKey, data_in, MessageSeqNo);

        /* Encrypt message using with RC4, result overwrites original buffer */
        // this->context->confidentiality == true
        this->context->SendRc4Seal.crypt(data_in.size(), data_in.data(), message_out.data());

        this->compute_signature(
            data_out.get_data(), this->context->SendRc4Seal, digest, MessageSeqNo);

        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    SEC_STATUS DecryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo) override {
        if (!this->context) {
            return SEC_E_NO_CONTEXT;
        }
        if (this->context->verbose & 0x400) {
            LOG(LOG_INFO, "NTLM_SSPI::DecryptMessage");
        }

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
};
