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
#include "core/RDP/nla/ntlm/ntlm.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "utils/hexdump.hpp"
#include "utils/translation.hpp"
#include "system/ssl_sha256.hpp"

#ifndef __EMSCRIPTEN__
#include "core/RDP/nla/kerberos/kerberos.hpp"
#endif

#include "transport/transport.hpp"

#include <vector>


static inline std::vector<uint8_t> UTF16_to_upper(array_view_const_u8 name)
{
    std::vector<uint8_t> result(name.data(), name.data()+name.size());
    ::UTF16Upper(result.data(), result.size());
    return result;
}


using array_md4 = std::array<uint8_t, SslMd4::DIGEST_LENGTH>;
static inline array_md4 Md4(array_view_const_u8 data)
{
    array_md4 result;
    SslMd4 md4;
    md4.update(data);
    md4.unchecked_final(result.data());
    return result;
}

using array_md5 = std::array<uint8_t, SslMd5::DIGEST_LENGTH>;
static inline array_md5 Rc4Key(array_view_const_u8 key, array_md5 plaintext)
{
    array_md5 cyphertext;
    SslRC4 rc4;
    rc4.set_key(key);
    rc4.crypt(plaintext.size(), plaintext.data(), cyphertext.data());
    return cyphertext;
}


static inline array_md5 Md5(array_view_const_u8 data)
{
    array_md5 result;
    SslMd5 md5;
    md5.update(data);
    md5.unchecked_final(result.data());
    return result;
}

static inline array_md5 Md5(array_view_const_u8 data1, const_bytes_view data2)
{
    array_md5 result;
    SslMd5 md5;
    md5.update(data1);
    md5.update(data2);
    md5.unchecked_final(result.data());
    return result;
}


static inline array_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data)
{
    array_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data);
    hmac_md5.unchecked_final(result.data());
    return result;
}

static inline array_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data1, array_view_const_u8 data2)
{
    array_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data1);
    hmac_md5.update(data2);
    hmac_md5.unchecked_final(result.data());
    return result;
}

