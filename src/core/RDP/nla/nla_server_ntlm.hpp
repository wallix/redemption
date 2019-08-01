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

#include <functional>

#include "core/RDP/nla/sspi.hpp"
#include "core/RDP/nla/credssp.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "utils/hexdump.hpp"
#include "utils/translation.hpp"
#include "system/ssl_sha256.hpp"

#include "utils/genrandom.hpp"
#include "utils/difftimeval.hpp"
#include "utils/utf.hpp"
#include "utils/stream.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"


// static const uint8_t lm_magic[] = "KGS!@#$%";

static const uint8_t client_sign_magic[] =
    "session key to client-to-server signing key magic constant";
static const uint8_t server_sign_magic[] =
    "session key to server-to-client signing key magic constant";
static const uint8_t client_seal_magic[] =
    "session key to client-to-server sealing key magic constant";
static const uint8_t server_seal_magic[] =
    "session key to server-to-client sealing key magic constant";

#include "transport/transport.hpp"


class rdpCredsspServerNTLM final
{
    static constexpr uint32_t cbMaxSignature = 16;

    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;

    TSRequest ts_request = {6}; // Credssp Version 6 Supported
    static const size_t CLIENT_NONCE_LENGTH = 32;
    ClientNonce SavedClientNonce;

    TimeObj & timeobj;
    Random & rand;
    array_view_u8 public_key;

    private:
    std::function<PasswordCallback(cbytes_view,cbytes_view,Array&)> set_password_cb;
    std::string& extra_message;
    Translation::language_t lang;
    const bool verbose;

    array_sha256 ClientServerHash;
    array_sha256 ServerClientHash;

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        LOG(LOG_INFO, "set hostname from UTF8");
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        LOG(LOG_INFO, "length=%lu", length);
    }

    void credssp_buffer_free() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::buffer_free");
        this->ts_request.negoTokens.init(0);
        this->ts_request.pubKeyAuth.init(0);
        this->ts_request.authInfo.init(0);
        this->ts_request.clientNonce.reset();
        this->ts_request.error_code = 0;
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

private:
    const bool NTLMv2 = true;
    bool UseMIC = true; // NTLMv2
public:
    NtlmState state = NTLM_STATE_INITIAL;

private:
    uint8_t MachineID[32];

public:
    SslRC4 SendRc4Seal {};
    SslRC4 RecvRc4Seal {};
private:
    uint32_t NegotiateFlags = 0;

public:

    std::vector<uint8_t> identity_User;
    std::vector<uint8_t> identity_Domain;
    Array identity_Password{0};

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
    NTLMNegotiateMessage NEGOTIATE_MESSAGE;
    NTLMChallengeMessage CHALLENGE_MESSAGE;
    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;

private:
    NtlmVersion version;
public:
    std::vector<uint8_t> SavedNegotiateMessage;
    std::vector<uint8_t> SavedChallengeMessage;
    std::vector<uint8_t> SavedAuthenticateMessage;

private:
    uint8_t Timestamp[8]{};
    uint8_t ChallengeTimestamp[8]{};
    array_challenge ServerChallenge;
    array_challenge ClientChallenge;
    array_md5 SessionBaseKey;
    array_md5 ExportedSessionKey;
public:
    /**
     * Generate client signing key (ClientSigningKey).\n
     * @msdn{cc236711}
     */

    array_md5 ClientSigningKey;
private:
    /**
     * Generate client sealing key (ClientSealingKey).\n
     * @msdn{cc236712}
     */

    array_md5 ClientSealingKey;
public:
    /**
     * Generate server signing key (ServerSigningKey).\n
     * @msdn{cc236711}
     */

    array_md5 ServerSigningKey;
