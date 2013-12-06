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

#ifndef _REDEMPTION_CORE_RDP_NLA_NTLM_NTLM_HPP_
#define _REDEMPTION_CORE_RDP_NLA_NTLM_NTLM_HPP_

#include "ssl_calls.hpp"
#include "genrandom.hpp"
#include "difftimeval.hpp"

#include "RDP/nla/ntlm/ntlm_message.hpp"
#include "RDP/nla/ntlm/ntlm_message_negotiate.hpp"
#include "RDP/nla/ntlm/ntlm_message_challenge.hpp"
#include "RDP/nla/ntlm/ntlm_message_authenticate.hpp"

enum NtlmState {
    NTLM_STATE_INITIAL,
    NTLM_STATE_NEGOTIATE,
    NTLM_STATE_CHALLENGE,
    NTLM_STATE_AUTHENTICATE,
    NTLM_STATE_FINAL
};
static const uint8_t lm_magic[] = "KGS!@#$%";

static const uint8_t client_sign_magic[] = "session key to client-to-server signing key magic constant";
static const uint8_t server_sign_magic[] = "session key to server-to-client signing key magic constant";
static const uint8_t client_seal_magic[] = "session key to client-to-server sealing key magic constant";
static const uint8_t server_seal_magic[] = "session key to server-to-client sealing key magic constant";
struct NTLMContext {

    UdevRandom randgen;

    bool server;
    bool NTLMv2;
    bool UseMIC;
    NtlmState state;

    int SendSeqNum;
    int RecvSeqNum;

    uint8_t MachineID[32];
    bool SendVersionInfo;
    bool confidentiality;

    // RC4_KEY SendRc4Seal;
    // RC4_KEY RecvRc4Seal;
    uint8_t* SendSigningKey;
    uint8_t* RecvSigningKey;
    uint8_t* SendSealingKey;
    uint8_t* RecvSealingKey;
    uint32_t NegotiateFlags;
    int LmCompatibilityLevel;
    int SuppressExtendedProtection;
    bool SendWorkstationName;
    // UNICODE_STRING Workstation;
    // UNICODE_STRING ServicePrincipalName;
    // SEC_WINNT_AUTH_IDENTITY identity;
    uint8_t* ChannelBindingToken;
    uint8_t ChannelBindingsHash[16];
    // SecPkgContext_Bindings Bindings;
    bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
    NTLMNegotiateMessage NEGOTIATE_MESSAGE;
    NTLMChallengeMessage CHALLENGE_MESSAGE;
    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE;
    BStream BuffNegotiateMessage;
    BStream BuffChallengeMessage;
    BStream BuffAuthenticateMessage;
    BStream BuffChallengeTargetInfo;
    BStream BuffAuthenticateTargetInfo;
    BStream BuffTargetName;
    BStream BuffNtChallengeResponse;
    BStream BuffLmChallengeResponse;
    uint8_t Timestamp[8];
    uint8_t ChallengeTimestamp[8];
    uint8_t ServerChallenge[8];
    uint8_t ClientChallenge[8];
    uint8_t SessionBaseKey[16];
    uint8_t KeyExchangeKey[16];
    uint8_t RandomSessionKey[16];
    uint8_t ExportedSessionKey[16];
    uint8_t EncryptedRandomSessionKey[16];
    uint8_t ClientSigningKey[16];
    uint8_t ClientSealingKey[16];
    uint8_t ServerSigningKey[16];
    uint8_t ServerSealingKey[16];
    uint8_t MessageIntegrityCheck[16];





#if 0

    /**
     * Output Restriction_Encoding.\n
     * Restriction_Encoding @msdn{cc236647}
     * @param NTLM context
     */

    void ntlm_output_restriction_encoding()
    {
	// wStream* s;
	// AV_PAIR* restrictions = &context->av_pairs->Restrictions;

	// BYTE machineID[32] =
        //     "\x3A\x15\x8E\xA6\x75\x82\xD8\xF7\x3E\x06\xFA\x7A\xB4\xDF\xFD\x43"
        //     "\x84\x6C\x02\x3A\xFD\x5A\x94\xFE\xCF\x97\x0F\x3D\x19\x2C\x38\x20";

	// restrictions->value = malloc(48);
	// restrictions->length = 48;

	// s = PStreamAllocAttach(restrictions->value, restrictions->length);

	// Stream_Write_UINT32(s, 48); /* Size */
	// Stream_Zero(s, 4); /* Z4 (set to zero) */

	// /* IntegrityLevel (bit 31 set to 1) */
	// Stream_Write_UINT8(s, 1);
	// Stream_Zero(s, 3);

	// Stream_Write_UINT32(s, 0x00002000); /* SubjectIntegrityLevel */
	// Stream_Write(s, machineID, 32); /* MachineID */

	// PStreamFreeDetach(s);
    }

#endif




    /**
     * Generate timestamp for AUTHENTICATE_MESSAGE.
     * @param NTLM context
     */

