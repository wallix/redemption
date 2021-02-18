/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan, Clément Moroldo
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#include "mod/rdp/rdp_negociation.hpp"
#include "core/log_id.hpp"
#include "core/app_path.hpp"
#include "core/RDP/autoreconnect.hpp"
#include "core/RDP/lic.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/RDP/gcc/userdata/mcs_channels.hpp"
#include "core/RDP/gcc/userdata/sc_core.hpp"
#include "core/RDP/gcc/userdata/sc_net.hpp"
#include "core/RDP/gcc/userdata/sc_sec1.hpp"
#include "core/RDP/gcc/userdata/cs_security.hpp"
#include "core/RDP/gcc/userdata/cs_net.hpp"
#include "core/RDP/gcc/userdata/cs_core.hpp"
#include "core/RDP/gcc/userdata/cs_cluster.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/sec.hpp"
#include "core/RDP/mcs.hpp"
#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "acl/auth_api.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "core/channels_authorizations.hpp"
#include "utils/genrandom.hpp"
#include "utils/redirection_info.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/multisz.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/zstring_view.hpp"

#include "system/tls_check_certificate.hpp"

#include <cstring>


RdpNegociation::RDPServerNotifier::RDPServerNotifier(
    SessionLogApi& session_log,
    bool server_cert_store,
    ServerCertCheck server_cert_check,
    std::unique_ptr<char[]>&& certif_path,
    ServerNotification server_access_allowed_message,
    ServerNotification server_cert_create_message,
    ServerNotification server_cert_success_message,
    ServerNotification server_cert_failure_message,
    ServerNotification server_cert_error_message,
    RDPVerbose verbose
) noexcept
: server_cert_check(server_cert_check)
, certif_path(std::move(certif_path))
, server_cert_store(server_cert_store)
, server_status_messages([&]{
    std::array<ServerNotification, 5> a;
    a[underlying_cast(Status::AccessAllowed)] = server_access_allowed_message;
    a[underlying_cast(Status::CertCreate)] = server_cert_create_message;
    a[underlying_cast(Status::CertSuccess)] = server_cert_success_message;
    a[underlying_cast(Status::CertFailure)] = server_cert_failure_message;
    a[underlying_cast(Status::CertError)] = server_cert_error_message;
    return a;
}())
, verbose(verbose)
, session_log(session_log)
{}

void RdpNegociation::RDPServerNotifier::server_cert_status(Status status, std::string_view error_msg)
{
    auto notification_type = this->server_status_messages[underlying_cast(status)];
    if (bool(notification_type & ServerNotification::syslog)) {
        auto log6 = [&](LogId id, zstring_view message){
            this->session_log.log6(id, {KVLog("description"_av, message),});
            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "%s", message);
        };

        switch (status)
        {
            case Status::AccessAllowed:
                log6(LogId::CERTIFICATE_CHECK_SUCCESS, "Connection to server allowed"_zv);
                break;
            case Status::CertCreate:
                log6(LogId::SERVER_CERTIFICATE_NEW, "New X.509 certificate created"_zv);
                break;
            case Status::CertSuccess:
                log6(LogId::SERVER_CERTIFICATE_MATCH_SUCCESS, "X.509 server certificate match"_zv);
                break;
            case Status::CertFailure:
                log6(LogId::SERVER_CERTIFICATE_MATCH_FAILURE, "X.509 server certificate match failure"_zv);
                break;
            case Status::CertError:
                log6(LogId::SERVER_CERTIFICATE_ERROR,
                        str_concat("X.509 server certificate internal error: "_zv, error_msg));
                break;
        }
    }
}

CertificateResult RdpNegociation::RDPServerNotifier::server_cert_callback(
    X509& certificate, std::string* error_message, const char* ip_address, int port)
{
    if (this->certificate_callback) {
        return this->certificate_callback(certificate);
    }

    const bool ensure_server_certificate_match =
        (server_cert_check == ServerCertCheck::fails_if_no_match_or_missing)
        ||(server_cert_check == ServerCertCheck::fails_if_no_match_and_succeed_if_no_know);

    const bool ensure_server_certificate_exists =
        (server_cert_check == ServerCertCheck::fails_if_no_match_or_missing)
        ||(server_cert_check == ServerCertCheck::succeed_if_exists_and_fails_if_missing);

    return tls_check_certificate(
        certificate,
        server_cert_store,
        ensure_server_certificate_match,
        ensure_server_certificate_exists,
        *this,
        certif_path.get(),
        error_message,
        ip_address,
        port)
      ? CertificateResult::valid
      : CertificateResult::invalid;
}

RdpNegociation::RdpNegociation(
    const ChannelsAuthorizations channels_authorizations,
    CHANNELS::ChannelDefArray& mod_channel_list,
    const CHANNELS::ChannelNameId auth_channel,
    const CHANNELS::ChannelNameId checkout_channel,
    CryptContext& decrypt,
    CryptContext& encrypt,
    const RdpLogonInfo& logon_info,
    bool enable_auth_channel,
    Transport& trans,
    FrontAPI& front,
    const ClientInfo& info,
    RedirectionInfo& redir_info,
    Random& gen,
    const TimeBase& time_base,
    const ModRDPParams& mod_rdp_params,
    SessionLogApi& session_log,
    LicenseApi& license_store,
    bool has_managed_drive,
    bool convert_remoteapp_to_desktop,
    const TLSClientParams & tls_client_params
    )
    : mod_channel_list(mod_channel_list)
    , channels_authorizations(channels_authorizations)
    , auth_channel(auth_channel)
    , checkout_channel(checkout_channel)
    , decrypt(decrypt)
    , encrypt(encrypt)
    , enable_auth_channel(enable_auth_channel)
    , redir_info(redir_info)
    , logon_info(logon_info)
    , front(front)
    , cbAutoReconnectCookie(info.cbAutoReconnectCookie)
    , keylayout(info.keylayout)
    , console_session(info.console_session)
    , front_bpp(info.screen_info.bpp)
    , performanceFlags(
        info.rdp5_performanceflags
        & (~(mod_rdp_params.adjust_performance_flags_for_recording
            ? static_cast<uint32_t>(PERF_ENABLE_FONT_SMOOTHING)
            : 0)
        )
        )
    , client_time_zone(info.client_time_zone)
    , gen(gen)
    , verbose(mod_rdp_params.verbose /*| (RDPVerbose::security|RDPVerbose::basic_trace)*/)
    , server_notifier(
        session_log,
        mod_rdp_params.server_cert_store,
        mod_rdp_params.server_cert_check,
        [](const char* device_id){
            // TODO return str_concat(certif_path, '/', device_id)
            auto certif_path = app_path(AppPath::Certif);
            size_t lg_certif_path = certif_path.size();
            size_t lg_dev_id = strlen(device_id);
            auto buffer = std::make_unique<char[]>(lg_certif_path + lg_dev_id + 2);
            memcpy(buffer.get(), certif_path.data(), lg_certif_path);
            buffer[lg_certif_path] = '/';
            memcpy(buffer.get()+lg_certif_path+1, device_id, lg_dev_id+1);
            return buffer;
        }(mod_rdp_params.device_id),
        mod_rdp_params.server_access_allowed_message,
        mod_rdp_params.server_cert_create_message,
        mod_rdp_params.server_cert_success_message,
        mod_rdp_params.server_cert_failure_message,
        mod_rdp_params.server_cert_error_message,
        mod_rdp_params.verbose
        )
    , nego(
        mod_rdp_params.enable_tls, mod_rdp_params.target_user,
        mod_rdp_params.enable_nla, mod_rdp_params.enable_restricted_admin_mode,
        mod_rdp_params.target_host, mod_rdp_params.enable_krb, gen, time_base,
        mod_rdp_params.close_box_extra_message_ref, mod_rdp_params.lang,
        tls_client_params,
        static_cast<RdpNego::Verbose>(mod_rdp_params.verbose)
        )
    , desktop_physical_width(info.desktop_physical_width)
    , desktop_physical_height(info.desktop_physical_height)
    , desktop_orientation(info.desktop_orientation)
    , desktop_scale_factor(info.desktop_scale_factor)
    , device_scale_factor(info.device_scale_factor)
    , trans(trans)
    , password_printing_mode(mod_rdp_params.password_printing_mode)
#ifndef __EMSCRIPTEN__
    , enable_session_probe(mod_rdp_params.session_probe_params.enable_session_probe)
#else
    , enable_session_probe(false)
#endif
    , enable_remotefx(mod_rdp_params.enable_remotefx && info.bitmap_codec_caps.haveRemoteFxCodec)
    , rdp_compression(mod_rdp_params.rdp_compression)
