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
#include "core/RDP/nla/credssp.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "utils/hexdump.hpp"
#include "utils/translation.hpp"
#include "system/ssl_sha256.hpp"
#include "system/ssl_md4.hpp"
#include "utils/difftimeval.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"

#include "transport/transport.hpp"

#include <vector>


class rdpCredsspClientNTLM
{
    static constexpr uint32_t cbMaxSignature = 16;
private:
    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;
    TSRequest ts_request;

    ClientNonce SavedClientNonce;

    std::vector<uint8_t> PublicKey;
    std::vector<uint8_t> ClientServerHash;
    std::vector<uint8_t> identity_User;
    std::vector<uint8_t> identity_Domain;
    std::vector<uint8_t> identity_Password;

    TimeObj & timeobj;
    Random & rand;

    // NTLMContextClient
    const bool NTLMv2 = true;
    bool UseMIC = true; // like NTLMv2
    NtlmState sspi_context_state = NTLM_STATE_INITIAL;

    SslRC4 SendRc4Seal {};

    uint32_t NegotiateFlags = 0;

    //int LmCompatibilityLevel;
    bool SendWorkstationName = true;
    std::vector<uint8_t> Workstation;

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
    NTLMChallengeMessage CHALLENGE_MESSAGE;
    NTLMNegotiateMessage NEGOTIATE_MESSAGE;
    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;

    NtlmVersion version;
    std::vector<uint8_t> SavedNegotiateMessage;
    std::vector<uint8_t> SavedChallengeMessage;
    std::vector<uint8_t> SavedAuthenticateMessage;

    uint8_t Timestamp[8]{};
    array_md5 SessionBaseKey; 
    array_md5 ExportedSessionKey;
    array_md5 EncryptedRandomSessionKey;
    array_md5 sspi_context_ClientSigningKey;
    array_md5 ClientSealingKey;
    array_md5 sspi_context_ServerSigningKey;
    array_md5 ServerSealingKey;
    array_md5 MessageIntegrityCheck;
    // uint8_t NtProofStr[16];

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    // Inlined

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    bool restricted_admin_mode;

    const char * target_host;
    std::string& extra_message;
    Translation::language_t lang;
    const bool verbose;

    enum class Res : bool { Err, Ok };

    enum : uint8_t { Start, Loop, Final } client_auth_data_state = Loop;
    std::vector<uint8_t> client_auth_data_input_buffer;

