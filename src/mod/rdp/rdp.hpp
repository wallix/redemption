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

#include "mod/rdp/rdp_orders.hpp"

#include "utils/stream.hpp"
#include "system/ssl_calls.hpp"
#include "mod/mod_api.hpp"
#include "acl/auth_api.hpp"
#include "core/report_message_api.hpp"
#include "core/front_api.hpp"

#include "core/server_notifier_api.hpp"
#include "core/RDP/x224.hpp"
#include "core/RDP/nego.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/lic.hpp"
#include "core/RDP/logon.hpp"
#include "core/channel_list.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/sec.hpp"
#include "utils/colors.hpp"
#include "core/RDP/autoreconnect.hpp"
#include "core/RDP/ServerRedirection.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/fastpath.hpp"
#include "core/RDP/PersistentKeyListPDU.hpp"
#include "core/RDP/protocol.hpp"
#include "core/RDP/RefreshRectPDU.hpp"
#include "core/RDP/SaveSessionInfoPDU.hpp"
#include "core/RDP/SuppressOutputPDU.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/mppc.hpp"
#include "core/RDP/capabilities/cap_bitmap.hpp"
#include "core/RDP/capabilities/order.hpp"
#include "core/RDP/capabilities/cap_bmpcache.hpp"
#include "core/RDP/capabilities/bmpcache2.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/capabilities/activate.hpp"
#include "core/RDP/capabilities/control.hpp"
#include "core/RDP/capabilities/pointer.hpp"
#include "core/RDP/capabilities/cap_share.hpp"
#include "core/RDP/capabilities/input.hpp"
#include "core/RDP/capabilities/cap_sound.hpp"
#include "core/RDP/capabilities/cap_font.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/capabilities/rail.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "core/RDP/capabilities/largepointer.hpp"
#include "core/RDP/capabilities/multifragmentupdate.hpp"
#include "core/RDP/capabilities/drawninegridcache.hpp"
#include "core/RDP/capabilities/drawgdiplus.hpp"
#include "core/RDP/capabilities/bitmapcachehostsupport.hpp"
#include "core/RDP/capabilities/surfacecommands.hpp"
#include "core/RDP/capabilities/bitmapcodecs.hpp"
#include "core/RDP/capabilities/frameacknowledge.hpp"
#include "core/RDP/capabilities/compdesk.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/RDPEA/audio_output.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/RDP/remote_programs.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "capture/transparentrecorder.hpp"

#include "core/client_info.hpp"
#include "utils/genrandom.hpp"
#include "utils/authorization_channels.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "core/channel_names.hpp"

#include "core/FSCC/FileInformation.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/drdynvc_channel.hpp"
#include "mod/rdp/channels/rail_channel.hpp"
#include "mod/rdp/channels/rail_session_manager.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/sespro_alternate_shell_based_launcher.hpp"
#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/sespro_clipboard_based_launcher.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/timeout.hpp"

#include <cstdlib>


class mod_rdp : public mod_api, public rdp_api
{
private:
    std::unique_ptr<VirtualChannelDataSender>     file_system_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     file_system_to_server_sender;

    std::unique_ptr<FileSystemVirtualChannel>     file_system_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     clipboard_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     clipboard_to_server_sender;

    std::unique_ptr<ClipboardVirtualChannel>      clipboard_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     dynamic_channel_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     dynamic_channel_to_server_sender;

    std::unique_ptr<DynamicChannelVirtualChannel> dynamic_channel_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     session_probe_to_server_sender;

    std::unique_ptr<SessionProbeVirtualChannel>   session_probe_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     remote_programs_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     remote_programs_to_server_sender;

    std::unique_ptr<RemoteProgramsVirtualChannel> remote_programs_virtual_channel;

    std::unique_ptr<RemoteProgramsSessionManager> remote_programs_session_manager;

    bool remote_apps_not_enabled = false;

protected:
    FileSystemDriveManager file_system_drive_manager;

    uint16_t front_width;
    uint16_t front_height;
    FrontAPI& front;

    class ToClientSender : public VirtualChannelDataSender
    {
        FrontAPI& front;
        const CHANNELS::ChannelDef& channel;
        const RDPVerbose verbose;

    public:
        ToClientSender(FrontAPI& front,
                       const CHANNELS::ChannelDef& channel,
                       RDPVerbose verbose)
        : front(front)
        , channel(channel)
        , verbose(verbose)
        {}

        void operator()(uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length)
                override
        {
            if ((
                bool(this->verbose & RDPVerbose::cliprdr_dump)
                && this->channel.name == channel_names::cliprdr
            ) || (
                bool(this->verbose & RDPVerbose::rdpdr_dump)
                && this->channel.name == channel_names::rdpdr
            )) {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, total_length, flags,
                    chunk_data, chunk_data_length);
            }

            this->front.send_to_channel(this->channel,
                chunk_data, total_length, chunk_data_length, flags);
        }
    };

    class ToServerSender : public VirtualChannelDataSender
    {
        OutTransport    transport;
        CryptContext&   encrypt;
        int             encryption_level;
        uint16_t        user_id;
        CHANNELS::ChannelNameId channel_name;
        uint16_t        channel_id;
        bool            show_protocol;

        const RDPVerbose verbose;

    public:
        ToServerSender(OutTransport transport,
                       CryptContext& encrypt,
                       int encryption_level,
                       uint16_t user_id,
                       CHANNELS::ChannelNameId channel_name,
                       uint16_t channel_id,
                       bool show_protocol,
                       RDPVerbose verbose)
        : transport(transport)
        , encrypt(encrypt)
        , encryption_level(encryption_level)
        , user_id(user_id)
        , channel_name(channel_name)
        , channel_id(channel_id)
        , show_protocol(show_protocol)
        , verbose(verbose)
        {}

        void operator()(uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length)
                override {
            CHANNELS::VirtualChannelPDU virtual_channel_pdu;

            if (this->show_protocol) {
                flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
            }

            if ((
                bool(this->verbose & RDPVerbose::cliprdr_dump)
                && this->channel_name == channel_names::cliprdr
            ) || (
                bool(this->verbose & RDPVerbose::rdpdr_dump)
                && this->channel_name == channel_names::rdpdr
            )) {
                const bool send              = true;
                const bool from_or_to_client = false;
                ::msgdump_c(send, from_or_to_client, total_length, flags,
                    chunk_data, chunk_data_length);
            }

            virtual_channel_pdu.send_to_server(this->transport,
                this->encrypt, this->encryption_level, this->user_id,
                this->channel_id, total_length, flags, chunk_data,
                chunk_data_length);
        }
    };

    CHANNELS::ChannelDefArray mod_channel_list;

    const AuthorizationChannels authorization_channels;

    data_size_type max_clipboard_data = 0;
    data_size_type max_rdpdr_data     = 0;
    data_size_type max_drdynvc_data   = 0;

    int  use_rdp5;

    uint16_t cbAutoReconnectCookie = 0;
    uint8_t  autoReconnectCookie[28] = { 0 };

    int  keylayout;

    uint8_t   lic_layer_license_key[16];
    uint8_t   lic_layer_license_sign_key[16];
    std::unique_ptr<uint8_t[]> lic_layer_license_data;
    size_t    lic_layer_license_size;

    rdp_orders orders;

    int      share_id;
    uint16_t userid;

    char hostname[HOST_NAME_MAX + 1]  = {};
    char username[128]  = {};
    char password[2048]  = {};
    char domain[256]  = {};
    char program[512] = {};
    char directory[512]  = {};

    char client_name[128]  = {};

    int encryptionLevel;
    int encryptionMethod;

    const int  key_flags;
          int  last_key_flags_sent = 0;
          bool first_scancode = true;

    uint32_t     server_public_key_len;
    uint8_t      client_crypt_random[512];
    CryptContext encrypt, decrypt;

    enum ModState : uint8_t {
          MOD_RDP_NEGO_INITIATE
        , MOD_RDP_NEGO
        , MOD_RDP_BASIC_SETTINGS_EXCHANGE
        , MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER
        , MOD_RDP_CHANNEL_JOIN_CONFIRME
        , MOD_RDP_GET_LICENSE
        , MOD_RDP_CONNECTED
    };

    enum : uint8_t {
        EARLY,
        WAITING_SYNCHRONIZE,
        WAITING_CTL_COOPERATE,
        WAITING_GRANT_CONTROL_COOPERATE,
        WAITING_FONT_MAP,
        UP_AND_RUNNING
    } connection_finalization_state;

    ModState state;
    Pointer cursors[32];
    const bool console_session;
    const uint8_t front_bpp;
    const uint32_t performanceFlags;
    const ClientTimeZone client_time_zone;
    Random & gen;
    const RDPVerbose verbose;
    const BmpCache::Verbose cache_verbose;

    const bool enable_auth_channel;

    CHANNELS::ChannelNameId auth_channel;
    int  auth_channel_flags;
    int  auth_channel_chanid;

    CHANNELS::ChannelNameId checkout_channel;
    int  checkout_channel_flags = 0;
    int  checkout_channel_chanid = 0;

    AuthApi & authentifier;
    ReportMessageApi & report_message;

    std::string& close_box_extra_message_ref;

    RdpNego nego;
    Transport& trans;

    char clientAddr[512];

    const bool enable_fastpath;                    // choice of programmer
          bool enable_fastpath_client_input_event; // choice of programmer + capability of server
    const bool enable_fastpath_server_update;      // = choice of programmer
    const bool enable_glyph_cache;
    const bool enable_session_probe;
    const bool session_probe_enable_launch_mask;
    const bool enable_mem3blt;
    const bool enable_new_pointer;
    const bool enable_transparent_mode;
    const bool enable_persistent_disk_bitmap_cache;
    const bool enable_cache_waiting_list;
    const bool persist_bitmap_cache_on_disk;
    const bool enable_ninegrid_bitmap;
    const bool disable_clipboard_log_syslog;
    const bool disable_clipboard_log_wrm;
    const bool disable_file_system_log_syslog;
    const bool disable_file_system_log_wrm;
    const RdpCompression rdp_compression;

    const std::chrono::milliseconds   session_probe_launch_timeout;
    const std::chrono::milliseconds   session_probe_launch_fallback_timeout;
    const bool                        session_probe_start_launch_timeout_timer_only_after_logon;
    const SessionProbeOnLaunchFailure session_probe_on_launch_failure;
    const std::chrono::milliseconds   session_probe_keepalive_timeout;
    const SessionProbeOnKeepaliveTimeout
                                      session_probe_on_keepalive_timeout;
    const bool                        session_probe_end_disconnected_session;
    const std::chrono::milliseconds   session_probe_disconnected_application_limit;
    const std::chrono::milliseconds   session_probe_disconnected_session_limit;
    const std::chrono::milliseconds   session_probe_idle_session_limit;
    const bool                        session_probe_use_clipboard_based_launcher;
    const bool                        session_probe_enable_log;
    const bool                        session_probe_enable_log_rotation;

    const bool                        use_session_probe_to_launch_remote_program;

    const std::chrono::milliseconds   session_probe_clipboard_based_launcher_clipboard_initialization_delay;
    const std::chrono::milliseconds   session_probe_clipboard_based_launcher_long_delay;
    const std::chrono::milliseconds   session_probe_clipboard_based_launcher_short_delay;

    const bool                        session_probe_allow_multiple_handshake;

    const bool                        session_probe_enable_crash_dump;

    const uint32_t                    session_probe_handle_usage_limit;
    const uint32_t                    session_probe_memory_usage_limit;

    const bool                        bogus_ios_rdpdr_virtual_channel;

    const bool                        enable_rdpdr_data_analysis;

    const bool                        experimental_fix_input_event_sync;

    std::string session_probe_target_informations;

    SessionProbeVirtualChannel * session_probe_virtual_channel_p = nullptr;

    std::string session_probe_extra_system_processes;
    std::string session_probe_outbound_connection_monitoring_rules;
    std::string session_probe_process_monitoring_rules;

    size_t recv_bmp_update;

    rdp_mppc_unified_dec mppc_dec;

    std::string * error_message;

    const bool                 disconnect_on_logon_user_change;
    const std::chrono::seconds open_session_timeout;

    Timeout open_session_timeout_checker;

    std::string output_filename;

    std::string end_session_reason;
    std::string end_session_message;

    const bool            server_cert_store;
    const ServerCertCheck server_cert_check;

    std::unique_ptr<char[]> certif_path;

    bool enable_polygonsc;
    bool enable_polygoncb;
    bool enable_polyline;
    bool enable_ellipsesc;
    bool enable_ellipsecb;
    bool enable_multidstblt;
    bool enable_multiopaquerect;
    bool enable_multipatblt;
    bool enable_multiscrblt;

    const bool remote_program;
    const bool remote_program_enhanced;

    TransparentRecorder * transparent_recorder;
    Transport           * persistent_key_list_transport;

    //uint64_t total_data_received;

    const uint32_t password_printing_mode;

    bool deactivation_reactivation_in_progress = false;

    RedirectionInfo & redir_info;

    const bool bogus_sc_net_size;
    const bool bogus_refresh_rect;

    BogusLinuxCursor bogus_linux_cursor;

    std::string real_alternate_shell;
    std::string real_working_dir;

    std::deque<std::unique_ptr<AsynchronousTask>> asynchronous_tasks;
    wait_obj                                      asynchronous_task_event;

    Translation::language_t lang;

    Font const & font;

    const bool allow_using_multiple_monitors;

    bool already_upped_and_running = false;

    bool input_event_disabled     = false;
    bool graphics_update_disabled = false;

    static constexpr std::array<uint32_t, BmpCache::MAXIMUM_NUMBER_OF_CACHES>
    BmpCacheRev2_Cache_NumEntries()
    { return std::array<uint32_t, BmpCache::MAXIMUM_NUMBER_OF_CACHES>{{ 120, 120, 2553, 0, 0 }}; }

    class ToServerAsynchronousSender : public VirtualChannelDataSender
    {
        std::unique_ptr<VirtualChannelDataSender> to_server_synchronous_sender;

        std::deque<std::unique_ptr<AsynchronousTask>> & asynchronous_tasks;

        wait_obj & asynchronous_task_event;

        RDPVerbose verbose;

    public:
        ToServerAsynchronousSender(
            std::unique_ptr<VirtualChannelDataSender> &
                to_server_synchronous_sender,
            std::deque<std::unique_ptr<AsynchronousTask>> &
                asynchronous_tasks,
            wait_obj & asynchronous_task_event,
            RDPVerbose verbose)
        : to_server_synchronous_sender(
            std::move(to_server_synchronous_sender))
        , asynchronous_tasks(asynchronous_tasks)
        , asynchronous_task_event(asynchronous_task_event)
        , verbose(verbose)
        {}

        VirtualChannelDataSender& SynchronousSender() override {
            return *(to_server_synchronous_sender.get());
        }

        void operator()(uint32_t total_length, uint32_t flags,
            const uint8_t* chunk_data, uint32_t chunk_data_length)
                override {
            std::unique_ptr<AsynchronousTask> asynchronous_task =
                std::make_unique<RdpdrSendClientMessageTask>(
                    total_length, flags, chunk_data, chunk_data_length,
                    *(this->to_server_synchronous_sender.get()),
                    this->verbose);

            if (this->asynchronous_tasks.empty()) {
                this->asynchronous_task_event.full_reset();

                asynchronous_task->configure_wait_object(
                    this->asynchronous_task_event);
            }

            this->asynchronous_tasks.push_back(std::move(asynchronous_task));
        }
    };


    inline ClipboardVirtualChannel& get_clipboard_virtual_channel() {
        if (!this->clipboard_virtual_channel) {
            assert(!this->clipboard_to_client_sender &&
                !this->clipboard_to_server_sender);

            this->clipboard_to_client_sender =
                this->create_to_client_sender(channel_names::cliprdr);
            this->clipboard_to_server_sender =
                this->create_to_server_sender(channel_names::cliprdr);

            this->clipboard_virtual_channel =
                std::make_unique<ClipboardVirtualChannel>(
                    this->clipboard_to_client_sender.get(),
                    this->clipboard_to_server_sender.get(),
                    this->front,
                    this->get_clipboard_virtual_channel_params());
        }

        return *this->clipboard_virtual_channel;
    }

    inline DynamicChannelVirtualChannel& get_dynamic_channel_virtual_channel() {
        if (!this->dynamic_channel_virtual_channel) {
            assert(!this->dynamic_channel_to_client_sender &&
                !this->dynamic_channel_to_server_sender);

            this->dynamic_channel_to_client_sender =
                this->create_to_client_sender(channel_names::drdynvc);
            this->dynamic_channel_to_server_sender =
                this->create_to_server_sender(channel_names::drdynvc);

            this->dynamic_channel_virtual_channel =
                std::make_unique<DynamicChannelVirtualChannel>(
                    this->dynamic_channel_to_client_sender.get(),
                    this->dynamic_channel_to_server_sender.get(),
                    this->get_dynamic_channel_virtual_channel_params());
        }

        return *this->dynamic_channel_virtual_channel;
    }

    inline FileSystemVirtualChannel& get_file_system_virtual_channel() {
        if (!this->file_system_virtual_channel) {
            assert(!this->file_system_to_client_sender &&
                !this->file_system_to_server_sender);

            this->file_system_to_client_sender =
                (((this->client_general_caps.os_major != OSMAJORTYPE_IOS) ||
                  !this->bogus_ios_rdpdr_virtual_channel) ?
                 this->create_to_client_sender(channel_names::rdpdr) :
                 nullptr);
            this->file_system_to_server_sender =
                this->create_to_server_sender(channel_names::rdpdr);

            this->file_system_virtual_channel =
                std::make_unique<FileSystemVirtualChannel>(
                    this->file_system_to_client_sender.get(),
                    this->file_system_to_server_sender.get(),
                    this->file_system_drive_manager,
                    this->front,
                    this->get_file_system_virtual_channel_params());
        }

        return *this->file_system_virtual_channel;
    }

    inline SessionProbeVirtualChannel& get_session_probe_virtual_channel() {
        if (!this->session_probe_virtual_channel) {
            assert(!this->session_probe_to_server_sender);

            this->session_probe_to_server_sender =
                this->create_to_server_sender(channel_names::sespro);

            FileSystemVirtualChannel& file_system_virtual_channel =
                get_file_system_virtual_channel();

            this->session_probe_virtual_channel =
                std::make_unique<SessionProbeVirtualChannel>(
                    this->session_probe_to_server_sender.get(),
                    this->front,
                    *this,
                    *this,
                    file_system_virtual_channel,
                    this->gen,
                    this->get_session_probe_virtual_channel_params());
        }

        return *this->session_probe_virtual_channel;
    }

    inline RemoteProgramsVirtualChannel& get_remote_programs_virtual_channel() {
        if (!this->remote_programs_virtual_channel) {
            assert(!this->remote_programs_to_client_sender &&
                !this->remote_programs_to_server_sender);

            this->remote_programs_to_client_sender =
                this->create_to_client_sender(channel_names::rail);
            this->remote_programs_to_server_sender =
                this->create_to_server_sender(channel_names::rail);

            this->remote_programs_virtual_channel =
                std::make_unique<RemoteProgramsVirtualChannel>(
                    this->remote_programs_to_client_sender.get(),
                    this->remote_programs_to_server_sender.get(),
                    this->front,
                    this->vars,
                    this->get_remote_programs_virtual_channel_params());
        }

        return *this->remote_programs_virtual_channel;
    }

    // TODO duplicated code in front
    struct write_x224_dt_tpdu_fn
    {
        void operator()(StreamSize<7>, OutStream & x224_header, std::size_t sz) const {
            X224::DT_TPDU_Send(x224_header, sz);
        }
    };

    struct write_sec_send_fn
    {
        uint32_t flags;
        CryptContext & encrypt;
        int encryption_level;

        void operator()(StreamSize<256>, OutStream & sec_header, uint8_t * packet_data, std::size_t packet_size) const {
            SEC::Sec_Send sec(sec_header, packet_data, packet_size, this->flags, this->encrypt, this->encryption_level);
            (void)sec;
        }
    };

    class RDPServerNotifier : public ServerNotifier {
    private:
        FrontAPI & front;

        ReportMessageApi & report_message;

        const ServerNotification server_access_allowed_message;
        const ServerNotification server_cert_create_message;
        const ServerNotification server_cert_success_message;
        const ServerNotification server_cert_failure_message;
        const ServerNotification server_cert_error_message;

        const RDPVerbose verbose;

        bool is_syslog_notification_enabled(ServerNotification server_notification) {
            return ((server_notification & ServerNotification::syslog) == ServerNotification::syslog);
        }

    public:
        RDPServerNotifier(
                FrontAPI & front,
                ReportMessageApi & report_message,
                ServerNotification server_access_allowed_message,
                ServerNotification server_cert_create_message,
                ServerNotification server_cert_success_message,
                ServerNotification server_cert_failure_message,
                ServerNotification server_cert_error_message,
                RDPVerbose verbose
            )
        : front(front)
        , report_message(report_message)
        , server_access_allowed_message(server_access_allowed_message)
        , server_cert_create_message(server_cert_create_message)
        , server_cert_success_message(server_cert_success_message)
        , server_cert_failure_message(server_cert_failure_message)
        , server_cert_error_message(server_cert_error_message)
        , verbose(verbose)
        {}

        void server_access_allowed() override {
            if (is_syslog_notification_enabled(this->server_access_allowed_message)) {
                this->log5_server_cert(
                    "CERTIFICATE_CHECK_SUCCESS",
                    "Connexion to server allowed"
                );
            }
        }

        void server_cert_create() override {
            if (is_syslog_notification_enabled(this->server_cert_create_message)) {
                this->log5_server_cert(
                    "SERVER_CERTIFICATE_NEW",
                    "New X.509 certificate created"
                );
            }
        }

        void server_cert_success() override {
            if (is_syslog_notification_enabled(this->server_cert_success_message)) {
                this->log5_server_cert(
                    "SERVER_CERTIFICATE_MATCH_SUCCESS",
                    "X.509 server certificate match"
                );
            }
        }

        void server_cert_failure() override {
            if (is_syslog_notification_enabled(this->server_cert_failure_message)) {
                this->log5_server_cert(
                    "SERVER_CERTIFICATE_MATCH_FAILURE",
                    "X.509 server certificate match failure"
                );
            }
        }

        void server_cert_error(const char * str_error) override {
            if (is_syslog_notification_enabled(this->server_cert_error_message)) {
                this->log5_server_cert(
                    "SERVER_CERTIFICATE_ERROR",
                    "X.509 server certificate internal error: " + std::string(str_error)
                );
            }
        }

    private:
        KeyQvalueFormatter message;
        void log5_server_cert(charp_or_string type, charp_or_string description)
        {
            this->message.assign(type.data, {{"description", description.data}});

            this->report_message.log5(this->message.str());

            if (bool(this->verbose & RDPVerbose::basic_trace)) {
                LOG(LOG_INFO, "%s", this->message.str());
            }

            {
                std::string message(type.data.data(), type.data.size());
                message += "=";
                message.append(description.data.data(), description.data.size());

                this->front.session_update(message);
            }
        }
    } server_notifier;

    std::unique_ptr<SessionProbeLauncher> session_probe_launcher;

    GCC::UserData::CSMonitor cs_monitor;

    uint16_t    client_execute_flags = 0;
    std::string client_execute_exe_or_file;
    std::string client_execute_working_dir;
    std::string client_execute_arguments;

    bool use_client_provided_remoteapp;

    bool should_ignore_first_client_execute = false;

    uint16_t    real_client_execute_flags = 0;
    std::string real_client_execute_exe_or_file;
    std::string real_client_execute_working_dir;
    std::string real_client_execute_arguments;

    time_t beginning;
    bool   session_disconnection_logged = false;

    rdpdr::RdpDrStatus rdpdrLogStatus;
    RDPECLIP::CliprdrLogState cliprdrLogStatus;

    class AsynchronousTaskEventHandler : public EventHandler::CB {
        mod_rdp& mod_;

    public:
        AsynchronousTaskEventHandler(mod_rdp& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_asynchronous_task_event(now, event, drawable);
        }
    } asynchronous_task_event_handler;

    class SessionProbeLauncherEventHandler : public EventHandler::CB {
        mod_rdp& mod_;

    public:
        SessionProbeLauncherEventHandler(mod_rdp& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_session_probe_launcher_event(now, event, drawable);
        }
    } session_probe_launcher_event_handler;

    class SessionProbeVirtualChannelEventHandler : public EventHandler::CB {
        mod_rdp& mod_;

    public:
        SessionProbeVirtualChannelEventHandler(mod_rdp& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_session_probe_virtual_channel_event(now, event, drawable);
        }
    } session_probe_virtual_channel_event_handler;

    class RemoteProgramSessionManagerEventHandler : public EventHandler::CB {
        mod_rdp& mod_;

    public:
        RemoteProgramSessionManagerEventHandler(mod_rdp& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_remote_program_session_manager_event(now, event, drawable);
        }
    } remote_program_session_manager_event_handler;

    class FileSystemVirtualChannelEventHandler : public EventHandler::CB {
        mod_rdp& mod_;

    public:
        FileSystemVirtualChannelEventHandler(mod_rdp& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_file_system_virtual_channel_event(now, event, drawable);
        }
    } file_system_virtual_channel_event_handler;

    bool clean_up_32_bpp_cursor;
    bool large_pointer_support;

    StaticOutStream<65536> multifragment_update_data;

    LargePointerCaps        client_large_pointer_caps;
    MultiFragmentUpdateCaps client_multi_fragment_update_caps;

    GeneralCaps const        client_general_caps;
    BitmapCaps const         client_bitmap_caps;
    OrderCaps const          client_order_caps;
    BmpCacheCaps const       client_bmp_cache_caps;
    BmpCache2Caps const      client_bmp_cache_2_caps;
    OffScreenCacheCaps const client_off_screen_cache_caps;
    GlyphCacheCaps const     client_glyph_cache_caps;
    RailCaps const           client_rail_caps;
    WindowListCaps const     client_window_list_caps;

    bool client_use_bmp_cache_2 = false;

    const bool perform_automatic_reconnection;
    std::array<uint8_t, 28>& server_auto_reconnect_packet_ref;

    bool is_server_auto_reconnec_packet_received = false;

    uint8_t client_random[SEC_RANDOM_SIZE] = { 0 };

    std::string load_balance_info;

    bool server_redirection_packet_received = false;

    ModRdpVariables vars;

    InfoPacketFlags info_packet_extra_flags;

    long int total_main_amount_data_rcv_from_client;
    long int total_cliprdr_amount_data_rcv_from_client;
    long int total_rail_amount_data_rcv_from_client;
    long int total_rdpdr_amount_data_rcv_from_client;
    long int total_drdynvc_amount_data_rcv_from_client;

    long int total_main_amount_data_rcv_from_server;
    long int total_cliprdr_amount_data_rcv_from_server;
    long int total_rail_amount_data_rcv_from_server;
    long int total_rdpdr_amount_data_rcv_from_server;
    long int total_drdynvc_amount_data_rcv_from_server;