#ifndef __EMSCRIPTEN__
    , session_probe_use_clipboard_based_launcher(
        mod_rdp_params.session_probe_params.used_clipboard_based_launcher
        && (!mod_rdp_params.application_params.target_application || !(*mod_rdp_params.application_params.target_application))
        && (!mod_rdp_params.application_params.use_client_provided_alternate_shell
        || !info.alternate_shell[0] || info.remote_program)
        )
#else
    , session_probe_use_clipboard_based_launcher(false)
#endif
    , remote_program(mod_rdp_params.remote_app_params.enable_remote_program)
    , bogus_sc_net_size(mod_rdp_params.bogus_sc_net_size)
    , allow_using_multiple_monitors(mod_rdp_params.allow_using_multiple_monitors)
    , bogus_monitor_layout_treatment(mod_rdp_params.bogus_monitor_layout_treatment)
    , allow_scale_factor(mod_rdp_params.allow_scale_factor)
    , cs_monitor(info.cs_monitor)
    , cs_monitor_ex(info.cs_monitor_ex)
    , perform_automatic_reconnection(mod_rdp_params.perform_automatic_reconnection)
    , server_auto_reconnect_packet_ref(mod_rdp_params.server_auto_reconnect_packet_ref)
    , info_packet_extra_flags(InfoPacketFlags(
        (info.has_sound_code ? INFO_REMOTECONSOLEAUDIO : InfoPacketFlags{})
      | (info.has_sound_capture_code ? INFO_AUDIOCAPTURE : InfoPacketFlags{})
    ))
    , has_managed_drive(has_managed_drive)
    , convert_remoteapp_to_desktop(convert_remoteapp_to_desktop)
    , send_channel_index(0)
    , license_client_name(info.hostname)
    , license_store(license_store)
    , use_license_store(mod_rdp_params.use_license_store)
    , build_number(info.build)
    , forward_build_number(mod_rdp_params.forward_client_build_number)
{
    this->negociation_result.front_width = info.screen_info.width;
    this->negociation_result.front_width -= this->negociation_result.front_width % 4;
    this->negociation_result.front_height = info.screen_info.height;

    if (this->cbAutoReconnectCookie) {
        ::memcpy(this->autoReconnectCookie, info.autoReconnectCookie, sizeof(this->autoReconnectCookie));
    }

    LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO,
        "RdpNegociation: enable_session_probe=%s", (this->enable_session_probe ? "yes" : "no"));

    strncpy(this->clientAddr, mod_rdp_params.client_address, sizeof(this->clientAddr) - 1);

    // Password is a multi-sz!
    // A multi-sz contains a sequence of null-terminated strings,
    //  terminated by an empty string (\0) so that the last two
    //  characters are both null terminators.
    SOHSeparatedStringsToMultiSZ(this->password, sizeof(this->password), mod_rdp_params.target_password);

    LOG(LOG_INFO, "Server key layout is 0x%x", unsigned(this->keylayout));

    this->nego.set_identity(this->logon_info.username(),
                            this->logon_info.domain(),
                            this->password,
                            this->logon_info.hostname());

    if (bool(this->verbose & RDPVerbose::connection)){
        this->redir_info.log(LOG_INFO, "Init with Redir_info");
    }

    {
        bool load_balance_info_used = false;

        const size_t load_balance_info_length = strlen(mod_rdp_params.load_balance_info);

        if (!this->redir_info.valid && load_balance_info_length) {
            if (load_balance_info_length + 2 < sizeof(this->redir_info.lb_info)) {
                load_balance_info_used = true;

                this->redir_info.valid = true;

                ::snprintf(::char_ptr_cast(this->redir_info.lb_info),
                    sizeof(this->redir_info.lb_info), "%s\x0D\x0A",
                    mod_rdp_params.load_balance_info);
                this->redir_info.lb_info_length = load_balance_info_length + 2;
            }
            else {
                LOG(LOG_WARNING, "RdpNegociation: load balance info too long! %zu >= %zu",
                    load_balance_info_length, sizeof(this->redir_info.lb_info));
            }
        }

        if (this->redir_info.valid && (this->redir_info.lb_info_length > 0)) {
            this->nego.set_lb_info(this->redir_info.lb_info,
                                    this->redir_info.lb_info_length);

            if (load_balance_info_used) {
                this->redir_info.valid = false;
            }
        }
    }
}

void RdpNegociation::set_program(char const* program, char const* directory) noexcept
{
    utils::strlcpy(this->program, program);
    utils::strlcpy(this->directory, directory);
}

void RdpNegociation::set_cert_callback(std::function<CertificateResult(X509&)> callback)
{
    this->server_notifier.certificate_callback = std::move(callback);
}

void RdpNegociation::start_negociation()
{
    this->nego.send_negotiation_request(this->trans);
    this->state = State::NEGO;
}

bool RdpNegociation::recv_data(TpduBuffer& buf)
{
    if (this->state == State::TERMINATED)
    {
        return true;
    }

    if (this->state == State::NEGO)
    {
        bool const run = this->nego.recv_next_data(buf, this->trans, this->server_notifier);

        if (not run) {
            this->send_connectInitialPDUwithGccConferenceCreateRequest();
            this->state = State::BASIC_SETTINGS_EXCHANGE;
        }

        return false;
    }

    buf.load_data(this->trans);
    while (buf.next(TpduBuffer::PDU)) {
        InStream x224_data(buf.current_pdu_buffer());
        switch (this->state)
        {
            case State::BASIC_SETTINGS_EXCHANGE:
                if (this->basic_settings_exchange(x224_data)) {
                    this->state = State::CHANNEL_CONNECTION_ATTACH_USER;
                }
                break;
            case State::CHANNEL_CONNECTION_ATTACH_USER:
                if (this->channel_connection_attach_user(x224_data)){
                    this->state = State::CHANNEL_JOIN_CONFIRM;
                }
                break;
            case State::CHANNEL_JOIN_CONFIRM:
                if (this->channel_join_confirm(x224_data)){
                    // RDP Security Commencement
                    // -------------------------

                    // RDP Security Commencement: If standard RDP security methods are being
                    // employed and encryption is in force (this is determined by examining the data
                    // embedded in the GCC Conference Create Response packet) then the client sends
                    // a Security Exchange PDU containing an encrypted 32-byte random number to the
                    // server. This random number is encrypted with the public key of the server
                    // (the server's public key, as well as a 32-byte server-generated random
                    // number, are both obtained from the data embedded in the GCC Conference Create
                    //  Response packet).

                    // The client and server then utilize the two 32-byte random numbers to generate
                    // session keys which are used to encrypt and validate the integrity of
                    // subsequent RDP traffic.

                    // From this point, all subsequent RDP traffic can be encrypted and a security
                    // header is include " with the data if encryption is in force (the Client Info
                    // and licensing PDUs are an exception in that they always have a security
                    // header). The Security Header follows the X.224 and MCS Headers and indicates
                    // whether the attached data is encrypted.

                    // Even if encryption is in force server-to-client traffic may not always be
                    // encrypted, while client-to-server traffic will always be encrypted by
                    // Microsoft RDP implementations (encryption of licensing PDUs is optional,
                    // however).

                    // Client                                                     Server
                    //    |------Security Exchange PDU ---------------------------> |
                    LOG_IF(bool(this->verbose & RDPVerbose::security), LOG_INFO,
                        "RdpNegociation: RDP Security Commencement");

                    if (this->negociation_result.encryptionLevel){
                        LOG_IF(bool(this->verbose & RDPVerbose::security), LOG_INFO,
                            "RdpNegociation: SecExchangePacket keylen=%u", this->server_public_key_len);

                        this->send_data_request(
                            GCC::MCS_GLOBAL_CHANNEL,
                            dynamic_packet(this->server_public_key_len + 32, [this](OutStream & stream) {
                                SEC::SecExchangePacket_Send mcs(
                                    stream,
                                    {this->client_crypt_random, this->server_public_key_len}
                                );
                                (void)mcs;
                            })
                        );
                    }

                    // Secure Settings Exchange
                    // ------------------------

                    // Secure Settings Exchange: Secure client data (such as the username,
                    // password and auto-reconnect cookie) is sent to the server using the Client
                    // Info PDU.

                    // Client                                                     Server
                    //    |------ Client Info PDU      ---------------------------> |

                    LOG_IF(bool(this->verbose & RDPVerbose::security), LOG_INFO,
                        "RdpNegociation: Secure Settings Exchange");

                    this->send_client_info_pdu();
                    this->state = State::GET_LICENSE;
                }
                break;
            default:
                if (this->get_license(x224_data)) {
                    this->state = State::TERMINATED;
                    return true;
                }
                break;
        }
    }
    return false;
}

