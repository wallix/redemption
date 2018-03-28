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
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan, Jennifer Inthavong
*/


#pragma once

#include "utils/genrandom.hpp"
#include "utils/difftimeval.hpp"
#include "utils/utf.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"
#include "core/RDP/nla/ntlm/ntlm_message_negotiate.hpp"
#include "core/RDP/nla/ntlm/ntlm_message_challenge.hpp"
#include "core/RDP/nla/ntlm/ntlm_message_authenticate.hpp"
#include "core/RDP/nla/sspi.hpp"

#include "system/ssl_md5.hpp"
#include "system/ssl_rc4.hpp"
#include "system/ssl_md4.hpp"

enum NtlmState {
    NTLM_STATE_INITIAL,
    NTLM_STATE_NEGOTIATE,
    NTLM_STATE_CHALLENGE,
    NTLM_STATE_AUTHENTICATE,
    NTLM_STATE_FINAL
};
//static const uint8_t lm_magic[] = "KGS!@#$%";

static const uint8_t client_sign_magic[] =
    "session key to client-to-server signing key magic constant";
static const uint8_t server_sign_magic[] =
    "session key to server-to-client signing key magic constant";
static const uint8_t client_seal_magic[] =
    "session key to client-to-server sealing key magic constant";
static const uint8_t server_seal_magic[] =
    "session key to server-to-client sealing key magic constant";

class NTLMContext
{
    TimeObj & timeobj;
    Random & rand;

public:
    bool server;
    bool NTLMv2;
    bool UseMIC;
    NtlmState state;

public:
    int SendSeqNum;
    int RecvSeqNum;

private:
    uint8_t MachineID[32];
    bool SendVersionInfo;
public:
    bool confidentiality;

    SslRC4 SendRc4Seal;
    SslRC4 RecvRc4Seal;
    uint8_t* SendSigningKey;
    uint8_t* RecvSigningKey;
private:
    uint8_t* SendSealingKey;
    uint8_t* RecvSealingKey;

public:
    uint32_t NegotiateFlags;

    //int LmCompatibilityLevel;
    bool SendWorkstationName;
public:
    Array Workstation;
    Array ServicePrincipalName;
public:
    SEC_WINNT_AUTH_IDENTITY identity;

    // uint8_t* ChannelBindingToken;
    // uint8_t ChannelBindingsHash[16];
    // SecPkgContext_Bindings Bindings;

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
public:
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
    uint8_t Timestamp[8];
    uint8_t ChallengeTimestamp[8];
public:
    uint8_t ServerChallenge[8];
private:
    uint8_t ClientChallenge[8];
public:
    uint8_t SessionBaseKey[SslMd5::DIGEST_LENGTH];
private:
    //uint8_t KeyExchangeKey[16];
    //uint8_t RandomSessionKey[16];
public:
    uint8_t ExportedSessionKey[16];
    uint8_t EncryptedRandomSessionKey[16];
public:
    uint8_t ClientSigningKey[16];
    uint8_t ClientSealingKey[16];
    uint8_t ServerSigningKey[16];
    uint8_t ServerSealingKey[16];
    uint8_t MessageIntegrityCheck[SslMd5::DIGEST_LENGTH];
    // uint8_t NtProofStr[16];