public:
    using Verbose = RDPVerbose;

    GCC::UserData::SCCore sc_core;
    GCC::UserData::SCSecurity sc_sec1;
    GCC::UserData::CSSecurity cs_security;

    mod_rdp( Transport & trans
           , FrontAPI & front
           , const ClientInfo & info
           , RedirectionInfo & redir_info
           , Random & gen
           , TimeObj & timeobj
           , const ModRDPParams & mod_rdp_params
           , AuthApi & authentifier
           , ReportMessageApi & report_message
           , ModRdpVariables vars
           )
        : front_width(info.width - (info.width % 4))
        , front_height(info.height)
        , front(front)
        , authorization_channels(
            mod_rdp_params.allow_channels ? *mod_rdp_params.allow_channels : std::string{},
            mod_rdp_params.deny_channels ? *mod_rdp_params.deny_channels : std::string{}
          )
        , use_rdp5(1)
        , cbAutoReconnectCookie(info.cbAutoReconnectCookie)
        , keylayout(info.keylayout)
        , orders( mod_rdp_params.target_host, mod_rdp_params.enable_persistent_disk_bitmap_cache
                , mod_rdp_params.persist_bitmap_cache_on_disk, mod_rdp_params.verbose
                , report_error_from_reporter(report_message))
        , share_id(0)
        , userid(0)
        , encryptionLevel(0)
        , key_flags(mod_rdp_params.key_flags)
        , last_key_flags_sent(key_flags)
        , server_public_key_len(0)
        , connection_finalization_state(EARLY)
        , state(MOD_RDP_NEGO_INITIATE)
        , console_session(info.console_session)
        , front_bpp(info.bpp)
        , performanceFlags(info.rdp5_performanceflags &
                           (~(mod_rdp_params.adjust_performance_flags_for_recording ?
                              static_cast<uint32_t>(PERF_ENABLE_FONT_SMOOTHING) : 0)))
        , client_time_zone(info.client_time_zone)
        , gen(gen)
        , verbose(/*RDPVerbose::export_metrics*/mod_rdp_params.verbose)
        , cache_verbose(mod_rdp_params.cache_verbose)
        , enable_auth_channel(mod_rdp_params.alternate_shell[0] && !mod_rdp_params.ignore_auth_channel)
        , auth_channel_flags(0)
        , auth_channel_chanid(0)
        , authentifier(authentifier)
        , report_message(report_message)
        , close_box_extra_message_ref(mod_rdp_params.close_box_extra_message_ref)
        , nego( mod_rdp_params.enable_tls, mod_rdp_params.target_user
              , mod_rdp_params.enable_nla, info.console_session
			  , mod_rdp_params.target_host
              , mod_rdp_params.enable_krb, gen, timeobj
              , this->close_box_extra_message_ref, mod_rdp_params.lang
              , static_cast<RdpNego::Verbose>(mod_rdp_params.verbose))
        , trans(trans)
        , enable_fastpath(mod_rdp_params.enable_fastpath)
        , enable_fastpath_client_input_event(false)
        , enable_fastpath_server_update(mod_rdp_params.enable_fastpath)
        , enable_glyph_cache(mod_rdp_params.enable_glyph_cache)
        , enable_session_probe(mod_rdp_params.enable_session_probe)
        , session_probe_enable_launch_mask(mod_rdp_params.session_probe_enable_launch_mask)
        , enable_mem3blt(mod_rdp_params.enable_mem3blt)
        , enable_new_pointer(mod_rdp_params.enable_new_pointer)
        , enable_transparent_mode(mod_rdp_params.enable_transparent_mode)
        , enable_persistent_disk_bitmap_cache(mod_rdp_params.enable_persistent_disk_bitmap_cache)
        , enable_cache_waiting_list(mod_rdp_params.enable_cache_waiting_list)
        , persist_bitmap_cache_on_disk(mod_rdp_params.persist_bitmap_cache_on_disk)
        , enable_ninegrid_bitmap(mod_rdp_params.enable_ninegrid_bitmap)
        , disable_clipboard_log_syslog(mod_rdp_params.disable_clipboard_log_syslog)
        , disable_clipboard_log_wrm(mod_rdp_params.disable_clipboard_log_wrm)
        , disable_file_system_log_syslog(mod_rdp_params.disable_file_system_log_syslog)
        , disable_file_system_log_wrm(mod_rdp_params.disable_file_system_log_wrm)
        , rdp_compression(mod_rdp_params.rdp_compression)
        , session_probe_launch_timeout(mod_rdp_params.session_probe_launch_timeout)
        , session_probe_launch_fallback_timeout(mod_rdp_params.session_probe_launch_fallback_timeout)
        , session_probe_start_launch_timeout_timer_only_after_logon(mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon)
        , session_probe_on_launch_failure(mod_rdp_params.session_probe_on_launch_failure)
        , session_probe_keepalive_timeout(mod_rdp_params.session_probe_keepalive_timeout)
        , session_probe_on_keepalive_timeout(mod_rdp_params.session_probe_on_keepalive_timeout)
        , session_probe_end_disconnected_session(mod_rdp_params.session_probe_end_disconnected_session)
        , session_probe_disconnected_application_limit(mod_rdp_params.session_probe_disconnected_application_limit)
        , session_probe_disconnected_session_limit(mod_rdp_params.session_probe_disconnected_session_limit)
        , session_probe_idle_session_limit(mod_rdp_params.session_probe_idle_session_limit)
        , session_probe_use_clipboard_based_launcher(mod_rdp_params.session_probe_use_clipboard_based_launcher &&
                                                     (!mod_rdp_params.target_application || !(*mod_rdp_params.target_application)) &&
                                                     (!mod_rdp_params.use_client_provided_alternate_shell ||
                                                      !info.alternate_shell[0] ||
                                                      info.remote_program))
        , session_probe_enable_log(mod_rdp_params.session_probe_enable_log)
        , session_probe_enable_log_rotation(mod_rdp_params.session_probe_enable_log_rotation)
        , use_session_probe_to_launch_remote_program(mod_rdp_params.use_session_probe_to_launch_remote_program)
        , session_probe_clipboard_based_launcher_clipboard_initialization_delay(mod_rdp_params.session_probe_clipboard_based_launcher_clipboard_initialization_delay)
        , session_probe_clipboard_based_launcher_long_delay(mod_rdp_params.session_probe_clipboard_based_launcher_long_delay)
        , session_probe_clipboard_based_launcher_short_delay(mod_rdp_params.session_probe_clipboard_based_launcher_short_delay)
        , session_probe_allow_multiple_handshake(mod_rdp_params.session_probe_allow_multiple_handshake)
        , session_probe_enable_crash_dump(mod_rdp_params.session_probe_enable_crash_dump)
        , session_probe_handle_usage_limit(mod_rdp_params.session_probe_handle_usage_limit)
        , session_probe_memory_usage_limit(mod_rdp_params.session_probe_memory_usage_limit)
        , bogus_ios_rdpdr_virtual_channel(mod_rdp_params.bogus_ios_rdpdr_virtual_channel)
        , enable_rdpdr_data_analysis(mod_rdp_params.enable_rdpdr_data_analysis)
        , experimental_fix_input_event_sync(mod_rdp_params.experimental_fix_input_event_sync)
        , session_probe_extra_system_processes(mod_rdp_params.session_probe_extra_system_processes)
        , session_probe_outbound_connection_monitoring_rules(mod_rdp_params.session_probe_outbound_connection_monitoring_rules)
        , session_probe_process_monitoring_rules(mod_rdp_params.session_probe_process_monitoring_rules)
        , recv_bmp_update(0)
        , error_message(mod_rdp_params.error_message)
        , disconnect_on_logon_user_change(mod_rdp_params.disconnect_on_logon_user_change)
        , open_session_timeout(mod_rdp_params.open_session_timeout)
        , open_session_timeout_checker(0)
        , output_filename(mod_rdp_params.output_filename)
        , server_cert_store(mod_rdp_params.server_cert_store)
        , server_cert_check(mod_rdp_params.server_cert_check)
        , certif_path([](const char * device_id){
            size_t lg_certif_path = strlen(app_path(AppPath::Certif));
            size_t lg_dev_id = strlen(device_id);
            char * buffer(new(std::nothrow) char[lg_certif_path + lg_dev_id + 2]);
            if (!buffer){
                throw Error(ERR_PATH_TOO_LONG);
            }
            memcpy(buffer, app_path(AppPath::Certif), lg_certif_path);
            buffer[lg_certif_path] =  '/';
            memcpy(buffer+lg_certif_path+1, device_id, lg_dev_id+1);
            return buffer;
        }(mod_rdp_params.device_id))

        , enable_polygonsc(false)
        , enable_polygoncb(false)
        , enable_polyline(false)
        , enable_ellipsesc(false)
        , enable_ellipsecb(false)
        , enable_multidstblt(false)
        , enable_multiopaquerect(false)
        , enable_multipatblt(false)
        , enable_multiscrblt(false)
        , remote_program(mod_rdp_params.remote_program)
        , remote_program_enhanced(mod_rdp_params.remote_program_enhanced)
        , transparent_recorder(nullptr)
        , persistent_key_list_transport(mod_rdp_params.persistent_key_list_transport)
        //, total_data_received(0)
        , password_printing_mode(mod_rdp_params.password_printing_mode)
        , redir_info(redir_info)
        , bogus_sc_net_size(mod_rdp_params.bogus_sc_net_size)
        , bogus_refresh_rect(mod_rdp_params.bogus_refresh_rect)
        , bogus_linux_cursor(mod_rdp_params.bogus_linux_cursor)
        , lang(mod_rdp_params.lang)
        , font(mod_rdp_params.font)
        , allow_using_multiple_monitors(mod_rdp_params.allow_using_multiple_monitors)
        , server_notifier(front,
                          report_message,
                          mod_rdp_params.server_access_allowed_message,
                          mod_rdp_params.server_cert_create_message,
                          mod_rdp_params.server_cert_success_message,
                          mod_rdp_params.server_cert_failure_message,
                          mod_rdp_params.server_cert_error_message,
                          mod_rdp_params.verbose
                         )
        , cs_monitor(info.cs_monitor)
        , use_client_provided_remoteapp(mod_rdp_params.use_client_provided_remoteapp)
        , should_ignore_first_client_execute(mod_rdp_params.should_ignore_first_client_execute)
        , asynchronous_task_event_handler(*this)
        , session_probe_launcher_event_handler(*this)
        , session_probe_virtual_channel_event_handler(*this)
        , remote_program_session_manager_event_handler(*this)
        , file_system_virtual_channel_event_handler(*this)
        , clean_up_32_bpp_cursor(mod_rdp_params.clean_up_32_bpp_cursor)
        , large_pointer_support(mod_rdp_params.large_pointer_support)
        , client_large_pointer_caps(info.large_pointer_caps)
        , client_multi_fragment_update_caps(info.multi_fragment_update_caps)
        , client_general_caps(info.general_caps)
        , client_bitmap_caps(info.bitmap_caps)
        , client_order_caps(info.order_caps)
        , client_bmp_cache_caps(info.bmp_cache_caps)
        , client_bmp_cache_2_caps(info.bmp_cache_2_caps)
        , client_off_screen_cache_caps(info.off_screen_cache_caps)
        , client_glyph_cache_caps(info.glyph_cache_caps)
        , client_rail_caps(info.rail_caps)
        , client_window_list_caps(info.window_list_caps)
        , client_use_bmp_cache_2(info.use_bmp_cache_2)
        , perform_automatic_reconnection(mod_rdp_params.perform_automatic_reconnection)
        , server_auto_reconnect_packet_ref(mod_rdp_params.server_auto_reconnect_packet_ref)
        , load_balance_info(mod_rdp_params.load_balance_info)
        , vars(vars)
        , info_packet_extra_flags(info.has_sound_code ? INFO_REMOTECONSOLEAUDIO : InfoPacketFlags{})
        , total_main_amount_data_rcv_from_client(0)
        , total_cliprdr_amount_data_rcv_from_client(0)
        , total_rail_amount_data_rcv_from_client(0)
        , total_rdpdr_amount_data_rcv_from_client(0)
        , total_drdynvc_amount_data_rcv_from_client(0)
        , total_main_amount_data_rcv_from_server(0)
        , total_cliprdr_amount_data_rcv_from_server(0)
        , total_rail_amount_data_rcv_from_server(0)
        , total_rdpdr_amount_data_rcv_from_server(0)
        , total_drdynvc_amount_data_rcv_from_server(0)
    {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            if (!enable_transparent_mode) {
                LOG(LOG_INFO, "Creation of new mod 'RDP'");
            }
            else {
                LOG(LOG_INFO, "Creation of new mod 'RDP Transparent'");

                if (this->output_filename.empty()) {
                    LOG(LOG_INFO, "Use transparent capabilities.");
                }
                else {
                    LOG(LOG_INFO, "Use proxy default capabilities.");
                }
            }

            mod_rdp_params.log();
        }

        // Clear client screen
        this->invoke_asynchronous_graphic_task(AsynchronousGraphicTask::clear_screen);

        this->beginning = timeobj.get_time().tv_sec;

        if (this->cbAutoReconnectCookie) {
            ::memcpy(this->autoReconnectCookie, info.autoReconnectCookie, sizeof(this->autoReconnectCookie));
        }

        if (this->bogus_linux_cursor == BogusLinuxCursor::smart) {
            this->bogus_linux_cursor =
                ((this->client_general_caps.os_major == OSMAJORTYPE_UNIX) ?
                 BogusLinuxCursor::enable : BogusLinuxCursor::disable);
        }

        if (this->enable_session_probe) {
            this->file_system_drive_manager.EnableSessionProbeDrive(
                mod_rdp_params.proxy_managed_drive_prefix, this->verbose);
        }

        if (mod_rdp_params.proxy_managed_drives && (*mod_rdp_params.proxy_managed_drives)) {
            this->configure_proxy_managed_drives(mod_rdp_params.proxy_managed_drives,
                                                 mod_rdp_params.proxy_managed_drive_prefix);
        }

        if (mod_rdp_params.transparent_recorder_transport) {
            this->transparent_recorder = new TransparentRecorder(mod_rdp_params.transparent_recorder_transport);
        }

        this->configure_extra_orders(mod_rdp_params.extra_orders);

        switch (mod_rdp_params.auth_channel) {
            case CHANNELS::ChannelNameId():
            case CHANNELS::ChannelNameId("*"):
                this->auth_channel = CHANNELS::ChannelNameId("wablnch");
                break;
            default:
                this->auth_channel = mod_rdp_params.auth_channel;
        }

        this->checkout_channel = mod_rdp_params.checkout_channel;

        memset(this->clientAddr, 0, sizeof(this->clientAddr));
        strncpy(this->clientAddr, mod_rdp_params.client_address, sizeof(this->clientAddr) - 1);
        this->lic_layer_license_size = 0;
        memset(this->lic_layer_license_key, 0, 16);
        memset(this->lic_layer_license_sign_key, 0, 16);
        // TODO CGR: license loading should be done before creating protocol layers
        struct stat st;
        char path[256];
        snprintf(path, sizeof(path), "%s/license.%s", app_path(AppPath::License), info.hostname);
        int fd = open(path, O_RDONLY);
        if (fd != -1){
            if (fstat(fd, &st) != 0){
                this->lic_layer_license_data.reset(new uint8_t[this->lic_layer_license_size]);
                if (this->lic_layer_license_data){
                    size_t lic_size = read(fd, this->lic_layer_license_data.get(), this->lic_layer_license_size);
                    if (lic_size != this->lic_layer_license_size){
                        LOG(LOG_ERR, "license file truncated : expected %zu, got %zu", this->lic_layer_license_size, lic_size);
                    }
                }
            }
            close(fd);
        }

        // from rdp_sec
        memset(this->client_crypt_random, 0, sizeof(this->client_crypt_random));

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        this->decrypt.encryptionMethod = 2; /* 128 bits */
        this->encrypt.encryptionMethod = 2; /* 128 bits */

        if (::strlen(info.hostname) >= sizeof(this->hostname)) {
            LOG(LOG_WARNING, "mod_rdp: hostname too long! %zu >= %zu", ::strlen(info.hostname), sizeof(this->hostname));
        }
        if (mod_rdp_params.hide_client_name) {
            ::gethostname(this->hostname, sizeof(this->hostname));
            this->hostname[sizeof(this->hostname) - 1] = 0;
            char* separator = strchr(this->hostname, '.');
            if (separator) *separator = 0;
        }
        else{
            ::strncpy(this->hostname, info.hostname, sizeof(this->hostname) - 1);
            this->hostname[sizeof(this->hostname) - 1] = 0;
        }

        const char * domain_pos   = nullptr;
        size_t       domain_len   = 0;
        const char * username_pos = nullptr;
        size_t       username_len = 0;
        const char * separator = strchr(mod_rdp_params.target_user, '\\');
        if (separator)
        {
            domain_pos   = mod_rdp_params.target_user;
            domain_len   = separator - mod_rdp_params.target_user;
            username_pos = ++separator;
            username_len = strlen(username_pos);
        }
        else
        {
            separator = strchr(mod_rdp_params.target_user, '@');
            if (separator)
            {
                domain_pos   = separator + 1;
                domain_len   = strlen(domain_pos);
                username_pos = mod_rdp_params.target_user;
                username_len = separator - mod_rdp_params.target_user;
                LOG(LOG_INFO, "mod_rdp: username_len=%zu", username_len);
            }
            else
            {
                username_pos = mod_rdp_params.target_user;
                username_len = strlen(username_pos);
            }
        }

        if (username_len >= sizeof(this->username)) {
            LOG(LOG_WARNING, "mod_rdp: username too long! %zu >= %zu", username_len, sizeof(this->username));
        }
        size_t count = std::min(sizeof(this->username) - 1, username_len);
        if (count > 0) strncpy(this->username, username_pos, count);
        this->username[count] = 0;

        if (domain_len >= sizeof(this->domain)) {
            LOG(LOG_WARNING, "mod_rdp: domain too long! %zu >= %zu", domain_len, sizeof(this->domain));
        }
        count = std::min(sizeof(this->domain) - 1, domain_len);
        if (count > 0) strncpy(this->domain, domain_pos, count);
        this->domain[count] = 0;

        LOG(LOG_INFO, "Remote RDP Server domain=\"%s\" login=\"%s\" host=\"%s\"",
            this->domain, this->username, this->hostname);

        // Password is a multi-sz!
        // A multi-sz contains a sequence of null-terminated strings,
        //  terminated by an empty string (\0) so that the last two
        //  characters are both null terminators.
        SOHSeparatedStringsToMultiSZ(this->password, sizeof(this->password), mod_rdp_params.target_password);

        snprintf(this->client_name, sizeof(this->client_name), "%s", info.hostname);

        std::string shell_arguments;
        if (mod_rdp_params.target_application && (*mod_rdp_params.target_application)) {
            shell_arguments = mod_rdp_params.shell_arguments;

            {
                const char * appid_marker = "${APPID}";
                size_t pos = shell_arguments.find(appid_marker, 0);
                if (pos != std::string::npos) {
                    shell_arguments.replace(pos, strlen(appid_marker), mod_rdp_params.target_application);
                }
            }

            if (mod_rdp_params.target_application_account && *mod_rdp_params.target_application_account) {
                const char * user_marker = "${USER}";
                size_t pos = shell_arguments.find(user_marker, 0);
                if (pos != std::string::npos) {
                    shell_arguments.replace(pos, strlen(user_marker), mod_rdp_params.target_application_account);
                }
            }

            if (mod_rdp_params.target_application_password && *mod_rdp_params.target_application_password) {
                const char * password_marker = "${PASSWORD}";
                size_t pos = shell_arguments.find(password_marker, 0);
                if (pos != std::string::npos) {
                    shell_arguments.replace(pos, strlen(password_marker), mod_rdp_params.target_application_password);
                }
            }
        }

        char session_probe_window_title[32] = { 0 };

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "enable_session_probe=%s",
                (this->enable_session_probe ? "yes" : "no"));
        }

        std::string session_probe_arguments = mod_rdp_params.session_probe_arguments;

        if (this->enable_session_probe) {
            auto replace_tag = [](std::string & str, const char * tag,
                                  const char * replacement_text) {
                const size_t replacement_text_len = ::strlen(replacement_text);
                const size_t tag_len              = ::strlen(tag);

                size_t pos = 0;
                while ((pos = str.find(tag, pos)) != std::string::npos) {
                    str.replace(pos, tag_len, replacement_text);
                    pos += replacement_text_len;
                }
            };

            // Executable file name of SP.
            char exe_var_str[16];
            if (mod_rdp_params.session_probe_customize_executable_name) {
                ::snprintf(exe_var_str, sizeof(exe_var_str), "-%d", ::getpid());
            }
            else {
                ::memset(exe_var_str, 0, sizeof(exe_var_str));
            }
            replace_tag(session_probe_arguments, "${EXE_VAR}",
                exe_var_str);

            // Target informations
            this->session_probe_target_informations  = mod_rdp_params.target_application;
            this->session_probe_target_informations += ":";
            this->session_probe_target_informations += mod_rdp_params.primary_user_id;

            if (this->remote_program) {
                char proxy_managed_connection_cookie[9];
                get_proxy_managed_connection_cookie(
                    this->session_probe_target_informations.c_str(),
                    this->session_probe_target_informations.length(),
                    proxy_managed_connection_cookie);
                std::string param = "/#";
                param += proxy_managed_connection_cookie;
                param += " ";
                replace_tag(session_probe_arguments,
                    "/${COOKIE_VAR} ", param.c_str());

                replace_tag(session_probe_arguments,
                    "${CBSPL_VAR} ", "");

                uint32_t r = this->gen.rand32();

                snprintf(session_probe_window_title,
                    sizeof(session_probe_window_title),
                    "%X%X%X%X",
                    ((r & 0xFF000000) >> 24),
                    ((r & 0x00FF0000) >> 16),
                    ((r & 0x0000FF00) >> 8),
                      r & 0x000000FF
                    );

                param  = "TITLE ";
                param += session_probe_window_title;
                param += "&";

                replace_tag(session_probe_arguments,
                    "${TITLE_VAR} ", param.c_str());
            }   // if (this->remote_program)
            else {
                if (mod_rdp_params.session_probe_use_clipboard_based_launcher &&
                    (mod_rdp_params.target_application && (*mod_rdp_params.target_application))) {
                    assert(!this->session_probe_use_clipboard_based_launcher);

                    LOG(LOG_WARNING,
                        "mod_rdp: "
                            "Clipboard based Session Probe launcher is not compatible with application. "
                            "Falled back to using AlternateShell based launcher.");
                }

                replace_tag(session_probe_arguments,
                    "${TITLE_VAR} ", "");

                if (this->session_probe_use_clipboard_based_launcher) {
                    replace_tag(session_probe_arguments,
                        "/${COOKIE_VAR} ", "");

                    replace_tag(session_probe_arguments,
                        "${CBSPL_VAR} ", "CD %TMP%&");
                }
                else {
                    char proxy_managed_connection_cookie[9];
                    get_proxy_managed_connection_cookie(
                        this->session_probe_target_informations.c_str(),
                        this->session_probe_target_informations.length(),
                        proxy_managed_connection_cookie);
                    std::string param = "/#";
                    param += proxy_managed_connection_cookie;
                    param += " ";
                    replace_tag(session_probe_arguments,
                        "/${COOKIE_VAR} ", param.c_str());

                    replace_tag(session_probe_arguments,
                        "${CBSPL_VAR} ", "");
                }
            }   // if (!this->remote_program)
        }

        if (mod_rdp_params.target_application && (*mod_rdp_params.target_application)) {
            if (this->remote_program) {
                if (this->enable_session_probe) {
                    if (this->use_session_probe_to_launch_remote_program) {
                        std::string alternate_shell(mod_rdp_params.alternate_shell);

                        if (!shell_arguments.empty()) {
                            alternate_shell += " ";
                            alternate_shell += shell_arguments;
                        }

                        this->real_alternate_shell = std::move(alternate_shell);
                        this->real_working_dir     = mod_rdp_params.shell_working_dir;
                    }
                    else {
                        this->real_alternate_shell = "[None]";

                        this->real_client_execute_flags       = 0;
                        this->real_client_execute_exe_or_file = mod_rdp_params.alternate_shell;
                        this->real_client_execute_arguments   = shell_arguments.c_str();
                        this->real_client_execute_working_dir = mod_rdp_params.shell_working_dir;
                    }

                    this->client_execute_exe_or_file = mod_rdp_params.session_probe_exe_or_file;
                    this->client_execute_arguments   = session_probe_arguments;
                    this->client_execute_working_dir = "%TMP%";
                    this->client_execute_flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;

                    this->session_probe_launcher =
                        std::make_unique<SessionProbeAlternateShellBasedLauncher>(
                            this->verbose);
                }
                else {
                    this->client_execute_exe_or_file = mod_rdp_params.alternate_shell;
                    this->client_execute_arguments   = std::move(shell_arguments);
                    this->client_execute_working_dir = mod_rdp_params.shell_working_dir;
                    this->client_execute_flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
                }
            }
            else {
                if (this->enable_session_probe) {
                    std::string alternate_shell(mod_rdp_params.alternate_shell);

                    if (!shell_arguments.empty()) {
                        alternate_shell += " ";
                        alternate_shell += shell_arguments;
                    }

                    this->real_alternate_shell = std::move(alternate_shell);
                    this->real_working_dir     = mod_rdp_params.shell_working_dir;

                    alternate_shell = mod_rdp_params.session_probe_exe_or_file;

                    if (!::strncmp(alternate_shell.c_str(), "||", 2))
                        alternate_shell.erase(0, 2);

                    alternate_shell += " ";
                    alternate_shell += session_probe_arguments;

                    strncpy(this->program, alternate_shell.c_str(), sizeof(this->program) - 1);
                    this->program[sizeof(this->program) - 1] = 0;
                    //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                    const char * session_probe_working_dir = "%TMP%";
                    strncpy(this->directory, session_probe_working_dir, sizeof(this->directory) - 1);
                    this->directory[sizeof(this->directory) - 1] = 0;

                    this->session_probe_launcher =
                        std::make_unique<SessionProbeAlternateShellBasedLauncher>(
                            this->verbose);
                }
                else {
                    std::string alternate_shell(mod_rdp_params.alternate_shell);

                    if (!shell_arguments.empty()) {
                        alternate_shell += " ";
                        alternate_shell += shell_arguments;
                    }

                    strncpy(this->program, alternate_shell.c_str(), sizeof(this->program) - 1);
                    this->program[sizeof(this->program) - 1] = 0;
                    strncpy(this->directory, mod_rdp_params.shell_working_dir, sizeof(this->directory) - 1);
                    this->directory[sizeof(this->directory) - 1] = 0;
                }
            }
        }
        else {
            if (this->remote_program) {
                if (mod_rdp_params.use_client_provided_remoteapp &&
                    mod_rdp_params.client_execute_exe_or_file &&
                    *mod_rdp_params.client_execute_exe_or_file) {
                    if (this->enable_session_probe) {
                        this->real_alternate_shell = "[None]";

                        this->real_client_execute_flags       = mod_rdp_params.client_execute_flags;
                        this->real_client_execute_exe_or_file = mod_rdp_params.client_execute_exe_or_file;
                        this->real_client_execute_arguments   = mod_rdp_params.client_execute_arguments;
                        this->real_client_execute_working_dir = mod_rdp_params.client_execute_working_dir;

                        this->client_execute_exe_or_file = mod_rdp_params.session_probe_exe_or_file;
                        this->client_execute_arguments   = session_probe_arguments;
                        this->client_execute_working_dir = "%TMP%";
                        this->client_execute_flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;

                        this->session_probe_launcher =
                            std::make_unique<SessionProbeAlternateShellBasedLauncher>(
                                this->verbose);
                    }
                    else {
                        this->client_execute_flags       = mod_rdp_params.client_execute_flags;
                        this->client_execute_exe_or_file = mod_rdp_params.client_execute_exe_or_file;
                        this->client_execute_arguments   = mod_rdp_params.client_execute_arguments;
                        this->client_execute_working_dir = mod_rdp_params.client_execute_working_dir;
                    }
                }
            }
            else {
                if (mod_rdp_params.use_client_provided_alternate_shell &&
                        info.alternate_shell[0] &&
                        !info.remote_program) {
                    if (this->enable_session_probe) {
                        this->real_alternate_shell = info.alternate_shell;
                        this->real_working_dir     = info.working_dir;

                        std::string alternate_shell(mod_rdp_params.session_probe_exe_or_file);

                        if (!::strncmp(alternate_shell.c_str(), "||", 2))
                            alternate_shell.erase(0, 2);

                        alternate_shell += " ";
                        alternate_shell += session_probe_arguments;

                        strncpy(this->program, alternate_shell.c_str(), sizeof(this->program) - 1);
                        this->program[sizeof(this->program) - 1] = 0;
                        //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                        const char * session_probe_working_dir = "%TMP%";
                        strncpy(this->directory, session_probe_working_dir, sizeof(this->directory) - 1);
                        this->directory[sizeof(this->directory) - 1] = 0;

                        this->session_probe_launcher =
                            std::make_unique<SessionProbeAlternateShellBasedLauncher>(
                                this->verbose);
                    }
                    else {
                        strncpy(this->program, info.alternate_shell, sizeof(this->program) - 1);
                        this->program[sizeof(this->program) - 1] = 0;
                        //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                        strncpy(this->directory, info.working_dir, sizeof(this->directory) - 1);
                        this->directory[sizeof(this->directory) - 1] = 0;
                    }
                }
                else {
                    if (this->enable_session_probe) {
                        std::string alternate_shell(mod_rdp_params.session_probe_exe_or_file);

                        if (!::strncmp(alternate_shell.c_str(), "||", 2))
                            alternate_shell.erase(0, 2);

                        alternate_shell += " ";
                        alternate_shell += session_probe_arguments;

                        if (this->session_probe_use_clipboard_based_launcher) {
                            this->session_probe_launcher =
                                std::make_unique<SessionProbeClipboardBasedLauncher>(
                                    *this, alternate_shell.c_str(),
                                    this->session_probe_clipboard_based_launcher_clipboard_initialization_delay,
                                    this->session_probe_clipboard_based_launcher_long_delay,
                                    this->session_probe_clipboard_based_launcher_short_delay,
                                    this->verbose);
                        }
                        else {
                            strncpy(this->program, alternate_shell.c_str(), sizeof(this->program) - 1);
                            this->program[sizeof(this->program) - 1] = 0;
                            //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                            const char * session_probe_working_dir = "%TMP%";
                            strncpy(this->directory, session_probe_working_dir, sizeof(this->directory) - 1);
                            this->directory[sizeof(this->directory) - 1] = 0;

                            this->session_probe_launcher =
                                std::make_unique<SessionProbeAlternateShellBasedLauncher>(
                                    this->verbose);
                        }
                    }
                }
            }
        }

        LOG(LOG_INFO, "Server key layout is %x", unsigned(this->keylayout));

        this->nego.set_identity(this->username,
                                this->domain,
                                this->password,
                                this->hostname);

        if (bool(this->verbose & RDPVerbose::connection)){
            this->redir_info.log(LOG_INFO, "Init with Redir_info");
        }
        {
            bool load_balance_info_used = false;

            const size_t load_balance_info_length = this->load_balance_info.length();

            if (!this->redir_info.valid && load_balance_info_length) {
                if (load_balance_info_length + 2 < sizeof(this->redir_info.lb_info)) {
                    load_balance_info_used = true;

                    this->redir_info.valid = true;

                    ::snprintf(::char_ptr_cast(this->redir_info.lb_info),
                        sizeof(this->redir_info.lb_info), "%s\x0D\x0A",
                        this->load_balance_info.c_str());
                    this->redir_info.lb_info_length = load_balance_info_length + 2;
                }
                else {
                    LOG(LOG_WARNING, "mod_rdp: load balance info too long! %zu >= %zu",
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

        if (this->remote_program) {
            this->remote_programs_session_manager =
                std::make_unique<RemoteProgramsSessionManager>(
                    front, *this, this->lang, this->font,
                    mod_rdp_params.theme, this->authentifier,
                    session_probe_window_title,
                    mod_rdp_params.client_execute,
                    mod_rdp_params.rail_disconnect_message_delay,
                    this->verbose
                );
        }

        LOG(LOG_INFO, "RDP mod built");
    }   // mod_rdp

    ~mod_rdp() override {
        if (this->enable_session_probe) {
            const bool disable_input_event     = false;
            const bool disable_graphics_update = false;
            this->disable_input_event_and_graphics_update(
                disable_input_event, disable_graphics_update);
        }

        delete this->transparent_recorder;

        if (!this->end_session_reason.empty()
        &&  !this->end_session_message.empty()) {
            this->report_message.report(
                this->end_session_reason.c_str(),
                this->end_session_message.c_str());
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp cache count  = %zu",
                this->orders.recv_bmp_cache_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv order count      = %zu",
                this->orders.recv_order_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp update count = %zu",
                this->recv_bmp_update);
        }

        this->remote_programs_session_manager.reset();

        if (!this->server_redirection_packet_received) {
            this->redir_info.reset();
        }
    }

    int get_fd() const override { return this->trans.get_fd(); }

protected:
    std::unique_ptr<VirtualChannelDataSender> create_to_client_sender(
        CHANNELS::ChannelNameId channel_name) const
    {
        if (!this->authorization_channels.is_authorized(channel_name))
        {
            return nullptr;
        }

        const CHANNELS::ChannelDefArray& front_channel_list =
            this->front.get_channel_list();

        const CHANNELS::ChannelDef* channel =
            front_channel_list.get_by_name(channel_name);
        if (!channel)
        {
            return nullptr;
        }

        std::unique_ptr<ToClientSender> to_client_sender =
            std::make_unique<ToClientSender>(this->front, *channel,
                this->verbose);

        return std::unique_ptr<VirtualChannelDataSender>(
            std::move(to_client_sender));
    }

    std::unique_ptr<VirtualChannelDataSender> create_to_server_sender(
        CHANNELS::ChannelNameId channel_name)
    {
        const CHANNELS::ChannelDef* channel =
            this->mod_channel_list.get_by_name(channel_name);
        if (!channel)
        {
            return nullptr;
        }

        std::unique_ptr<ToServerSender> to_server_sender =
            std::make_unique<ToServerSender>(
                this->trans,
                this->encrypt,
                this->encryptionLevel,
                this->userid,
                channel_name,
                channel->chanid,
                (channel->flags &
                 GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL),
                this->verbose);

        if (channel_name != channel_names::rdpdr) {
            return std::unique_ptr<VirtualChannelDataSender>(
                std::move(to_server_sender));
        }

        std::unique_ptr<VirtualChannelDataSender>
            virtual_channel_data_sender(std::move(to_server_sender));

        std::unique_ptr<ToServerAsynchronousSender>
            to_server_asynchronous_sender =
                std::make_unique<ToServerAsynchronousSender>(
                    virtual_channel_data_sender,
                    this->asynchronous_tasks,
                    this->asynchronous_task_event,
                    this->verbose);

        return std::unique_ptr<VirtualChannelDataSender>(
            std::move(to_server_asynchronous_sender));
    }

    const ClipboardVirtualChannel::Params
        get_clipboard_virtual_channel_params() const
    {
        ClipboardVirtualChannel::Params clipboard_virtual_channel_params(this->report_message);

        clipboard_virtual_channel_params.exchanged_data_limit            =
            this->max_clipboard_data;
        clipboard_virtual_channel_params.verbose                         =
            this->verbose;
        clipboard_virtual_channel_params.clipboard_down_authorized       =
            this->authorization_channels.cliprdr_down_is_authorized();
        clipboard_virtual_channel_params.clipboard_up_authorized         =
            this->authorization_channels.cliprdr_up_is_authorized();
        clipboard_virtual_channel_params.clipboard_file_authorized       =
            this->authorization_channels.cliprdr_file_is_authorized();
        clipboard_virtual_channel_params.dont_log_data_into_syslog       =
            this->disable_clipboard_log_syslog;
        clipboard_virtual_channel_params.dont_log_data_into_wrm          =
            this->disable_clipboard_log_wrm;

        return clipboard_virtual_channel_params;
    }

    const DynamicChannelVirtualChannel::Params
        get_dynamic_channel_virtual_channel_params() const
    {
        DynamicChannelVirtualChannel::Params dynamic_channel_virtual_channel_params(this->report_message);

        dynamic_channel_virtual_channel_params.exchanged_data_limit =
            this->max_drdynvc_data;
        dynamic_channel_virtual_channel_params.verbose              =
            this->verbose;

        return dynamic_channel_virtual_channel_params;
    }

    const FileSystemVirtualChannel::Params
        get_file_system_virtual_channel_params() const
    {
        FileSystemVirtualChannel::Params file_system_virtual_channel_params(this->report_message);

        file_system_virtual_channel_params.exchanged_data_limit            =
            this->max_rdpdr_data;
        file_system_virtual_channel_params.verbose                         =
            this->verbose;

        file_system_virtual_channel_params.client_name                     =
            this->client_name;
        file_system_virtual_channel_params.file_system_read_authorized     =
            this->authorization_channels.rdpdr_drive_read_is_authorized();
        file_system_virtual_channel_params.file_system_write_authorized    =
            this->authorization_channels.rdpdr_drive_write_is_authorized();
        file_system_virtual_channel_params.parallel_port_authorized        =
            this->authorization_channels.rdpdr_type_is_authorized(
                rdpdr::RDPDR_DTYP_PARALLEL);
        file_system_virtual_channel_params.print_authorized                =
            this->authorization_channels.rdpdr_type_is_authorized(
                rdpdr::RDPDR_DTYP_PRINT);
        file_system_virtual_channel_params.serial_port_authorized          =
            this->authorization_channels.rdpdr_type_is_authorized(
                rdpdr::RDPDR_DTYP_SERIAL);
        file_system_virtual_channel_params.smart_card_authorized           =
            this->authorization_channels.rdpdr_type_is_authorized(
                rdpdr::RDPDR_DTYP_SMARTCARD);
        file_system_virtual_channel_params.random_number                   =
            ::getpid();

        file_system_virtual_channel_params.dont_log_data_into_syslog       =
            this->disable_file_system_log_syslog;
        file_system_virtual_channel_params.dont_log_data_into_wrm          =
            this->disable_file_system_log_wrm;

        return file_system_virtual_channel_params;
    }

    const SessionProbeVirtualChannel::Params
        get_session_probe_virtual_channel_params() const
    {
        SessionProbeVirtualChannel::Params
            session_probe_virtual_channel_params(this->report_message);

        session_probe_virtual_channel_params.exchanged_data_limit                   =
            static_cast<data_size_type>(-1);
        session_probe_virtual_channel_params.verbose                                =
            this->verbose;

        session_probe_virtual_channel_params.session_probe_launch_timeout           =
            this->session_probe_launch_timeout;
        session_probe_virtual_channel_params.session_probe_launch_fallback_timeout  =
            this->session_probe_launch_fallback_timeout;
        session_probe_virtual_channel_params.session_probe_keepalive_timeout        =
            this->session_probe_keepalive_timeout;
        session_probe_virtual_channel_params.session_probe_on_keepalive_timeout     =
            this->session_probe_on_keepalive_timeout;

        session_probe_virtual_channel_params.session_probe_on_launch_failure        =
            this->session_probe_on_launch_failure;

        session_probe_virtual_channel_params.session_probe_end_disconnected_session =
            this->session_probe_end_disconnected_session;

        session_probe_virtual_channel_params.target_informations                    =
            this->session_probe_target_informations.c_str();

        session_probe_virtual_channel_params.front_width                            =
            this->front_width;
        session_probe_virtual_channel_params.front_height                           =
            this->front_height;

        session_probe_virtual_channel_params.session_probe_disconnected_application_limit       =
            this->session_probe_disconnected_application_limit;
        session_probe_virtual_channel_params.session_probe_disconnected_session_limit           =
            this->session_probe_disconnected_session_limit;
        session_probe_virtual_channel_params.session_probe_idle_session_limit       =
            this->session_probe_idle_session_limit;

        session_probe_virtual_channel_params.session_probe_enable_log               =
            this->session_probe_enable_log;
        session_probe_virtual_channel_params.session_probe_enable_log_rotation      =
            this->session_probe_enable_log_rotation;

        session_probe_virtual_channel_params.session_probe_allow_multiple_handshake =
            this->session_probe_allow_multiple_handshake;

        session_probe_virtual_channel_params.session_probe_enable_crash_dump        =
            this->session_probe_enable_crash_dump;

        session_probe_virtual_channel_params.session_probe_handle_usage_limit        =
            this->session_probe_handle_usage_limit;
        session_probe_virtual_channel_params.session_probe_memory_usage_limit        =
            this->session_probe_memory_usage_limit;

        session_probe_virtual_channel_params.real_alternate_shell                   =
            this->real_alternate_shell.c_str();
        session_probe_virtual_channel_params.real_working_dir                       =
            this->real_working_dir.c_str();

        session_probe_virtual_channel_params.session_probe_extra_system_processes   =
            this->session_probe_extra_system_processes.c_str();

        session_probe_virtual_channel_params.session_probe_outbound_connection_monitoring_rules =
            this->session_probe_outbound_connection_monitoring_rules.c_str();

        session_probe_virtual_channel_params.session_probe_process_monitoring_rules =
            this->session_probe_process_monitoring_rules.c_str();

        session_probe_virtual_channel_params.lang                                   =
            this->lang;

        session_probe_virtual_channel_params.bogus_refresh_rect_ex                  =
            (this->bogus_refresh_rect && this->allow_using_multiple_monitors &&
             (this->cs_monitor.monitorCount > 1));
        session_probe_virtual_channel_params.show_maximized                         =
            (!this->remote_program);

        return session_probe_virtual_channel_params;
    }

    const RemoteProgramsVirtualChannel::Params
        get_remote_programs_virtual_channel_params() const
    {
        RemoteProgramsVirtualChannel::Params remote_programs_virtual_channel_params(this->report_message);

        remote_programs_virtual_channel_params.exchanged_data_limit               =
            0;
        remote_programs_virtual_channel_params.verbose                            =
            this->verbose;

        remote_programs_virtual_channel_params.client_execute_flags               =
            this->client_execute_flags;
        remote_programs_virtual_channel_params.client_execute_exe_or_file         =
            this->client_execute_exe_or_file.c_str();
        remote_programs_virtual_channel_params.client_execute_working_dir         =
            this->client_execute_working_dir.c_str();
        remote_programs_virtual_channel_params.client_execute_arguments           =
            this->client_execute_arguments.c_str();

        remote_programs_virtual_channel_params.client_execute_flags_2             =
            this->real_client_execute_flags;
        remote_programs_virtual_channel_params.client_execute_exe_or_file_2       =
            this->real_client_execute_exe_or_file.c_str();
        remote_programs_virtual_channel_params.client_execute_working_dir_2       =
            this->real_client_execute_working_dir.c_str();
        remote_programs_virtual_channel_params.client_execute_arguments_2         =
            this->real_client_execute_arguments.c_str();

        remote_programs_virtual_channel_params.rail_session_manager               =
            this->remote_programs_session_manager.get();

        remote_programs_virtual_channel_params.should_ignore_first_client_execute =
            this->should_ignore_first_client_execute;

        remote_programs_virtual_channel_params.use_session_probe_to_launch_remote_program   =
            this->use_session_probe_to_launch_remote_program;

        remote_programs_virtual_channel_params.client_supports_handshakeex_pdu    =
            (this->client_rail_caps.RailSupportLevel & TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED);
        remote_programs_virtual_channel_params.client_supports_enhanced_remoteapp =
            this->remote_program_enhanced;

        return remote_programs_virtual_channel_params;
    }

public:
    static void get_proxy_managed_connection_cookie(const char * target_informations,
            size_t target_informations_length, char (&cookie)[9]) {
        SslSha1 sha1;
        sha1.update(byte_ptr_cast(target_informations), target_informations_length);
        uint8_t sig[SslSha1::DIGEST_LENGTH];
        sha1.final(sig);

        static_assert(((sizeof(cookie) % 2) == 1), "Buffer size must be an odd number");

        char * temp = cookie;
        ::memset(cookie, 0, sizeof(cookie));
        for (unsigned i = 0, c = std::min<unsigned>(sizeof(cookie) / 2, sizeof(sig) / 2);
             i < c; ++i) {
            snprintf(temp, 3, "%02X", sig[i]);
            temp += 2;
        }
    }

    void configure_extra_orders(const char * extra_orders) {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "RDP Extra orders=\"%s\"", extra_orders);
        }

        char * end;
        char const * p = extra_orders;
        for (int order_number = std::strtol(p, &end, 0);
            p != end;
            order_number = std::strtol(p, &end, 0))
        {
            if (bool(this->verbose & RDPVerbose::capabilities)) {
                LOG(LOG_INFO, "RDP Extra orders number=%d", order_number);
            }
            switch (order_number) {
            case RDP::MULTIDSTBLT:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiDstBlt");
                }
                this->enable_multidstblt = true;
                break;
            case RDP::MULTIOPAQUERECT:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiOpaqueRect");
                }
                this->enable_multiopaquerect = true;
                break;
            case RDP::MULTIPATBLT:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiPatBlt");
                }
                this->enable_multipatblt = true;
                break;
            case RDP::MULTISCRBLT:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=MultiScrBlt");
                }
                this->enable_multiscrblt = true;
                break;
            case RDP::POLYGONSC:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=PolygonSC");
                }
                this->enable_polygonsc = true;
                break;
            case RDP::POLYGONCB:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=PolygonCB");
                }
                this->enable_polygoncb = true;
                break;
            case RDP::POLYLINE:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=Polyline");
                }
                this->enable_polyline = true;
                break;
            case RDP::ELLIPSESC:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=EllipseSC");
                }
                this->enable_ellipsesc = true;
                break;
            case RDP::ELLIPSECB:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Extra orders=EllipseCB");
                }
                this->enable_ellipsecb = true;
                break;
            default:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "RDP Unknown Extra orders");
                }
                break;
            }

            p = end;
            while (*p && (*p == ' ' || *p == '\t' || *p == ',')) {
                ++p;
            }
        }
    }   // configure_extra_orders

    void configure_proxy_managed_drives(const char * proxy_managed_drives, const char * proxy_managed_drive_prefix) {
        if (bool(this->verbose & RDPVerbose::connection)) {
            LOG(LOG_INFO, "Proxy managed drives=\"%s\"", proxy_managed_drives);
        }

        for (auto & r : get_line(proxy_managed_drives, ',')) {
            auto const trimmed_range = trim(r);

            if (trimmed_range.empty()) continue;

            if (bool(this->verbose & RDPVerbose::connection)) {
                LOG(LOG_INFO, "Proxy managed drive=\"%.*s\"",
                    int(trimmed_range.size()), trimmed_range.begin());
            }

            this->file_system_drive_manager.EnableDrive(
                array_view_const_char{trimmed_range.begin(), trimmed_range.end()},
                proxy_managed_drive_prefix, this->verbose);
        }
    }   // configure_proxy_managed_drives

    void rdp_input_scancode( long param1, long param2, long device_flags, long time, Keymap2 *) override {
        if ((UP_AND_RUNNING == this->connection_finalization_state) &&
            !this->input_event_disabled) {
            if (this->first_scancode && !(device_flags & 0x8000) &&
                (!this->enable_session_probe ||
                 !this->session_probe_launcher->is_keyboard_sequences_started() ||
                 this->get_session_probe_virtual_channel().has_been_launched())
               ) {
                LOG(LOG_INFO, "mod_rdp::rdp_input_scancode: First Keyboard Event. Resend the Synchronize Event to server.");

                this->first_scancode = false;

                this->send_input(time, RDP_INPUT_SYNCHRONIZE, 0, this->last_key_flags_sent, 0);
            }

            this->send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);

            if (this->remote_programs_session_manager) {
                this->remote_programs_session_manager->input_scancode(param1, param2, device_flags);
            }
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_UNICODE, flag, unicode, 0);
        }
    }

    void rdp_input_synchronize( uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        (void)time;
        (void)param2;
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_SYNCHRONIZE, device_flags, param1, 0);
        }
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 *) override {
        //if (!(MOUSE_FLAG_MOVE & device_flags)) {
        //    LOG(LOG_INFO, "rdp_input_mouse x=%d y=%d device_flags=%d", x, y, device_flags);
        //}
        if ((UP_AND_RUNNING == this->connection_finalization_state) &&
            !this->input_event_disabled) {
            this->send_input(0, RDP_INPUT_MOUSE, device_flags, x, y);

            if (this->remote_programs_session_manager) {
                this->remote_programs_session_manager->input_mouse(device_flags, x, y);
            }
        }
    }

    void send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name, uint8_t const * data
                              , size_t length, size_t chunk_size, int flags) override {
        if (this->transparent_recorder) {
            this->transparent_recorder->send_to_front_channel( mod_channel_name, data, length
                                                             , chunk_size, flags);
        }

        const CHANNELS::ChannelDef * front_channel = this->front.get_channel_list().get_by_name(mod_channel_name);
        if (front_channel) {
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
    }

private:
    void process_asynchronous_task_event(time_t, wait_obj& /* event*/, gdi::GraphicApi&) {
        if (!this->asynchronous_tasks.front()->run(this->asynchronous_task_event)) {
            this->asynchronous_tasks.pop_front();
        }

        this->asynchronous_task_event.full_reset();

        if (!this->asynchronous_tasks.empty()) {
            this->asynchronous_tasks.front()->configure_wait_object(this->asynchronous_task_event);
        }
    }

    void process_session_probe_launcher_event(time_t, wait_obj& /*event*/, gdi::GraphicApi&) {
        if (this->session_probe_launcher) {
            this->session_probe_launcher->on_event();
        }
    }

    void process_session_probe_virtual_channel_event(time_t, wait_obj& event, gdi::GraphicApi&) {
        //LOG(LOG_INFO, "mod_rdp::process_session_probe_virtual_channel_event() ...");
        try{
            if (this->session_probe_virtual_channel_p) {
                this->session_probe_virtual_channel_p->process_event();
            }
        }
        catch (Error const & e) {
            if (e.id != ERR_SESSION_PROBE_ENDING_IN_PROGRESS)
                throw;

            this->end_session_reason.clear();
            this->end_session_message.clear();

            this->authentifier.disconnect_target();
            this->authentifier.set_auth_error_message(TR(trkeys::session_logoff_in_progress, this->lang));

            event.signal = BACK_EVENT_NEXT;
        }
        //LOG(LOG_INFO, "mod_rdp::process_session_probe_virtual_channel_event() done.");
    }

    void process_remote_program_session_manager_event(time_t, wait_obj& /*event*/, gdi::GraphicApi&) {
        if (this->remote_programs_session_manager) {
            this->remote_programs_session_manager->process_event();
        }
    }

    void process_file_system_virtual_channel_event(time_t, wait_obj& /*event*/, gdi::GraphicApi&) {
        if (this->file_system_virtual_channel) {
            this->file_system_virtual_channel->process_event();
        }
    }

public:
    void get_event_handlers(std::vector<EventHandler>& out_event_handlers) override {
        mod_api::get_event_handlers(out_event_handlers);

        if (!this->asynchronous_tasks.empty()) {
            out_event_handlers.emplace_back(
                &this->asynchronous_task_event,
                &this->asynchronous_task_event_handler,
                this->asynchronous_tasks.front()->get_file_descriptor()
            );
        }

        if (this->session_probe_launcher) {
            if (wait_obj* event = this->session_probe_launcher->get_event()) {
                out_event_handlers.emplace_back(
                    event,
                    &this->session_probe_launcher_event_handler,
                    INVALID_SOCKET
                );
            }
        }

        if (this->session_probe_virtual_channel_p) {
            if (wait_obj* event = this->session_probe_virtual_channel_p->get_event()) {
                out_event_handlers.emplace_back(
                    event,
                    &this->session_probe_virtual_channel_event_handler,
                    INVALID_SOCKET
                );
            }
        }

        if (this->remote_programs_session_manager) {
            if (wait_obj* event = this->remote_programs_session_manager->get_event()) {
                out_event_handlers.emplace_back(
                    event,
                    &this->remote_program_session_manager_event_handler,
                    INVALID_SOCKET
                );
            }
        }

        if (this->file_system_virtual_channel) {
            if (wait_obj* event = this->file_system_virtual_channel->get_event()) {
                out_event_handlers.emplace_back(
                    event,
                    &this->file_system_virtual_channel_event_handler,
                    INVALID_SOCKET
                );
            }
        }
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name,
        InStream & chunk, size_t length, uint32_t flags
    ) override {
        if (bool(this->verbose & RDPVerbose::channels)) {
            LOG(LOG_INFO,
                "mod_rdp::send_to_mod_channel: front_channel_channel=\"%s\"",
                front_channel_name);
        }
        const CHANNELS::ChannelDef * mod_channel = this->mod_channel_list.get_by_name(front_channel_name);
        if (!mod_channel) {
            return;
        }
        if (bool(this->verbose & RDPVerbose::channels)) {
            mod_channel->log(unsigned(mod_channel - &this->mod_channel_list[0]));
        }

        switch (front_channel_name) {
            case channel_names::cliprdr:
                this->total_cliprdr_amount_data_rcv_from_client += length;
                this->send_to_mod_cliprdr_channel(mod_channel, chunk, length, flags);
                break;
            case channel_names::rail:
                this->total_rail_amount_data_rcv_from_client += length;
                this->send_to_mod_rail_channel(mod_channel, chunk, length, flags);
                break;
            case channel_names::rdpdr:
                this->total_rdpdr_amount_data_rcv_from_client += length;
                this->send_to_mod_rdpdr_channel(mod_channel, chunk, length, flags);
                break;
            case channel_names::drdynvc:
                this->total_drdynvc_amount_data_rcv_from_client += length;
                this->send_to_mod_drdynvc_channel(mod_channel, chunk, length, flags);
                break;
            default:
                this->total_main_amount_data_rcv_from_client += length;
                this->send_to_channel(*mod_channel, chunk.get_data(), chunk.get_capacity(), length, flags);
        }
    }

    void log_metrics() override {
        LOG(LOG_INFO, "Session_id=%u user=\"%s\" account=\"%s\" target_host=\"%s\" Client data received by channels - main:%ld cliprdr:%ld rail:%ld  rdpdr:%ld drdynvc:%ld", this->redir_info.session_id, this->username, this->nego.get_user_name(), this->nego.get_target_host()/*"user_account", "0.0.0.0"*/,
                              this->total_main_amount_data_rcv_from_client,
                              this->total_cliprdr_amount_data_rcv_from_client,
                              this->total_rail_amount_data_rcv_from_client,
                              this->total_rdpdr_amount_data_rcv_from_client,
                              this->total_drdynvc_amount_data_rcv_from_client);

        LOG(LOG_INFO, "Session_id=%u user=\"%s\" account=\"%s\"  target_host=\"%s\" Server data received by channels - main:%ld cliprdr:%ld rail:%ld  rdpdr:%ld drdynvc:%ld", this->redir_info.session_id, this->username, this->nego.get_user_name(), this->nego.get_target_host(),
                              this->total_main_amount_data_rcv_from_server,
                              this->total_cliprdr_amount_data_rcv_from_server,
                              this->total_rail_amount_data_rcv_from_server,
                              this->total_rdpdr_amount_data_rcv_from_server,
                              this->total_drdynvc_amount_data_rcv_from_server);
    }

private:
    void send_to_mod_cliprdr_channel(const CHANNELS::ChannelDef * /*cliprdr_channel*/,
                                     InStream & chunk, size_t length, uint32_t flags) {
        ClipboardVirtualChannel& channel = this->get_clipboard_virtual_channel();


        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            InStream clone = chunk.clone();
            RDPECLIP::streamLogCliprdr(clone, flags, this->cliprdrLogStatus);
        }

        if (this->session_probe_launcher) {
            if (!this->session_probe_launcher->process_client_cliprdr_message(chunk, length, flags)) {
                return;
            }
        }

        channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());
    }

    void send_to_mod_rail_channel(const CHANNELS::ChannelDef *,
                                  InStream & chunk, size_t length, uint32_t flags) {
        RemoteProgramsVirtualChannel& channel = this->get_remote_programs_virtual_channel();

        channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());

    }   // send_to_mod_rail_channel

