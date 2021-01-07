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
#include "utils/genrandom.hpp"
#include "utils/timebase.hpp"
#include "utils/difftimeval.hpp"
#include "utils/utf.hpp"

#include "core/RDP/nla/ntlm_message.hpp"

#include "transport/transport.hpp"

#include <vector>


class rdpClientNTLM
{
    static constexpr uint32_t cbMaxSignature = 16;

private:
    TSCredentials ts_credentials;

    ClientNonce SavedClientNonce;

    std::vector<uint8_t> PublicKey;
    std::vector<uint8_t> ClientServerHash;
    std::vector<uint8_t> utf16_user;
    std::vector<uint8_t> utf16_domain;
    std::vector<uint8_t> identity_Password;

    const TimeBase & time_base;
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
    std::vector<uint8_t> savedNegotiateMessage;

    array_md5 ExportedSessionKey;
    array_md5 ClientSealingKey;
    array_md5 sspi_context_ServerSigningKey;
    bool restricted_admin_mode;

    const bool credssp_verbose;
    const bool verbose;

    enum class Res : bool { Err, Ok };

    public:
    credssp::State state;

    private:
    enum : uint8_t { Start, Loop, Final } client_auth_data_state = Start;

    auto CryptAndSign(SslRC4 & rc4, uint32_t mseqno, bytes_view payload) -> std::vector<uint8_t>
    {
        auto ClientSigningKey = ::Md5(this->ExportedSessionKey,
            "session key to client-to-server signing key magic constant\0"_av);

        auto seqno = std::vector<uint8_t>{} << out_uint32_le(mseqno);
        auto encrypted_pubkey = Rc4CryptVector(rc4, payload);
        array_md5 digest = ::HmacMd5(ClientSigningKey, seqno, payload);
        auto ciphertext = Rc4Crypt<8>(rc4, {digest.data(), 8});
        /* Concatenate version, ciphertext and sequence number to build signature */
        return std::vector<uint8_t>{} << out_uint32_le(1)
                                      << ciphertext
                                      << out_uint32_le(mseqno)
                                      << encrypted_pubkey;
    }

public:
    rdpClientNTLM(bytes_view user,
               bytes_view domain,
               uint8_t * pass,
               const char * hostname,
               u8_array_view public_key,
               const bool restricted_admin_mode,
               Random & rand,
               const TimeBase & time_base,
               const bool credssp_verbose,
               const bool verbose)
        : PublicKey(public_key.data(), public_key.data()+public_key.size())
        , utf16_user(::UTF8toUTF16(user))
        , utf16_domain(::UTF8toUTF16(domain))
        , identity_Password(::UTF8toUTF16({pass,strlen(reinterpret_cast<char*>(pass))}))
        , time_base(time_base)
        , rand(rand)
        , Workstation(::UTF8toUTF16({hostname, strlen(hostname)}))
        , restricted_admin_mode(restricted_admin_mode)
        , credssp_verbose(credssp_verbose)
        , verbose(verbose)
    {
        if (this->verbose){
            LOG(LOG_INFO, "rdpNTLMClient datas: (restricted_admin=%s)", (restricted_admin_mode?"true":"false"));
            hexdump_d(this->PublicKey);
            hexdump_d(this->utf16_user);
            hexdump_d(this->utf16_domain);
            hexdump_d(this->identity_Password);
            hexdump_d(this->Workstation);
            LOG(LOG_INFO, "rdpNTLMClient datas done ==============");
        }
    }