static inline array_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data1, array_view_const_u8 data2, array_view_const_u8 data3)
{
    array_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data1);
    hmac_md5.update(data2);
    hmac_md5.update(data3);
    hmac_md5.unchecked_final(result.data());
    return result;
}

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
    std::vector<uint8_t> ServerClientHash;
    std::vector<uint8_t> ServicePrincipalName;
    std::vector<uint8_t> identity_User;
    std::vector<uint8_t> identity_Domain;
    std::vector<uint8_t> identity_Password;

    bool sspi_context_initialized = false;
    TimeObj & timeobj;
    Random & rand;

    // NTLMContextClient
    const bool NTLMv2 = true;
    bool UseMIC = true; // like NTLMv2
    NtlmState sspi_context_state = NTLM_STATE_INITIAL;
    uint8_t MachineID[32];
    const bool SendVersionInfo = true;
    const bool confidentiality = true;

    SslRC4 SendRc4Seal {};
    SslRC4 RecvRc4Seal {};

    uint32_t NegotiateFlags = 0;

    //int LmCompatibilityLevel;
    bool SendWorkstationName = true;
    std::vector<uint8_t> Workstation;

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
    NTLMNegotiateMessage NEGOTIATE_MESSAGE;
    NTLMChallengeMessage CHALLENGE_MESSAGE;
    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;

    NtlmVersion version;
    std::vector<uint8_t> SavedNegotiateMessage;
    std::vector<uint8_t> SavedChallengeMessage;
    std::vector<uint8_t> SavedAuthenticateMessage;

    uint8_t Timestamp[8]{};
    uint8_t ChallengeTimestamp[8]{};
    uint8_t ServerChallenge[8]{};
    uint8_t ClientChallenge[8]{};
    array_md5 SessionBaseKey; 
    array_md5 ExportedSessionKey;
    array_md5 EncryptedRandomSessionKey;
    array_md5 sspi_context_ClientSigningKey;
    array_md5 ClientSealingKey;
    array_md5 sspi_context_ServerSigningKey;
    array_md5 ServerSealingKey;
    array_md5 MessageIntegrityCheck;
    // uint8_t NtProofStr[16];

    SEC_STATUS sspi_context_read_challenge(array_view_const_u8 input_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Read Challenge");
        InStream in_stream(input_buffer);
        this->CHALLENGE_MESSAGE.recv(in_stream);
        this->SavedChallengeMessage.assign(in_stream.get_bytes().data(),in_stream.get_bytes().data()+in_stream.get_offset());

        this->sspi_context_state = NTLM_STATE_AUTHENTICATE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS sspi_context_write_authenticate(Array& output_buffer, Random & rand, TimeObj & timeobj) {
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
        size_t temp_size = 1 + 1 + 6 + 8 + 8 + 4 + AvPairsStream.size() + 4;
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContextClient Compute response: AvPairs size %zu", AvPairsStream.size());
            LOG(LOG_INFO, "NTLMContextClient Compute response: temp size %zu", temp_size);
        }

        auto unique_temp = std::make_unique<uint8_t[]>(temp_size);
        uint8_t* temp = unique_temp.get();
        memset(temp, 0, temp_size);
        temp[0] = 0x01;
        temp[1] = 0x01;

        // compute ClientChallenge (nonce(8))
        // /* ClientChallenge is used in computation of LMv2 and NTLMv2 responses */

        // compute ClientTimeStamp
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient TimeStamp");
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
        memcpy(&temp[1+1+6], this->Timestamp, 8);

        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Generate Client Challenge");
        rand.random(this->ClientChallenge, 8);
        memcpy(&temp[1+1+6+8], this->ClientChallenge, 8);

        memcpy(&temp[1+1+6+8+8+4], AvPairsStream.get_data(), AvPairsStream.size());

        // NtProofStr = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                       Concat(ServerChallenge, temp))

        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: NtProofStr");

        memcpy(this->ServerChallenge, this->CHALLENGE_MESSAGE.serverChallenge, 8);

        array_md5 NtProofStr = ::HmacMd5(make_array_view(ResponseKeyNT),{this->ServerChallenge, 8},{temp, temp_size});


        // NtChallengeResponse = Concat(NtProofStr, temp)

        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: NtChallengeResponse");
        auto & NtChallengeResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        // BStream & NtChallengeResponse = this->BuffNtChallengeResponse;
        NtChallengeResponse.reset();
        NtChallengeResponse.ostream.out_copy_bytes(NtProofStr.data(), NtProofStr.size());
        NtChallengeResponse.ostream.out_copy_bytes(temp, temp_size);
        NtChallengeResponse.mark_end();

        LOG_IF(this->verbose, LOG_INFO, "Compute response: NtChallengeResponse Ready");

        unique_temp.reset();

        LOG_IF(this->verbose, LOG_INFO, "Compute response: temp buff successfully deleted");
        // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
        //                                         Concat(ServerChallenge, ClientChallenge))
        // LmChallengeResponse.ChallengeFromClient = ClientChallenge
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response: LmChallengeResponse");
        auto & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
        // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
        LmChallengeResponse.reset();

        array_md5 LCResponse = ::HmacMd5(ResponseKeyLM, {this->ServerChallenge, 8}, {this->ClientChallenge, 8});

        LmChallengeResponse.ostream.out_copy_bytes(LCResponse);
        LmChallengeResponse.ostream.out_copy_bytes(this->ClientChallenge, 8);
        LmChallengeResponse.mark_end();

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
        rand.random(this->ExportedSessionKey.data(), SslMd5::DIGEST_LENGTH);
        this->EncryptedRandomSessionKey = ::Rc4Key(this->SessionBaseKey, this->ExportedSessionKey);

        auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
        AuthEncryptedRSK.reset();
        AuthEncryptedRSK.ostream.out_copy_bytes(this->EncryptedRandomSessionKey.data(), 16);
        AuthEncryptedRSK.mark_end();

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
        this->RecvRc4Seal.set_key(this->ServerSealingKey);
        
        this->NegotiateFlags = 0;
        if (this->NTLMv2) {
            this->NegotiateFlags |= NTLMSSP_NEGOTIATE_56;
            if (this->SendVersionInfo) {
                this->NegotiateFlags |= NTLMSSP_NEGOTIATE_VERSION;
            }
        }

        if (this->UseMIC) {
            this->NegotiateFlags |= NTLMSSP_NEGOTIATE_TARGET_INFO;
        }
        if (this->SendWorkstationName) {
            this->NegotiateFlags |= NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED;
        }
        if (this->confidentiality) {
            this->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
        }
        if (this->CHALLENGE_MESSAGE.negoFlags.flags & NTLMSSP_NEGOTIATE_KEY_EXCH) {
            this->NegotiateFlags |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        }
        this->NegotiateFlags |= (NTLMSSP_NEGOTIATE_128
                               | NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
                               | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                               | NTLMSSP_NEGOTIATE_NTLM
                               | NTLMSSP_NEGOTIATE_SIGN
                               | NTLMSSP_REQUEST_TARGET
                               | NTLMSSP_NEGOTIATE_UNICODE);

        this->AUTHENTICATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
            this->version.ntlm_get_version_info();
            this->AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();
        }
        else {
            this->version.ignore_version_info();
            this->AUTHENTICATE_MESSAGE.version.ignore_version_info();
        }

        uint32_t flag = this->AUTHENTICATE_MESSAGE.negoFlags.flags;

        if (!(flag & NTLMSSP_NEGOTIATE_KEY_EXCH)) {
            // If flag is not set, encryted session key buffer is not send
            this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer.reset();
        }
        if (flag & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
            auto & workstationbuff = this->AUTHENTICATE_MESSAGE.Workstation.buffer;
            workstationbuff.reset();
            workstationbuff.ostream.out_copy_bytes(this->Workstation);
            workstationbuff.mark_end();
        }

        //flag |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
        this->AUTHENTICATE_MESSAGE.DomainName.buffer.reset();
        this->AUTHENTICATE_MESSAGE.DomainName.buffer.ostream.out_copy_bytes(this->identity_Domain);
        this->AUTHENTICATE_MESSAGE.DomainName.buffer.mark_end();

        this->AUTHENTICATE_MESSAGE.UserName.buffer.reset();
        this->AUTHENTICATE_MESSAGE.UserName.buffer.ostream.out_copy_bytes(this->identity_User);
        this->AUTHENTICATE_MESSAGE.UserName.buffer.mark_end();

        this->sspi_context_state = NTLM_STATE_FINAL;
                                             
        StaticOutStream<65535> out_stream;
        if (this->UseMIC) {
            this->AUTHENTICATE_MESSAGE.ignore_mic = true;
            this->AUTHENTICATE_MESSAGE.emit(out_stream);
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
        this->AUTHENTICATE_MESSAGE.emit(out_stream);
        output_buffer.init(out_stream.get_offset());
        output_buffer.copy(out_stream.get_bytes());
        if (this->verbose) {
            this->AUTHENTICATE_MESSAGE.log();
        }
        return SEC_I_COMPLETE_NEEDED;
    }
        
    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    // Inlined

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;

    // GSS_Wrap
    // ENCRYPT_MESSAGE EncryptMessage;

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;
    std::pair<SEC_STATUS,std::vector<uint8_t>> sspi_DecryptMessage(array_view_const_u8 data_in, unsigned long MessageSeqNo) 
    {
        std::vector<uint8_t> data_out;
    
        if (!this->sspi_context_initialized) {
            return {SEC_E_NO_CONTEXT, data_out};
        }
        LOG_IF(this->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

        if (data_in.size() < cbMaxSignature) {
            return {SEC_E_INVALID_TOKEN, data_out};
        }

        // data_in [signature][data_buffer]

        auto data_buffer = data_in.array_from_offset(cbMaxSignature);
        data_out.resize(data_buffer.size(), 0);

        /* Decrypt message using with RC4, result overwrites original buffer */
        // this->confidentiality == true
        this->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.data());

        std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        array_md5 digest = ::HmacMd5(this->sspi_context_ServerSigningKey, seqno, data_out);

        /* Concatenate version, ciphertext and sequence number to build signature */
        std::array<uint8_t,16> expected_signature{
            1, 0, 0, 0, // Version
            0, 0, 0, 0, 0, 0, 0, 0,
            uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        this->RecvRc4Seal.crypt(8, digest.data(), &expected_signature[4]);


        if (memcmp(data_in.data(), expected_signature.data(), 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature.data(), 16);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(data_in.data(), 16);

            return {SEC_E_MESSAGE_ALTERED, data_out};
        }
        return {SEC_E_OK, data_out};
    }

    bool restricted_admin_mode;

    const char * target_host;
    std::string& extra_message;
    Translation::language_t lang;
    const bool verbose;

    void credssp_generate_client_nonce(Random & rand) {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp generate client nonce");
        rand.random(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH);
        this->SavedClientNonce.initialized = true;
        this->credssp_set_client_nonce();
    }

    void credssp_get_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp get client nonce");
        if (this->ts_request.clientNonce.isset()){
            this->SavedClientNonce = this->ts_request.clientNonce;
        }
    }
    void credssp_set_client_nonce() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::credssp set client nonce");
        if (!this->ts_request.clientNonce.isset()) {
            this->ts_request.clientNonce = this->SavedClientNonce;
        }
    }

    void credssp_generate_public_key_hash_client_to_server() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::generate credssp public key hash (client->server)");
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Client-To-Server Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH));

        sha256.update({this->PublicKey.data(),this->PublicKey.size()});
        sha256.final(hash);
        this->ClientServerHash.assign(hash, hash+sizeof(hash));
    }

    void credssp_generate_public_key_hash_server_to_client() {
        LOG(LOG_INFO, "rdpCredsspClientNTLM::generate credssp public key hash (server->client)");
        SslSha256 sha256;
        uint8_t hash[SslSha256::DIGEST_LENGTH];
        sha256.update("CredSSP Server-To-Client Binding Hash\0"_av);
        sha256.update(make_array_view(this->SavedClientNonce.data, ClientNonce::CLIENT_NONCE_LENGTH));
        sha256.update({this->PublicKey.data(),this->PublicKey.size()});
        sha256.final(hash);
        this->ServerClientHash.assign(hash, hash + sizeof(hash));
    }

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


    SEC_STATUS credssp_encrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::encrypt_public_key_echo");
        uint32_t version = this->ts_request.use_version;

        array_view_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};
        if (version >= 5) {
            this->credssp_generate_client_nonce(this->rand);
            this->credssp_generate_public_key_hash_client_to_server();
            public_key = {this->ClientServerHash.data(), this->ClientServerHash.size()};
        }

        if (!this->sspi_context_initialized) {
            return SEC_E_NO_CONTEXT;
        }

        unsigned long MessageSeqNo = this->send_seq_num++;
        // data_out [signature][data_buffer]
        std::vector<uint8_t> data_out(public_key.size() + cbMaxSignature);
        std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        array_md5 digest = ::HmacMd5(this->sspi_context_ClientSigningKey, seqno, public_key);
        
        this->SendRc4Seal.crypt(public_key.size(), public_key.data(), data_out.data()+cbMaxSignature);
        this->sspi_compute_signature(data_out.data(), this->SendRc4Seal, digest.data(), MessageSeqNo);
        