private:
    void send_to_mod_rdpdr_channel(const CHANNELS::ChannelDef * rdpdr_channel,
                                   InStream & chunk, size_t length, uint32_t flags) {
        if (!this->enable_rdpdr_data_analysis &&
            this->authorization_channels.rdpdr_type_all_is_authorized() &&
            !this->file_system_drive_manager.HasManagedDrive()) {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                if (bool(this->verbose & (RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump))) {
                    LOG(LOG_INFO,
                        "mod_rdp::send_to_mod_rdpdr_channel: recv from Client, "
                            "send Chunked Virtual Channel Data transparently.");
                }

                if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                    const bool send              = false;
                    const bool from_or_to_client = false;
                    uint32_t total_length = length;
                    if (total_length > CHANNELS::CHANNEL_CHUNK_LENGTH) {
                        total_length = chunk.get_capacity() - chunk.get_offset();
                    }
                    ::msgdump_d(send, from_or_to_client, length, flags,
                    chunk.get_data(), total_length);

                    rdpdr::streamLog(chunk, this->rdpdrLogStatus);
                }
            }

            this->send_to_channel(*rdpdr_channel, chunk.get_data(), chunk.get_capacity(), length, flags);
            return;
        }

        FileSystemVirtualChannel& channel = this->get_file_system_virtual_channel();

        channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());
    }

    void send_to_mod_drdynvc_channel(const CHANNELS::ChannelDef */* rdpdr_channel*/,
                                     InStream & chunk, size_t length, uint32_t flags) {
        // if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
        //     if (bool(this->verbose & (RDPVerbose::drdynvc | RDPVerbose::drdynvc_dump))) {

        //         LOG(LOG_INFO,
        //             "mod_rdp::send_to_mod_drdynvc_channel: recv from Client, "
        //                 "send Chunked Virtual Channel Data transparently.");
        //     }

        //     if (bool(this->verbose & RDPVerbose::drdynvc_dump)) {
        //         const bool send              = false;
        //         const bool from_or_to_client = false;
        //         uint32_t total_length = length;
        //         if (total_length > CHANNELS::CHANNEL_CHUNK_LENGTH) {
        //             total_length = chunk.get_capacity() - chunk.get_offset();
        //         }
        //         ::msgdump_d(send, from_or_to_client, length, flags,
        //         chunk.get_data(), total_length);

        //         rdpdr::streamLog(chunk, this->rdpdrLogStatus);
        //     }
        // }

        // this->send_to_channel(*rdpdr_channel, chunk.get_data(), chunk.get_capacity(), length, flags);
        // return;


        DynamicChannelVirtualChannel& channel = this->get_dynamic_channel_virtual_channel();

        channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());
    }

public:
    // Method used by session to transmit sesman answer for auth_channel

    void send_auth_channel_data(const char * string_data) override {
        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        StaticOutStream<65536> stream_data;
        uint32_t data_size = std::min(::strlen(string_data) + 1, stream_data.get_capacity());

        stream_data.out_copy_bytes(string_data, data_size);

        virtual_channel_pdu.send_to_server(
            this->trans, this->encrypt, this->encryptionLevel
          , this->userid, this->auth_channel_chanid
          , stream_data.get_offset()
          , this->auth_channel_flags
          , stream_data.get_data()
          , stream_data.get_offset());
    }

private:
    void send_checkout_channel_data(const char * string_data) {
        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        StaticOutStream<65536> stream_data;

        uint32_t data_size = std::min(::strlen(string_data), stream_data.get_capacity());

        stream_data.out_uint16_le(1);           // Version
        stream_data.out_uint16_le(data_size);
        stream_data.out_copy_bytes(string_data, data_size);

        virtual_channel_pdu.send_to_server(
            this->trans, this->encrypt, this->encryptionLevel
          , this->userid, this->checkout_channel_chanid
          , stream_data.get_offset()
          , this->checkout_channel_flags
          , stream_data.get_data()
          , stream_data.get_offset());
    }

    void send_to_channel(
        const CHANNELS::ChannelDef & channel,
        uint8_t const * chunk, std::size_t chunk_size,
        size_t length, uint32_t flags
    ) {
        if (channel.name == channel_names::rdpsnd && bool(this->verbose & RDPVerbose::rdpsnd)) {
            InStream clone(chunk, chunk_size);
            rdpsnd::streamLogClient(clone, flags);
        }


        if (bool(this->verbose & RDPVerbose::channels)) {
            LOG( LOG_INFO, "mod_rdp::send_to_channel length=%zu chunk_size=%zu", length, chunk_size);
            channel.log(-1u);
        }

        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        }

        if (chunk_size <= CHANNELS::CHANNEL_CHUNK_LENGTH) {
            CHANNELS::VirtualChannelPDU virtual_channel_pdu;

            virtual_channel_pdu.send_to_server(
                this->trans, this->encrypt, this->encryptionLevel
              , this->userid, channel.chanid, length, flags, chunk, chunk_size);
        }
        else {
            uint8_t const * virtual_channel_data = chunk;
            size_t          remaining_data_length = length;

            auto get_channel_control_flags = [] (uint32_t flags, size_t data_length,
                                                 size_t remaining_data_length,
                                                 size_t virtual_channel_data_length) -> uint32_t {
                if (remaining_data_length == data_length) {
                    return (flags & (~CHANNELS::CHANNEL_FLAG_LAST));
                }
                else if (remaining_data_length == virtual_channel_data_length) {
                    return (flags & (~CHANNELS::CHANNEL_FLAG_FIRST));
                }

                return (flags & (~(CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)));
            };

            do {
                const size_t virtual_channel_data_length =
                    std::min<size_t>(remaining_data_length, CHANNELS::CHANNEL_CHUNK_LENGTH);

                CHANNELS::VirtualChannelPDU virtual_channel_pdu;

                LOG(LOG_INFO, "send to server");

                virtual_channel_pdu.send_to_server( this->trans, this->encrypt, this->encryptionLevel
                                                  , this->userid, channel.chanid, length
                                                  , get_channel_control_flags(flags, length, remaining_data_length, virtual_channel_data_length)
                                                  , virtual_channel_data, virtual_channel_data_length);

                remaining_data_length -= virtual_channel_data_length;
                virtual_channel_data  += virtual_channel_data_length;
            }
            while (remaining_data_length);
        }

        if (bool(this->verbose & RDPVerbose::channels)) {
            LOG(LOG_INFO, "mod_rdp::send_to_channel done");
        }
    }

    template<class... WriterData>
    void send_data_request(uint16_t channelId, WriterData... writer_data) {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "send data request");
        }

        write_packets(
            this->trans,
            writer_data...,
            [this, channelId](StreamSize<256>, OutStream & mcs_header, std::size_t packet_size) {
                MCS::SendDataRequest_Send mcs(
                    static_cast<OutPerStream&>(mcs_header), this->userid,
                    channelId, 1, 3, packet_size, MCS::PER_ENCODING
                );

                (void)mcs;
            },
            write_x224_dt_tpdu_fn{}
        );
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "send data request done");
        }
    }

    template<class... WriterData>
    void send_data_request_ex(uint16_t channelId, WriterData ... writer_data) {
        this->send_data_request(
            channelId,
            writer_data...,
            write_sec_send_fn{0, this->encrypt, this->encryptionLevel}
        );
    }