    bool verbose;

public:
    explicit NTLMContext(Random & rand, TimeObj & timeobj)
        : timeobj(timeobj)
        , rand(rand)
        , server(false)
        , NTLMv2(true)
        , UseMIC(false)
        , state(NTLM_STATE_INITIAL)
        , SendSeqNum(0)
        , RecvSeqNum(0)
        , MachineID()
        , SendVersionInfo(true)
        , confidentiality(true)
        , SendRc4Seal()
        , RecvRc4Seal()
        , SendSigningKey(nullptr)
        , RecvSigningKey(nullptr)
        , SendSealingKey(nullptr)
        , RecvSealingKey(nullptr)
        , NegotiateFlags(0)
        //, LmCompatibilityLevel(3)
        , SendWorkstationName(true)
        , Workstation(0)
        , ServicePrincipalName(0)
        , SavedNegotiateMessage(0)
        , SavedChallengeMessage(0)
        , SavedAuthenticateMessage(0)
        , Timestamp()
        , ChallengeTimestamp()
        , ServerChallenge()
        , ClientChallenge()
        , SessionBaseKey()
        //, KeyExchangeKey()
        //, RandomSessionKey()
        , ExportedSessionKey()
        , EncryptedRandomSessionKey()
        , ClientSigningKey()
        , ClientSealingKey()
        , ServerSigningKey()
        , ServerSealingKey()
        , MessageIntegrityCheck()
        , verbose(0)
        // , SendSingleHostData(false)
    {
        // this->SavedNegotiateMessage.init(0);
        // this->SavedChallengeMessage.init(0);
        // this->SavedAuthenticateMessage.init(0);
        // this->LmCompatibilityLevel = 3;
        memset(this->MachineID, 0xAA, sizeof(this->MachineID));
        memset(this->MessageIntegrityCheck, 0x00, sizeof(this->MessageIntegrityCheck));

        // memset(this->Timestamp, 0x00, 8);
        // memset(this->ChallengeTimestamp, 0x00, 8);
        // memset(this->ServerChallenge, 0x00, 8);
        // memset(this->ClientChallenge, 0x00, 8);
        // memset(this->SessionBaseKey, 0x00, 16);
        // memset(this->KeyExchangeKey, 0x00, 16);
        // memset(this->RandomSessionKey, 0x00, 16);
        // memset(this->ExportedSessionKey, 0x00, 16);
        // memset(this->EncryptedRandomSessionKey, 0x00, 16);
        // memset(this->ClientSigningKey, 0x00, 16);
        // memset(this->ClientSealingKey, 0x00, 16);
        // memset(this->ServerSigningKey, 0x00, 16);
        // memset(this->ServerSealingKey, 0x00, 16);

        // this->Workstation.init(0);
        // this->ServicePrincipalName.init(0);

        if (this->NTLMv2) {
            this->UseMIC = true;
        }
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Init");
        }
    }

    NTLMContext(NTLMContext const &) = delete;
    NTLMContext& operator = (NTLMContext const &) = delete;

    /**
     * Generate timestamp for AUTHENTICATE_MESSAGE.
     */

    void ntlm_generate_timestamp()
    {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext TimeStamp");
        }
        uint8_t ZeroTimestamp[8] = {};

        if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0)
            memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
        else {
            timeval tv = this->timeobj.get_time();
            struct {
                uint32_t low;
                uint32_t high;
            } timestamp;
            timestamp.low = tv.tv_usec;
            timestamp.high = tv.tv_sec;
            memcpy(this->Timestamp, &timestamp, sizeof(timestamp));
        }
    }

    /**
     * Generate client challenge (8-byte nonce).
     */
    // client method
    void ntlm_generate_client_challenge()
    {
        // /* ClientChallenge is used in computation of LMv2 and NTLMv2 responses */
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Generate Client Challenge");
        }
        this->rand.random(this->ClientChallenge, 8);

    }
    /**
     * Generate server challenge (8-byte nonce).
     */
    // server method
    void ntlm_generate_server_challenge()
    {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Generate Server Challenge");
        }
        this->rand.random(this->ServerChallenge, 8);
    }
    // client method
    void ntlm_get_server_challenge() {
        memcpy(this->ServerChallenge, this->CHALLENGE_MESSAGE.serverChallenge, 8);
    }

    /**
     * Generate RandomSessionKey (16-byte nonce).
     */
    // client method
    //void ntlm_generate_random_session_key()
    //{
    //    if (this->verbose) {
    //        LOG(LOG_INFO, "NTLMContext Generate Random Session Key");
    //    }
    //    this->rand.random(this->RandomSessionKey, 16);
    //}

    // client method ??
    void ntlm_generate_exported_session_key() {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Generate Exported Session Key");
        }
        this->rand.random(this->ExportedSessionKey, 16);
    }

    // client method
    //void ntlm_generate_key_exchange_key()
    //{
    //    // /* In NTLMv2, KeyExchangeKey is the 128-bit SessionBaseKey */
    //    memcpy(this->KeyExchangeKey, this->SessionBaseKey, 16);
    //}

    // all strings are in unicode utf16
    void NTOWFv2_FromHash(const uint8_t * hash,   size_t hash_size,
                          const uint8_t * user,   size_t user_size,
                          const uint8_t * domain, size_t domain_size,
                          uint8_t * buff, size_t buff_size) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext NTOWFv2 Hash");
        }
        SslHMAC_Md5 hmac_md5(hash, hash_size);

        uint8_t * userup = new uint8_t[user_size];
        memcpy(userup, user, user_size);
        UTF16Upper(userup, user_size);
        hmac_md5.update(userup, user_size);
        delete [] userup;
        userup = nullptr;

        uint8_t tmp_md5[SslMd5::DIGEST_LENGTH] = {};

        // hmac_md5.update(user, user_size);
        hmac_md5.update(domain, domain_size);
        hmac_md5.final(tmp_md5);
        // TODO: check if buff_size is SslMd5::DIGEST_LENGTH
        // if it is so no need to use a temporary variable
        // and copy digest afterward.
        memset(buff, 0, buff_size);
        memcpy(buff, tmp_md5,
            std::min(buff_size,
            static_cast<size_t>(SslMd5::DIGEST_LENGTH)));
    }
    // all strings are in unicode utf16
    void hash_password(const uint8_t * pass, size_t pass_size, uint8_t (&hash)[SslMd4::DIGEST_LENGTH]) {
        SslMd4 md4;
        md4.update(pass, pass_size);
        md4.final(hash);
    }

    // all strings are in unicode utf16
    void NTOWFv2(const uint8_t * pass,   size_t pass_size,
                 const uint8_t * user,   size_t user_size,
                 const uint8_t * domain, size_t domain_size,
                 uint8_t * buff, size_t buff_size) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext NTOWFv2");
        }
        SslMd4 md4;
        uint8_t md4password[SslMd4::DIGEST_LENGTH] = {};

        md4.update(pass, pass_size);
        md4.final(md4password);

        SslHMAC_Md5 hmac_md5(md4password, sizeof(md4password));

        uint8_t * userup = new uint8_t[user_size];
        memcpy(userup, user, user_size);
        UTF16Upper(userup, user_size);
        hmac_md5.update(userup, user_size);

        delete [] userup;

        uint8_t tmp_md5[SslMd5::DIGEST_LENGTH] = {};

        userup = nullptr;
        hmac_md5.update(domain, domain_size);
        hmac_md5.final(tmp_md5);
        // TODO: check if buff_size is SslMd5::DIGEST_LENGTH
        // if it is so no need to use a temporary variable
        // and copy digest afterward.
        memset(buff, 0, buff_size);
        memcpy(buff, tmp_md5,
            std::min(buff_size, static_cast<size_t>(SslMd5::DIGEST_LENGTH)));
    }
    // all strings are in unicode utf16
    void LMOWFv2(const uint8_t * pass,   size_t pass_size,
                 const uint8_t * user,   size_t user_size,
                 const uint8_t * domain, size_t domain_size,
                 uint8_t * buff, size_t buff_size) {
        NTOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                buff, buff_size);
    }

    // client method
    // ntlmv2_compute_response_from_challenge generates :
    // - timestamp
    // - client challenge
    // - NtChallengeResponse
    // - LmChallengeResponse
    // all strings are in unicode utf16
    void ntlmv2_compute_response_from_challenge(const uint8_t * pass,   size_t pass_size,
                                                const uint8_t * user,   size_t user_size,
                                                const uint8_t * domain, size_t domain_size) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response from challenge");
        }
        uint8_t ResponseKeyNT[16] = {};
        uint8_t ResponseKeyLM[16] = {};
        this->NTOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                ResponseKeyNT, sizeof(ResponseKeyNT));
        this->LMOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                ResponseKeyLM, sizeof(ResponseKeyLM));

        // struct NTLMv2_Client_Challenge = temp
        // temp = { 0x01, 0x01, Z(6), Time, ClientChallenge, Z(4), ServerName , Z(4) }
        // Z(n) = { 0x00, ... , 0x00 } n times
        // ServerName = AvPairs received in Challenge message
        auto & AvPairsStream = this->CHALLENGE_MESSAGE.TargetInfo.buffer;
        // BStream AvPairsStream;
        // this->CHALLENGE_MESSAGE.AvPairList.emit(AvPairsStream);
        size_t temp_size = 1 + 1 + 6 + 8 + 8 + 4 + AvPairsStream.size() + 4;
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response: AvPairs size %zu", AvPairsStream.size());
            LOG(LOG_INFO, "NTLMContext Compute response: temp size %zu", temp_size);
        }

        uint8_t * temp = new uint8_t[temp_size];
        memset(temp, 0, temp_size);
        temp[0] = 0x01;
        temp[1] = 0x01;
        // compute ClientTimeStamp
        this->ntlm_generate_timestamp();
        // compute ClientChallenge (nonce(8))
        this->ntlm_generate_client_challenge();
        memcpy(&temp[1+1+6], this->Timestamp, 8);
        memcpy(&temp[1+1+6+8], this->ClientChallenge, 8);
        memcpy(&temp[1+1+6+8+8+4], AvPairsStream.get_data(), AvPairsStream.size());

        // NtProofStr = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                       Concat(ServerChallenge, temp))

        uint8_t NtProofStr[SslMd5::DIGEST_LENGTH] = {};
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response: NtProofStr");
        }
        SslHMAC_Md5 hmac_md5resp(ResponseKeyNT, sizeof(ResponseKeyNT));

        this->ntlm_get_server_challenge();
        hmac_md5resp.update(this->ServerChallenge, 8);
        hmac_md5resp.update(temp, temp_size);
        hmac_md5resp.final(NtProofStr);

        // NtChallengeResponse = Concat(NtProofStr, temp)

        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response: NtChallengeResponse");
        }
        auto & NtChallengeResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        // BStream & NtChallengeResponse = this->BuffNtChallengeResponse;
        NtChallengeResponse.reset();
        NtChallengeResponse.ostream.out_copy_bytes(NtProofStr, sizeof(NtProofStr));
        NtChallengeResponse.ostream.out_copy_bytes(temp, temp_size);
        NtChallengeResponse.mark_end();

        if (this->verbose) {
            LOG(LOG_INFO, "Compute response: NtChallengeResponse Ready");
        }

        delete [] temp;
        temp = nullptr;
        if (this->verbose) {
            LOG(LOG_INFO, "Compute response: temp buff successfully deleted");
        }
        // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
        //                                         Concat(ServerChallenge, ClientChallenge))
        // LmChallengeResponse.ChallengeFromClient = ClientChallenge
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response: LmChallengeResponse");
        }
        auto & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
        // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
        SslHMAC_Md5 hmac_md5lmresp(ResponseKeyLM, sizeof(ResponseKeyLM));
        LmChallengeResponse.reset();
        hmac_md5lmresp.update(this->ServerChallenge, 8);
        hmac_md5lmresp.update(this->ClientChallenge, 8);
        uint8_t LCResponse[SslMd5::DIGEST_LENGTH] = {};
        hmac_md5lmresp.final(LCResponse);

        LmChallengeResponse.ostream.out_copy_bytes(LCResponse, SslMd5::DIGEST_LENGTH);
        LmChallengeResponse.ostream.out_copy_bytes(this->ClientChallenge, 8);
        LmChallengeResponse.mark_end();

        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response: SessionBaseKey");
        }
        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                           NtProofStr)
        SslHMAC_Md5 hmac_md5seskey(ResponseKeyNT, sizeof(ResponseKeyNT));
        hmac_md5seskey.update(NtProofStr, sizeof(NtProofStr));
        hmac_md5seskey.final(this->SessionBaseKey);
    }

    // static method for both client and server (encrypt and decrypt)
    void ntlm_rc4k(uint8_t* key, int length, uint8_t* plaintext, uint8_t* ciphertext)
    {
        SslRC4 rc4;
        rc4.set_key(key, 16);
        rc4.crypt(length, plaintext, ciphertext);
    }

    // client method for authenticate message
    void ntlm_encrypt_random_session_key() {
        // EncryptedRandomSessionKey = RC4K(KeyExchangeKey, ExportedSessionKey)
        // ExportedSessionKey = NONCE(16) (random 16bytes number)
        // KeyExchangeKey = SessionBaseKey
        // EncryptedRandomSessionKey = RC4K(SessionBaseKey, NONCE(16))

        // generate NONCE(16) exportedsessionkey
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Encrypt RandomSessionKey");
        }
        this->ntlm_generate_exported_session_key();
        this->ntlm_rc4k(this->SessionBaseKey, 16,
                        this->ExportedSessionKey, this->EncryptedRandomSessionKey);

        auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
        AuthEncryptedRSK.reset();
        AuthEncryptedRSK.ostream.out_copy_bytes(this->EncryptedRandomSessionKey, 16);
        AuthEncryptedRSK.mark_end();
    }
    // server method to decrypt exported session key from authenticate message with
    // session base key computed with Responses.
    void ntlm_decrypt_exported_session_key() {
        auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Decrypt RandomSessionKey");
        }
        memcpy(this->EncryptedRandomSessionKey, AuthEncryptedRSK.get_data(),
               AuthEncryptedRSK.size());
        this->ntlm_rc4k(this->SessionBaseKey, 16,
                        this->EncryptedRandomSessionKey, this->ExportedSessionKey);
    }

    /**
     * Generate signing key.\n
     * @msdn{cc236711}
     * @param sign_magic Sign magic string
     * @param signing_key Destination signing key
     */

    void ntlm_generate_signing_key(const uint8_t * sign_magic, size_t magic_size, uint8_t (&signing_key)[SslMd5::DIGEST_LENGTH])
    {
        SslMd5 md5sign;
        md5sign.update(this->ExportedSessionKey, 16);
        md5sign.update(sign_magic, magic_size);
        md5sign.final(signing_key);
    }


    /**
     * Generate client signing key (ClientSigningKey).\n
     * @msdn{cc236711}
     */

    void ntlm_generate_client_signing_key()
    {
        this->ntlm_generate_signing_key(client_sign_magic, sizeof(client_sign_magic),
                                        this->ClientSigningKey);
    }

    /**
     * Generate server signing key (ServerSigningKey).\n
     * @msdn{cc236711}
     */

    void ntlm_generate_server_signing_key()
    {
        this->ntlm_generate_signing_key(server_sign_magic, sizeof(server_sign_magic),
                                        this->ServerSigningKey);
    }


    /**
     * Generate sealing key.\n
     * @msdn{cc236712}
     * @param seal_magic Seal magic string
     * @param sealing_key Destination sealing key
     */

    void ntlm_generate_sealing_key(const uint8_t * seal_magic, size_t magic_size, uint8_t (&sealing_key)[SslMd5::DIGEST_LENGTH])
    {
        SslMd5 md5seal;
        md5seal.update(this->ExportedSessionKey, 16);
        md5seal.update(seal_magic, magic_size);
        md5seal.final(sealing_key);
    }

    /**
     * Generate client sealing key (ClientSealingKey).\n
     * @msdn{cc236712}
     */

    void ntlm_generate_client_sealing_key()
    {
        ntlm_generate_sealing_key(client_seal_magic, sizeof(client_seal_magic),
                                  this->ClientSealingKey);
    }

    /**
     * Generate server sealing key (ServerSealingKey).\n
     * @msdn{cc236712}
     */

    void ntlm_generate_server_sealing_key()
    {
        ntlm_generate_sealing_key(server_seal_magic, sizeof(server_seal_magic),
                                  this->ServerSealingKey);
    }

    void ntlm_compute_MIC() {
        SslHMAC_Md5 hmac_md5resp(this->ExportedSessionKey, 16);
        hmac_md5resp.update(this->SavedNegotiateMessage.get_data(),
                            this->SavedNegotiateMessage.size());
        hmac_md5resp.update(this->SavedChallengeMessage.get_data(),
                            this->SavedChallengeMessage.size());
        hmac_md5resp.update(this->SavedAuthenticateMessage.get_data(),
                            this->SavedAuthenticateMessage.size());
        hmac_md5resp.final(this->MessageIntegrityCheck);
    }


    // all strings are in unicode utf16
    //void ntlm_compute_lm_v2_response(const uint8_t * pass,   size_t pass_size,
    //                                 const uint8_t * user,   size_t user_size,
    //                                 const uint8_t * domain, size_t domain_size)
    //{
    //
    //    uint8_t ResponseKeyLM[16] = {};
    //    this->LMOWFv2(pass, pass_size, user, user_size, domain, domain_size,
    //            ResponseKeyLM, sizeof(ResponseKeyLM));
    //    // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
    //    //                                         Concat(ServerChallenge, ClientChallenge))
    //    // LmChallengeResponse.ChallengeFromClient = ClientChallenge
    //    BStream & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.Buffer;
    //    // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
    //    SslHMAC_Md5 hmac_md5lmresp(ResponseKeyLM, sizeof(ResponseKeyLM));
    //    LmChallengeResponse.reset();
    //    hmac_md5lmresp.update(this->ServerChallenge, 8);
    //    hmac_md5lmresp.update(this->ClientChallenge, 8);
    //    uint8_t LCResponse[SslMd5::DIGEST_LENGTH] = {};
    //    hmac_md5lmresp.final(LCResponse);
    //    LmChallengeResponse.out_copy_bytes(LCResponse, 16);
    //    LmChallengeResponse.out_copy_bytes(this->ClientChallenge, 8);
    //    LmChallengeResponse.mark_end();
    //
    //}



    /**
     * Initialize RC4 stream cipher states for sealing.
     */

    void ntlm_init_rc4_seal_states()
    {
        if (this->server) {
            this->SendSigningKey = this->ServerSigningKey;
            this->RecvSigningKey = this->ClientSigningKey;
            this->SendSealingKey = this->ClientSealingKey;
            this->RecvSealingKey = this->ServerSealingKey;
            this->SendRc4Seal.set_key(this->ServerSealingKey, 16);
            this->RecvRc4Seal.set_key(this->ClientSealingKey, 16);
        }
        else {
            this->SendSigningKey = this->ClientSigningKey;
            this->RecvSigningKey = this->ServerSigningKey;
            this->SendSealingKey = this->ServerSealingKey;
            this->RecvSealingKey = this->ClientSealingKey;
            this->SendRc4Seal.set_key(this->ClientSealingKey, 16);
            this->RecvRc4Seal.set_key(this->ServerSealingKey, 16);
        }
    }

    // server check nt response
    bool ntlm_check_nt_response_from_authenticate(const uint8_t * hash, size_t hash_size) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Check NtResponse");
        }
        auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        size_t temp_size = AuthNtResponse.size() - 16;
        // LOG(LOG_INFO, "tmp size = %u", temp_size);
        uint8_t NtProofStr_from_msg[16] = {};
        InStream in_AuthNtResponse(AuthNtResponse.ostream.get_current(), AuthNtResponse.ostream.tailroom());
        in_AuthNtResponse.in_copy_bytes(NtProofStr_from_msg, 16);
        uint8_t * temp = new uint8_t[temp_size];
        in_AuthNtResponse.in_copy_bytes(temp, temp_size);
        AuthNtResponse.ostream.rewind();

        uint8_t NtProofStr[SslMd5::DIGEST_LENGTH] = {};
        uint8_t ResponseKeyNT[16] = {};
        // LOG(LOG_INFO, "NTLM CHECK NT RESPONSE FROM AUTHENTICATE");
        // LOG(LOG_INFO, "UserName size = %u", UserName.size());
        // LOG(LOG_INFO, "DomainName size = %u", DomainName.size());
        // LOG(LOG_INFO, "hash size = %u", hash_size);

        this->NTOWFv2_FromHash(hash, hash_size,
                               UserName.get_data(), UserName.size(),
                               DomainName.get_data(), DomainName.size(),
                               ResponseKeyNT, sizeof(ResponseKeyNT));
        // LOG(LOG_INFO, "ResponseKeyNT");
        // hexdump_c(ResponseKeyNT, sizeof(ResponseKeyNT));
        SslHMAC_Md5 hmac_md5resp(ResponseKeyNT, sizeof(ResponseKeyNT));
        hmac_md5resp.update(this->ServerChallenge, 8);
        hmac_md5resp.update(temp, temp_size);
        hmac_md5resp.final(NtProofStr);

        bool res = !memcmp(NtProofStr, NtProofStr_from_msg, 16);

        delete [] temp;
        temp = nullptr;

        return res;
    }

    // Server check lm response
    bool ntlm_check_lm_response_from_authenticate(const uint8_t * hash, size_t hash_size) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Check LmResponse");
        }
        auto & AuthLmResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
        auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        size_t lm_response_size = AuthLmResponse.size(); // should be 24
        if (lm_response_size != 24) {
            return false;
        }
        uint8_t response[16] = {};
        InStream in_AuthLmResponse(AuthLmResponse.ostream.get_current(), AuthLmResponse.ostream.tailroom());
        in_AuthLmResponse.in_copy_bytes(response, 16);
        in_AuthLmResponse.in_copy_bytes(this->ClientChallenge, 8);
        AuthLmResponse.ostream.rewind();

        uint8_t compute_response[SslMd5::DIGEST_LENGTH] = {};
        uint8_t ResponseKeyLM[16] = {};
        this->NTOWFv2_FromHash(hash, hash_size,
                               UserName.get_data(), UserName.size(),
                               DomainName.get_data(), DomainName.size(),
                               ResponseKeyLM, sizeof(ResponseKeyLM));

        SslHMAC_Md5 hmac_md5resp(ResponseKeyLM, sizeof(ResponseKeyLM));
        hmac_md5resp.update(this->ServerChallenge, 8);
        hmac_md5resp.update(this->ClientChallenge, 8);
        hmac_md5resp.final(compute_response);

        bool res = !memcmp(response, compute_response, 16);

        return res;
    }

    // server compute Session Base Key
    void ntlm_compute_session_base_key(const uint8_t * hash, size_t hash_size) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute Session Base Key");
        }
        auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        uint8_t NtProofStr[16] = {};
        InStream(AuthNtResponse.ostream.get_current(), AuthNtResponse.ostream.tailroom())
            .in_copy_bytes(NtProofStr, 16);
        AuthNtResponse.ostream.rewind();
        uint8_t ResponseKeyNT[16] = {};
        this->NTOWFv2_FromHash(hash, hash_size,
                               UserName.get_data(), UserName.size(),
                               DomainName.get_data(), DomainName.size(),
                               ResponseKeyNT, sizeof(ResponseKeyNT));
        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                           NtProofStr)
        SslHMAC_Md5 hmac_md5seskey(ResponseKeyNT, sizeof(ResponseKeyNT));
        hmac_md5seskey.update(NtProofStr, sizeof(NtProofStr));
        hmac_md5seskey.final(this->SessionBaseKey);
    }


    // server method
    bool ntlm_check_nego() {
        uint32_t & negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
        if (!((negoFlag & NTLMSSP_REQUEST_TARGET) &&
              (negoFlag & NTLMSSP_NEGOTIATE_NTLM) &&
              (negoFlag & NTLMSSP_NEGOTIATE_ALWAYS_SIGN) &&
              (negoFlag & NTLMSSP_NEGOTIATE_UNICODE))) {
            return false;
        }
        this->NegotiateFlags = negoFlag;
        return true;
    }

    void ntlm_set_negotiate_flags() {
        uint32_t & negoFlag = this->NegotiateFlags;
        if (this->NTLMv2) {
            negoFlag |= NTLMSSP_NEGOTIATE_56;
            negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
            negoFlag |= NTLMSSP_NEGOTIATE_LM_KEY;
            negoFlag |= NTLMSSP_NEGOTIATE_OEM;
        }

        negoFlag |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        negoFlag |= NTLMSSP_NEGOTIATE_128;
        negoFlag |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
        negoFlag |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        negoFlag |= NTLMSSP_NEGOTIATE_NTLM;
        negoFlag |= NTLMSSP_NEGOTIATE_SIGN;
        negoFlag |= NTLMSSP_REQUEST_TARGET;
        negoFlag |= NTLMSSP_NEGOTIATE_UNICODE;

        if (this->confidentiality) {
            negoFlag |= NTLMSSP_NEGOTIATE_SEAL;
        }

        if (this->SendVersionInfo) {
            negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
        }

        if (negoFlag & NTLMSSP_NEGOTIATE_VERSION) {
            this->version.ntlm_get_version_info();
        }
        else {
            this->version.ignore_version_info();
        }

        this->NegotiateFlags = negoFlag;
        this->NEGOTIATE_MESSAGE.negoFlags.flags = negoFlag;
    }

    void ntlm_set_negotiate_flags_auth() {
        uint32_t negoFlag = 0;
        if (this->NTLMv2) {
            negoFlag |= NTLMSSP_NEGOTIATE_56;
            if (this->SendVersionInfo) {
                negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
            }
        }

        if (this->UseMIC) {
            negoFlag |= NTLMSSP_NEGOTIATE_TARGET_INFO;
        }
        if (this->SendWorkstationName) {
            negoFlag |= NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED;
        }
        if (this->confidentiality) {
            negoFlag |= NTLMSSP_NEGOTIATE_SEAL;
        }
        if (this->CHALLENGE_MESSAGE.negoFlags.flags & NTLMSSP_NEGOTIATE_KEY_EXCH) {
            negoFlag |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        }
        negoFlag |= NTLMSSP_NEGOTIATE_128;
        negoFlag |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
        negoFlag |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        negoFlag |= NTLMSSP_NEGOTIATE_NTLM;
        negoFlag |= NTLMSSP_NEGOTIATE_SIGN;
        negoFlag |= NTLMSSP_REQUEST_TARGET;
        negoFlag |= NTLMSSP_NEGOTIATE_UNICODE;

        // if (this->SendVersionInfo) {
        //     negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
        // }

        if (negoFlag & NTLMSSP_NEGOTIATE_VERSION) {
            this->version.ntlm_get_version_info();
        }
        else {
            this->version.ignore_version_info();
        }

        this->NegotiateFlags = negoFlag;
        this->AUTHENTICATE_MESSAGE.negoFlags.flags = negoFlag;
    }



    // server method
    // TODO COMPLETE
    void ntlm_construct_challenge_target_info() {
        uint8_t win7[] =  {
            0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
        };
        uint8_t upwin7[] =  {
            0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00
        };
        NtlmAvPairList & list = this->CHALLENGE_MESSAGE.AvPairList;
        list.add(MsvAvTimestamp,       this->Timestamp, 8);
        list.add(MsvAvNbDomainName,    upwin7,          sizeof(upwin7));
        list.add(MsvAvNbComputerName,  upwin7,          sizeof(upwin7));
        list.add(MsvAvDnsDomainName,   win7,            sizeof(win7));
        list.add(MsvAvDnsComputerName, win7,            sizeof(win7));
    }



    // CLIENT BUILD NEGOTIATE
    void ntlm_client_build_negotiate() {
        if (this->server) {
            return;
        }
        this->ntlm_set_negotiate_flags();
        this->NEGOTIATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;
        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
            this->NEGOTIATE_MESSAGE.version.ntlm_get_version_info();
        }
        else {
            this->NEGOTIATE_MESSAGE.version.ignore_version_info();
        }

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
            auto & workstationbuff = this->NEGOTIATE_MESSAGE.Workstation.buffer;
            workstationbuff.reset();
            workstationbuff.ostream.out_copy_bytes(this->Workstation.get_data(),
                                                   this->Workstation.size());
            workstationbuff.mark_end();
        }

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED) {
            auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            domain.reset();
            domain.ostream.out_copy_bytes(this->identity.Domain.get_data(),
                                          this->identity.Domain.size());
            domain.mark_end();
        }

        this->state = NTLM_STATE_CHALLENGE;
    }

    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    void ntlm_server_build_challenge() {
        if (!this->server) {
            return;
        }
        if (!this->ntlm_check_nego()) {
            LOG(LOG_ERR, "ERROR CHECK NEGO FLAGS");
        }
        this->ntlm_generate_server_challenge();
        memcpy(this->CHALLENGE_MESSAGE.serverChallenge, this->ServerChallenge, 8);
        this->ntlm_generate_timestamp();
        this->ntlm_construct_challenge_target_info();

        this->CHALLENGE_MESSAGE.negoFlags.flags = this->NegotiateFlags;
        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
            this->CHALLENGE_MESSAGE.version.ntlm_get_version_info();
        }
        else {
            this->CHALLENGE_MESSAGE.version.ignore_version_info();
        }

        this->state = NTLM_STATE_AUTHENTICATE;
    }

    // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE
    // all strings are in unicode utf16
    void ntlm_client_build_authenticate(const uint8_t * password, size_t pass_size,
                                        const uint8_t * userName, size_t user_size,
                                        const uint8_t * userDomain, size_t domain_size,
                                        const uint8_t * workstation, size_t work_size) {
        if (this->server) {
            return;
        }
        this->ntlmv2_compute_response_from_challenge(password, pass_size,
                                                     userName, user_size,
                                                     userDomain, domain_size);
        this->ntlm_encrypt_random_session_key();
        this->ntlm_generate_client_signing_key();
        this->ntlm_generate_client_sealing_key();
        this->ntlm_generate_server_signing_key();
        this->ntlm_generate_server_sealing_key();
        this->ntlm_init_rc4_seal_states();
        this->ntlm_set_negotiate_flags_auth();
        // this->AUTHENTICATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;

        uint32_t flag = this->AUTHENTICATE_MESSAGE.negoFlags.flags;
        if (flag & NTLMSSP_NEGOTIATE_VERSION) {
            this->AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();
        }
        else {
            this->AUTHENTICATE_MESSAGE.version.ignore_version_info();
        }

        if (!(flag & NTLMSSP_NEGOTIATE_KEY_EXCH)) {
            // If flag is not set, encryted session key buffer is not send
            this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer.reset();
        }
        if (flag & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED) {
            auto & workstationbuff = this->AUTHENTICATE_MESSAGE.Workstation.buffer;
            workstationbuff.reset();
            workstationbuff.ostream.out_copy_bytes(workstation, work_size);
            workstationbuff.mark_end();
        }

        //flag |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
        auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        domain.reset();
        domain.ostream.out_copy_bytes(userDomain, domain_size);
        domain.mark_end();

        auto & user = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        user.reset();
        user.ostream.out_copy_bytes(userName, user_size);
        user.mark_end();

        // this->AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();

        this->state = NTLM_STATE_FINAL;
    }

    void ntlm_server_fetch_hash(uint8_t (&hash)[SslMd4::DIGEST_LENGTH]) {
        // TODO get password hash from DC or find ourself
        // LOG(LOG_INFO, "MARK %u, %u, %u",
        //     this->identity.User.size(),
        //     this->identity.Domain.size(),
        //     this->identity.Password.size());

        if (this->identity.Password.size() > 0) {
            // password is available
            this->hash_password(this->identity.Password.get_data(),
                                this->identity.Password.size(),
                                hash);
        }
    }

    // SERVER PROCEED RESPONSE CHECKING
    SEC_STATUS ntlm_server_proceed_authenticate(const uint8_t (&hash)[16]) {
        if (!this->server) {
            return SEC_E_INTERNAL_ERROR;
        }
        if (!this->ntlm_check_nt_response_from_authenticate(hash, 16)) {
            LOG(LOG_ERR, "NT RESPONSE NOT MATCHING STOP AUTHENTICATE");
            return SEC_E_LOGON_DENIED;
        }
        if (!this->ntlm_check_lm_response_from_authenticate(hash, 16)) {
            LOG(LOG_ERR, "LM RESPONSE NOT MATCHING STOP AUTHENTICATE");
            return SEC_E_LOGON_DENIED;
        }
        // SERVER COMPUTE SHARED KEY WITH CLIENT
        this->ntlm_compute_session_base_key(hash, 16);
        this->ntlm_decrypt_exported_session_key();

        this->ntlm_generate_client_signing_key();
        this->ntlm_generate_client_sealing_key();
        this->ntlm_generate_server_signing_key();
        this->ntlm_generate_server_sealing_key();
        this->ntlm_init_rc4_seal_states();
        if (this->UseMIC) {
            this->ntlm_compute_MIC();
            if (memcmp(this->MessageIntegrityCheck,
                       this->AUTHENTICATE_MESSAGE.MIC, 16)) {
                LOG(LOG_ERR, "MIC NOT MATCHING STOP AUTHENTICATE");
                hexdump_c(this->MessageIntegrityCheck, 16);
                hexdump_c(this->AUTHENTICATE_MESSAGE.MIC, 16);
                return SEC_E_MESSAGE_ALTERED;
            }
        }
        this->state = NTLM_STATE_FINAL;
        return SEC_I_COMPLETE_NEEDED;
    }

    void ntlm_SetContextWorkstation(const uint8_t * workstation) {
        // CHECK UTF8 or UTF16 (should store in UTF16)
        if (workstation) {
            size_t host_len = UTF8Len(workstation);
            this->Workstation.init(host_len * 2);
            UTF8toUTF16(workstation, this->Workstation.get_data(), host_len * 2);
            this->SendWorkstationName = true;
        }
        else {
            this->Workstation.init(0);
            this->SendWorkstationName = false;
        }
    }
    void ntlm_SetContextServicePrincipalName(const uint8_t * pszTargetName) {
        // CHECK UTF8 or UTF16 (should store in UTF16)
        if (pszTargetName) {
            size_t host_len = UTF8Len(pszTargetName);
            this->ServicePrincipalName.init(host_len * 2);
            UTF8toUTF16(pszTargetName, this->ServicePrincipalName.get_data(), host_len * 2);
        }
        else {
            this->ServicePrincipalName.init(0);
        }
    }


    // READ WRITE FUNCTIONS
    SEC_STATUS write_negotiate(PSecBuffer output_buffer) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Write Negotiate");
        }
        this->ntlm_client_build_negotiate();
        StaticOutStream<65535> out_stream;
        this->NEGOTIATE_MESSAGE.emit(out_stream);
        output_buffer->Buffer.init(out_stream.get_offset());
        output_buffer->Buffer.copy(out_stream.get_data(), out_stream.get_offset());

        this->SavedNegotiateMessage.init(out_stream.get_offset());
        this->SavedNegotiateMessage.copy(out_stream.get_data(), out_stream.get_offset());
        this->state = NTLM_STATE_CHALLENGE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS read_negotiate(PSecBuffer input_buffer) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Read Negotiate");
        }
        InStream in_stream(input_buffer->Buffer.get_data(),
                           input_buffer->Buffer.size());
        this->NEGOTIATE_MESSAGE.recv(in_stream);
        if (!this->ntlm_check_nego())
            return SEC_E_INVALID_TOKEN;

        this->SavedNegotiateMessage.init(in_stream.get_offset());
        this->SavedNegotiateMessage.copy(in_stream.get_data(), in_stream.get_offset());

        this->state = NTLM_STATE_CHALLENGE;
        return SEC_I_CONTINUE_NEEDED;
    }
    SEC_STATUS write_challenge(PSecBuffer output_buffer) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Write Challenge");
        }
        this->ntlm_server_build_challenge();
        StaticOutStream<65535> out_stream;
        this->CHALLENGE_MESSAGE.emit(out_stream);
        output_buffer->Buffer.init(out_stream.get_offset());
        output_buffer->Buffer.copy(out_stream.get_data(), out_stream.get_offset());

        this->SavedChallengeMessage.init(out_stream.get_offset());
        this->SavedChallengeMessage.copy(out_stream.get_data(), out_stream.get_offset());

        this->state = NTLM_STATE_AUTHENTICATE;
        return SEC_I_CONTINUE_NEEDED;
    }
    SEC_STATUS read_challenge(PSecBuffer input_buffer) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Read Challenge");
        }
        InStream in_stream(input_buffer->Buffer.get_data(),
                           input_buffer->Buffer.size());
        this->CHALLENGE_MESSAGE.recv(in_stream);
        this->SavedChallengeMessage.init(in_stream.get_offset());
        this->SavedChallengeMessage.copy(in_stream.get_data(), in_stream.get_offset());

        this->state = NTLM_STATE_AUTHENTICATE;
        return SEC_I_CONTINUE_NEEDED;
    }
    SEC_STATUS write_authenticate(PSecBuffer output_buffer) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Write Authenticate");
        }
        SEC_WINNT_AUTH_IDENTITY & id = this->identity;
        this->ntlm_client_build_authenticate(id.Password.get_data(),
                                             id.Password.size(),
                                             id.User.get_data(),
                                             id.User.size(),
                                             id.Domain.get_data(),
                                             id.Domain.size(),
                                             this->Workstation.get_data(),
                                             this->Workstation.size());
        StaticOutStream<65535> out_stream;
        if (this->UseMIC) {
            this->AUTHENTICATE_MESSAGE.ignore_mic = true;
            this->AUTHENTICATE_MESSAGE.emit(out_stream);
            this->AUTHENTICATE_MESSAGE.ignore_mic = false;

            this->SavedAuthenticateMessage.init(out_stream.get_offset());
            this->SavedAuthenticateMessage.copy(out_stream.get_data(),
                                                out_stream.get_offset());
            this->ntlm_compute_MIC();
            memcpy(this->AUTHENTICATE_MESSAGE.MIC, this->MessageIntegrityCheck, 16);
            // this->AUTHENTICATE_MESSAGE.has_mic = true;
        }
        out_stream.rewind();
        this->AUTHENTICATE_MESSAGE.ignore_mic = false;
        this->AUTHENTICATE_MESSAGE.emit(out_stream);
        output_buffer->Buffer.init(out_stream.get_offset());
        output_buffer->Buffer.copy(out_stream.get_data(), out_stream.get_offset());
        if (this->verbose) {
            this->AUTHENTICATE_MESSAGE.log();
        }
        return SEC_I_COMPLETE_NEEDED;
    }
    SEC_STATUS read_authenticate(PSecBuffer input_buffer) {
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Read Authenticate");
        }
        InStream in_stream(input_buffer->Buffer.get_data(), input_buffer->Buffer.size());
        this->AUTHENTICATE_MESSAGE.recv(in_stream);
        if (this->AUTHENTICATE_MESSAGE.has_mic) {
            this->UseMIC = true;
            this->SavedAuthenticateMessage.init(in_stream.get_offset());
            constexpr std::size_t null_data_sz = 16;
            uint8_t const null_data[null_data_sz]{0u};
            auto const p = in_stream.get_data();
            std::size_t offset = 0u;
            this->SavedAuthenticateMessage.copy(p + offset, this->AUTHENTICATE_MESSAGE.PayloadOffset, offset);
            offset += this->AUTHENTICATE_MESSAGE.PayloadOffset;
            this->SavedAuthenticateMessage.copy(null_data, null_data_sz, offset);
            offset += null_data_sz;
            this->SavedAuthenticateMessage.copy(p + offset, in_stream.get_offset() - offset, offset);
        }
        this->identity.User.init(this->AUTHENTICATE_MESSAGE.UserName.buffer.size());
        this->identity.User.copy(this->AUTHENTICATE_MESSAGE.UserName.buffer.get_data(),
                                  this->AUTHENTICATE_MESSAGE.UserName.buffer.size());
        // LOG(LOG_INFO, "USER from authenticate size = %u", this->identity.User.size());
        // hexdump_c(this->identity.User.get_data(), this->identity.User.size());
        this->identity.Domain.init(this->AUTHENTICATE_MESSAGE.DomainName.buffer.size());
        this->identity.Domain.copy(this->AUTHENTICATE_MESSAGE.DomainName.buffer.get_data(),
                                    this->AUTHENTICATE_MESSAGE.DomainName.buffer.size());
        // LOG(LOG_INFO, "DOMAIN from authenticate size = %u", this->identity.Domain.size());
        // hexdump_c(this->identity.Domain.get_data(), this->identity.Domain.size());

        if ((this->identity.User.size() == 0) &&
            (this->identity.Domain.size() == 0)) {
            LOG(LOG_ERR, "ANONYMOUS User not allowed");
            return SEC_E_LOGON_DENIED;
        }
        uint8_t hash[16];
        this->ntlm_server_fetch_hash(hash);
        SEC_STATUS status = this->ntlm_server_proceed_authenticate(hash);
        return status;
    }
};