    void ntlm_generate_timestamp()
    {
        uint8_t ZeroTimestamp[8] = {};

	if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0)
            memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
	else {
            timeval tv = tvtime();
            struct {
                uint32_t low;
                uint32_t high;
            } timestamp;
            timestamp.low = tv.tv_usec;
            timestamp.high = tv.tv_sec;
            memcpy(this->Timestamp, &timestamp, sizeof(timestamp));
        }
    }

    void ntlm_generate_client_challenge()
    {
	// /* ClientChallenge is used in computation of LMv2 and NTLMv2 responses */
        this->randgen.random(this->ClientChallenge, 8);
    }

    void ntlm_generate_server_challenge()
    {
        this->randgen.random(this->ServerChallenge, 8);
    }

    void ntlm_get_server_challenge() {
        memcpy(this->ServerChallenge, this->CHALLENGE_MESSAGE.serverChallenge, 8);
    }

    void ntlm_generate_random_session_key()
    {
        this->randgen.random(this->RandomSessionKey, 16);
    }

    void ntlm_generate_exported_session_key() {
        this->randgen.random(this->ExportedSessionKey, 16);
    }
    // void ntlm_generate_exported_session_key()
    // {
    //     memcpy(this->ExportedSessionKey, this->RandomSessionKey, 16);
    // }

    void ntlm_generate_key_exchange_key()
    {
	// /* In NTLMv2, KeyExchangeKey is the 128-bit SessionBaseKey */
	memcpy(this->KeyExchangeKey, this->SessionBaseKey, 16);
    }


    void NTOWFv2(const uint8_t * pass,   size_t pass_size,
                 const uint8_t * user,   size_t user_size,
                 const uint8_t * domain, size_t domain_size,
                 uint8_t * buff, size_t buff_size) {
        SslMd4 md4;
        uint8_t md4password[16] = {};
        // pass UNICODE (UTF16)
        md4.update(pass, pass_size);
        md4.final(md4password, sizeof(md4password));
        SslHMAC_Md5 hmac_md5(md4password, sizeof(md4password));

        // TODO user to uppercase !!!!!
        // user and domain in UNICODE UTF16
        hmac_md5.update(user, user_size);
        hmac_md5.update(domain, domain_size);
        hmac_md5.final(buff, buff_size);
    }
    void LMOWFv2(const uint8_t * pass,   size_t pass_size,
                 const uint8_t * user,   size_t user_size,
                 const uint8_t * domain, size_t domain_size,
                 uint8_t * buff, size_t buff_size) {
        NTOWFv2(pass, pass_size, user, user_size, domain, domain_size,
                buff, buff_size);
    }
    void ntlmv2_compute_response_from_challenge(const uint8_t * pass,   size_t pass_size,
                                                const uint8_t * user,   size_t user_size,
                                                const uint8_t * domain, size_t domain_size) {
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
        BStream AvPairsStream;
        this->CHALLENGE_MESSAGE.AvPairList.emit(AvPairsStream);
        uint8_t temp_size = 1 + 1 + 6 + 8 + 8 + 4 + AvPairsStream.size() + 4;
        uint8_t * temp = new uint8_t[temp_size];
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
        uint8_t NtProofStr[16] = {};
        SslHMAC_Md5 hmac_md5resp(ResponseKeyNT, sizeof(ResponseKeyNT));
        // TODO take ServerChallenge from ChallengeMessage
        this->ntlm_get_server_challenge();
        hmac_md5resp.update(this->ServerChallenge, 8);
        hmac_md5resp.update(temp, temp_size);
        hmac_md5resp.final(NtProofStr, sizeof(NtProofStr));

        // NtChallengeResponse = Concat(NtProofStr, temp)
        BStream & NtChallengeResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.Buffer;
        // BStream & NtChallengeResponse = this->BuffNtChallengeResponse;
        NtChallengeResponse.reset();
        NtChallengeResponse.out_copy_bytes(NtProofStr, sizeof(NtProofStr));
        NtChallengeResponse.out_copy_bytes(temp, temp_size);
        NtChallengeResponse.mark_end();

        delete temp;

        // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
        //                                         Concat(ServerChallenge, ClientChallenge))
        // LmChallengeResponse.ChallengeFromClient = ClientChallenge
        BStream & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.Buffer;
        // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
        SslHMAC_Md5 hmac_md5lmresp(ResponseKeyLM, sizeof(ResponseKeyLM));
        LmChallengeResponse.reset();
        hmac_md5lmresp.update(this->ServerChallenge, 8);
        hmac_md5lmresp.update(this->ClientChallenge, 8);
        uint8_t LCResponse[16] = {};
        hmac_md5lmresp.final(LCResponse, 16);
        LmChallengeResponse.out_copy_bytes(LCResponse, 16);
        LmChallengeResponse.out_copy_bytes(this->ClientChallenge, 8);
        LmChallengeResponse.mark_end();

        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                           NtProofStr)
        SslHMAC_Md5 hmac_md5seskey(ResponseKeyNT, sizeof(ResponseKeyNT));
        hmac_md5seskey.update(NtProofStr, sizeof(NtProofStr));
        hmac_md5seskey.final(this->SessionBaseKey, 16);
    }


    void ntlm_encrypt_random_session_key() {
        // EncryptedRandomSessionKey = RC4K(KeyExchangeKey, ExportedSessionKey)
        // ExportedSessionKey = NONCE(16) (random 16bytes number)
        // KeyExchangeKey = SessionBaseKey
        // EncryptedRandomSessionKey = RC4K(SessionBaseKey, NONCE(16))
        SslRC4 rc4;
        rc4.set_key(this->SessionBaseKey, 16);
        // generate NONCE(16) exportedsessionkey
        this->ntlm_generate_exported_session_key();
        rc4.crypt(16, this->ExportedSessionKey, this->EncryptedRandomSessionKey);
    }



    /**
     * Generate signing key.\n
     * @msdn{cc236711}
     * @param exported_session_key ExportedSessionKey
     * @param sign_magic Sign magic string
     * @param signing_key Destination signing key
     */

    void ntlm_generate_signing_key(const uint8_t * sign_magic, size_t magic_size, uint8_t* signing_key)
    {
        SslMd5 md5sign;
        md5sign.update(this->ExportedSessionKey, 16);
        md5sign.update(sign_magic, magic_size);
        md5sign.final(signing_key, 16);
    }


    /**
     * Generate client signing key (ClientSigningKey).\n
     * @msdn{cc236711}
     * @param NTLM context
     */

    void ntlm_generate_client_signing_key()
    {
	this->ntlm_generate_signing_key(client_sign_magic, sizeof(client_sign_magic),
                                        this->ClientSigningKey);
    }

    /**
     * Generate server signing key (ServerSigningKey).\n
     * @msdn{cc236711}
     * @param NTLM context
     */

    void ntlm_generate_server_signing_key()
    {
	ntlm_generate_signing_key(server_sign_magic, sizeof(server_sign_magic),
                                  this->ServerSigningKey);
    }


    /**
     * Generate sealing key.\n
     * @msdn{cc236712}
     * @param exported_session_key ExportedSessionKey
     * @param seal_magic Seal magic string
     * @param sealing_key Destination sealing key
     */

    void ntlm_generate_sealing_key(const uint8_t * seal_magic, size_t magic_size, uint8_t* sealing_key)
    {
        SslMd5 md5seal;
        md5seal.update(this->ExportedSessionKey, 16);
        md5seal.update(seal_magic, magic_size);
        md5seal.final(sealing_key, 16);
    }

    /**
     * Generate client sealing key (ClientSealingKey).\n
     * @msdn{cc236712}
     * @param NTLM context
     */

    void ntlm_generate_client_sealing_key()
    {
	ntlm_generate_signing_key(client_seal_magic, sizeof(client_seal_magic),
                                  this->ClientSealingKey);
    }

    /**
     * Generate server sealing key (ServerSealingKey).\n
     * @msdn{cc236712}
     * @param NTLM context
     */

    void ntlm_generate_server_sealing_key()
    {
	ntlm_generate_signing_key(server_seal_magic, sizeof(server_seal_magic),
                                  this->ServerSealingKey);
    }





    void ntlm_fetch_ntlm_v2_hash(char* hash)
    {
// 	WINPR_SAM* sam;
// 	WINPR_SAM_ENTRY* entry;

// 	sam = SamOpen(1);
// 	if (sam == NULL)
//             return;

// 	entry = SamLookupUserW(sam,
//                                (LPWSTR) context->identity.User, context->identity.UserLength * 2,
//                                (LPWSTR) context->identity.Domain, context->identity.DomainLength * 2);

// 	if (entry != NULL)
//             {
// #ifdef WITH_DEBUG_NTLM
// 		fprintf(stderr, "NTLM Hash:\n");
// 		winpr_HexDump(entry->NtHash, 16);
// #endif

// 		NTOWFv2FromHashW(entry->NtHash,
//                                  (LPWSTR) context->identity.User, context->identity.UserLength * 2,
//                                  (LPWSTR) context->identity.Domain, context->identity.DomainLength * 2,
//                                  (BYTE*) hash);

// 		SamFreeEntry(sam, entry);
// 		SamClose(sam);

// 		return;
//             }

// 	entry = SamLookupUserW(sam,
//                                (LPWSTR) context->identity.User, context->identity.UserLength * 2, NULL, 0);

// 	if (entry != NULL)
//             {
// #ifdef WITH_DEBUG_NTLM
// 		fprintf(stderr, "NTLM Hash:\n");
// 		winpr_HexDump(entry->NtHash, 16);
// #endif

// 		NTOWFv2FromHashW(entry->NtHash,
//                                  (LPWSTR) context->identity.User, context->identity.UserLength * 2,
//                                  (LPWSTR) context->identity.Domain, context->identity.DomainLength * 2,
//                                  (BYTE*) hash);

// 		SamFreeEntry(sam, entry);
// 		SamClose(sam);

// 		return;
//             }
// 	else
//             {
// 		fprintf(stderr, "Error: Could not find user in SAM database\n");
//             }
// 	SamClose(sam);
    }

    void ntlm_convert_password_hash(uint8_t* hash)
    {
	// int i, hn, ln;
	// char* PasswordHash = NULL;
	// UINT32 PasswordHashLength = 0;

	// /* Password contains a password hash of length (PasswordLength / SSPI_CREDENTIALS_HASH_LENGTH_FACTOR) */

	// PasswordHashLength = context->identity.PasswordLength / SSPI_CREDENTIALS_HASH_LENGTH_FACTOR;
	// ConvertFromUnicode(CP_UTF8, 0, context->identity.Password, PasswordHashLength, &PasswordHash, 0, NULL, NULL);
	// CharUpperBuffA(PasswordHash, PasswordHashLength);

	// for (i = 0; i < 32; i += 2)
        //     {
	// 	hn = PasswordHash[i] > '9' ? PasswordHash[i] - 'A' + 10 : PasswordHash[i] - '0';
	// 	ln = PasswordHash[i + 1] > '9' ? PasswordHash[i + 1] - 'A' + 10 : PasswordHash[i + 1] - '0';
	// 	hash[i / 2] = (hn << 4) | ln;
        //     }

	// free(PasswordHash);
    }

    void ntlm_compute_ntlm_v2_hash(char* hash)
    {
	// if (context->identity.PasswordLength > 256)
        //     {
	// 	BYTE PasswordHash[16];

	// 	/* Special case for WinPR: password hash */
	// 	ntlm_convert_password_hash(context, PasswordHash);

	// 	NTOWFv2FromHashW(PasswordHash,
        //                          (LPWSTR) context->identity.User, context->identity.UserLength * 2,
        //                          (LPWSTR) context->identity.Domain, context->identity.DomainLength * 2,
        //                          (BYTE*) hash);
        //     }
	// else if (context->identity.PasswordLength > 0)
        //     {
	// 	NTOWFv2W((LPWSTR) context->identity.Password, context->identity.PasswordLength * 2,
        //                  (LPWSTR) context->identity.User, context->identity.UserLength * 2,
        //                  (LPWSTR) context->identity.Domain, context->identity.DomainLength * 2, (BYTE*) hash);
        //     }
	// else
        //     {
	// 	ntlm_fetch_ntlm_v2_hash(context, hash);
        //     }
    }

    void ntlm_compute_lm_v2_response()
    {
	// char* response;
	// char value[16];
	// char ntlm_v2_hash[16];

	// if (context->LmCompatibilityLevel < 2)
        //     {
	// 	sspi_SecBufferAlloc(&context->LmChallengeResponse, 24);
	// 	ZeroMemory(context->LmChallengeResponse.pvBuffer, 24);
	// 	return;
        //     }

	// /* Compute the NTLMv2 hash */
	// ntlm_compute_ntlm_v2_hash(context, ntlm_v2_hash);

	// /* Concatenate the server and client challenges */
	// CopyMemory(value, context->ServerChallenge, 8);
	// CopyMemory(&value[8], context->ClientChallenge, 8);

	// sspi_SecBufferAlloc(&context->LmChallengeResponse, 24);
	// response = (char*) context->LmChallengeResponse.pvBuffer;

	// /* Compute the HMAC-MD5 hash of the resulting value using the NTLMv2 hash as the key */
	// HMAC(EVP_md5(), (void*) ntlm_v2_hash, 16, (void*) value, 16, (void*) response, NULL);

	// /* Concatenate the resulting HMAC-MD5 hash and the client challenge, giving us the LMv2 response (24 bytes) */
	// CopyMemory(&response[16], context->ClientChallenge, 8);
    }

    /**
     * Compute NTLMv2 Response.\n
     * NTLMv2_RESPONSE @msdn{cc236653}\n
     * NTLMv2 Authentication @msdn{cc236700}
     * @param NTLM context
     */

    void ntlm_compute_ntlm_v2_response()
    {
	// uint8_t* blob;
	// uint8_t ntlm_v2_hash[16];
	// uint8_t nt_proof_str[16];
        // BStream ntlm_v2_temp;
        // BStream ntlm_v2_temp_chal;
        // BStream TargetInfo;

	// TargetInfo = this->ChallengeTargetInfo;

// 	sspi_SecBufferAlloc(&ntlm_v2_temp, TargetInfo->cbBuffer + 28);

// 	ZeroMemory(ntlm_v2_temp.pvBuffer, ntlm_v2_temp.cbBuffer);
// 	blob = (BYTE*) ntlm_v2_temp.pvBuffer;

// 	/* Compute the NTLMv2 hash */
// 	ntlm_compute_ntlm_v2_hash(context, (char*) ntlm_v2_hash);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "Password (length = %d)\n", context->identity.PasswordLength * 2);
// 	winpr_HexDump((BYTE*) context->identity.Password, context->identity.PasswordLength * 2);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "Username (length = %d)\n", context->identity.UserLength * 2);
// 	winpr_HexDump((BYTE*) context->identity.User, context->identity.UserLength * 2);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "Domain (length = %d)\n", context->identity.DomainLength * 2);
// 	winpr_HexDump((BYTE*) context->identity.Domain, context->identity.DomainLength * 2);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "Workstation (length = %d)\n", context->Workstation.Length);
// 	winpr_HexDump((BYTE*) context->Workstation.Buffer, context->Workstation.Length);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "NTOWFv2, NTLMv2 Hash\n");
// 	winpr_HexDump(ntlm_v2_hash, 16);
// 	fprintf(stderr, "\n");
// #endif

// 	/* Construct temp */
// 	blob[0] = 1; /* RespType (1 byte) */
// 	blob[1] = 1; /* HighRespType (1 byte) */
// 	/* Reserved1 (2 bytes) */
// 	/* Reserved2 (4 bytes) */
// 	CopyMemory(&blob[8], context->Timestamp, 8); /* Timestamp (8 bytes) */
// 	CopyMemory(&blob[16], context->ClientChallenge, 8); /* ClientChallenge (8 bytes) */
// 	/* Reserved3 (4 bytes) */
// 	CopyMemory(&blob[28], TargetInfo->pvBuffer, TargetInfo->cbBuffer);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "NTLMv2 Response Temp Blob\n");
// 	winpr_HexDump(ntlm_v2_temp.pvBuffer, ntlm_v2_temp.cbBuffer);
// 	fprintf(stderr, "\n");
// #endif

// 	/* Concatenate server challenge with temp */
// 	sspi_SecBufferAlloc(&ntlm_v2_temp_chal, ntlm_v2_temp.cbBuffer + 8);
// 	blob = (BYTE*) ntlm_v2_temp_chal.pvBuffer;
// 	CopyMemory(blob, context->ServerChallenge, 8);
// 	CopyMemory(&blob[8], ntlm_v2_temp.pvBuffer, ntlm_v2_temp.cbBuffer);

// 	HMAC(EVP_md5(), (void*) ntlm_v2_hash, 16, ntlm_v2_temp_chal.pvBuffer,
//              ntlm_v2_temp_chal.cbBuffer, (void*) nt_proof_str, NULL);

// 	/* NtChallengeResponse, Concatenate NTProofStr with temp */
// 	sspi_SecBufferAlloc(&context->NtChallengeResponse, ntlm_v2_temp.cbBuffer + 16);
// 	blob = (BYTE*) context->NtChallengeResponse.pvBuffer;
// 	CopyMemory(blob, nt_proof_str, 16);
// 	CopyMemory(&blob[16], ntlm_v2_temp.pvBuffer, ntlm_v2_temp.cbBuffer);

// 	/* Compute SessionBaseKey, the HMAC-MD5 hash of NTProofStr using the NTLMv2 hash as the key */
// 	HMAC(EVP_md5(), (void*) ntlm_v2_hash, 16, (void*) nt_proof_str, 16, (void*) context->SessionBaseKey, NULL);

// 	sspi_SecBufferFree(&ntlm_v2_temp);
// 	sspi_SecBufferFree(&ntlm_v2_temp_chal);
    }

    /**
     * Encrypt the given plain text using RC4 and the given key.
     * @param key RC4 key
     * @param length text length
     * @param plaintext plain text
     * @param ciphertext cipher text
     */

    void ntlm_rc4k(uint8_t* key, int length, uint8_t* plaintext, uint8_t* ciphertext)
    {
	// RC4_KEY rc4;

	// /* Initialize RC4 cipher with key */
	// RC4_set_key(&rc4, 16, (void*) key);

	// /* Encrypt plaintext with key */
	// RC4(&rc4, length, (void*) plaintext, (void*) ciphertext);
    }

    /**
     * Generate client challenge (8-byte nonce).
     * @param NTLM context
     */


    /**
     * Generate server challenge (8-byte nonce).
     * @param NTLM context
     */




    /**
     * Generate RandomSessionKey (16-byte nonce).
     * @param NTLM context
     */


    /**
     * Decrypt RandomSessionKey (RC4-encrypted RandomSessionKey, using KeyExchangeKey as the key).
     * @param NTLM context
     */

    void ntlm_decrypt_random_session_key()
    {
	// /* In NTLMv2, EncryptedRandomSessionKey is the ExportedSessionKey RC4-encrypted with the KeyExchangeKey */
	// ntlm_rc4k(context->KeyExchangeKey, 16, context->EncryptedRandomSessionKey, context->RandomSessionKey);
    }





    /**
     * Initialize RC4 stream cipher states for sealing.
     * @param NTLM context
     */

    void ntlm_init_rc4_seal_states()
    {
	// if (context->server)
        //     {
	// 	context->SendSigningKey = context->ServerSigningKey;
	// 	context->RecvSigningKey = context->ClientSigningKey;
	// 	context->SendSealingKey = context->ClientSealingKey;
	// 	context->RecvSealingKey = context->ServerSealingKey;
	// 	RC4_set_key(&context->SendRc4Seal, 16, context->ServerSealingKey);
	// 	RC4_set_key(&context->RecvRc4Seal, 16, context->ClientSealingKey);
        //     }
	// else
        //     {
	// 	context->SendSigningKey = context->ClientSigningKey;
	// 	context->RecvSigningKey = context->ServerSigningKey;
	// 	context->SendSealingKey = context->ServerSealingKey;
	// 	context->RecvSealingKey = context->ClientSealingKey;
	// 	RC4_set_key(&context->SendRc4Seal, 16, context->ClientSealingKey);
	// 	RC4_set_key(&context->RecvRc4Seal, 16, context->ServerSealingKey);
        //     }
    }

    void ntlm_compute_message_integrity_check()
    {
	// HMAC_CTX hmac_ctx;

	// /*
	//  * Compute the HMAC-MD5 hash of ConcatenationOf(NEGOTIATE_MESSAGE,
	//  * CHALLENGE_MESSAGE, AUTHENTICATE_MESSAGE) using the ExportedSessionKey
	//  */

	// HMAC_CTX_init(&hmac_ctx);
	// HMAC_Init_ex(&hmac_ctx, context->ExportedSessionKey, 16, EVP_md5(), NULL);
	// HMAC_Update(&hmac_ctx, context->NegotiateMessage.pvBuffer, context->NegotiateMessage.cbBuffer);
	// HMAC_Update(&hmac_ctx, context->ChallengeMessage.pvBuffer, context->ChallengeMessage.cbBuffer);
	// HMAC_Update(&hmac_ctx, context->AuthenticateMessage.pvBuffer, context->AuthenticateMessage.cbBuffer);
	// HMAC_Final(&hmac_ctx, context->MessageIntegrityCheck, NULL);
	// HMAC_CTX_cleanup(&hmac_ctx);
    }