public:
    wait_obj& get_event() override {
        return this->event;
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

    void send_connectInitialPDUwithGccConferenceCreateRequest()
    {
        char * hostname = this->hostname;

        /* Generic Conference Control (T.124) ConferenceCreateRequest */
        write_packets(
            this->trans,
            [this, &hostname](StreamSize<65536-1024>, OutStream & stream) {
                // ------------------------------------------------------------
                GCC::UserData::CSCore cs_core;

                Rect primary_monitor_rect =
                    this->cs_monitor.get_primary_monitor_rect();

                cs_core.version = this->use_rdp5?0x00080004:0x00080001;
                const bool single_monitor =
                    (!this->allow_using_multiple_monitors ||
                     (this->cs_monitor.monitorCount < 2));
                cs_core.desktopWidth  = (single_monitor ? this->front_width : primary_monitor_rect.cx + 1);
                cs_core.desktopHeight = (single_monitor ? this->front_height : primary_monitor_rect.cy + 1);
                //cs_core.highColorDepth = this->front_bpp;
                cs_core.highColorDepth = ((this->front_bpp == 32)
                    ? uint16_t(GCC::UserData::HIGH_COLOR_24BPP) : this->front_bpp);
                cs_core.keyboardLayout = this->keylayout;
                if (this->front_bpp == 32) {
                    cs_core.supportedColorDepths = 15;
                    cs_core.earlyCapabilityFlags |= GCC::UserData::RNS_UD_CS_WANT_32BPP_SESSION;
                }
                if (!single_monitor) {
                    LOG(LOG_INFO, "not a single_monitor");
                    cs_core.earlyCapabilityFlags |= GCC::UserData::RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU;
                }

                uint16_t hostlen = strlen(hostname);
                uint16_t maxhostlen = std::min(uint16_t(15), hostlen);
                for (size_t i = 0; i < maxhostlen ; i++){
                    cs_core.clientName[i] = hostname[i];
                }
                memset(&(cs_core.clientName[maxhostlen]), 0, (16 - maxhostlen) * sizeof(uint16_t));

                if (this->nego.tls){
                    cs_core.serverSelectedProtocol = this->nego.selected_protocol;
                }
                if (bool(this->verbose & RDPVerbose::security)) {
                    cs_core.log("Sending to Server");
                }
                cs_core.emit(stream);
                // ------------------------------------------------------------

                GCC::UserData::CSCluster cs_cluster;
                // TODO CGR: values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?
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
                    }
                }
                // if (!this->nego.tls){
                //     if (this->console_session){
                //         cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID | (3 << 2) ; // REDIRECTION V4
                //     }
                //     else {
                //         cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED            | (2 << 2) ; // REDIRECTION V3
                //     }
                //     }
                // else {
                //     cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED * ((3 << 2)|1);  // REDIRECTION V4
                //     if (this->console_session){
                //         cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID ;
                //     }
                // }
                if (bool(this->verbose & RDPVerbose::security)) {
                    cs_cluster.log("Sending to server");
                }
                cs_cluster.emit(stream);
                // ------------------------------------------------------------

                if (bool(this->verbose & RDPVerbose::security)) {
                    this->cs_security.log("Sending to server");
                }
                cs_security.emit(stream);
                // ------------------------------------------------------------

                const CHANNELS::ChannelDefArray & channel_list = this->front.get_channel_list();
                size_t num_channels = channel_list.size();
                if ((num_channels > 0) || this->enable_auth_channel ||
                    this->file_system_drive_manager.HasManagedDrive() ||
                    this->checkout_channel.c_str()[0]) {
                    /* Here we need to put channel information in order
                    to redirect channel data
                    from client to server passing through the "proxy" */
                    GCC::UserData::CSNet cs_net;
                    cs_net.channelCount = num_channels;
                    bool has_cliprdr_channel = false;
                    bool has_rdpdr_channel   = false;
                    bool has_rdpsnd_channel  = false;
                    for (size_t index = 0; index < num_channels; index++) {
                        const CHANNELS::ChannelDef & channel_item = channel_list[index];

                        if (!this->remote_program && channel_item.name == channel_names::rail) {
                            ::memset(cs_net.channelDefArray[index].name, 0,
                                sizeof(cs_net.channelDefArray[index].name));
                        }
                        else if (this->authorization_channels.is_authorized(channel_item.name) ||
                                 ((channel_item.name == channel_names::rdpdr ||
                                   channel_item.name == channel_names::rdpsnd) &&
                                  this->file_system_drive_manager.HasManagedDrive())
                        ) {
                            switch (channel_item.name) {
                                case channel_names::cliprdr: has_cliprdr_channel = true; break;
                                case channel_names::rdpdr:   has_rdpdr_channel = true; break;
                                case channel_names::rdpsnd:  has_rdpsnd_channel = true; break;
                            }
                            ::memcpy(cs_net.channelDefArray[index].name, channel_item.name.c_str(), 8);
                        }
                        else {
                            ::memset(cs_net.channelDefArray[index].name, 0,
                                sizeof(cs_net.channelDefArray[index].name));
                        }
                        cs_net.channelDefArray[index].options = channel_item.flags;
                        CHANNELS::ChannelDef def;
                        def.name = CHANNELS::ChannelNameId(cs_net.channelDefArray[index].name);
                        def.flags = channel_item.flags;
                        if (bool(this->verbose & RDPVerbose::channels)) {
                            def.log(index);
                        }
                        this->mod_channel_list.push_back(def);
                    }

                    // Inject a new channel for file system virtual channel (rdpdr)
                    if (!has_rdpdr_channel && this->file_system_drive_manager.HasManagedDrive()) {
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
                    if (!has_cliprdr_channel && this->session_probe_use_clipboard_based_launcher) {
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
                    if (!has_rdpsnd_channel &&
                        this->file_system_drive_manager.HasManagedDrive()) {
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

                    if (bool(this->verbose & RDPVerbose::security)) {
                        cs_net.log("Sending to server");
                    }
                    cs_net.emit(stream);
                }

                if (!single_monitor) {
                    //if (bool(this->verbose & RDPVerbose::security)) {
                        this->cs_monitor.log("Sending to server");
                    //}
                    this->cs_monitor.emit(stream);
                }
            },
            [](StreamSize<256>, OutStream & gcc_header, std::size_t packet_size) {
                GCC::Create_Request_Send(
                    static_cast<OutPerStream&>(gcc_header),
                    packet_size
                );
            },
            [](StreamSize<256>, OutStream & mcs_header, std::size_t packet_size) {
                MCS::CONNECT_INITIAL_Send mcs(mcs_header, packet_size, MCS::BER_ENCODING);
                (void)mcs;
            },
            write_x224_dt_tpdu_fn{}
        );

        this->state = MOD_RDP_BASIC_SETTINGS_EXCHANGE;
    }

    void basic_settings_exchange(InStream & x224_data)
    {
        if (bool(this->verbose & RDPVerbose::security)){
            LOG(LOG_INFO, "mod_rdp::Basic Settings Exchange");
        }

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
                        this->sc_core.recv(f.payload);
                        if (bool(this->verbose & RDPVerbose::connection)) {
                            sc_core.log("Received from server");
                        }
                        if (0x0080001 == sc_core.version){ // can't use rdp5
                            this->use_rdp5 = 0;
                        }
                    }
                    break;
                case SC_SECURITY:
                    LOG(LOG_INFO, "=================== SC_SECURITY =============");
                    {
                        this->sc_sec1.recv(f.payload);

                        if (bool(this->verbose & RDPVerbose::security)) {
                            this->sc_sec1.log("Received from server");
                        }

                        this->encryptionLevel = this->sc_sec1.encryptionLevel;
                        this->encryptionMethod = this->sc_sec1.encryptionMethod;

                        if (this->sc_sec1.encryptionLevel == 0
                            &&  this->sc_sec1.encryptionMethod == 0) { /* no encryption */
                            LOG(LOG_INFO, "No encryption");
                        }
                        else {

                            uint8_t serverRandom[SEC_RANDOM_SIZE] = {};
                            uint8_t modulus[SEC_MAX_MODULUS_SIZE] = {};
                            uint8_t exponent[SEC_EXPONENT_SIZE] = {};

                            memcpy(serverRandom, this->sc_sec1.serverRandom, this->sc_sec1.serverRandomLen);
//                                        LOG(LOG_INFO, "================= SC_SECURITY got random =============");
                            // serverCertificate (variable): The variable-length certificate containing the
                            //  server's public key information. The length in bytes is given by the
                            // serverCertLen field. If the encryptionMethod and encryptionLevel fields are
                            // both set to 0 then this field MUST NOT be present.

                            /* RSA info */
                            if (sc_sec1.dwVersion == GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
//                                        LOG(LOG_INFO, "================= SC_SECURITY CERT_CHAIN_VERSION_1");

                                memcpy(exponent, this->sc_sec1.proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
                                memcpy(modulus, this->sc_sec1.proprietaryCertificate.RSAPK.modulus,
                                       this->sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE);

                                this->server_public_key_len = this->sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE;

                            }
                            else {
                                #ifndef __EMSCRIPTEN__

//                                            LOG(LOG_INFO, "================= SC_SECURITY CERT_CHAIN_X509");
                                uint32_t const certcount = this->sc_sec1.x509.certCount;
                                if (certcount < 2){
                                    LOG(LOG_ERR, "Server didn't send enough X509 certificates");
                                    throw Error(ERR_SEC);
                                }

                                X509 *cert = this->sc_sec1.x509.certs[certcount - 1];

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

                                #endif // __EMSCRIPTEN__
                            }

                            /* Generate a client random, and determine encryption keys */
                            this->gen.random(this->client_random, SEC_RANDOM_SIZE);
                            if (bool(this->verbose & RDPVerbose::security)) {
                                LOG(LOG_INFO, "mod_rdp: Generate client random");
                            }

                            ssllib ssl;

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

                            ssl.rsa_encrypt(
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
                            memcpy(encrypt.sign_key, key_block.blob0, 16);
                            if (sc_sec1.encryptionMethod == 1){
                                ssl.sec_make_40bit(encrypt.sign_key);
                            }
                            this->decrypt.generate_key(key_block.key1, this->sc_sec1.encryptionMethod);
                            this->encrypt.generate_key(key_block.key2, this->sc_sec1.encryptionMethod);
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
                        if (bool(this->verbose & RDPVerbose::channels)){
                            LOG(LOG_INFO, "server_channels_count=%" PRIu16 " sent_channels_count=%zu",
                                sc_net.channelCount,
                                mod_channel_list.size());
                        }
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

        if (bool(this->verbose & RDPVerbose::connection)) {
            LOG(LOG_INFO, "mod_rdp::Channel Connection");
        }

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

        if (bool(this->verbose & RDPVerbose::connection)){
            LOG(LOG_INFO, "Send MCS::ErectDomainRequest");
        }
        write_packets(
            this->trans,
            [](StreamSize<256>, OutStream & mcs_header){
                MCS::ErectDomainRequest_Send mcs(
                    static_cast<OutPerStream&>(mcs_header),
                    0, 0, MCS::PER_ENCODING
                );
                (void)mcs;
            },
            write_x224_dt_tpdu_fn{}
        );

        if (bool(this->verbose & RDPVerbose::connection)){
            LOG(LOG_INFO, "Send MCS::AttachUserRequest");
        }
        write_packets(
            this->trans,
            [](StreamSize<256>, OutStream & mcs_data){
                MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);
                (void)mcs;
            },
            write_x224_dt_tpdu_fn{}
        );
        this->state = MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER;
        if (bool(this->verbose & RDPVerbose::connection)){
            LOG(LOG_INFO, "mod_rdp::Basic Settings Exchange end");
        }
    }

    void channel_connection_attach_user(InStream & stream)
    {
        if (bool(this->verbose & RDPVerbose::channels)){
            LOG(LOG_INFO, "mod_rdp::Channel Connection Attach User");
        }

        X224::DT_TPDU_Recv x224(stream);
        InStream & mcs_cjcf_data = x224.payload;
        MCS::AttachUserConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
        if (mcs.initiator_flag){
            this->userid = mcs.initiator;
        }

        size_t const num_channels = this->mod_channel_list.size();

        uint16_t channels_id[CHANNELS::MAX_STATIC_VIRTUAL_CHANNELS + 2];
        channels_id[0] = this->userid + GCC::MCS_USERCHANNEL_BASE;
        channels_id[1] = GCC::MCS_GLOBAL_CHANNEL;
        for (size_t index = 0; index < num_channels; index++){
            channels_id[index+2] = this->mod_channel_list[index].chanid;
        }

        for (size_t index = 0; index < num_channels+2; index++) {
            if (bool(this->verbose & RDPVerbose::channels)){
                LOG(LOG_INFO, "cjrq[%zu] = %" PRIu16, index, channels_id[index]);
            }
            write_packets(
                this->trans,
                [this, &channels_id, index](StreamSize<256>, OutStream & mcs_cjrq_data){
                    MCS::ChannelJoinRequest_Send mcs(
                        mcs_cjrq_data, this->userid,
                        channels_id[index], MCS::PER_ENCODING
                    );
                    (void)mcs;
                },
                write_x224_dt_tpdu_fn{}
            );
        }

        if (bool(this->verbose & RDPVerbose::channels)){
            LOG(LOG_INFO, "mod_rdp::Channel Connection Attach User end");
            LOG(LOG_INFO, "Waiting for Channel Join Confirm");
        }

        this->state = MOD_RDP_CHANNEL_JOIN_CONFIRME;
        this->send_channel_index = 0;
    }

    size_t send_channel_index;

    void channel_join_confirme(time_t now, InStream & x224_data)
    {
        {
            X224::DT_TPDU_Recv x224(x224_data);
            InStream & mcs_cjcf_data = x224.payload;
            MCS::ChannelJoinConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
            // TODO If mcs.result is negative channel is not confirmed and should be removed from mod_channel list
            if (bool(this->verbose & RDPVerbose::channels)){
                LOG(LOG_INFO, "cjcf[%zu] = %" PRIu16, this->send_channel_index, mcs.channelId);
            }
        }

        ++this->send_channel_index;
        if (this->send_channel_index < this->mod_channel_list.size()+2) {
            return ;
        }

        if (bool(this->verbose & RDPVerbose::channels)){
            LOG(LOG_INFO, "mod_rdp::Channel Join Confirme end");
        }

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
        if (bool(this->verbose & RDPVerbose::security)){
            LOG(LOG_INFO, "mod_rdp::RDP Security Commencement");
        }

        if (this->encryptionLevel){
            if (bool(this->verbose & RDPVerbose::security)){
                LOG(LOG_INFO, "mod_rdp::SecExchangePacket keylen=%u",
                    this->server_public_key_len);
            }

            this->send_data_request(
                GCC::MCS_GLOBAL_CHANNEL,
                dynamic_packet(this->server_public_key_len + 32, [this](OutStream & stream) {
                    SEC::SecExchangePacket_Send mcs(
                        stream, this->client_crypt_random, this->server_public_key_len
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

        if (bool(this->verbose & RDPVerbose::security)){
            LOG(LOG_INFO, "mod_rdp::Secure Settings Exchange");
        }

        this->send_client_info_pdu(now);
        this->state = MOD_RDP_GET_LICENSE;
    }

    void get_license(InStream & stream)
    {
        if (bool(this->verbose & RDPVerbose::license)){
            LOG(LOG_INFO, "mod_rdp::Licensing");
        }
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

        const char * hostname = this->hostname;
        const char * username;
        char username_a_domain[512];
        if (this->domain[0]) {
            snprintf(username_a_domain, sizeof(username_a_domain), "%s@%s", this->username, this->domain);
            username = username_a_domain;
        }
        else {
            username = this->username;
        }
        LOG(LOG_INFO, "Rdp::Get license: username=\"%s\"", username);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)

        X224::DT_TPDU_Recv x224(stream);

        // TODO Shouldn't we use mcs_type to manage possible Deconnection Ultimatum here
        //int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);
        MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);
        SEC::SecSpecialPacket_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);

        if (sec.flags & SEC::SEC_LICENSE_PKT) {
            LIC::RecvFactory flic(sec.payload);

            switch (flic.tag) {
            case LIC::LICENSE_REQUEST:
                if (bool(this->verbose & RDPVerbose::license)) {
                    LOG(LOG_INFO, "Rdp::License Request");
                }
                {
                    LIC::LicenseRequest_Recv lic(sec.payload);
                    uint8_t null_data[48]{};
                    /* We currently use null client keys. This is a bit naughty but, hey,
                        the security of license negotiation isn't exactly paramount. */
                    SEC::SessionKey keyblock(null_data, null_data, lic.server_random);

                    /* Store first 16 bytes of session key as MAC secret */
                    memcpy(this->lic_layer_license_sign_key, keyblock.get_MAC_salt_key(), 16);
                    memcpy(this->lic_layer_license_key, keyblock.get_LicensingEncryptionKey(), 16);
                }
                this->send_data_request(
                    GCC::MCS_GLOBAL_CHANNEL,
                    [this, &hostname, &username](StreamSize<65535 - 1024>, OutStream & lic_data) {
                        if (this->lic_layer_license_size > 0) {
                            uint8_t hwid[LIC::LICENSE_HWID_SIZE];

                            buf_out_uint32(hwid, 2);
                            memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                            /* Generate a signature for the HWID buffer */
                            uint8_t signature[LIC::LICENSE_SIGNATURE_SIZE];

                            uint8_t lenhdr[4];
                            buf_out_uint32(lenhdr, sizeof(hwid));

                            Sign sign(this->lic_layer_license_sign_key, 16);
                            sign.update(lenhdr, sizeof(lenhdr));
                            sign.update(hwid, sizeof(hwid));

                            static_assert(static_cast<size_t>(SslMd5::DIGEST_LENGTH) == static_cast<size_t>(LIC::LICENSE_SIGNATURE_SIZE), "");
                            sign.final(signature, sizeof(signature));


                            /* Now encrypt the HWID */

                            SslRC4 rc4;
                            rc4.set_key(this->lic_layer_license_key, 16);

                            // in, out
                            rc4.crypt(LIC::LICENSE_HWID_SIZE, hwid, hwid);

                            LIC::ClientLicenseInfo_Send(
                                lic_data, this->use_rdp5?3:2,
                                this->lic_layer_license_size,
                                this->lic_layer_license_data.get(),
                                hwid, signature
                            );
                        }
                        else {
                            LIC::NewLicenseRequest_Send(
                                lic_data, this->use_rdp5?3:2, username, hostname
                            );
                        }
                    },
                    write_sec_send_fn{SEC::SEC_LICENSE_PKT, this->encrypt, 0}
                );
                break;
            case LIC::PLATFORM_CHALLENGE:
                if (bool(this->verbose & RDPVerbose::license)){
                    LOG(LOG_INFO, "Rdp::Platform Challenge");
                }
                {
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
                    rc4_decrypt_token.set_key(this->lic_layer_license_key, 16);
                    // size, in, out
                    rc4_decrypt_token.crypt(LIC::LICENSE_TOKEN_SIZE, decrypt_token, decrypt_token);

                    /* Generate a signature for a buffer of token and HWID */
                    buf_out_uint32(hwid, 2);
                    memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                    uint8_t sealed_buffer[LIC::LICENSE_TOKEN_SIZE + LIC::LICENSE_HWID_SIZE];
                    memcpy(sealed_buffer, decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                    memcpy(sealed_buffer + LIC::LICENSE_TOKEN_SIZE, hwid, LIC::LICENSE_HWID_SIZE);

                    uint8_t lenhdr[4];
                    buf_out_uint32(lenhdr, sizeof(sealed_buffer));

                    Sign sign(this->lic_layer_license_sign_key, 16);
                    sign.update(lenhdr, sizeof(lenhdr));
                    sign.update(sealed_buffer, sizeof(sealed_buffer));

                    static_assert(static_cast<size_t>(SslMd5::DIGEST_LENGTH) == static_cast<size_t>(LIC::LICENSE_SIGNATURE_SIZE), "");
                    sign.final(out_sig, sizeof(out_sig));

                    /* Now encrypt the HWID */
                    memcpy(crypt_hwid, hwid, LIC::LICENSE_HWID_SIZE);
                    SslRC4 rc4_hwid;
                    rc4_hwid.set_key(this->lic_layer_license_key, 16);
                    // size, in, out
                    rc4_hwid.crypt(LIC::LICENSE_HWID_SIZE, crypt_hwid, crypt_hwid);

                    this->send_data_request(
                        GCC::MCS_GLOBAL_CHANNEL,
                        [&, this](StreamSize<65535 - 1024>, OutStream & lic_data) {
                            LIC::ClientPlatformChallengeResponse_Send(
                                lic_data, this->use_rdp5?3:2, out_token, crypt_hwid, out_sig
                            );
                        },
                        write_sec_send_fn{SEC::SEC_LICENSE_PKT, this->encrypt, 0}
                    );
                }
                break;
            case LIC::NEW_LICENSE:
                {
                    if (bool(this->verbose & RDPVerbose::license)){
                        LOG(LOG_INFO, "Rdp::New License");
                    }

                    LIC::NewLicense_Recv lic(sec.payload, this->lic_layer_license_key);

                    // TODO CGR: Save license to keep a local copy of the license of a remote server thus avoiding to ask it every time we connect.
                    // Not obvious files is the best choice to do that
                        this->state = MOD_RDP_CONNECTED;

                    LOG(LOG_WARNING, "New license not saved");
                }
                break;
            case LIC::UPGRADE_LICENSE:
                {
                    if (bool(this->verbose & RDPVerbose::license)){
                        LOG(LOG_INFO, "Rdp::Upgrade License");
                    }
                    LIC::UpgradeLicense_Recv lic(sec.payload, this->lic_layer_license_key);

                    LOG(LOG_WARNING, "Upgraded license not saved");
                }
                break;
            case LIC::ERROR_ALERT:
                {
                    if (bool(this->verbose & RDPVerbose::license)){
                        LOG(LOG_INFO, "Rdp::Get license status");
                    }
                    LIC::ErrorAlert_Recv lic(sec.payload);
                    if ((lic.validClientMessage.dwErrorCode == LIC::STATUS_VALID_CLIENT)
                        && (lic.validClientMessage.dwStateTransition == LIC::ST_NO_TRANSITION)){
                        this->state = MOD_RDP_CONNECTED;
                    }
                    else {
                        LOG(LOG_ERR, "RDP::License Alert: error=%u transition=%u",
                            lic.validClientMessage.dwErrorCode, lic.validClientMessage.dwStateTransition);
                    }
                    this->state = MOD_RDP_CONNECTED;
                }
                break;
            default:
                {
                    LOG(LOG_ERR, "Unexpected license tag sent from server (tag = 0x%x)", flic.tag);
                    throw Error(ERR_SEC);
                }
                break;
            }

            if (sec.payload.get_current() != sec.payload.get_data_end()){
                LOG(LOG_ERR, "all data should have been consumed %s:%d tag = 0x%x", __FILE__, __LINE__, flic.tag);
                throw Error(ERR_SEC);
            }
        }
        else {
            LOG(LOG_WARNING, "Failed to get expected license negotiation PDU, sec.flags=0x%x", sec.flags);
            hexdump(x224.payload.get_data(), x224.payload.get_capacity());
            //throw Error(ERR_SEC);
            this->state = MOD_RDP_CONNECTED;
            hexdump(sec.payload.get_data(), sec.payload.get_capacity());
        }
    }

    // Capabilities Exchange
    // ---------------------

    // Capabilities Negotiation: The server sends the set of capabilities it
    // supports to the client in a Demand Active PDU. The client responds with its
    // capabilities by sending a Confirm Active PDU.

    // Client                                                     Server
    //    | <------- Demand Active PDU ---------------------------- |
    //    |--------- Confirm Active PDU --------------------------> |

    // Connection Finalization
    // -----------------------

    // Connection Finalization: The client and server send PDUs to finalize the
    // connection details. The client-to-server and server-to-client PDUs exchanged
    // during this phase may be sent concurrently as long as the sequencing in
    // either direction is maintained (there are no cross-dependencies between any
    // of the client-to-server and server-to-client PDUs). After the client receives
    // the Font Map PDU it can start sending mouse and keyboard input to the server,
    // and upon receipt of the Font List PDU the server can start sending graphics
    // output to the client.

    // Client                                                     Server
    //    |----------Synchronize PDU------------------------------> |
    //    |----------Control PDU Cooperate------------------------> |
    //    |----------Control PDU Request Control------------------> |
    //    |----------Persistent Key List PDU(s)-------------------> |
    //    |----------Font List PDU--------------------------------> |

    //    | <--------Synchronize PDU------------------------------- |
    //    | <--------Control PDU Cooperate------------------------- |
    //    | <--------Control PDU Granted Control------------------- |
    //    | <--------Font Map PDU---------------------------------- |

    // All PDU's in the client-to-server direction must be sent in the specified
    // order and all PDU's in the server to client direction must be sent in the
    // specified order. However, there is no requirement that client to server PDU's
    // be sent before server-to-client PDU's. PDU's may be sent concurrently as long
    // as the sequencing in either direction is maintained.


    // Besides input and graphics data, other data that can be exchanged between
    // client and server after the connection has been finalized include "
    // connection management information and virtual channel messages (exchanged
    // between client-side plug-ins and server-side applications).

    void connected_fast_path(gdi::GraphicApi & drawable, array_view_u8 array)
    {
        InStream stream(array);
        FastPath::ServerUpdatePDU_Recv su(stream, this->decrypt, array.data());
        if (this->enable_transparent_mode) {
            //total_data_received += su.payload.size();
            //LOG(LOG_INFO, "total_data_received=%llu", total_data_received);
            if (this->transparent_recorder) {
                this->transparent_recorder->send_fastpath_data(su.payload);
            }
            this->front.send_fastpath_data(su.payload);

            return;
        }

        while (su.payload.in_remain()) {
            FastPath::Update_Recv upd(su.payload, &this->mppc_dec);

            if (bool(this->verbose & RDPVerbose::connection)) {
                const char * m = "UNKNOWN ORDER";
                using FU = FastPath::UpdateType;
                switch (static_cast<FastPath::UpdateType>(upd.updateCode))
                {
                case FU::ORDERS:      m = "ORDERS"; break;
                case FU::BITMAP:      m = "BITMAP"; break;
                case FU::PALETTE:     m = "PALETTE"; break;
                case FU::SYNCHRONIZE: m = "SYNCHRONIZE"; break;
                case FU::SURFCMDS:    m = "SYNCHRONIZE"; break;
                case FU::PTR_NULL:    m = "PTR_NULL"; break;
                case FU::PTR_DEFAULT: m = "PTR_DEFAULT"; break;
                case FU::PTR_POSITION:m = "PTR_POSITION"; break;
                case FU::COLOR:       m = "COLOR"; break;
                case FU::CACHED:      m = "CACHED"; break;
                case FU::POINTER:     m = "POINTER"; break;
                }
                LOG(LOG_INFO, "FastPath::UpdateType::%s", m);
                upd.log(LOG_INFO);
            }

            if (upd.fragmentation != FastPath::FASTPATH_FRAGMENT_SINGLE) {
                if (upd.fragmentation == FastPath::FASTPATH_FRAGMENT_FIRST) {
                    this->multifragment_update_data.rewind();
                }
                this->multifragment_update_data.out_copy_bytes(upd.payload.get_data(), upd.payload.get_capacity());
                if (upd.fragmentation != FastPath::FASTPATH_FRAGMENT_LAST) {
                    continue;
                }
            }

            InStream fud(this->multifragment_update_data.get_data(), this->multifragment_update_data.get_offset());

            InStream& stream = ((upd.fragmentation == FastPath::FASTPATH_FRAGMENT_SINGLE) ? upd.payload : fud);

            switch (static_cast<FastPath::UpdateType>(upd.updateCode)) {
            case FastPath::UpdateType::ORDERS:
                this->front.begin_update();
                this->orders.process_orders(
                    stream, true, drawable,
                    this->front_width, this->front_height);
                this->front.end_update();
                break;

            case FastPath::UpdateType::BITMAP:
                this->front.begin_update();
                this->process_bitmap_updates(stream, true, drawable);
                this->front.end_update();
                break;

            case FastPath::UpdateType::PALETTE:
                this->front.begin_update();
                this->process_palette(stream, true);
                this->front.end_update();
                break;

            case FastPath::UpdateType::SYNCHRONIZE:
                // TODO: we should propagate SYNCHRONIZE to front
                break;

            case FastPath::UpdateType::SURFCMDS:
                LOG( LOG_ERR
                , "mod::rdp: received unsupported fast-path PUD, updateCode = %s"
                , "FastPath::UPDATETYPE_SURFCMDS");
                throw Error(ERR_RDP_FASTPATH);

            case FastPath::UpdateType::PTR_NULL:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer null (Fast)");
                }
                drawable.set_pointer(Pointer(NullPointer{}));
                break;

            case FastPath::UpdateType::PTR_DEFAULT:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer default (Fast)");
                }
                drawable.set_pointer(Pointer(SystemDefaultPointer{}));
                break;

            case FastPath::UpdateType::PTR_POSITION:
                {
                    if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                        LOG(LOG_INFO, "Process pointer position (Fast)");
                    }

                    const unsigned expected = 4; /* xPos(2) + yPos(2) */
                    if (!stream.in_check_rem(expected)){
                        LOG(LOG_ERR, "Truncated Fast-Path Pointer Position Update, need=%u remains=%zu",
                            expected, stream.in_remain());
                        //throw Error(ERR_RDP_DATA_TRUNCATED);
                        break;
                    }

                    uint16_t xPos = stream.in_uint16_le();
                    uint16_t yPos = stream.in_uint16_le();
                    this->front.update_pointer_position(xPos, yPos);
                }
                break;


// 2.2.9.1.2.1.7 Fast-Path Color Pointer Update (TS_FP_COLORPOINTERATTRIBUTE)
// =========================================================================

// updateHeader (1 byte): An 8-bit, unsigned integer. The format of this field is
// the same as the updateHeader byte field specified in the Fast-Path Update
// (section 2.2.9.1.2.1) structure. The updateCode bitfield (4 bits in size) MUST
// be set to FASTPATH_UPDATETYPE_COLOR (9).

// compressionFlags (1 byte): An 8-bit, unsigned integer. The format of this optional
// field (as well as the possible values) is the same as the compressionFlags field
// specified in the Fast-Path Update structure.

// size (2 bytes): A 16-bit, unsigned integer. The format of this field (as well as
// the possible values) is the same as the size field specified in the Fast-Path
// Update structure.

// colorPointerUpdateData (variable): Color pointer data. Both slow-path and
// fast-path utilize the same data format, a Color Pointer Update (section
// 2.2.9.1.1.4.4) structure, to represent this information.


            case FastPath::UpdateType::COLOR:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer color (Fast)");
                }
                this->process_color_pointer_pdu(stream, drawable);
                break;

            case FastPath::UpdateType::CACHED:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer cached (Fast)");
                }
                this->process_cached_pointer_pdu(stream, drawable);
                break;

            case FastPath::UpdateType::POINTER:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer new (Fast)");
                }
                this->process_new_pointer_pdu(stream, drawable);
                break;

            default:
                LOG( LOG_ERR
                , "mod::rdp: received unexpected fast-path PUD, updateCode = %u"
                , upd.updateCode);
                throw Error(ERR_RDP_FASTPATH);
            }
        }

        // TODO Chech all data in the PDU is consumed
    }

    void connected_slow_path(time_t now, gdi::GraphicApi & drawable, InStream & stream)
    {
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)

        X224::DT_TPDU_Recv x224(stream);

        const int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);

        if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum){
            LOG(LOG_INFO, "mod::rdp::DisconnectProviderUltimatum received");
            x224.payload.rewind();
            MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
            const char * reason = MCS::get_reason(mcs.reason);
            LOG(LOG_ERR, "mod::rdp::DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);

            this->end_session_reason.clear();
            this->end_session_message.clear();

            if ((!this->session_probe_virtual_channel_p
                || !this->session_probe_virtual_channel_p->is_disconnection_reconnection_required())
             && !this->remote_apps_not_enabled) {
                this->authentifier.disconnect_target();
            }
            this->report_message.report("CLOSE_SESSION_SUCCESSFUL", "OK.");

            if (!this->session_disconnection_logged) {
                double seconds = ::difftime(now, this->beginning);

                char duration[1024];
                snprintf(duration, sizeof(duration), "%d:%02d:%02d",
                    (int(seconds) / 3600), ((int(seconds) % 3600) / 60),
                    (int(seconds) % 60));

                auto info = key_qvalue_pairs({
                    {"type", "SESSION_DISCONNECTION"},
                    {"duration", duration},
                    });

                this->report_message.log5(info);

                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO, "%s", info);
                }
                this->session_disconnection_logged = true;
            }
            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
        }


        MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);
        SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->encryptionLevel);
        if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL){
            if (bool(this->verbose & RDPVerbose::channels)) {
                LOG(LOG_INFO, "received channel data on mcs.chanid=%u", mcs.channelId);
            }

            int num_channel_src = this->mod_channel_list.get_index_by_id(mcs.channelId);
            if (num_channel_src == -1) {
                LOG(LOG_ERR, "mod::rdp::MOD_RDP_CONNECTED::Unknown Channel id=%d", mcs.channelId);
                throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
            }

            const CHANNELS::ChannelDef & mod_channel = this->mod_channel_list[num_channel_src];
            if (bool(this->verbose & RDPVerbose::channels)) {
                mod_channel.log(num_channel_src);
            }

            uint32_t length = sec.payload.in_uint32_le();
            int flags = sec.payload.in_uint32_le();
            size_t chunk_size = sec.payload.in_remain();

            // If channel name is our virtual channel, then don't send data to front
                 if (mod_channel.name == this->auth_channel && this->enable_auth_channel) {
                this->process_auth_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            else if (mod_channel.name == this->checkout_channel) {
                this->process_checkout_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            else if (mod_channel.name == channel_names::sespro) {
                this->process_session_probe_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            // Clipboard is a Clipboard PDU
            else if (mod_channel.name == channel_names::cliprdr) {
                this->total_cliprdr_amount_data_rcv_from_server += length;
                this->process_cliprdr_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            else if (mod_channel.name == channel_names::rail) {
                this->total_rail_amount_data_rcv_from_server += length;
                this->process_rail_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            else if (mod_channel.name == channel_names::rdpdr) {
                this->total_rdpdr_amount_data_rcv_from_server += length;
                this->process_rdpdr_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            else if (mod_channel.name == channel_names::drdynvc) {
                this->total_drdynvc_amount_data_rcv_from_server += length;
                this->process_drdynvc_event(mod_channel, sec.payload, length, flags, chunk_size);
            }
            else {
                if (mod_channel.name == channel_names::rdpsnd && bool(this->verbose & RDPVerbose::rdpsnd)) {
                    InStream clone = sec.payload.clone();
                    rdpsnd::streamLogServer(clone, flags);
                }

                this->total_main_amount_data_rcv_from_server += length;

                this->send_to_front_channel(
                    mod_channel.name, sec.payload.get_current(), length, chunk_size, flags
                );
            }


            sec.payload.in_skip_bytes(sec.payload.in_remain());

        }
        else {

            uint8_t const * next_packet = sec.payload.get_current();
            while (next_packet < sec.payload.get_data_end()) {
                sec.payload.rewind();
                sec.payload.in_skip_bytes(next_packet - sec.payload.get_data());

                uint8_t const * current_packet = next_packet;

                if  (peekFlowPDU(sec.payload)){
                    if (bool(this->verbose & RDPVerbose::connection)) {
                        LOG(LOG_WARNING, "FlowPDU TYPE");
                    }
                    ShareFlow_Recv sflow(sec.payload);
                    // ignoring
                    // if (sctrl.flow_pdu_type == FLOW_TEST_PDU) {
                    //     this->send_flow_response_pdu(sctrl.flow_id,
                    //                                  sctrl.flow_number);
                    // }
                    next_packet = sec.payload.get_current();
                }
                else {

                    ShareControl_Recv sctrl(sec.payload);
                    //sctrl.log();
                    next_packet += sctrl.totalLength;

                    if (bool(this->verbose & RDPVerbose::basic_trace)) {
                        LOG(LOG_WARNING, "LOOPING on PDUs: %u", unsigned(sctrl.totalLength));
                    }

                    switch (sctrl.pduType) {
                    case PDUTYPE_DATAPDU:
                        if (bool(this->verbose & RDPVerbose::basic_trace)) {
                            LOG(LOG_WARNING, "PDUTYPE_DATAPDU");
                        }
                        switch (this->connection_finalization_state){
                        case EARLY:
                        {
                            ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                            // sdata.log();
                            LOG(LOG_ERR, "sdata.pdutype2=%u", sdata.pdutype2);

                            if (sdata.pdutype2 == PDUTYPE2_SET_ERROR_INFO_PDU)
                            {
                                if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");}
                                uint32_t error_info = this->get_error_info_from_pdu(sdata.payload);
                                this->process_error_info(error_info);
                                if (error_info == ERRINFO_SERVER_DENIED_CONNECTION) {
                                    this->close_box_extra_message_ref += " ";
                                    this->close_box_extra_message_ref += TR(trkeys::err_server_denied_connection, this->lang);
                                }
                            }

                            LOG(LOG_ERR, "Rdp::finalization is early");
                            throw Error(ERR_SEC);
                        }
                        case WAITING_SYNCHRONIZE:
                            if (bool(this->verbose & RDPVerbose::basic_trace)){
                                LOG(LOG_WARNING, "WAITING_SYNCHRONIZE");
                            }

                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();

                                if (sdata.pdutype2 == PDUTYPE2_MONITOR_LAYOUT_PDU) {

                                    MonitorLayoutPDU monitor_layout_pdu;

                                    monitor_layout_pdu.recv(sdata.payload);
                                    monitor_layout_pdu.log(
                                        "Rdp::receiving the server-to-client Monitor Layout PDU");

                                    if (this->cs_monitor.monitorCount &&
                                        (monitor_layout_pdu.get_monitorCount() !=
                                         this->cs_monitor.monitorCount)) {

                                        LOG(LOG_ERR, "Server do not support the display monitor layout of the client");
                                        throw Error(ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT);
                                    }
                                }
                                else {
                                    LOG(LOG_INFO, "Resizing to %ux%ux%u", this->front_width, this->front_height, this->orders.bpp);


                                    if (this->transparent_recorder) {
                                        this->transparent_recorder->server_resize(this->front_width,
                                            this->front_height, this->orders.bpp);
                                    }


                                    if (FrontAPI::ResizeResult::fail == this->front.server_resize(this->front_width, this->front_height, this->orders.bpp)){
                                        LOG(LOG_ERR, "Resize not available on older clients,"
                                            " change client resolution to match server resolution");
                                        throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
                                    }

                                    this->connection_finalization_state = WAITING_CTL_COOPERATE;
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                }
                            }
                            break;
                        case WAITING_CTL_COOPERATE:
                            if (bool(this->verbose & RDPVerbose::basic_trace)){
                                LOG(LOG_WARNING, "WAITING_CTL_COOPERATE");
                            }
                            this->connection_finalization_state = WAITING_GRANT_CONTROL_COOPERATE;
                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();
                                sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                            }
                            break;
                        case WAITING_GRANT_CONTROL_COOPERATE:
                            if (bool(this->verbose & RDPVerbose::basic_trace)){
                                LOG(LOG_WARNING, "WAITING_GRANT_CONTROL_COOPERATE");
                            }
                            this->connection_finalization_state = WAITING_FONT_MAP;
                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();
                                sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                            }
                            break;
                        case WAITING_FONT_MAP:
                            if (bool(this->verbose & RDPVerbose::basic_trace)){
                                LOG(LOG_WARNING, "PDUTYPE2_FONTMAP");
                            }
                            this->connection_finalization_state = UP_AND_RUNNING;

                            if (!this->deactivation_reactivation_in_progress) {
                                this->report_message.log5("type=\"SESSION_ESTABLISHED_SUCCESSFULLY\"");
                            }

                            // Synchronize sent to indicate server the state of sticky keys (x-locks)
                            // Must be sent at this point of the protocol (sent before, it xwould be ignored or replaced)
                            rdp_input_synchronize(0, 0, (this->key_flags & 0x07), 0);
                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();
                                sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                            }

                            this->deactivation_reactivation_in_progress = false;

                            if (!this->already_upped_and_running) {
                                this->do_enable_session_probe();

                                if (this->open_session_timeout.count() > 0) {
                                    this->event.set_trigger_time(wait_obj::NOW);
                                }

                                this->already_upped_and_running = true;
                            }

                            if (this->front.can_be_start_capture()) {
                                if (this->bogus_refresh_rect
                                 && allow_using_multiple_monitors
                                 && this->cs_monitor.monitorCount > 1
                                ) {
                                    this->rdp_suppress_display_updates();
                                    this->rdp_allow_display_updates(0, 0, this->front_width, this->front_height);
                                }
                                this->rdp_input_invalidate(Rect(0, 0, this->front_width, this->front_height));
                            }
                            break;
                        case UP_AND_RUNNING:
                            if (this->enable_transparent_mode)
                            {
                                sec.payload.rewind();
                                sec.payload.in_skip_bytes(current_packet - sec.payload.get_data());

                                StaticOutStream<65535> copy_stream;
                                copy_stream.out_copy_bytes(current_packet, next_packet - current_packet);

                                //total_data_received += copy_stream.size();
                                //LOG(LOG_INFO, "total_data_received=%llu", total_data_received);

                                if (this->transparent_recorder) {
                                    this->transparent_recorder->send_data_indication_ex(
                                        mcs.channelId,
                                        copy_stream.get_data(),
                                        copy_stream.get_offset()
                                    );
                                }
                                this->front.send_data_indication_ex(
                                    mcs.channelId,
                                    copy_stream.get_data(),
                                    copy_stream.get_offset()
                                );

                                next_packet = sec.payload.get_data_end();

                                break;
                            }

                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();

                                switch (sdata.pdutype2) {
                                case PDUTYPE2_UPDATE:
                                    {
                                        if (bool(this->verbose & RDPVerbose::basic_trace)){
                                            LOG(LOG_INFO, "PDUTYPE2_UPDATE");
                                        }
                                        // MS-RDPBCGR: 1.3.6
                                        // -----------------
                                        // The most fundamental output that a server can send to a connected client
                                        // is bitmap images of the remote session using the Update Bitmap PDU. This
                                        // allows the client to render the working space and enables a user to
                                        // interact with the session running on the server. The global palette
                                        // information for a session is sent to the client in the Update Palette PDU.

                                        SlowPath::GraphicsUpdate_Recv gur(sdata.payload);
                                        switch (gur.update_type) {
                                        case RDP_UPDATE_ORDERS:
                                            if (bool(this->verbose & RDPVerbose::graphics)){ LOG(LOG_INFO, "RDP_UPDATE_ORDERS"); }
                                            this->front.begin_update();
                                            this->orders.process_orders(sdata.payload, false,
                                                drawable, this->front_width, this->front_height);
                                            this->front.end_update();
                                            break;
                                        case RDP_UPDATE_BITMAP:
                                            if (bool(this->verbose & RDPVerbose::graphics)){ LOG(LOG_INFO, "RDP_UPDATE_BITMAP");}
                                            this->front.begin_update();
                                            this->process_bitmap_updates(sdata.payload, false, drawable);
                                            this->front.end_update();
                                            break;
                                        case RDP_UPDATE_PALETTE:
                                            if (bool(this->verbose & RDPVerbose::graphics)){ LOG(LOG_INFO, "RDP_UPDATE_PALETTE");}
                                            this->front.begin_update();
                                            this->process_palette(sdata.payload, false);
                                            this->front.end_update();
                                            break;
                                        case RDP_UPDATE_SYNCHRONIZE:
                                            if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "RDP_UPDATE_SYNCHRONIZE");}
                                            sdata.payload.in_skip_bytes(2);
                                            break;
                                        default:
                                            if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_WARNING, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_UNKNOWN");}
                                            break;
                                        }
                                    }
                                    break;
                                case PDUTYPE2_CONTROL:
                                    if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_CONTROL");}
                                    // TODO CGR: Data should actually be consumed
                                        sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_SYNCHRONIZE:
                                    if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");}
                                    // TODO CGR: Data should actually be consumed
                                        sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_POINTER:
                                    if (bool(this->verbose & RDPVerbose::graphics_pointer)){ LOG(LOG_INFO, "PDUTYPE2_POINTER");}
                                    this->process_pointer_pdu(sdata.payload, drawable);
                                    // TODO CGR: Data should actually be consumed
                                        sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_PLAY_SOUND:
                                    if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_PLAY_SOUND");}
                                    // TODO CGR: Data should actually be consumed
                                        sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_SAVE_SESSION_INFO:
                                    if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SAVE_SESSION_INFO");}
                                    // TODO CGR: Data should actually be consumed
                                    this->process_save_session_info(sdata.payload);
                                    break;
                                case PDUTYPE2_SET_ERROR_INFO_PDU:
                                    {
                                        if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");}
                                        uint32_t error_info = this->get_error_info_from_pdu(sdata.payload);
                                        this->process_error_info(error_info);
                                    }
                                    break;
                                case PDUTYPE2_SHUTDOWN_DENIED:
                                    //if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED");}
                                    LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED Received");
                                    break;

                                case PDUTYPE2_SET_KEYBOARD_INDICATORS:
                                    {
                                        if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SET_KEYBOARD_INDICATORS");}

                                        sdata.payload.in_skip_bytes(2); // UnitId(2)

                                        uint16_t LedFlags = sdata.payload.in_uint16_le();

                                        this->front.set_keyboard_indicators(LedFlags);

                                        assert(sdata.payload.get_current() == sdata.payload.get_data_end());
                                    }
                                    break;

                                default:
                                    LOG(LOG_WARNING, "PDUTYPE2 unsupported tag=%u", sdata.pdutype2);
                                    // TODO CGR: Data should actually be consumed
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                }
                            }
                            break;
                        }
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU:
                        {
                            if (bool(this->verbose & RDPVerbose::connection)){
                                 LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");
                            }

                            this->orders.reset();

// 2.2.1.13.1.1 Demand Active PDU Data (TS_DEMAND_ACTIVE_PDU)
// ==========================================================

//    shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1 ) containing information
//  about the packet. The type subfield of the pduType field of the Share Control Header MUST be set to
// PDUTYPE_DEMANDACTIVEPDU (1).

//    shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the packet (see [T128]
// section 8.4.2 for more information regarding share IDs).

                            this->share_id = sctrl.payload.in_uint32_le();

//    lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes of the sourceDescriptor
// field.
                            uint16_t lengthSourceDescriptor = sctrl.payload.in_uint16_le();

//    lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined size in bytes of the
// numberCapabilities, pad2Octets, and capabilitySets fields.

                            uint16_t lengthCombinedCapabilities = sctrl.payload.in_uint16_le();

//    sourceDescriptor (variable): A variable-length array of bytes containing a source descriptor (see
// [T128] section 8.4.1 for more information regarding source descriptors).

                            // TODO before skipping we should check we do not go outside current stream
                            sctrl.payload.in_skip_bytes(lengthSourceDescriptor);

// numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets included in the
// Demand Active PDU.

// pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number
//  of capability sets is specified by the numberCapabilities field.

                            this->process_server_caps(sctrl.payload, lengthCombinedCapabilities);


// sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.

                            uint32_t sessionId = sctrl.payload.in_uint32_le();

                            (void)sessionId;
                            this->send_confirm_active();
                            this->send_synchronise();
                            this->send_control(RDP_CTL_COOPERATE);
                            this->send_control(RDP_CTL_REQUEST_CONTROL);

                            /* Including RDP 5.0 capabilities */
                            if (this->use_rdp5){
                                LOG(LOG_INFO, "use rdp5");
                                if (this->enable_persistent_disk_bitmap_cache &&
                                    this->persist_bitmap_cache_on_disk) {
                                    if (!this->deactivation_reactivation_in_progress) {
                                        this->send_persistent_key_list();
                                    }
                                }
                                this->send_fonts(3);
                            }
                            else{
                                LOG(LOG_INFO, "not using rdp5");
                                this->send_fonts(1);
                                this->send_fonts(2);
                            }

                            this->send_input(0, RDP_INPUT_SYNCHRONIZE, 0,
                                (this->experimental_fix_input_event_sync ? (this->key_flags & 0x07) : 0), 0);

/*
                            LOG(LOG_INFO, "Resizing to %ux%ux%u", this->front_width, this->front_height, this->orders.bpp);
                            if (this->transparent_recorder) {
                                this->transparent_recorder->server_resize(this->front_width,
                                    this->front_height, this->orders.bpp);
                            }
                            if (-1 == this->front.server_resize(this->front_width, this->front_height, this->orders.bpp)){
                                LOG(LOG_ERR, "Resize not available on older clients,"
                                    " change client resolution to match server resolution");
                                throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
                            }
*/
                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                        }
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE_DEACTIVATEALLPDU"); }
                        LOG(LOG_INFO, "Deactivate All PDU");
                        this->deactivation_reactivation_in_progress = true;
                        // TODO CGR: Data should actually be consumed
                            // TODO CGR: Check we are indeed expecting Synchronize... dubious
                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        {
                            if (bool(this->verbose & RDPVerbose::connection)){
                                LOG(LOG_INFO, "PDUTYPE_SERVER_REDIR_PKT");
                            }
                            sctrl.payload.in_skip_bytes(2);
                            ServerRedirectionPDU server_redirect;
                            server_redirect.receive(sctrl.payload);
                            sctrl.payload.in_skip_bytes(1);
                            server_redirect.export_to_redirection_info(this->redir_info);
                            this->server_redirection_packet_received = true;
                            if (bool(this->verbose & RDPVerbose::connection)){
                                server_redirect.log(LOG_INFO, "Got Packet");
                                this->redir_info.log(LOG_INFO, "RInfo Ini");
                            }
                            if (!server_redirect.Noredirect()) {
                                LOG(LOG_ERR, "Server Redirection thrown");
                                throw Error(ERR_RDP_SERVER_REDIR);
                            }
                        }
                        break;
                    default:
                        LOG(LOG_INFO, "unknown PDU %u", sctrl.pduType);
                        break;
                    }
                // TODO check sctrl.payload is completely consumed

                }
            }
        }
    }

    TpduBuffer buf;

    void draw_event(time_t now, gdi::GraphicApi & drawable_) override
    {
        //LOG(LOG_INFO, "mod_rdp::draw_event()");

        if (this->remote_programs_session_manager) {
            this->remote_programs_session_manager->set_drawable(&drawable_);
        }

        if (AsynchronousGraphicTask::none != this->asynchronous_graphic_task) {
            this->perform_asynchronous_graphic_task(drawable_);
            return;
        }

        bool run = true;
        bool waked_up_by_time = this->event.is_waked_up_by_time();

        if (this->state == MOD_RDP_NEGO_INITIATE) {
            LOG(LOG_INFO, "RdpNego::NEGO_STATE_INITIAL");
            this->nego.send_negotiation_request(this->trans);
            this->event.reset_trigger_time();
            this->state = MOD_RDP_NEGO;
            run = false;
        }
        else if (this->state == MOD_RDP_NEGO && !waked_up_by_time) {
            run = this->nego.recv_next_data(this->buf, this->trans, RdpNego::ServerCert{
                this->server_cert_store,
                this->server_cert_check,
                this->certif_path.get(),
                this->server_notifier
            });
            if (not run) {
                this->send_connectInitialPDUwithGccConferenceCreateRequest();
            }
        }
        else if (!waked_up_by_time) {
            this->buf.load_data(this->trans);
        }

        if (run && !waked_up_by_time) {
            while (this->buf.next_pdu()) {
                InStream x224_data(this->buf.current_pdu_buffer());

                try{
                    //LOG(LOG_INFO, "mod_rdp::draw_event() state switch");
                    switch (this->state){
                    case MOD_RDP_NEGO_INITIATE:
                    case MOD_RDP_NEGO:
                        assert(false);
                        break;

                    case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
                        this->basic_settings_exchange(x224_data);
                        break;

                    case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
                        this->channel_connection_attach_user(x224_data);
                        break;

                    case MOD_RDP_CHANNEL_JOIN_CONFIRME:
                        this->channel_join_confirme(now, x224_data);
                        break;

                    case MOD_RDP_GET_LICENSE:
                        this->get_license(x224_data);
                        break;

                    case MOD_RDP_CONNECTED:
                        gdi::GraphicApi & drawable =
                            ( this->remote_programs_session_manager
                            ? (*this->remote_programs_session_manager)
                            : ( this->graphics_update_disabled
                                ? gdi::null_gd()
                                : drawable_
                            ));
                        if (this->buf.current_pdu_is_fast_path()) {
                            this->connected_fast_path(drawable, this->buf.current_pdu_buffer());
                        }
                        else {
                            this->connected_slow_path(now, drawable, x224_data);
                        }
                        break;
                    }
                }
                catch(Error const & e){
                    LOG(LOG_INFO, "mod_rdp::draw_event() state switch raised exception");

                    if (e.id == ERR_RDP_SERVER_REDIR) {
                        this->front.must_be_stop_capture();
                        throw;
                    }

                    if (this->session_probe_virtual_channel_p &&
                        this->session_probe_virtual_channel_p->is_disconnection_reconnection_required()) {
                        throw Error(ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION);
                    }
                    else {
                        this->front.must_be_stop_capture();
                    }

                    if (this->remote_apps_not_enabled) {
                        throw Error(ERR_RAIL_NOT_ENABLED);
                    }

                    if (e.id != ERR_MCS_APPID_IS_MCS_DPUM)
                    {
                        char const* reason =
                            ((UP_AND_RUNNING == this->connection_finalization_state) ?
                            "SESSION_EXCEPTION" : "SESSION_EXCEPTION_NO_RECORD");

                        this->report_message.report(reason, e.errmsg());

                        this->end_session_reason.clear();
                        this->end_session_message.clear();
                    }

                    if ((e.id == ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED) ||
                        (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_MISSED) ||
                        (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED) ||
                        (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE) ||
                        (e.id == ERR_NLA_AUTHENTICATION_FAILED)) {
                        throw;
                    }

                    StaticOutStream<256> stream;
                    X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
                    try {
                        this->trans.send(stream.get_data(), stream.get_offset());
                        LOG(LOG_INFO, "Connection to server closed");
                    }
                    catch(Error const & e){
                        LOG(LOG_INFO, "Connection to server Already closed: error=%u", e.id);
                    };

                    this->event.signal = BACK_EVENT_NEXT;

                    if (this->enable_session_probe) {
                        const bool disable_input_event     = false;
                        const bool disable_graphics_update = false;
                        this->disable_input_event_and_graphics_update(
                            disable_input_event, disable_graphics_update);
                    }

                    if ((e.id == ERR_LIC) ||
                        (e.id == ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT) ||
                        (e.id == ERR_RAIL_CLIENT_EXECUTE) ||
                        (e.id == ERR_RAIL_STARTING_PROGRAM) ||
                        (e.id == ERR_RAIL_UNAUTHORIZED_PROGRAM) ||

                        (e.id == ERR_SESSION_PROBE_ASBL_FSVC_UNAVAILABLE) ||
                        (e.id == ERR_SESSION_PROBE_ASBL_MAYBE_SOMETHING_BLOCKS) ||
                        (e.id == ERR_SESSION_PROBE_ASBL_UNKNOWN_REASON) ||
                        (e.id == ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE) ||
                        (e.id == ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE) ||
                        (e.id == ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET) ||
                        (e.id == ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS) ||
                        (e.id == ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED) ||
                        (e.id == ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG) ||
                        (e.id == ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG) ||

                        (e.id == ERR_SESSION_PROBE_LAUNCH)) {
                        throw;
                    }

                    if (UP_AND_RUNNING != this->connection_finalization_state &&
                        !this->already_upped_and_running) {
                        const char * statestr = "UNKNOWN_STATE";
                        const char * statedescr = "Unknow state.";
                        switch (this->state) {
                            #define CASE(e, trkey)                                         \
                                case e:                                                    \
                                    statestr = #e + 4; statedescr = TR(trkey, this->lang); \
                                    this->close_box_extra_message_ref += " ";              \
                                    this->close_box_extra_message_ref += statedescr;       \
                                    this->close_box_extra_message_ref += " (";             \
                                    this->close_box_extra_message_ref += statestr;         \
                                    this->close_box_extra_message_ref += ")";              \
                                break
                            CASE(MOD_RDP_NEGO_INITIATE, trkeys::err_mod_rdp_nego);
                            CASE(MOD_RDP_NEGO, trkeys::err_mod_rdp_nego);
                            CASE(MOD_RDP_BASIC_SETTINGS_EXCHANGE, trkeys::err_mod_rdp_basic_settings_exchange);
                            CASE(MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER, trkeys::err_mod_rdp_channel_connection_attach_user);
                            CASE(MOD_RDP_CHANNEL_JOIN_CONFIRME, trkeys::mod_rdp_channel_join_confirme);
                            CASE(MOD_RDP_GET_LICENSE, trkeys::mod_rdp_get_license);
                            CASE(MOD_RDP_CONNECTED, trkeys::err_mod_rdp_connected);
                            #undef CASE
                        }

                        LOG(LOG_ERR, "Creation of new mod 'RDP' failed at %s state. %s",
                            statestr, statedescr);
                        throw Error(ERR_SESSION_UNKNOWN_BACKEND);
                    }
                }
            }
        }

        //LOG(LOG_INFO, "mod_rdp::draw_event() session timeout check count=%u",
        //        static_cast<unsigned>(this->open_session_timeout.count()));
        if (this->open_session_timeout.count()) {
            LOG(LOG_INFO, "mod_rdp::draw_event() session timeout check switch");
            switch(this->open_session_timeout_checker.check(now)) {
            case Timeout::TIMEOUT_REACHED:
                LOG(LOG_INFO, "mod_rdp::draw_event() Timeout::TIMEOUT_REACHED");
                if (this->error_message) {
                    *this->error_message = "Logon timer expired!";
                }

                this->report_message.report("CONNECTION_FAILED", "Logon timer expired.");

                if (this->enable_session_probe) {
                    const bool disable_input_event     = false;
                    const bool disable_graphics_update = false;
                    this->disable_input_event_and_graphics_update(
                        disable_input_event, disable_graphics_update);
                }

                LOG(LOG_ERR,
                    "Logon timer expired on %s. The session will be disconnected.",
                    this->hostname);
                throw Error(ERR_RDP_OPEN_SESSION_TIMEOUT);
            break;
            case Timeout::TIMEOUT_NOT_REACHED:
                LOG(LOG_INFO, "mod_rdp::draw_event() Timeout::TIMEOUT_NOT_REACHED");
                this->event.set_trigger_time(1000000);
            break;
            case Timeout::TIMEOUT_INACTIVE:
                LOG(LOG_INFO, "mod_rdp::draw_event() Timeout::TIMEOUT_INACTIVE");
            break;
            }
        }

/*
        //LOG(LOG_INFO, "mod_rdp::draw_event() session_probe_virtual_channel_p");
        try{
            if (this->session_probe_virtual_channel_p) {
                this->session_probe_virtual_channel_p->process_event();
            }
        }
        catch (Error const & e) {
            if (e.id != ERR_SESSION_PROBE_ENDING_IN_PROGRESS)
                throw;

            this->end_session_reason.clear();
            this->end_session_message.clear();

            this->authentifier.disconnect_target();
            this->authentifier.set_auth_error_message(TR(trkeys::session_logoff_in_progress, this->lang));

            this->event.signal = BACK_EVENT_NEXT;
        }
*/
        //LOG(LOG_INFO, "mod_rdp::draw_event() done");
    }   // draw_event

    // 1.3.1.3 Deactivation-Reactivation Sequence
    // ==========================================

    // After the connection sequence has run to completion, the server may determine
    // that the client needs to be connected to a waiting, disconnected session. To
    // accomplish this task the server signals the client with a Deactivate All PDU.
    // A Deactivate All PDU implies that the connection will be dropped or that a
    // capability renegotiation will occur. If a capability renegotiation needs to
    // be performed then the server will re-execute the connection sequence,
    // starting with the Demand Active PDU (the Capability Negotiation and
    // Connection Finalization phases as described in section 1.3.1.1) but excluding
    // the Persistent Key List PDU.


    // 2.2.1.13.1.1 Demand Active PDU Data (TS_DEMAND_ACTIVE_PDU)
    // ==========================================================
    // The TS_DEMAND_ACTIVE_PDU structure is a standard T.128 Demand Active PDU (see [T128] section 8.4.1).

    // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1) containing information about the packet. The type subfield of the pduType field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).

    // shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for the packet (see [T128] section 8.4.2 for more information regarding share IDs).

    // lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size in bytes of the sourceDescriptor field.

    // lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer. The combined size in bytes of the numberCapabilities, pad2Octets, and capabilitySets fields.

    // sourceDescriptor (variable): A variable-length array of bytes containing a source descriptor (see [T128] section 8.4.1 for more information regarding source descriptors).

    // numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets include " in the Demand Active PDU.

    // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

    // capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number of capability sets is specified by the numberCapabilities field.

    // sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.

    void send_confirm_active() {
        if (bool(this->verbose & RDPVerbose::capabilities)){
            LOG(LOG_INFO, "mod_rdp::send_confirm_active");
        }
        this->send_data_request_ex(
            GCC::MCS_GLOBAL_CHANNEL,
            [this](StreamSize<65536>, OutStream & stream) {
                RDP::ConfirmActivePDU_Send confirm_active_pdu(stream);

                confirm_active_pdu.emit_begin(this->share_id);

                GeneralCaps general_caps;
                general_caps.extraflags  =
                    this->use_rdp5
                    ? NO_BITMAP_COMPRESSION_HDR | AUTORECONNECT_SUPPORTED | LONG_CREDENTIALS_SUPPORTED
                    : 0
                    ;
                // Slow/Fast-path
                general_caps.extraflags |=
                    this->enable_fastpath_server_update
                    ? FASTPATH_OUTPUT_SUPPORTED
                    : 0
                    ;
                if (this->enable_transparent_mode) {
                    general_caps = this->client_general_caps;
                }
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    general_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(general_caps);

                BitmapCaps bitmap_caps;
                // TODO Client SHOULD set this field to the color depth requested in the Client Core Data
                bitmap_caps.preferredBitsPerPixel = this->orders.bpp;
                //bitmap_caps.preferredBitsPerPixel = this->front_bpp;
                bitmap_caps.desktopWidth          = this->front_width;
                bitmap_caps.desktopHeight         = this->front_height;
                bitmap_caps.bitmapCompressionFlag = 0x0001; // This field MUST be set to TRUE (0x0001).
                //bitmap_caps.drawingFlags = DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY | DRAW_ALLOW_COLOR_SUBSAMPLING | DRAW_ALLOW_SKIP_ALPHA;
                bitmap_caps.drawingFlags = DRAW_ALLOW_SKIP_ALPHA;
                if (this->enable_transparent_mode) {
                    bitmap_caps = this->client_bitmap_caps;
                }
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    bitmap_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(bitmap_caps);

                OrderCaps order_caps;
                order_caps.numberFonts                                   = 0;
                order_caps.orderFlags                                    = /*0x2a*/
                                                                            NEGOTIATEORDERSUPPORT   /* 0x02 */
                                                                        | ZEROBOUNDSDELTASSUPPORT /* 0x08 */
                                                                        | COLORINDEXSUPPORT       /* 0x20 */
                                                                        | ORDERFLAGS_EXTRA_FLAGS  /* 0x80 */
                                                                        ;
                order_caps.orderSupport[TS_NEG_DSTBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX]        = (this->enable_multidstblt     ? 1 : 0);
                order_caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]    = (this->enable_multiopaquerect ? 1 : 0);
                order_caps.orderSupport[TS_NEG_MULTIPATBLT_INDEX]        = (this->enable_multipatblt     ? 1 : 0);
                order_caps.orderSupport[TS_NEG_MULTISCRBLT_INDEX]        = (this->enable_multiscrblt     ? 1 : 0);
                order_caps.orderSupport[TS_NEG_PATBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_SCRBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX]            = (this->enable_mem3blt         ? 1 : 0);
                order_caps.orderSupport[TS_NEG_LINETO_INDEX]             = 1;
                order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = 0;
                order_caps.orderSupport[UnusedIndex3]                    = 1;
                order_caps.orderSupport[UnusedIndex5]                    = 1;
                order_caps.orderSupport[TS_NEG_POLYGON_SC_INDEX]         = (this->enable_polygonsc       ? 1 : 0);
                order_caps.orderSupport[TS_NEG_POLYGON_CB_INDEX]         = (this->enable_polygoncb       ? 1 : 0);
                order_caps.orderSupport[TS_NEG_POLYLINE_INDEX]           = (this->enable_polyline        ? 1 : 0);
                //order_caps.orderSupport[TS_NEG_FAST_GLYPH_INDEX]         = 1;
                order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]         = (this->enable_ellipsesc       ? 1 : 0);
                order_caps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX]         = (this->enable_ellipsecb       ? 1 : 0);
                order_caps.orderSupport[TS_NEG_INDEX_INDEX]              = 1;
                order_caps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX] = (this->enable_ninegrid_bitmap ? 1 : 0);
                order_caps.textFlags                                     = 0x06a1;
                order_caps.orderSupportExFlags                           = ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT;
                order_caps.textANSICodePage                              = 0x4e4; // Windows-1252 codepage is passed (latin-1)

                // Apparently, these primary drawing orders are supported
                // by both rdesktop and xfreerdp :
                // TS_NEG_DSTBLT_INDEX
                // TS_NEG_PATBLT_INDEX
                // TS_NEG_SCRBLT_INDEX
                // TS_NEG_MEMBLT_INDEX
                // TS_NEG_LINETO_INDEX
                // others orders may not be supported.

                // intersect with client order capabilities
                // which may not be supported by clients.

                enum OrdersIndexes idxs[] = {
                      TS_NEG_DSTBLT_INDEX
                    , TS_NEG_PATBLT_INDEX
                    , TS_NEG_SCRBLT_INDEX
                    , TS_NEG_MEMBLT_INDEX
                    , TS_NEG_MEM3BLT_INDEX
                    , TS_NEG_DRAWNINEGRID_INDEX
                    , TS_NEG_LINETO_INDEX
//                    , TS_NEG_MULTI_DRAWNINEGRID_INDEX
//                    , TS_NEG_SAVEBITMAP_INDEX
                    , TS_NEG_MULTIDSTBLT_INDEX
                    , TS_NEG_MULTIPATBLT_INDEX
                    , TS_NEG_MULTISCRBLT_INDEX
                    , TS_NEG_MULTIOPAQUERECT_INDEX
//                    , TS_NEG_FAST_INDEX_INDEX
                    , TS_NEG_POLYGON_SC_INDEX
                    , TS_NEG_POLYGON_CB_INDEX
                    , TS_NEG_POLYLINE_INDEX
//                    , TS_NEG_FAST_GLYPH_INDEX
                    , TS_NEG_ELLIPSE_SC_INDEX
                    , TS_NEG_ELLIPSE_CB_INDEX
                    , TS_NEG_INDEX_INDEX
                };

                for (auto idx : idxs){
                    order_caps.orderSupport[idx] &= this->client_order_caps.orderSupport[idx];
                }

                if (bool(this->verbose & RDPVerbose::capabilities) && !order_caps.orderSupport[TS_NEG_MEMBLT_INDEX]) {
                    LOG(LOG_INFO, "MemBlt Primary Drawing Order is disabled.");
                }

                order_caps.orderSupportExFlags &= this->client_order_caps.orderSupportExFlags;

                // LOG(LOG_INFO, ">>>>>>>>ORDER CAPABILITIES : ELLIPSE : %d",
                //     order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX]);
                if (this->enable_transparent_mode) {
                    order_caps = this->client_order_caps;
                }
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    order_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(order_caps);

                BmpCacheCaps bmpcache_caps;
                bmpcache_caps.cache0Entries         = 0x258;
                bmpcache_caps.cache0MaximumCellSize = nbbytes(this->orders.bpp) * 0x100;
                bmpcache_caps.cache1Entries         = 0x12c;
                bmpcache_caps.cache1MaximumCellSize = nbbytes(this->orders.bpp) * 0x400;
                bmpcache_caps.cache2Entries         = 0x106;
                bmpcache_caps.cache2MaximumCellSize = nbbytes(this->orders.bpp) * 0x1000;

                BmpCache2Caps bmpcache2_caps;
                bmpcache2_caps.cacheFlags           = PERSISTENT_KEYS_EXPECTED_FLAG | (this->enable_cache_waiting_list ? ALLOW_CACHE_WAITING_LIST_FLAG : 0);
                bmpcache2_caps.numCellCaches        = 3;
                bmpcache2_caps.bitmapCache0CellInfo = this->BmpCacheRev2_Cache_NumEntries()[0];
                bmpcache2_caps.bitmapCache1CellInfo = this->BmpCacheRev2_Cache_NumEntries()[1];
                bmpcache2_caps.bitmapCache2CellInfo = (this->BmpCacheRev2_Cache_NumEntries()[2] | 0x80000000);

                bool use_bitmapcache_rev2 = false;

                if (this->enable_transparent_mode) {
                    use_bitmapcache_rev2 = this->client_use_bmp_cache_2;
                    if (use_bitmapcache_rev2) {
                        bmpcache2_caps = this->client_bmp_cache_2_caps;
                    }
                    else {
                        bmpcache_caps = this->client_bmp_cache_caps;
                    }
                }
                else {
                    use_bitmapcache_rev2 = this->enable_persistent_disk_bitmap_cache;
                }

                if (use_bitmapcache_rev2) {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bmpcache2_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(bmpcache2_caps);

                    if (!this->enable_transparent_mode && !this->deactivation_reactivation_in_progress) {
                        this->orders.create_cache_bitmap(
                            this->BmpCacheRev2_Cache_NumEntries()[0], nbbytes(this->orders.bpp) * 16 * 16, false,
                            this->BmpCacheRev2_Cache_NumEntries()[1], nbbytes(this->orders.bpp) * 32 * 32, false,
                            this->BmpCacheRev2_Cache_NumEntries()[2], nbbytes(this->orders.bpp) * 64 * 64, this->enable_persistent_disk_bitmap_cache,
                            this->enable_cache_waiting_list,
                            this->cache_verbose);
                    }
                }
                else {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bmpcache_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(bmpcache_caps);

                    if (!this->enable_transparent_mode && !this->deactivation_reactivation_in_progress) {
                        this->orders.create_cache_bitmap(
                            0x258, nbbytes(this->orders.bpp) * 0x100,   false,
                            0x12c, nbbytes(this->orders.bpp) * 0x400,   false,
                            0x106, nbbytes(this->orders.bpp) * 0x1000,  false,
                            false,
                            this->cache_verbose);
                    }
                }

                ColorCacheCaps colorcache_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    colorcache_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(colorcache_caps);

                ActivationCaps activation_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    activation_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(activation_caps);

                ControlCaps control_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    control_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(control_caps);

                PointerCaps pointer_caps;
                pointer_caps.len                       = 10;
                if (this->enable_new_pointer == false) {
                    pointer_caps.pointerCacheSize      = 0;
                    pointer_caps.colorPointerCacheSize = 20;
                    pointer_caps.len                   = 8;
                    assert(pointer_caps.colorPointerCacheSize <= sizeof(this->cursors) / sizeof(Pointer));
                }
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    pointer_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(pointer_caps);

                ShareCaps share_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    share_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(share_caps);

                InputCaps input_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    input_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(input_caps);

                SoundCaps sound_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    sound_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(sound_caps);

                FontCaps font_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    font_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(font_caps);

                GlyphCacheCaps glyphcache_caps;
                if (this->enable_glyph_cache) {
                    glyphcache_caps = this->client_glyph_cache_caps;

                    glyphcache_caps.FragCache         = 0;  // Not yet supported
                    if (glyphcache_caps.GlyphSupportLevel != GlyphCacheCaps::GLYPH_SUPPORT_NONE) {
                        glyphcache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_PARTIAL;
                    }
                }
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    glyphcache_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(glyphcache_caps);

                if (this->remote_program) {
                    RailCaps rail_caps = this->client_rail_caps;
                    rail_caps.RailSupportLevel &= (TS_RAIL_LEVEL_SUPPORTED | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED | TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        rail_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(rail_caps);

                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        this->client_window_list_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(this->client_window_list_caps);
                }

                if (this->large_pointer_support &&
                    this->client_large_pointer_caps.largePointerSupportFlags) {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        this->client_large_pointer_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(this->client_large_pointer_caps);

                    if (this->client_multi_fragment_update_caps.MaxRequestSize) {
                        MultiFragmentUpdateCaps multi_fragment_update_caps;

                        multi_fragment_update_caps = this->client_multi_fragment_update_caps;

                        if (multi_fragment_update_caps.MaxRequestSize > this->multifragment_update_data.get_capacity()) {
                            multi_fragment_update_caps.MaxRequestSize = this->multifragment_update_data.get_capacity();
                        }
                        if (bool(this->verbose & RDPVerbose::capabilities)) {
                            multi_fragment_update_caps.log("Sending to server");
                        }
                        confirm_active_pdu.emit_capability_set(multi_fragment_update_caps);
                    }
                }

                if (this->enable_ninegrid_bitmap) {
                    DrawNineGridCacheCaps ninegrid_caps(DRAW_NINEGRID_SUPPORTED, 0xffff, 256);
                    confirm_active_pdu.emit_capability_set(ninegrid_caps);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        ninegrid_caps.log("Sending to server");
                    }
                }

                confirm_active_pdu.emit_end();
            },
            [this](StreamSize<256>, OutStream & sctrl_header, std::size_t packet_size) {
                // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
                // containing information about the packet. The type subfield of the pduType
                // field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).
                ShareControl_Send(sctrl_header, PDUTYPE_CONFIRMACTIVEPDU,
                    this->userid + GCC::MCS_USERCHANNEL_BASE, packet_size);
            }
        );

        if (bool(this->verbose & RDPVerbose::capabilities)){
            LOG(LOG_INFO, "mod_rdp::send_confirm_active done");
            LOG(LOG_INFO, "Waiting for answer to confirm active");
        }
    }   // send_confirm_active