//        /* Concatenate version, ciphertext and sequence number to build signature */
//        std::array<uint8_t,16> expected_signature{
//            1, 0, 0, 0, // Version
//            0, 0, 0, 0, 0, 0, 0, 0,
//            uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
//        this->RecvRc4Seal.crypt(8, digest, signature.data()+4);

        this->ts_request.pubKeyAuth.init(data_out.size());
        this->ts_request.pubKeyAuth.copy(const_bytes_view{data_out.data(),data_out.size()});
        return SEC_E_OK;
    }

    SEC_STATUS credssp_decrypt_public_key_echo() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::decrypt_public_key_echo");

        array_view_const_u8 data_in = this->ts_request.pubKeyAuth.av();
        unsigned long MessageSeqNo = this->recv_seq_num++;
        std::vector<uint8_t> data_out;
    
        if (!this->sspi_context_initialized) {
            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "DecryptMessage failure: SEC_E_NO_CONTEXT");
            return SEC_E_NO_CONTEXT;
        }
        LOG_IF(this->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

        if (data_in.size() < cbMaxSignature) {
            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "DecryptMessage failure: SEC_E_INVALID_TOKEN");
            return SEC_E_INVALID_TOKEN;
        }

        // data_in [signature][data_buffer]

        auto data_buffer = data_in.array_from_offset(cbMaxSignature);
        data_out.resize(data_buffer.size(), 0);

        /* Decrypt message using with RC4, result overwrites original buffer */
        // this->confidentiality == true
        this->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.data());

        std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        array_md5 digest = ::HmacMd5(this->sspi_context_ServerSigningKey, seqno, data_out);

        uint8_t expected_signature[16] = {};
        this->sspi_compute_signature(
            expected_signature, this->RecvRc4Seal, digest.data(), MessageSeqNo);
            
        if (memcmp(data_in.data(), expected_signature, 16) != 0) {
            /* signature verification failed! */
            LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
            LOG(LOG_ERR, "Expected Signature:");
            hexdump_c(expected_signature, 16);
            LOG(LOG_ERR, "Actual Signature:");
            hexdump_c(data_in.data(), 16);

            if (this->ts_request.pubKeyAuth.size() == 0) {
                // report_error
                this->extra_message = " ";
                this->extra_message.append(TR(trkeys::err_login_password, this->lang));
                LOG(LOG_INFO, "Provided login/password is probably incorrect.");
            }
            LOG(LOG_ERR, "DecryptMessage failure: SEC_E_MESSAGE_ALTERED");
            return SEC_E_MESSAGE_ALTERED;
        }

        const uint32_t version = this->ts_request.use_version;

        array_view_const_u8 public_key = {this->PublicKey.data(),this->PublicKey.size()};
        if (version >= 5) {
            this->credssp_get_client_nonce();
            this->credssp_generate_public_key_hash_server_to_client();
            public_key = {this->ServerClientHash.data(), this->ServerClientHash.size()};
        }

        array_view_u8 public_key2 = {data_out.data(), data_out.size()};

        if (public_key2.size() != public_key.size()) {
            LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", public_key2.size(), public_key.size());
            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        if (version < 5) {
            // if we are client and protocol is 2,3,4
            // then get the public key minus one
            ::ap_integer_decrement_le(public_key2);
        }

        if (memcmp(public_key.data(), public_key2.data(), public_key.size()) != 0) {
            LOG(LOG_ERR, "Could not verify server's public key echo");

            LOG(LOG_ERR, "Expected (length = %zu):", public_key.size());
            hexdump_av_c(public_key);

            LOG(LOG_ERR, "Actual (length = %zu):", public_key.size());
            hexdump_av_c(public_key2);

            return SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
        }

        return SEC_E_OK;
    }

    enum class Res : bool { Err, Ok };

    enum : uint8_t { Start, Loop, Final } client_auth_data_state = Start;
    std::vector<uint8_t> client_auth_data_input_buffer;

    Res sm_credssp_client_authenticate_stop(InStream & in_stream, OutTransport transport)
    {
        /* Encrypted Public Key +1 */
        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Receiving Encrypted PubKey + 1");

        this->ts_request.recv(in_stream);

        /* Verify Server Public Key Echo */
        SEC_STATUS status = this->credssp_decrypt_public_key_echo();
        this->ts_request.negoTokens.init(0);
        this->ts_request.pubKeyAuth.init(0);
        this->ts_request.authInfo.init(0);
        this->ts_request.clientNonce.reset();
        this->ts_request.error_code = 0;

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!");
            return Res::Err;
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

        StaticOutStream<65536> ts_credentials_send;
        this->ts_credentials.emit(ts_credentials_send);

        if (!this->sspi_context_initialized) {
            LOG(LOG_ERR, "credssp_encrypt_ts_credentials error status:SEC_E_NO_CONTEXT");
            return Res::Err;
        }

        array_view_const_u8 data_in = {ts_credentials_send.get_data(), ts_credentials_send.get_offset()};
        unsigned long MessageSeqNo = this->send_seq_num++;
        // data_out [signature][data_buffer]
        std::vector<uint8_t> data_out;
        data_out.resize(data_in.size() + cbMaxSignature, 0);
        
        std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        array_md5 digest = ::HmacMd5(this->sspi_context_ClientSigningKey, seqno, data_in);

        this->SendRc4Seal.crypt(data_in.size(), data_in.data(), data_out.data()+cbMaxSignature);
        this->sspi_compute_signature(data_out.data(), this->SendRc4Seal, digest.data(), MessageSeqNo);
        
        this->ts_request.authInfo.init(data_out.size());
        this->ts_request.authInfo.copy(const_bytes_view{data_out.data(),data_out.size()});

        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Credentials");
        StaticOutStream<65536> ts_request_emit;
        this->ts_request.emit(ts_request_emit);
        transport.get_transport().send(ts_request_emit.get_bytes());
        this->ts_request.negoTokens.init(0);
        this->ts_request.pubKeyAuth.init(0);
        this->ts_request.authInfo.init(0);
        this->ts_request.clientNonce.reset();
        this->ts_request.error_code = 0;

        return Res::Ok;
    }