bool RdpNegociation::basic_settings_exchange(InStream & x224_data)
{
    LOG_IF(bool(this->verbose & RDPVerbose::security), LOG_INFO, "RdpNegociation: Basic Settings Exchange");

    {
        X224::DT_TPDU_Recv x224(x224_data);

        MCS::CONNECT_RESPONSE_PDU_Recv mcs(x224.payload, MCS::BER_ENCODING);
        GCC::Create_Response_Recv gcc_cr(mcs.payload);

        while (gcc_cr.payload.in_check_rem(4)) {
            GCC::UserData::RecvFactory f(gcc_cr.payload);
            switch (f.tag) {
            case SC_CORE:
//                            LOG(LOG_INFO, "=================== SC_CORE =============");
                {
                    GCC::UserData::SCCore sc_core;
                    sc_core.recv(f.payload);
                    if (bool(this->verbose & RDPVerbose::connection)) {
                        sc_core.log("Received from server");
                    }
                    if (0x0080001 == sc_core.version){ // can't use rdp5
                        this->negociation_result.use_rdp5 = false;
                    }
                }
                break;
            case SC_SECURITY:
                LOG(LOG_INFO, "=================== SC_SECURITY =============");
                {
                    GCC::UserData::SCSecurity sc_sec1;
                    sc_sec1.recv(f.payload);

                    if (bool(this->verbose & RDPVerbose::security)) {
                        sc_sec1.log("Received from server");
                    }

                    this->negociation_result.encryptionLevel = sc_sec1.encryptionLevel;
                    this->negociation_result.encryptionMethod = sc_sec1.encryptionMethod;

                    if (sc_sec1.encryptionLevel == 0
                        &&  sc_sec1.encryptionMethod == 0) { /* no encryption */
                        LOG(LOG_INFO, "No encryption");
                    }
                    else {
                        uint8_t serverRandom[SEC_RANDOM_SIZE] = {};
                        uint8_t modulus[SEC_MAX_MODULUS_SIZE] = {};
                        uint8_t exponent[SEC_EXPONENT_SIZE] = {};

                        memcpy(serverRandom, sc_sec1.serverRandom, sc_sec1.serverRandomLen);
//                                        LOG(LOG_INFO, "================= SC_SECURITY got random =============");
                        // serverCertificate (variable): The variable-length certificate containing the
                        //  server's public key information. The length in bytes is given by the
                        // serverCertLen field. If the encryptionMethod and encryptionLevel fields are
                        // both set to 0 then this field MUST NOT be present.

                        /* RSA info */
                        if (sc_sec1.dwVersion == GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
//                                        LOG(LOG_INFO, "================= SC_SECURITY CERT_CHAIN_VERSION_1");

                            memcpy(exponent, sc_sec1.proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
                            memcpy(modulus, sc_sec1.proprietaryCertificate.RSAPK.modulus,
                                    sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE);

                            this->server_public_key_len = sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE;

                        }
                        else {
#ifndef __EMSCRIPTEN__
                            // LOG(LOG_INFO, "================= SC_SECURITY CERT_CHAIN_X509");
                            uint32_t const certcount = sc_sec1.x509.certCount;
                            if (certcount < 2){
                                LOG(LOG_ERR, "Server didn't send enough X509 certificates");
                                throw Error(ERR_SEC);
                            }

                            X509 *cert = sc_sec1.x509.certs[certcount - 1];

                            // TODO CGR: Currently, we don't use the CA Certificate, we should
                            // TODO *) Verify the server certificate (server_cert) with the CA certificate.
                            // TODO *) Store the CA Certificate with the hostname of the server we are connecting to as key, and compare it when we connect the next time, in order to prevent MITM-attacks.

                            /* By some reason, Microsoft sets the OID of the Public RSA key to
                                the oid for "MD5 with RSA Encryption" instead of "RSA Encryption"

                                Kudos to Richard Levitte for the following (. intuitive .)
                                lines of code that resets the OID and let's us extract the key. */

                            RSA * server_public_key = nullptr;

                            {
                                X509_PUBKEY * key = X509_get_X509_PUBKEY(cert);
                                if (!key) {
                                    LOG(LOG_ERR, "Failed to get public key from certificate");
                                    throw Error(ERR_SEC);
                                }
                                X509_ALGOR * algor;
                                if (X509_PUBKEY_get0_param(nullptr, nullptr, nullptr, &algor, key) != 1) {
                                    LOG(LOG_ERR, "Failed to get algorithm used for public key.");
                                    throw Error(ERR_SEC);
                                }

                                int const nid = OBJ_obj2nid(algor->algorithm);
                                if ((nid == NID_md5WithRSAEncryption)
                                || (nid == NID_shaWithRSAEncryption)) {
                                    #if OPENSSL_VERSION_NUMBER < 0x10100000L
                                    X509_PUBKEY_set0_param(key, OBJ_nid2obj(NID_rsaEncryption), 0, nullptr, nullptr, 0);
                                    #else
                                    const unsigned char *p;
                                    int pklen;
                                    if (!X509_PUBKEY_get0_param(nullptr, &p, &pklen, nullptr, key)) {
                                        LOG(LOG_ERR, "Failed to get algorithm used for public key.");
                                        throw Error(ERR_SEC);
                                    }
                                    if (!(server_public_key = d2i_RSAPublicKey(nullptr, &p, pklen))) {
                                        LOG(LOG_ERR, "Failed to extract public key from certificate");
                                        throw Error(ERR_SEC);
                                    }
                                    #endif
                                }
                            }

                            // LOG(LOG_INFO, "================= SC_SECURITY X509_get_pubkey");

                            #if OPENSSL_VERSION_NUMBER >= 0x10100000L
                            if (!server_public_key)
                            #endif
                            {
                                EVP_PKEY * epk = X509_get_pubkey(cert);
                                if (nullptr == epk){
                                    LOG(LOG_ERR, "Failed to extract public key from certificate");
                                    throw Error(ERR_SEC);
                                }
                                server_public_key = EVP_PKEY_get1_RSA(epk);
                                EVP_PKEY_free(epk);
                            }
                            this->server_public_key_len = RSA_size(server_public_key);

                            if (nullptr == server_public_key){
                                LOG(LOG_ERR, "Failed to parse X509 server key");
                                throw Error(ERR_SEC);
                            }

                            if ((this->server_public_key_len < SEC_MODULUS_SIZE)
                            ||  (this->server_public_key_len > SEC_MAX_MODULUS_SIZE)){
                                LOG(LOG_ERR, "Wrong server public key size (%u bits)", this->server_public_key_len * 8);
                                throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
                            }

                            BIGNUM const *e, *n;
                            #if OPENSSL_VERSION_NUMBER < 0x10100000L
                            e = server_public_key->e;
                            n = server_public_key->n;
                            #else
                            RSA_get0_key(server_public_key, &n, &e, nullptr);
                            #endif

                            if ((BN_num_bytes(e) > SEC_EXPONENT_SIZE)
                                ||  (BN_num_bytes(n) > SEC_MAX_MODULUS_SIZE)){
                                LOG(LOG_ERR, "Failed to extract RSA exponent and modulus");
                                throw Error(ERR_SEC);
                            }

                            int len_e = BN_bn2bin(e, exponent);
                            int len_n = BN_bn2bin(n, modulus);
                            reverseit(exponent, len_e);
                            reverseit(modulus, len_n);
                            RSA_free(server_public_key);
#else
                            LOG(LOG_ERR, "SCSecurity CERT_CHAIN_X509 not implemented");
                            throw Error(ERR_SEC);
#endif // __EMSCRIPTEN__
                        }

                        /* Generate a client random, and determine encryption keys */
                        this->gen.random(this->client_random, SEC_RANDOM_SIZE);
                        LOG_IF(bool(this->verbose & RDPVerbose::security), LOG_INFO, "RdpNegociation: Generate client random");

//                                        LOG(LOG_INFO, "================= SC_SECURITY rsa_encrypt");
//                                        LOG(LOG_INFO, "================= SC_SECURITY client_random");
//                                        hexdump(this->client_random, SEC_RANDOM_SIZE);
//                                        LOG(LOG_INFO, "================= SC_SECURITY SEC_RANDOM_SIZE=%u",
//                                            static_cast<unsigned>(SEC_RANDOM_SIZE));

//                                        LOG(LOG_INFO, "================= SC_SECURITY server_public_key_len");
//                                        hexdump(modulus, this->server_public_key_len);
//                                        LOG(LOG_INFO, "================= SC_SECURITY server_public_key_len=%u",
//                                            static_cast<unsigned>(this->server_public_key_len));

//                                        LOG(LOG_INFO, "================= SC_SECURITY exponent");
//                                        hexdump(exponent, SEC_EXPONENT_SIZE);
//                                        LOG(LOG_INFO, "================= SC_SECURITY exponent_size=%u",
//                                            static_cast<unsigned>(SEC_EXPONENT_SIZE));

                        ssllib::rsa_encrypt(
                            this->client_crypt_random,
                            SEC_RANDOM_SIZE,
                            this->client_random,
                            this->server_public_key_len,
                            modulus,
                            SEC_EXPONENT_SIZE,
                            exponent);

//                                        LOG(LOG_INFO, "================= SC_SECURITY client_crypt_random");
//                                        hexdump(this->client_crypt_random, sizeof(this->client_crypt_random));
//                                        LOG(LOG_INFO, "================= SC_SECURITY SEC_RANDOM_SIZE=%u",
//                                            static_cast<unsigned>(sizeof(this->client_crypt_random)));

                        SEC::KeyBlock key_block(this->client_random, serverRandom);
                        memcpy(this->encrypt.sign_key, key_block.blob0, 16);
                        if (sc_sec1.encryptionMethod == 1){
                            ssllib::sec_make_40bit(this->encrypt.sign_key);
                        }
                        this->decrypt.generate_key(key_block.key1, sc_sec1.encryptionMethod);
                        this->encrypt.generate_key(key_block.key2, sc_sec1.encryptionMethod);
                    }
                }
                break;
            case SC_NET:
//                            LOG(LOG_INFO, "=================== SC_NET =============");

                {
                    GCC::UserData::SCNet sc_net;
                    sc_net.recv(f.payload, this->bogus_sc_net_size);

                    /* We assume that the channel_id array is confirmed in the same order
                        that it has been sent. If there are any channels not confirmed, they're
                        going to be the last channels on the array sent in MCS Connect Initial */
                    LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO,
                        "server_channels_count=%" PRIu16 " sent_channels_count=%zu",
                        sc_net.channelCount,
                        this->mod_channel_list.size());

                    for (uint32_t index = 0; index < sc_net.channelCount; index++) {
                        if (bool(this->verbose & RDPVerbose::channels)){
                            this->mod_channel_list[index].log(index);
                        }
                        this->mod_channel_list.set_chanid(index, sc_net.channelDefArray[index].id);
                    }
                    if (bool(this->verbose & RDPVerbose::channels)) {
                        sc_net.log("Received from server");
                    }
                }
                break;
            default:
                LOG(LOG_ERR, "unsupported GCC UserData response tag 0x%x", f.tag);
                throw Error(ERR_GCC);
            }
        }

        if (gcc_cr.payload.in_check_rem(1)) {
            LOG(LOG_ERR, "Error while parsing GCC UserData : short header");
            throw Error(ERR_GCC);
        }
    }

    LOG_IF(bool(this->verbose & RDPVerbose::connection), LOG_INFO, "RdpNegociation: Channel Connection");

    // Channel Connection
    // ------------------
    // Channel Connection: The client sends an MCS Erect Domain Request PDU,
    // followed by an MCS Attach User Request PDU to attach the primary user
    // identity to the MCS domain.

    // The server responds with an MCS Attach User Response PDU containing the user
    // channel ID.

    // The client then proceeds to join the :
    // - user channel,
    // - the input/output (I/O) channel
    // - and all of the static virtual channels

    // (the I/O and static virtual channel IDs are obtained from the data embedded
    //  in the GCC packets) by using multiple MCS Channel Join Request PDUs.

    // The server confirms each channel with an MCS Channel Join Confirm PDU.
    // (The client only sends a Channel Join Request after it has received the
    // Channel Join Confirm for the previously sent request.)

    // From this point, all subsequent data sent from the client to the server is
    // wrapped in an MCS Send Data Request PDU, while data sent from the server to
    //  the client is wrapped in an MCS Send Data Indication PDU. This is in
    // addition to the data being wrapped by an X.224 Data PDU.

    // Client                                                     Server
    //    |-------MCS Erect Domain Request PDU--------------------> |
    //    |-------MCS Attach User Request PDU---------------------> |

    //    | <-----MCS Attach User Confirm PDU---------------------- |

    //    |-------MCS Channel Join Request PDU--------------------> |
    //    | <-----MCS Channel Join Confirm PDU--------------------- |

    LOG_IF(bool(this->verbose & RDPVerbose::connection), LOG_INFO,
        "Send MCS::ErectDomainRequest");

    write_packets(
        this->trans,
        [](StreamSize<256> /*maxlen*/, OutStream & mcs_header){
            MCS::ErectDomainRequest_Send mcs(mcs_header, 0, 0, MCS::PER_ENCODING);
            (void)mcs;
        },
        X224::write_x224_dt_tpdu_fn{}
    );

    LOG_IF(bool(this->verbose & RDPVerbose::connection), LOG_INFO,
        "Send MCS::AttachUserRequest");

    write_packets(
        this->trans,
        [](StreamSize<256> /*maxlen*/, OutStream & mcs_data){
            MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);
            (void)mcs;
        },
        X224::write_x224_dt_tpdu_fn{}
    );

    LOG_IF(bool(this->verbose & RDPVerbose::connection), LOG_INFO,
        "RdpNegociation: Basic Settings Exchange end");

    return true;
}