//     SECURITY_STATUS ntlm_read_NegotiateMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	NTLM_NEGOTIATE_MESSAGE* message;

// 	message = &context->NEGOTIATE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_NEGOTIATE_MESSAGE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	ntlm_read_message_header(s, (NTLM_MESSAGE_HEADER*) message);

// 	if (!ntlm_validate_message_header(s, (NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_NEGOTIATE))
//             {
// 		Stream_Free(s, FALSE);
// 		return SEC_E_INVALID_TOKEN;
//             }

// 	Stream_Read_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	if (!((message->NegotiateFlags & NTLMSSP_REQUEST_TARGET) &&
//               (message->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM) &&
//               (message->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN) &&
//               (message->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE)))
//             {
// 		Stream_Free(s, FALSE);
// 		return SEC_E_INVALID_TOKEN;
//             }

// 	context->NegotiateFlags = message->NegotiateFlags;

// 	/* only set if NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED is set */

// 	/* DomainNameFields (8 bytes) */
// 	ntlm_read_message_fields(s, &(message->DomainName));

// 	/* only set if NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED is set */

// 	/* WorkstationFields (8 bytes) */
// 	ntlm_read_message_fields(s, &(message->Workstation));

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_read_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	length = Stream_GetPosition(s);
// 	buffer->cbBuffer = length;