// 3.2.5.9.2 Processing Slow-Path Pointer Update PDU
// =================================================

// The structure and fields of the Slow-Path Pointer Update PDU are specified in section 2.2.9.1.1.4,
// and the techniques specified in section 3.2.5.9.2 demonstrate how to process the contents of the PDU.
// The messageType field contains an identifier that describes the type of Pointer Update data (see
// section 2.2.9.1.1.4 for a list of possible values) present in the pointerAttributeData field:

// Pointer Position Update (section 2.2.9.1.1.4.2)
// System Pointer Update (section 2.2.9.1.1.4.3)
// Color Pointer Update (section 2.2.9.1.1.4.4)
// New Pointer Update (section 2.2.9.1.1.4.5)
// Cached Pointer Update (section 2.2.9.1.1.4.6)

// If a slow-path update structure is received which does not match one of the known types, the client
// SHOULD ignore the data in the update.

// Once this PDU has been processed, the client MUST carry out any operations necessary to update the
// local pointer position (in the case of the Position Update) or change the shape (in the case of the
// System, Color, New, and Cached Pointer Updates). In the case of the Color and New Pointer Updates
// the new pointer image MUST also be stored in the Pointer Image Cache (section 3.2.1.11), in the slot
// specified by the cacheIndex field. This necessary step ensures that the client is able to correctly
// process future Cached Pointer Updates.


    void process_pointer_pdu(InStream & stream, gdi::GraphicApi & drawable)
    {
        if (bool(this->verbose & RDPVerbose::graphics_pointer)){
            LOG(LOG_INFO, "mod_rdp::process_pointer_pdu");
        }

        int message_type = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */
        switch (message_type) {
        // Cached Pointer Update (section 2.2.9.1.1.4.6)
        case RDP_POINTER_CACHED:
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer cached");
            }
            this->process_cached_pointer_pdu(stream, drawable);
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer cached done");
            }
            break;
        // Color Pointer Update (section 2.2.9.1.1.4.4)
        case RDP_POINTER_COLOR:
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer color");
            }
            this->process_color_pointer_pdu(stream, drawable);
            if (bool(this->verbose & RDPVerbose::graphics_pointer)){
                LOG(LOG_INFO, "Process pointer color done");
            }
            break;
        // New Pointer Update (section 2.2.9.1.1.4.5)
        case RDP_POINTER_NEW:
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer new");
            }
            if (enable_new_pointer) {
                this->process_new_pointer_pdu(stream, drawable); // Pointer with arbitrary color depth
            }
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer new done");
            }
            break;
        // System Pointer Update (section 2.2.9.1.1.4.3)
        case RDP_POINTER_SYSTEM:
        {
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer system");
            }
            // TODO: actually show mouse cursor or get back to default
            int system_pointer_type = stream.in_uint32_le();
            this->process_system_pointer_pdu(system_pointer_type, drawable);
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer system done");
            }
        }
        break;
        // Pointer Position Update (section 2.2.9.1.1.4.2)

        // [ referenced from 3.2.5.9.2 Processing Slow-Path Pointer Update PDU]
        // 2.2.9.1.1.4.2 Pointer Position Update (TS_POINTERPOSATTRIBUTE)
        // ==============================================================

        // The TS_POINTERPOSATTRIBUTE structure is used to indicate that
        // the client pointer MUST be moved to the specified position
        // relative to the top-left corner of the server's desktop ([T128]
        // section 8.14.4).

        // position (4 bytes): Point (section 2.2.9.1.1.4.1) structure
        // containing the new x-coordinates and y-coordinates of the pointer.
        case RDP_POINTER_MOVE:
            {
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer position");
                }

                const unsigned expected = 4; /* xPos(2) + yPos(2) */
                if (!stream.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated Pointer Position Update, need=%u remains=%zu",
                        expected, stream.in_remain());
                    //throw Error(ERR_RDP_DATA_TRUNCATED);
                    break;
                }

                uint16_t xPos = stream.in_uint16_le();
                uint16_t yPos = stream.in_uint16_le();
                this->front.update_pointer_position(xPos, yPos);
            }
            break;
        default:
            break;
        }
        if (bool(this->verbose & RDPVerbose::graphics_pointer)){
            LOG(LOG_INFO, "mod_rdp::process_pointer_pdu done");
        }
    }

    void process_palette(InStream & stream, bool fast_path) {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "mod_rdp::process_palette");
        }

        RDP::UpdatePaletteData_Recv(stream, fast_path, this->orders.global_palette);
        this->front.set_palette(this->orders.global_palette);

        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "mod_rdp::process_palette done");
        }
    }

    // 2.2.5.1.1 Set Error Info PDU Data (TS_SET_ERROR_INFO_PDU)
    // =========================================================
    // The TS_SET_ERROR_INFO_PDU structure contains the contents of the Set Error
    // Info PDU, which is a Share Data Header (section 2.2.8.1.1.1.2) with an error
    // value field.

    // shareDataHeader (18 bytes): Share Data Header containing information about
    // the packet. The type subfield of the pduType field of the Share Control
    // Header (section 2.2.8.1.1.1.1) MUST be set to PDUTYPE_DATAPDU (7). The
    // pduType2 field of the Share Data Header MUST be set to
    // PDUTYPE2_SET_ERROR_INFO_PDU (47), and the pduSource field MUST be set to 0.

    // errorInfo (4 bytes): A 32-bit, unsigned integer. Error code.

    // Protocol-independent codes:
    // +---------------------------------------------+-----------------------------+
    // | 0x00000001 ERRINFO_RPC_INITIATED_DISCONNECT | The disconnection was       |
    // |                                             | initiated by an             |
    // |                                             | administrative tool on the  |
    // |                                             | server in another session.  |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000002 ERRINFO_RPC_INITIATED_LOGOFF     | The disconnection was due   |
    // |                                             | to a forced logoff initiated|
    // |                                             | by an administrative tool   |
    // |                                             | on the server in another    |
    // |                                             | session.                    |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000003 ERRINFO_IDLE_TIMEOUT             | The idle session limit timer|
    // |                                             | on the server has elapsed.  |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000004 ERRINFO_LOGON_TIMEOUT            | The active session limit    |
    // |                                             | timer on the server has     |
    // |                                             | elapsed.                    |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000005                                  | Another user connected to   |
    // | ERRINFO_DISCONNECTED_BY_OTHERCONNECTION     | the server, forcing the     |
    // |                                             | disconnection of the current|
    // |                                             | connection.                 |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000006 ERRINFO_OUT_OF_MEMORY            | The server ran out of       |
    // |                                             | available memory resources. |
    // +---------------------------------------------+-----------------------------+
    // | 0x00000007 ERRINFO_SERVER_DENIED_CONNECTION | The server denied the       |
    // |                                             | connection.                 |
    // +---------------------------------------------+-----+-----------------------+
    // | 0x00000009                                  | The user cannot connect to  |
    // | ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES      | the server due to           |
    // |                                             | insufficient access         |
    // |                                             | privileges.                 |
    // +---------------------------------------------+-----------------------------+
    // | 0x0000000A                                  | The server does not accept  |
    // | ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED   | saved user credentials and  |
    // |                                             | requires that the user enter|
    // |                                             | their credentials for each  |
    // |                                             | connection.                 |
    // +-----------------------------------------+---+-----------------------------+
    // | 0x0000000B                              | The disconnection was initiated |
    // | ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER | by an administrative tool on    |
    // |                                         | the server running in the user's|
    // |                                         | session.                        |
    // +-----------------------------------------+---------------------------------+
    // | 0x0000000C ERRINFO_LOGOFF_BY_USER       | The disconnection was initiated |
    // |                                         | by the user logging off his or  |
    // |                                         | her session on the server.      |
    // +-----------------------------------------+---------------------------------+

    // Protocol-independent licensing codes:
    // +-------------------------------------------+-------------------------------+
    // | 0x00000100 ERRINFO_LICENSE_INTERNAL       | An internal error has occurred|
    // |                                           | in the Terminal Services      |
    // |                                           | licensing component.          |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000101                                | A Remote Desktop License      |
    // | ERRINFO_LICENSE_NO_LICENSE_SERVER         | Server ([MS-RDPELE] section   |
    // |                                           | 1.1) could not be found to    |
    // |                                           | provide a license.            |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000102 ERRINFO_LICENSE_NO_LICENSE     | There are no Client Access    |
    // |                                           | Licenses ([MS-RDPELE] section |
    // |                                           | 1.1) available for the target |
    // |                                           | remote computer.              |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000103 ERRINFO_LICENSE_BAD_CLIENT_MSG | The remote computer received  |
    // |                                           | an invalid licensing message  |
    // |                                           | from the client.              |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000104                                | The Client Access License     |
    // | ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE | ([MS-RDPELE] section 1.1)     |
    // |                                           | stored by the client has been |
    // |                                           |  modified.                    |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000105                                | The Client Access License     |
    // | ERRINFO_LICENSE_BAD_CLIENT_LICENSE        | ([MS-RDPELE] section 1.1)     |
    // |                                           | stored by the client is in an |
    // |                                           | invalid format.               |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000106                                | Network problems have caused  |
    // | ERRINFO_LICENSE_CANT_FINISH_PROTOCOL      | the licensing protocol        |
    // |                                           | ([MS-RDPELE] section 1.3.3)   |
    // |                                           | to be terminated.             |
    // +-------------------------------------------+-------------------------------+
    // | 0x00000107                                | The client prematurely ended  |
    // | ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL     | the licensing protocol        |
    // |                                           | ([MS-RDPELE] section 1.3.3).  |
    // +---------------------------------------+---+-------------------------------+
    // | 0x00000108                            | A licensing message ([MS-RDPELE]  |
    // | ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION | sections 2.2 and 5.1) was         |
    // |                                       | incorrectly encrypted.            |
    // +---------------------------------------+-----------------------------------+
    // | 0x00000109                            | The Client Access License         |
    // | ERRINFO_LICENSE_CANT_UPGRADE_LICENSE  | ([MS-RDPELE] section 1.1) stored  |
    // |                                       | by the client could not be        |
    // |                                       | upgraded or renewed.              |
    // +---------------------------------------+-----------------------------------+
    // | 0x0000010A                            | The remote computer is not        |
    // | ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS | licensed to accept remote         |
    // |                                       |  connections.                     |
    // +---------------------------------------+-----------------------------------+

    // Protocol-independent codes generated by Connection Broker:
    // +----------------------------------------------+----------------------------+
    // | Value                                        | Meaning                    |
    // +----------------------------------------------+----------------------------+
    // | 0x0000400                                    | The target endpoint could  |
    // | ERRINFO_CB_DESTINATION_NOT_FOUND             | not be found.              |
    // +----------------------------------------------+----------------------------+
    // | 0x0000402                                    | The target endpoint to     |
    // | ERRINFO_CB_LOADING_DESTINATION               | which the client is being  |
    // |                                              | redirected is              |
    // |                                              | disconnecting from the     |
    // |                                              | Connection Broker.         |
    // +----------------------------------------------+----------------------------+
    // | 0x0000404                                    | An error occurred while    |
    // | ERRINFO_CB_REDIRECTING_TO_DESTINATION        | the connection was being   |
    // |                                              | redirected to the target   |
    // |                                              | endpoint.                  |
    // +----------------------------------------------+----------------------------+
    // | 0x0000405                                    | An error occurred while    |
    // | ERRINFO_CB_SESSION_ONLINE_VM_WAKE            | the target endpoint (a     |
    // |                                              | virtual machine) was being |
    // |                                              | awakened.                  |
    // +----------------------------------------------+----------------------------+
    // | 0x0000406                                    | An error occurred while    |
    // | ERRINFO_CB_SESSION_ONLINE_VM_BOOT            | the target endpoint (a     |
    // |                                              | virtual machine) was being |
    // |                                              | started.                   |
    // +----------------------------------------------+----------------------------+
    // | 0x0000407                                    | The IP address of the      |
    // | ERRINFO_CB_SESSION_ONLINE_VM_NO_DNS          | target endpoint (a virtual |
    // |                                              | machine) cannot be         |
    // |                                              | determined.                |
    // +----------------------------------------------+----------------------------+
    // | 0x0000408                                    | There are no available     |
    // | ERRINFO_CB_DESTINATION_POOL_NOT_FREE         | endpoints in the pool      |
    // |                                              | managed by the Connection  |
    // |                                              | Broker.                    |
    // +----------------------------------------------+----------------------------+
    // | 0x0000409                                    | Processing of the          |
    // | ERRINFO_CB_CONNECTION_CANCELLED              | connection has been        |
    // |                                              | cancelled.                 |
    // +----------------------------------------------+----------------------------+
    // | 0x0000410                                    | The settings contained in  |
    // | ERRINFO_CB_CONNECTION_ERROR_INVALID_SETTINGS | the routingToken field of  |
    // |                                              | the X.224 Connection       |
    // |                                              | Request PDU (section       |
    // |                                              | 2.2.1.1) cannot be         |
    // |                                              | validated.                 |
    // +----------------------------------------------+----------------------------+
    // | 0x0000411                                    | A time-out occurred while  |
    // | ERRINFO_CB_SESSION_ONLINE_VM_BOOT_TIMEOUT    | the target endpoint (a     |
    // |                                              | virtual machine) was being |
    // |                                              | started.                   |
    // +----------------------------------------------+----------------------------+
    // | 0x0000412                                    | A session monitoring error |
    // | ERRINFO_CB_SESSION_ONLINE_VM_SESSMON_FAILED  | occurred while the target  |
    // |                                              | endpoint (a virtual        |
    // |                                              | machine) was being         |
    // |                                              | started.                   |
    // +----------------------------------------------+----------------------------+

    // RDP specific codes:
    // +------------------------------------+--------------------------------------+
    // | 0x000010C9 ERRINFO_UNKNOWNPDUTYPE2 | Unknown pduType2 field in a received |
    // |                                    | Share Data Header (section           |
    // |                                    | 2.2.8.1.1.1.2).                      |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CA ERRINFO_UNKNOWNPDUTYPE  | Unknown pduType field in a received  |
    // |                                    | Share Control Header (section        |
    // |                                    | 2.2.8.1.1.1.1).                      |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CB ERRINFO_DATAPDUSEQUENCE | An out-of-sequence Slow-Path Data PDU|
    // |                                    | (section 2.2.8.1.1.1.1) has been     |
    // |                                    | received.                            |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CD                         | An out-of-sequence Slow-Path Non-Data|
    // | ERRINFO_CONTROLPDUSEQUENCE         | PDU (section 2.2.8.1.1.1.1) has been |
    // |                                    | received.                            |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CE                         | A Control PDU (sections 2.2.1.15 and |
    // | ERRINFO_INVALIDCONTROLPDUACTION    | 2.2.1.16) has been received with an  |
    // |                                    | invalid action field.                |
    // +------------------------------------+--------------------------------------+
    // | 0x000010CF                         | (a) A Slow-Path Input Event (section |
    // | ERRINFO_INVALIDINPUTPDUTYPE        | 2.2.8.1.1.3.1.1) has been received   |
    // |                                    | with an invalid messageType field.   |
    // |                                    | (b) A Fast-Path Input Event (section |
    // |                                    | 2.2.8.1.2.2) has been received with  |
    // |                                    | an invalid eventCode field.          |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D0                         | (a) A Slow-Path Mouse Event (section |
    // | ERRINFO_INVALIDINPUTPDUMOUSE       | 2.2.8.1.1.3.1.1.3) or Extended Mouse |
    // |                                    | Event (section 2.2.8.1.1.3.1.1.4)    |
    // |                                    | has been received with an invalid    |
    // |                                    | pointerFlags field.                  |
    // |                                    | (b) A Fast-Path Mouse Event (section |
    // |                                    | 2.2.8.1.2.2.3) or Fast-Path Extended |
    // |                                    | Mouse Event (section 2.2.8.1.2.2.4)  |
    // |                                    | has been received with an invalid    |
    // |                                    | pointerFlags field.                  |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D1                         | An invalid Refresh Rect PDU (section |
    // | ERRINFO_INVALIDREFRESHRECTPDU      | 2.2.11.2) has been received.         |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D2                         | The server failed to construct the   |
    // | ERRINFO_CREATEUSERDATAFAILED       | GCC Conference Create Response user  |
    // |                                    | data (section 2.2.1.4).              |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D3 ERRINFO_CONNECTFAILED   | Processing during the Channel        |
    // |                                    | Connection phase of the RDP          |
    // |                                    | Connection Sequence (see section     |
    // |                                    | 1.3.1.1 for an overview of the RDP   |
    // |                                    | Connection Sequence phases) has      |
    // |                                    | failed.                              |
    // +------------------------------------+--------------------------------------+
    // | 0x000010D4                         | A Confirm Active PDU (section        |
    // | ERRINFO_CONFIRMACTIVEWRONGSHAREID  | 2.2.1.13.2) was received from the    |
    // |                                    | client with an invalid shareId field.|
    // +------------------------------------+-+------------------------------------+
    // | 0x000010D5                           | A Confirm Active PDU (section      |
    // | ERRINFO_CONFIRMACTIVEWRONGORIGINATOR | 2.2.1.13.2) was received from the  |
    // |                                      | client with an invalid originatorId|
    // |                                      | field.                             |
    // +--------------------------------------+------------------------------------+
    // | 0x000010DA                           | There is not enough data to process|
    // | ERRINFO_PERSISTENTKEYPDUBADLENGTH    | a Persistent Key List PDU (section |
    // |                                      | 2.2.1.17).                         |
    // +--------------------------------------+------------------------------------+
    // | 0x000010DB                           | A Persistent Key List PDU (section |
    // | ERRINFO_PERSISTENTKEYPDUILLEGALFIRST | 2.2.1.17) marked as                |
    // |                                      | PERSIST_PDU_FIRST (0x01) was       |
    // |                                      | received after the reception of a  |
    // |                                      | prior Persistent Key List PDU also |
    // |                                      | marked as PERSIST_PDU_FIRST.       |
    // +--------------------------------------+---+--------------------------------+
    // | 0x000010DC                               | A Persistent Key List PDU      |
    // | ERRINFO_PERSISTENTKEYPDUTOOMANYTOTALKEYS | (section 2.2.1.17) was received|
    // |                                          | which specified a total number |
    // |                                          | of bitmap cache entries larger |
    // |                                          | than 262144.                   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010DD                               | A Persistent Key List PDU      |
    // | ERRINFO_PERSISTENTKEYPDUTOOMANYCACHEKEYS | (section 2.2.1.17) was received|
    // |                                          | which specified an invalid     |
    // |                                          | total number of keys for a     |
    // |                                          | bitmap cache (the number of    |
    // |                                          | entries that can be stored     |
    // |                                          | within each bitmap cache is    |
    // |                                          | specified in the Revision 1 or |
    // |                                          | 2 Bitmap Cache Capability Set  |
    // |                                          | (section 2.2.7.1.4) that is    |
    // |                                          | sent from client to server).   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010DE ERRINFO_INPUTPDUBADLENGTH     | There is not enough data to    |
    // |                                          | process Input Event PDU Data   |
    // |                                          | (section 2.2.8.1.1.3.          |
    // |                                          | 2.2.8.1.2).                    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010DF                               | There is not enough data to    |
    // | ERRINFO_BITMAPCACHEERRORPDUBADLENGTH     | process the shareDataHeader,   |
    // |                                          | NumInfoBlocks, Pad1, and Pad2  |
    // |                                          | fields of the Bitmap Cache     |
    // |                                          | Error PDU Data ([MS-RDPEGDI]   |
    // |                                          | section 2.2.2.3.1.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E0  ERRINFO_SECURITYDATATOOSHORT | (a) The dataSignature field of |
    // |                                          | the Fast-Path Input Event PDU  |
    // |                                          | (section 2.2.8.1.2) does not   |
    // |                                          | contain enough data.           |
    // |                                          | (b) The fipsInformation and    |
    // |                                          | dataSignature fields of the    |
    // |                                          | Fast-Path Input Event PDU      |
    // |                                          | (section 2.2.8.1.2) do not     |
    // |                                          | contain enough data.           |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E1 ERRINFO_VCHANNELDATATOOSHORT  | (a) There is not enough data   |
    // |                                          | in the Client Network Data     |
    // |                                          | (section 2.2.1.3.4) to read the|
    // |                                          | virtual channel configuration  |
    // |                                          | data.                          |
    // |                                          | (b) There is not enough data   |
    // |                                          | to read a complete Channel     |
    // |                                          | PDU Header (section 2.2.6.1.1).|
    // +------------------------------------------+--------------------------------+
    // | 0x000010E2 ERRINFO_SHAREDATATOOSHORT     | (a) There is not enough data   |
    // |                                          | to process Control PDU Data    |
    // |                                          | (section 2.2.1.15.1).          |
    // |                                          | (b) There is not enough data   |
    // |                                          | to read a complete Share       |
    // |                                          | Control Header (section        |
    // |                                          | 2.2.8.1.1.1.1).                |
    // |                                          | (c) There is not enough data   |
    // |                                          | to read a complete Share Data  |
    // |                                          | Header (section 2.2.8.1.1.1.2) |
    // |                                          | of a Slow-Path Data PDU        |
    // |                                          | (section 2.2.8.1.1.1.1).       |
    // |                                          | (d) There is not enough data   |
    // |                                          | to process Font List PDU Data  |
    // |                                          | (section 2.2.1.18.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E3 ERRINFO_BADSUPRESSOUTPUTPDU   | (a) There is not enough data   |
    // |                                          | to process Suppress Output PDU |
    // |                                          | Data (section 2.2.11.3.1).     |
    // |                                          | (b) The allowDisplayUpdates    |
    // |                                          | field of the Suppress Output   |
    // |                                          | PDU Data (section 2.2.11.3.1)  |
    // |                                          | is invalid.                    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E5                               | (a) There is not enough data   |
    // | ERRINFO_CONFIRMACTIVEPDUTOOSHORT         | to read the shareControlHeader,|
    // |                                          | shareId, originatorId,         |
    // |                                          | lengthSourceDescriptor, and    |
    // |                                          | lengthCombinedCapabilities     |
    // |                                          | fields of the Confirm Active   |
    // |                                          | PDU Data (section              |
    // |                                          | 2.2.1.13.2.1).                 |
    // |                                          | (b) There is not enough data   |
    // |                                          | to read the sourceDescriptor,  |
    // |                                          | numberCapabilities, pad2Octets,|
    // |                                          | and capabilitySets fields of   |
    // |                                          | the Confirm Active PDU Data    |
    // |                                          | (section 2.2.1.13.2.1).        |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E7 ERRINFO_CAPABILITYSETTOOSMALL | There is not enough data to    |
    // |                                          | read the capabilitySetType and |
    // |                                          | the lengthCapability fields in |
    // |                                          | a received Capability Set      |
    // |                                          | (section 2.2.1.13.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E8 ERRINFO_CAPABILITYSETTOOLARGE | A Capability Set (section      |
    // |                                          | 2.2.1.13.1.1.1) has been       |
    // |                                          | received with a                |
    // |                                          | lengthCapability field that    |
    // |                                          | contains a value greater than  |
    // |                                          | the total length of the data   |
    // |                                          | received.                      |
    // +------------------------------------------+--------------------------------+
    // | 0x000010E9 ERRINFO_NOCURSORCACHE         | (a) Both the                   |
    // |                                          | colorPointerCacheSize and      |
    // |                                          | pointerCacheSize fields in the |
    // |                                          | Pointer Capability Set         |
    // |                                          | (section 2.2.7.1.5) are set to |
    // |                                          | zero.                          |
    // |                                          | (b) The pointerCacheSize field |
    // |                                          | in the Pointer Capability Set  |
    // |                                          | (section 2.2.7.1.5) is not     |
    // |                                          | present, and the               |
    // |                                          | colorPointerCacheSize field is |
    // |                                          | set to zero.                   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010EA ERRINFO_BADCAPABILITIES       | The capabilities received from |
    // |                                          | the client in the Confirm      |
    // |                                          | Active PDU (section 2.2.1.13.2)|
    // |                                          | were not accepted by the       |
    // |                                          | server.                        |
    // +------------------------------------------+--------------------------------+
    // | 0x000010EC                               | An error occurred while using  |
    // | ERRINFO_VIRTUALCHANNELDECOMPRESSIONERR   | the bulk compressor (section   |
    // |                                          | 3.1.8 and [MS- RDPEGDI] section|
    // |                                          | 3.1.8) to decompress a Virtual |
    // |                                          | Channel PDU (section 2.2.6.1). |
    // +------------------------------------------+--------------------------------+
    // | 0x000010ED                               | An invalid bulk compression    |
    // | ERRINFO_INVALIDVCCOMPRESSIONTYPE         | package was specified in the   |
    // |                                          | flags field of the Channel PDU |
    // |                                          | Header (section 2.2.6.1.1).    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010EF ERRINFO_INVALIDCHANNELID      | An invalid MCS channel ID was  |
    // |                                          | specified in the mcsPdu field  |
    // |                                          | of the Virtual Channel PDU     |
    // |                                          | (section 2.2.6.1).             |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F0 ERRINFO_VCHANNELSTOOMANY      | The client requested more than |
    // |                                          | the maximum allowed 31 static  |
    // |                                          | virtual channels in the Client |
    // |                                          | Network Data (section          |
    // |                                          | 2.2.1.3.4).                    |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F3 ERRINFO_REMOTEAPPSNOTENABLED  | The INFO_RAIL flag (0x00008000)|
    // |                                          | MUST be set in the flags field |
    // |                                          | of the Info Packet (section    |
    // |                                          | 2.2.1.11.1.1) as the session   |
    // |                                          | on the remote server can only  |
    // |                                          | host remote applications.      |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F4 ERRINFO_CACHECAPNOTSET        | The client sent a Persistent   |
    // |                                          | Key List PDU (section 2.2.1.17)|
    // |                                          | without including the          |
    // |                                          | prerequisite Revision 2 Bitmap |
    // |                                          | Cache Capability Set (section  |
    // |                                          | 2.2.7.1.4.2) in the Confirm    |
    // |                                          | Active PDU (section            |
    // |                                          | 2.2.1.13.2).                   |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F5                               | The NumInfoBlocks field in the |
    // |ERRINFO_BITMAPCACHEERRORPDUBADLENGTH2     | Bitmap Cache Error PDU Data is |
    // |                                          | inconsistent with the amount   |
    // |                                          | of data in the Info field      |
    // |                                          | ([MS-RDPEGDI] section          |
    // |                                          | 2.2.2.3.1.1).                  |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F6                               | There is not enough data to    |
    // | ERRINFO_OFFSCRCACHEERRORPDUBADLENGTH     | process an Offscreen Bitmap    |
    // |                                          | Cache Error PDU ([MS-RDPEGDI]  |
    // |                                          | section 2.2.2.3.2).            |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F7                               | There is not enough data to    |
    // | ERRINFO_DNGCACHEERRORPDUBADLENGTH        | process a DrawNineGrid Cache   |
    // |                                          | Error PDU ([MS-RDPEGDI]        |
    // |                                          | section 2.2.2.3.3).            |
    // +------------------------------------------+--------------------------------+
    // | 0x000010F8 ERRINFO_GDIPLUSPDUBADLENGTH   | There is not enough data to    |
    // |                                          | process a GDI+ Error PDU       |
    // |                                          | ([MS-RDPEGDI] section          |
    // |                                          | 2.2.2.3.4).                    |
    // +------------------------------------------+--------------------------------+
    // | 0x00001111 ERRINFO_SECURITYDATATOOSHORT2 | There is not enough data to    |
    // |                                          | read a Basic Security Header   |
    // |                                          | (section 2.2.8.1.1.2.1).       |
    // +------------------------------------------+--------------------------------+
    // | 0x00001112 ERRINFO_SECURITYDATATOOSHORT3 | There is not enough data to    |
    // |                                          | read a Non- FIPS Security      |
    // |                                          | Header (section 2.2.8.1.1.2.2) |
    // |                                          | or FIPS Security Header        |
    // |                                          | (section 2.2.8.1.1.2.3).       |
    // +------------------------------------------+--------------------------------+
    // | 0x00001113 ERRINFO_SECURITYDATATOOSHORT4 | There is not enough data to    |
    // |                                          | read the basicSecurityHeader   |
    // |                                          | and length fields of the       |
    // |                                          | Security Exchange PDU Data     |
    // |                                          | (section 2.2.1.10.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x00001114 ERRINFO_SECURITYDATATOOSHORT5 | There is not enough data to    |
    // |                                          | read the CodePage, flags,      |
    // |                                          | cbDomain, cbUserName,          |
    // |                                          | cbPassword, cbAlternateShell,  |
    // |                                          | cbWorkingDir, Domain, UserName,|
    // |                                          | Password, AlternateShell, and  |
    // |                                          | WorkingDir fields in the Info  |
    // |                                          | Packet (section 2.2.1.11.1.1). |
    // +------------------------------------------+--------------------------------+
    // | 0x00001115 ERRINFO_SECURITYDATATOOSHORT6 | There is not enough data to    |
    // |                                          | read the CodePage, flags,      |
    // |                                          | cbDomain, cbUserName,          |
    // |                                          | cbPassword, cbAlternateShell,  |
    // |                                          | and cbWorkingDir fields in the |
    // |                                          | Info Packet (section           |
    // |                                          | 2.2.1.11.1.1).                 |
    // +------------------------------------------+--------------------------------+
    // | 0x00001116 ERRINFO_SECURITYDATATOOSHORT7 | There is not enough data to    |
    // |                                          | read the clientAddressFamily   |
    // |                                          | and cbClientAddress fields in  |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001117 ERRINFO_SECURITYDATATOOSHORT8 | There is not enough data to    |
    // |                                          | read the clientAddress field in|
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001118 ERRINFO_SECURITYDATATOOSHORT9 | There is not enough data to    |
    // |                                          | read the cbClientDir field in  |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001119 ERRINFO_SECURITYDATATOOSHORT10| There is not enough data to    |
    // |                                          | read the clientDir field in the|
    // |                                          | Extended Info Packet (section  |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111A ERRINFO_SECURITYDATATOOSHORT11| There is not enough data to    |
    // |                                          | read the clientTimeZone field  |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111B ERRINFO_SECURITYDATATOOSHORT12| There is not enough data to    |
    // |                                          | read the clientSessionId field |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111C ERRINFO_SECURITYDATATOOSHORT13| There is not enough data to    |
    // |                                          | read the performanceFlags      |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111D ERRINFO_SECURITYDATATOOSHORT14| There is not enough data to    |
    // |                                          | read the cbAutoReconnectLen    |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111E ERRINFO_SECURITYDATATOOSHORT15| There is not enough data to    |
    // |                                          | read the autoReconnectCookie   |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x0000111F ERRINFO_SECURITYDATATOOSHORT16| The cbAutoReconnectLen field   |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1)       |
    // |                                          | contains a value which is      |
    // |                                          | larger than the maximum        |
    // |                                          | allowed length of 128 bytes.   |
    // +------------------------------------------+--------------------------------+
    // | 0x00001120 ERRINFO_SECURITYDATATOOSHORT17| There is not enough data to    |
    // |                                          | read the clientAddressFamily   |
    // |                                          | and cbClientAddress fields in  |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001121 ERRINFO_SECURITYDATATOOSHORT18| There is not enough data to    |
    // |                                          | read the clientAddress field in|
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001122 ERRINFO_SECURITYDATATOOSHORT19| There is not enough data to    |
    // |                                          | read the cbClientDir field in  |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001123 ERRINFO_SECURITYDATATOOSHORT20| There is not enough data to    |
    // |                                          | read the clientDir field in    |
    // |                                          | the Extended Info Packet       |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001124 ERRINFO_SECURITYDATATOOSHORT21| There is not enough data to    |
    // |                                          | read the clientTimeZone field  |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001125 ERRINFO_SECURITYDATATOOSHORT22| There is not enough data to    |
    // |                                          | read the clientSessionId field |
    // |                                          | in the Extended Info Packet    |
    // |                                          | (section 2.2.1.11.1.1.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001126 ERRINFO_SECURITYDATATOOSHORT23| There is not enough data to    |
    // |                                          | read the Client Info PDU Data  |
    // |                                          | (section 2.2.1.11.1).          |
    // +------------------------------------------+--------------------------------+
    // | 0x00001129 ERRINFO_BADMONITORDATA        | The monitorCount field in the  |
    // |                                          | Client Monitor Data (section   |
    // |                                          | 2.2.1.3.6) is invalid.         |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112A                               | The server-side decompression  |
    // | ERRINFO_VCDECOMPRESSEDREASSEMBLEFAILED   | buffer is invalid, or the size |
    // |                                          | of the decompressed VC data    |
    // |                                          | exceeds the chunking size      |
    // |                                          | specified in the Virtual       |
    // |                                          | Channel Capability Set         |
    // |                                          | (section 2.2.7.1.10).          |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112B ERRINFO_VCDATATOOLONG         | The size of a received Virtual |
    // |                                          | Channel PDU (section 2.2.6.1)  |
    // |                                          | exceeds the chunking size      |
    // |                                          | specified in the Virtual       |
    // |                                          | Channel Capability Set         |
    // |                                          | (section 2.2.7.1.10).          |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112C ERRINFO_BAD_FRAME_ACK_DATA    | There is not enough data to    |
    // |                                          | read a                         |
    // |                                          | TS_FRAME_ACKNOWLEDGE_PDU ([MS- |
    // |                                          | RDPRFX] section 2.2.3.1).      |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112D                               | The graphics mode requested by |
    // | ERRINFO_GRAPHICSMODENOTSUPPORTED         | the client is not supported by |
    // |                                          | the server.                    |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112E                               | The server-side graphics       |
    // | ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED     | subsystem failed to reset.     |
    // +------------------------------------------+--------------------------------+
    // | 0x0000112F                               | The server-side graphics       |
    // | ERRINFO_GRAPHICSSUBSYSTEMFAILED          | subsystem is in an error state |
    // |                                          | and unable to continue         |
    // |                                          | graphics encoding.             |
    // +------------------------------------------+--------------------------------+
    // | 0x00001130                               | There is not enough data to    |
    // | ERRINFO_TIMEZONEKEYNAMELENGTHTOOSHORT    | read the                       |
    // |                                          | cbDynamicDSTTimeZoneKeyName    |
    // |                                          | field in the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x00001131                               | The length reported in the     |
    // | ERRINFO_TIMEZONEKEYNAMELENGTHTOOLONG     | cbDynamicDSTTimeZoneKeyName    |
    // |                                          | field of the Extended Info     |
    // |                                          | Packet (section                |
    // |                                          | 2.2.1.11.1.1.1) is too long.   |
    // +------------------------------------------+--------------------------------+
    // | 0x00001132                               | The                            |
    // | ERRINFO_DYNAMICDSTDISABLEDFIELDMISSING   | dynamicDaylightTimeDisabled    |
    // |                                          | field is not present in the    |
    // |                                          | Extended Info Packet (section  |
    // |                                          | 2.2.1.11.1.1.1).               |
    // +------------------------------------------+--------------------------------+
    // | 0x00001191                               | An attempt to update the       |
    // | ERRINFO_UPDATESESSIONKEYFAILED           | session keys while using       |
    // |                                          | Standard RDP Security          |
    // |                                          | mechanisms (section 5.3.7)     |
    // |                                          | failed.                        |
    // +------------------------------------------+--------------------------------+
    // | 0x00001192 ERRINFO_DECRYPTFAILED         | (a) Decryption using Standard  |
    // |                                          | RDP Security mechanisms        |
    // |                                          | (section 5.3.6) failed.        |
    // |                                          | (b) Session key creation using |
    // |                                          | Standard RDP Security          |
    // |                                          | mechanisms (section 5.3.5)     |
    // |                                          | failed.                        |
    // +------------------------------------------+--------------------------------+
    // | 0x00001193 ERRINFO_ENCRYPTFAILED         | Encryption using Standard RDP  |
    // |                                          | Security mechanisms (section   |
    // |                                          | 5.3.6) failed.                 |
    // +------------------------------------------+--------------------------------+
    // | 0x00001194 ERRINFO_ENCPKGMISMATCH        | Failed to find a usable        |
    // |                                          | Encryption Method (section     |
    // |                                          | 5.3.2) in the encryptionMethods|
    // |                                          | field of the Client Security   |
    // |                                          | Data (section 2.2.1.4.3).      |
    // +------------------------------------------+--------------------------------+
    // | 0x00001195 ERRINFO_DECRYPTFAILED2        | Encryption using Standard RDP  |
    // |                                          | Security mechanisms (section   |
    // |                                          | 5.3.6) failed. Unencrypted     |
    // |                                          | data was encountered in a      |
    // |                                          | protocol stream which is meant |
    // |                                          | to be encrypted with Standard  |
    // |                                          | RDP Security mechanisms        |
    // |                                          | (section 5.3.6).               |
    // +------------------------------------------+--------------------------------+

    enum {
        ERRINFO_RPC_INITIATED_DISCONNECT          = 0x00000001,
        ERRINFO_RPC_INITIATED_LOGOFF              = 0x00000002,
        ERRINFO_IDLE_TIMEOUT                      = 0x00000003,
        ERRINFO_LOGON_TIMEOUT                     = 0x00000004,
        ERRINFO_DISCONNECTED_BY_OTHERCONNECTION   = 0x00000005,
        ERRINFO_OUT_OF_MEMORY                     = 0x00000006,
        ERRINFO_SERVER_DENIED_CONNECTION          = 0x00000007,
        ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES    = 0x00000009,
        ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED = 0x0000000A,
        ERRINFO_RPC_INITIATED_DISCONNECT_BYUSER   = 0x0000000B,
        ERRINFO_LOGOFF_BY_USER                    = 0x0000000C,
        ERRINFO_LICENSE_INTERNAL                  = 0x00000100,
        ERRINFO_LICENSE_NO_LICENSE_SERVER         = 0x00000101,
        ERRINFO_LICENSE_NO_LICENSE                = 0x00000102,
        ERRINFO_LICENSE_BAD_CLIENT_MSG            = 0x00000103,
        ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE = 0x00000104,
        ERRINFO_LICENSE_BAD_CLIENT_LICENSE        = 0x00000105,
        ERRINFO_LICENSE_CANT_FINISH_PROTOCOL      = 0x00000106,
        ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL     = 0x00000107,
        ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION     = 0x00000108,
        ERRINFO_LICENSE_CANT_UPGRADE_LICENSE      = 0x00000109,
        ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS     = 0x0000010A,

        ERRINFO_CB_DESTINATION_NOT_FOUND             = 0x00000400,
        ERRINFO_CB_LOADING_DESTINATION               = 0x00000402,
        ERRINFO_CB_REDIRECTING_TO_DESTINATION        = 0x00000404,
        ERRINFO_CB_SESSION_ONLINE_VM_WAKE            = 0x00000405,
        ERRINFO_CB_SESSION_ONLINE_VM_BOOT            = 0x00000406,
        ERRINFO_CB_SESSION_ONLINE_VM_NO_DNS          = 0x00000407,
        ERRINFO_CB_DESTINATION_POOL_NOT_FREE         = 0x00000408,
        ERRINFO_CB_CONNECTION_CANCELLED              = 0x00000409,
        ERRINFO_CB_CONNECTION_ERROR_INVALID_SETTINGS = 0x00000410,
        ERRINFO_CB_SESSION_ONLINE_VM_BOOT_TIMEOUT    = 0x00000411,
        ERRINFO_CB_SESSION_ONLINE_VM_SESSMON_FAILED  = 0x00000412,

        ERRINFO_UNKNOWNPDUTYPE2                   = 0x000010C9,
        ERRINFO_UNKNOWNPDUTYPE                    = 0x000010CA,
        ERRINFO_DATAPDUSEQUENCE                   = 0x000010CB,
        ERRINFO_CONTROLPDUSEQUENCE                = 0x000010CD,
        ERRINFO_INVALIDCONTROLPDUACTION           = 0x000010CE,
        ERRINFO_INVALIDINPUTPDUTYPE               = 0x000010CF,
        ERRINFO_INVALIDINPUTPDUMOUSE              = 0x000010D0,
        ERRINFO_INVALIDREFRESHRECTPDU             = 0x000010D1,
        ERRINFO_CREATEUSERDATAFAILED              = 0x000010D2,
        ERRINFO_CONNECTFAILED                     = 0x000010D3,
        ERRINFO_CONFIRMACTIVEWRONGSHAREID         = 0x000010D4,
        ERRINFO_CONFIRMACTIVEWRONGORIGINATOR      = 0x000010D5,
        ERRINFO_PERSISTENTKEYPDUBADLENGTH         = 0x000010DA,
        ERRINFO_PERSISTENTKEYPDUILLEGALFIRST      = 0x000010DB,
        ERRINFO_PERSISTENTKEYPDUTOOMANYTOTALKEYS  = 0x000010DC,
        ERRINFO_PERSISTENTKEYPDUTOOMANYCACHEKEYS  = 0x000010DD,
        ERRINFO_INPUTPDUBADLENGTH                 = 0x000010DE,
        ERRINFO_BITMAPCACHEERRORPDUBADLENGTH      = 0x000010DF,
        ERRINFO_SECURITYDATATOOSHORT              = 0x000010E0,
        ERRINFO_VCHANNELDATATOOSHORT              = 0x000010E1,
        ERRINFO_SHAREDATATOOSHORT                 = 0x000010E2,
        ERRINFO_BADSUPRESSOUTPUTPDU               = 0x000010E3,
        ERRINFO_CONFIRMACTIVEPDUTOOSHORT          = 0x000010E5,
        ERRINFO_CAPABILITYSETTOOSMALL             = 0x000010E7,
        ERRINFO_CAPABILITYSETTOOLARGE             = 0x000010E8,
        ERRINFO_NOCURSORCACHE                     = 0x000010E9,
        ERRINFO_BADCAPABILITIES                   = 0x000010EA,
        ERRINFO_VIRTUALCHANNELDECOMPRESSIONERR    = 0x000010EC,
        ERRINFO_INVALIDVCCOMPRESSIONTYPE          = 0x000010ED,
        ERRINFO_INVALIDCHANNELID                  = 0x000010EF,
        ERRINFO_VCHANNELSTOOMANY                  = 0x000010F0,
        ERRINFO_REMOTEAPPSNOTENABLED              = 0x000010F3,
        ERRINFO_CACHECAPNOTSET                    = 0x000010F4,
        ERRINFO_BITMAPCACHEERRORPDUBADLENGTH2     = 0x000010F5,
        ERRINFO_OFFSCRCACHEERRORPDUBADLENGTH      = 0x000010F6,
        ERRINFO_DNGCACHEERRORPDUBADLENGTH         = 0x000010F7,
        ERRINFO_GDIPLUSPDUBADLENGTH               = 0x000010F8,
        ERRINFO_SECURITYDATATOOSHORT2             = 0x00001111,
        ERRINFO_SECURITYDATATOOSHORT3             = 0x00001112,
        ERRINFO_SECURITYDATATOOSHORT4             = 0x00001113,
        ERRINFO_SECURITYDATATOOSHORT5             = 0x00001114,
        ERRINFO_SECURITYDATATOOSHORT6             = 0x00001115,
        ERRINFO_SECURITYDATATOOSHORT7             = 0x00001116,
        ERRINFO_SECURITYDATATOOSHORT8             = 0x00001117,
        ERRINFO_SECURITYDATATOOSHORT9             = 0x00001118,
        ERRINFO_SECURITYDATATOOSHORT10            = 0x00001119,
        ERRINFO_SECURITYDATATOOSHORT11            = 0x0000111A,
        ERRINFO_SECURITYDATATOOSHORT12            = 0x0000111B,
        ERRINFO_SECURITYDATATOOSHORT13            = 0x0000111C,
        ERRINFO_SECURITYDATATOOSHORT14            = 0x0000111D,
        ERRINFO_SECURITYDATATOOSHORT15            = 0x0000111E,
        ERRINFO_SECURITYDATATOOSHORT16            = 0x0000111F,
        ERRINFO_SECURITYDATATOOSHORT17            = 0x00001120,
        ERRINFO_SECURITYDATATOOSHORT18            = 0x00001121,
        ERRINFO_SECURITYDATATOOSHORT19            = 0x00001122,
        ERRINFO_SECURITYDATATOOSHORT20            = 0x00001123,
        ERRINFO_SECURITYDATATOOSHORT21            = 0x00001124,
        ERRINFO_SECURITYDATATOOSHORT22            = 0x00001125,
        ERRINFO_SECURITYDATATOOSHORT23            = 0x00001126,
        ERRINFO_BADMONITORDATA                    = 0x00001129,
        ERRINFO_VCDECOMPRESSEDREASSEMBLEFAILED    = 0x0000112A,
        ERRINFO_VCDATATOOLONG                     = 0x0000112B,
        ERRINFO_BAD_FRAME_ACK_DATA                = 0x0000112C,
        ERRINFO_GRAPHICSMODENOTSUPPORTED          = 0x0000112D,
        ERRINFO_GRAPHICSSUBSYSTEMRESETFAILED      = 0x0000112E,
        ERRINFO_GRAPHICSSUBSYSTEMFAILED           = 0x0000112F,
        ERRINFO_TIMEZONEKEYNAMELENGTHTOOSHORT     = 0x00001130,
        ERRINFO_TIMEZONEKEYNAMELENGTHTOOLONG      = 0x00001131,
        ERRINFO_DYNAMICDSTDISABLEDFIELDMISSING    = 0x00001132,
        ERRINFO_UPDATESESSIONKEYFAILED            = 0x00001191,
        ERRINFO_DECRYPTFAILED                     = 0x00001192,
        ERRINFO_ENCRYPTFAILED                     = 0x00001193,
        ERRINFO_ENCPKGMISMATCH                    = 0x00001194,
        ERRINFO_DECRYPTFAILED2                    = 0x00001195
    };

    const char* get_error_info_name(uint32_t errorInfo) {
        switch (errorInfo){
            #define CASE(e) case ERRINFO_##e: return #e
            CASE(RPC_INITIATED_DISCONNECT);
            CASE(RPC_INITIATED_LOGOFF);
            CASE(IDLE_TIMEOUT);
            CASE(LOGON_TIMEOUT);
            CASE(DISCONNECTED_BY_OTHERCONNECTION);
            CASE(OUT_OF_MEMORY);
            CASE(SERVER_DENIED_CONNECTION);
            CASE(SERVER_INSUFFICIENT_PRIVILEGES);
            CASE(SERVER_FRESH_CREDENTIALS_REQUIRED);
            CASE(RPC_INITIATED_DISCONNECT_BYUSER);
            CASE(LOGOFF_BY_USER);
            CASE(LICENSE_INTERNAL);
            CASE(LICENSE_NO_LICENSE_SERVER);
            CASE(LICENSE_NO_LICENSE);
            CASE(LICENSE_BAD_CLIENT_MSG);
            CASE(LICENSE_HWID_DOESNT_MATCH_LICENSE);
            CASE(LICENSE_BAD_CLIENT_LICENSE);
            CASE(LICENSE_CANT_FINISH_PROTOCOL);
            CASE(LICENSE_CLIENT_ENDED_PROTOCOL);
            CASE(LICENSE_BAD_CLIENT_ENCRYPTION);
            CASE(LICENSE_CANT_UPGRADE_LICENSE);
            CASE(LICENSE_NO_REMOTE_CONNECTIONS);
            CASE(CB_DESTINATION_NOT_FOUND);
            CASE(CB_LOADING_DESTINATION);
            CASE(CB_REDIRECTING_TO_DESTINATION);
            CASE(CB_SESSION_ONLINE_VM_WAKE);
            CASE(CB_SESSION_ONLINE_VM_BOOT);
            CASE(CB_SESSION_ONLINE_VM_NO_DNS);
            CASE(CB_DESTINATION_POOL_NOT_FREE);
            CASE(CB_CONNECTION_CANCELLED);
            CASE(CB_CONNECTION_ERROR_INVALID_SETTINGS);
            CASE(CB_SESSION_ONLINE_VM_BOOT_TIMEOUT);
            CASE(CB_SESSION_ONLINE_VM_SESSMON_FAILED);
            CASE(UNKNOWNPDUTYPE2);
            CASE(UNKNOWNPDUTYPE);
            CASE(DATAPDUSEQUENCE);
            CASE(CONTROLPDUSEQUENCE);
            CASE(INVALIDCONTROLPDUACTION);
            CASE(INVALIDINPUTPDUTYPE);
            CASE(INVALIDINPUTPDUMOUSE);
            CASE(INVALIDREFRESHRECTPDU);
            CASE(CREATEUSERDATAFAILED);
            CASE(CONNECTFAILED);
            CASE(CONFIRMACTIVEWRONGSHAREID);
            CASE(CONFIRMACTIVEWRONGORIGINATOR);
            CASE(PERSISTENTKEYPDUBADLENGTH);
            CASE(PERSISTENTKEYPDUILLEGALFIRST);
            CASE(PERSISTENTKEYPDUTOOMANYTOTALKEYS);
            CASE(PERSISTENTKEYPDUTOOMANYCACHEKEYS);
            CASE(INPUTPDUBADLENGTH);
            CASE(BITMAPCACHEERRORPDUBADLENGTH);
            CASE(SECURITYDATATOOSHORT);
            CASE(VCHANNELDATATOOSHORT);
            CASE(SHAREDATATOOSHORT);
            CASE(BADSUPRESSOUTPUTPDU);
            CASE(CONFIRMACTIVEPDUTOOSHORT);
            CASE(CAPABILITYSETTOOSMALL);
            CASE(CAPABILITYSETTOOLARGE);
            CASE(NOCURSORCACHE);
            CASE(BADCAPABILITIES);
            CASE(VIRTUALCHANNELDECOMPRESSIONERR);
            CASE(INVALIDVCCOMPRESSIONTYPE);
            CASE(INVALIDCHANNELID);
            CASE(VCHANNELSTOOMANY);
            CASE(REMOTEAPPSNOTENABLED);
            CASE(CACHECAPNOTSET);
            CASE(BITMAPCACHEERRORPDUBADLENGTH2);
            CASE(OFFSCRCACHEERRORPDUBADLENGTH);
            CASE(DNGCACHEERRORPDUBADLENGTH);
            CASE(GDIPLUSPDUBADLENGTH);
            CASE(SECURITYDATATOOSHORT2);
            CASE(SECURITYDATATOOSHORT3);
            CASE(SECURITYDATATOOSHORT4);
            CASE(SECURITYDATATOOSHORT5);
            CASE(SECURITYDATATOOSHORT6);
            CASE(SECURITYDATATOOSHORT7);
            CASE(SECURITYDATATOOSHORT8);
            CASE(SECURITYDATATOOSHORT9);
            CASE(SECURITYDATATOOSHORT10);
            CASE(SECURITYDATATOOSHORT11);
            CASE(SECURITYDATATOOSHORT12);
            CASE(SECURITYDATATOOSHORT13);
            CASE(SECURITYDATATOOSHORT14);
            CASE(SECURITYDATATOOSHORT15);
            CASE(SECURITYDATATOOSHORT16);
            CASE(SECURITYDATATOOSHORT17);
            CASE(SECURITYDATATOOSHORT18);
            CASE(SECURITYDATATOOSHORT19);
            CASE(SECURITYDATATOOSHORT20);
            CASE(SECURITYDATATOOSHORT21);
            CASE(SECURITYDATATOOSHORT22);
            CASE(SECURITYDATATOOSHORT23);
            CASE(BADMONITORDATA);
            CASE(VCDECOMPRESSEDREASSEMBLEFAILED);
            CASE(VCDATATOOLONG);
            CASE(BAD_FRAME_ACK_DATA);
            CASE(GRAPHICSMODENOTSUPPORTED);
            CASE(GRAPHICSSUBSYSTEMRESETFAILED);
            CASE(GRAPHICSSUBSYSTEMFAILED);
            CASE(TIMEZONEKEYNAMELENGTHTOOSHORT);
            CASE(TIMEZONEKEYNAMELENGTHTOOLONG);
            CASE(DYNAMICDSTDISABLEDFIELDMISSING);
            CASE(UPDATESESSIONKEYFAILED);
            CASE(DECRYPTFAILED);
            CASE(ENCRYPTFAILED);
            CASE(ENCPKGMISMATCH);
            CASE(DECRYPTFAILED2);
            default:
                return "?";
        }
    }   // get_error_info_name

    uint32_t get_error_info_from_pdu(InStream & stream) {
        return stream.in_uint32_le();
    }

    void process_error_info(uint32_t errorInfo) {
        const char* errorInfo_name = get_error_info_name(errorInfo);
        LOG(LOG_INFO, "process disconnect pdu : code=0x%08X error=%s", errorInfo, errorInfo_name);

        if (errorInfo) {
            this->close_box_extra_message_ref += " (";
            this->close_box_extra_message_ref += errorInfo_name;
            this->close_box_extra_message_ref += ")";
        }

        switch (errorInfo){
        case ERRINFO_DISCONNECTED_BY_OTHERCONNECTION:
            this->authentifier.set_auth_error_message(TR(trkeys::disconnected_by_otherconnection, this->lang));
            break;
        case ERRINFO_REMOTEAPPSNOTENABLED:
            this->remote_apps_not_enabled = true;
            break;
        }
    }   // process_error_info

    void process_logon_info(const char * domain, const char * username) {
        char domain_username_format_0[2048];
        char domain_username_format_1[2048];

        snprintf(domain_username_format_0, sizeof(domain_username_format_0),
            "%s@%s", username, domain);
        snprintf(domain_username_format_1, sizeof(domain_username_format_0),
            "%s\\%s", domain, username);
        //LOG(LOG_INFO,
        //    "Domain username format 0=(%s) Domain username format 1=(%s)",
        //    domain_username_format_0, domain_username_format_0);

        if (this->disconnect_on_logon_user_change &&
            ((::strcasecmp(domain, this->domain) || ::strcasecmp(username, this->username)) &&
             (this->domain[0] ||
              (::strcasecmp(domain_username_format_0, this->username) &&
               ::strcasecmp(domain_username_format_1, this->username) &&
               ::strcasecmp(username, this->username))))) {
            if (this->error_message) {
                *this->error_message = "Unauthorized logon user change detected!";
            }

            this->end_session_reason  = "OPEN_SESSION_FAILED";
            this->end_session_message = "Unauthorized logon user change detected.";

            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s%s%s) -> (%s%s%s). "
                    "The session will be disconnected.",
                this->hostname, this->domain,
                (*this->domain ? "\\" : ""),
                this->username, domain,
                ((domain && (*domain)) ? "\\" : ""),
                username);
            throw Error(ERR_RDP_LOGON_USER_CHANGED);
        }

        if (this->session_probe_virtual_channel_p &&
            this->session_probe_start_launch_timeout_timer_only_after_logon) {
            this->session_probe_virtual_channel_p->start_launch_timeout_timer();
        }

        this->report_message.report("OPEN_SESSION_SUCCESSFUL", "OK.");
        this->end_session_reason = "CLOSE_SESSION_SUCCESSFUL";
        this->end_session_message = "OK.";

        if (this->open_session_timeout.count()) {
            this->open_session_timeout_checker.cancel_timeout();

            this->event.reset_trigger_time();
        }

        if (this->enable_session_probe) {
            const bool disable_input_event     = true;
            const bool disable_graphics_update = this->session_probe_enable_launch_mask;
            this->disable_input_event_and_graphics_update(
                disable_input_event, disable_graphics_update);
        }
    }   // process_logon_info

    void process_save_session_info(InStream & stream) {
        RDP::SaveSessionInfoPDUData_Recv ssipdudata(stream);

        this->logged_on = CLIENT_LOGGED;

        switch (ssipdudata.infoType) {
        case RDP::INFOTYPE_LOGON:
        {
            LOG(LOG_INFO, "process save session info : Logon");
            RDP::LogonInfoVersion1_Recv liv1(ssipdudata.payload);

            process_logon_info(reinterpret_cast<char *>(liv1.Domain),
                reinterpret_cast<char *>(liv1.UserName));

            this->front.send_savesessioninfo();
        }
        break;
        case RDP::INFOTYPE_LOGON_LONG:
        {
            LOG(LOG_INFO, "process save session info : Logon long");
            RDP::LogonInfoVersion2_Recv liv2(ssipdudata.payload);

            process_logon_info(reinterpret_cast<char *>(liv2.Domain),
                reinterpret_cast<char *>(liv2.UserName));

            this->front.send_savesessioninfo();
        }
        break;
        case RDP::INFOTYPE_LOGON_PLAINNOTIFY:
        {
            LOG(LOG_INFO, "process save session info : Logon plainnotify");
            RDP::PlainNotify_Recv pn(ssipdudata.payload);

            if (this->enable_session_probe) {
                const bool disable_input_event     = true;
                const bool disable_graphics_update = this->session_probe_enable_launch_mask;
                this->disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);
            }

            if (this->session_probe_virtual_channel_p &&
                this->session_probe_start_launch_timeout_timer_only_after_logon) {
                this->session_probe_virtual_channel_p->start_launch_timeout_timer();
            }
        }
        break;
        case RDP::INFOTYPE_LOGON_EXTENDED_INFO:
        {
            LOG(LOG_INFO, "process save session info : Logon extended info");
            RDP::LogonInfoExtended_Recv lie(ssipdudata.payload);

            RDP::LogonInfoField_Recv lif(lie.payload);

            if (lie.FieldsPresent & RDP::LOGON_EX_AUTORECONNECTCOOKIE) {
                LOG(LOG_INFO, "process save session info : Auto-reconnect cookie");

                RDP::ServerAutoReconnectPacket auto_reconnect;

                auto_reconnect.receive(lif.payload);
                auto_reconnect.log(LOG_INFO);

                OutStream stream(
                    this->server_auto_reconnect_packet_ref.data(),
                    this->server_auto_reconnect_packet_ref.size());

                auto_reconnect.emit(stream);

                this->is_server_auto_reconnec_packet_received = true;
            }

            if (lie.FieldsPresent & RDP::LOGON_EX_LOGONERRORS) {
                LOG(LOG_INFO, "process save session info : Logon Errors Info");

                RDP::LogonErrorsInfo_Recv lei(lif.payload);

                if ((RDP::LOGON_MSG_SESSION_CONTINUE != lei.ErrorNotificationType) &&
                    (RDP::LOGON_WARNING >= lei.ErrorNotificationData) &&
                    this->remote_program) {
                    LOG(LOG_ERR, "Can not redirect user's focus to the WinLogon screen in RemoteApp mode!");

                    std::string errmsg = "(RemoteApp) ";

                    errmsg += RDP::LogonErrorsInfo_Recv::ErrorNotificationDataToShortMessage(lei.ErrorNotificationData);
                    this->authentifier.set_auth_error_message(errmsg.c_str());
                    throw Error(ERR_RAIL_LOGON_FAILED_OR_WARNING);
                }
            }
        }
        break;
        }

        stream.in_skip_bytes(stream.in_remain());
    }

    // TODO CGR: this can probably be unified with process_confirm_active in front
    void process_server_caps(InStream & stream, uint16_t len) {
        // TODO check stream consumed and len
        (void)len;
        if (bool(this->verbose & RDPVerbose::capabilities)){
            LOG(LOG_INFO, "mod_rdp::process_server_caps");
        }

        FILE * const output_file =
            !this->output_filename.empty()
            ? fopen(this->output_filename.c_str(), "w")
            : nullptr;
        SCOPE_EXIT(if (output_file) { fclose(output_file); });

        uint16_t ncapsets = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (uint16_t n = 0; n < ncapsets; n++) {
            unsigned expected = 4; /* capabilitySetType(2) + lengthCapability(2) */

            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated Demand active PDU data, need=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_MCS_PDU_TRUNCATED);
            }

            uint16_t capset_type = stream.in_uint16_le();
            uint16_t capset_length = stream.in_uint16_le();

            expected = capset_length - 4 /* capabilitySetType(2) + lengthCapability(2) */;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated Demand active PDU data, need=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_MCS_PDU_TRUNCATED);
            }

            uint8_t const * next = stream.get_current() + expected;

            switch (capset_type) {
            case CAPSTYPE_GENERAL:
                {
                    GeneralCaps general_caps;
                    general_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        general_caps.log("Received from server");
                    }
                    if (output_file) {
                        general_caps.dump(output_file);
                    }
                }
                break;
            case CAPSTYPE_BITMAP:
                {
                    BitmapCaps bitmap_caps;
                    bitmap_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bitmap_caps.log("Received from server");
                    }
                    if (output_file) {
                        bitmap_caps.dump(output_file);
                    }
                    this->orders.bpp = bitmap_caps.preferredBitsPerPixel;
                    this->front_width = bitmap_caps.desktopWidth;
                    this->front_height = bitmap_caps.desktopHeight;
                }
                break;
            case CAPSTYPE_ORDER:
                {
                    OrderCaps order_caps;
                    order_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        order_caps.log("Received from server");
                    }
                    if (output_file) {
                        order_caps.dump(output_file);
                    }
                }
                break;
            case CAPSTYPE_INPUT:
                {
                    InputCaps input_caps;
                    input_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        input_caps.log("Received from server");
                    }

                    this->enable_fastpath_client_input_event =
                        (this->enable_fastpath && ((input_caps.inputFlags & (INPUT_FLAG_FASTPATH_INPUT | INPUT_FLAG_FASTPATH_INPUT2)) != 0));
                }
                break;
            case CAPSTYPE_RAIL:
                {
                    RailCaps rail_caps;
                    rail_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        rail_caps.log("Received from server");
                    }
                }
                break;
            case CAPSTYPE_WINDOW:
                {
                    WindowListCaps window_list_caps;
                    window_list_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        window_list_caps.log("Received from server");
                    }
                }
                break;
            case CAPSTYPE_POINTER:
                {
                    PointerCaps pointer_caps;
                    pointer_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        pointer_caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSETTYPE_MULTIFRAGMENTUPDATE:
                {
                    MultiFragmentUpdateCaps multifrag_caps;
                    multifrag_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        multifrag_caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSETTYPE_LARGE_POINTER:
                {
                    LargePointerCaps large_pointer_caps;
                    large_pointer_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        large_pointer_caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_SOUND:
                {
                    SoundCaps sound_caps;
                    sound_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        sound_caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_FONT:
                {
                    FontCaps fontCaps;
                    fontCaps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        fontCaps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_ACTIVATION:
                {
                    ActivationCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_VIRTUALCHANNEL:
                {
                    VirtualChannelCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_DRAWGDIPLUS:
                {
                    DrawGdiPlusCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_COLORCACHE:
                {
                    ColorCacheCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_BITMAPCACHE_HOSTSUPPORT:
                {
                    BitmapCacheHostSupportCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSTYPE_SHARE:
                {
                    ShareCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSETTYPE_COMPDESK:
                {
                    CompDeskCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSETTYPE_SURFACE_COMMANDS:
                {
                    SurfaceCommandsCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSETTYPE_BITMAP_CODECS:
                {
                    BitmapCodecCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            case CAPSETTYPE_FRAME_ACKNOWLEDGE:
                {
                    FrameAcknowledgeCaps caps;
                    caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        caps.log("Receiving from server");
                    }
                }
                break;
            default:
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_WARNING,
                        "Unprocessed Capability Set is encountered. capabilitySetType=%s(%u)",
                        ::get_capabilitySetType_name(capset_type),
                        capset_type);
                }
                break;
            }
            stream.in_skip_bytes(next - stream.get_current());
        }

        if (bool(this->verbose & RDPVerbose::capabilities)){
            LOG(LOG_INFO, "mod_rdp::process_server_caps done");
        }
    }   // process_server_caps

    void send_control(int action) {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_control");
        }

        this->send_data_request_ex(
            GCC::MCS_GLOBAL_CHANNEL,
            [this, action](StreamSize<256>, OutStream & stream) {
                ShareData sdata(stream);
                sdata.emit_begin(PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

                // Payload
                stream.out_uint16_le(action);
                stream.out_uint16_le(0); /* userid */
                stream.out_uint32_le(0); /* control id */

                // Packet trailer
                sdata.emit_end();
            },
            [this](StreamSize<256>, OutStream & sctrl_header, std::size_t packet_size) {
                ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->userid + GCC::MCS_USERCHANNEL_BASE, packet_size);

            }
        );

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_control done");
        }
    }

    /* Send persistent bitmap cache enumeration PDU's
       Not implemented yet because it should be implemented
       before in process_data case. The problem is that
       we don't save the bitmap key list attached with rdp_bmpcache2 capability
       message so we can't develop this function yet */
    template<class DataWriter>
    void send_persistent_key_list_pdu(DataWriter data_writer) {
        this->send_pdu_type2(PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST, RDP::STREAM_MED, data_writer);
    }

    template<class DataWriter>
    void send_pdu_type2(uint8_t pdu_type2, uint8_t stream_id, DataWriter data_writer) {
        using packet_size_t = decltype(details_::packet_size(data_writer));
        this->send_data_request_ex(
            GCC::MCS_GLOBAL_CHANNEL,
            [this, &data_writer, pdu_type2, stream_id](
                StreamSize<256 + packet_size_t{}>, OutStream & stream) {
                ShareData sdata(stream);
                sdata.emit_begin(pdu_type2, this->share_id, stream_id);
                {
                    OutStream substream(stream.get_current(), packet_size_t{});
                    data_writer(packet_size_t{}, substream);
                    stream.out_skip_bytes(substream.get_offset());
                }
                sdata.emit_end();
            },
            [this](StreamSize<256>, OutStream & sctrl_header, std::size_t packet_size) {
                ShareControl_Send(
                    sctrl_header, PDUTYPE_DATAPDU,
                    this->userid + GCC::MCS_USERCHANNEL_BASE, packet_size
                );
            }
        );
    }

    void send_persistent_key_list_regular() {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_regular");
        }

        uint16_t totalEntriesCache[BmpCache::MAXIMUM_NUMBER_OF_CACHES] = { 0, 0, 0, 0, 0 };

        for (uint8_t cache_id = 0; cache_id < this->orders.bmp_cache->number_of_cache; cache_id++) {
            const BmpCache::cache_ & cache = this->orders.bmp_cache->get_cache(cache_id);
            if (cache.persistent()) {
                uint16_t idx = 0;
                while (idx < cache.size() && cache[idx]) {
                    ++idx;
                }
                uint32_t const max_cache_num_entries = this->BmpCacheRev2_Cache_NumEntries()[cache_id];
                totalEntriesCache[cache_id] = std::min<uint32_t>(idx, max_cache_num_entries);
                //LOG(LOG_INFO, "totalEntriesCache[%d]=%d", cache_id, idx);
            }
        }
        //LOG(LOG_INFO, "totalEntriesCache0=%u totalEntriesCache1=%u totalEntriesCache2=%u totalEntriesCache3=%u totalEntriesCache4=%u",
        //    totalEntriesCache[0], totalEntriesCache[1], totalEntriesCache[2], totalEntriesCache[3], totalEntriesCache[4]);

        uint16_t total_number_of_entries = totalEntriesCache[0] + totalEntriesCache[1] + totalEntriesCache[2] +
                                           totalEntriesCache[3] + totalEntriesCache[4];
        if (total_number_of_entries > 0) {
            RDP::PersistentKeyListPDUData pklpdu;
            pklpdu.bBitMask |= RDP::PERSIST_FIRST_PDU;

            uint16_t number_of_entries     = 0;
            uint8_t  pdu_number_of_entries = 0;
            for (uint8_t cache_id = 0; cache_id < this->orders.bmp_cache->number_of_cache; cache_id++) {
                const BmpCache::cache_ & cache = this->orders.bmp_cache->get_cache(cache_id);

                if (!cache.persistent()) {
                    continue;
                }

                const uint16_t entries_max = totalEntriesCache[cache_id];
                for (uint16_t cache_index = 0; cache_index < entries_max; cache_index++) {
                    pklpdu.entries[pdu_number_of_entries].Key1 = cache[cache_index].sig.sig_32[0];
                    pklpdu.entries[pdu_number_of_entries].Key2 = cache[cache_index].sig.sig_32[1];

                    pklpdu.numEntriesCache[cache_id]++;
                    number_of_entries++;
                    pdu_number_of_entries++;

                    if ((pdu_number_of_entries == RDP::PersistentKeyListPDUData::MAXIMUM_ENCAPSULATED_BITMAP_KEYS) ||
                        (number_of_entries == total_number_of_entries))
                    {
                        if (number_of_entries == total_number_of_entries) {
                            pklpdu.bBitMask |= RDP::PERSIST_LAST_PDU;
                        }

                        pklpdu.totalEntriesCache[0] = totalEntriesCache[0];
                        pklpdu.totalEntriesCache[1] = totalEntriesCache[1];
                        pklpdu.totalEntriesCache[2] = totalEntriesCache[2];
                        pklpdu.totalEntriesCache[3] = totalEntriesCache[3];
                        pklpdu.totalEntriesCache[4] = totalEntriesCache[4];

                        //pklpdu.log(LOG_INFO, "Send to server");

                        this->send_persistent_key_list_pdu(
                            [&pklpdu](StreamSize<2048>, OutStream & pdu_data_stream) {
                                pklpdu.emit(pdu_data_stream);
                            }
                        );

                        pklpdu.reset();

                        pdu_number_of_entries = 0;
                    }
                }
            }
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_regular done");
        }
    }   // send_persistent_key_list_regular

    void send_persistent_key_list_transparent() {
        if (!this->persistent_key_list_transport) {
            return;
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_transparent");
        }

        try
        {
            while (1) {
                this->send_persistent_key_list_pdu(
                    [this](StreamSize<65535>, OutStream & pdu_data_stream) {
                        uint8_t * data = pdu_data_stream.get_data();
                        this->persistent_key_list_transport->recv_boom(data, 2/*pdu_size(2)*/);
                        std::size_t pdu_size = Parse(data).in_uint16_le();
                        this->persistent_key_list_transport->recv_boom(data, pdu_size);

                        pdu_data_stream.out_skip_bytes(pdu_size);

                        if (bool(this->verbose & RDPVerbose::basic_trace)) {
                            InStream stream(data, pdu_size);
                            RDP::PersistentKeyListPDUData pklpdu;
                            pklpdu.receive(stream);
                            pklpdu.log(LOG_INFO, "Send to server");
                        }
                    }
                );
            }
        }
        catch (Error const & e)
        {
            if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
                LOG(LOG_ERR, "mod_rdp::send_persistent_key_list_transparent: error=%u", e.id);
                throw;
            }
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list_transparent done");
        }
    }

    void send_persistent_key_list() {
        if (this->enable_transparent_mode) {
            this->send_persistent_key_list_transparent();
        }
        else {
            this->send_persistent_key_list_regular();
        }
    }

    // TODO CGR: duplicated code in front
    void send_synchronise() {
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::send_synchronise");
        }

        this->send_pdu_type2(
            PDUTYPE2_SYNCHRONIZE, RDP::STREAM_MED,
            [](StreamSize<4>, OutStream & stream) {
                stream.out_uint16_le(1); /* type */
                stream.out_uint16_le(1002);
            }
        );

        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::send_synchronise done");
        }
    }

    void send_fonts(int seq) {
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::send_fonts");
        }

        this->send_pdu_type2(
            PDUTYPE2_FONTLIST, RDP::STREAM_MED,
            [seq](StreamSize<8>, OutStream & stream){
                // Payload
                stream.out_uint16_le(0); /* number of fonts */
                stream.out_uint16_le(0); /* pad? */
                stream.out_uint16_le(seq); /* unknown */
                stream.out_uint16_le(0x32); /* entry size */
            }
        );

        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::send_fonts done");
        }
    }

