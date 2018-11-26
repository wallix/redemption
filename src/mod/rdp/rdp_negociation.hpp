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

#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "core/RDP/logon.hpp"
#include "core/RDP/nego.hpp"
#include "core/channel_names.hpp"
#include "core/server_notifier_api.hpp"
#include "gdi/screen_info.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "mod/rdp/rdp_negociation_data.hpp"
#include "utils/crypto/ssl_lib.hpp"
#include "utils/key_qvalue_pairs.hpp"

#include <functional> // std::reference_wrapper
#include <memory>

class AuthorizationChannels;
class ClientInfo;
class CryptContext;
class FrontAPI;
class ModRDPParams;
class Random;
class RedirectionInfo;
class ReportMessageApi;
class TimeObj;
class Transport;
class ArcsightLogInfo;
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
        GET_LICENSE
    };

private:
    class RDPServerNotifier : public ServerNotifier
    {
    public:
        explicit RDPServerNotifier(
            FrontAPI& front,
            ReportMessageApi& report_message,
            ServerNotification server_access_allowed_message,
            ServerNotification server_cert_create_message,
            ServerNotification server_cert_success_message,
            ServerNotification server_cert_failure_message,
            ServerNotification server_cert_error_message,
            RDPVerbose verbose
        ) noexcept;

        void server_access_allowed() override;
        void server_cert_create() override;
        void server_cert_success() override;
        void server_cert_failure() override;
        void server_cert_error(const char * str_error) override;

    private:
        const ServerNotification server_access_allowed_message;
        const ServerNotification server_cert_create_message;
        const ServerNotification server_cert_success_message;
        const ServerNotification server_cert_failure_message;
        const ServerNotification server_cert_error_message;

        const RDPVerbose verbose;
        KeyQvalueFormatter message;
        FrontAPI& front;
        ReportMessageApi& report_message;

        void log6_server_cert(charp_or_string type, charp_or_string description, const ArcsightLogInfo & arc_info);
    };

private:
    State state = State::NEGO_INITIATE;

    CHANNELS::ChannelDefArray& mod_channel_list;
    const AuthorizationChannels& authorization_channels;
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

    const int keylayout;

    uint32_t server_public_key_len = 0;
    uint8_t client_crypt_random[512] {};

    const bool console_session;
    const BitsPerPixel front_bpp;
    const uint32_t performanceFlags;
    const ClientTimeZone client_time_zone;
    Random& gen;
    const RDPVerbose verbose;
    const bool            server_cert_store;
    const ServerCertCheck server_cert_check;
    std::unique_ptr<char[]> certif_path;
    RDPServerNotifier server_notifier;
    const RdpNego::ServerCert server_cert;
    RdpNego nego;

    Transport& trans;
    const uint32_t password_printing_mode;
    const bool enable_session_probe;
    const RdpCompression rdp_compression;
    const bool session_probe_use_clipboard_based_launcher;
    const bool remote_program;
    const bool bogus_sc_net_size;

    const bool allow_using_multiple_monitors;
    GCC::UserData::CSMonitor cs_monitor;

    const bool perform_automatic_reconnection;
    const std::array<uint8_t, 28> server_auto_reconnect_packet_ref;

    InfoPacketFlags info_packet_extra_flags;

    char clientAddr[512] = {};
    const bool has_managed_drive;
    char directory[512] = {};
    char program[512] = {};

    char password[2048] = {};

    size_t send_channel_index;

    size_t lic_layer_license_size = 0;
    uint8_t lic_layer_license_key[16] = {};
    uint8_t lic_layer_license_sign_key[16] = {};
    std::unique_ptr<uint8_t[]> lic_layer_license_data;

    uint8_t client_random[SEC_RANDOM_SIZE] = { 0 };

public:
    RdpNegociation(
        std::reference_wrapper<const AuthorizationChannels> authorization_channels,
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
        TimeObj& timeobj,
        const ModRDPParams& mod_rdp_params,
        ReportMessageApi& report_message,
        bool has_managed_drive
    );

    void set_program(char const* program, char const* directory) noexcept;
    void start_negociation();
    bool recv_data(TpduBuffer& buf);

    RdpNegociationResult const& get_result() const noexcept
    {
        assert(this->state == State::GET_LICENSE);
        return this->negociation_result;
    }

    State get_state() const noexcept
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