// 	sspi_SecBufferAlloc(&context->NegotiateMessage, length);
// 	CopyMemory(context->NegotiateMessage.pvBuffer, buffer->pvBuffer, buffer->cbBuffer);
// 	context->NegotiateMessage.BufferType = buffer->BufferType;

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "NEGOTIATE_MESSAGE (length = %d)\n", (int) context->NegotiateMessage.cbBuffer);
// 	winpr_HexDump(context->NegotiateMessage.pvBuffer, context->NegotiateMessage.cbBuffer);
// 	fprintf(stderr, "\n");

// 	ntlm_print_negotiate_flags(message->NegotiateFlags);

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));
// #endif

// 	context->state = NTLM_STATE_CHALLENGE;

// 	Stream_Free(s, FALSE);

// 	return SEC_I_CONTINUE_NEEDED;
//     }

//     SECURITY_STATUS ntlm_write_NegotiateMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	NTLM_NEGOTIATE_MESSAGE* message;

// 	message = &context->NEGOTIATE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_NEGOTIATE_MESSAGE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	ntlm_populate_message_header((NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_NEGOTIATE);

// 	if (context->NTLMv2)
//             {
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_56;
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_VERSION;
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_LM_KEY;
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM;
//             }

// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_KEY_EXCH;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_128;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_NTLM;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
// 	message->NegotiateFlags |= NTLMSSP_REQUEST_TARGET;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_UNICODE;