public:

    BackEvent_t get_signal_event() {
        return this->event.signal;
    }

    void send_input_slowpath(int time, int message_type, int device_flags, int param1, int param2) {
        if (bool(this->verbose & RDPVerbose::input)){
            LOG(LOG_INFO, "mod_rdp::send_input_slowpath");
        }

        if (message_type == RDP_INPUT_SYNCHRONIZE) {
            LOG(LOG_INFO, "mod_rdp::send_input_slowpath: Synchronize Event toggleFlags=0x%X",
                static_cast<unsigned>(param1));
        }

        this->send_pdu_type2(
            PDUTYPE2_INPUT, RDP::STREAM_HI,
            [&](StreamSize<16>, OutStream & stream){
                // Payload
                stream.out_uint16_le(1); /* number of events */
                stream.out_uint16_le(0);
                stream.out_uint32_le(time);
                stream.out_uint16_le(message_type);
                stream.out_uint16_le(device_flags);
                stream.out_uint16_le(param1);
                stream.out_uint16_le(param2);
            }
        );

        if (bool(this->verbose & RDPVerbose::input)){
            LOG(LOG_INFO, "mod_rdp::send_input_slowpath done");
        }
    }

    void send_input_fastpath(int time, int message_type, uint16_t device_flags, int param1, int param2) {
        (void)time;
        if (bool(this->verbose & RDPVerbose::input)) {
            LOG(LOG_INFO, "mod_rdp::send_input_fastpath");
        }

        write_packets(
            this->trans,
            [&](StreamSize<256>, OutStream & stream) {
                switch (message_type) {
                case RDP_INPUT_SCANCODE:
                    FastPath::KeyboardEvent_Send(stream, device_flags, param1);
                    break;

                case RDP_INPUT_UNICODE:
                    FastPath::UniCodeKeyboardEvent_Send(stream, device_flags, param1);
                    break;

                case RDP_INPUT_SYNCHRONIZE:
                    LOG(LOG_INFO, "mod_rdp::send_input_fastpath: Synchronize Event toggleFlags=0x%X",
                        static_cast<unsigned>(param1));

                    FastPath::SynchronizeEvent_Send(stream, param1);
                    break;

                case RDP_INPUT_MOUSE:
                    FastPath::MouseEvent_Send(stream, device_flags, param1, param2);
                    break;

                default:
                    LOG(LOG_ERR, "unsupported fast-path input message type 0x%x", unsigned(message_type));
                    throw Error(ERR_RDP_FASTPATH);
                }
            },
            [&](StreamSize<256>, OutStream & fastpath_header, uint8_t * packet_data, std::size_t packet_size) {
                FastPath::ClientInputEventPDU_Send out_cie(
                    fastpath_header, packet_data, packet_size, 1,
                    this->encrypt, this->encryptionLevel, this->encryptionMethod
                );
                (void)out_cie;
            }
        );

        if (bool(this->verbose & RDPVerbose::input)) {
            LOG(LOG_INFO, "mod_rdp::send_input_fastpath done");
        }
    }

    void send_input(int time, int message_type, int device_flags, int param1, int param2) override {
        if (this->enable_fastpath_client_input_event == false) {
            this->send_input_slowpath(time, message_type, device_flags, param1, param2);
        }
        else {
            this->send_input_fastpath(time, message_type, device_flags, param1, param2);
        }

        if (message_type == RDP_INPUT_SYNCHRONIZE) {
            this->last_key_flags_sent = param1;
        }
    }

    void rdp_input_invalidate(Rect r) override {
        if (bool(this->verbose & RDPVerbose::input)){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate");
        }
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            if (!r.isempty()){
                RDP::RefreshRectPDU rrpdu(this->share_id,
                                          this->userid,
                                          this->encryptionLevel,
                                          this->encrypt);

                rrpdu.addInclusiveRect(r.x, r.y, r.x + r.cx - 1, r.y + r.cy - 1);

                rrpdu.emit(this->trans);
            }
        }
        //this->draw_event(time(nullptr), this->front);
        if (bool(this->verbose & RDPVerbose::input)){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate done");
        }
    }

    void rdp_input_invalidate2(array_view<Rect const> vr) override {
        if (bool(this->verbose & RDPVerbose::input)){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate 2");
        }
        if ((UP_AND_RUNNING == this->connection_finalization_state)
            && (vr.size() > 0)) {
            RDP::RefreshRectPDU rrpdu(this->share_id,
                                      this->userid,
                                      this->encryptionLevel,
                                      this->encrypt);
            for (Rect const & rect : vr) {
                if (!rect.isempty()){
                    rrpdu.addInclusiveRect(rect.x, rect.y, rect.x + rect.cx - 1, rect.y + rect.cy - 1);
                }
            }
            rrpdu.emit(this->trans);
        }
        if (bool(this->verbose & RDPVerbose::input)){
            LOG(LOG_INFO, "mod_rdp::rdp_input_invalidate 2 done");
        }
    }

    void rdp_allow_display_updates(uint16_t left, uint16_t top,
            uint16_t right, uint16_t bottom) override {
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::rdp_allow_display_updates");
        }

        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_pdu_type2(
                PDUTYPE2_SUPPRESS_OUTPUT, RDP::STREAM_MED,
                [left, top, right, bottom](StreamSize<32>, OutStream & stream) {
                    RDP::SuppressOutputPDUData sopdud(left, top, right, bottom);

                    sopdud.emit(stream);
                }
            );
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::rdp_allow_display_updates done");
        }
    }

    void rdp_suppress_display_updates() override {
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::rdp_suppress_display_updates");
        }

        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_pdu_type2(
                PDUTYPE2_SUPPRESS_OUTPUT, RDP::STREAM_MED,
                [](StreamSize<32>, OutStream & stream) {
                    RDP::SuppressOutputPDUData sopdud;

                    sopdud.emit(stream);
                }
            );
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::rdp_suppress_display_updates done");
        }
    }

    void refresh(Rect r) override {
        this->rdp_input_invalidate(r);
    }

    // [referenced from 2.2.9.1.2.1.7 Fast-Path Color Pointer Update (TS_FP_COLORPOINTERATTRIBUTE) ]
    // [referenced from 3.2.5.9.2 Processing Slow-Path Pointer Update PDU]
    // 2.2.9.1.1.4.4 Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
    // =============================================================

    // The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
    // color pointer, as specified in [T128] section 8.14.3. This pointer update
    // is used for both monochrome and color pointers in RDP.

    //    cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
    // entry in the pointer cache in which to store the pointer image. The number
    // of cache entries is specified using the Pointer Capability Set (section 2.2.7.1.5).

    //    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1 ) structure containing
    // the x-coordinates and y-coordinates of the pointer hotspot.

    //    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer
    // in pixels. The maximum allowed pointer width is 96 pixels if the client
    // indicated support for large pointers by setting the LARGE_POINTER_FLAG (0x00000001)
    // in the Large Pointer Capability Set (section 2.2.7.2.7). If the LARGE_POINTER_FLAG
    // was not set, the maximum allowed pointer width is 32 pixels.

    //    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
    // in pixels. The maximum allowed pointer height is 96 pixels if the client
    // indicated support for large pointers by setting the LARGE_POINTER_FLAG (0x00000001)
    // in the Large Pointer Capability Set (section 2.2.7.2.7). If the LARGE_POINTER_FLAG
    // was not set, the maximum allowed pointer height is 32 pixels.

    //    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
    // andMaskData field.

    //    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
    // xorMaskData field.

    //    xorMaskData (variable): A variable-length array of bytes. Contains the 24-bpp,
    // bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte boundary for
    // each encoded scan-line. For example, if a 3x3 pixel cursor is being sent, then each
    // scan-line will consume 10 bytes (3 pixels per scan-line multiplied by 3 bytes per pixel,
    // rounded up to the next even number of bytes).

    //    andMaskData (variable): A variable-length array of bytes. Contains the 1-bpp, bottom-up
    // AND mask scan-line data. The AND mask is padded to a 2-byte boundary for each encoded scan-line.
    // For example, if a 7x7 pixel cursor is being sent, then each scan-line will consume 2 bytes
    // (7 pixels per scan-line multiplied by 1 bpp, rounded up to the next even number of bytes).

    //    pad (1 byte): An optional 8-bit, unsigned integer. Padding. Values in this field MUST be ignored.

    void process_color_pointer_pdu(InStream & stream, gdi::GraphicApi & drawable) {
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu");
        }
        unsigned pointer_cache_idx = stream.in_uint16_le();
        if (pointer_cache_idx >= (sizeof(this->cursors) / sizeof(this->cursors[0]))) {
            LOG(LOG_ERR, "mod_rdp::process_color_pointer_pdu: index out of bounds");
            throw Error(ERR_RDP_PROCESS_COLOR_POINTER_CACHE_NOT_OK);
        }

        auto hotspot_x      = stream.in_uint16_le();
        auto hotspot_y      = stream.in_uint16_le();
        auto width = stream.in_uint16_le();
        auto height = stream.in_uint16_le();
        auto mlen = stream.in_uint16_le(); /* mask length */
        auto dlen = stream.in_uint16_le(); /* data length */
        const uint8_t * data = stream.in_uint8p(dlen);
        const uint8_t * mask = stream.in_uint8p(mlen);

        Pointer cursor(CursorSize{width, height}, Hotspot{hotspot_x, hotspot_y}, {data, dlen}, {mask, mlen});
        this->cursors[pointer_cache_idx] = cursor;

        drawable.set_pointer(cursor);
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO, "mod_rdp::process_color_pointer_pdu done");
        }
    }

    // [ referenced from 3.2.5.9.2 Processing Slow-Path Pointer Update PDU]
    // 2.2.9.1.1.4.6 Cached Pointer Update (TS_CACHEDPOINTERATTRIBUTE)
    // ---------------------------------------------------------------

    // The TS_CACHEDPOINTERATTRIBUTE structure is used to instruct the
    // client to change the current pointer shape to one already present
    // in the pointer cache.

    // cacheIndex (2 bytes): A 16-bit, unsigned integer. A zero-based
    // cache entry containing the cache index of the cached pointer to
    // which the client's pointer MUST be changed. The pointer data MUST
    // have already been cached using either the Color Pointer Update
    // (section 2.2.9.1.1.4.4) or New Pointer Update (section 2.2.9.1.1.4.5).

    void process_cached_pointer_pdu(InStream & stream, gdi::GraphicApi & drawable)
    {
        if (bool(this->verbose & RDPVerbose::graphics_pointer)){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu");
        }

        // TODO Add check that the idx transmitted is actually an used pointer
        uint16_t pointer_idx = stream.in_uint16_le();
        if (pointer_idx >= (sizeof(this->cursors) / sizeof(Pointer))) {
            LOG(LOG_ERR,
                "mod_rdp::process_cached_pointer_pdu pointer cache idx overflow (%d)",
                pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }
        Pointer & cursor = this->cursors[pointer_idx];
        if (cursor.is_valid()) {
            drawable.set_pointer(cursor);
        }
        else {
            LOG(LOG_WARNING,  "mod_rdp::process_cached_pointer_pdu: invalid cache cell index, use system default. index=%u",
                pointer_idx);
        }
        if (bool(this->verbose & RDPVerbose::graphics_pointer)){
            LOG(LOG_INFO, "mod_rdp::process_cached_pointer_pdu done");
        }
    }

    // [ referenced from 3.2.5.9.2 Processing Slow-Path Pointer Update PDU]
    // 2.2.9.1.1.4.3 System Pointer Update (TS_SYSTEMPOINTERATTRIBUTE)
    // ---------------------------------------------------------------

    // systemPointerType (4 bytes): A 32-bit, unsigned integer. The type of system pointer.

    // +---------------------------+-----------------------------+
    // |      Value                |      Meaning                |
    // +---------------------------+-----------------------------+
    // | SYSPTR_NULL    0x00000000 | The hidden pointer.         |
    // +---------------------------+-----------------------------+
    // | SYSPTR_DEFAULT 0x00007F00 | The default system pointer. |
    // +---------------------------+-----------------------------+

    void process_system_pointer_pdu(int system_pointer_type, gdi::GraphicApi & drawable)
    {
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu");
        }
        switch (system_pointer_type) {
        case RDP_NULL_POINTER:
            {
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu - null");
                }
                drawable.set_pointer(Pointer(NullPointer{}));
            }
            break;
        default:
            {
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu - default");
                }
                Pointer cursor(NormalPointer{});
                drawable.set_pointer(cursor);
            }
            break;
        }
        if (bool(this->verbose & RDPVerbose::graphics_pointer)){
            LOG(LOG_INFO, "mod_rdp::process_system_pointer_pdu done");
        }
    }

    // [ referenced from 3.2.5.9.2 Processing Slow-Path Pointer Update PDU]
    // 2.2.9.1.1.4.5 New Pointer Update (TS_POINTERATTRIBUTE)
    // ------------------------------------------------------

    // The TS_POINTERATTRIBUTE structure is used to send pointer data at an arbitrary
    // color depth. Support for the New Pointer Update is advertised in the Pointer
    // Capability Set (section 2.2.7.1.5).


    // xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in bits-per-pixel
    // of the XOR mask contained in the colorPtrAttr field.

    // colorPtrAttr (variable): Encapsulated Color Pointer Update (section 2.2.9.1.1.4.4)
    //  structure which contains information about the pointer. The Color Pointer Update
    //  fields are all used, as specified in section 2.2.9.1.1.4.4; however color XOR data
    //  is presented in the color depth described in the xorBpp field (for 8 bpp, each byte
    //  contains one palette index; for 4 bpp, there are two palette indices per byte).

    void process_new_pointer_pdu(InStream & stream, gdi::GraphicApi & drawable) {
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu");
        }

        unsigned data_bpp  = stream.in_uint16_le(); /* data bpp */
        unsigned pointer_idx = stream.in_uint16_le();
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO,
                "mod_rdp::process_new_pointer_pdu xorBpp=%u pointer_idx=%u",
                data_bpp, pointer_idx);
        }

        if (pointer_idx >= (sizeof(this->cursors) / sizeof(Pointer))) {
            LOG(LOG_ERR,
                "mod_rdp::process_new_pointer_pdu pointer cache idx overflow (%u)",
                pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }

        auto hotspot_x      = stream.in_uint16_le();
        auto hotspot_y      = stream.in_uint16_le();
        auto width            = stream.in_uint16_le();
        auto height            = stream.in_uint16_le();

        uint16_t mlen = stream.in_uint16_le(); /* mask length */
        uint16_t dlen = stream.in_uint16_le(); /* data length */
        if (!stream.in_check_rem(mlen + dlen)){
            LOG(LOG_ERR, "Not enough data for cursor (dlen=%u mlen=%u need=%u remain=%zu)",
                mlen, dlen, static_cast<uint16_t>(mlen+dlen), stream.in_remain());
            throw Error(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK);
        }

        const uint8_t * data = stream.in_uint8p(dlen);
        const uint8_t * mask = stream.in_uint8p(mlen);

        Pointer cursor({width, height}, {hotspot_x, hotspot_y},{data, dlen}, {mask, mlen}, data_bpp, this->orders.global_palette, this->clean_up_32_bpp_cursor, this->bogus_linux_cursor);
        this->cursors[pointer_idx] = cursor;

        drawable.set_pointer(cursor);
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu done");
        }
    }   // process_new_pointer_pdu