private:
    /**
     * Generate server sealing key (ServerSealingKey).\n
     * @msdn{cc236712}
     */

    array_md5 ServerSealingKey;
    array_md5 MessageIntegrityCheck;
    // uint8_t NtProofStr[16];

    private:

    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    void ntlm_server_build_challenge(TimeObj & timeobj, Random & rand) {
        uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
        uint32_t const mask = NTLMSSP_REQUEST_TARGET
                            | NTLMSSP_NEGOTIATE_NTLM
                            | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                            | NTLMSSP_NEGOTIATE_UNICODE;
        if ((negoFlag & mask) != mask) {
            LOG(LOG_ERR, "ERROR CHECK NEGO FLAGS");
        }
        this->NegotiateFlags = negoFlag;
    
        rand.random(this->ServerChallenge.data(), this->ServerChallenge.size());
        this->CHALLENGE_MESSAGE.serverChallenge = this->ServerChallenge;

        uint8_t ZeroTimestamp[8] = {};

        if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0) {
            memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
        }
        else {
            const timeval tv = timeobj.get_time();
            OutStream out_stream(this->Timestamp);
            out_stream.out_uint32_le(tv.tv_usec);
            out_stream.out_uint32_le(tv.tv_sec);
        }

        // NTLM: construct challenge target info
        std::vector<uint8_t> win7{ 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00 };
        std::vector<uint8_t> upwin7{ 0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00 };
         
        auto & list = this->CHALLENGE_MESSAGE.AvPairList;
        list.push_back(AvPair({MsvAvNbComputerName, upwin7}));
        list.push_back(AvPair({MsvAvNbDomainName, upwin7}));
        list.push_back(AvPair({MsvAvDnsComputerName, win7}));
        list.push_back(AvPair({MsvAvDnsDomainName, win7}));
        list.push_back({MsvAvTimestamp, std::vector<uint8_t>(this->Timestamp, this->Timestamp+sizeof(this->Timestamp))});

        this->CHALLENGE_MESSAGE.negoFlags.flags = negoFlag;
        if (negoFlag & NTLMSSP_NEGOTIATE_VERSION) {
            this->CHALLENGE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            this->CHALLENGE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            this->CHALLENGE_MESSAGE.version.ProductBuild        = 7601;
            this->CHALLENGE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
            
        }

        this->state = NTLM_STATE_AUTHENTICATE;
    }

    // SERVER PROCEED RESPONSE CHECKING
    SEC_STATUS ntlm_server_proceed_authenticate(array_view_const_u8 hash) {
        if (!this->AUTHENTICATE_MESSAGE.check_nt_response_from_authenticate(hash, this->ServerChallenge)) {
            LOG(LOG_ERR, "NT RESPONSE NOT MATCHING STOP AUTHENTICATE");
            return SEC_E_LOGON_DENIED;
        }
        if (!this->AUTHENTICATE_MESSAGE.check_lm_response_from_authenticate(hash, this->ServerChallenge)) {
            LOG(LOG_ERR, "LM RESPONSE NOT MATCHING STOP AUTHENTICATE");
            return SEC_E_LOGON_DENIED;
        }
        // SERVER COMPUTE SHARED KEY WITH CLIENT
        this->SessionBaseKey = this->AUTHENTICATE_MESSAGE.compute_session_base_key(hash);
        this->ExportedSessionKey = this->AUTHENTICATE_MESSAGE.get_exported_session_key(this->SessionBaseKey);
        this->ClientSigningKey = Md5(this->ExportedSessionKey, make_array_view(client_sign_magic));
        this->ClientSealingKey = Md5(this->ExportedSessionKey, make_array_view(client_seal_magic));
        this->ServerSigningKey = Md5(this->ExportedSessionKey, make_array_view(server_sign_magic));
        this->ServerSealingKey  = Md5(this->ExportedSessionKey, make_array_view(server_seal_magic));

        /**
         * Initialize RC4 stream cipher states for sealing.
         */

        this->SendRc4Seal.set_key(this->ServerSealingKey);
        this->RecvRc4Seal.set_key(this->ClientSealingKey);

        // =======================================================

        if (this->UseMIC) {
            this->MessageIntegrityCheck = HmacMd5(this->ExportedSessionKey,
                this->SavedNegotiateMessage,
                this->SavedChallengeMessage,
                this->SavedAuthenticateMessage);

            if (0 != memcmp(this->MessageIntegrityCheck.data(), this->AUTHENTICATE_MESSAGE.MIC, 16)) {
                LOG(LOG_ERR, "MIC NOT MATCHING STOP AUTHENTICATE");
                hexdump_c(this->MessageIntegrityCheck.data(), 16);
                hexdump_c(this->AUTHENTICATE_MESSAGE.MIC, 16);
                return SEC_E_MESSAGE_ALTERED;
            }
        }
        this->state = NTLM_STATE_FINAL;
        return SEC_I_COMPLETE_NEEDED;
    }

    public:

    SEC_STATUS read_negotiate(array_view_const_u8 input_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Read Negotiate");
        InStream in_stream(input_buffer);
        RecvNTLMNegotiateMessage(in_stream, this->NEGOTIATE_MESSAGE);
        uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
        uint32_t const mask = NTLMSSP_REQUEST_TARGET
                            | NTLMSSP_NEGOTIATE_NTLM
                            | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                            | NTLMSSP_NEGOTIATE_UNICODE;
        if ((negoFlag & mask) != mask) {
            return SEC_E_INVALID_TOKEN;
        }
        this->NegotiateFlags = negoFlag;

        this->SavedNegotiateMessage.clear();
        push_back_array(this->SavedNegotiateMessage, in_stream.get_consumed_bytes());

        this->state = NTLM_STATE_CHALLENGE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS write_challenge(Array& output_buffer, TimeObj & timeobj, Random & rand) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Write Challenge");
        this->ntlm_server_build_challenge(timeobj, rand);
        StaticOutStream<65535> out_stream;
        EmitNTLMChallengeMessage(out_stream, this->CHALLENGE_MESSAGE);
        output_buffer.init(out_stream.get_offset());
        output_buffer.copy(out_stream.get_bytes());

        this->SavedChallengeMessage.clear();
        push_back_array(this->SavedChallengeMessage, out_stream.get_bytes());

        this->state = NTLM_STATE_AUTHENTICATE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS read_authenticate(array_view_const_u8 input_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Read Authenticate");
        InStream in_stream(input_buffer);
        recvNTLMAuthenticateMessage(in_stream, this->AUTHENTICATE_MESSAGE);
        if (this->AUTHENTICATE_MESSAGE.has_mic) {
            this->UseMIC = true;
            this->SavedAuthenticateMessage.clear();
            constexpr std::size_t null_data_sz = 16;
            uint8_t const null_data[null_data_sz]{0u};
            push_back_array(this->SavedAuthenticateMessage, {in_stream.get_data(), this->AUTHENTICATE_MESSAGE.PayloadOffset});
            push_back_array(this->SavedAuthenticateMessage, {null_data, null_data_sz});
            push_back_array(this->SavedAuthenticateMessage, {in_stream.get_data() + this->AUTHENTICATE_MESSAGE.PayloadOffset + null_data_sz, in_stream.get_offset() - this->AUTHENTICATE_MESSAGE.PayloadOffset - null_data_sz});
        }

        auto & avuser = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        this->identity_User.assign(avuser.data(), avuser.data()+avuser.size());
        auto & avdomain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        this->identity_Domain.assign(avdomain.data(), avdomain.data()+avdomain.size());


        if ((this->identity_User.size() == 0) && (this->identity_Domain.size() == 0)){
            LOG(LOG_ERR, "ANONYMOUS User not allowed");
            return SEC_E_LOGON_DENIED;
        }

        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS check_authenticate() {
        cbytes_view password_av{this->identity_Password.get_data(), this->identity_Password.size()};
        array_md4 hash;
        if (password_av.size() > 0){
            hash = Md4(password_av);
        }
        return this->ntlm_server_proceed_authenticate(hash);
    }

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext
    // -> only for clients : unused for NTLM server

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    SEC_STATUS AcceptSecurityContext(array_view_const_u8 input_buffer, Array& output_buffer, TimeObj & timeobj, Random & rand)
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::AcceptSecurityContext");

        if (this->state == NTLM_STATE_INITIAL) {

            this->state = NTLM_STATE_NEGOTIATE;
            SEC_STATUS status = this->read_negotiate(input_buffer);
            if (status != SEC_I_CONTINUE_NEEDED) {
                return SEC_E_INVALID_TOKEN;
            }

            if (this->state == NTLM_STATE_CHALLENGE) {
                return this->write_challenge(output_buffer, timeobj, rand);
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

        if (this->state == NTLM_STATE_AUTHENTICATE) {
            SEC_STATUS status = this->read_authenticate(input_buffer);

            if (status == SEC_I_CONTINUE_NEEDED) {
                if (!this->set_password_cb) {
                    return SEC_E_LOGON_DENIED;
                }
                switch (set_password_cb(this->identity_User
                                       ,this->identity_Domain
                                       ,this->identity_Password)) {
                    case PasswordCallback::Error:
                        return SEC_E_LOGON_DENIED;
                    case PasswordCallback::Ok:
                        this->state = NTLM_STATE_WAIT_PASSWORD;
                        break;
                    case PasswordCallback::Wait:
                        this->state = NTLM_STATE_WAIT_PASSWORD;
                        return SEC_I_LOCAL_LOGON;
                }
            }
        }

        if (this->state == NTLM_STATE_WAIT_PASSWORD) {
            SEC_STATUS status = this->check_authenticate();
            if (status != SEC_I_CONTINUE_NEEDED && status != SEC_I_COMPLETE_NEEDED) {
                return status;
            }

            output_buffer.init(0);

            return status;
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

private:
    // ENCRYPT_MESSAGE EncryptMessage;
    SEC_STATUS EncryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo)
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::EncryptMessage");

        // data_out [signature][data_buffer]

        data_out.init(data_in.size() + cbMaxSignature);
        auto message_out = data_out.av().from_at(cbMaxSignature);
        array_md5 digest = HmacMd5(this->ServerSigningKey, out_uint32_le(MessageSeqNo), data_in);
        // this->confidentiality == true
        this->SendRc4Seal.crypt(data_in.size(), data_in.data(), message_out.data());
        
        uint8_t * signature = data_out.get_data();
        uint8_t checksum[8];
        /* RC4-encrypt first 8 bytes of digest */
        this->SendRc4Seal.crypt(8, digest.data(), checksum);

        uint32_t version = 1;
        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(signature, &version, 4);
        memcpy(&signature[4], checksum, 8);
        memcpy(&signature[12], &MessageSeqNo, 4);
        
        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;

public:
    rdpCredsspServerNTLM(array_view_u8 key,
               Random & rand,
               TimeObj & timeobj,
               std::string& extra_message,
               Translation::language_t lang,
               std::function<PasswordCallback(cbytes_view,cbytes_view,Array&)> set_password_cb,
               const bool verbose = false)
        : timeobj(timeobj)
        , rand(rand)
        , public_key(key)
        , set_password_cb(set_password_cb)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
    {
        memset(this->MachineID, 0xAA, sizeof(this->MachineID));
        memset(this->MessageIntegrityCheck.data(), 0x00, this->MessageIntegrityCheck.size());

        LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Init");

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::Initialization: NTLM Authentication");
        this->SetHostnameFromUtf8(nullptr);
        LOG_IF(this->verbose, LOG_INFO, "this->server_auth_data.state = ServerAuthenticateData::Start");
        this->server_auth_data.state = ServerAuthenticateData::Start;
        // TODO: sspi_GlobalInit();

        // Note: NTLMAcquireCredentialHandle never fails

        /*
        * from tspkg.dll: 0x00000112
        * ASC_REQ_MUTUAL_AUTH
        * ASC_REQ_CONFIDENTIALITY
        * ASC_REQ_ALLOCATE_MEMORY
        */
        this->server_auth_data.state = ServerAuthenticateData::Loop;
        this->identity_User = {};
        this->identity_Domain = {};
        this->identity_Password.init(0);
    }

public:
    credssp::State credssp_server_authenticate_next(InStream & in_stream, OutStream & out_stream)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::credssp_server_authenticate_next");

        switch (this->server_auth_data.state)
        {
            case ServerAuthenticateData::Start:
              LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Start");
              return credssp::State::Err;
            case ServerAuthenticateData::Loop:
                LOG(LOG_INFO, "ServerAuthenticateData::Loop");
                if (Res::Err == this->sm_credssp_server_authenticate_recv(in_stream, out_stream)) {
                    LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                    return credssp::State::Err;
                }
                return credssp::State::Cont;
            case ServerAuthenticateData::Final:
               LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final");
               if (Res::Err == this->sm_credssp_server_authenticate_final(in_stream)) {
                   LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final::Err");
                    return credssp::State::Err;
                }
                this->server_auth_data.state = ServerAuthenticateData::Start;
                return credssp::State::Finish;
        }

        return credssp::State::Err;
    }
private:

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_public_key_echo");

        array_view_const_u8 data_in = this->ts_request.pubKeyAuth.av();
        unsigned long MessageSeqNo = this->recv_seq_num++;
        LOG_IF(this->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

        if (data_in.size() < cbMaxSignature) {
            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "DecryptMessage failure: SEC_E_INVALID_TOKEN 0x%08X", SEC_E_INVALID_TOKEN);
            return SEC_E_INVALID_TOKEN;
        }
        // data_in [signature][data_buffer]

        auto data_buffer = data_in.from_at(cbMaxSignature);
        std::vector<uint8_t> result_buffer(data_buffer.size());

        /* Decrypt message using with RC4 */
        // context->confidentiality == true
        this->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), result_buffer.data());

        array_md5 digest = HmacMd5(this->ClientSigningKey, out_uint32_le(MessageSeqNo), result_buffer);
        uint8_t checksum[8];
        /* RC4-encrypt first 8 bytes of digest */
        this->RecvRc4Seal.crypt(8, digest.data(), checksum);

        std::vector<uint8_t> expected_signature;
        uint32_t seal_version = 1;
        /* Concatenate version, ciphertext and sequence number to build signature */
        
        push_back_array(expected_signature, out_uint32_le(seal_version));
        push_back_array(expected_signature, {checksum, 8});
        push_back_array(expected_signature, out_uint32_le(MessageSeqNo));

        if (memcmp(data_in.data(), expected_signature.data(),  expected_signature.size()) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(data_in.data(), 16);

            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "DecryptMessage failure: SEC_E_MESSAGE_ALTERED 0x%08X", SEC_E_MESSAGE_ALTERED);
            return SEC_E_MESSAGE_ALTERED;
        }

        if (this->ts_request.use_version >= 5) {
            if (this->ts_request.clientNonce.isset()){
                this->SavedClientNonce = this->ts_request.clientNonce;
            }
            this->ClientServerHash = Sha256("CredSSP Client-To-Server Binding Hash\0"_av,
                                    make_array_view(this->SavedClientNonce.data, CLIENT_NONCE_LENGTH),
                                    this->public_key);
            this->public_key = this->ClientServerHash;
        }

        if (result_buffer.size() != this->public_key.size()) {
            LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", result_buffer.size(), this->public_key.size());
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        if (memcmp(this->public_key.data(), result_buffer.data(), public_key.size()) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo");

            LOG(LOG_ERR, "Expected (length = %zu):", this->public_key.size());
            hexdump_c(this->public_key);

            LOG(LOG_ERR, "Actual (length = %zu):", this->public_key.size());
            hexdump_c(result_buffer);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }
        return SEC_E_OK;
    }

    SEC_STATUS credssp_decrypt_ts_credentials() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_ts_credentials");

        if (this->ts_request.authInfo.size() < 1) {
            LOG(LOG_ERR, "credssp_decrypt_ts_credentials missing ts_request.authInfo buffer");
            return SEC_E_INVALID_TOKEN;
        }

        Array Buffer;

        array_view_const_u8 data_in = this->ts_request.authInfo.av();
        unsigned long MessageSeqNo = this->recv_seq_num++;
        LOG_IF(this->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");
        if (data_in.size() < cbMaxSignature) {
            return SEC_E_INVALID_TOKEN;
        }
        // data_in [signature][data_buffer]

        auto data_buffer = data_in.from_at(cbMaxSignature);
        Buffer.init(data_buffer.size());

        /* Decrypt message using with RC4, result overwrites original buffer */
        // context->confidentiality == true
        this->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), Buffer.get_data());

        array_md5 digest = HmacMd5(this->ClientSigningKey, out_uint32_le(MessageSeqNo), Buffer.av());

        uint8_t expected_signature[16] = {};
        uint8_t * signature = expected_signature;
        uint8_t checksum[8];
        /* RC4-encrypt first 8 bytes of digest */
        this->RecvRc4Seal.crypt(8, digest.data(), checksum);

        uint32_t version = 1;
        /* Concatenate version, ciphertext and sequence number to build signature */
        memcpy(signature, &version, 4);
        memcpy(&signature[4], checksum, 8);
        memcpy(&signature[12], &MessageSeqNo, 4);

        if (memcmp(data_in.data(), expected_signature, 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature, 16);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(data_in.data(), 16);

            return SEC_E_MESSAGE_ALTERED;
        }

        InStream decrypted_creds(Buffer.av());
        this->ts_credentials.recv(decrypted_creds);

        return SEC_E_OK;
    }

    Res sm_credssp_server_authenticate_recv(InStream & in_stream, OutStream & out_stream)
    {
        LOG_IF(this->verbose, LOG_INFO,"rdpCredsspServer::sm_credssp_server_authenticate_recv");

        if (this->state_accept_security_context != SEC_I_LOCAL_LOGON) {
            /* receive authentication token */
            this->ts_request.recv(in_stream);
        }

        if (this->ts_request.negoTokens.size() < 1) {
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


        SEC_STATUS status = this->AcceptSecurityContext(this->ts_request.negoTokens.av(), /*output*/this->ts_request.negoTokens, this->timeobj, this->rand);
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

            this->ts_request.negoTokens.init(0);

            LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::encrypt_public_key_echo");
            uint32_t version = this->ts_request.use_version;

            if (version >= 5) {
                if (this->ts_request.clientNonce.isset()){
                    this->SavedClientNonce = this->ts_request.clientNonce;
                }
                this->ServerClientHash = Sha256("CredSSP Server-To-Client Binding Hash\0"_av,
                                            make_array_view(this->SavedClientNonce.data, CLIENT_NONCE_LENGTH),
                                            this->public_key);
                this->public_key = this->ServerClientHash;
            }
            else {
                // if we are server and protocol is 2,3,4
                // then echos the public key +1
                ::ap_integer_increment_le(this->public_key);
            }

            this->EncryptMessage(
                this->public_key, this->ts_request.pubKeyAuth, this->send_seq_num++);

        }

        if ((status != SEC_E_OK) && (status != SEC_I_CONTINUE_NEEDED)) {
            LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X", status);
            return Res::Err;
        }

        this->ts_request.emit(out_stream);
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

    Res sm_credssp_server_authenticate_final(InStream & in_stream)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::sm_credssp_server_authenticate_final");
        /* Receive encrypted credentials */
        this->ts_request.recv(in_stream);

        SEC_STATUS status = this->credssp_decrypt_ts_credentials();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X", status);
            return Res::Err;
        }

        return Res::Ok;
    }
};


