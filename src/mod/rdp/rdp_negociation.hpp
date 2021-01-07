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

#pragma once

#include "acl/license_api.hpp"

#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "core/RDP/gcc/userdata/cs_monitor_ex.hpp"
#include "core/RDP/logon.hpp"
#include "core/RDP/nego.hpp"
#include "core/channel_names.hpp"
#include "core/server_notifier_api.hpp"
#include "core/channels_authorizations.hpp"
#include "gdi/screen_info.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "mod/rdp/rdp_negociation_data.hpp"
#include "utils/crypto/ssl_lib.hpp"
#include "acl/auth_api.hpp"

#include <functional> // std::reference_wrapper
#include <memory>
#include <array>

class ChannelsAuthorizations;
class ClientInfo;
class CryptContext;
class FrontAPI;
class ModRDPParams;
class Random;
class RedirectionInfo;
class Transport;
namespace CHANNELS
{
    class ChannelDefArray;
}

class RdpNegociation
{
public:
    enum class State : uint8_t {
        NEGO_INITIATE,
        NEGO,
        BASIC_SETTINGS_EXCHANGE,
        CHANNEL_CONNECTION_ATTACH_USER,
        CHANNEL_JOIN_CONFIRM,
        GET_LICENSE,
        TERMINATED,
    };

private:
    class RDPServerNotifier : public ServerNotifier
    {
    public:
        explicit RDPServerNotifier(
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
        ) noexcept;

        void server_cert_status(Status status, std::string_view error_msg = {}) override;

        CertificateResult server_cert_callback(X509& certificate, std::string* error_message, const char* ip_address, int port) override;

    private:
        friend class RdpNegociation;
        std::function<CertificateResult(X509&)> certificate_callback;

        const ServerCertCheck server_cert_check;
        std::unique_ptr<char[]> certif_path;
        const bool server_cert_store;
        const std::array<ServerNotification, 5> server_status_messages;

        const RDPVerbose verbose;
        SessionLogApi& session_log;
    };

private:
    State state = State::NEGO_INITIATE;

    CHANNELS::ChannelDefArray& mod_channel_list;
    const ChannelsAuthorizations channels_authorizations;
    const CHANNELS::ChannelNameId auth_channel;
    const CHANNELS::ChannelNameId checkout_channel;

    CryptContext& decrypt;
    CryptContext& encrypt;

    const bool enable_auth_channel;
    RedirectionInfo& redir_info;
    const RdpLogonInfo logon_info;

    FrontAPI& front;

    RdpNegociationResult negociation_result;

    uint16_t cbAutoReconnectCookie = 0;
    uint8_t autoReconnectCookie[28] = { 0 };

    const uint32_t keylayout;

    uint32_t server_public_key_len = 0;
    uint8_t client_crypt_random[512] {};

    const bool console_session;
    const BitsPerPixel front_bpp;
    const uint32_t performanceFlags;
    const ClientTimeZone client_time_zone;
    Random& gen;
    const RDPVerbose verbose;
    RDPServerNotifier server_notifier;
    RdpNego nego;

    const uint32_t desktop_physical_width;
    const uint32_t desktop_physical_height;
    const uint16_t desktop_orientation;
    const uint32_t desktop_scale_factor;
    const uint32_t device_scale_factor;

    Transport& trans;
    const uint32_t password_printing_mode;
    const bool enable_session_probe;
    const bool enable_remotefx;
    RdpCompression rdp_compression;
    const bool session_probe_use_clipboard_based_launcher;
    const bool remote_program;
    const bool bogus_sc_net_size;

    const bool allow_using_multiple_monitors;
    const bool allow_scale_factor;
    GCC::UserData::CSMonitor cs_monitor;
    GCC::UserData::CSMonitorEx cs_monitor_ex;

    const bool perform_automatic_reconnection;
    const std::array<uint8_t, 28> server_auto_reconnect_packet_ref;

    InfoPacketFlags info_packet_extra_flags;

    char clientAddr[512] = {};
    const bool has_managed_drive;
    const bool convert_remoteapp_to_desktop;
    char directory[512] = {};
    char program[512] = {};

    char password[2048] = {};

    size_t send_channel_index;

    size_t lic_layer_license_size = 0;
    uint8_t lic_layer_license_key[16] = {};
    uint8_t lic_layer_license_sign_key[16] = {};
    uint8_t lic_layer_license_data[4096] = {};

    uint8_t client_random[SEC_RANDOM_SIZE] = { 0 };

    std::string license_client_name;
    LicenseApi& license_store;
    const bool use_license_store;

    /// Client build number.
    /// Represents the 'clientBuild' field of Client Core Data (TS_UD_CS_CORE)
    /// as specified in [MS-RDPBCGR] §2.2.1.3.2.
    /// Note: must be forwarded from client to server
    /// for correct smartcard support; see issue #27767.
    const uint32_t build_number;

    /// Indicates whether to forward the client build number to the server.
    /// If set to 'false' the default (static) build number will be used.
    const bool forward_build_number;

public:
    RdpNegociation(
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
    );

    void set_program(char const* program, char const* directory) noexcept;

    void set_cert_callback(std::function<CertificateResult(X509&)> callback);

    void start_negociation();
    bool recv_data(TpduBuffer& buf);

    [[nodiscard]] RdpNegociationResult const& get_result() const noexcept;

    [[nodiscard]] State get_state() const noexcept
    {
        return this->state;
    }

private:
    bool basic_settings_exchange(InStream & x224_data);
    void send_connectInitialPDUwithGccConferenceCreateRequest();
    bool channel_connection_attach_user(InStream & stream);
    bool channel_join_confirm(InStream & x224_data);
    bool get_license(InStream & stream);

    template<class... WriterData>
    void send_data_request(uint16_t channelId, WriterData... writer_data);
    void send_client_info_pdu();
};