private:
    void process_bitmap_updates(InStream & stream, bool fast_path, gdi::GraphicApi & drawable) {
        if (bool(this->verbose & RDPVerbose::graphics)){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates");
        }

        this->recv_bmp_update++;

        if (fast_path) {
            stream.in_skip_bytes(2); // updateType(2)
        }

        // RDP-BCGR: 2.2.9.1.1.3.1.2 Bitmap Update (TS_UPDATE_BITMAP)
        // ----------------------------------------------------------
        // The TS_UPDATE_BITMAP structure contains one or more rectangular
        // clippings taken from the server-side screen frame buffer (see [T128]
        // section 8.17).

        // shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
        // containing information about the packet. The type subfield of the
        // pduType field of the Share Control Header (section 2.2.8.1.1.1.1)
        // MUST be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share
        // Data Header MUST be set to PDUTYPE2_UPDATE (2).

        // bitmapData (variable): The actual bitmap update data, as specified in
        // section 2.2.9.1.1.3.1.2.1.

        // 2.2.9.1.1.3.1.2.1 Bitmap Update Data (TS_UPDATE_BITMAP_DATA)
        // ------------------------------------------------------------
        // The TS_UPDATE_BITMAP_DATA structure encapsulates the bitmap data that
        // defines a Bitmap Update (section 2.2.9.1.1.3.1.2).

        // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update
        // type. This field MUST be set to UPDATETYPE_BITMAP (0x0001).

        // numberRectangles (2 bytes): A 16-bit, unsigned integer.
        // The number of screen rectangles present in the rectangles field.
        size_t numberRectangles = stream.in_uint16_le();
        if (bool(this->verbose & RDPVerbose::graphics)){
            LOG(LOG_INFO, "/* ---------------- Sending %zu rectangles ----------------- */", numberRectangles);
        }

        for (size_t i = 0; i < numberRectangles; i++) {

            // rectangles (variable): Variable-length array of TS_BITMAP_DATA
            // (section 2.2.9.1.1.3.1.2.2) structures, each of which contains a
            // rectangular clipping taken from the server-side screen frame buffer.
            // The number of screen clippings in the array is specified by the
            // numberRectangles field.

            // 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
            // ----------------------------------------------

            // The TS_BITMAP_DATA structure wraps the bitmap data bytestream
            // for a screen area rectangle containing a clipping taken from
            // the server-side screen frame buffer.

            // A 16-bit, unsigned integer. Left bound of the rectangle.

            // A 16-bit, unsigned integer. Top bound of the rectangle.

            // A 16-bit, unsigned integer. Right bound of the rectangle.

            // A 16-bit, unsigned integer. Bottom bound of the rectangle.

            // A 16-bit, unsigned integer. The width of the rectangle.

            // A 16-bit, unsigned integer. The height of the rectangle.

            // A 16-bit, unsigned integer. The color depth of the rectangle
            // data in bits-per-pixel.

            // CGR: As far as I understand we should have
            // align4(right-left) == width and bottom-top == height
            // maybe put some assertion to check it's true
            // LOG(LOG_ERR, "left=%u top=%u right=%u bottom=%u width=%u height=%u bpp=%u", left, top, right, bottom, width, height, bpp);

            // A 16-bit, unsigned integer. The flags describing the format
            // of the bitmap data in the bitmapDataStream field.

            // +-----------------------------------+---------------------------+
            // | 0x0001 BITMAP_COMPRESSION         | Indicates that the bitmap |
            // |                                   | data is compressed. This  |
            // |                                   | implies that the          |
            // |                                   | bitmapComprHdr field is   |
            // |                                   | present if the NO_BITMAP_C|
            // |                                   |OMPRESSION_HDR (0x0400)    |
            // |                                   | flag is not set.          |
            // +-----------------------------------+---------------------------+
            // | 0x0400 NO_BITMAP_COMPRESSION_HDR  | Indicates that the        |
            // |                                   | bitmapComprHdr field is   |
            // |                                   | not present(removed for   |
            // |                                   | bandwidth efficiency to   |
            // |                                   | save 8 bytes).            |
            // +-----------------------------------+---------------------------+

            RDPBitmapData bmpdata;

            bmpdata.receive(stream);

            Rect boundary( bmpdata.dest_left
                           , bmpdata.dest_top
                           , bmpdata.dest_right - bmpdata.dest_left + 1
                           , bmpdata.dest_bottom - bmpdata.dest_top + 1
                           );

            // BITMAP_COMPRESSION 0x0001
            // Indicates that the bitmap data is compressed. This implies
            // that the bitmapComprHdr field is present if the
            // NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not set.

            if (bool(this->verbose & RDPVerbose::graphics)) {
                LOG( LOG_INFO
                     , "/* Rect [%zu] bpp=%" PRIu16
                       " width=%" PRIu16 " height=%" PRIu16
                       " b(%" PRId16 ", %" PRId16 ", %" PRIu16 ", %" PRIu16 ") */"
                     , i
                     , bmpdata.bits_per_pixel
                     , bmpdata.width
                     , bmpdata.height
                     , boundary.x
                     , boundary.y
                     , boundary.cx
                     , boundary.cy
                     );
            }

            // bitmapComprHdr (8 bytes): Optional Compressed Data Header
            // structure (see Compressed Data Header (TS_CD_HEADER)
            // (section 2.2.9.1.1.3.1.2.3)) specifying the bitmap data
            // in the bitmapDataStream. This field MUST be present if
            // the BITMAP_COMPRESSION (0x0001) flag is present in the
            // Flags field, but the NO_BITMAP_COMPRESSION_HDR (0x0400)
            // flag is not.

            if (bmpdata.flags & BITMAP_COMPRESSION) {
                if ((bmpdata.width <= 0) || (bmpdata.height <= 0)) {
                    LOG( LOG_WARNING
                         , "Unexpected bitmap size: width=%" PRIu16 " height=%" PRIu16 " size=%" PRIu16
                           " left=%" PRIu16 ", top=%" PRIu16 ", right=%" PRIu16 ", bottom=%" PRIu16
                         , bmpdata.width
                         , bmpdata.height
                         , bmpdata.cb_comp_main_body_size
                         , bmpdata.dest_left
                         , bmpdata.dest_top
                         , bmpdata.dest_right
                         , bmpdata.dest_bottom
                         );
                }
            }

            // TODO CGR: check which sanity checks should be done
                //            if (bufsize != bitmap.bmp_size){
                //                LOG(LOG_WARNING, "Unexpected bufsize in bitmap received [%u != %u] width=%u height=%u bpp=%u",
                //                    bufsize, bitmap.bmp_size, width, height, bpp);
                //            }
                const uint8_t * data = stream.in_uint8p(bmpdata.bitmap_size());
            Bitmap bitmap( this->orders.bpp
                           , bmpdata.bits_per_pixel
                           , &this->orders.global_palette
                           , bmpdata.width
                           , bmpdata.height
                           , data
                           , bmpdata.bitmap_size()
                           , (bmpdata.flags & BITMAP_COMPRESSION)
                           );

            if (   bmpdata.cb_scan_width
                   && ((bmpdata.cb_scan_width - bitmap.line_size()) >= nbbytes(bitmap.bpp()))) {
                LOG( LOG_WARNING
                     , "Bad line size: line_size=%" PRIu16 " width=%" PRIu16 " height=%" PRIu16 " bpp=%" PRIu16
                     , bmpdata.cb_scan_width
                     , bmpdata.width
                     , bmpdata.height
                     , bmpdata.bits_per_pixel
                     );
            }

            if (   bmpdata.cb_uncompressed_size
                   && (bmpdata.cb_uncompressed_size != bitmap.bmp_size())) {
                LOG( LOG_WARNING
                     , "final_size should be size of decompressed bitmap [%" PRIu16 " != %zu]"
                       " width=%" PRIu16 " height=%" PRIu16 " bpp=%" PRIu16
                     , bmpdata.cb_uncompressed_size
                     , bitmap.bmp_size()
                     , bmpdata.width
                     , bmpdata.height
                     , bmpdata.bits_per_pixel
                     );
            }

            drawable.draw(bmpdata, bitmap);
        }
        if (bool(this->verbose & RDPVerbose::graphics)){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates done");
        }
    }   // process_bitmap_updates

    void send_client_info_pdu(const time_t & now) {
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu");
        }

        InfoPacket infoPacket( this->use_rdp5
                             , this->domain
                             , this->username
                             , this->password
                             , this->program
                             , this->directory
                             , this->performanceFlags
                             , this->clientAddr
                             );
        infoPacket.extendedInfoPacket.clientTimeZone = this->client_time_zone;
        infoPacket.flags |= this->info_packet_extra_flags;

        if (this->perform_automatic_reconnection) {
            InStream in_s(this->server_auto_reconnect_packet_ref.data(),
                this->server_auto_reconnect_packet_ref.size());
            RDP::ServerAutoReconnectPacket server_auto_reconnect_packet;
            server_auto_reconnect_packet.receive(in_s);

            if (bool(this->verbose & RDPVerbose::basic_trace)){
                LOG(LOG_INFO, "Use Server Auto-Reconnect Packet");
                LOG(LOG_INFO, "Server Reconnect Random");
                hexdump(server_auto_reconnect_packet.ArcRandomBits,
                    sizeof(server_auto_reconnect_packet.ArcRandomBits));
            }

            OutStream out_s(infoPacket.extendedInfoPacket.autoReconnectCookie,
                sizeof(infoPacket.extendedInfoPacket.autoReconnectCookie));

            uint8_t digest[SslMd5::DIGEST_LENGTH] = { 0 };

            SslHMAC_Md5 hmac_md5(server_auto_reconnect_packet.ArcRandomBits,
                sizeof(server_auto_reconnect_packet.ArcRandomBits));
            if (!this->nego.enhanced_rdp_security_is_in_effect()) {
                if (bool(this->verbose & RDPVerbose::basic_trace)){
                    LOG(LOG_INFO, "Use client random");
                }
                hmac_md5.update(this->client_random, sizeof(this->client_random));
            }
            else {
                if (bool(this->verbose & RDPVerbose::basic_trace)){
                    LOG(LOG_INFO, "Use NULL client random");
                }
                uint8_t tmp_client_random[32] = { 0 };
                hmac_md5.update(tmp_client_random, sizeof(tmp_client_random));
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
            infoPacket.extendedInfoPacket.cbAutoReconnectLen =
                this->cbAutoReconnectCookie;
            ::memcpy(infoPacket.extendedInfoPacket.autoReconnectCookie, this->autoReconnectCookie,
                sizeof(infoPacket.extendedInfoPacket.autoReconnectCookie));
        }

        this->send_data_request(
            GCC::MCS_GLOBAL_CHANNEL,
            [this, &infoPacket](StreamSize<1024>, OutStream & stream) {
                if (bool(this->rdp_compression)) {
                    infoPacket.flags |= INFO_COMPRESSION;
                    infoPacket.flags &= ~CompressionTypeMask;
                    infoPacket.flags |= (static_cast<unsigned>(this->rdp_compression) - 1) << 9;
                }

                if (this->enable_session_probe) {
                    infoPacket.flags &= ~INFO_MAXIMIZESHELL;
                }

                if (this->remote_program) {
                    infoPacket.flags |= INFO_RAIL;

                    // if (this->remote_program_enhanced) {
                    //    infoPacket.flags |= INFO_HIDEF_RAIL_SUPPORTED;
                    // }
                }

                infoPacket.emit(stream);

            },
            write_sec_send_fn{SEC::SEC_INFO_PKT, this->encrypt, this->encryptionLevel}
        );
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            infoPacket.log("Send data request", this->password_printing_mode, !this->enable_session_probe);
        }

        if (this->open_session_timeout.count()) {
            this->open_session_timeout_checker.restart_timeout(
                now, this->open_session_timeout.count());
            this->event.set_trigger_time(1000000);
        }
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "mod_rdp::send_client_info_pdu done");
        }
    }