    std::vector<uint8_t> authenticate_start()
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLM Authentication : Negotiate - sending Authentication Token");
        this->savedNegotiateMessage = emitNTLMNegotiateMessage();
        this->client_auth_data_state = Loop;
        return emitTSRequest(6, this->savedNegotiateMessage, {}, {}, 0, {}, false, this->credssp_verbose);
        LOG_IF(this->verbose, LOG_INFO, "NTLM Authentication : Negotiate - sending Authentication Token done");
    }


    std::vector<uint8_t> authenticate_next(bytes_view in_data)
    {
        LOG_IF(this->verbose, LOG_INFO, "authenticate_next");
        switch (this->client_auth_data_state)
        {
            case Loop:
            {
                LOG_IF(this->verbose, LOG_INFO, "Client Authentication : Receiving Authentication Token - Challenge");
                TSRequest ts_request = recvTSRequest(in_data, this->credssp_verbose);
                NTLMChallengeMessage server_challenge = recvNTLMChallengeMessage(ts_request.negoTokens);

                LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response from challenge");

                // ntlmv2_compute_response_from_challenge generates :
                // - timestamp
                // - client challenge
                // - NtChallengeResponse
                // - LmChallengeResponse
                // all strings are in unicode utf16

                array_md5 ResponseKeyNT = ::HmacMd5(::Md4(this->identity_Password),::UTF16_to_upper(this->utf16_user), this->utf16_domain);
                array_md5 ResponseKeyLM = ::HmacMd5(::Md4(this->identity_Password),::UTF16_to_upper(this->utf16_user), this->utf16_domain);

                const timeval tv = this->time_base.get_current_time(); // Timestamp
                array_challenge ClientChallenge; // Nonce(8)
                this->rand.random(ClientChallenge.data(), 8);
                if (this->verbose){
                    LOG(LOG_INFO, "Time Stamp (%ld, %ld)", tv.tv_sec, tv.tv_usec);
                    LOG(LOG_INFO, "Client Random Challenge {0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x}",
                        ClientChallenge[0], ClientChallenge[1], ClientChallenge[2], ClientChallenge[3],
                        ClientChallenge[4], ClientChallenge[5], ClientChallenge[6], ClientChallenge[7]
                    );
                }
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
                if (this->verbose){
                    LOG(LOG_INFO, "Random ExportedSessionKey");
                    hexdump_d(this->ExportedSessionKey.data(), SslMd5::DIGEST_LENGTH);
                }
                auto AuthEncryptedRSK = std::vector<uint8_t>{} << ::Rc4Key(SessionBaseKey, this->ExportedSessionKey);

                // NTLM Signing Key @msdn{cc236711} and Sealing Key @msdn{cc236712}
                this->ClientSealingKey = ::Md5(this->ExportedSessionKey,
                        "session key to client-to-server sealing key magic constant\0"_av);
                this->sspi_context_ServerSigningKey = ::Md5(this->ExportedSessionKey,
                        "session key to server-to-client signing key magic constant\0"_av);

                this->SendRc4Seal.set_key(this->ClientSealingKey);

                auto flag_if = [](uint32_t flag, bool cond) -> uint32_t { return flag * cond; };

                uint32_t flags = flag_if(NTLMSSP_NEGOTIATE_56, this->NTLMv2)
                               | flag_if(NTLMSSP_NEGOTIATE_TARGET_INFO, this->UseMIC)
                               | flag_if(NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED, this->Workstation.size() != 0)
                               | flag_if(NTLMSSP_NEGOTIATE_KEY_EXCH ,server_challenge.negoFlags.flags & NTLMSSP_NEGOTIATE_KEY_EXCH)
                               | NTLMSSP_NEGOTIATE_SEAL
                               | NTLMSSP_NEGOTIATE_128
                               | NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
                               | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                               | NTLMSSP_NEGOTIATE_NTLM
                               | NTLMSSP_NEGOTIATE_SIGN
                               | NTLMSSP_REQUEST_TARGET
                               | NTLMSSP_NEGOTIATE_UNICODE
                               | NTLMSSP_NEGOTIATE_VERSION;

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
                    this->utf16_user,
                    this->Workstation,
                    AuthEncryptedRSK,
                    this->UseMIC,
                    mic_offset);

                if (this->UseMIC) {
                    array_md5 MessageIntegrityCheck = ::HmacMd5(this->ExportedSessionKey,
                                                            this->savedNegotiateMessage,
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
                                this->utf16_user,
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
                    if (this->verbose){
                        LOG(LOG_INFO, "Random Client Nonce");
                        hexdump_d(this->SavedClientNonce.clientNonce.data(), CLIENT_NONCE_LENGTH);
                     }

                    this->SavedClientNonce.initialized = true;
                    auto client_to_server_hash = Sha256("CredSSP Client-To-Server Binding Hash\0"_av,
                                    this->SavedClientNonce.clientNonce,
                                    this->PublicKey);
                    v = emitTSRequest(6, auth_message, {},
                                      CryptAndSign(this->SendRc4Seal, 0 /* msg seqno */, client_to_server_hash),
                                      0, this->SavedClientNonce.clientNonce, true, this->credssp_verbose);
                }
                else {
                    v = emitTSRequest(6, auth_message, {},
                                      CryptAndSign(this->SendRc4Seal, 0 /* msg seqno */, this->PublicKey),
                                      0, {}, false, this->credssp_verbose);
                }

                this->client_auth_data_state = Final;
                this->state = credssp::State::Cont;
                return v;
            }
            case Final:
            {
                LOG_IF(this->verbose, LOG_INFO, "Client Authentication : Receiving Encrypted PubKey + 1");

                TSRequest ts_request = recvTSRequest(in_data, this->credssp_verbose);
                uint32_t error_code = ts_request.error_code;

                if (ts_request.pubKeyAuth.size() < cbMaxSignature) {
                    // Provided Password is probably incorrect
                    LOG(LOG_ERR, "DecryptMessage failure: SEC_E_INVALID_TOKEN");
                    // return SEC_E_INVALID_TOKEN;
                    LOG(LOG_ERR, "Could not verify public key echo!");
                    this->state = credssp::State::Err;
                    return {};
                }

                /* Verify Server Public Key Echo */
                // data_in [signature][data_buffer]

                const unsigned recv_seqno = 0;
                u8_array_view pubkeyAuth_payload = {ts_request.pubKeyAuth.data()+cbMaxSignature, ts_request.pubKeyAuth.size()-cbMaxSignature};
                u8_array_view pubkeyAuth_signature = {ts_request.pubKeyAuth.data(),cbMaxSignature};

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
                    this->state = credssp::State::Err;
                    return {};
                }

                if (ts_request.use_version < 5) {
                    // if we are client and protocol is 2,3,4, then get the public key minus one
                    ::ap_integer_decrement_le(make_writable_array_view(pubkeyAuth_encrypted_payload));
                    if (!are_buffer_equal(this->PublicKey, pubkeyAuth_encrypted_payload)){
                        LOG(LOG_ERR, "Server's public key echo signature verification failed");
                        LOG(LOG_ERR, "Expected Signature:"); hexdump_c(expected_signature);
                        LOG(LOG_ERR, "Actual Signature:"); hexdump_c(pubkeyAuth_signature);
                        this->state = credssp::State::Err;
                        return {};
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
                        this->state = credssp::State::Err;
                        return {};
                    }
                }

                LOG_IF(this->verbose, LOG_INFO, "rdpClientNTLM::encrypt_ts_credentials");

                std::vector<uint8_t> ts_credentials;
                if (this->ts_credentials.credType == 1){
                    if (this->restricted_admin_mode) {
                        LOG(LOG_INFO, "Restricted Admin Mode");
                        ts_credentials = emitTSCredentialsPassword({},{},{}, this->credssp_verbose);
                    }
                    else {
                        ts_credentials = emitTSCredentialsPassword(this->utf16_domain,this->utf16_user,this->identity_Password, this->credssp_verbose);
                        LOG(LOG_INFO, "TSCredentialsPassword: Domain User Password");
//                            hexdump_d(ts_credentials);
                    }
                }
                else {
                    // Card Reader Not Supported Yet
                    ts_credentials = emitTSCredentialsSmartCard(
                                    /*pin*/{},/*userHint*/{},/*domainHint*/{},
                                    /*keySpec*/0,/*cardName*/{},/*readerName*/{},
                                    /*containerName*/{}, /*cspName*/{}, this->credssp_verbose);
                }

                // authInfo [signature][data_buffer]
                std::vector<uint8_t> authInfo = CryptAndSign(this->SendRc4Seal, 1 /* seqno */, ts_credentials);
                auto v = emitTSRequest(ts_request.version,
                                       {}, // negoTokens
                                       authInfo,
                                       {}, // pubKeyAuth
                                       error_code,
                                       this->SavedClientNonce.clientNonce,
                                       this->SavedClientNonce.initialized, this->credssp_verbose);

                this->client_auth_data_state = Start;
                this->state = credssp::State::Finish;
                return v;
            }
            default:
            break;
        }
        this->state = credssp::State::Err;
        return {};
    }
};

