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

using array_hmac_md5 = std::array<uint8_t, SslMd5::DIGEST_LENGTH>;
static inline array_hmac_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data)
{
    array_hmac_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data);
    hmac_md5.unchecked_final(result.data());
    return result;
}

static inline array_hmac_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data1, array_view_const_u8 data2)
{
    array_hmac_md5 result;
    SslHMAC_Md5 hmac_md5(key);
    hmac_md5.update(data1);
    hmac_md5.update(data2);
    hmac_md5.unchecked_final(result.data());
    return result;
}

static inline array_hmac_md5 HmacMd5(array_view_const_u8 key, array_view_const_u8 data1, array_view_const_u8 data2, array_view_const_u8 data3)
{
    array_hmac_md5 result;
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
    Array ServicePrincipalName;
    SEC_WINNT_AUTH_IDENTITY identity;
    
    bool sspi_context_initialized = false;
    TimeObj & timeobj;
    Random & rand;

    class NTLMContextClient
    {
        const bool NTLMv2 = true;
        bool UseMIC;
    public:
        NtlmState state = NTLM_STATE_INITIAL;

    private:
        uint8_t MachineID[32];
        const bool SendVersionInfo = true;
        const bool confidentiality = true;

        using array16 = uint8_t[16];

    public:
        SslRC4 SendRc4Seal {};
        SslRC4 RecvRc4Seal {};
    private:
        // TODO unused
        array16* RecvSealingKey = nullptr;

    public:
        uint32_t NegotiateFlags = 0;

    private:
        //int LmCompatibilityLevel;
        bool SendWorkstationName = true;
    public:
        Array Workstation;
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
    public:
        uint8_t ServerChallenge[8]{};
    private:
        uint8_t ClientChallenge[8]{};
    public:
        array_hmac_md5 SessionBaseKey; 
    private:
        //uint8_t KeyExchangeKey[16];
        //uint8_t RandomSessionKey[16];
    public:
        uint8_t ExportedSessionKey[16]{};
        uint8_t EncryptedRandomSessionKey[16]{};
        uint8_t ClientSigningKey[16]{};
        uint8_t ClientSealingKey[16]{};
        uint8_t ServerSigningKey[16]{};
        uint8_t ServerSealingKey[16]{};
        array_hmac_md5 MessageIntegrityCheck;
        // uint8_t NtProofStr[16];

        const bool verbose;

    public:
        explicit NTLMContextClient( bool verbose = false)
            : UseMIC(this->NTLMv2/* == true*/)
            //, LmCompatibilityLevel(3)
            , Workstation(0)
            , ServicePrincipalName(0)
            , SavedNegotiateMessage(0)
            , SavedChallengeMessage(0)
            , SavedAuthenticateMessage(0)
            //, KeyExchangeKey()
            //, RandomSessionKey()
            //, SendSingleHostData(false)
            , verbose(verbose)
        {
            memset(this->MachineID, 0xAA, sizeof(this->MachineID));
            memset(this->MessageIntegrityCheck.data(), 0x00, this->MessageIntegrityCheck.size());

            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Init");
        }

        NTLMContextClient(NTLMContextClient const &) = delete;
        NTLMContextClient& operator = (NTLMContextClient const &) = delete;


        // static method for both client and server (encrypt and decrypt)
        void ntlm_rc4k(uint8_t* key, int length, uint8_t* plaintext, uint8_t* ciphertext)
        {
            SslRC4 rc4;
            // TODO check size
            rc4.set_key({key, 16});
            rc4.crypt(length, plaintext, ciphertext);
        }

        /**
         * Initialize RC4 stream cipher states for sealing.
         */

        void ntlm_init_rc4_seal_states()
        {
            this->SendRc4Seal.set_key(make_array_view(this->ClientSealingKey));
            this->RecvRc4Seal.set_key(make_array_view(this->ServerSealingKey));
        }

        // server check nt response
        bool ntlm_check_nt_response_from_authenticate(array_view_const_u8 hash) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Check NtResponse");
            auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
            size_t temp_size = AuthNtResponse.size() - 16;

            uint8_t NtProofStr_from_msg[16] = {};
            InStream in_AuthNtResponse(AuthNtResponse.ostream.get_current(), AuthNtResponse.ostream.tailroom());
            in_AuthNtResponse.in_copy_bytes(NtProofStr_from_msg, 16);

            auto unique_temp = std::make_unique<uint8_t[]>(temp_size);
            uint8_t* temp = unique_temp.get();
            in_AuthNtResponse.in_copy_bytes(temp, temp_size);
            AuthNtResponse.ostream.rewind();

            array_view_const_u8 user = this->AUTHENTICATE_MESSAGE.UserName.buffer.av();
            
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2 Hash");

            auto userNameUppercase = ::UTF16_to_upper(user);
            array_hmac_md5 ResponseKeyNT = ::HmacMd5(hash, userNameUppercase, this->AUTHENTICATE_MESSAGE.DomainName.buffer.av());
            array_hmac_md5 NtProofStr = ::HmacMd5(ResponseKeyNT, make_array_view(this->ServerChallenge), {temp, temp_size});

            return 0 == memcmp(NtProofStr.data(), NtProofStr_from_msg, 16);
        }

        // Server check lm response
        bool ntlm_check_lm_response_from_authenticate(array_view_const_u8 hash) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Check LmResponse");
            auto & AuthLmResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
            size_t lm_response_size = AuthLmResponse.size(); // should be 24
            if (lm_response_size != 24) {
                return false;
            }
            uint8_t response[16] = {};
            InStream in_AuthLmResponse(AuthLmResponse.ostream.get_current(), AuthLmResponse.ostream.tailroom());
            in_AuthLmResponse.in_copy_bytes(response, 16);
            in_AuthLmResponse.in_copy_bytes(this->ClientChallenge, 8);
            AuthLmResponse.ostream.rewind();

            array_view_const_u8 user = this->AUTHENTICATE_MESSAGE.UserName.buffer.av();

            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2 Hash");

            auto userNameUppercase = ::UTF16_to_upper(user);
            array_hmac_md5 ResponseKeyLM = ::HmacMd5(hash, userNameUppercase, this->AUTHENTICATE_MESSAGE.DomainName.buffer.av());
            array_hmac_md5 compute_response = ::HmacMd5(make_array_view(ResponseKeyLM),{this->ServerChallenge, 8},{this->ClientChallenge, 8});

            return !memcmp(response, compute_response.data(), 16);
        }

        // server compute Session Base Key
        void ntlm_compute_session_base_key(array_view_const_u8 hash) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute Session Base Key");
            auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
            auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            uint8_t NtProofStr[16] = {};

            InStream(AuthNtResponse.ostream.get_current(), AuthNtResponse.ostream.tailroom())
                .in_copy_bytes(NtProofStr, 16);
            AuthNtResponse.ostream.rewind();

            array_view_const_u8 user = this->AUTHENTICATE_MESSAGE.UserName.buffer.av();
            
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2 Hash");

            auto userNameUppercase = ::UTF16_to_upper(user);
            array_hmac_md5 ResponseKeyNT = ::HmacMd5(hash, userNameUppercase, DomainName.av());
            
            // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain), NtProofStr)
            this->SessionBaseKey = ::HmacMd5(make_array_view(ResponseKeyNT), {NtProofStr, sizeof(NtProofStr)});
        }


        // server method
        bool ntlm_check_nego() {
            uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
            uint32_t const mask = NTLMSSP_REQUEST_TARGET
                                | NTLMSSP_NEGOTIATE_NTLM
                                | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                                | NTLMSSP_NEGOTIATE_UNICODE;
            if ((negoFlag & mask) != mask) {
                return false;
            }
            this->NegotiateFlags = negoFlag;
            return true;
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
            this->NegotiateFlags |= (this->NTLMv2)
                                   * (NTLMSSP_NEGOTIATE_56
                                   |  NTLMSSP_NEGOTIATE_VERSION
                                   |  NTLMSSP_NEGOTIATE_LM_KEY
                                   |  NTLMSSP_NEGOTIATE_OEM)
               | (
                 NTLMSSP_NEGOTIATE_KEY_EXCH
               | NTLMSSP_NEGOTIATE_128
               | NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
               | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
               | NTLMSSP_NEGOTIATE_NTLM
               | NTLMSSP_NEGOTIATE_SIGN
               | NTLMSSP_REQUEST_TARGET
               | NTLMSSP_NEGOTIATE_UNICODE);

            if (this->confidentiality) {
                this->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
            }

            if (this->SendVersionInfo) {
                this->NegotiateFlags |= NTLMSSP_NEGOTIATE_VERSION;
            }

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
                auto & workstationbuff = this->NEGOTIATE_MESSAGE.Workstation.buffer;
                workstationbuff.reset();
                workstationbuff.ostream.out_copy_bytes(this->Workstation.get_data(),
                                                       this->Workstation.size());
                workstationbuff.mark_end();
            }

            if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED) {
                auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
                domain.reset();
                auto domain_av = this->identity.get_domain_utf16_av();
                domain.ostream.out_copy_bytes(domain_av);
                domain.mark_end();
            }

            this->state = NTLM_STATE_CHALLENGE;
        }

        // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE
        // all strings are in unicode utf16

        void ntlm_SetContextWorkstation(array_view_const_char workstation) {
            // CHECK UTF8 or UTF16 (should store in UTF16)
            if (!workstation.empty()) {
                size_t host_len = UTF8Len(workstation.data());
                this->Workstation.init(host_len * 2);
                UTF8toUTF16(workstation, this->Workstation.get_data(), host_len * 2);
                this->SendWorkstationName = true;
            }
            else {
                this->Workstation.init(0);
                this->SendWorkstationName = false;
            }
        }

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

        // READ WRITE FUNCTIONS
        SEC_STATUS write_negotiate(Array& output_buffer) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Write Negotiate");
            this->ntlm_client_build_negotiate();
            StaticOutStream<65535> out_stream;
            this->NEGOTIATE_MESSAGE.emit(out_stream);
            output_buffer.init(out_stream.get_offset());
            output_buffer.copy(out_stream.get_bytes());

            this->SavedNegotiateMessage.init(out_stream.get_offset());
            this->SavedNegotiateMessage.copy(out_stream.get_bytes());
            this->state = NTLM_STATE_CHALLENGE;
            return SEC_I_CONTINUE_NEEDED;
        }

        SEC_STATUS read_challenge(array_view_const_u8 input_buffer) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Read Challenge");
            InStream in_stream(input_buffer);
            this->CHALLENGE_MESSAGE.recv(in_stream);
            this->SavedChallengeMessage.init(in_stream.get_offset());
            this->SavedChallengeMessage.copy(in_stream.get_bytes());

            this->state = NTLM_STATE_AUTHENTICATE;
            return SEC_I_CONTINUE_NEEDED;
        }

        SEC_STATUS write_authenticate(Array& output_buffer, Random & rand, TimeObj & timeobj) {
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Write Authenticate");
            auto password = this->identity.get_password_utf16_av();
            auto userName = this->identity.get_user_utf16_av();
            auto userDomain = this->identity.get_domain_utf16_av();
            auto workstation = this->Workstation.av();
 
            // client method
            // ntlmv2_compute_response_from_challenge generates :
            // - timestamp
            // - client challenge
            // - NtChallengeResponse
            // - LmChallengeResponse
            // all strings are in unicode utf16

            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient Compute response from challenge");
            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2");
            array_md4 md4password = ::Md4(password);
            auto userNameUppercase = ::UTF16_to_upper(userName);
            array_hmac_md5 ResponseKeyNT = ::HmacMd5(md4password,userNameUppercase,userDomain);

            LOG_IF(this->verbose, LOG_INFO, "NTLMContextClient NTOWFv2");

            array_md4 md4password_b = ::Md4(password);
            auto userNameUppercase_b = ::UTF16_to_upper(userName);
            array_hmac_md5 ResponseKeyLM = ::HmacMd5(md4password_b,userNameUppercase_b,userDomain);

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

            array_hmac_md5 NtProofStr = ::HmacMd5(make_array_view(ResponseKeyNT),{this->ServerChallenge, 8},{temp, temp_size});


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

            array_hmac_md5 LCResponse = ::HmacMd5(ResponseKeyLM, {this->ServerChallenge, 8}, {this->ClientChallenge, 8});

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
            rand.random(this->ExportedSessionKey, SslMd5::DIGEST_LENGTH);
            this->ntlm_rc4k(this->SessionBaseKey.data(), SslMd5::DIGEST_LENGTH,
                            this->ExportedSessionKey, this->EncryptedRandomSessionKey);

            auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
            AuthEncryptedRSK.reset();
            AuthEncryptedRSK.ostream.out_copy_bytes(this->EncryptedRandomSessionKey, 16);
            AuthEncryptedRSK.mark_end();

            // NTLM Client Signing Key @msdn{cc236711}
            SslMd5 md5sign_client;
            md5sign_client.update({this->ExportedSessionKey, 16});
            md5sign_client.update("session key to client-to-server signing key magic constant\0"_av);
            md5sign_client.final(this->ClientSigningKey);

            // NTLM Client Sealing Key @msdn{cc236712}
            SslMd5 md5seal_client;
            md5seal_client.update(make_array_view(this->ExportedSessionKey));
            md5seal_client.update("session key to client-to-server sealing key magic constant\0"_av);
            md5seal_client.final(this->ClientSealingKey);

            // NTLM Server signing key @msdn{cc236711}

            SslMd5 md5sign_server;
            md5sign_server.update({this->ExportedSessionKey, 16});
            md5sign_server.update("session key to server-to-client signing key magic constant\0"_av);
            md5sign_server.final(this->ServerSigningKey);

            // NTLM Server Sealing Key @msdn{cc236712}
            
            SslMd5 md5seal_server;
            md5seal_server.update(make_array_view(this->ExportedSessionKey));
            md5seal_server.update("session key to server-to-client sealing key magic constant\0"_av);
            md5seal_server.final(this->ServerSealingKey);
            
            this->SendRc4Seal.set_key(make_array_view(this->ClientSealingKey));
            this->RecvRc4Seal.set_key(make_array_view(this->ServerSealingKey));

            
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
                workstationbuff.ostream.out_copy_bytes(workstation);
                workstationbuff.mark_end();
            }

            //flag |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
            auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            domain.reset();
            domain.ostream.out_copy_bytes(userDomain);
            domain.mark_end();

            auto & user = this->AUTHENTICATE_MESSAGE.UserName.buffer;
            user.reset();
            user.ostream.out_copy_bytes(userName);
            user.mark_end();

            // this->AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();

            this->state = NTLM_STATE_FINAL;
                                                 
            StaticOutStream<65535> out_stream;
            if (this->UseMIC) {
                this->AUTHENTICATE_MESSAGE.ignore_mic = true;
                this->AUTHENTICATE_MESSAGE.emit(out_stream);
                this->AUTHENTICATE_MESSAGE.ignore_mic = false;

                this->SavedAuthenticateMessage.init(out_stream.get_offset());
                this->SavedAuthenticateMessage.copy(out_stream.get_bytes());

                this->MessageIntegrityCheck = ::HmacMd5(make_array_view(this->ExportedSessionKey), 
                                                        this->SavedNegotiateMessage.av(),
                                                        this->SavedChallengeMessage.av(),
                                                        this->SavedAuthenticateMessage.av());

                memcpy(this->AUTHENTICATE_MESSAGE.MIC, this->MessageIntegrityCheck.data(), this->MessageIntegrityCheck.size());
                // this->AUTHENTICATE_MESSAGE.has_mic = true;
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

    } sspi_context;
        
    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;
    // Inlined

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext;
    SEC_STATUS sspi_InitializeSecurityContext(
        array_view_const_char pszTargetName, array_view_const_u8 input_buffer, Array& output_buffer
    )
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::InitializeSecurityContext");

        if (!this->sspi_context_initialized) {

            this->sspi_context.ntlm_SetContextWorkstation(pszTargetName);
            this->sspi_context.ntlm_SetContextServicePrincipalName(pszTargetName);

            this->sspi_context.identity.CopyAuthIdentity(this->identity.get_user_utf16_av(),
                                                    this->identity.get_domain_utf16_av(),
                                                    this->identity.get_password_utf16_av());
            this->sspi_context_initialized = true;
        }

        if (this->sspi_context.state == NTLM_STATE_INITIAL) {
            this->sspi_context.state = NTLM_STATE_NEGOTIATE;
        }
        if (this->sspi_context.state == NTLM_STATE_NEGOTIATE) {
            return this->sspi_context.write_negotiate(output_buffer);
        }

        if (this->sspi_context.state == NTLM_STATE_CHALLENGE) {
            this->sspi_context.read_challenge(input_buffer);
        }
        if (this->sspi_context.state == NTLM_STATE_AUTHENTICATE) {
            return this->sspi_context.write_authenticate(output_buffer, this->rand, this->timeobj);
        }

        return SEC_E_OUT_OF_SEQUENCE;
    }

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
        LOG_IF(this->sspi_context.verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

        if (data_in.size() < cbMaxSignature) {
            return {SEC_E_INVALID_TOKEN, data_out};
        }

        // data_in [signature][data_buffer]

        auto data_buffer = data_in.array_from_offset(cbMaxSignature);
        data_out.resize(data_buffer.size(), 0);

        /* Decrypt message using with RC4, result overwrites original buffer */
        // this->sspi_context.confidentiality == true
        this->sspi_context.RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.data());

        std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        array_hmac_md5 digest = ::HmacMd5(this->sspi_context.ServerSigningKey, seqno, data_out);

        /* Concatenate version, ciphertext and sequence number to build signature */
        std::array<uint8_t,16> expected_signature{
            1, 0, 0, 0, // Version
            0, 0, 0, 0, 0, 0, 0, 0,
            uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        this->sspi_context.RecvRc4Seal.crypt(8, digest.data(), &expected_signature[4]);


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

    void SetHostnameFromUtf8(const uint8_t * pszTargetName) {
        size_t length = (pszTargetName && *pszTargetName) ? strlen(char_ptr_cast(pszTargetName)) : 0;
        this->ServicePrincipalName.init(length + 1);
        this->ServicePrincipalName.copy({pszTargetName, length});
        this->ServicePrincipalName.get_data()[length] = 0;
    }

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
        array_hmac_md5 digest = ::HmacMd5(this->sspi_context.ClientSigningKey, seqno, public_key);
        
        this->sspi_context.SendRc4Seal.crypt(public_key.size(), public_key.data(), data_out.data()+cbMaxSignature);
        this->sspi_compute_signature(data_out.data(), this->sspi_context.SendRc4Seal, digest.data(), MessageSeqNo);
        
//        /* Concatenate version, ciphertext and sequence number to build signature */
//        std::array<uint8_t,16> expected_signature{
//            1, 0, 0, 0, // Version
//            0, 0, 0, 0, 0, 0, 0, 0,
//            uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
//        this->sspi_context.RecvRc4Seal.crypt(8, digest, signature.data()+4);

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
        LOG_IF(this->sspi_context.verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

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
        // this->sspi_context.confidentiality == true
        this->sspi_context.RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), data_out.data());

        std::array<uint8_t,4> seqno{uint8_t(MessageSeqNo),uint8_t(MessageSeqNo>>8),uint8_t(MessageSeqNo>>16),uint8_t(MessageSeqNo>>24)};
        array_hmac_md5 digest = ::HmacMd5(this->sspi_context.ServerSigningKey, seqno, data_out);

        uint8_t expected_signature[16] = {};
        this->sspi_compute_signature(
            expected_signature, this->sspi_context.RecvRc4Seal, digest.data(), MessageSeqNo);
            
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

    void credssp_buffer_free() {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::buffer_free");
        this->ts_request.negoTokens.init(0);
        this->ts_request.pubKeyAuth.init(0);
        this->ts_request.authInfo.init(0);
        this->ts_request.clientNonce.reset();
        this->ts_request.error_code = 0;
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
        this->credssp_buffer_free();

        if (status != SEC_E_OK) {
            LOG(LOG_ERR, "Could not verify public key echo!");
            this->credssp_buffer_free();
            return Res::Err;
        }

        /* Send encrypted credentials */

        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::encrypt_ts_credentials");
        if (this->restricted_admin_mode) {
            LOG(LOG_INFO, "Restricted Admin Mode");
            this->ts_credentials.set_credentials_from_av({},{},{});
        }
        else {
            this->ts_credentials.set_credentials_from_av(this->identity.get_domain_utf16_av(),
                                                        this->identity.get_user_utf16_av(),
                                                        this->identity.get_password_utf16_av());
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
        array_hmac_md5 digest = ::HmacMd5(this->sspi_context.ClientSigningKey, seqno, data_in);

        this->sspi_context.SendRc4Seal.crypt(data_in.size(), data_in.data(), data_out.data()+cbMaxSignature);
        this->sspi_compute_signature(data_out.data(), this->sspi_context.SendRc4Seal, digest.data(), MessageSeqNo);
        
        this->ts_request.authInfo.init(data_out.size());
        this->ts_request.authInfo.copy(const_bytes_view{data_out.data(),data_out.size()});

        LOG_IF(this->verbose, LOG_INFO, "rdpCredssp - Client Authentication : Sending Credentials");
        StaticOutStream<65536> ts_request_emit;
        this->ts_request.emit(ts_request_emit);
        transport.get_transport().send(ts_request_emit.get_bytes());
        this->credssp_buffer_free();

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
        , sspi_context(verbose)
        , restricted_admin_mode(restricted_admin_mode)
        , target_host(target_host)
        , extra_message(extra_message)
        , lang(lang)
        , verbose(verbose)
    {
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::Initialization");
        LOG_IF(this->verbose, LOG_INFO, "rdpCredsspClientNTLM::set_credentials");
        this->identity.SetUserFromUtf8(user);
        this->identity.SetDomainFromUtf8(domain);
        this->identity.SetPasswordFromUtf8(pass);
        this->SetHostnameFromUtf8(hostname);
        this->identity.SetKrbAuthIdentity(user, pass);

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
        SEC_STATUS status1 = this->sspi_InitializeSecurityContext(
            bytes_view(this->ServicePrincipalName.av()).as_chars(),
            this->client_auth_data_input_buffer,
            /*output*/this->ts_request.negoTokens);
        SEC_STATUS encrypted = SEC_E_INVALID_TOKEN;

        if ((status1 != SEC_I_COMPLETE_AND_CONTINUE) &&
            (status1 != SEC_I_COMPLETE_NEEDED) &&
            (status1 != SEC_E_OK) &&
            (status1 != SEC_I_CONTINUE_NEEDED)) {
            LOG(LOG_ERR, "Initialize Security Context Error !");
            throw ERR_CREDSSP_NTLM_INIT_FAILED;
        }

        if ((status1 == SEC_I_COMPLETE_AND_CONTINUE) ||
            (status1 == SEC_I_COMPLETE_NEEDED) ||
            (status1 == SEC_E_OK)) {
            // have_pub_key_auth = true;
            encrypted = this->credssp_encrypt_public_key_echo();
            if (status1 == SEC_I_COMPLETE_NEEDED) {
                status1 = SEC_E_OK;
            }
            else if (status1 == SEC_I_COMPLETE_AND_CONTINUE) {
                status1 = SEC_I_CONTINUE_NEEDED;
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

            this->credssp_buffer_free();
        }

        if (status1 != SEC_I_CONTINUE_NEEDED) {
            LOG_IF(this->verbose, LOG_INFO, "rdpCredssp Token loop: CONTINUE_NEEDED");

            this->client_auth_data_state = Final;
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

                SEC_STATUS status = this->sspi_InitializeSecurityContext(
                    bytes_view(this->ServicePrincipalName.av()).as_chars(),
                    this->client_auth_data_input_buffer,
                    /*output*/this->ts_request.negoTokens);

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

                    this->credssp_buffer_free();
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