public:
    bool is_up_and_running() override {
        return (UP_AND_RUNNING == this->connection_finalization_state);
    }

    void disconnect(time_t now) override {
        if (this->is_up_and_running()) {
            if (bool(this->verbose & RDPVerbose::basic_trace)){
                LOG(LOG_INFO, "mod_rdp::disconnect()");
            }
            // this->send_shutdown_request();
            // this->draw_event(time(nullptr));
            this->send_disconnect_ultimatum();
        }
        if (!this->session_disconnection_logged) {
            double seconds = ::difftime(now, this->beginning);

            char extra[1024];
            snprintf(extra, sizeof(extra), "%d:%02d:%02d",
                (int(seconds) / 3600), ((int(seconds) % 3600) / 60),
                (int(seconds) % 60));

            auto info = key_qvalue_pairs({
                {"type", "SESSION_DISCONNECTION"},
                {"duration", extra},
                });

            this->report_message.log5(info);

            this->session_disconnection_logged = true;
        }
    }

private:
    //void send_shutdown_request() {
    //    LOG(LOG_INFO, "SEND SHUTDOWN REQUEST PDU");
    //
    //    BStream stream(65536);
    //    ShareData sdata(stream);
    //    sdata.emit_begin(PDUTYPE2_SHUTDOWN_REQUEST, this->share_id,
    //                     RDP::STREAM_MED);
    //    sdata.emit_end();
    //    BStream sctrl_header(256);
    //    ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU,
    //                      this->userid + GCC::MCS_USERCHANNEL_BASE,
    //                      stream.size());
    //    HStream target_stream(1024, 65536);
    //    target_stream.out_copy_bytes(sctrl_header);
    //    target_stream.out_copy_bytes(stream);
    //    target_stream.mark_end();
    //
    //    this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    //}

    void send_disconnect_ultimatum() {
        if (bool(this->verbose & RDPVerbose::basic_trace)){
            LOG(LOG_INFO, "SEND MCS DISCONNECT PROVIDER ULTIMATUM PDU");
        }
        write_packets(
            this->trans,
            [](StreamSize<256>, OutStream & mcs_data) {
                MCS::DisconnectProviderUltimatum_Send(mcs_data, 3, MCS::PER_ENCODING);
            },
            write_x224_dt_tpdu_fn{}
        );
    }

    //void send_flow_response_pdu(uint8_t flow_id, uint8_t flow_number) {
    //    LOG(LOG_INFO, "SEND FLOW RESPONSE PDU n° %u", flow_number);
    //    BStream flowpdu(256);
    //    FlowPDU_Send(flowpdu, FLOW_RESPONSE_PDU, flow_id, flow_number,
    //                 this->userid + GCC::MCS_USERCHANNEL_BASE);
    //    HStream target_stream(1024, 65536);
    //    target_stream.out_copy_bytes(flowpdu);
    //    target_stream.mark_end();
    //    this->send_data_request_ex(GCC::MCS_GLOBAL_CHANNEL, target_stream);
    //}

    void process_auth_event(
        const CHANNELS::ChannelDef & auth_channel,
        InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size
    ) {
        (void)length;
        (void)chunk_size;
        assert(stream.in_remain() == chunk_size);

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) !=
            (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST))
        {
            LOG(LOG_WARNING, "mod_rdp::process_auth_event: Chunked Virtual Channel Data ignored!");
            return;
        }

        std::string auth_channel_message(char_ptr_cast(stream.get_current()), stream.in_remain());

        this->auth_channel_flags  = flags;
        this->auth_channel_chanid = auth_channel.chanid;

        const char Log[] = "Log=";

        if (!auth_channel_message.compare(0, sizeof(Log) - 1, Log)) {
            const char * log_string =
                (auth_channel_message.c_str() + sizeof(Log) - 1);
            LOG(LOG_INFO, "WABLauncher: %s", log_string);
        }
        else {
            LOG(LOG_INFO, "Auth channel data=\"%s\"", auth_channel_message);

            this->authentifier.set_auth_channel_target(
                auth_channel_message.c_str());
        }
    }

    void process_checkout_event(
        const CHANNELS::ChannelDef & checkout_channel,
        InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size
    ) {
        (void)length;
        (void)chunk_size;
        assert(stream.in_remain() == chunk_size);

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) !=
            (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST))
        {
            LOG(LOG_WARNING, "mod_rdp::process_checkout_event: Chunked Virtual Channel Data ignored!");
            return;
        }

        {
            const unsigned expected = 4;    // Version(2) + DataLength(2)
            if (!stream.in_check_rem(expected)) {
                LOG( LOG_ERR
                   , "mod_rdp::process_checkout_event: data truncated (1), expected=%u remains=%zu"
                   , expected, stream.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        uint16_t const version = stream.in_uint16_le();
        uint16_t const data_length = stream.in_uint16_le();

        LOG(LOG_INFO, "mod_rdp::process_checkout_event: Version=%u DataLength=%u", version, data_length);

        std::string checkout_channel_message(char_ptr_cast(stream.get_current()), stream.in_remain());

        this->checkout_channel_flags  = flags;
        this->checkout_channel_chanid = checkout_channel.chanid;

        LOG(LOG_INFO, "mod_rdp::process_checkout_event: Data=\"%s\"", checkout_channel_message);

        send_checkout_channel_data("{ \"ReturnCode\": 0, \"ReturnMessage\": \"Succeeded.\" }");
    }

    void process_session_probe_event(
        const CHANNELS::ChannelDef & session_probe_channel,
        InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size
    ) {
        (void)session_probe_channel;
        SessionProbeVirtualChannel& channel = this->get_session_probe_virtual_channel();

        std::unique_ptr<AsynchronousTask> out_asynchronous_task;

        channel.process_server_message(length, flags, stream.get_current(), chunk_size,
            out_asynchronous_task);

        assert(!out_asynchronous_task);
    }

    void process_cliprdr_event(
        const CHANNELS::ChannelDef & cliprdr_channel, InStream & stream,
        uint32_t length, uint32_t flags, size_t chunk_size
    ) {
        (void)cliprdr_channel;
        ClipboardVirtualChannel& channel = this->get_clipboard_virtual_channel();

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            InStream clone = stream.clone();
            RDPECLIP::streamLogCliprdr(clone, flags, this->cliprdrLogStatus);
        }

        std::unique_ptr<AsynchronousTask> out_asynchronous_task;

        channel.process_server_message(length, flags, stream.get_current(), chunk_size,
            out_asynchronous_task);

        assert(!out_asynchronous_task);
    }   // process_cliprdr_event

    void process_rail_event(const CHANNELS::ChannelDef & rail_channel,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        (void)rail_channel;
        RemoteProgramsVirtualChannel& channel = this->get_remote_programs_virtual_channel();

        std::unique_ptr<AsynchronousTask> out_asynchronous_task;

        channel.process_server_message(length, flags, stream.get_current(), chunk_size,
            out_asynchronous_task);

        assert(!out_asynchronous_task);
    }

    void process_rdpdr_event(const CHANNELS::ChannelDef &,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        if (!this->enable_rdpdr_data_analysis &&
            this->authorization_channels.rdpdr_type_all_is_authorized() &&
            !this->file_system_drive_manager.HasManagedDrive()) {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                if (bool(this->verbose & (RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump))) {

                    LOG(LOG_INFO,
                        "mod_rdp::process_rdpdr_event: sending to Client, "
                            "send Chunked Virtual Channel Data transparently.");
                }

                if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                    const bool send              = false;
                    const bool from_or_to_client = false;

                    ::msgdump_d(send, from_or_to_client, length, flags,
                        stream.get_data()+8, chunk_size);

                    rdpdr::streamLog(stream, this->rdpdrLogStatus);
                }
            }

            this->send_to_front_channel(
                channel_names::rdpdr, stream.get_current(), length, chunk_size, flags);
            return;
        }

        FileSystemVirtualChannel& channel = this->get_file_system_virtual_channel();

        std::unique_ptr<AsynchronousTask> out_asynchronous_task;

        channel.process_server_message(length, flags, stream.get_current(), chunk_size,
            out_asynchronous_task);

        if (out_asynchronous_task) {
            if (this->asynchronous_tasks.empty()) {
                this->asynchronous_task_event.full_reset();

                out_asynchronous_task->configure_wait_object(this->asynchronous_task_event);
            }

            this->asynchronous_tasks.push_back(std::move(out_asynchronous_task));
        }
    }

    void process_drdynvc_event(const CHANNELS::ChannelDef &,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size) {
        // if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
        //     if (bool(this->verbose & (RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump))) {

        //         LOG(LOG_INFO,
        //             "mod_rdp::process_drdynvc_event: sending to Client, "
        //                 "send Chunked Virtual Channel Data transparently.");
        //     }

        //     if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
        //         const bool send              = false;
        //         const bool from_or_to_client = false;

        //         ::msgdump_d(send, from_or_to_client, length, flags,
        //             stream.get_data()+8, chunk_size);

        //         rdpdr::streamLog(stream, this->rdpdrLogStatus);
        //     }
        // }

        // this->send_to_front_channel(
        //     channel_names::rdpdr, stream.get_current(), length, chunk_size, flags);
        // return;


        DynamicChannelVirtualChannel& channel = this->get_dynamic_channel_virtual_channel();

        std::unique_ptr<AsynchronousTask> out_asynchronous_task;

        channel.process_server_message(length, flags, stream.get_current(), chunk_size,
            out_asynchronous_task);

        if (out_asynchronous_task) {
            if (this->asynchronous_tasks.empty()) {
                this->asynchronous_task_event.full_reset();

                out_asynchronous_task->configure_wait_object(this->asynchronous_task_event);
            }

            this->asynchronous_tasks.push_back(std::move(out_asynchronous_task));
        }
    }

    bool disable_input_event_and_graphics_update(bool disable_input_event,
            bool disable_graphics_update) override {
        bool need_full_screen_update =
            (this->graphics_update_disabled && !disable_graphics_update);

        if (this->input_event_disabled != disable_input_event) {
            LOG(LOG_INFO, "Mod_rdp: %s input event.",
                (disable_input_event ? "Disable" : "Enable"));
        }
        if (this->graphics_update_disabled != disable_graphics_update) {
            LOG(LOG_INFO, "Mod_rdp: %s graphics update.",
                (disable_graphics_update ? "Disable" : "Enable"));
        }

        this->input_event_disabled     = disable_input_event;
        this->graphics_update_disabled = disable_graphics_update;

        if (this->remote_programs_session_manager) {
            this->remote_programs_session_manager->disable_graphics_update(
                disable_graphics_update);
        }

        return need_full_screen_update;
    }

    void do_enable_session_probe() {
        if (this->enable_session_probe) {
            ClipboardVirtualChannel& cvc =
                this->get_clipboard_virtual_channel();
            if (this->session_probe_launcher) {
                cvc.set_session_probe_launcher(
                    this->session_probe_launcher.get());
            }

            FileSystemVirtualChannel& fsvc =
                this->get_file_system_virtual_channel();
            if (this->session_probe_launcher) {
                fsvc.set_session_probe_launcher(
                    this->session_probe_launcher.get());
            }

            if (this->session_probe_launcher) {
                this->file_system_drive_manager.set_session_probe_launcher(
                    this->session_probe_launcher.get());
            }

            SessionProbeVirtualChannel& spvc =
                this->get_session_probe_virtual_channel();
            if (this->session_probe_launcher) {
                spvc.set_session_probe_launcher(this->session_probe_launcher.get());
            }
            this->session_probe_virtual_channel_p = &spvc;
            if (!this->session_probe_start_launch_timeout_timer_only_after_logon) {
                spvc.start_launch_timeout_timer();
            }

            if (this->session_probe_launcher) {
                this->session_probe_launcher->set_clipboard_virtual_channel(
                    &cvc);

                this->session_probe_launcher->set_session_probe_virtual_channel(
                    this->session_probe_virtual_channel_p);
            }

            if (this->remote_program) {
                RemoteProgramsVirtualChannel& rpvc =
                    this->get_remote_programs_virtual_channel();

                rpvc.set_session_probe_virtual_channel(
                    this->session_probe_virtual_channel_p);

                if (this->session_probe_launcher) {
                    rpvc.set_session_probe_launcher(
                        this->session_probe_launcher.get());

                    this->session_probe_launcher->set_remote_programs_virtual_channel(
                        &rpvc);
                }
            }
        }
    }

public:
    windowing_api* get_windowing_api() const {
        if (this->remote_programs_session_manager) {
            return this->remote_programs_session_manager.get();
        }

        return nullptr;
    }

    Dimension get_dim() const override
    { return Dimension(this->front_width, this->front_height); }

    bool is_auto_reconnectable() override {
        return (this->is_server_auto_reconnec_packet_received &&
            this->is_up_and_running() &&
            (!this->session_probe_launcher || this->session_probe_launcher->is_stopped()));
    }

private:
    void auth_rail_exec(uint16_t flags, const char* original_exe_or_file,
            const char* exe_or_file, const char* working_dir,
            const char* arguments, const char* account, const char* password) override {
        if (this->remote_program) {
            RemoteProgramsVirtualChannel& rpvc =
                this->get_remote_programs_virtual_channel();

            rpvc.auth_rail_exec(flags, original_exe_or_file, exe_or_file,
                working_dir, arguments, account, password);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::auth_rail_exec(): Current session has no Remote Program Virtual Channel");
        }
    }

    void auth_rail_exec_cancel(uint16_t flags, const char* original_exe_or_file,
            uint16_t exec_result) override {
        if (this->remote_program) {
            RemoteProgramsVirtualChannel& rpvc =
                this->get_remote_programs_virtual_channel();

            rpvc.auth_rail_exec_cancel(flags, original_exe_or_file, exec_result);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::auth_rail_exec(): Current session has no Remote Program Virtual Channel");
        }
    }

    void sespro_rail_exec_result(uint16_t flags, const char* exe_or_file,
        uint16_t exec_result, uint32_t raw_result) override {
        if (this->remote_program) {
            RemoteProgramsVirtualChannel& rpvc =
                this->get_remote_programs_virtual_channel();

            rpvc.sespro_rail_exec_result(flags, exe_or_file, exec_result, raw_result);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::sespro_rail_exec_result(): Current session has no Remote Program Virtual Channel");
        }
    }
};
