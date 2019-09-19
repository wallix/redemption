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

#include "core/RDP/nla/credssp.hpp"
#include "utils/hexdump.hpp"
#include "system/ssl_sha256.hpp"
#include "system/ssl_md4.hpp"
#include "utils/difftimeval.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"

#include "transport/transport.hpp"

#include <vector>


class rdpClientNTLM
{
    static constexpr uint32_t cbMaxSignature = 16;
private:
    int recv_seq_num = 0;

    TSCredentials ts_credentials;

    ClientNonce SavedClientNonce;

    std::vector<uint8_t> PublicKey;
    std::vector<uint8_t> ClientServerHash;
    std::vector<uint8_t> identity_User;
    std::vector<uint8_t> utf16_domain;
    std::vector<uint8_t> identity_Password;

    TimeObj & timeobj;
    Random & rand;

    // NTLMContextClient
    const bool NTLMv2 = true;
    bool UseMIC = true; // like NTLMv2

    SslRC4 SendRc4Seal {};

    //int LmCompatibilityLevel;
    std::vector<uint8_t> Workstation;

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
//    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;

    NtlmVersion version;
    std::vector<uint8_t> SavedNegotiateMessage;

    array_md5 ExportedSessionKey;
    array_md5 ClientSealingKey;
    array_md5 sspi_context_ServerSigningKey;
    bool restricted_admin_mode;

    const bool verbose;

    enum class Res : bool { Err, Ok };

    enum : uint8_t { Start, Loop, Final } client_auth_data_state = Start;

    auto CryptAndSign(SslRC4 & rc4, uint32_t mseqno, bytes_view payload) -> std::vector<uint8_t>
    {
    
        auto ClientSigningKey = ::Md5(this->ExportedSessionKey,
            "session key to client-to-server signing key magic constant\0"_av);

        std::array<uint8_t,4> seqno;
        OutStream stream_sq(seqno);
        stream_sq.out_uint32_le(mseqno);
        
        auto encrypted_pubkey = Rc4CryptVector(rc4, payload);
        
        /* Concatenate version, ciphertext and sequence number to build signature */
        std::array<uint8_t, 16> signature;
        OutStream stream(signature);
        stream.out_uint32_le(1); // version
        array_md5 digest = ::HmacMd5(ClientSigningKey, seqno, payload);
        // We only keep half of MD5 for signature
        stream.out_copy_bytes(Rc4Crypt<8>(rc4, {digest.data(), 8}));
        stream.out_uint32_le(mseqno);
        
        return std::vector<uint8_t>{} << signature << encrypted_pubkey;
    };


public:
    rdpClientNTLM(uint8_t * user,
               bytes_view domain,
               uint8_t * pass,
               uint8_t * hostname,
               const char * target_host,
               array_view_const_u8 public_key,
               const bool restricted_admin_mode,
               Random & rand,
               TimeObj & timeobj,
               const bool verbose = false)
        : PublicKey(public_key.data(), public_key.data()+public_key.size())
        , identity_User(::UTF8toUTF16({user,strlen(reinterpret_cast<char*>(user))}))
        , utf16_domain(::UTF8toUTF16(domain))
        , identity_Password(::UTF8toUTF16({pass,strlen(reinterpret_cast<char*>(pass))}))
        , timeobj(timeobj)
        , rand(rand)
        , Workstation(::UTF8toUTF16({hostname, strlen(char_ptr_cast(hostname))}))
        , restricted_admin_mode(restricted_admin_mode)
        , verbose(verbose)
    {
    }


    void client_authenticate_start(StaticOutStream<65536> & ts_request_emit)
    {
        LOG(LOG_INFO, "Credssp: NTLM Authentication");

        /* receive server response and place in input buffer */
        LOG_IF(this->verbose, LOG_INFO, "NTLM Send Negotiate");
        auto negoTokens = emitNTLMNegotiateMessage();

        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
        auto v = emitTSRequest(6, negoTokens, {}, {}, 0, {}, false);
        ts_request_emit.out_copy_bytes(v);

        this->SavedNegotiateMessage = std::move(negoTokens);
        this->client_auth_data_state = Loop;
        return;
    }


