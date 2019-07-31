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

    array_view_u8 public_key;

    private:
    std::function<PasswordCallback(cbytes_view,cbytes_view,Array&)> set_password_cb;
    std::string& extra_message;
    Translation::language_t lang;
    const bool verbose;

    Array ClientServerHash;
    Array ServerClientHash;

    Array ServicePrincipalName;

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        LOG(LOG_INFO, "set hostname from UTF8");
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        LOG(LOG_INFO, "length=%lu", length);
        this->ServicePrincipalName.init(length + 1);
        if (length){
            this->ServicePrincipalName.copy({pszTargetName, length});
        }
        this->ServicePrincipalName.get_data()[length] = 0;
    }

    void credssp_generate_public_key_hash_client_to_server() {
        LOG(LOG_DEBUG, "rdpCredsspServer::generate credssp public key hash (client->server)");
        Array & SavedHash = this->ClientServerHash;
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, CLIENT_NONCE_LENGTH));
        sha256.update(this->public_key);
        sha256.final(hash);
        SavedHash.init(sizeof(hash));
        memcpy(SavedHash.get_data(), hash, sizeof(hash));
    }

    void credssp_generate_public_key_hash_server_to_client() {
        LOG(LOG_DEBUG, "rdpCredsspServer::generate credssp public key hash (server->client)");
        Array & SavedHash = this->ServerClientHash;
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Server-To-Client Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, CLIENT_NONCE_LENGTH));
        sha256.update(this->public_key);
        sha256.final(hash);
        SavedHash.init(sizeof(hash));
        memcpy(SavedHash.get_data(), hash, sizeof(hash));
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
    class NTLMContextServer
    {
        TimeObj & timeobj;
        Random & rand;

        const bool NTLMv2 = true;
        bool UseMIC;
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
        Array ServicePrincipalName;
        SEC_WINNT_AUTH_IDENTITY identity;

        // bool SendSingleHostData;
        // NTLM_SINGLE_HOST_DATA SingleHostData;
        NTLMNegotiateMessage NEGOTIATE_MESSAGE;
        NTLMChallengeMessage CHALLENGE_MESSAGE;
        NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;

    private:
        NtlmVersion version;
    public:
        Array SavedNegotiateMessage;
        Array SavedChallengeMessage;
        Array SavedAuthenticateMessage;

    private:
        uint8_t Timestamp[8]{};
        uint8_t ChallengeTimestamp[8]{};
        uint8_t ServerChallenge[8]{};
        uint8_t ClientChallenge[8]{};
        array_md5 SessionBaseKey;
        array_md5 ExportedSessionKey;
        array_md5 EncryptedRandomSessionKey;
    public:
        array_md5 ClientSigningKey;
    private:
        array_md5 ClientSealingKey;
    public:
        uint8_t ServerSigningKey[16]{};
    private:
        uint8_t ServerSealingKey[16]{};
        array_md5 MessageIntegrityCheck;
        // uint8_t NtProofStr[16];
    public:
        const bool verbose;

    public:
        explicit NTLMContextServer(Random & rand, TimeObj & timeobj, bool verbose = false)
            : timeobj(timeobj)
            , rand(rand)
            , UseMIC(this->NTLMv2/* == true*/)
            , ServicePrincipalName(0)
            , SavedNegotiateMessage(0)
            , SavedChallengeMessage(0)
            , SavedAuthenticateMessage(0)
            , verbose(verbose)
        {
            memset(this->MachineID, 0xAA, sizeof(this->MachineID));
            memset(this->MessageIntegrityCheck.data(), 0x00, this->MessageIntegrityCheck.size());

            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Init");
        }

        NTLMContextServer(NTLMContextServer const &) = delete;
        NTLMContextServer& operator = (NTLMContextServer const &) = delete;

        private:
        /**
         * Generate timestamp for AUTHENTICATE_MESSAGE.
         */
        void ntlm_generate_timestamp()
        {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer TimeStamp");
            uint8_t ZeroTimestamp[8] = {};

            if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0) {
                memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
            }
            else {
                const timeval tv = this->timeobj.get_time();
                OutStream out_stream(this->Timestamp);
                out_stream.out_uint32_le(tv.tv_usec);
                out_stream.out_uint32_le(tv.tv_sec);
            }
        }

        /**
         * Generate server challenge (8-byte nonce).
         */
        void ntlm_generate_server_challenge()
        {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Generate Server Challenge");
            this->rand.random(this->ServerChallenge, 8);
        }

        // all strings are in unicode utf16
        void NTOWFv2(array_view_const_u8 pass,
                     array_view_const_u8 user,
                     array_view_const_u8 domain,
                     array_view_u8 buff) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer NTOWFv2");
            
            array_md4 md4password = Md4(pass);
            auto userup = UTF16_to_upper(user);
            array_md5 tmp_md5 = HmacMd5(md4password, userup, domain);

            // TODO: check if buff_size is SslMd5::DIGEST_LENGTH
            // if it is so no need to use a temporary variable
            // and copy digest afterward.
            memset(buff.data(), 0, buff.size());
            memcpy(buff.data(), tmp_md5.data(), std::min(buff.size(), size_t(SslMd5::DIGEST_LENGTH)));
        }

        // server method to decrypt exported session key from authenticate message with
        // session base key computed with Responses.
        void ntlm_decrypt_exported_session_key() {
            auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Decrypt RandomSessionKey");
            memcpy(this->EncryptedRandomSessionKey.data(), AuthEncryptedRSK.data(), AuthEncryptedRSK.size());

            // ntlm_rc4k
            this->ExportedSessionKey = Rc4Key(this->SessionBaseKey, this->EncryptedRandomSessionKey); 
        }

        void ntlm_compute_MIC() {
            this->MessageIntegrityCheck = HmacMd5(this->ExportedSessionKey,
                this->SavedNegotiateMessage.av(),
                this->SavedChallengeMessage.av(),
                this->SavedAuthenticateMessage.av());
        }


        private:

        // server check nt response
        bool ntlm_check_nt_response_from_authenticate(NTLMAuthenticateMessage & AUTHENTICATE_MESSAGE, array_view_const_u8 hash) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Check NtResponse");
            auto & AuthNtResponse = AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
            auto & DomainName = AUTHENTICATE_MESSAGE.DomainName.buffer;
            auto & UserName = AUTHENTICATE_MESSAGE.UserName.buffer;

            auto userup = UTF16_to_upper(UserName);
            array_md5 ResponseKeyNT = HmacMd5(hash, userup, DomainName);
            array_md5 NtProofStr = HmacMd5(ResponseKeyNT,
                                           make_array_view(this->ServerChallenge),
                                           {AuthNtResponse.data()+16, AuthNtResponse.size()-16});

            uint8_t NtProofStr_from_msg[16] = {};
            memcpy(NtProofStr_from_msg, AuthNtResponse.data(), 16); 

            return !memcmp(NtProofStr.data(), NtProofStr_from_msg, 16);
        }

        // Server check lm response
        bool ntlm_check_lm_response_from_authenticate(array_view_const_u8 hash) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Check LmResponse");
            auto & AuthLmResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
            auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;

            size_t lm_response_size = AuthLmResponse.size(); // should be 24
            if (lm_response_size != 24) {
                return false;
            }
            LMv2_Response response(AuthLmResponse);

            auto userup = UTF16_to_upper(UserName);
            array_md5 ResponseKeyLM = HmacMd5(hash, userup, DomainName);

            auto computed_response = compute_LMv2_Response(ResponseKeyLM, 
                                                          {this->ServerChallenge, 8},
                                                          {response.ClientChallenge, sizeof(response.ClientChallenge)});

            return !memcmp(response.Response, computed_response.data(), SslMd5::DIGEST_LENGTH);
        }

        // server compute Session Base Key
        void ntlm_compute_session_base_key(array_view_const_u8 hash) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Compute Session Base Key");
            auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
            auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
            uint8_t NtProofStr[16] = {};
            memcpy(NtProofStr, AuthNtResponse.data(), 16);

            auto userup = UTF16_to_upper(UserName);
            array_md5 ResponseKeyNT = HmacMd5(hash, userup, DomainName);;

            // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain), NtProofStr)
            this->SessionBaseKey = HmacMd5(ResponseKeyNT, {NtProofStr, sizeof(NtProofStr)});
        }

        // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
        void ntlm_server_build_challenge() {
            uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
            uint32_t const mask = NTLMSSP_REQUEST_TARGET
                                | NTLMSSP_NEGOTIATE_NTLM
                                | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                                | NTLMSSP_NEGOTIATE_UNICODE;
            if ((negoFlag & mask) != mask) {
                LOG(LOG_ERR, "ERROR CHECK NEGO FLAGS");
            }
            this->NegotiateFlags = negoFlag;
        
            this->ntlm_generate_server_challenge();
            memcpy(this->CHALLENGE_MESSAGE.serverChallenge, this->ServerChallenge, 8);
            this->ntlm_generate_timestamp();

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

        void ntlm_server_fetch_hash(uint8_t (&hash)[SslMd4::DIGEST_LENGTH]) {
            // TODO get password hash from DC or find ourself
            // LOG(LOG_INFO, "MARK %u, %u, %u",
            //     this->identity.User.size(),
            //     this->identity.Domain.size(),
            //     this->identity.Password.size());

            auto password_av = this->identity.get_password_utf16_av();
            if (password_av.size() > 0) {
                // password is available
                SslMd4 md4;
                md4.update(password_av);
                md4.final(hash);
            }
        }

        // SERVER PROCEED RESPONSE CHECKING
        SEC_STATUS ntlm_server_proceed_authenticate(const uint8_t (&hash)[16]) {
            if (!this->ntlm_check_nt_response_from_authenticate(this->AUTHENTICATE_MESSAGE, make_array_view(hash))) {
                LOG(LOG_ERR, "NT RESPONSE NOT MATCHING STOP AUTHENTICATE");
                return SEC_E_LOGON_DENIED;
            }
            if (!this->ntlm_check_lm_response_from_authenticate(make_array_view(hash))) {
                LOG(LOG_ERR, "LM RESPONSE NOT MATCHING STOP AUTHENTICATE");
                return SEC_E_LOGON_DENIED;
            }
            // SERVER COMPUTE SHARED KEY WITH CLIENT
            this->ntlm_compute_session_base_key(make_array_view(hash));
            this->ntlm_decrypt_exported_session_key();

            /**
             * Generate client signing key (ClientSigningKey).\n
             * @msdn{cc236711}
             */

            this->ClientSigningKey = Md5(this->ExportedSessionKey, make_array_view(client_sign_magic));

            /**
             * Generate client sealing key (ClientSealingKey).\n
             * @msdn{cc236712}
             */

            this->ClientSealingKey = Md5(this->ExportedSessionKey, make_array_view(client_seal_magic));

            /**
             * Generate server signing key (ServerSigningKey).\n
             * @msdn{cc236711}
             */

            SslMd5 md5sign_server;
            md5sign_server.update(this->ExportedSessionKey);
            md5sign_server.update(make_array_view(server_sign_magic));
            md5sign_server.final(this->ServerSigningKey);

            /**
             * Generate server sealing key (ServerSealingKey).\n
             * @msdn{cc236712}
             */

            SslMd5 md5seal_server;
            md5seal_server.update(this->ExportedSessionKey);
            md5seal_server.update(make_array_view(server_seal_magic));
            md5seal_server.final(this->ServerSealingKey);

            /**
             * Initialize RC4 stream cipher states for sealing.
             */

            this->SendRc4Seal.set_key(make_array_view(this->ServerSealingKey));
            this->RecvRc4Seal.set_key(this->ClientSealingKey);

            // =======================================================

            if (this->UseMIC) {
                this->ntlm_compute_MIC();
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
        void ntlm_SetContextServicePrincipalName(array_view_const_char pszTargetName) {
            // CHECK UTF8 or UTF16 (should store in UTF16)
            if (!pszTargetName.empty()) {
                size_t host_len = UTF8Len(pszTargetName.data());
                this->ServicePrincipalName.init(host_len * 2);
                UTF8toUTF16(pszTargetName, this->ServicePrincipalName.get_data(), host_len * 2);
            }
            else {
                this->ServicePrincipalName.init(0);
            }
        }

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

            this->SavedNegotiateMessage.init(in_stream.get_offset());
            this->SavedNegotiateMessage.copy(in_stream.get_bytes());

            this->state = NTLM_STATE_CHALLENGE;
            return SEC_I_CONTINUE_NEEDED;
        }

        SEC_STATUS write_challenge(Array& output_buffer) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Write Challenge");
            this->ntlm_server_build_challenge();
            StaticOutStream<65535> out_stream;
            EmitNTLMChallengeMessage(out_stream, this->CHALLENGE_MESSAGE);
            output_buffer.init(out_stream.get_offset());
            output_buffer.copy(out_stream.get_bytes());

            this->SavedChallengeMessage.init(out_stream.get_offset());
            this->SavedChallengeMessage.copy(out_stream.get_bytes());

            this->state = NTLM_STATE_AUTHENTICATE;
            return SEC_I_CONTINUE_NEEDED;
        }

        SEC_STATUS read_authenticate(array_view_const_u8 input_buffer) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Read Authenticate");
            InStream in_stream(input_buffer);
            recvNTLMAuthenticateMessage(in_stream, this->AUTHENTICATE_MESSAGE);
            if (this->AUTHENTICATE_MESSAGE.has_mic) {
                this->UseMIC = true;
                this->SavedAuthenticateMessage.init(in_stream.get_offset());
                constexpr std::size_t null_data_sz = 16;
                uint8_t const null_data[null_data_sz]{0u};
                auto const p = in_stream.get_data();
                std::size_t offset = 0u;
                this->SavedAuthenticateMessage.copy({p + offset, this->AUTHENTICATE_MESSAGE.PayloadOffset}, offset);
                offset += this->AUTHENTICATE_MESSAGE.PayloadOffset;
                this->SavedAuthenticateMessage.copy({null_data, null_data_sz}, offset);
                offset += null_data_sz;
                this->SavedAuthenticateMessage.copy({p + offset, in_stream.get_offset() - offset}, offset);
            }

            this->identity.user_init_copy(this->AUTHENTICATE_MESSAGE.UserName.buffer);
            this->identity.domain_init_copy(this->AUTHENTICATE_MESSAGE.DomainName.buffer);

            if (this->identity.is_empty_user_domain()){
                LOG(LOG_ERR, "ANONYMOUS User not allowed");
                return SEC_E_LOGON_DENIED;
            }

            return SEC_I_CONTINUE_NEEDED;
        }

        SEC_STATUS check_authenticate() {
            uint8_t hash[16];
            this->ntlm_server_fetch_hash(hash);
            return this->ntlm_server_proceed_authenticate(hash);
        }
    } ntlm_context;

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext
    // -> only for clients : unused for NTLM server

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;
    SEC_STATUS AcceptSecurityContext(array_view_const_u8 input_buffer, Array& output_buffer)
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::AcceptSecurityContext");

        if (this->ntlm_context.state == NTLM_STATE_INITIAL) {

            this->ntlm_context.state = NTLM_STATE_NEGOTIATE;
            SEC_STATUS status = this->ntlm_context.read_negotiate(input_buffer);
            if (status != SEC_I_CONTINUE_NEEDED) {
                return SEC_E_INVALID_TOKEN;
            }

            if (this->ntlm_context.state == NTLM_STATE_CHALLENGE) {
                return this->ntlm_context.write_challenge(output_buffer);
            }

            return SEC_E_OUT_OF_SEQUENCE;
        }

        if (this->ntlm_context.state == NTLM_STATE_AUTHENTICATE) {
            SEC_STATUS status = this->ntlm_context.read_authenticate(input_buffer);

            if (status == SEC_I_CONTINUE_NEEDED) {
                if (!this->set_password_cb) {
                    return SEC_E_LOGON_DENIED;
                }
                switch (set_password_cb(this->ntlm_context.identity.get_user_utf16_av()
                                       ,this->ntlm_context.identity.get_domain_utf16_av()
                                       ,this->ntlm_context.identity.Password)) {
                    case PasswordCallback::Error:
                        return SEC_E_LOGON_DENIED;
                    case PasswordCallback::Ok:
                        this->ntlm_context.state = NTLM_STATE_WAIT_PASSWORD;
                        break;
                    case PasswordCallback::Wait:
                        this->ntlm_context.state = NTLM_STATE_WAIT_PASSWORD;
                        return SEC_I_LOCAL_LOGON;
                }
            }
        }

        if (this->ntlm_context.state == NTLM_STATE_WAIT_PASSWORD) {
            SEC_STATUS status = this->ntlm_context.check_authenticate();
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

    static void compute_signature(uint8_t* signature, SslRC4& rc4, uint8_t (&digest)[SslMd5::DIGEST_LENGTH], uint32_t SeqNo)
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

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;
    SEC_STATUS EncryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo)
    {
        LOG_IF(this->ntlm_context.verbose, LOG_INFO, "NTLM_SSPI::EncryptMessage");

        // data_out [signature][data_buffer]

        data_out.init(data_in.size() + cbMaxSignature);
        auto message_out = data_out.av().from_at(cbMaxSignature);
        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->compute_hmac_md5(digest, this->ntlm_context.ServerSigningKey, data_in, MessageSeqNo);
        // this->ntlm_context.confidentiality == true
        this->ntlm_context.SendRc4Seal.crypt(data_in.size(), data_in.data(), message_out.data());
        this->compute_signature(data_out.get_data(), this->ntlm_context.SendRc4Seal, digest, MessageSeqNo);
        return SEC_E_OK;
    }

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    SEC_STATUS DecryptMessage(array_view_const_u8 data_in, Array& data_out, unsigned long MessageSeqNo)
    {
        LOG_IF(this->ntlm_context.verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

        if (data_in.size() < cbMaxSignature) {
            return SEC_E_INVALID_TOKEN;
        }

        // data_in [signature][data_buffer]

        auto data_buffer = data_in.from_at(cbMaxSignature);
        data_out.init(data_buffer.size());

        /* Decrypt message using with RC4, result overwrites original buffer */
        // context->confidentiality == true
        this->ntlm_context.RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.get_data());

        uint8_t digest[SslMd5::DIGEST_LENGTH];
        this->compute_hmac_md5(digest, this->ntlm_context.ClientSigningKey.data(), data_out.av(), MessageSeqNo);

        uint8_t expected_signature[16] = {};
        this->compute_signature(
            expected_signature, this->ntlm_context.RecvRc4Seal, digest, MessageSeqNo);

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

public:
    rdpCredsspServerNTLM(array_view_u8 key,
               Random & rand,
               TimeObj & timeobj,
               std::string& extra_message,
               Translation::language_t lang,
               std::function<PasswordCallback(cbytes_view,cbytes_view,Array&)> set_password_cb,
               const bool verbose = false)
        : public_key(key)
        , set_password_cb(set_password_cb)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
        , ntlm_context(rand, timeobj)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::Initialization: NTLM Authentication");
        LOG_IF(this->verbose, LOG_INFO, "this->identity.SetUserFromUtf8(nullptr)");
        LOG_IF(this->verbose, LOG_INFO, "this->SetHostnameFromUtf8(nullptr)");
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
        this->ntlm_context.identity.CopyAuthIdentity({},{},{});
        this->ntlm_context.ntlm_SetContextServicePrincipalName(nullptr);
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

    SEC_STATUS credssp_encrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::encrypt_public_key_echo");
        uint32_t version = this->ts_request.use_version;

        if (version >= 5) {
            if (this->ts_request.clientNonce.isset()){
                this->SavedClientNonce = this->ts_request.clientNonce;
            }
            this->credssp_generate_public_key_hash_server_to_client();
            this->public_key = this->ServerClientHash.av();
        }
        else {
            // if we are server and protocol is 2,3,4
            // then echos the public key +1
            ::ap_integer_increment_le(this->public_key);
        }

        return this->EncryptMessage(
            this->public_key, this->ts_request.pubKeyAuth, this->send_seq_num++);
    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspServer::decrypt_public_key_echo");

        Array Buffer;

        SEC_STATUS const status = this->DecryptMessage(
            this->ts_request.pubKeyAuth.av(), Buffer, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            if (this->ts_request.pubKeyAuth.size() == 0) {
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
            this->public_key = this->ClientServerHash.av();
        }

        array_view_u8 public_key2 = Buffer.av();

        if (public_key2.size() != this->public_key.size()) {
            LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", public_key2.size(), this->public_key.size());
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        if (memcmp(this->public_key.data(), public_key2.data(), public_key.size()) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo");

            LOG(LOG_ERR, "Expected (length = %zu):", this->public_key.size());
            hexdump_c(this->public_key);

            LOG(LOG_ERR, "Actual (length = %zu):", this->public_key.size());
            hexdump_c(public_key2);

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

        const SEC_STATUS status = this->DecryptMessage(
            this->ts_request.authInfo.av(), Buffer, this->recv_seq_num++);

        if (status != SEC_E_OK) {
            return status;
        }

        InStream decrypted_creds(Buffer.av());
        this->ts_credentials.recv(decrypted_creds);

        // hexdump(this->ts_credentials.passCreds.userName,
        //         this->ts_credentials.passCreds.userName_length);
        // hexdump(this->ts_credentials.passCreds.domainName,
        //         this->ts_credentials.passCreds.domainName_length);
        // hexdump(this->ts_credentials.passCreds.password,
        //         this->ts_credentials.passCreds.password_length);

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


        SEC_STATUS status = this->AcceptSecurityContext(this->ts_request.negoTokens.av(), /*output*/this->ts_request.negoTokens);
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

            this->credssp_encrypt_public_key_echo();
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