public:
    rdpCredsspClientNTLM(OutTransport transport,
               uint8_t * user,
               uint8_t * domain,
               uint8_t * pass,
               uint8_t * hostname,
               const char * target_host,
               const bool restricted_admin_mode,
               Random & rand,
               TimeObj & timeobj,
               std::string& extra_message,
               Translation::language_t lang,
               const bool verbose = false)
        : ts_request(6) // Credssp Version 6 Supported
        , SavedClientNonce()
        , timeobj(timeobj)
        , rand(rand)
        , restricted_admin_mode(restricted_admin_mode)
        , target_host(target_host)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
    {
        memset(this->MachineID, 0xAA, sizeof(this->MachineID));
        memset(this->MessageIntegrityCheck.data(), 0x00, this->MessageIntegrityCheck.size());

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::Initialization");
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::set_credentials");
        this->identity_User = ::UTF8toUTF16({user,strlen(reinterpret_cast<char*>(user))});
        this->identity_Domain = ::UTF8toUTF16({domain,strlen(reinterpret_cast<char*>(domain))});
        this->identity_Password = ::UTF8toUTF16({pass,strlen(reinterpret_cast<char*>(pass))});

        if (hostname){
            size_t length = strlen(char_ptr_cast(hostname));
            this->ServicePrincipalName.assign(hostname, hostname + length);
        }
        else {
            this->ServicePrincipalName.clear();
        }
        this->ServicePrincipalName.push_back(0);

        array_view_const_char spn = bytes_view(this->ServicePrincipalName).as_chars();
        if (!this->sspi_context_initialized) {
            if (!spn.empty()) {
                this->Workstation = ::UTF8toUTF16(spn);
                this->SendWorkstationName = true;
            }
            else {
                this->Workstation.clear();
                this->SendWorkstationName = false;
            }
            this->sspi_context_initialized = true;
        }

        this->client_auth_data_state = Start;

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::client_authenticate");

        // ============================================
        /* Get Public Key From TLS Layer and hostname */
        // ============================================

        auto const key = transport.get_transport().get_public_key();
        this->PublicKey.assign(key.data(), key.data()+key.size());

        LOG(LOG_INFO, "Credssp: NTLM Authentication");
       
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::AcquireCredentialsHandle");

        this->client_auth_data_state = Loop;

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
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");

        this->sspi_context_state = NTLM_STATE_NEGOTIATE;
        LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Write Negotiate");
        this->NegotiateFlags |= (
              NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
            | NTLMSSP_NEGOTIATE_KEY_EXCH | NTLMSSP_NEGOTIATE_128 
            | NTLMSSP_NEGOTIATE_SIGN | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
            | NTLMSSP_NEGOTIATE_NTLM | NTLMSSP_REQUEST_TARGET 
            | NTLMSSP_NEGOTIATE_UNICODE)
        | (this->NTLMv2) * (NTLMSSP_NEGOTIATE_56
            |  NTLMSSP_NEGOTIATE_VERSION
            |  NTLMSSP_NEGOTIATE_LM_KEY
            |  NTLMSSP_NEGOTIATE_OEM)
        | (this->confidentiality) * NTLMSSP_NEGOTIATE_SEAL
        | (this->SendVersionInfo) * NTLMSSP_NEGOTIATE_VERSION;

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
            this->version.ntlm_get_version_info();
            this->NEGOTIATE_MESSAGE.version.ntlm_get_version_info();
        }
        else {
            this->version.ignore_version_info();
            this->NEGOTIATE_MESSAGE.version.ignore_version_info();
        }
        this->NEGOTIATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
            this->NEGOTIATE_MESSAGE.Workstation.buffer.reset();
            this->NEGOTIATE_MESSAGE.Workstation.buffer.ostream.out_copy_bytes(this->Workstation);
            this->NEGOTIATE_MESSAGE.Workstation.buffer.mark_end();
        }

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED) {
            auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            domain.reset();
            domain.ostream.out_copy_bytes(this->identity_Domain);
            domain.mark_end();
        }

        this->sspi_context_state = NTLM_STATE_CHALLENGE;

        StaticOutStream<65535> out_stream;
        this->NEGOTIATE_MESSAGE.emit(out_stream);
        this->SavedNegotiateMessage.assign(out_stream.get_bytes().data(), out_stream.get_bytes().data()+out_stream.get_offset());        this->ts_request.negoTokens.init(this->SavedNegotiateMessage.size());
        this->ts_request.negoTokens.copy(this->SavedNegotiateMessage);
        this->sspi_context_state = NTLM_STATE_CHALLENGE;

        /* send authentication token to server */
        if (this->ts_request.negoTokens.size() > 0) {
            if (this->ts_request.negoTokens.size() > 0){
                LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
            }

            StaticOutStream<65536> ts_request_emit;
            this->ts_request.emit(ts_request_emit);
            transport.get_transport().send(ts_request_emit.get_bytes());

            this->ts_request.negoTokens.init(0);
            this->ts_request.pubKeyAuth.init(0);
            this->ts_request.authInfo.init(0);
            this->ts_request.clientNonce.reset();
            this->ts_request.error_code = 0;
        }
    }

    credssp::State credssp_client_authenticate_next(InStream & in_stream, OutTransport transport)
    {
        switch (this->client_auth_data_state)
        {
            case Start:
                return credssp::State::Err;

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

                LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");
                auto status = SEC_E_OUT_OF_SEQUENCE;

                if (this->sspi_context_state == NTLM_STATE_INITIAL) {
                    this->sspi_context_state = NTLM_STATE_NEGOTIATE;
                }
                if (this->sspi_context_state == NTLM_STATE_NEGOTIATE) {
                    LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Write Negotiate");
                    this->NegotiateFlags |= (
                          NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
                        | NTLMSSP_NEGOTIATE_KEY_EXCH | NTLMSSP_NEGOTIATE_128 
                        | NTLMSSP_NEGOTIATE_SIGN | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                        | NTLMSSP_NEGOTIATE_NTLM | NTLMSSP_REQUEST_TARGET 
                        | NTLMSSP_NEGOTIATE_UNICODE)
                    | (this->NTLMv2) * (NTLMSSP_NEGOTIATE_56
                        |  NTLMSSP_NEGOTIATE_VERSION
                        |  NTLMSSP_NEGOTIATE_LM_KEY
                        |  NTLMSSP_NEGOTIATE_OEM)
                    | (this->confidentiality) * NTLMSSP_NEGOTIATE_SEAL
                    | (this->SendVersionInfo) * NTLMSSP_NEGOTIATE_VERSION;

                    if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
                        this->version.ntlm_get_version_info();
                        this->NEGOTIATE_MESSAGE.version.ntlm_get_version_info();
                    }
                    else {
                        this->version.ignore_version_info();
                        this->NEGOTIATE_MESSAGE.version.ignore_version_info();
                    }
                    this->NEGOTIATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;

                    if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
                        this->NEGOTIATE_MESSAGE.Workstation.buffer.reset();
                        this->NEGOTIATE_MESSAGE.Workstation.buffer.ostream.out_copy_bytes(this->Workstation);
                        this->NEGOTIATE_MESSAGE.Workstation.buffer.mark_end();
                    }

                    if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED) {
                        auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
                        domain.reset();
                        domain.ostream.out_copy_bytes(this->identity_Domain);
                        domain.mark_end();
                    }

                    this->sspi_context_state = NTLM_STATE_CHALLENGE;

                    StaticOutStream<65535> out_stream;
                    this->NEGOTIATE_MESSAGE.emit(out_stream);
                    this->SavedNegotiateMessage.assign(out_stream.get_bytes().data(), out_stream.get_bytes().data()+out_stream.get_offset());                    this->ts_request.negoTokens.init(this->SavedNegotiateMessage.size());
                    this->ts_request.negoTokens.copy(this->SavedNegotiateMessage);
                    this->sspi_context_state = NTLM_STATE_CHALLENGE;
                    status = SEC_I_CONTINUE_NEEDED;
                }
                else {
                    if (this->sspi_context_state == NTLM_STATE_CHALLENGE) {
                        this->sspi_context_read_challenge(this->client_auth_data_input_buffer);
                    }
                    if (this->sspi_context_state == NTLM_STATE_AUTHENTICATE) {
                        status = this->sspi_context_write_authenticate(this->ts_request.negoTokens, this->rand, this->timeobj);
                    }
                }

                if ((status != SEC_I_COMPLETE_AND_CONTINUE) &&
                    (status != SEC_I_COMPLETE_NEEDED) &&
                    (status != SEC_E_OK) &&
                    (status != SEC_I_CONTINUE_NEEDED)) {
                    LOG(LOG_ERR, "Initialize Security Context Error !");
                    return credssp::State::Err;
                }

                SEC_STATUS encrypted = SEC_E_INVALID_TOKEN;

                if ((status == SEC_I_COMPLETE_AND_CONTINUE) ||
                    (status == SEC_I_COMPLETE_NEEDED) ||
                    (status == SEC_E_OK)) {
                    // have_pub_key_auth = true;
                    encrypted = this->credssp_encrypt_public_key_echo();
                    if (status == SEC_I_COMPLETE_NEEDED) {
                        status = SEC_E_OK;
                    }
                    else if (status == SEC_I_COMPLETE_AND_CONTINUE) {
                        status = SEC_I_CONTINUE_NEEDED;
                    }
                }

                /* send authentication token to server */
                if ((this->ts_request.negoTokens.size() > 0)||(encrypted == SEC_E_OK)) {
                    // #ifdef WITH_DEBUG_CREDSSP
                    //             LOG(LOG_ERR, "Sending Authentication Token");
                    //             hexdump_c(this->ts_request.negoTokens.pvBuffer, this->ts_request.negoTokens.cbBuffer);
                    // #endif
                    if (this->ts_request.negoTokens.size() > 0){
                        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Authentication Token");
                    }

                    LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::send");
                    StaticOutStream<65536> ts_request_emit;
                    this->ts_request.emit(ts_request_emit);
                    transport.get_transport().send(ts_request_emit.get_bytes());

                    this->ts_request.negoTokens.init(0);
                    this->ts_request.pubKeyAuth.init(0);
                    this->ts_request.authInfo.init(0);
                    this->ts_request.clientNonce.reset();
                    this->ts_request.error_code = 0;
                }

                if (status != SEC_I_CONTINUE_NEEDED) {
                    LOG_IF(this->verbose, LOG_INFO, "rdpCredssp Token loop: CONTINUE_NEEDED");

                    this->client_auth_data_state = Final;
                }
                return credssp::State::Cont;
            }
            case Final:
                if (Res::Err == this->sm_credssp_client_authenticate_stop(in_stream, transport)) {
                    return credssp::State::Err;
                }
                this->client_auth_data_state = Start;
                return credssp::State::Finish;
        }

        return credssp::State::Err;
    }
};