    static void sspi_compute_signature(uint8_t* signature, SslRC4& rc4, uint8_t* digest, uint32_t SeqNo)
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
    rdpCredsspClientNTLM(OutTransport transport,
               uint8_t * user,
               uint8_t * domain,
               uint8_t * pass,
               uint8_t * hostname,
               const char * target_host,
               array_view_const_u8 public_key,
               const bool restricted_admin_mode,
               Random & rand,
               TimeObj & timeobj,
               std::string& extra_message,
               Translation::language_t lang,
               const bool verbose = false)
        : ts_request(6) // Credssp Version 6 Supported
        , timeobj(timeobj)
        , rand(rand)
        , restricted_admin_mode(restricted_admin_mode)
        , target_host(target_host)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
    {
        memset(this->MessageIntegrityCheck.data(), 0x00, this->MessageIntegrityCheck.size());

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::Initialization");
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::set_credentials");
        this->identity_User = ::UTF8toUTF16({user,strlen(reinterpret_cast<char*>(user))});
        this->identity_Domain = ::UTF8toUTF16({domain,strlen(reinterpret_cast<char*>(domain))});
        this->identity_Password = ::UTF8toUTF16({pass,strlen(reinterpret_cast<char*>(pass))});

        size_t length_hostname = strlen(char_ptr_cast(hostname));

        this->Workstation = ::UTF8toUTF16({hostname, length_hostname});
        this->SendWorkstationName = length_hostname != 0;

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::client_authenticate");

        // ============================================
        /* Get Public Key From TLS Layer and hostname */
        // ============================================

        this->PublicKey.assign(public_key.data(), public_key.data()+public_key.size());

        LOG(LOG_INFO, "Credssp: NTLM Authentication");
       
        /*
         * from tspkg.dll: 0x00000132
         * ISC_REQ_MUTUAL_AUTH
         * ISC_REQ_CONFIDENTIALITY
         * ISC_REQ_USE_SESSION_KEY
         * ISC_REQ_ALLOCATE_MEMORY
         */
        //unsigned long const fContextReq
        //  = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

        /* receive server response and place in input buffer */
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Write Negotiate");

        this->NegotiateFlags = set_negotiate_flags(this->NTLMv2, false, false, true)
            | (this->NTLMv2) * (NTLMSSP_NEGOTIATE_LM_KEY|NTLMSSP_NEGOTIATE_OEM);

        this->version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
        this->version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
        this->version.ProductBuild        = 7601;
        this->version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;

        this->NEGOTIATE_MESSAGE.version = this->version;


        this->NEGOTIATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
            this->NEGOTIATE_MESSAGE.Workstation.buffer = this->Workstation;
        }

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED) {
            this->AUTHENTICATE_MESSAGE.DomainName.buffer = this->identity_Domain;
        }

        StaticOutStream<65535> out_stream;
        emitNTLMNegotiateMessage(out_stream, this->NEGOTIATE_MESSAGE);
        this->SavedNegotiateMessage.assign(out_stream.get_bytes().data(), out_stream.get_bytes().data()+out_stream.get_offset());

        this->ts_request.negoTokens.assign(out_stream.get_bytes().data(),out_stream.get_bytes().data()+out_stream.get_bytes().size());

        /* send authentication token to server */
        if (this->ts_request.negoTokens.size() > 0) {
            if (this->ts_request.negoTokens.size() > 0){
                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
            }

            StaticOutStream<65536> ts_request_emit;
            this->ts_request.emit(ts_request_emit);
            transport.get_transport().send(ts_request_emit.get_bytes());
        }
        this->sspi_context_state = NTLM_STATE_CHALLENGE;
    }


    uint32_t set_negotiate_flags(bool ntlmv2, bool use_mic, bool send_workstation_name, bool negotiate_key_exchange)
    {
        uint32_t flags = 0;
        if (ntlmv2) {
            flags |= NTLMSSP_NEGOTIATE_56;
        }

        if (use_mic) {
            flags |= NTLMSSP_NEGOTIATE_TARGET_INFO;
        }
        if (send_workstation_name) {
            flags |= NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED;
        }
        flags |= NTLMSSP_NEGOTIATE_SEAL;
        if (negotiate_key_exchange) {
            flags |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        }
        flags |= (NTLMSSP_NEGOTIATE_128
              | NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
              | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
              | NTLMSSP_NEGOTIATE_NTLM
              | NTLMSSP_NEGOTIATE_SIGN
              | NTLMSSP_REQUEST_TARGET
              | NTLMSSP_NEGOTIATE_UNICODE
              | NTLMSSP_NEGOTIATE_VERSION);
        return flags;
    }

    credssp::State credssp_client_authenticate_next(InStream & in_stream, StaticOutStream<65536> & ts_request_emit)
    {
        switch (this->client_auth_data_state)
        {
            case Loop:
            {
                this->ts_request.recv(in_stream);

                // #ifdef WITH_DEBUG_CREDSSP
                // LOG(LOG_ERR, "Receiving Authentication Token (%d)", (int) this->ts_request.negoTokens.cbBuffer);
                // hexdump_c(this->ts_request.negoTokens.pvBuffer, this->ts_request.negoTokens.cbBuffer);
                // #endif
                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Authentication Token");
                this->client_auth_data_input_buffer.assign(this->ts_request.negoTokens.data(),
                                                           this->ts_request.negoTokens.data()+this->ts_request.negoTokens.size());
                /*
                 * from tspkg.dll: 0x00000132
                 * ISC_REQ_MUTUAL_AUTH
                 * ISC_REQ_CONFIDENTIALITY
                 * ISC_REQ_USE_SESSION_KEY
                 * ISC_REQ_ALLOCATE_MEMORY
                 */
                //unsigned long const fContextReq
                //  = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

                array_view_const_u8 input_buffer =  this->client_auth_data_input_buffer;
                
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Read Challenge");
                InStream in_stream(input_buffer);
                RecvNTLMChallengeMessage(in_stream, this->CHALLENGE_MESSAGE);
                this->SavedChallengeMessage.assign(in_stream.get_consumed_bytes().data(),in_stream.get_consumed_bytes().data()+in_stream.get_offset());

                this->sspi_context_state = NTLM_STATE_AUTHENTICATE;

                
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Write Authenticate");
                
                // client method
                // ntlmv2_compute_response_from_challenge generates :
                // - timestamp
                // - client challenge
                // - NtChallengeResponse
                // - LmChallengeResponse
                // all strings are in unicode utf16

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response from challenge");
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2");
                array_md4 md4password = ::Md4(this->identity_Password);
                auto userNameUppercase = ::UTF16_to_upper(this->identity_User);
                array_md5 ResponseKeyNT = ::HmacMd5(md4password,userNameUppercase, this->identity_Domain);

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2");

                array_md4 md4password_b = ::Md4(this->identity_Password);
                auto userNameUppercase_b = ::UTF16_to_upper(this->identity_User);
                array_md5 ResponseKeyLM = ::HmacMd5(md4password_b,userNameUppercase_b, this->identity_Domain);

                // struct NTLMv2_Client_Challenge = temp
                // temp = { 0x01, 0x01, Z(6), Time, ClientChallenge, Z(4), ServerName , Z(4) }
                // Z(n) = { 0x00, ... , 0x00 } n times
                // ServerName = AvPairs received in Challenge message
                auto & AvPairsStream = this->CHALLENGE_MESSAGE.TargetInfo.buffer;
                // BStream AvPairsStream;
                // this->CHALLENGE_MESSAGE.AvPairList.emit(AvPairsStream);

                std::vector<uint8_t> temp;
                for (auto x: {1, 1, 0, 0, 0, 0, 0, 0}){
                    temp.push_back(x);
                }

                // compute ClientChallenge (nonce(8))
                // /* ClientChallenge is used in computation of LMv2 and NTLMv2 responses */

                // compute ClientTimeStamp
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient TimeStamp");

                const timeval tv = this->timeobj.get_time();
                push_back_array(temp, out_uint32_le(tv.tv_usec));
                push_back_array(temp, out_uint32_le(tv.tv_sec));

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Generate Client Challenge");
                array_challenge ClientChallenge;
                this->rand.random(ClientChallenge.data(), 8);
                push_back_array(temp, {ClientChallenge.data(), 8});
                push_back_array(temp, out_uint32_le(0));
                push_back_array(temp, {AvPairsStream.data(), AvPairsStream.size()});
                push_back_array(temp, out_uint32_le(0));
                // NtProofStr = HMAC_MD5(NTOWFv2(password, user, userdomain), Concat(ServerChallenge, temp))

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: NtProofStr");

                array_challenge ServerChallenge = this->CHALLENGE_MESSAGE.serverChallenge;

                array_md5 NtProofStr = ::HmacMd5(make_array_view(ResponseKeyNT),ServerChallenge,temp);


                // NtChallengeResponse = Concat(NtProofStr, temp)

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: NtChallengeResponse");
                auto & NtChallengeResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
                // BStream & NtChallengeResponse = this->BuffNtChallengeResponse;
                NtChallengeResponse.assign(NtProofStr.data(), NtProofStr.data()+NtProofStr.size());
                NtChallengeResponse.insert(std::end(NtChallengeResponse), temp.data(), temp.data() + temp.size());

                LOG_IF(this->verbose, LOG_INFO, "Compute response: NtChallengeResponse Ready");

                // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
                //                                         Concat(ServerChallenge, ClientChallenge))
                // LmChallengeResponse.ChallengeFromClient = ClientChallenge
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: LmChallengeResponse");
                
                auto response = compute_LMv2_Response(ResponseKeyLM, ServerChallenge, ClientChallenge);
                this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer.assign(response.data(), response.data()+response.size());
                push_back_array(this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer, ClientChallenge);

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: SessionBaseKey");
                // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain), NtProofStr)
                this->SessionBaseKey = ::HmacMd5(make_array_view(ResponseKeyNT), NtProofStr);

                // EncryptedRandomSessionKey = RC4K(KeyExchangeKey, ExportedSessionKey)
                // ExportedSessionKey = NONCE(16) (random 16bytes number)
                // KeyExchangeKey = SessionBaseKey
                // EncryptedRandomSessionKey = RC4K(SessionBaseKey, NONCE(16))

                // generate NONCE(16) exportedsessionkey
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Encrypt RandomSessionKey");
                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Generate Exported Session Key");
                this->rand.random(this->ExportedSessionKey.data(), SslMd5::DIGEST_LENGTH);
                this->EncryptedRandomSessionKey = ::Rc4Key(this->SessionBaseKey, this->ExportedSessionKey);

                auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
                AuthEncryptedRSK.assign(this->EncryptedRandomSessionKey.data(), this->EncryptedRandomSessionKey.data()+16);

                // NTLM Signing Key @msdn{cc236711} and Sealing Key @msdn{cc236712}
                this->sspi_context_ClientSigningKey = ::Md5(this->ExportedSessionKey,
                        "session key to client-to-server signing key magic constant\0"_av);
                this->ClientSealingKey = ::Md5(this->ExportedSessionKey,
                        "session key to client-to-server sealing key magic constant\0"_av);
                this->sspi_context_ServerSigningKey = ::Md5(this->ExportedSessionKey,
                        "session key to server-to-client signing key magic constant\0"_av);
                this->ServerSealingKey = ::Md5(this->ExportedSessionKey,
                        "session key to server-to-client sealing key magic constant\0"_av);

                this->SendRc4Seal.set_key(this->ClientSealingKey);
                

                uint32_t flags = set_negotiate_flags(
                                    this->NTLMv2, 
                                    this->UseMIC, 
                                    this->SendWorkstationName, 
                                    this->CHALLENGE_MESSAGE.negoFlags.flags & NTLMSSP_NEGOTIATE_KEY_EXCH);

                this->AUTHENTICATE_MESSAGE.negoFlags.flags = flags;

                this->version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
                this->version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
                this->version.ProductBuild        = 7601;
                this->version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
                this->AUTHENTICATE_MESSAGE.version = this->version;

                if (!(flags & NTLMSSP_NEGOTIATE_KEY_EXCH)) {
                    // If flag is not set, encryted session key buffer is not send
                    this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer.clear();
                }
                if (flags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
                    this->AUTHENTICATE_MESSAGE.Workstation.buffer = this->Workstation;
                }

                //flag |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
                this->AUTHENTICATE_MESSAGE.DomainName.buffer = this->identity_Domain;
                this->AUTHENTICATE_MESSAGE.UserName.buffer = this->identity_User;

                StaticOutStream<65535> out_stream;
                if (this->UseMIC) {
                    this->AUTHENTICATE_MESSAGE.ignore_mic = true;
                    emitNTLMAuthenticateMessage(out_stream, this->AUTHENTICATE_MESSAGE);
                    this->AUTHENTICATE_MESSAGE.ignore_mic = false;

                    this->SavedAuthenticateMessage.assign(out_stream.get_bytes().data(),out_stream.get_bytes().data()+out_stream.get_offset());

                    this->MessageIntegrityCheck = ::HmacMd5(this->ExportedSessionKey, 
                                                            this->SavedNegotiateMessage,
                                                            this->SavedChallengeMessage,
                                                            this->SavedAuthenticateMessage);

                    memcpy(this->AUTHENTICATE_MESSAGE.MIC, this->MessageIntegrityCheck.data(), this->MessageIntegrityCheck.size());
                }
                out_stream.rewind();
                this->AUTHENTICATE_MESSAGE.ignore_mic = false;
                emitNTLMAuthenticateMessage(out_stream, this->AUTHENTICATE_MESSAGE);
                auto out_stream_bytes = out_stream.get_bytes();
                this->ts_request.negoTokens.assign(out_stream_bytes.data(),out_stream_bytes.data()+out_stream_bytes.size());
                if (this->verbose) {
                    logNTLMAuthenticateMessage(this->AUTHENTICATE_MESSAGE);
                }
                this->sspi_context_state = NTLM_STATE_FINAL;

                // have_pub_key_auth = true;

                LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::encrypt_public_key_echo");
                uint32_t version = this->ts_request.use_version;

                array_view_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};
                if (version >= 5) {
                    LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp generate client nonce");
                    this->rand.random(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH);
                    this->SavedClientNonce.initialized = true;
                    this->ts_request.clientNonce = this->SavedClientNonce;
                    
                    LOG(LOG_INFO, "rdpCredsspClientNTLM::generate credssp public key hash (client->server)");
                    SslSha256 sha256;
                    uint8_t hash[SslSha256::DIGEST_LENGTH];
                    sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
                    sha256.update(make_array_view(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH));

                    sha256.update({this->PublicKey.data(),this->PublicKey.size()});
                    sha256.final(hash);
                    this->ClientServerHash.assign(hash, hash+sizeof(hash));
                    public_key = {this->ClientServerHash.data(), this->ClientServerHash.size()};
                }

                unsigned long MessageSeqNo = this->send_seq_num++;
                // data_out [signature][data_buffer]
                std::vector<uint8_t> data_out(public_key.size() + cbMaxSignature);
                std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
                array_md5 digest = ::HmacMd5(this->sspi_context_ClientSigningKey, seqno, public_key);
                
                this->SendRc4Seal.crypt(public_key.size(), public_key.data(), data_out.data()+cbMaxSignature);
                this->sspi_compute_signature(data_out.data(), this->SendRc4Seal, digest.data(), MessageSeqNo);
                this->ts_request.pubKeyAuth.assign(data_out.data(),data_out.data()+data_out.size());

                /* send authentication token to server */
                if (this->ts_request.negoTokens.size() > 0){
                    LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
                }

                LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::send");
                this->ts_request.emit(ts_request_emit);

                this->client_auth_data_state = Final;
                return credssp::State::Cont;
            }
            case Final:
            {
                /* Encrypted Public Key +1 */
                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Encrypted PubKey + 1");

                this->ts_request.recv(in_stream);

                if (this->ts_request.pubKeyAuth.size() < cbMaxSignature) {
                    if (this->ts_request.pubKeyAuth.size() == 0) {
                        // report_error
                        this->extra_message = " ";
                        this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                        LOG(LOG_INFO, "Provided login/password is probably incorrect.");
                    }
                    LOG(LOG_ERR, "DecryptMessage failure: SEC_E_INVALID_TOKEN");
                    // return SEC_E_INVALID_TOKEN;
                    LOG(LOG_ERR, "Could not verify public key echo!");
                    return credssp::State::Err;
                }

                /* Verify Server Public Key Echo */
                unsigned long MessageSeqNo = this->recv_seq_num++;
                
                // data_in [signature][data_buffer]

                array_view_const_u8 pubkeyAuth_payload = {this->ts_request.pubKeyAuth.data()+cbMaxSignature, this->ts_request.pubKeyAuth.size()-cbMaxSignature};
                array_view_const_u8 pubkeyAuth_signature = {this->ts_request.pubKeyAuth.data(),cbMaxSignature};

                SslRC4 RecvRc4Seal {};
                RecvRc4Seal.set_key(this->ServerSealingKey);
                // decrypt message using RC4
                auto pubkeyAuth_encrypted_payload = Rc4CryptVector(RecvRc4Seal, pubkeyAuth_payload);
               
                std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
                array_md5 digest = ::HmacMd5(this->sspi_context_ServerSigningKey, seqno, pubkeyAuth_encrypted_payload);
                /* RC4-encrypt first 8 bytes of digest (digest is 16 bytes long)*/
                auto checksum = Rc4Crypt<8>(RecvRc4Seal, digest);
                /* Concatenate version, ciphertext and sequence number to build signature */
                StaticOutStream<16> expected_signature;
                expected_signature.out_uint32_le(1); // version 1
                expected_signature.out_copy_bytes(checksum);
                expected_signature.out_uint32_le(MessageSeqNo);

                if (!are_buffer_equal(pubkeyAuth_signature, expected_signature.get_bytes())) {
                    LOG(LOG_ERR, "public key echo signature verification failed, something nasty is going on!");
                    LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature.get_bytes());
                    LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                    return credssp::State::Err;
                }

                if (this->ts_request.use_version < 5) {
                    // if we are client and protocol is 2,3,4, then get the public key minus one
                    ::ap_integer_decrement_le(pubkeyAuth_encrypted_payload);
                    if (!are_buffer_equal(this->PublicKey, pubkeyAuth_encrypted_payload)){
                        LOG(LOG_ERR, "Server's public key echo signature verification failed");
                        LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature.get_bytes());
                        LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                        return credssp::State::Err;
                    }
                }
                else {
                    auto hash = Sha256("CredSSP Server-To-Client Binding Hash\0"_av,
                           make_array_view(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH),
                           this->PublicKey);
                    if (!are_buffer_equal(hash, pubkeyAuth_encrypted_payload)){
                        LOG(LOG_ERR, "Server's public key echo signature verification failed");
                        LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature.get_bytes());
                        LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                        return credssp::State::Err;
                    }
                }

                /* Send encrypted credentials */
                LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::encrypt_ts_credentials");
                if (this->restricted_admin_mode) {
                    LOG(LOG_INFO, "Restricted Admin Mode");
                    this->ts_credentials.set_credentials_from_av({},{},{});
                }
                else {
                    this->ts_credentials.set_credentials_from_av(this->identity_Domain, this->identity_User, this->identity_Password);
                }

                {
                    StaticOutStream<65536> ts_credentials_send;
                    this->ts_credentials.emit(ts_credentials_send);
                    array_view_const_u8 data_in = {ts_credentials_send.get_data(), ts_credentials_send.get_offset()};
                    unsigned long MessageSeqNo = this->send_seq_num++;

                    // data_out [signature][data_buffer]
                    std::vector<uint8_t> data_out(data_in.size() + cbMaxSignature, 0);
                    std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
                    array_md5 digest = ::HmacMd5(this->sspi_context_ClientSigningKey, seqno, data_in);

                    this->SendRc4Seal.crypt(data_in.size(), data_in.data(), data_out.data()+cbMaxSignature);
                    this->sspi_compute_signature(data_out.data(), this->SendRc4Seal, digest.data(), MessageSeqNo);
                    
                    this->ts_request.negoTokens.clear();
                    this->ts_request.pubKeyAuth.clear();
                    this->ts_request.error_code = 0;

                    this->ts_request.authInfo.assign(data_out.data(),data_out.data()+data_out.size());
                    this->ts_request.clientNonce = this->SavedClientNonce;
                    this->ts_request.emit(ts_request_emit);
                }
                this->client_auth_data_state = Start;
                return credssp::State::Finish;
            }
            default:
                return credssp::State::Err;
        }
        return credssp::State::Err;
    }
};