// 	if (context->confidentiality)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;

// 	if (context->SendVersionInfo)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_VERSION;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_get_version_info(&(message->Version));

// 	context->NegotiateFlags = message->NegotiateFlags;

// 	/* Message Header (12 bytes) */
// 	ntlm_write_message_header(s, (NTLM_MESSAGE_HEADER*) message);

// 	Stream_Write_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	/* only set if NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED is set */

// 	/* DomainNameFields (8 bytes) */
// 	ntlm_write_message_fields(s, &(message->DomainName));

// 	/* only set if NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED is set */

// 	/* WorkstationFields (8 bytes) */
// 	ntlm_write_message_fields(s, &(message->Workstation));

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_write_version_info(s, &(message->Version));

// 	length = Stream_GetPosition(s);
// 	buffer->cbBuffer = length;

// 	sspi_SecBufferAlloc(&context->NegotiateMessage, length);
// 	CopyMemory(context->NegotiateMessage.pvBuffer, buffer->pvBuffer, buffer->cbBuffer);
// 	context->NegotiateMessage.BufferType = buffer->BufferType;

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "NEGOTIATE_MESSAGE (length = %d)\n", length);
// 	winpr_HexDump(Stream_Buffer(s), length);
// 	fprintf(stderr, "\n");

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));
// #endif

// 	context->state = NTLM_STATE_CHALLENGE;

// 	Stream_Free(s, FALSE);

// 	return SEC_I_CONTINUE_NEEDED;
//     }

//     SECURITY_STATUS ntlm_read_ChallengeMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	PBYTE StartOffset;
// 	PBYTE PayloadOffset;
// 	NTLM_AV_PAIR* AvTimestamp;
// 	NTLM_CHALLENGE_MESSAGE* message;

// 	ntlm_generate_client_challenge(context);

// 	message = &context->CHALLENGE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_CHALLENGE_MESSAGE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	StartOffset = Stream_Pointer(s);

// 	ntlm_read_message_header(s, (NTLM_MESSAGE_HEADER*) message);

// 	if (!ntlm_validate_message_header(s, (NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_CHALLENGE))
//             {
// 		Stream_Free(s, FALSE);
// 		return SEC_E_INVALID_TOKEN;
//             }

// 	/* TargetNameFields (8 bytes) */
// 	ntlm_read_message_fields(s, &(message->TargetName));

// 	Stream_Read_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */
// 	context->NegotiateFlags = message->NegotiateFlags;

// 	Stream_Read(s, message->ServerChallenge, 8); /* ServerChallenge (8 bytes) */
// 	CopyMemory(context->ServerChallenge, message->ServerChallenge, 8);

// 	Stream_Read(s, message->Reserved, 8); /* Reserved (8 bytes), should be ignored */

// 	/* TargetInfoFields (8 bytes) */
// 	ntlm_read_message_fields(s, &(message->TargetInfo));

// 	if (context->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_read_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	/* Payload (variable) */
// 	PayloadOffset = Stream_Pointer(s);

// 	if (message->TargetName.Len > 0)
//             ntlm_read_message_fields_buffer(s, &(message->TargetName));

// 	if (message->TargetInfo.Len > 0)
//             {
// 		ntlm_read_message_fields_buffer(s, &(message->TargetInfo));

// 		context->ChallengeTargetInfo.pvBuffer = message->TargetInfo.Buffer;
// 		context->ChallengeTargetInfo.cbBuffer = message->TargetInfo.Len;

// 		AvTimestamp = ntlm_av_pair_get((NTLM_AV_PAIR*) message->TargetInfo.Buffer, MsvAvTimestamp);

// 		if (AvTimestamp != NULL)
//                     {
// 			if (context->NTLMv2)
//                             context->UseMIC = TRUE;

// 			CopyMemory(context->ChallengeTimestamp, ntlm_av_pair_get_value_pointer(AvTimestamp), 8);
//                     }
//             }

// 	length = (PayloadOffset - StartOffset) + message->TargetName.Len + message->TargetInfo.Len;

// 	sspi_SecBufferAlloc(&context->ChallengeMessage, length);
// 	CopyMemory(context->ChallengeMessage.pvBuffer, StartOffset, length);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "CHALLENGE_MESSAGE (length = %d)\n", length);
// 	winpr_HexDump(context->ChallengeMessage.pvBuffer, context->ChallengeMessage.cbBuffer);
// 	fprintf(stderr, "\n");

// 	ntlm_print_negotiate_flags(context->NegotiateFlags);

// 	if (context->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));

// 	ntlm_print_message_fields(&(message->TargetName), "TargetName");
// 	ntlm_print_message_fields(&(message->TargetInfo), "TargetInfo");

// 	if (context->ChallengeTargetInfo.cbBuffer > 0)
//             {
// 		fprintf(stderr, "ChallengeTargetInfo (%d):\n", (int) context->ChallengeTargetInfo.cbBuffer);
// 		ntlm_print_av_pair_list(context->ChallengeTargetInfo.pvBuffer);
//             }
// #endif
// 	/* AV_PAIRs */

// 	if (context->NTLMv2)
//             {
// 		ntlm_construct_authenticate_target_info(context);
// 		sspi_SecBufferFree(&context->ChallengeTargetInfo);
// 		context->ChallengeTargetInfo.pvBuffer = context->AuthenticateTargetInfo.pvBuffer;
// 		context->ChallengeTargetInfo.cbBuffer = context->AuthenticateTargetInfo.cbBuffer;
//             }

// 	/* Timestamp */
// 	ntlm_generate_timestamp(context);

// 	/* LmChallengeResponse */
// 	ntlm_compute_lm_v2_response(context);

// 	/* NtChallengeResponse */
// 	ntlm_compute_ntlm_v2_response(context);

// 	/* KeyExchangeKey */
// 	ntlm_generate_key_exchange_key(context);

// 	/* RandomSessionKey */
// 	ntlm_generate_random_session_key(context);

// 	/* ExportedSessionKey */
// 	ntlm_generate_exported_session_key(context);

// 	/* EncryptedRandomSessionKey */
// 	ntlm_encrypt_random_session_key(context);

// 	/* Generate signing keys */
// 	ntlm_generate_client_signing_key(context);
// 	ntlm_generate_server_signing_key(context);

// 	/* Generate sealing keys */
// 	ntlm_generate_client_sealing_key(context);
// 	ntlm_generate_server_sealing_key(context);

