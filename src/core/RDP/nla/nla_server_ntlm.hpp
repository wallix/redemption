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


static inline std::vector<AvPair> && operator<<(std::vector<AvPair>&& v, array_view<AvPair const> a)
{
    v.insert(v.end(), a.begin(), a.end());
    return std::move(v);
}

static inline std::vector<AvPair> & operator<<(std::vector<AvPair> & v, array_view<AvPair const> a)
{
    v.insert(v.end(), a.begin(), a.end());
    return v;
}

class NtlmServer final
{
    static constexpr uint32_t cbMaxSignature = 16;

    int send_seq_num = 0;
    int recv_seq_num = 0;

    TSCredentials ts_credentials;

public:
    bool is_domain;
    bool is_server;
    std::vector<NTLM_AV_ID> avFieldsTags;
    std::vector<uint8_t> TargetName;
    std::vector<uint8_t> netbiosComputerName;
    std::vector<uint8_t> netbiosDomainName;
    std::vector<uint8_t> dnsComputerName;
    std::vector<uint8_t> dnsDomainName;
    std::vector<uint8_t> dnsTreeName;
    uint32_t credssp_version;
    const NtlmVersion ntlm_version;
//    TSRequest ts_request = {6}; // Credssp Version 6 Supported
    bool ignore_bogus_nego_flags = false;
private:
    uint32_t error_code = 0;
    static const size_t CLIENT_NONCE_LENGTH = 32;
    ClientNonce SavedClientNonce;

    TimeObj & timeobj;
    Random & rand;
    array_view_u8 public_key;

    private:
    std::function<PasswordCallback(bytes_view,bytes_view,std::vector<uint8_t>&)> set_password_cb;
    const bool verbose;

    array_sha256 ClientServerHash;

public:

    credssp::State state = credssp::State::Cont; 
    struct ServerAuthenticateData
    {
        enum : uint8_t { Start, Loop, Final } state = Start;
    };

    ServerAuthenticateData server_auth_data;

    enum class Res : bool { Err, Ok };

protected:
    SEC_STATUS state_accept_security_context = SEC_I_INCOMPLETE_CREDENTIALS;

    const bool NTLMv2 = true;
    bool UseMIC = true; // NTLMv2
public:
    NtlmState ntlm_state = NTLM_STATE_INITIAL;

private:
    uint8_t MachineID[32];

public:
    SslRC4 SendRc4Seal {};
    SslRC4 RecvRc4Seal {};

public:

    std::vector<uint8_t> identity_User;
    std::vector<uint8_t> identity_Domain;
    std::vector<uint8_t> identity_Password;

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;

public:
    std::vector<uint8_t> SavedNegotiateMessage;
    std::vector<uint8_t> SavedChallengeMessage;

private:
    uint8_t Timestamp[8]{};
    uint8_t ChallengeTimestamp[8]{};
    array_challenge ServerChallenge;
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

    // SERVER PROCEED RESPONSE CHECKING
    public:

    // GSS_Acquire_cred
    // ACQUIRE_CREDENTIALS_HANDLE_FN AcquireCredentialsHandle;

    // GSS_Init_sec_context
    // INITIALIZE_SECURITY_CONTEXT_FN InitializeSecurityContext
    // -> only for clients : unused for NTLM server

    // GSS_Accept_sec_context
    // ACCEPT_SECURITY_CONTEXT AcceptSecurityContext;

private:
    // ENCRYPT_MESSAGE EncryptMessage;