    credssp::State client_authenticate_next(bytes_view in_data, StaticOutStream<65536> & ts_request_emit)
    {
        switch (this->client_auth_data_state)
        {
            case Loop:
            {
                LOG_IF(this->verbose, LOG_INFO, "Client Authentication : Receiving Authentication Token - Challenge");
                TSRequest ts_request = recvTSRequest(in_data);
                NTLMChallengeMessage server_challenge = recvNTLMChallengeMessage(ts_request.negoTokens);

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response from challenge");

                // ntlmv2_compute_response_from_challenge generates :
                // - timestamp
                // - client challenge
                // - NtChallengeResponse
                // - LmChallengeResponse
                // all strings are in unicode utf16

                array_md5 ResponseKeyNT = ::HmacMd5(::Md4(this->identity_Password),::UTF16_to_upper(this->identity_User), this->utf16_domain);
                array_md5 ResponseKeyLM = ::HmacMd5(::Md4(this->identity_Password),::UTF16_to_upper(this->identity_User), this->utf16_domain);

                const timeval tv = this->timeobj.get_time(); // Timestamp
                array_challenge ClientChallenge; // Nonce(8)
                this->rand.random(ClientChallenge.data(), 8);

                // NTLMv2_Client_Challenge = { 0x01, 0x01, Zero(6), Time, ClientChallenge, Zero(4), ServerName , Zero(4) }
                // Zero(n) = { 0x00, ... , 0x00 } n times
                // ServerName = AvPairs received in Challenge message
                auto NTLMv2_Client_Challenge = std::vector<uint8_t>{} 
                     << std::array<uint8_t,8>{1, 1, 0, 0, 0, 0, 0, 0}
                     << out_uint32_le(tv.tv_usec) << out_uint32_le(tv.tv_sec)
                     << bytes_view({ClientChallenge.data(), 8})
                     << std::array<uint8_t,4>{0, 0, 0, 0}
                     << server_challenge.TargetInfo.buffer
                     << std::array<uint8_t,4>{0, 0, 0, 0};

                // NtProofStr = HMAC_MD5(NTOWFv2(password, user, userdomain), Concat(ServerChallenge, NTLMv2_Client_Challenge))
                array_md5 NtProofStr = ::HmacMd5(ResponseKeyNT, server_challenge.serverChallenge, NTLMv2_Client_Challenge);
                // NtChallengeResponse = Concat(NtProofStr, NTLMv2_Client_Challenge)
                auto NtChallengeResponse = std::vector<uint8_t>{} << NtProofStr << NTLMv2_Client_Challenge;

                // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain), Concat(ServerChallenge, ClientChallenge))
                // LmChallengeResponse.ChallengeFromClient = ClientChallenge
                auto LmChallengeResponse = std::vector<uint8_t>{}
                    << compute_LMv2_Response(ResponseKeyLM, server_challenge.serverChallenge, ClientChallenge) 
                    << ClientChallenge;

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: SessionBaseKey");
                // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain), NtProofStr)
                // generate NONCE(16) ExportedSessionKey
                // EncryptedRandomSessionKey = RC4K(SessionBaseKey, NONCE(16))
                array_md5 SessionBaseKey = ::HmacMd5(ResponseKeyNT, NtProofStr);
                this->rand.random(this->ExportedSessionKey.data(), SslMd5::DIGEST_LENGTH);
                auto AuthEncryptedRSK = std::vector<uint8_t>{} << ::Rc4Key(SessionBaseKey, this->ExportedSessionKey);

                // NTLM Signing Key @msdn{cc236711} and Sealing Key @msdn{cc236712}
                this->ClientSealingKey = ::Md5(this->ExportedSessionKey,
                        "session key to client-to-server sealing key magic constant\0"_av);
                this->sspi_context_ServerSigningKey = ::Md5(this->ExportedSessionKey,
                        "session key to server-to-client signing key magic constant\0"_av);

                this->SendRc4Seal.set_key(this->ClientSealingKey);

                uint32_t flags = set_negotiate_flags(
                                    this->NTLMv2, 
                                    this->UseMIC, 
                                    this->Workstation.size() != 0, 
                                    server_challenge.negoFlags.flags & NTLMSSP_NEGOTIATE_KEY_EXCH);

                NTLMAuthenticateMessage AuthenticateMessage;
                AuthenticateMessage.version = this->version;

                //flag |= NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED;
                NtlmVersion ntlm_version{WINDOWS_MAJOR_VERSION_6,WINDOWS_MINOR_VERSION_1,7601,NTLMSSP_REVISION_W2K3};
                size_t mic_offset = 0;
                auto auth_message = emitNTLMAuthenticateMessage(
                    flags,
                    ntlm_version,
                    LmChallengeResponse,
                    NtChallengeResponse,
                    this->utf16_domain,
                    this->identity_User,
                    this->Workstation,
                    AuthEncryptedRSK,
                    this->UseMIC,
                    mic_offset);

                if (this->UseMIC) {
                    array_md5 MessageIntegrityCheck = ::HmacMd5(this->ExportedSessionKey,
                                                            this->SavedNegotiateMessage,
                                                            server_challenge.raw_bytes,
                                                            auth_message);
                    memcpy(auth_message.data()+mic_offset, MessageIntegrityCheck.data(), MessageIntegrityCheck.size()); 
                }

                if (this->verbose) {
                    logNTLMAuthenticateMessage(flags,
                                ntlm_version,
                                LmChallengeResponse,
                                NtChallengeResponse,
                                this->utf16_domain,
                                this->identity_User,
                                this->Workstation,
                                AuthEncryptedRSK,
                                {auth_message.data()+mic_offset,this->UseMIC?16U:0U},
                                auth_message); 
                }
                // have_pub_key_auth = true;

                LOG_IF(this->verbose, LOG_INFO, "rdpClientNTLM::encrypt_public_key_echo");
                uint32_t version = ts_request.use_version;

                // send authentication token to server
                std::vector<uint8_t> v;
                if (version >= 5) {
                    this->rand.random(this->SavedClientNonce.clientNonce.data(), CLIENT_NONCE_LENGTH);
                    this->SavedClientNonce.initialized = true;
                    auto client_to_server_hash = Sha256("CredSSP Client-To-Server Binding Hash\0"_av, 
                                    this->SavedClientNonce.clientNonce,
                                    this->PublicKey);
                    v = emitTSRequest(6, auth_message, {}, 
                                      CryptAndSign(this->SendRc4Seal, 0 /* msg seqno */, client_to_server_hash),
                                      0, this->SavedClientNonce.clientNonce, true);
                }
                else {
                    v = emitTSRequest(6, auth_message, {},
                                      CryptAndSign(this->SendRc4Seal, 0 /* msg seqno */, this->PublicKey),
                                      0, {}, false);
                }
                ts_request_emit.out_copy_bytes(v);

                this->client_auth_data_state = Final;
                return credssp::State::Cont;
            }
            case Final:
            {
                LOG_IF(this->verbose, LOG_INFO, "Client Authentication : Receiving Encrypted PubKey + 1");

                TSRequest ts_request = recvTSRequest(in_data);
                uint32_t error_code = ts_request.error_code;

                if (ts_request.pubKeyAuth.size() < cbMaxSignature) {
                    // Provided Password is probably incorrect
                    LOG(LOG_ERR, "DecryptMessage failure: SEC_E_INVALID_TOKEN");
                    // return SEC_E_INVALID_TOKEN;
                    LOG(LOG_ERR, "Could not verify public key echo!");
                    return credssp::State::Err;
                }

                /* Verify Server Public Key Echo */
                // data_in [signature][data_buffer]

                const unsigned recv_seqno = 0;
                array_view_const_u8 pubkeyAuth_payload = {ts_request.pubKeyAuth.data()+cbMaxSignature, ts_request.pubKeyAuth.size()-cbMaxSignature};
                array_view_const_u8 pubkeyAuth_signature = {ts_request.pubKeyAuth.data(),cbMaxSignature};

                SslRC4 RecvRc4Seal {};
                RecvRc4Seal.set_key(::Md5(this->ExportedSessionKey, "session key to server-to-client sealing key magic constant\0"_av));
                auto pubkeyAuth_encrypted_payload = Rc4CryptVector(RecvRc4Seal, pubkeyAuth_payload); // decrypt message using RC4
                array_md5 digest = ::HmacMd5(this->sspi_context_ServerSigningKey, out_uint32_le(recv_seqno), pubkeyAuth_encrypted_payload);

                // Concatenate version, ciphertext and sequence number to build signature
                auto expected_signature = std::vector<uint8_t>{} 
                    << out_uint32_le(1)                 // version 1
                    << Rc4Crypt<8>(RecvRc4Seal, digest) // RC4-encrypt first 8 bytes of digest (digest is 16 bytes long)
                    << out_uint32_le(recv_seqno);       // sequence number 

                if (!are_buffer_equal(pubkeyAuth_signature, expected_signature)) {
                    LOG(LOG_ERR, "public key echo signature verification failed, something nasty is going on!");
                    LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature);
                    LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                    return credssp::State::Err;
                }

                if (ts_request.use_version < 5) {
                    // if we are client and protocol is 2,3,4, then get the public key minus one
                    ::ap_integer_decrement_le(pubkeyAuth_encrypted_payload);
                    if (!are_buffer_equal(this->PublicKey, pubkeyAuth_encrypted_payload)){
                        LOG(LOG_ERR, "Server's public key echo signature verification failed");
                        LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature);
                        LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                        return credssp::State::Err;
                    }
                }
                else {
                    auto hash = Sha256("CredSSP Server-To-Client Binding Hash\0"_av,
                           this->SavedClientNonce.clientNonce,
                           this->PublicKey);
                    if (!are_buffer_equal(hash, pubkeyAuth_encrypted_payload)){
                        LOG(LOG_ERR, "Server's public key echo signature verification failed");
                        LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature);
                        LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                        return credssp::State::Err;
                    }
                }