// Basic Settings Exchange
// -----------------------

// Basic Settings Exchange: Basic settings are exchanged between the client and
// server by using the MCS Connect Initial and MCS Connect Response PDUs. The
// Connect Initial PDU contains a GCC Conference Create Request, while the
// Connect Response PDU contains a GCC Conference Create Response.

// These two Generic Conference Control (GCC) packets contain concatenated
// blocks of settings data (such as core data, security data and network data)
// which are read by client and server


// Client                                                     Server
//    |--------------MCS Connect Initial PDU with-------------> |
//                   GCC Conference Create Request
//    | <------------MCS Connect Response PDU with------------- |
//                   GCC conference Create Response

void RdpNegociation::send_connectInitialPDUwithGccConferenceCreateRequest()
{
    char const* hostname = this->logon_info.hostname();

    /* Generic Conference Control (T.124) ConferenceCreateRequest */
    write_packets(
        this->trans,
        [this, &hostname](StreamSize<65536-1024> /*maxlen*/, OutStream & stream) {
            // 216: optional parameters up to serverSelectedProtocol
            // 234: optional parameters up to deviceScaleFactor
            const uint16_t cs_core_length = (allow_scale_factor && this->device_scale_factor > 0)
                ? 234 : 216;

            GCC::UserData::CSCore cs_core(cs_core_length);
            LOG(LOG_INFO, "Sending CS_CORE to server: color_depth %d", int(this->front_bpp));

            if (this->enable_remotefx) {
                cs_core.connectionType = GCC::UserData::CONNECTION_TYPE_LAN;
            }

            if (cs_core.connectionType != 0) {
                cs_core.earlyCapabilityFlags |= GCC::UserData::RNS_UD_CS_VALID_CONNECTION_TYPE;
            }

            Rect primary_monitor_rect = this->cs_monitor.get_primary_monitor_rect();

            cs_core.version = this->negociation_result.use_rdp5 ? 0x00080004 : 0x00080001;
            const bool single_monitor =
                (!this->allow_using_multiple_monitors ||
                    (this->cs_monitor.monitorCount < 2));
            cs_core.desktopWidth  = (single_monitor ? this->negociation_result.front_width : primary_monitor_rect.cx + 1);
            cs_core.desktopHeight = (single_monitor ? this->negociation_result.front_height : primary_monitor_rect.cy + 1);
            //cs_core.highColorDepth = this->front_bpp;
            cs_core.highColorDepth = ((this->front_bpp == BitsPerPixel{32})
                ? uint16_t(GCC::UserData::HIGH_COLOR_24BPP)
                : safe_cast<uint16_t>(this->front_bpp));
            cs_core.keyboardLayout = this->keylayout;
            if (this->front_bpp == BitsPerPixel{32}) {
                LOG(LOG_INFO, "Asking for 32 bits to server");
                cs_core.supportedColorDepths = GCC::UserData::RNS_UD_24BPP_SUPPORT
                        | GCC::UserData::RNS_UD_16BPP_SUPPORT
                        | GCC::UserData::RNS_UD_15BPP_SUPPORT
                        | GCC::UserData::RNS_UD_32BPP_SUPPORT;
                cs_core.earlyCapabilityFlags |= GCC::UserData::RNS_UD_CS_WANT_32BPP_SESSION;
            }
            if (!single_monitor) {
                LOG(LOG_INFO, "not a single_monitor");
                cs_core.earlyCapabilityFlags |= GCC::UserData::RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU;
            }
            else if (this->bogus_monitor_layout_treatment) {
                LOG(LOG_INFO, "RdpNegociation::send_connectInitialPDUwithGccConferenceCreateRequest: bogus_monitor_layout_treatment");
                cs_core.earlyCapabilityFlags &= ~GCC::UserData::RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU;
            }

            uint16_t hostlen = strlen(hostname);
            uint16_t maxhostlen = std::min(uint16_t(15), hostlen);
            for (size_t i = 0; i < maxhostlen ; i++){
                cs_core.clientName[i] = byte_ptr(hostname)[i];
            }
            memset(&(cs_core.clientName[maxhostlen]), 0, (16 - maxhostlen) * sizeof(uint16_t));

            /// Note: forwarding may be required for correct smartcard support; see issue #27767.
            if (this->forward_build_number) {
                cs_core.clientBuild = this->build_number;
            }

            if (this->nego.tls){
                cs_core.serverSelectedProtocol = this->nego.selected_protocol;
            }

            cs_core.desktopPhysicalWidth = this->desktop_physical_width;
            cs_core.desktopPhysicalHeight = this->desktop_physical_height;
            cs_core.desktopOrientation = this->desktop_orientation;
            cs_core.desktopScaleFactor = this->desktop_scale_factor;
            cs_core.deviceScaleFactor = this->device_scale_factor;

            if (bool(this->verbose & RDPVerbose::security)) {
                cs_core.log("Sending to Server");
            }
            cs_core.emit(stream, cs_core_length);
            // --------------------------------------------------------------------------------

            GCC::UserData::CSCluster cs_cluster;
            {
                LOG(LOG_INFO, "CS_Cluster: Server Redirection Supported");
                if (!this->nego.tls){
                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTION_SUPPORTED;
                    cs_cluster.flags |= (2 << 2); // REDIRECTION V3
                } else {
                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTION_SUPPORTED;
                    cs_cluster.flags |= (3 << 2);  // REDIRECTION V4
                }
                if (this->redir_info.valid) {
                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID;
                    cs_cluster.redirectedSessionID = this->redir_info.session_id;
                    LOG(LOG_INFO, "Effective Redirection SessionId=%u",
                        cs_cluster.redirectedSessionID);
                }
                if (this->console_session) {
                    cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID;
                    cs_cluster.redirectedSessionID = 0;
                    LOG(LOG_INFO, "Requires session (Console) for administration");
                }
            }
            if (bool(this->verbose & RDPVerbose::security)) {
                cs_cluster.log("Sending to server");
            }
            cs_cluster.emit(stream);
            // ------------------------------------------------------------

            {
                GCC::UserData::CSSecurity cs_security;
                if (bool(this->verbose & RDPVerbose::security)) {
                    cs_security.log("Sending to server");
                }
                cs_security.emit(stream);
            }
            // ------------------------------------------------------------

            const CHANNELS::ChannelDefArray & channel_list = this->front.get_channel_list();
            size_t num_channels = channel_list.size();
            if ((num_channels > 0)
            || this->enable_auth_channel
            || this->has_managed_drive
            || this->checkout_channel.c_str()[0]) {
                /* Here we need to put channel information in order
                to redirect channel data
                from client to server passing through the "proxy" */
                GCC::UserData::CSNet cs_net;
                cs_net.channelCount = 0;
                bool has_cliprdr_channel = false;
                bool has_rdpdr_channel   = false;
                bool has_rdpsnd_channel  = false;



                for (size_t index = 0, adjusted_index = 0; index < num_channels; index++) {
                    const CHANNELS::ChannelDef & channel_item = channel_list[index];

                    channel_item.log(index);

                    if (!this->remote_program && channel_item.name == channel_names::rail) {
                        continue;
                    }

                    if (this->channels_authorizations.is_authorized(channel_item.name)
                     || ((channel_item.name == channel_names::rdpdr
                       || channel_item.name == channel_names::rdpsnd)
                      && this->has_managed_drive)
                    ) {
                        switch (channel_item.name) {
                            case channel_names::cliprdr: has_cliprdr_channel = true; break;
                            case channel_names::rdpdr:   has_rdpdr_channel = true; break;
                            case channel_names::rdpsnd:  has_rdpsnd_channel = true; break;
                        }
                        ::memcpy(cs_net.channelDefArray[adjusted_index].name, channel_item.name.c_str(), 8);
                    }
                    else {
                        continue;
                    }
                    cs_net.channelDefArray[adjusted_index].options = channel_item.flags;
                    CHANNELS::ChannelDef def;
                    def.name = CHANNELS::ChannelNameId(cs_net.channelDefArray[adjusted_index].name);
                    def.flags = cs_net.channelDefArray[adjusted_index].options;
                    if (bool(this->verbose & RDPVerbose::channels)) {
                        def.log(adjusted_index);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                    adjusted_index++;
                }

                // Inject a new channel for file system virtual channel (rdpdr)
                if (!has_rdpdr_channel && this->has_managed_drive) {
                    LOG(LOG_INFO, "Inject rdpdr");

                    ::snprintf(cs_net.channelDefArray[cs_net.channelCount].name,
                            sizeof(cs_net.channelDefArray[cs_net.channelCount].name),
                            "%s", channel_names::rdpdr.c_str());
                    cs_net.channelDefArray[cs_net.channelCount].options =
                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                        | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP;
                    CHANNELS::ChannelDef def;
                    def.name = channel_names::rdpdr;
                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                    if (bool(this->verbose & RDPVerbose::channels)){
                        def.log(cs_net.channelCount);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                }

                // Inject a new channel for clipboard channel (cliprdr)
                if (!has_cliprdr_channel && this->enable_session_probe && this->session_probe_use_clipboard_based_launcher) {
                    LOG(LOG_INFO, "Inject cliprdr");
                    ::snprintf(cs_net.channelDefArray[cs_net.channelCount].name,
                            sizeof(cs_net.channelDefArray[cs_net.channelCount].name),
                            "%s", channel_names::cliprdr.c_str());
                    cs_net.channelDefArray[cs_net.channelCount].options =
                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                        | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                        | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL;
                    CHANNELS::ChannelDef def;
                    def.name = channel_names::cliprdr;
                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                    if (bool(this->verbose & RDPVerbose::channels)){
                        def.log(cs_net.channelCount);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                }

                // The RDPDR channel advertised by the client is ONLY accepted by the RDP
                //  server 2012 if the RDPSND channel is also advertised.
                if (!has_rdpsnd_channel && this->has_managed_drive) {
                    LOG(LOG_INFO, "Inject rdpsnd");
                    ::snprintf(cs_net.channelDefArray[cs_net.channelCount].name,
                            sizeof(cs_net.channelDefArray[cs_net.channelCount].name),
                            "%s", channel_names::rdpsnd.c_str());
                    cs_net.channelDefArray[cs_net.channelCount].options =
                          GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                        | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP;
                    CHANNELS::ChannelDef def;
                    def.name = channel_names::rdpsnd;
                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                    if (bool(this->verbose & RDPVerbose::channels)){
                        def.log(cs_net.channelCount);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                }

                // Inject a new channel for auth_channel virtual channel (wablauncher)
                if (this->enable_auth_channel) {
                    LOG(LOG_INFO, "Inject auth_channel");
                    assert(this->auth_channel.c_str()[0]);
                    memcpy(cs_net.channelDefArray[cs_net.channelCount].name, this->auth_channel.c_str(), 8);
                    cs_net.channelDefArray[cs_net.channelCount].options =
                        GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED;
                    CHANNELS::ChannelDef def;
                    def.name = this->auth_channel;
                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                    if (bool(this->verbose & RDPVerbose::channels)){
                        def.log(cs_net.channelCount);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                }

                // Inject a new channel for checkout_channel virtual channel
                if (this->checkout_channel.c_str()[0]) {
                    LOG(LOG_INFO, "Inject checkout channel");
                    memcpy(cs_net.channelDefArray[cs_net.channelCount].name, this->checkout_channel.c_str(), 8);
                    cs_net.channelDefArray[cs_net.channelCount].options =
                        GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED;
                    CHANNELS::ChannelDef def;
                    def.name = this->checkout_channel;
                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                    if (bool(this->verbose & RDPVerbose::channels)){
                        def.log(cs_net.channelCount);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                }

                if (this->enable_session_probe) {
                    LOG(LOG_INFO, "Enable session probe");
                    memcpy(cs_net.channelDefArray[cs_net.channelCount].name, channel_names::sespro.c_str(), 8);
                    cs_net.channelDefArray[cs_net.channelCount].options =
                        GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED;
                    CHANNELS::ChannelDef def;
                    def.name = channel_names::sespro;
                    def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                    if (bool(this->verbose & RDPVerbose::channels)){
                        def.log(cs_net.channelCount);
                    }
                    this->mod_channel_list.push_back(def);
                    cs_net.channelCount++;
                }

                if (this->convert_remoteapp_to_desktop) {
                    LOG(LOG_INFO, "Convert remote app to desktop");
                    {
                        memcpy(cs_net.channelDefArray[cs_net.channelCount].name, channel_names::rail.c_str(), 8);
                        cs_net.channelDefArray[cs_net.channelCount].options =
                              GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                            | GCC::UserData::CSNet::CHANNEL_OPTION_ENCRYPT_RDP
                            | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                            | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                            | GCC::UserData::CSNet::REMOTE_CONTROL_PERSISTENT;
                        CHANNELS::ChannelDef def;
                        def.name = channel_names::rail;
                        def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                        if (bool(this->verbose & RDPVerbose::channels)){
                            def.log(cs_net.channelCount);
                        }
                        this->mod_channel_list.push_back(def);
                        cs_net.channelCount++;
                    }

                    {
                        memcpy(cs_net.channelDefArray[cs_net.channelCount].name, channel_names::rail_wi.c_str(), 8);
                        cs_net.channelDefArray[cs_net.channelCount].options =
                              GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                            | GCC::UserData::CSNet::CHANNEL_OPTION_ENCRYPT_RDP
                            | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                            | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                            | GCC::UserData::CSNet::REMOTE_CONTROL_PERSISTENT;
                        CHANNELS::ChannelDef def;
                        def.name = channel_names::rail_wi;
                        def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                        if (bool(this->verbose & RDPVerbose::channels)){
                            def.log(cs_net.channelCount);
                        }
                        this->mod_channel_list.push_back(def);
                        cs_net.channelCount++;
                    }

                    {
                        memcpy(cs_net.channelDefArray[cs_net.channelCount].name, channel_names::rail_ri.c_str(), 8);
                        cs_net.channelDefArray[cs_net.channelCount].options =
                              GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                            | GCC::UserData::CSNet::CHANNEL_OPTION_ENCRYPT_RDP
                            | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                            | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                            | GCC::UserData::CSNet::REMOTE_CONTROL_PERSISTENT;
                        CHANNELS::ChannelDef def;
                        def.name = channel_names::rail_ri;
                        def.flags = cs_net.channelDefArray[cs_net.channelCount].options;
                        if (bool(this->verbose & RDPVerbose::channels)){
                            def.log(cs_net.channelCount);
                        }
                        this->mod_channel_list.push_back(def);
                        cs_net.channelCount++;
                    }
                }

                if (bool(this->verbose & RDPVerbose::channels)) {
                    cs_net.log("Sending to server");
                }
                cs_net.emit(stream);
            }

            if (!single_monitor) {
                //if (bool(this->verbose & RDPVerbose::security)) {
                    this->cs_monitor.log("Sending to server");
                //}
                this->cs_monitor.emit(stream);

                if (allow_scale_factor
                 && this->cs_monitor.monitorCount == this->cs_monitor_ex.monitorCount
                ) {
                    this->cs_monitor_ex.log("Sending to server");
                    this->cs_monitor_ex.emit(stream);
                }
            }
        },
        [](StreamSize<256> /*maxlen*/, OutStream & gcc_header, std::size_t packet_size) {
            GCC::Create_Request_Send(gcc_header, packet_size);
        },
        [](StreamSize<256> /*maxlen*/, OutStream & mcs_header, std::size_t packet_size) {
            MCS::CONNECT_INITIAL_Send mcs(mcs_header, packet_size, MCS::BER_ENCODING);
            (void)mcs;
        },
        X224::write_x224_dt_tpdu_fn{}
    );
}

bool RdpNegociation::channel_connection_attach_user(InStream & stream)
{
    LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO,
        "RdpNegociation: Channel Connection Attach User");

    X224::DT_TPDU_Recv x224(stream);
    InStream & mcs_cjcf_data = x224.payload;
    MCS::AttachUserConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
    if (mcs.initiator_flag){
        this->negociation_result.userid = mcs.initiator;
    }

    size_t const num_channels = this->mod_channel_list.size();

    uint16_t channels_id[CHANNELS::MAX_STATIC_VIRTUAL_CHANNELS + 2];
    channels_id[0] = this->negociation_result.userid + GCC::MCS_USERCHANNEL_BASE;
    channels_id[1] = GCC::MCS_GLOBAL_CHANNEL;
    for (size_t index = 0; index < num_channels; index++){
        channels_id[index+2] = this->mod_channel_list[index].chanid;
    }

    for (size_t index = 0; index < num_channels+2; index++) {
        LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO,
            "cjrq[%zu] = %" PRIu16, index, channels_id[index]);
        write_packets(
            this->trans,
            [this, &channels_id, index](StreamSize<256> /*maxlen*/, OutStream & mcs_cjrq_data){
                MCS::ChannelJoinRequest_Send mcs(
                    mcs_cjrq_data, this->negociation_result.userid,
                    channels_id[index], MCS::PER_ENCODING
                );
                (void)mcs;
            },
            X224::write_x224_dt_tpdu_fn{}
        );
    }

    if (bool(this->verbose & RDPVerbose::channels)){
        LOG(LOG_INFO, "RdpNegociation: Channel Connection Attach User end");
        LOG(LOG_INFO, "RdpNegociation: Waiting for Channel Join Confirm");
    }

    this->send_channel_index = 0;
    return true;
}

bool RdpNegociation::channel_join_confirm(InStream & x224_data)
{
    {
        X224::DT_TPDU_Recv x224(x224_data);
        InStream & mcs_cjcf_data = x224.payload;
        MCS::ChannelJoinConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
        // TODO If mcs.result is negative channel is not confirmed and should be removed from mod_channel list
        LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO,
            "cjcf[%zu] = %" PRIu16, this->send_channel_index, mcs.channelId);
    }

    ++this->send_channel_index;
    if (this->send_channel_index < this->mod_channel_list.size()+2) {
        return false;
    }
    LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO,
        "RdpNegociation: Channel Join Confirme end");
    return true;
}

bool RdpNegociation::get_license(InStream & stream)
{
    LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO, "RdpNegociation: Licensing");

    bool r = false;

    // Licensing
    // ---------

    // Licensing: The goal of the licensing exchange is to transfer a
    // license from the server to the client.

    // The client should store this license and on subsequent
    // connections send the license to the server for validation.
    // However, in some situations the client may not be issued a
    // license to store. In effect, the packets exchanged during this
    // phase of the protocol depend on the licensing mechanisms
    // employed by the server. Within the context of this document
    // we will assume that the client will not be issued a license to
    // store. For details regarding more advanced licensing scenarios
    // that take place during the Licensing Phase, see [MS-RDPELE].

    // Client                                                     Server
    //    | <------ License Error PDU Valid Client ---------------- |

    // 2.2.1.12 Server License Error PDU - Valid Client
    // ================================================

    // The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
    // from server to client during the Licensing phase of the RDP Connection
    // Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
    // phases). This licensing PDU indicates that the server will not issue the
    // client a license to store and that the Licensing Phase has ended
    // successfully. This is one possible licensing PDU that may be sent during the
    // Licensing Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible
    // licensing PDUs).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

    // mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU)
    // which encapsulates an MCS Send Data Indication structure (SDin, choice 26
    // from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure
    // definitions are given in [T125] section 7, parts 7 and 10). The userData
    // field of the MCS Send Data Indication contains a Security Header and a Valid
    // Client License Data (section 2.2.1.12.1) structure.

    // securityHeader (variable): Security header. The format of the security header
    // depends on the Encryption Level and Encryption Method selected by the server
    // (sections 5.3.2 and 2.2.1.4.3).

    // This field MUST contain one of the following headers:
    //  - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
    // selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW
    // (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008)
    // flag.
    //  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method
    // selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
    // ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
    // (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008)
    // flag.
    //  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
    // selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the
    // embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

    // If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2),
    // ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field
    // of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the
    // licensing PDU is not encrypted), then the field MUST contain a Basic Security
    // Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not
    // set on the Security Exchange PDU (section 2.2.1.10).

    // The flags field of the security header MUST contain the SEC_LICENSE_PKT
    // (0x0080) flag (see Basic (TS_SECURITY_HEADER)).

    // validClientLicenseData (variable): The actual contents of the License Error
    // (Valid Client) PDU, as specified in section 2.2.1.12.1.

    const char * hostname = this->logon_info.hostname();
    const char * username;
    char username_a_domain[512];
    if (this->logon_info.domain().c_str()[0]) {
        snprintf(username_a_domain, sizeof(username_a_domain), "%s@%s",
            this->logon_info.username().c_str(), this->logon_info.domain().c_str());
        username = username_a_domain;
    }
    else {
        username = this->logon_info.username().c_str();
    }
    LOG(LOG_INFO, "RdpNegociation: Get license: username=\"%s\"", username);
    // read tpktHeader (4 bytes = 3 0 len)
    // TPDU class 0    (3 bytes = LI F0 PDU_DT)

    X224::DT_TPDU_Recv x224(stream);
    // TODO Shouldn't we use mcs_type to manage possible Deconnection Ultimatum here
    //int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);
    MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);
    SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->negociation_result.encryptionLevel);

    if (sec.flags & SEC::SEC_LICENSE_PKT) {
        LIC::RecvFactory flic(sec.payload);

        LOG(LOG_INFO, "RdpNegociation: get_license LIC::RecvFactory::bMsgType=%u", flic.tag);

        switch (flic.tag) {
        case LIC::LICENSE_REQUEST:
            LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO, "RdpNegociation: Server License Request");
            {
                LIC::ServerLicenseRequest SvrLicReq = LIC::ServerLicenseRequest_Receiver(sec.payload);
                if (bool(this->verbose & RDPVerbose::license)) {
                    SvrLicReq.log(LOG_INFO);
                }
                uint8_t null_data[48]{};
                /* We currently use null client keys. This is a bit naughty but, hey,
                    the security of license negotiation isn't exactly paramount. */
                SEC::SessionKey keyblock(null_data, null_data, SvrLicReq.ServerRandom);

                /* Store first 16 bytes of session key as MAC secret */
                memcpy(this->lic_layer_license_sign_key, keyblock.get_MAC_salt_key(), 16);
                memcpy(this->lic_layer_license_key, keyblock.get_LicensingEncryptionKey(), 16);

                if (this->use_license_store) {
                    uint8_t CompanyNameU8[4096] {};
                    ::UTF16toUTF8_buf(SvrLicReq.ProductInfo.CompanyName, writable_bytes_view{CompanyNameU8, sizeof(CompanyNameU8)});

                    uint8_t ProductIdU8[4096] {};
                    ::UTF16toUTF8_buf(SvrLicReq.ProductInfo.ProductId, writable_bytes_view{ProductIdU8, sizeof(ProductIdU8)});

                    for (uint32_t i = 0; i < SvrLicReq.ScopeList.ScopeCount; ++i) {
                        bytes_view out = this->license_store.get_license(
                                this->license_client_name.c_str(),
                                SvrLicReq.ProductInfo.dwVersion,
                                ::char_ptr_cast(SvrLicReq.ScopeList.ScopeArray[i].blobData.data()),
                                ::char_ptr_cast(CompanyNameU8),
                                ::char_ptr_cast(ProductIdU8),
                                writable_bytes_view(this->lic_layer_license_data, sizeof(lic_layer_license_data)),
                                bool(this->verbose & RDPVerbose::license)
                            );
                        if (not out.empty())
                        {
                            this->lic_layer_license_size = out.size();

                            LOG(LOG_INFO, "RdpNegociation: LicenseSize=%zu", this->lic_layer_license_size);

                            break;
                        }
                    }
                }
            }

            this->send_data_request(
                GCC::MCS_GLOBAL_CHANNEL,
                [this, &hostname, &username](StreamSize<65535 - 1024>, OutStream & lic_data) {
                    if (this->lic_layer_license_size > 0) {
                        LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO, "RdpNegociation: Client License Info");
                        uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                        OutStream(hwid).out_uint32_le(2);
                        memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                        /* Generate a signature for the HWID buffer */
                        uint8_t signature[LIC::LICENSE_SIGNATURE_SIZE];

                        uint8_t lenhdr[4];
                        OutStream(lenhdr).out_uint32_le(sizeof(hwid));

                        Sign sign(make_array_view(this->lic_layer_license_sign_key));
                        sign.update(make_array_view(lenhdr));
                        sign.update(make_array_view(hwid));

                        static_assert(static_cast<size_t>(SslMd5::DIGEST_LENGTH) == static_cast<size_t>(LIC::LICENSE_SIGNATURE_SIZE));
                        sign.final(signature);

                        /* Now encrypt the HWID */

                        SslRC4 rc4;
                        rc4.set_key(make_array_view(this->lic_layer_license_key));

                        // in, out
                        rc4.crypt(LIC::LICENSE_HWID_SIZE, hwid, hwid);

                        LIC::ClientLicenseInfo_Send(
                            lic_data, this->negociation_result.use_rdp5 ? 3 : 2,
                            this->lic_layer_license_size,
                            this->lic_layer_license_data,
                            hwid, signature
                        );
                    }
                    else {
                        LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO, "RdpNegociation: Client New License Request");
                        LIC::NewLicenseRequest_Send(
                            lic_data, this->negociation_result.use_rdp5 ? 3 : 2,
                            username, hostname
                        );
                    }
                },
                SEC::write_sec_send_fn{SEC::SEC_LICENSE_PKT, this->encrypt, 0}
            );
            break;
        case LIC::PLATFORM_CHALLENGE:
            LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO,
                "RdpNegociation: Server Platform Challenge");

            {
                LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO,
                    "RdpNegociation: Client Platform Challenge Response");

                LIC::PlatformChallenge_Recv lic(sec.payload);

                uint8_t out_token[LIC::LICENSE_TOKEN_SIZE];
                uint8_t decrypt_token[LIC::LICENSE_TOKEN_SIZE];
                uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                uint8_t crypt_hwid[LIC::LICENSE_HWID_SIZE];
                uint8_t out_sig[LIC::LICENSE_SIGNATURE_SIZE];

                memcpy(out_token, lic.encryptedPlatformChallenge.blob, LIC::LICENSE_TOKEN_SIZE);
                /* Decrypt the token. It should read TEST in Unicode. */
                memcpy(decrypt_token, lic.encryptedPlatformChallenge.blob, LIC::LICENSE_TOKEN_SIZE);
                SslRC4 rc4_decrypt_token;
                rc4_decrypt_token.set_key(make_array_view(this->lic_layer_license_key));
                // size, in, out
                rc4_decrypt_token.crypt(LIC::LICENSE_TOKEN_SIZE, decrypt_token, decrypt_token);

                /* Generate a signature for a buffer of token and HWID */
                OutStream(hwid).out_uint32_le(2);
                memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                uint8_t sealed_buffer[LIC::LICENSE_TOKEN_SIZE + LIC::LICENSE_HWID_SIZE];
                memcpy(sealed_buffer, decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                memcpy(sealed_buffer + LIC::LICENSE_TOKEN_SIZE, hwid, LIC::LICENSE_HWID_SIZE);

                uint8_t lenhdr[4];
                OutStream(lenhdr).out_uint32_le(sizeof(sealed_buffer));

                Sign sign(make_array_view(this->lic_layer_license_sign_key));
                sign.update(make_array_view(lenhdr));
                sign.update(make_array_view(sealed_buffer));

                static_assert(static_cast<size_t>(SslMd5::DIGEST_LENGTH) == static_cast<size_t>(LIC::LICENSE_SIGNATURE_SIZE));
                sign.final(out_sig);

                /* Now encrypt the HWID */
                memcpy(crypt_hwid, hwid, LIC::LICENSE_HWID_SIZE);
                SslRC4 rc4_hwid;
                rc4_hwid.set_key(make_array_view(this->lic_layer_license_key));
                // size, in, out
                rc4_hwid.crypt(LIC::LICENSE_HWID_SIZE, crypt_hwid, crypt_hwid);

                this->send_data_request(
                    GCC::MCS_GLOBAL_CHANNEL,
                    [&, this](StreamSize<65535 - 1024>, OutStream & lic_data) {
                        LIC::ClientPlatformChallengeResponse_Send(
                            lic_data, this->negociation_result.use_rdp5 ? 3 : 2,
                            out_token, crypt_hwid, out_sig
                        );
                    },
                    SEC::write_sec_send_fn{SEC::SEC_LICENSE_PKT, this->encrypt, 0}
                );
            }
            break;
        case LIC::NEW_LICENSE:
        case LIC::UPGRADE_LICENSE:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO, "RdpNegociation: Server %s License",
                    ((LIC::NEW_LICENSE == flic.tag) ? "New" : "Upgrade"));

                LIC::NewOrUpgradeLicense_Recv lic(sec.payload, this->lic_layer_license_key,
                    ((LIC::NEW_LICENSE == flic.tag) ? LIC::NewOrUpgradeLicense_Recv::Type::New : LIC::NewOrUpgradeLicense_Recv::Type::Upgrade));
                if (bool(this->verbose & RDPVerbose::license)) {
                    lic.log(LOG_INFO);
                }

                // TODO CGR: Save license to keep a local copy of the license of a remote server thus avoiding to ask it every time we connect.
                // Not obvious files is the best choice to do that

                bool license_saved = false;

                if (this->use_license_store) {
                    uint8_t CompanyNameU8[4096] {};
                    ::UTF16toUTF8_buf(bytes_view(lic.licenseInfo.pbCompanyName, lic.licenseInfo.cbCompanyName), writable_bytes_view{CompanyNameU8, sizeof(CompanyNameU8)});

                    uint8_t ProductIdU8[4096] {};
                    ::UTF16toUTF8_buf(bytes_view(lic.licenseInfo.pbProductId, lic.licenseInfo.cbProductId), writable_bytes_view{ProductIdU8, sizeof(ProductIdU8)});

                    license_saved = this->license_store.put_license(
                            this->license_client_name.c_str(),
                            lic.licenseInfo.dwVersion,
                            ::char_ptr_cast(lic.licenseInfo.pbScope),
                            ::char_ptr_cast(CompanyNameU8),
                            ::char_ptr_cast(ProductIdU8),
                            bytes_view(lic.licenseInfo.pbLicenseInfo, lic.licenseInfo.cbLicenseInfo),
                            bool(this->verbose & RDPVerbose::license)
                        );
                }

                if (!license_saved) {
                    LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_WARNING, "RdpNegociation: %s license not saved.",
                        ((LIC::NEW_LICENSE == flic.tag) ? "New" : "Upgrade"));
                }

                r = true;
            }
            break;
        case LIC::ERROR_ALERT:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::license), LOG_INFO,
                    "RdpNegociation: Server License Error Message");

                LIC::ErrorAlert_Recv lic(sec.payload);
                if ((lic.validClientMessage.dwErrorCode != LIC::STATUS_VALID_CLIENT)
                 || (lic.validClientMessage.dwStateTransition != LIC::ST_NO_TRANSITION)){
                    LOG(LOG_ERR, "RdpNegociation: License Alert: dwErrorCode=%s(%u) dwStateTransition=%s(%u)",
                        LIC::ValidClientMessage::ErrorCodeToString(lic.validClientMessage.dwErrorCode), lic.validClientMessage.dwErrorCode,
                        LIC::ValidClientMessage::StateTransitionToString(lic.validClientMessage.dwStateTransition), lic.validClientMessage.dwStateTransition);
                }
                r = true;
            }
            break;
        default:
            LOG(LOG_ERR, "RdpNegociation: Unexpected licensing packet sent from server. bMsgType = 0x%X", flic.tag);
            throw Error(ERR_SEC);
        }

        if (sec.payload.get_current() != sec.payload.get_data_end()){
            LOG(LOG_ERR, "RdpNegociation: All data should have been consumed. bMsgType= 0x%X", flic.tag);
            throw Error(ERR_SEC);
        }
    }
    else {
        LOG(LOG_WARNING, "RdpNegociation: Failed to get expected license negotiation PDU. sec.flags=0x%X", sec.flags);
        hexdump(x224.payload.get_data(), x224.payload.get_capacity());
        //throw Error(ERR_SEC);
        r = true;
        hexdump(sec.payload.get_data(), sec.payload.get_capacity());
    }
    return r;
}