// 	/* Initialize RC4 seal state using client sealing key */
// 	ntlm_init_rc4_seal_states(context);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "ClientChallenge\n");
// 	winpr_HexDump(context->ClientChallenge, 8);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerChallenge\n");
// 	winpr_HexDump(context->ServerChallenge, 8);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "SessionBaseKey\n");
// 	winpr_HexDump(context->SessionBaseKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "KeyExchangeKey\n");
// 	winpr_HexDump(context->KeyExchangeKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ExportedSessionKey\n");
// 	winpr_HexDump(context->ExportedSessionKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "RandomSessionKey\n");
// 	winpr_HexDump(context->RandomSessionKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ClientSigningKey\n");
// 	winpr_HexDump(context->ClientSigningKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ClientSealingKey\n");
// 	winpr_HexDump(context->ClientSealingKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerSigningKey\n");
// 	winpr_HexDump(context->ServerSigningKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerSealingKey\n");
// 	winpr_HexDump(context->ServerSealingKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "Timestamp\n");
// 	winpr_HexDump(context->Timestamp, 8);
// 	fprintf(stderr, "\n");
// #endif

// 	context->state = NTLM_STATE_AUTHENTICATE;

// 	ntlm_free_message_fields_buffer(&(message->TargetName));

// 	Stream_Free(s, FALSE);

// 	return SEC_I_CONTINUE_NEEDED;
//     }

//     SECURITY_STATUS ntlm_write_ChallengeMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	UINT32 PayloadOffset;
// 	NTLM_CHALLENGE_MESSAGE* message;

// 	message = &context->CHALLENGE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_CHALLENGE_MESSAGE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	/* Version */
// 	ntlm_get_version_info(&(message->Version));

// 	/* Server Challenge */
// 	ntlm_generate_server_challenge(context);

// 	/* Timestamp */
// 	ntlm_generate_timestamp(context);

// 	/* TargetInfo */
// 	ntlm_construct_challenge_target_info(context);

// 	/* ServerChallenge */
// 	CopyMemory(message->ServerChallenge, context->ServerChallenge, 8);

// 	message->NegotiateFlags = context->NegotiateFlags;

// 	ntlm_populate_message_header((NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_CHALLENGE);

// 	/* Message Header (12 bytes) */
// 	ntlm_write_message_header(s, (NTLM_MESSAGE_HEADER*) message);

// 	if (message->NegotiateFlags & NTLMSSP_REQUEST_TARGET)
//             {
// 		message->TargetName.Len = (UINT16) context->TargetName.cbBuffer;
// 		message->TargetName.Buffer = context->TargetName.pvBuffer;
//             }

// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_TARGET_INFO;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_TARGET_INFO)
//             {
// 		message->TargetInfo.Len = (UINT16) context->ChallengeTargetInfo.cbBuffer;
// 		message->TargetInfo.Buffer = context->ChallengeTargetInfo.pvBuffer;
//             }

// 	PayloadOffset = 48;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             PayloadOffset += 8;

// 	message->TargetName.BufferOffset = PayloadOffset;
// 	message->TargetInfo.BufferOffset = message->TargetName.BufferOffset + message->TargetName.Len;

// 	/* TargetNameFields (8 bytes) */
// 	ntlm_write_message_fields(s, &(message->TargetName));

// 	Stream_Write_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	Stream_Write(s, message->ServerChallenge, 8); /* ServerChallenge (8 bytes) */
// 	Stream_Write(s, message->Reserved, 8); /* Reserved (8 bytes), should be ignored */

// 	/* TargetInfoFields (8 bytes) */
// 	ntlm_write_message_fields(s, &(message->TargetInfo));

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_write_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	/* Payload (variable) */

// 	if (message->NegotiateFlags & NTLMSSP_REQUEST_TARGET)
//             ntlm_write_message_fields_buffer(s, &(message->TargetName));

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_TARGET_INFO)
//             ntlm_write_message_fields_buffer(s, &(message->TargetInfo));

// 	length = Stream_GetPosition(s);
// 	buffer->cbBuffer = length;

// 	sspi_SecBufferAlloc(&context->ChallengeMessage, length);
// 	CopyMemory(context->ChallengeMessage.pvBuffer, Stream_Buffer(s), length);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "CHALLENGE_MESSAGE (length = %d)\n", length);
// 	winpr_HexDump(context->ChallengeMessage.pvBuffer, context->ChallengeMessage.cbBuffer);
// 	fprintf(stderr, "\n");

// 	ntlm_print_negotiate_flags(message->NegotiateFlags);

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));

// 	ntlm_print_message_fields(&(message->TargetName), "TargetName");
// 	ntlm_print_message_fields(&(message->TargetInfo), "TargetInfo");
// #endif

// 	context->state = NTLM_STATE_AUTHENTICATE;

// 	Stream_Free(s, FALSE);

// 	return SEC_I_CONTINUE_NEEDED;
//     }

//     SECURITY_STATUS ntlm_read_AuthenticateMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	UINT32 flags;
// 	UINT32 MicOffset;
// 	NTLM_AV_PAIR* AvFlags;
// 	NTLMv2_RESPONSE response;
// 	UINT32 PayloadBufferOffset;
// 	NTLM_AUTHENTICATE_MESSAGE* message;

// 	flags = 0;
// 	MicOffset = 0;
// 	AvFlags = NULL;

// 	message = &context->AUTHENTICATE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_AUTHENTICATE_MESSAGE));
// 	ZeroMemory(&response, sizeof(NTLMv2_RESPONSE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	ntlm_read_message_header(s, (NTLM_MESSAGE_HEADER*) message);

// 	if (!ntlm_validate_message_header(s, (NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_AUTHENTICATE))
//             {
// 		Stream_Free(s, FALSE);
// 		return SEC_E_INVALID_TOKEN;
//             }

// 	ntlm_read_message_fields(s, &(message->LmChallengeResponse)); /* LmChallengeResponseFields (8 bytes) */

// 	ntlm_read_message_fields(s, &(message->NtChallengeResponse)); /* NtChallengeResponseFields (8 bytes) */

// 	/* only set if NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED is set */

// 	ntlm_read_message_fields(s, &(message->DomainName)); /* DomainNameFields (8 bytes) */

// 	ntlm_read_message_fields(s, &(message->UserName)); /* UserNameFields (8 bytes) */

// 	/* only set if NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED is set */

// 	ntlm_read_message_fields(s, &(message->Workstation)); /* WorkstationFields (8 bytes) */

// 	ntlm_read_message_fields(s, &(message->EncryptedRandomSessionKey)); /* EncryptedRandomSessionKeyFields (8 bytes) */

// 	Stream_Read_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_read_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	PayloadBufferOffset = Stream_GetPosition(s);

// 	ntlm_read_message_fields_buffer(s, &(message->DomainName)); /* DomainName */

// 	ntlm_read_message_fields_buffer(s, &(message->UserName)); /* UserName */

// 	ntlm_read_message_fields_buffer(s, &(message->Workstation)); /* Workstation */

// 	ntlm_read_message_fields_buffer(s, &(message->LmChallengeResponse)); /* LmChallengeResponse */

// 	ntlm_read_message_fields_buffer(s, &(message->NtChallengeResponse)); /* NtChallengeResponse */

// 	if (message->NtChallengeResponse.Len > 0)
//             {
// 		wStream* s = Stream_New(message->NtChallengeResponse.Buffer, message->NtChallengeResponse.Len);
// 		ntlm_read_ntlm_v2_response(s, &response);
// 		Stream_Free(s, FALSE);