    // GSS_Unwrap
    // DECRYPT_MESSAGE DecryptMessage;

public:
    NtlmServer(bool is_domain,
               bool is_server,
               bytes_view TargetName, 
               bytes_view NetbiosComputerName, bytes_view NetbiosDomainName,
               bytes_view DnsComputerName, bytes_view DnsDomainName,
               bytes_view DnsTreeName,
               array_view_u8 key,
               const std::vector<enum NTLM_AV_ID> & avFieldsTags,
               Random & rand,
               TimeObj & timeobj,
               std::function<PasswordCallback(bytes_view,bytes_view,std::vector<uint8_t>&)> set_password_cb,
               uint32_t credssp_version, const NtlmVersion ntlm_version,
               bool ignore_bogus_nego_flags,
               const bool verbose = false)
        : is_domain(is_domain)
        , is_server(is_server)  
        , avFieldsTags(avFieldsTags.data(),avFieldsTags.data()+avFieldsTags.size())
        , TargetName(TargetName.data(), TargetName.data()+TargetName.size())
        , netbiosComputerName(NetbiosComputerName.data(), NetbiosComputerName.data()+NetbiosComputerName.size())
        , netbiosDomainName(NetbiosDomainName.data(), NetbiosDomainName.data()+NetbiosDomainName.size())
        , dnsComputerName(DnsComputerName.data(), DnsComputerName.data()+DnsComputerName.size())
        , dnsDomainName(DnsDomainName.data(), DnsDomainName.data()+DnsDomainName.size())
        , dnsTreeName(DnsTreeName.data(), DnsTreeName.data()+DnsTreeName.size())
        , credssp_version(credssp_version)
        , ntlm_version(ntlm_version)
        , ignore_bogus_nego_flags(ignore_bogus_nego_flags)
        , timeobj(timeobj)
        , rand(rand)
        , public_key(key)
        , set_password_cb(set_password_cb)
        , verbose(verbose)
    {
        memset(this->MachineID, 0xAA, sizeof(this->MachineID));

        LOG_IF(this->verbose, LOG_INFO, "this->server_auth_data.state = ServerAuthenticateData::Start");
        this->server_auth_data.state = ServerAuthenticateData::Start;

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
        this->identity_Password = {};
    }

public:
    std::vector<uint8_t> authenticate_next(bytes_view in_data)
    {
        LOG_IF(this->verbose, LOG_INFO, "NTLMServer::authenticate_next");

        switch (this->server_auth_data.state)
        {
            case ServerAuthenticateData::Start:
              LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Start");
              this->state = credssp::State::Err;
              return {};
            case ServerAuthenticateData::Loop:
            {
                std::vector<uint8_t> result;
                switch (this->ntlm_state) {
                case NTLM_STATE_INITIAL:
                {
                    /* receive authentication token */
                    TSRequest ts_request_in = recvTSRequest(in_data);
                                        
                    this->error_code = ts_request_in.error_code;

                    if (ts_request_in.negoTokens.size() < 1) {
                        LOG(LOG_ERR, "CredSSP: invalid ts_request.negoToken!");
                        LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                        this->state = credssp::State::Err;
                        return {};
                    }

                    // unsigned long const fContextReq = 0
                    //     | ASC_REQ_MUTUAL_AUTH
                    //     | ASC_REQ_CONFIDENTIALITY
                    //     | ASC_REQ_CONNECTION
                    //     | ASC_REQ_USE_SESSION_KEY
                    //     | ASC_REQ_REPLAY_DETECT
                    //     | ASC_REQ_SEQUENCE_DETECT
                    //     | ASC_REQ_EXTENDED_ERROR;

                    LOG_IF(this->verbose, LOG_INFO, "+++++++++++++++++NTLM_SSPI::AcceptSecurityContext::NTLM_STATE_INITIAL");

                    this->ntlm_state = NTLM_STATE_NEGOTIATE;

                    LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Read Negotiate");
                    NTLMNegotiateMessage negotiate_message = recvNTLMNegotiateMessage(ts_request_in.negoTokens);

                    uint32_t const mask = NTLMSSP_REQUEST_TARGET
                                |NTLMSSP_NEGOTIATE_NTLM
                                |NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                                |NTLMSSP_NEGOTIATE_UNICODE;

                    if ((negotiate_message.negoFlags.flags & mask) != mask) {
                        LOG_IF(this->verbose, LOG_INFO, "NTLM Negotiate : unsupported negotiate flag %u", negotiate_message.negoFlags.flags);
                        this->state = credssp::State::Err;
                        return {};
                    }

                    this->SavedNegotiateMessage = negotiate_message.raw_bytes;

                    LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Write Challenge");

                    rand.random(this->ServerChallenge.data(), this->ServerChallenge.size());

                    NTLMChallengeMessage challenge_message;
                    challenge_message.TargetName.buffer = ::UTF8toUTF16(this->TargetName);
                    challenge_message.serverChallenge = this->ServerChallenge;

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

                    uint32_t negoFlags = negotiate_message.negoFlags.flags;
                    // flags from negotiate mandatory: NTLMSSP_REQUEST_TARGET|NTLMSSP_NEGOTIATE_NTLM|NTLMSSP_NEGOTIATE_ALWAYS_SIGN|NTLMSSP_NEGOTIATE_UNICODE;
                    if (negoFlags & NTLMSSP_NEGOTIATE_VERSION) {
                        challenge_message.version = NtlmVersion{WINDOWS_MAJOR_VERSION_6, WINDOWS_MINOR_VERSION_1, 7601, NTLMSSP_REVISION_W2K3};
                    }

                    if (!ignore_bogus_nego_flags 
                    && (negoFlags & NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY)
                    && (negoFlags & NTLMSSP_NEGOTIATE_LM_KEY)) {
                        negoFlags ^= NTLMSSP_NEGOTIATE_LM_KEY;
                    }

                    if (!ignore_bogus_nego_flags 
                    && (negoFlags & NTLMSSP_NEGOTIATE_UNICODE)
                    && (negoFlags & NTLMSSP_NEGOTIATE_OEM)) {
                        negoFlags ^= NTLMSSP_NEGOTIATE_OEM;
                    }

                    // We should provide parameter to know if TARGET_TYPE is SERVER or DOMAIN
                    // and set the matching flag accordingly
                    if (challenge_message.TargetName.buffer.size() > 0){
                        // forcing some flags
                        negoFlags |= (NTLMSSP_TARGET_TYPE_SERVER);
                    }

                    if (!ignore_bogus_nego_flags){
                        // Means TargetInfo contains something. As we indeed do have something
                        // this flag should always be set here (except in bogus configurations)
                        negoFlags ^= NTLMSSP_NEGOTIATE_TARGET_INFO;
                    }

                    logNtlmFlags(negoFlags);

                    // NTLM: construct challenge target info
                    // WIN7
                    if (this->avFieldsTags.size() == 0){
                        this->avFieldsTags = {MsvAvNbComputerName, MsvAvNbDomainName, 
                                              MsvAvDnsComputerName, MsvAvDnsDomainName, 
                                              MsvAvDnsTreeName, MsvAvFlags, MsvAvTimestamp,
                                              MsvAvSingleHost, MsvAvTargetName, MsvChannelBindings
                                              };
                    }
                    for (auto tag: this->avFieldsTags){
                        switch (tag){
                        case MsvAvNbDomainName:
                        {
                            // NETBIOS Domain Name
                            std::vector<uint8_t> nb_domain_name_u16 = ::UTF8toUTF16(this->netbiosDomainName);
                            challenge_message.AvPairList.push_back(AvPair({MsvAvNbDomainName, nb_domain_name_u16}));
                        }
                        break;
                        case MsvAvNbComputerName:
                        {
                            // NETBIOS Computer Name
                            std::vector<uint8_t> nb_computer_name_u16 = ::UTF8toUTF16(this->netbiosComputerName);
                            challenge_message.AvPairList.push_back(AvPair({MsvAvNbComputerName, nb_computer_name_u16}));
                        }
                        break;
                        case MsvAvDnsDomainName:
                        {
                            // DNS Domain Name
                            auto dsn_domain_name_u16 = ::UTF8toUTF16(this->dnsDomainName);
                            challenge_message.AvPairList.push_back(AvPair({MsvAvDnsDomainName, dsn_domain_name_u16}));
                        }
                        break;
                        case MsvAvDnsTreeName:
                        {
                            // DNS Domain Name
                            auto dsn_tree_name_u16 = ::UTF8toUTF16(this->dnsTreeName);
                            challenge_message.AvPairList.push_back(AvPair({MsvAvDnsTreeName, dsn_tree_name_u16}));
                        }
                        break;
                        case MsvAvDnsComputerName:
                        {
                            // DNS Computer Name
                            auto dns_computer_name_u16 = ::UTF8toUTF16(this->dnsComputerName);
                            challenge_message.AvPairList.push_back(AvPair({MsvAvDnsComputerName, dns_computer_name_u16}));
                        }
                        break;
                        case MsvAvTimestamp:
                            challenge_message.AvPairList.push_back(AvPair({MsvAvTimestamp, std::vector<uint8_t>(this->Timestamp, this->Timestamp+sizeof(this->Timestamp))}));
                        break;
                        default:
                        break;
                        }
                    }

                    auto target_info = emitTargetInfo(challenge_message.AvPairList);
                    auto raw_ntlm_version = emitNtlmVersion(
                                                this->ntlm_version.ProductMajorVersion,
                                                this->ntlm_version.ProductMinorVersion,
                                                this->ntlm_version.ProductBuild,
                                                this->ntlm_version.NtlmRevisionCurrent);
                    if (this->is_server){
                        negoFlags |= NTLMSSP_TARGET_TYPE_SERVER;
                        negoFlags &= ~NTLMSSP_TARGET_TYPE_DOMAIN;
                    }
                    else {
                        negoFlags &= ~NTLMSSP_TARGET_TYPE_SERVER;
                    }
                    if (this->is_domain){
                        negoFlags |= NTLMSSP_TARGET_TYPE_DOMAIN;
                        negoFlags &= ~NTLMSSP_TARGET_TYPE_SERVER;
                    }
                    else {
                        negoFlags &= ~NTLMSSP_TARGET_TYPE_DOMAIN;
                    }

                    auto challenge = emitNTLMChallengeMessage(challenge_message, negoFlags, raw_ntlm_version, target_info);
                    auto negoTokens = std::vector<uint8_t>{} << challenge;

                    this->SavedChallengeMessage = challenge;

                    this->ntlm_state = NTLM_STATE_AUTHENTICATE;

                    LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::AcceptSecurityContext::NTLM_STATE_INITIAL::SEC_I_CONTINUE_NEEDED");
                    this->state_accept_security_context = SEC_I_CONTINUE_NEEDED;

                    result = emitTSRequest(std::min(ts_request_in.version,this->credssp_version),
                                           negoTokens,
                                           ts_request_in.authInfo,
                                           ts_request_in.pubKeyAuth,
                                           ts_request_in.error_code,
                                           ts_request_in.clientNonce.clientNonce,
                                           ts_request_in.clientNonce.initialized);
                    this->error_code = ts_request_in.error_code;

                    LOG_IF(this->verbose, LOG_INFO, "NTLMServer::buffer_free");
                    this->error_code = 0;

                    this->state = credssp::State::Cont;
                    return result;                    
                    break;
                }

                case NTLM_STATE_AUTHENTICATE:
                {
                    /* receive authentication token */
                    TSRequest ts_request_in = recvTSRequest(in_data);
                                        
                    this->error_code = ts_request_in.error_code;

                    if (ts_request_in.negoTokens.size() < 1) {
                        LOG(LOG_ERR, "CredSSP: invalid ts_request.negoToken!");
                        LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                        this->state = credssp::State::Err;
                        return {};
                    }

                    // unsigned long const fContextReq = 0
                    //     | ASC_REQ_MUTUAL_AUTH
                    //     | ASC_REQ_CONFIDENTIALITY
                    //     | ASC_REQ_CONNECTION
                    //     | ASC_REQ_USE_SESSION_KEY
                    //     | ASC_REQ_REPLAY_DETECT
                    //     | ASC_REQ_SEQUENCE_DETECT
                    //     | ASC_REQ_EXTENDED_ERROR;

                    LOG_IF(this->verbose, LOG_INFO, "++++++++++++++++++++++++++++++NTLM_SSPI::AcceptSecurityContext::NTLM_STATE_AUTHENTICATE");
                    LOG_IF(this->verbose, LOG_INFO, "NTLMContextServer Read Authenticate");
                    InStream in_stream(ts_request_in.negoTokens);
                    NTLMAuthenticateMessage authenticate = recvNTLMAuthenticateMessage(in_stream);

                    if (authenticate.has_mic) {
                        this->UseMIC = true;
                    }

                    auto & avuser = authenticate.UserName.buffer;
                    this->identity_User.assign(avuser.data(), avuser.data()+avuser.size());
                    auto & avdomain = authenticate.DomainName.buffer;
                    this->identity_Domain.assign(avdomain.data(), avdomain.data()+avdomain.size());

                    if ((this->identity_User.size() == 0) && (this->identity_Domain.size() == 0)){
                        LOG(LOG_ERR, "ANONYMOUS User not allowed");
                        LOG_IF(this->verbose, LOG_INFO, "++++++++++++++++++++++++++++++NTLM_SSPI::AcceptSecurityContext::NTLM_STATE_AUTHENTICATE::SEC_E_LOGON_DENIED");
                        // SEC_E_LOGON_DENIED;
                        this->state = credssp::State::Err;
                        return {};
                    }

                    auto res = (set_password_cb(this->identity_User, this->identity_Domain, this->identity_Password));

                    if (res == PasswordCallback::Error){
                        LOG_IF(this->verbose, LOG_INFO, "++++++++++++++++++++++++++++++NTLM_SSPI::AcceptSecurityContext::NTLM_STATE_AUTHENTICATE::SEC_E_LOGON_DENIED (3)");
                        // SEC_E_LOGON_DENIED;
                        this->state = credssp::State::Err;
                        return {};
                    }

                    // TODO: NTLM_STATE_WAIT_PASSWORD
                    // CGR: I removed support for interactive local password typing
                    // I don't see it as relevant in the context. However it could
                    // be useful to take into account that password callback 
                    // (check of password for provided user/domain)
                    // could not be immediate and may need some waiting
                    // before continuing ongoing authentication protocol.
                    // I will see to that later.

                    array_md4 hash;
                    if (this->identity_Password.size() > 0){
                        hash = Md4(this->identity_Password);
                    }
                    
                    
                    if (!authenticate.check_nt_response_from_authenticate(hash, this->ServerChallenge)) {
                        LOG(LOG_ERR, "NT RESPONSE NOT MATCHING STOP AUTHENTICATE");
                        // SEC_E_LOGON_DENIED;
                        this->state = credssp::State::Err;
                        return {};
                    }
                    if (!authenticate.check_lm_response_from_authenticate(hash, this->ServerChallenge)) {
                        // SEC_E_LOGON_DENIED;
                        this->state = credssp::State::Err;
                        return {};
                    }
                    // SERVER COMPUTE SHARED KEY WITH CLIENT
                    array_md5 SessionBaseKey = authenticate.compute_session_base_key(hash);
                    array_md5 ExportedSessionKey = authenticate.get_exported_session_key(SessionBaseKey);
                    this->ClientSigningKey = Md5(ExportedSessionKey, make_array_view(client_sign_magic));
                    this->ClientSealingKey = Md5(ExportedSessionKey, make_array_view(client_seal_magic));
                    this->ServerSigningKey = Md5(ExportedSessionKey, make_array_view(server_sign_magic));
                    this->ServerSealingKey  = Md5(ExportedSessionKey, make_array_view(server_seal_magic));

                    /**
                     * Initialize RC4 stream cipher states for sealing.
                     */

                    this->SendRc4Seal.set_key(this->ServerSealingKey);
                    this->RecvRc4Seal.set_key(this->ClientSealingKey);

                    // =======================================================

                    if (authenticate.has_mic) {
                        this->MessageIntegrityCheck = HmacMd5(ExportedSessionKey, 
                                                        this->SavedNegotiateMessage, 
                                                        this->SavedChallengeMessage,
                                                        authenticate.get_bytes());

                        if (0 != memcmp(this->MessageIntegrityCheck.data(), authenticate.MIC, 16)) {
                            LOG(LOG_ERR, "MIC NOT MATCHING STOP AUTHENTICATE");
                            hexdump_c(this->MessageIntegrityCheck.data(), 16);
                            hexdump_c(authenticate.MIC, 16);
                            // SEC_E_MESSAGE_ALTERED;
                            this->state = credssp::State::Err;
                            return {};
                        }
                    }
                    this->ntlm_state = NTLM_STATE_FINAL;
                    this->state_accept_security_context = SEC_I_COMPLETE_NEEDED;

                    LOG_IF(this->verbose, LOG_INFO, "NTLMServer::decrypt_public_key_echo");

                    unsigned long MessageSeqNo = this->recv_seq_num++;
                    LOG_IF(this->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");

                    if (ts_request_in.pubKeyAuth.size() < cbMaxSignature) {
                        if (ts_request_in.pubKeyAuth.size() == 0) {
                            // report_error
                            LOG(LOG_INFO, "Provided login/password is probably incorrect.");
                        }
                        LOG(LOG_ERR, "DecryptMessage failure: SEC_E_INVALID_TOKEN 0x%08X", SEC_E_INVALID_TOKEN);
                        // SEC_E_INVALID_TOKEN; /* DO NOT SEND CREDENTIALS! */
                        LOG(LOG_ERR, "Error: could not verify client's public key echo");
                        LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                        this->state = credssp::State::Err;
                        return {};
                    }

                    // ts_request_in.pubKeyAuth [signature][data_buffer]
                    array_view_u8 data_buffer = {ts_request_in.pubKeyAuth.data()+cbMaxSignature, ts_request_in.pubKeyAuth.size()-cbMaxSignature};
                    std::vector<uint8_t> result_buffer(data_buffer.size());

                    /* Decrypt message using RC4 */
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

                    if (memcmp(ts_request_in.pubKeyAuth.data(), expected_signature.data(),  expected_signature.size()) != 0) {
                        /* signature verification failed! */
                        LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
                        LOG(LOG_ERR, "Expected Signature:");
                        hexdump_c(expected_signature);
                        LOG(LOG_ERR, "Actual Signature:");
                        hexdump_c(ts_request_in.pubKeyAuth.data(), 16);

                        if (ts_request_in.pubKeyAuth.size() == 0) {
                            // report_error
                            LOG(LOG_INFO, "Provided login/password is probably incorrect.");
                        }
                        LOG(LOG_ERR, "DecryptMessage failure: SEC_E_MESSAGE_ALTERED 0x%08X", SEC_E_MESSAGE_ALTERED);
                        // SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
                        LOG(LOG_ERR, "Error: could not verify client's public key echo");
                        LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                        this->state = credssp::State::Err;
                        return {};
                    }

                    if (ts_request_in.use_version >= 5) {
                        if (ts_request_in.clientNonce.isset()){
                            this->SavedClientNonce = ts_request_in.clientNonce;
                        }
                        this->ClientServerHash = Sha256("CredSSP Client-To-Server Binding Hash\0"_av,
                                                this->SavedClientNonce.clientNonce,
                                                this->public_key);
                    }

                    if (result_buffer.size() !=  this->ClientServerHash.size()) {
                        LOG(LOG_ERR, "Decrypted Pub Key length or hash length does not match ! (%zu != %zu)", result_buffer.size(), this->public_key.size());
                        // SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
                        LOG(LOG_ERR, "Error: could not verify client's public key echo");
                        LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                        this->state = credssp::State::Err;
                        return {};
                    }
                    if (memcmp(this->ClientServerHash.data(), result_buffer.data(), this->ClientServerHash.size()) != 0) {
                        LOG(LOG_ERR, "Could not verify server's public key echo");

                        LOG(LOG_ERR, "Expected (length = %zu):", this->ClientServerHash.size());
                        hexdump_c(this->ClientServerHash);

                        LOG(LOG_ERR, "Actual (length = %zu):", this->ClientServerHash.size());
                        hexdump_c(result_buffer);

                        // SEC_E_MESSAGE_ALTERED; /* DO NOT SEND CREDENTIALS! */
                        LOG(LOG_ERR, "Error: could not verify client's public key echo");
                        LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                        this->state = credssp::State::Err;
                        return {};
                    }

                    LOG_IF(this->verbose, LOG_INFO, "NTLMServer::encrypt_public_key_echo");
                    uint32_t version = ts_request_in.use_version;

                    if (version >= 5) {
                        if (ts_request_in.clientNonce.isset()){
                            this->SavedClientNonce = ts_request_in.clientNonce;
                        }
                        array_sha256 ServerClientHash = Sha256("CredSSP Server-To-Client Binding Hash\0"_av,
                                                    this->SavedClientNonce.clientNonce,
                                                    this->public_key);
                        this->public_key = ServerClientHash;
                    }
                    else {
                        // if we are server and protocol is 2,3,4
                        // then echos the public key +1
                        ::ap_integer_increment_le(this->public_key);
                    }

                    LOG_IF(this->verbose, LOG_INFO, "NTLM_SSPI::EncryptMessage");

                    // data_out [signature][data_buffer]
                    std::vector<uint8_t> data_out(cbMaxSignature+this->public_key.size());
                    // data_buffer
                    {
                        array_view_u8 data_buffer = {&data_out.data()[cbMaxSignature], this->public_key.size()};
                        this->SendRc4Seal.crypt(this->public_key.size(), this->public_key.data(), data_buffer.data());
                    }
                    // signature
                    {
                        unsigned long MessageSeqNo = this->send_seq_num++;
                        array_md5 digest = HmacMd5(this->ServerSigningKey, out_uint32_le(MessageSeqNo), this->public_key);
                        array_view_u8 signature{data_out.data(), cbMaxSignature};
                        uint8_t checksum[8];
                        /* RC4-encrypt first 8 bytes of digest */
                        this->SendRc4Seal.crypt(8, digest.data(), checksum);

                        uint32_t seal_version = 1;
                        /* Concatenate version, ciphertext and sequence number to build signature */
                        auto av_version = out_uint32_le(seal_version);
                        memcpy(signature.data(), av_version.data(), av_version.size());
                        memcpy(signature.data()+4, checksum, 8);
                        auto av_seqno = out_uint32_le(MessageSeqNo);
                        memcpy(signature.data()+12, av_seqno.data(), av_seqno.size());
                    }

                    ts_request_in.pubKeyAuth.assign(data_out.data(),data_out.data()+data_out.size());

                    result = emitTSRequest(std::min(ts_request_in.version,this->credssp_version),
                                           {},
                                           ts_request_in.authInfo,
                                           ts_request_in.pubKeyAuth,
                                           ts_request_in.error_code,
                                           {},
                                           false);
                    this->error_code = 0;

                    this->server_auth_data.state = ServerAuthenticateData::Final;
                    this->state = credssp::State::Cont;
                    return result;                    
                }

                default:
                    LOG_IF(this->verbose, LOG_INFO, "+++++++++++++++++NTLM_SSPI::AcceptSecurityContext:: OTHER UNEXPECTED NTLM STATE");
                    this->state_accept_security_context = SEC_E_OUT_OF_SEQUENCE;
                    LOG(LOG_ERR, "AcceptSecurityContext status: 0x%08X", SEC_E_OUT_OF_SEQUENCE);
                    LOG(LOG_INFO, "ServerAuthenticateData::Loop::Err");
                    this->state = credssp::State::Err;
                    return {};
                } // Switch
                this->state = credssp::State::Err;
                return {};
            }

            case ServerAuthenticateData::Final:
            {
                LOG_IF(this->verbose, LOG_INFO, "rdpNTLMServer::server_authenticate_final");
                TSRequest ts_request_in_final = recvTSRequest(in_data);
                this->error_code = ts_request_in_final.error_code;

                if (ts_request_in_final.authInfo.size() < 1) {
                    LOG(LOG_ERR, "credssp_decrypt_ts_credentials missing ts_request.authInfo buffer");
                    LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X", SEC_E_INVALID_TOKEN);
                    LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final::Err");
                    this->state = credssp::State::Err;
                    return {};
                }

                unsigned long MessageSeqNo = this->recv_seq_num++;
                LOG_IF(this->verbose & 0x400, LOG_INFO, "NTLM_SSPI::DecryptMessage");
                if (ts_request_in_final.authInfo.size() < cbMaxSignature) {
                    LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X", SEC_E_INVALID_TOKEN);
                    LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final::Err");
                    this->state = credssp::State::Err;
                    return {};
                }
                // ts_request_in_final.authInfo [signature][data_buffer]

                array_view_const_u8 data_buffer = {ts_request_in_final.authInfo.data()+cbMaxSignature, ts_request_in_final.authInfo.size()-cbMaxSignature};
                auto decrypted_creds = std::vector<uint8_t>(data_buffer.size());

                /* Decrypt message using with RC4, result overwrites original buffer */
                // context->confidentiality == true
                this->RecvRc4Seal.crypt(data_buffer.size(), data_buffer.data(), decrypted_creds.data());

                array_md5 digest = HmacMd5(this->ClientSigningKey, out_uint32_le(MessageSeqNo), decrypted_creds);

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

                if (memcmp(ts_request_in_final.authInfo.data(), expected_signature, 16) != 0) {
                    /* signature verification failed! */
                    LOG(LOG_ERR, "signature verification failed, something nasty is going on!");
                    LOG(LOG_ERR, "Expected Signature:");
                    hexdump_c(expected_signature, 16);
                    LOG(LOG_ERR, "Actual Signature:");
                    hexdump_c(ts_request_in_final.authInfo.data(), 16);

                    LOG(LOG_ERR, "Could not decrypt TSCredentials status: 0x%08X", SEC_E_MESSAGE_ALTERED);
                    LOG_IF(this->verbose, LOG_INFO, "ServerAuthenticateData::Final::Err");
                    this->state = credssp::State::Err;
                    return {};
                }

                this->ts_credentials = recvTSCredentials(decrypted_creds);
                this->server_auth_data.state = ServerAuthenticateData::Start;
                this->state = credssp::State::Finish;
                return {};
            }
        }
        this->state = credssp::State::Err;
        return {};
    }

};