// TODO same in mod_rdp
template<class... WriterData>
void RdpNegociation::send_data_request(uint16_t channelId, WriterData... writer_data) {
    LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "send data request");

    write_packets(
        this->trans,
        writer_data...,
        [this, channelId](StreamSize<256> /*maxlen*/, OutStream & mcs_header, std::size_t packet_size) {
            MCS::SendDataRequest_Send mcs(
                mcs_header, this->negociation_result.userid,
                channelId, 1, 3, packet_size, MCS::PER_ENCODING
            );

            (void)mcs;
        },
        X224::write_x224_dt_tpdu_fn{}
    );
    LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "send data request done");
}

void RdpNegociation::send_client_info_pdu()
{
    LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO,
        "RdpNegociation: send_client_info_pdu");

    InfoPacket infoPacket( this->negociation_result.use_rdp5
                            , this->logon_info.domain().c_str()
                            , this->logon_info.username().c_str()
                            , (this->nego.restricted_admin_mode ? "" : this->password)
                            , this->program
                            , this->directory
                            , this->performanceFlags
                            , this->clientAddr
                            );
    infoPacket.extendedInfoPacket.clientTimeZone = this->client_time_zone;
    infoPacket.flags |= this->info_packet_extra_flags;
    if (bool(this->rdp_compression)) {
        infoPacket.flags |= INFO_COMPRESSION;
        infoPacket.flags &= ~CompressionTypeMask;
        infoPacket.flags |= (static_cast<unsigned>(this->rdp_compression) - 1) << 9;
    }

    if (this->enable_session_probe) {
        // INFO_MAXIMIZESHELL is necessary for XRDP target.
        infoPacket.flags &= ~INFO_MAXIMIZESHELL;
    }

    if (this->remote_program) {
        infoPacket.flags |= INFO_RAIL;

        // if (this->remote_program_enhanced) {
        //    infoPacket.flags |= INFO_HIDEF_RAIL_SUPPORTED;
        // }
    }

    if (!this->channels_authorizations.rdpsnd_audio_output_is_authorized()) {
        infoPacket.flags &= ~INFO_REMOTECONSOLEAUDIO;
        infoPacket.flags |=  INFO_NOAUDIOPLAYBACK;
    }

    if (!this->channels_authorizations.rdpsnd_audio_input_is_authorized()) {
        infoPacket.flags &= ~INFO_AUDIOCAPTURE;
    }

    if (this->perform_automatic_reconnection) {
        InStream in_s(this->server_auto_reconnect_packet_ref);
        RDP::ServerAutoReconnectPacket server_auto_reconnect_packet;
        server_auto_reconnect_packet.receive(in_s);

        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "Use Server Auto-Reconnect Packet");
            LOG(LOG_INFO, "Server Reconnect Random");
            hexdump(server_auto_reconnect_packet.ArcRandomBits,
                sizeof(server_auto_reconnect_packet.ArcRandomBits));
        }

        OutStream out_s(infoPacket.extendedInfoPacket.autoReconnectCookie);

        uint8_t digest[SslMd5::DIGEST_LENGTH] = { 0 };

        SslHMAC_Md5 hmac_md5(make_array_view(server_auto_reconnect_packet.ArcRandomBits));
        if (!this->nego.enhanced_rdp_security_is_in_effect()) {
            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO,
                "Use client random");
            hmac_md5.update(make_array_view(this->client_random));
        }
        else {
            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO,
                "Use NULL client random");
            uint8_t tmp_client_random[32] = { 0 };
            hmac_md5.update(make_array_view(tmp_client_random));
        }
        hmac_md5.final(digest);

        infoPacket.extendedInfoPacket.cbAutoReconnectLen = 0x1C;

        out_s.out_uint32_le(0x1C);  // cbLen(4)
        out_s.out_uint32_le(1);     // Version(4)
        out_s.out_uint32_le(server_auto_reconnect_packet.LogonId);  // LogonId(4)
        out_s.out_copy_bytes(digest, sizeof(digest));
        if (bool(this->verbose & RDPVerbose::security)){
            LOG(LOG_INFO, "Client Security Verifier");
            hexdump(digest, sizeof(digest));
        }
    }
    else if (this->cbAutoReconnectCookie) {
        infoPacket.extendedInfoPacket.cbAutoReconnectLen = this->cbAutoReconnectCookie;
        ::memcpy(infoPacket.extendedInfoPacket.autoReconnectCookie, this->autoReconnectCookie,
            sizeof(infoPacket.extendedInfoPacket.autoReconnectCookie));
    }

    this->send_data_request(
        GCC::MCS_GLOBAL_CHANNEL,
        [&infoPacket](StreamSize<2048> /*maxlen*/, OutStream & stream) {
            infoPacket.emit(stream);
        },
        SEC::write_sec_send_fn{SEC::SEC_INFO_PKT, this->encrypt, this->negociation_result.encryptionLevel}
    );

    if (bool(this->verbose & RDPVerbose::basic_trace)) {
        infoPacket.log("Send data request", this->password_printing_mode, !this->enable_session_probe);
    }

    LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO,
        "RdpNegociation: send_client_info_pdu done");
}

RdpNegociationResult const& RdpNegociation::get_result() const noexcept
{
    assert(this->state == State::TERMINATED);
    return this->negociation_result;
}