// 		context->NtChallengeResponse.pvBuffer = message->NtChallengeResponse.Buffer;
// 		context->NtChallengeResponse.cbBuffer = message->NtChallengeResponse.Len;

// 		context->ChallengeTargetInfo.pvBuffer = (void*) response.Challenge.AvPairs;
// 		context->ChallengeTargetInfo.cbBuffer = message->NtChallengeResponse.Len - (28 + 16);

// 		CopyMemory(context->ClientChallenge, response.Challenge.ClientChallenge, 8);

// 		AvFlags = ntlm_av_pair_get(response.Challenge.AvPairs, MsvAvFlags);

// 		if (AvFlags != NULL)
//                     flags = *((UINT32*) ntlm_av_pair_get_value_pointer(AvFlags));
//             }

// 	/* EncryptedRandomSessionKey */
// 	ntlm_read_message_fields_buffer(s, &(message->EncryptedRandomSessionKey));
// 	CopyMemory(context->EncryptedRandomSessionKey, message->EncryptedRandomSessionKey.Buffer, 16);

// 	length = Stream_GetPosition(s);
// 	sspi_SecBufferAlloc(&context->AuthenticateMessage, length);
// 	CopyMemory(context->AuthenticateMessage.pvBuffer, Stream_Buffer(s), length);
// 	buffer->cbBuffer = length;

// 	Stream_SetPosition(s, PayloadBufferOffset);

// 	if (flags & MSV_AV_FLAGS_MESSAGE_INTEGRITY_CHECK)
//             {
// 		MicOffset = Stream_GetPosition(s);
// 		Stream_Read(s, message->MessageIntegrityCheck, 16);
// 		PayloadBufferOffset += 16;
//             }

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "AUTHENTICATE_MESSAGE (length = %d)\n", (int) context->AuthenticateMessage.cbBuffer);
// 	winpr_HexDump(context->AuthenticateMessage.pvBuffer, context->AuthenticateMessage.cbBuffer);
// 	fprintf(stderr, "\n");

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));

// 	ntlm_print_message_fields(&(message->DomainName), "DomainName");
// 	ntlm_print_message_fields(&(message->UserName), "UserName");
// 	ntlm_print_message_fields(&(message->Workstation), "Workstation");
// 	ntlm_print_message_fields(&(message->LmChallengeResponse), "LmChallengeResponse");
// 	ntlm_print_message_fields(&(message->NtChallengeResponse), "NtChallengeResponse");
// 	ntlm_print_message_fields(&(message->EncryptedRandomSessionKey), "EncryptedRandomSessionKey");

// 	ntlm_print_av_pair_list(response.Challenge.AvPairs);

// 	if (flags & MSV_AV_FLAGS_MESSAGE_INTEGRITY_CHECK)
//             {
// 		fprintf(stderr, "MessageIntegrityCheck:\n");
// 		winpr_HexDump(message->MessageIntegrityCheck, 16);
//             }
// #endif

// 	if (message->UserName.Len > 0)
//             {
// 		context->identity.User = (UINT16*) malloc(message->UserName.Len);
// 		CopyMemory(context->identity.User, message->UserName.Buffer, message->UserName.Len);
// 		context->identity.UserLength = message->UserName.Len / 2;
//             }

// 	if (message->DomainName.Len > 0)
//             {
// 		context->identity.Domain = (UINT16*) malloc(message->DomainName.Len);
// 		CopyMemory(context->identity.Domain, message->DomainName.Buffer, message->DomainName.Len);
// 		context->identity.DomainLength = message->DomainName.Len / 2;
//             }

// 	/* LmChallengeResponse */
// 	ntlm_compute_lm_v2_response(context);

// 	/* NtChallengeResponse */
// 	ntlm_compute_ntlm_v2_response(context);

// 	/* KeyExchangeKey */
// 	ntlm_generate_key_exchange_key(context);

// 	/* EncryptedRandomSessionKey */
// 	ntlm_decrypt_random_session_key(context);

// 	/* ExportedSessionKey */
// 	ntlm_generate_exported_session_key(context);

// 	if (flags & MSV_AV_FLAGS_MESSAGE_INTEGRITY_CHECK)
//             {
// 		ZeroMemory(&((PBYTE) context->AuthenticateMessage.pvBuffer)[MicOffset], 16);
// 		ntlm_compute_message_integrity_check(context);
// 		CopyMemory(&((PBYTE) context->AuthenticateMessage.pvBuffer)[MicOffset], message->MessageIntegrityCheck, 16);

// 		if (memcmp(context->MessageIntegrityCheck, message->MessageIntegrityCheck, 16) != 0)
//                     {
// 			fprintf(stderr, "Message Integrity Check (MIC) verification failed!\n");

// 			fprintf(stderr, "Expected MIC:\n");
// 			winpr_HexDump(context->MessageIntegrityCheck, 16);
// 			fprintf(stderr, "Actual MIC:\n");
// 			winpr_HexDump(message->MessageIntegrityCheck, 16);
// 			Stream_Free(s, FALSE);

// 			return SEC_E_MESSAGE_ALTERED;
//                     }
//             }

// 	/* Generate signing keys */
// 	ntlm_generate_client_signing_key(context);
// 	ntlm_generate_server_signing_key(context);

// 	/* Generate sealing keys */
// 	ntlm_generate_client_sealing_key(context);
// 	ntlm_generate_server_sealing_key(context);

// 	/* Initialize RC4 seal state */
// 	ntlm_init_rc4_seal_states(context);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "ClientChallenge\n");
// 	winpr_HexDump(context->ClientChallenge, 8);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerChallenge\n");
// 	winpr_HexDump(context->ServerChallenge, 8);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "SessionBaseKey\n");
// 	winpr_HexDump(context->SessionBaseKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "KeyExchangeKey\n");
// 	winpr_HexDump(context->KeyExchangeKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ExportedSessionKey\n");
// 	winpr_HexDump(context->ExportedSessionKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "RandomSessionKey\n");
// 	winpr_HexDump(context->RandomSessionKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ClientSigningKey\n");
// 	winpr_HexDump(context->ClientSigningKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ClientSealingKey\n");
// 	winpr_HexDump(context->ClientSealingKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerSigningKey\n");
// 	winpr_HexDump(context->ServerSigningKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerSealingKey\n");
// 	winpr_HexDump(context->ServerSealingKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "Timestamp\n");
// 	winpr_HexDump(context->Timestamp, 8);
// 	fprintf(stderr, "\n");
// #endif

// 	context->state = NTLM_STATE_FINAL;

// 	Stream_Free(s, FALSE);

// 	ntlm_free_message_fields_buffer(&(message->DomainName));
// 	ntlm_free_message_fields_buffer(&(message->UserName));
// 	ntlm_free_message_fields_buffer(&(message->Workstation));
// 	ntlm_free_message_fields_buffer(&(message->LmChallengeResponse));
// 	ntlm_free_message_fields_buffer(&(message->NtChallengeResponse));
// 	ntlm_free_message_fields_buffer(&(message->EncryptedRandomSessionKey));

// 	return SEC_I_COMPLETE_NEEDED;
//     }