                LOG_IF(this->verbose, LOG_INFO, "rdpClientNTLM::encrypt_ts_credentials");

                {
                    std::vector<uint8_t> ts_credentials;
                    if (this->ts_credentials.credType == 1){
                        if (this->restricted_admin_mode) {
                            LOG(LOG_INFO, "Restricted Admin Mode");
                            ts_credentials = emitTSCredentialsPassword({},{},{});
                        }
                        else {
                            ts_credentials = emitTSCredentialsPassword(this->utf16_domain,this->identity_User,this->identity_Password);
                            LOG(LOG_INFO, "TSCredentialsPassword: Domain User Password");
                            hexdump_d(ts_credentials);
                        }
                    }
                    else {
                        // Card Reader Not Supported Yet
                        ts_credentials = emitTSCredentialsSmartCard(
                                        /*pin*/{},/*userHint*/{},/*domainHint*/{},
                                        /*keySpec*/0,/*cardName*/{},/*readerName*/{},
                                        /*containerName*/{}, /*cspName*/{});
                    }
                    
                    unsigned long MessageSeqNo = 1;

                    // authInfo [signature][data_buffer]
                    std::vector<uint8_t> authInfo = CryptAndSign(this->SendRc4Seal, MessageSeqNo, ts_credentials);
                    auto v = emitTSRequest(ts_request.version,
                                           {}, // negoTokens
                                           authInfo,
                                           {}, // pubKeyAuth
                                           error_code,
                                           this->SavedClientNonce.clientNonce,
                                           this->SavedClientNonce.initialized);
                    ts_request_emit.out_copy_bytes(v);

                }
                this->client_auth_data_state = Start;
                return credssp::State::Finish;
            }
            default:
                return credssp::State::Err;
        }
        return credssp::State::Err;
    }

    uint32_t set_negotiate_flags(bool ntlmv2, bool use_mic, bool send_workstation_name, bool negotiate_key_exchange)
    {
        /*
         * from tspkg.dll: 0x00000132
         * ISC_REQ_MUTUAL_AUTH
         * ISC_REQ_CONFIDENTIALITY
         * ISC_REQ_USE_SESSION_KEY
         * ISC_REQ_ALLOCATE_MEMORYSessionBaseKey
         */
        //unsigned long const fContextReq
        //  = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;

        uint32_t flags = 0;
        if (ntlmv2) {
            flags |= NTLMSSP_NEGOTIATE_56;
        }

        if (use_mic) {
            flags |= NTLMSSP_NEGOTIATE_TARGET_INFO;
        }
        if (send_workstation_name) {
            flags |= NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED;
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
};