//     /**
//      * Send NTLMSSP AUTHENTICATE_MESSAGE.\n
//      * AUTHENTICATE_MESSAGE @msdn{cc236643}
//      * @param NTLM context
//      * @param buffer
//      */

//     SECURITY_STATUS ntlm_write_AuthenticateMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	UINT32 MicOffset = 0;
// 	UINT32 PayloadBufferOffset;
// 	NTLM_AUTHENTICATE_MESSAGE* message;

// 	message = &context->AUTHENTICATE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_AUTHENTICATE_MESSAGE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	if (context->NTLMv2)
//             {
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_56;

// 		if (context->SendVersionInfo)
//                     message->NegotiateFlags |= NTLMSSP_NEGOTIATE_VERSION;
//             }

// 	if (context->UseMIC)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_TARGET_INFO;

// 	if (context->SendWorkstationName)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED;

// 	if (context->confidentiality)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;

// 	if (context->CHALLENGE_MESSAGE.NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_KEY_EXCH;

// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_128;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_NTLM;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
// 	message->NegotiateFlags |= NTLMSSP_REQUEST_TARGET;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_UNICODE;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_get_version_info(&(message->Version));

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED)
//             {
// 		message->Workstation.Len = context->Workstation.Length;
// 		message->Workstation.Buffer = (BYTE*) context->Workstation.Buffer;
//             }

// 	if (context->identity.DomainLength > 0)
//             {
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
// 		message->DomainName.Len = (UINT16) context->identity.DomainLength * 2;
// 		message->DomainName.Buffer = (BYTE*) context->identity.Domain;
//             }

// 	message->UserName.Len = (UINT16) context->identity.UserLength * 2;
// 	message->UserName.Buffer = (BYTE*) context->identity.User;

// 	message->LmChallengeResponse.Len = (UINT16) context->LmChallengeResponse.cbBuffer;
// 	message->LmChallengeResponse.Buffer = (BYTE*) context->LmChallengeResponse.pvBuffer;

// 	//if (context->NTLMv2)
// 	//	ZeroMemory(message->LmChallengeResponse.Buffer, message->LmChallengeResponse.Len);

// 	message->NtChallengeResponse.Len = (UINT16) context->NtChallengeResponse.cbBuffer;
// 	message->NtChallengeResponse.Buffer = (BYTE*) context->NtChallengeResponse.pvBuffer;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
//             {
// 		message->EncryptedRandomSessionKey.Len = 16;
// 		message->EncryptedRandomSessionKey.Buffer = context->EncryptedRandomSessionKey;
//             }

// 	PayloadBufferOffset = 64;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             PayloadBufferOffset += 8; /* Version (8 bytes) */

// 	if (context->UseMIC)
//             PayloadBufferOffset += 16; /* Message Integrity Check (16 bytes) */

// 	message->DomainName.BufferOffset = PayloadBufferOffset;
// 	message->UserName.BufferOffset = message->DomainName.BufferOffset + message->DomainName.Len;
// 	message->Workstation.BufferOffset = message->UserName.BufferOffset + message->UserName.Len;
// 	message->LmChallengeResponse.BufferOffset = message->Workstation.BufferOffset + message->Workstation.Len;
// 	message->NtChallengeResponse.BufferOffset = message->LmChallengeResponse.BufferOffset + message->LmChallengeResponse.Len;
// 	message->EncryptedRandomSessionKey.BufferOffset = message->NtChallengeResponse.BufferOffset + message->NtChallengeResponse.Len;

// 	ntlm_populate_message_header((NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_AUTHENTICATE);

// 	ntlm_write_message_header(s, (NTLM_MESSAGE_HEADER*) message); /* Message Header (12 bytes) */

// 	ntlm_write_message_fields(s, &(message->LmChallengeResponse)); /* LmChallengeResponseFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->NtChallengeResponse)); /* NtChallengeResponseFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->DomainName)); /* DomainNameFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->UserName)); /* UserNameFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->Workstation)); /* WorkstationFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->EncryptedRandomSessionKey)); /* EncryptedRandomSessionKeyFields (8 bytes) */

// 	Stream_Write_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_write_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	if (context->UseMIC)
//             {
// 		MicOffset = Stream_GetPosition(s);
// 		Stream_Zero(s, 16); /* Message Integrity Check (16 bytes) */
//             }

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED)
//             ntlm_write_message_fields_buffer(s, &(message->DomainName)); /* DomainName */

// 	ntlm_write_message_fields_buffer(s, &(message->UserName)); /* UserName */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED)
//             ntlm_write_message_fields_buffer(s, &(message->Workstation)); /* Workstation */

// 	ntlm_write_message_fields_buffer(s, &(message->LmChallengeResponse)); /* LmChallengeResponse */

// 	ntlm_write_message_fields_buffer(s, &(message->NtChallengeResponse)); /* NtChallengeResponse */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
//             ntlm_write_message_fields_buffer(s, &(message->EncryptedRandomSessionKey)); /* EncryptedRandomSessionKey */

// 	length = Stream_GetPosition(s);
// 	sspi_SecBufferAlloc(&context->AuthenticateMessage, length);
// 	CopyMemory(context->AuthenticateMessage.pvBuffer, Stream_Buffer(s), length);
// 	buffer->cbBuffer = length;

// 	if (context->UseMIC)
//             {
// 		/* Message Integrity Check */
// 		ntlm_compute_message_integrity_check(context);

// 		Stream_SetPosition(s, MicOffset);
// 		Stream_Write(s, context->MessageIntegrityCheck, 16);
// 		Stream_SetPosition(s, length);
//             }

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "AUTHENTICATE_MESSAGE (length = %d)\n", length);
// 	winpr_HexDump(Stream_Buffer(s), length);
// 	fprintf(stderr, "\n");

// 	ntlm_print_negotiate_flags(message->NegotiateFlags);

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));

// 	if (context->AuthenticateTargetInfo.cbBuffer > 0)
//             {
// 		fprintf(stderr, "AuthenticateTargetInfo (%d):\n", (int) context->AuthenticateTargetInfo.cbBuffer);
// 		ntlm_print_av_pair_list(context->AuthenticateTargetInfo.pvBuffer);
//             }

// 	ntlm_print_message_fields(&(message->DomainName), "DomainName");
// 	ntlm_print_message_fields(&(message->UserName), "UserName");
// 	ntlm_print_message_fields(&(message->Workstation), "Workstation");
// 	ntlm_print_message_fields(&(message->LmChallengeResponse), "LmChallengeResponse");
// 	ntlm_print_message_fields(&(message->NtChallengeResponse), "NtChallengeResponse");
// 	ntlm_print_message_fields(&(message->EncryptedRandomSessionKey), "EncryptedRandomSessionKey");

// 	if (context->UseMIC)
//             {
// 		fprintf(stderr, "MessageIntegrityCheck (length = 16)\n");
// 		winpr_HexDump(context->MessageIntegrityCheck, 16);
// 		fprintf(stderr, "\n");
//             }
// #endif

// 	context->state = NTLM_STATE_FINAL;

// 	Stream_Free(s, FALSE);

// 	return SEC_I_COMPLETE_NEEDED;
//     }


};




#endif
