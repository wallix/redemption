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

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"

#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/RDP/PersistentKeyListPDU.hpp"
#include "core/RDP/RefreshRectPDU.hpp"
#include "core/RDP/SaveSessionInfoPDU.hpp"
#include "core/RDP/ServerRedirection.hpp"
#include "core/RDP/SuppressOutputPDU.hpp"
#include "core/RDP/autoreconnect.hpp"
#include "core/RDP/bitmapupdate.hpp"

#include "core/RDP/capabilities/activate.hpp"
#include "core/RDP/capabilities/bitmapcachehostsupport.hpp"
#include "core/RDP/capabilities/bitmapcodecs.hpp"
#include "core/RDP/capabilities/bmpcache2.hpp"
#include "core/RDP/capabilities/cap_bitmap.hpp"
#include "core/RDP/capabilities/cap_bmpcache.hpp"
#include "core/RDP/capabilities/cap_brushcache.hpp"
#include "core/RDP/capabilities/cap_font.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/capabilities/cap_share.hpp"
#include "core/RDP/capabilities/cap_sound.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/capabilities/compdesk.hpp"
#include "core/RDP/capabilities/control.hpp"
#include "core/RDP/capabilities/drawgdiplus.hpp"
#include "core/RDP/capabilities/frameacknowledge.hpp"
#include "core/RDP/capabilities/input.hpp"
#include "core/RDP/capabilities/largepointer.hpp"
#include "core/RDP/capabilities/multifragmentupdate.hpp"
#include "core/RDP/capabilities/order.hpp"
#include "core/RDP/capabilities/pointer.hpp"
#include "core/RDP/capabilities/surfacecommands.hpp"
#include "core/RDP/capabilities/window.hpp"

#include "core/RDP/gcc/userdata/mcs_channels.hpp"
#include "core/RDP/gcc/userdata/cs_net.hpp"

#include "core/RDP/clipboard.hpp"
#include "core/RDP/fastpath.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/mppc.hpp"
#include "core/RDP/protocol.hpp"
#include "core/RDP/remote_programs.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/sec.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/RDP/x224.hpp"
#include "core/RDP/orders/remoteFx.hpp"
#include "core/RDP/orders/RDPSurfaceCommands.hpp"
#include "core/RDP/virtual_channel_pdu.hpp"
#include "core/RDP/windows_execute_shell_params.hpp"

#include "core/RDPEA/audio_output.hpp"

#include "utils/timebase.hpp"
#include "core/log_id.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "gdi/screen_functions.hpp"
#include "gdi/osd_api.hpp"

#ifndef __EMSCRIPTEN__
// TODO: annoying as it introduces a dependency on openssl in rdp mod
// see how to extract something more abstract where it's used
// check : enable_server_cert_external_validation workflow
#include "system/tls_cert_to_escaped_string.hpp"
#endif

#ifdef __EMSCRIPTEN__
class RDPMetrics;
struct FileValidatorService;
# define IF_ENABLE_METRICS(m) do {} while(0)
# include "mod/rdp/windowing_api.hpp"
#else
# include "mod/rdp/rdp_metrics.hpp"
# include "mod/file_validator_service.hpp"
# define IF_ENABLE_METRICS(m) do { if (this->metrics) this->metrics->m; } while (0)
# include "mod/rdp/channels/rail_session_manager.hpp"
# include "mod/rdp/channels/rail_channel.hpp"
# include "mod/rdp/channels/sespro_alternate_shell_based_launcher.hpp"
# include "mod/rdp/channels/sespro_channel.hpp"
# include "mod/rdp/channels/sespro_clipboard_based_launcher.hpp"
# include "mod/rdp/channels/cliprdr_channel.hpp"
# include "mod/rdp/channels/validator_params.hpp"
# include "mod/rdp/channels/clipboard_virtual_channels_params.hpp"
# include "mod/rdp/channels/drdynvc_channel.hpp"
# include "mod/rdp/channels/rdpdr_channel.hpp"
# include "mod/rdp/channels/rdpdr_asynchronous_task.hpp"
# include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
# include "mod/rdp/channels/asynchronous_task_container.hpp"
# include "core/RDP/channels/rdpdr.hpp"
# include "core/RDP/capabilities/rail.hpp"
# include "RAIL/client_execute.hpp"
#endif

#include "mod/mod_api.hpp"

#include "mod/rdp/mod_rdp_factory.hpp"
#include "mod/rdp/mod_rdp_variables.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "mod/rdp/rdp_negociation_data.hpp"
#include "mod/rdp/rdp_orders.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/server_transport_context.hpp"

#include "core/channels_authorizations.hpp"
#include "utils/genrandom.hpp"
#include "utils/parse_primary_drawing_orders.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/splitter.hpp"
#include "mod/rdp/rdp_negociation.hpp"
#include "acl/auth_api.hpp"
#include "configs/config.hpp"


class mod_rdp_channels
{
public:
    CHANNELS::ChannelDefArray mod_channel_list;

    const ChannelsAuthorizations channels_authorizations;

public:
    const bool enable_auth_channel;
    const CHANNELS::ChannelNameId auth_channel;
    const CHANNELS::ChannelNameId checkout_channel;
#ifndef __EMSCRIPTEN__
    int auth_channel_flags      = 0;
    int auth_channel_chanid     = 0;
    int checkout_channel_flags  = 0;
    int checkout_channel_chanid = 0;
    RDPMetrics * metrics;

private:
    Random & gen;

public:
    struct SessionProbe
    {
        const bool enable_session_probe;
        const bool enable_launch_mask;
        const bool start_launch_timeout_timer_only_after_logon;


    private:
        const bool used_to_launch_remote_program;

    public:
        std::unique_ptr<SessionProbeLauncher> session_probe_launcher;

    private:
        const SessionProbeVirtualChannelParams session_probe_channel_params;

        std::string target_informations;

        struct ChannelParams
        {
            std::string real_alternate_shell;
            std::string real_working_dir;
        } channel_params;

    public:
        SessionProbe(ModRdpSessionProbeParams const& sespro_params)
        : enable_session_probe(sespro_params.enable_session_probe)
        , enable_launch_mask(sespro_params.enable_launch_mask && this->enable_session_probe)
        , start_launch_timeout_timer_only_after_logon(sespro_params.start_launch_timeout_timer_only_after_logon)
        , used_to_launch_remote_program(sespro_params.used_to_launch_remote_program)
        , session_probe_channel_params(sespro_params.vc_params)
        {}

        friend class mod_rdp_channels;
    } session_probe;

    struct RemoteApp
    {
        const bool enable_remote_program;

        const bool perform_automatic_reconnection;

    private:
        const bool remote_program_enhanced;

    public:
        const bool convert_remoteapp_to_desktop;

    private:
        const bool should_ignore_first_client_execute;

    public:
        const std::chrono::milliseconds bypass_legal_notice_delay;
        const std::chrono::milliseconds bypass_legal_notice_timeout;

    private:
        WindowsExecuteShellParams windows_execute_shell_params;
        WindowsExecuteShellParams real_windows_execute_shell_params;

    public:
        RemoteApp(ModRDPParams::RemoteAppParams const& remote_app_params, bool perform_automatic_reconnection)
        : enable_remote_program(remote_app_params.enable_remote_program)
        , perform_automatic_reconnection(perform_automatic_reconnection)
        , remote_program_enhanced(remote_app_params.remote_program_enhanced)
        , convert_remoteapp_to_desktop(remote_app_params.convert_remoteapp_to_desktop)
        , should_ignore_first_client_execute(
            remote_app_params.should_ignore_first_client_execute)
        , bypass_legal_notice_delay(remote_app_params.bypass_legal_notice_delay)
        , bypass_legal_notice_timeout(remote_app_params.bypass_legal_notice_timeout)
        {}

        friend class mod_rdp_channels;
    } remote_app;

private:
    struct Clipboard
    {
        const bool disable_log_syslog;
        const bool log_only_relevant_activities;

        Clipboard(ModRDPParams::ClipboardParams const& clipboard_params)
        : disable_log_syslog(clipboard_params.disable_log_syslog)
        , log_only_relevant_activities(clipboard_params.log_only_relevant_activities)
        {}
    } clipboard;

    struct DynamicChannels
    {
        const char * allowed_channels;
        const char * denied_channels;

        DynamicChannels(ModRDPParams::DynamicChannelsParams const& dynamic_channels_params)
        : allowed_channels(dynamic_channels_params.allowed_channels)
        , denied_channels(dynamic_channels_params.denied_channels)
        {}
    } dynamic_channels;

public:
    struct FileSystem
    {
        const bool disable_log_syslog;
        const bool bogus_ios_rdpdr_virtual_channel;

        const bool enable_rdpdr_data_analysis;

        const bool smartcard_passthrough;

        FileSystem(ModRDPParams::FileSystemParams const& file_system_params)
        : disable_log_syslog(file_system_params.disable_log_syslog)
        , bogus_ios_rdpdr_virtual_channel(file_system_params.bogus_ios_rdpdr_virtual_channel)
        , enable_rdpdr_data_analysis(file_system_params.enable_rdpdr_data_analysis)
        , smartcard_passthrough(file_system_params.smartcard_passthrough)
        {}
    } file_system;

    AsynchronousTaskContainer asynchronous_tasks;

    class Drive
    {
        const bool use_application_driver;
        const std::string proxy_managed_prefix;

    public:
        FileSystemDriveManager file_system_drive_manager;

        Drive(
            ApplicationParams const& application_params,
            ModRDPParams::DriveParams const& drive_params,
            AsynchronousTaskContainer& asynchronous_tasks,
            RDPVerbose verbose)
        : use_application_driver(
            !::strncasecmp(application_params.alternate_shell.c_str(), "\\\\tsclient\\SESPRO\\AppDriver.exe", 31))
        , proxy_managed_prefix(drive_params.proxy_managed_prefix)
        , file_system_drive_manager(asynchronous_tasks, verbose)
        {
            if (drive_params.proxy_managed_drives && *drive_params.proxy_managed_drives) {
                if (bool(verbose & RDPVerbose::connection)) {
                    LOG(LOG_INFO, "Proxy managed drives=\"%s\"",
                        drive_params.proxy_managed_drives);
                }

                for (auto & r : get_line(drive_params.proxy_managed_drives, ',')) {
                    auto const trimmed_range = trim(r);

                    if (trimmed_range.empty()) continue;

                    if (bool(verbose & RDPVerbose::connection)) {
                        LOG(LOG_INFO, "Proxy managed drive=\"%.*s\"",
                            int(trimmed_range.size()), trimmed_range.begin());
                    }

                    this->file_system_drive_manager.enable_drive(
                        FileSystemDriveManager::DriveName(
                            std::string_view{trimmed_range.begin(), trimmed_range.size()}),
                        this->proxy_managed_prefix);
                }
            }
        }

        friend class mod_rdp_channels;
    } drive;

    ModRdpFactory& mod_rdp_factory;

    std::unique_ptr<VirtualChannelDataSender>     clipboard_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     clipboard_to_server_sender;

    std::unique_ptr<ClipboardVirtualChannel>      clipboard_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     file_system_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     file_system_to_server_sender;

    std::unique_ptr<FileSystemVirtualChannel>     file_system_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     dynamic_channel_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     dynamic_channel_to_server_sender;

    std::unique_ptr<DynamicChannelVirtualChannel> dynamic_channel_virtual_channel;

    std::unique_ptr<VirtualChannelDataSender>     session_probe_to_server_sender;

public:
    std::unique_ptr<SessionProbeVirtualChannel>   session_probe_virtual_channel;

private:
    std::unique_ptr<VirtualChannelDataSender>     remote_programs_to_client_sender;
    std::unique_ptr<VirtualChannelDataSender>     remote_programs_to_server_sender;

    std::unique_ptr<RemoteProgramsVirtualChannel> remote_programs_virtual_channel;

public:
    std::unique_ptr<RemoteProgramsSessionManager> remote_programs_session_manager;

private:
    RDPECLIP::CliprdrLogState cliprdrLogStatus;
    rdpdr::RdpDrStatus rdpdrLogStatus;

    const RDPVerbose verbose;

    gdi::OsdApi & osd;
    EventContainer & events;
    SessionLogApi& session_log;
    FileValidatorService * file_validator_service;
    ValidatorParams validator_params;
    SessionProbeVirtualChannel::Callbacks & callbacks;

public:
    mod_rdp_channels(
        const ChannelsAuthorizations & channels_authorizations,
        const ModRDPParams & mod_rdp_params, const RDPVerbose verbose,
        Random & gen, RDPMetrics * metrics,
        gdi::OsdApi & osd,
        EventContainer & events,
        SessionLogApi& session_log,
        FileValidatorService * file_validator_service,
        ModRdpFactory& mod_rdp_factory,
        SessionProbeVirtualChannel::Callbacks & callbacks)
    : channels_authorizations(channels_authorizations)
    , enable_auth_channel(mod_rdp_params.application_params.alternate_shell[0]
                        && !mod_rdp_params.ignore_auth_channel)
    , auth_channel(mod_rdp_params.auth_channel)
    , checkout_channel(mod_rdp_params.checkout_channel)
    , metrics(metrics)
    , gen(gen)
    , session_probe(mod_rdp_params.session_probe_params)
    , remote_app(mod_rdp_params.remote_app_params, mod_rdp_params.perform_automatic_reconnection)
    , clipboard(mod_rdp_params.clipboard_params)
    , dynamic_channels(mod_rdp_params.dynamic_channels_params)
    , file_system(mod_rdp_params.file_system_params)
    , asynchronous_tasks(events)
    , drive(mod_rdp_params.application_params, mod_rdp_params.drive_params, asynchronous_tasks, verbose)
    , mod_rdp_factory(mod_rdp_factory)
    , verbose(verbose)
    , osd(osd)
    , events(events)
    , session_log(session_log)
    , file_validator_service(file_validator_service)
    , validator_params(mod_rdp_params.validator_params)
    , callbacks(callbacks)
    {}

#ifndef __EMSCRIPTEN__
    void DLP_antivirus_check_channels_files()
    {
        if (this->clipboard_virtual_channel) {
            this->clipboard_virtual_channel->DLP_antivirus_check_channels_files();
        }
    }
#endif

    void init_remote_program_and_session_probe(
        gdi::GraphicApi& gd,
        mod_api & mod_rdp,
        const ModRDPParams & mod_rdp_params,
        const ClientInfo & info,
        // TODO OutParam<...>
        char (&program)[512],
        char (&directory)[512])
    {
        // TODO: to make init code clearer we would prefer to have to consecutive inits
        // - one for remote_program initialisation
        // - one for session probe initialisation

        // Something like:

        // if probe: init_session_probe(... time_base);
        // if remote_prog: init_remote_program(... lang, font, identifier, program, directory);

        // This could probably work like two consecutive filters
        // one to prepare part of the context, the other used to prepare the remaining context.
        // There should be a way to prepare some objects useful for the remaining work to do

        if (this->remote_app.enable_remote_program) {
            char session_probe_window_title[32] = { 0 };
            uint32_t const r = this->gen.rand32();

            snprintf(session_probe_window_title,
                sizeof(session_probe_window_title),
                "%X%X%X%X",
                (r & 0xFF000000) >> 24,
                (r & 0x00FF0000) >> 16,
                (r & 0x0000FF00) >> 8,
                 r & 0x000000FF
            );

            if (mod_rdp_params.session_probe_params.enable_session_probe) {
                this->init_remote_program_with_session_probe(
                    mod_rdp_params.remote_app_params,
                    mod_rdp_params.application_params,
                    mod_rdp_params.session_probe_params,
                    session_probe_window_title);
            }
            else {
                this->init_remote_program_without_session_probe(
                    mod_rdp_params.remote_app_params,
                    mod_rdp_params.application_params);
            }

            this->remote_programs_session_manager = std::make_unique<RemoteProgramsSessionManager>(
                this->events, gd, mod_rdp, mod_rdp_params.lang,
                mod_rdp_params.font, mod_rdp_params.theme, session_probe_window_title,
                mod_rdp_params.remote_app_params.rail_client_execute,
                mod_rdp_params.remote_app_params.rail_disconnect_message_delay,
                this->verbose
            );
        }
        else {
            if (mod_rdp_params.session_probe_params.enable_session_probe) {
                this->init_no_remote_program_with_session_probe(
                    mod_rdp, info, mod_rdp_params.application_params, mod_rdp_params.session_probe_params, program, directory);
            }
            else  {
                this->init_no_remote_program_no_session_probe(
                    info, mod_rdp_params.application_params, program, directory);
            }
        }
    }

private:
    std::unique_ptr<VirtualChannelDataSender> create_to_client_sender(
        CHANNELS::ChannelNameId channel_name, FrontAPI& front) const
    {
        if (!this->channels_authorizations.is_authorized(channel_name))
        {
            return nullptr;
        }

        const CHANNELS::ChannelDefArray& front_channel_list = front.get_channel_list();

        const CHANNELS::ChannelDef* channel = front_channel_list.get_by_name(channel_name);
        if (!channel)
        {
            return nullptr;
        }

        class ToClientSender : public VirtualChannelDataSender
        {
            FrontAPI& front;
            const CHANNELS::ChannelDef& channel;
            const bool verbose;

        public:
            explicit ToClientSender(
                FrontAPI& front,
                const CHANNELS::ChannelDef& channel,
                bool verbose) noexcept
            : front(front)
            , channel(channel)
            , verbose(verbose)
            {}

            void operator()(uint32_t total_length, uint32_t flags,
                bytes_view chunk_data) override
            {
                if (this->verbose) {
                    const bool send              = true;
                    const bool from_or_to_client = true;
                    ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
                }

                this->front.send_to_channel(this->channel, chunk_data, total_length, flags);
            }
        };

        bool const verbose
          = bool(this->verbose & (RDPVerbose::cliprdr_dump | RDPVerbose::rdpdr_dump))
          && (channel_name == channel_names::cliprdr || channel_name == channel_names::rdpdr);
        return std::make_unique<ToClientSender>(front, *channel, verbose);
    }

    inline void create_clipboard_virtual_channel(FrontAPI & front, ServerTransportContext & stc, FileValidatorService * file_validator_service) {
        assert(!this->clipboard_to_client_sender
            && !this->clipboard_to_server_sender);

        this->clipboard_to_client_sender = this->create_to_client_sender(channel_names::cliprdr, front);
        this->clipboard_to_server_sender = this->create_to_server_synchronous_sender(channel_names::cliprdr, stc);

        ClipboardVirtualChannelParams cvc_params;
        cvc_params.clipboard_down_authorized = this->channels_authorizations.cliprdr_down_is_authorized();
        cvc_params.clipboard_up_authorized   = this->channels_authorizations.cliprdr_up_is_authorized();
        cvc_params.clipboard_file_authorized = this->channels_authorizations.cliprdr_file_is_authorized();
        cvc_params.dont_log_data_into_syslog = this->clipboard.disable_log_syslog;
        cvc_params.log_only_relevant_clipboard_activities = this->clipboard.log_only_relevant_activities;
        cvc_params.validator_params = this->validator_params;

        this->clipboard_virtual_channel = std::make_unique<ClipboardVirtualChannel>(
            this->clipboard_to_client_sender.get(),
            this->clipboard_to_server_sender.get(),
            this->events,
            this->osd,
            std::move(cvc_params),
            file_validator_service,
            ClipboardVirtualChannel::FileStorage{
                this->mod_rdp_factory.get_fdx_capture(),
                this->mod_rdp_factory.always_file_storage,
                this->mod_rdp_factory.tmp_dir,
            },
            this->session_log,
            this->verbose
        );
    }

    class ToServerSender : public VirtualChannelDataSender
    {
        ServerTransportContext stc;
        const uint16_t channel_id;
        const bool     show_protocol;
        const bool     verbose;

    public:
        explicit ToServerSender(
            ServerTransportContext & stc,
            uint16_t channel_id,
            bool show_protocol,
            bool verbose) noexcept
        : stc(stc)
        , channel_id(channel_id)
        , show_protocol(show_protocol)
        , verbose(verbose)
        {}

        void operator()(uint32_t total_length, uint32_t flags, bytes_view chunk_data) override
        {
            if (this->show_protocol) {
                flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
            }

            if (verbose) {
                const bool send              = true;
                const bool from_or_to_client = false;
                ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
            }

            CHANNELS::VirtualChannelPDU virtual_channel_pdu;
            virtual_channel_pdu.send_to_server(this->stc, this->channel_id,
                total_length, flags, chunk_data);
        }
    };

    std::unique_ptr<VirtualChannelDataSender> create_to_server_synchronous_sender(CHANNELS::ChannelNameId channel_name, ServerTransportContext & stc)
    {
        const CHANNELS::ChannelDef* channel = this->mod_channel_list.get_by_name(channel_name);
        if (!channel)
        {
            return nullptr;
        }

        bool const verbose
          = bool(this->verbose & (RDPVerbose::cliprdr_dump | RDPVerbose::rdpdr_dump))
          && (channel_name == channel_names::cliprdr || channel_name == channel_names::rdpdr);

        return std::make_unique<ToServerSender>(
            stc,
            channel->chanid,
            (channel->flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL),
            verbose);
    }


    std::unique_ptr<VirtualChannelDataSender> create_to_server_asynchronous_sender(CHANNELS::ChannelNameId channel_name, ServerTransportContext & stc)
    {
        class ToServerAsynchronousSender : public VirtualChannelDataSender
        {
            ToServerSender to_server_synchronous_sender;

            AsynchronousTaskContainer& asynchronous_tasks;

            RDPVerbose verbose;

        public:
            explicit ToServerAsynchronousSender(
                ServerTransportContext & stc,
                uint16_t channel_id,
                bool show_protocol,
                AsynchronousTaskContainer& asynchronous_tasks,
                RDPVerbose rdp_verbose,
                bool verbose)
            : to_server_synchronous_sender(stc, channel_id, show_protocol, verbose)
            , asynchronous_tasks(asynchronous_tasks)
            , verbose(rdp_verbose)
            {}

            void operator()(
                uint32_t total_length, uint32_t flags, bytes_view chunk_data) override
            {
                this->asynchronous_tasks.add(
                    std::make_unique<RdpdrSendClientMessageTask>(
                        total_length, flags, chunk_data,
                        this->to_server_synchronous_sender,
                        this->verbose
                    )
                );
            }
        };

        const CHANNELS::ChannelDef* channel = this->mod_channel_list.get_by_name(channel_name);
        if (!channel)
        {
            return nullptr;
        }

        bool const verbose
          = bool(this->verbose & (RDPVerbose::cliprdr_dump | RDPVerbose::rdpdr_dump))
          && (channel_name == channel_names::cliprdr || channel_name == channel_names::rdpdr);

        return std::make_unique<ToServerAsynchronousSender>(
            stc,
            channel->chanid,
            (channel->flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL),
            this->asynchronous_tasks,
            this->verbose,
            verbose);
    }


    void create_dynamic_channel_virtual_channel(FrontAPI& front, ServerTransportContext & stc) {
        assert(!this->dynamic_channel_to_client_sender && !this->dynamic_channel_to_server_sender);

        this->dynamic_channel_to_client_sender = this->create_to_client_sender(channel_names::drdynvc, front);
        this->dynamic_channel_to_server_sender = this->create_to_server_synchronous_sender(channel_names::drdynvc, stc);

        DynamicChannelVirtualChannelParam dynamic_channel_virtual_channel_params;

        dynamic_channel_virtual_channel_params.allowed_channels =
            this->dynamic_channels.allowed_channels;
        dynamic_channel_virtual_channel_params.denied_channels =
            this->dynamic_channels.denied_channels;

        this->dynamic_channel_virtual_channel =
            std::make_unique<DynamicChannelVirtualChannel>(
                this->dynamic_channel_to_client_sender.get(),
                this->dynamic_channel_to_server_sender.get(),
                dynamic_channel_virtual_channel_params,
                this->session_log,
                this->verbose);
    }

    void create_file_system_virtual_channel(
        FrontAPI& front,
        ServerTransportContext & stc,
        GeneralCaps const & client_general_caps,
        const char (& client_name)[128]
    ) {
        assert(!this->file_system_to_client_sender && !this->file_system_to_server_sender);

        this->file_system_to_client_sender = (((client_general_caps.os_major != OSMAJORTYPE_IOS)
                                                || !this->file_system.bogus_ios_rdpdr_virtual_channel)
                                            ? this->create_to_client_sender(channel_names::rdpdr, front)
                                            : nullptr);
        this->file_system_to_server_sender = this->create_to_server_asynchronous_sender(channel_names::rdpdr, stc);

        FileSystemVirtualChannelParams fsvc_params;

        fsvc_params.file_system_read_authorized = this->channels_authorizations.rdpdr_drive_read_is_authorized();
        fsvc_params.file_system_write_authorized = this->channels_authorizations.rdpdr_drive_write_is_authorized();
        fsvc_params.parallel_port_authorized = this->channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL);
        fsvc_params.print_authorized = this->channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT);
        fsvc_params.serial_port_authorized = this->channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL);
        fsvc_params.smart_card_authorized = this->channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD);
        fsvc_params.dont_log_data_into_syslog = this->file_system.disable_log_syslog;
        fsvc_params.smartcard_passthrough = this->file_system.smartcard_passthrough;

        this->file_system_virtual_channel =  std::make_unique<FileSystemVirtualChannel>(
            this->events,
            this->file_system_to_client_sender.get(),
            this->file_system_to_server_sender.get(),
            this->drive.file_system_drive_manager,
            false,
            "",
            client_name,
            ::getpid(),
            this->drive.proxy_managed_prefix.c_str(),
            fsvc_params,
            this->session_log,
            this->verbose);
        if (this->file_system_to_server_sender) {
            if (this->session_probe.enable_session_probe || this->drive.use_application_driver) {
                this->file_system_virtual_channel->enable_session_probe_drive();
            }
        }
    }

public:
    void create_session_probe_virtual_channel(
        SessionProbeVariables vars,
        FrontAPI& front,
        ServerTransportContext stc,
        rdp_api& rdp, const Language & lang,
        const bool bogus_refresh_rect,
        const uint32_t monitor_count,
        GeneralCaps const & client_general_caps,
        const char (& client_name)[128]
    ) {
        assert(!this->session_probe_to_server_sender);

        this->session_probe_to_server_sender =
            this->create_to_server_synchronous_sender(channel_names::sespro, stc);

        if (!this->file_system_virtual_channel) {
            this->create_file_system_virtual_channel(front, stc, client_general_caps, client_name);
        }

        FileSystemVirtualChannel& file_system_virtual_channel = *this->file_system_virtual_channel;

        SessionProbeVirtualChannel::Params sp_vc_params;

        sp_vc_params.sespro_params = this->session_probe.session_probe_channel_params;
        sp_vc_params.target_informations = this->session_probe.target_informations.c_str();

        sp_vc_params.front_width = stc.negociation_result.front_width;
        sp_vc_params.front_height = stc.negociation_result.front_height;
        sp_vc_params.real_alternate_shell = this->session_probe.channel_params.real_alternate_shell.c_str();
        sp_vc_params.real_working_dir = this->session_probe.channel_params.real_working_dir.c_str();
        sp_vc_params.lang = lang;
        sp_vc_params.bogus_refresh_rect_ex = (bogus_refresh_rect && monitor_count);
        sp_vc_params.show_maximized = !this->remote_app.enable_remote_program;
        sp_vc_params.disconnect_session_instead_of_logoff_session = this->remote_app.enable_remote_program;

        this->session_probe_virtual_channel = std::make_unique<SessionProbeVirtualChannel>(
            this->events,
            this->session_log,
            vars,
            this->session_probe_to_server_sender.get(),
            front,
            rdp,
            file_system_virtual_channel,
            this->gen,
            sp_vc_params,
            this->callbacks,
            this->verbose
            );
    }

private:
    inline void create_remote_programs_virtual_channel(
                    FrontAPI& front,
                    ServerTransportContext & stc,
                    const ModRdpVariables & vars,
                    RailCaps const & client_rail_caps) {
        assert(!this->remote_programs_to_client_sender &&
            !this->remote_programs_to_server_sender);

        if (!this->remote_app.convert_remoteapp_to_desktop) {
            this->remote_programs_to_client_sender =
                this->create_to_client_sender(channel_names::rail, front);
        }
        this->remote_programs_to_server_sender =
            this->create_to_server_synchronous_sender(channel_names::rail, stc);

        RemoteProgramsVirtualChannelParams remote_programs_virtual_channel_params;

        remote_programs_virtual_channel_params.use_session_probe_to_launch_remote_program   =
            this->session_probe.used_to_launch_remote_program;

        if (!this->remote_app.perform_automatic_reconnection) {
            remote_programs_virtual_channel_params.windows_execute_shell_params = this->remote_app.windows_execute_shell_params;
            remote_programs_virtual_channel_params.windows_execute_shell_params_2 = this->remote_app.real_windows_execute_shell_params;
        }

        remote_programs_virtual_channel_params.rail_session_manager               =
            this->remote_programs_session_manager.get();

        remote_programs_virtual_channel_params.should_ignore_first_client_execute =
            this->remote_app.should_ignore_first_client_execute;

        remote_programs_virtual_channel_params.client_supports_handshakeex_pdu    =
            (client_rail_caps.RailSupportLevel & TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED);
        remote_programs_virtual_channel_params.client_supports_enhanced_remoteapp =
            this->remote_app.remote_program_enhanced;


        this->remote_programs_virtual_channel =
            std::make_unique<RemoteProgramsVirtualChannel>(
                this->remote_programs_to_client_sender.get(),
                this->remote_programs_to_server_sender.get(),
                front,
                this->remote_app.convert_remoteapp_to_desktop,
                stc.negociation_result.front_width,
                stc.negociation_result.front_height,
                vars,
                remote_programs_virtual_channel_params,
                this->session_log, this->verbose);
    }

    // TODO: make that private again when callers will be moved to channels
    static void send_to_front_channel(FrontAPI & front, CHANNELS::ChannelNameId mod_channel_name, uint8_t const * data, size_t length, size_t chunk_size, int flags) {
        const CHANNELS::ChannelDef * front_channel = front.get_channel_list().get_by_name(mod_channel_name);
        if (front_channel) {
            front.send_to_channel(*front_channel, {data, chunk_size}, length, flags);
        }
    }

public:
    void process_cliprdr_event(
        InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
        FrontAPI& front,
        ServerTransportContext & stc,
        FileValidatorService * file_validator_service)
    {
        if (!this->clipboard_virtual_channel) {
            this->create_clipboard_virtual_channel(front, stc, file_validator_service);
        }

        ClipboardVirtualChannel& channel = *this->clipboard_virtual_channel;

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            InStream clone = stream.clone();
            RDPECLIP::streamLogCliprdr(clone, flags, this->cliprdrLogStatus);// FIX
        }

        channel.process_server_message(length, flags, {stream.get_current(), chunk_size});
    }   // process_cliprdr_event


    void process_auth_event(
        ModRdpVariables vars,
        const CHANNELS::ChannelDef & auth_channel,
        InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
        FrontAPI& front,
        ServerTransportContext & stc,
        GeneralCaps const & client_general_caps,
        const char (& client_name)[128]
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

        std::string              order;
        std::vector<std::string> parameters;
        ::parse_server_message(auth_channel_message.c_str(), order, parameters);

        if (!::strcasecmp(order.c_str(), "Input") && !parameters.empty()) {
            if (::strcasecmp(parameters[0].c_str(), "Enable") != 0){
                this->callbacks.disable_input_event();
            }
            else {
                this->callbacks.enable_input_event();
            }
            this->callbacks.enable_graphics_update();
        }
        else if (!::strcasecmp(order.c_str(), "Log") && !parameters.empty()) {
            LOG(LOG_INFO, "WABLauncher: %s", parameters[0]);
        }
        else if (!::strcasecmp(order.c_str(), "RemoveDrive") && parameters.empty()) {

            if (!this->file_system_virtual_channel) {
                this->create_file_system_virtual_channel(front, stc, client_general_caps, client_name);
            }

            FileSystemVirtualChannel& rdpdr_channel = *this->file_system_virtual_channel;

            rdpdr_channel.disable_session_probe_drive();
        }
        else {
            LOG(LOG_INFO, "Auth channel data=\"%s\"", auth_channel_message);
            vars.set_acl<cfg::context::auth_channel_target>(auth_channel_message);
        }
    }

    void process_checkout_event(
        ModRdpVariables vars, const CHANNELS::ChannelDef & checkout_channel,
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

        // Version(2) + DataLength(2)
        ::check_throw(stream, 4, "mod_rdp::process_checkout_event", ERR_RDP_DATA_TRUNCATED);

        uint16_t const version = stream.in_uint16_le();
        (void)version;
        uint16_t const data_length = stream.in_uint16_le();
        (void)data_length;

//        assert(stream.in_remain() == data_length);

        this->checkout_channel_flags  = flags;
        this->checkout_channel_chanid = checkout_channel.chanid;

        bytes_view checkout_channel_message = stream.remaining_bytes();

        vars.set_acl<cfg::context::pm_request>(std::string_view(checkout_channel_message.as_charp(), checkout_channel_message.size()));
    }

    void process_session_probe_event(InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size)
    {
        SessionProbeVirtualChannel& channel = *this->session_probe_virtual_channel;
        channel.process_server_message(length, flags, {stream.get_current(), chunk_size});
    }

    void process_rail_event(const CHANNELS::ChannelDef & rail_channel,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
            FrontAPI& front,
            ServerTransportContext & stc,
            ModRdpVariables vars,
            RailCaps const & client_rail_caps)
    {
        (void)rail_channel;

        if (!this->remote_programs_virtual_channel) {
            this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
        }

        RemoteProgramsVirtualChannel& channel = *this->remote_programs_virtual_channel;
        channel.process_server_message(length, flags, {stream.get_current(), chunk_size});
    }

    void send_to_mod_rail_channel(InStream & chunk, size_t length, uint32_t flags,
                    FrontAPI& front,
                    ServerTransportContext & stc,
                    ModRdpVariables vars,
                    RailCaps const & client_rail_caps) {

        if (!this->remote_programs_virtual_channel) {
            this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
        }

        RemoteProgramsVirtualChannel& channel = *this->remote_programs_virtual_channel;


        channel.process_client_message(length, flags, chunk.remaining_bytes());

    }   // send_to_mod_rail_channel

    void send_to_mod_cliprdr_channel(InStream & chunk, size_t length, uint32_t flags,
                            FrontAPI& front,
                            ServerTransportContext & stc) {

        if (!this->clipboard_virtual_channel) {
            this->create_clipboard_virtual_channel(front, stc, this->file_validator_service);
        }
        ClipboardVirtualChannel& channel = *this->clipboard_virtual_channel;

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            InStream clone = chunk.clone();
            RDPECLIP::streamLogCliprdr(clone, flags, this->cliprdrLogStatus);
        }

        if (this->session_probe.session_probe_launcher) {
            if (!this->session_probe.session_probe_launcher->process_client_cliprdr_message(chunk, length, flags)) {
                return;
            }
        }

        channel.process_client_message(length, flags, chunk.remaining_bytes());
    }

    void process_drdynvc_event(InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
                                FrontAPI& front, ServerTransportContext & stc)
    {

        if (!this->dynamic_channel_virtual_channel) {
            this->create_dynamic_channel_virtual_channel(front, stc);
        }

        DynamicChannelVirtualChannel& channel = *this->dynamic_channel_virtual_channel;
        channel.process_server_message(length, flags, {stream.get_current(), chunk_size});
    }

    void process_unknown_channel_event(const CHANNELS::ChannelDef & channel,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
            FrontAPI& front) {

        if (channel.name == channel_names::rdpsnd && bool(this->verbose & RDPVerbose::rdpsnd)) {
            InStream clone = stream.clone();
            rdpsnd::streamLogServer(clone, flags);
        }

        this->send_to_front_channel(front, channel.name, stream.get_current(), length, chunk_size, flags);
    }

    void process_rdpdr_event(InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
                            FrontAPI& front,
                            ServerTransportContext & stc,
                            GeneralCaps const & client_general_caps,
                            const char (& client_name)[128])
    {
        if (!this->file_system.enable_rdpdr_data_analysis
        &&   this->channels_authorizations.rdpdr_type_all_is_authorized()
        &&  !this->drive.file_system_drive_manager.has_managed_drive()) {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                LOG_IF(bool(this->verbose & (RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump)),
                    LOG_INFO,
                    "mod_rdp::process_rdpdr_event: sending to Client, "
                    "send Chunked Virtual Channel Data transparently.");

                if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                    const bool send              = false;
                    const bool from_or_to_client = false;

                    ::msgdump_d(send, from_or_to_client, length, flags,
                        {stream.get_data()+8, chunk_size});

                    rdpdr::streamLog(stream, this->rdpdrLogStatus);
                }
            }

            this->send_to_front_channel(front, channel_names::rdpdr, stream.get_current(), length, chunk_size, flags);
            return;
        }

        if (!this->file_system_virtual_channel) {
            this->create_file_system_virtual_channel(front, stc, client_general_caps, client_name);
        }

        FileSystemVirtualChannel& channel = *this->file_system_virtual_channel;
        channel.process_server_message(length, flags, {stream.get_current(), chunk_size});
    }

    // TODO free function
    static void replace(std::string & text_with_tags, std::string_view marker, std::string_view replacement){
        size_t pos = 0;
        while ((pos = text_with_tags.find(marker, pos)) != std::string::npos) {
            text_with_tags.replace(pos, marker.size(), replacement.data());
            pos += replacement.size();
        }
    }

    static void replace_shell_arguments(
        std::string & text_with_tags,
        ApplicationParams const& application_params
    ){
        mod_rdp_channels::replace(text_with_tags, "${APPID}", application_params.target_application);
        mod_rdp_channels::replace(text_with_tags, "${USER}", application_params.target_application_account);
        mod_rdp_channels::replace(text_with_tags, "${PASSWORD}", application_params.target_application_password);
    }

    static std::string get_shell_arguments(
       ApplicationParams const& application_params)
    {
        std::string shell_arguments = application_params.shell_arguments;
        replace_shell_arguments(shell_arguments, application_params);
        return shell_arguments;
    }

    static void replace_probe_arguments(std::string & text_with_tags,
                                        std::string_view marker1, std::string_view replacement1,
                                        std::string_view marker2, std::string_view replacement2,
                                        std::string_view marker3, std::string_view replacement3,
                                        std::string_view marker4, std::string_view replacement4){
        mod_rdp_channels::replace(text_with_tags, marker1, replacement1);
        mod_rdp_channels::replace(text_with_tags, marker2, replacement2);
        mod_rdp_channels::replace(text_with_tags, marker3, replacement3);
        mod_rdp_channels::replace(text_with_tags, marker4, replacement4);
    }

    void init_remote_program_with_session_probe(
        const ModRDPParams::RemoteAppParams & remote_app_params,
        const ApplicationParams & application_params,
        const ModRdpSessionProbeParams & session_probe_params,
        char const* session_probe_window_title)
    {
        bool has_target = (application_params.target_application && *application_params.target_application);
        bool use_client_provided_remoteapp
          = (remote_app_params.use_client_provided_remoteapp
          && not remote_app_params.windows_execute_shell_params.exe_or_file.empty());

        if (has_target) {
            if (session_probe_params.used_to_launch_remote_program) {
                std::string shell_arguments = get_shell_arguments(application_params);

                if (shell_arguments.empty()) {
                    this->session_probe.channel_params.real_alternate_shell = application_params.alternate_shell;
                }
                else {
                    str_assign(this->session_probe.channel_params.real_alternate_shell,
                        application_params.alternate_shell, ' ', shell_arguments);
                }

                this->session_probe.channel_params.real_working_dir     = application_params.shell_working_dir;
            }
            else {
                mod_rdp_channels::replace_shell_arguments(
                    this->remote_app.real_windows_execute_shell_params.arguments, application_params);
                this->remote_app.real_windows_execute_shell_params.flags       = 0;
            }
            this->remote_app.windows_execute_shell_params.flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
            this->session_probe.session_probe_launcher = std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
        }
        else if (use_client_provided_remoteapp) {
            this->remote_app.real_windows_execute_shell_params.arguments = remote_app_params.windows_execute_shell_params.arguments;
            this->remote_app.real_windows_execute_shell_params.flags     = remote_app_params.windows_execute_shell_params.flags;
            this->remote_app.windows_execute_shell_params.flags          = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
            this->session_probe.session_probe_launcher = std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
        }

        if (has_target || use_client_provided_remoteapp){
            if (use_client_provided_remoteapp
             || !session_probe_params.used_to_launch_remote_program
            ) {
                this->session_probe.channel_params.real_alternate_shell = "[None]";
                this->remote_app.real_windows_execute_shell_params.exe_or_file = remote_app_params.windows_execute_shell_params.exe_or_file;
                this->remote_app.real_windows_execute_shell_params.working_dir = remote_app_params.windows_execute_shell_params.working_dir;
            }

            this->remote_app.windows_execute_shell_params.exe_or_file = session_probe_params.exe_or_file;

            // Executable file name of SP.
            char exe_var_str[16] {};
            if (session_probe_params.customize_executable_name) {
                ::snprintf(exe_var_str, sizeof(exe_var_str), "-%d", ::getpid());
            }

            // Target informations
            str_assign(this->session_probe.target_informations, application_params.target_application, ':');
            if (!session_probe_params.is_public_session) {
                this->session_probe.target_informations += application_params.primary_user_id;
            }

            std::string title_param = str_concat("TITLE ", session_probe_window_title, '&');

            std::string cookie_param
              = this->session_probe.target_informations.empty()
              ? std::string()
              : str_concat("/#", this->session_probe.target_informations, ' ');

            if (session_probe_params.alternate_directory_environment_variable.empty()) {
                this->remote_app.windows_execute_shell_params.working_dir = "%TMP%";
            }
            else {
                this->remote_app.windows_execute_shell_params.working_dir = "%";
                this->remote_app.windows_execute_shell_params.working_dir.append(session_probe_params.alternate_directory_environment_variable);
                this->remote_app.windows_execute_shell_params.working_dir.append("%");
            }
            LOG(LOG_INFO, "(SPADEV) WorkingDir: \"%s\"", this->remote_app.windows_execute_shell_params.working_dir.c_str());

            this->remote_app.windows_execute_shell_params.arguments   = session_probe_params.arguments;
            mod_rdp_channels::replace_probe_arguments(this->remote_app.windows_execute_shell_params.arguments,
                "${EXE_VAR}", exe_var_str,
                "${TITLE_VAR} ", title_param,
                "/${COOKIE_VAR} ", cookie_param,
                "${CBSPL_VAR} ", "");
        }
    }


    void init_remote_program_without_session_probe(
        const ModRDPParams::RemoteAppParams & remote_app_params,
        const ApplicationParams & application_params)
    {
        if (application_params.target_application && *application_params.target_application) {
            this->remote_app.windows_execute_shell_params.exe_or_file = application_params.alternate_shell;
            this->remote_app.windows_execute_shell_params.arguments   = get_shell_arguments(application_params);
            this->remote_app.windows_execute_shell_params.working_dir = application_params.shell_working_dir;
            this->remote_app.windows_execute_shell_params.flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
        }
        else if (remote_app_params.use_client_provided_remoteapp
            && not remote_app_params.windows_execute_shell_params.exe_or_file.empty()
        ) {
            this->remote_app.windows_execute_shell_params = remote_app_params.windows_execute_shell_params;
        }
    }


    void init_no_remote_program_with_session_probe(
        mod_api & mod_rdp,
        const ClientInfo & info,
        const ApplicationParams & application_params,
        const ModRdpSessionProbeParams & session_probe_params,
        char (&program)[512],
        char (&directory)[512])
    {
        // Executable file name of SP.
        char exe_var_str[16] {};
        if (session_probe_params.customize_executable_name) {
            ::snprintf(exe_var_str, sizeof(exe_var_str), "-%d", ::getpid());
        }

        // Target informations
        str_assign(this->session_probe.target_informations, application_params.target_application, ':');
        if (!session_probe_params.is_public_session) {
            this->session_probe.target_informations += application_params.primary_user_id;
        }

        if (session_probe_params.used_clipboard_based_launcher
            && application_params.target_application && *application_params.target_application
        ) {
            LOG(LOG_WARNING, "mod_rdp: "
                "Clipboard based Session Probe launcher is not compatible with application. "
                "Falled back to using AlternateShell based launcher.");
        }

        bool const used_clipboard_based_launcher =
            session_probe_params.used_clipboard_based_launcher
         && (!application_params.target_application || !*application_params.target_application)
         && (!application_params.use_client_provided_alternate_shell
          || !application_params.alternate_shell[0]
        );

        std::string cookie_param = [&]() -> std::string {
            if (used_clipboard_based_launcher){
                return std::string{};
            }

            if (this->session_probe.target_informations.empty()) {
                return std::string{};
            }

            if (session_probe_params.fix_too_long_cookie
             && this->session_probe.target_informations.length() > 20
            ){
                SslSha1 sha1;
                sha1.update(this->session_probe.target_informations);

                uint8_t sig[SslSha1::DIGEST_LENGTH];
                sha1.final(sig);

                char clipboard_based_launcher_cookie[32];
                snprintf(
                    clipboard_based_launcher_cookie,
                    sizeof(clipboard_based_launcher_cookie),
                    "/#%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X ",
                    sig[0], sig[1], sig[2], sig[3], sig[4],
                    sig[5], sig[6], sig[7], sig[8], sig[9]);

                return std::string(clipboard_based_launcher_cookie);
            }

            return str_concat("/#", this->session_probe.target_informations, ' ');
        }();

        std::string cd_tmp = session_probe_params.alternate_directory_environment_variable.empty()
            ? std::string("CD %TMP%&")
            : str_concat("CD %"_av, session_probe_params.alternate_directory_environment_variable, "%&"_av);

        LOG(LOG_INFO, "(SPADEV) Chdir: \"%s\"", cd_tmp.c_str());

        std::string arguments = session_probe_params.arguments;
        mod_rdp_channels::replace_probe_arguments(arguments,
            "${EXE_VAR}", exe_var_str,
            "${TITLE_VAR} ", "",
            "/${COOKIE_VAR} ", cookie_param,
            "${CBSPL_VAR} ", used_clipboard_based_launcher ? cd_tmp.c_str() : ""
        );

        std::string alternate_shell = session_probe_params.exe_or_file;

        if (!::strncmp(alternate_shell.c_str(), "||", 2)) {
            alternate_shell.erase(0, 2);
        }

        str_append(alternate_shell, ' ', arguments);

        if (application_params.target_application && *application_params.target_application) {
            std::string shell_arguments = get_shell_arguments(application_params);

            this->session_probe.channel_params.real_alternate_shell = shell_arguments.empty()
                ? std::string(application_params.alternate_shell)
                : str_concat(application_params.alternate_shell, ' ', shell_arguments);
            this->session_probe.channel_params.real_working_dir     = application_params.shell_working_dir;
        }
        else if (application_params.use_client_provided_alternate_shell
            && info.alternate_shell[0] && !info.remote_program
        ) {
            this->session_probe.channel_params.real_alternate_shell = info.alternate_shell;
            this->session_probe.channel_params.real_working_dir     = info.working_dir;
        }
        else if (used_clipboard_based_launcher) {
            this->session_probe.session_probe_launcher =
                std::make_unique<SessionProbeClipboardBasedLauncher>(
                    this->events,
                    mod_rdp, alternate_shell.c_str(),
                    session_probe_params.clipboard_based_launcher,
                    this->verbose);

            return ;
        }

        strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
        program[sizeof(program) - 1] = 0;
        //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

        if (session_probe_params.alternate_directory_environment_variable.empty()) {
            const char * session_probe_working_dir = "%TMP%";
            strncpy(directory, session_probe_working_dir, sizeof(directory) - 1);
            directory[sizeof(directory) - 1] = 0;
        }
        else {
            snprintf(directory, sizeof(directory), "%%%s%%", session_probe_params.alternate_directory_environment_variable.c_str());
        }
        LOG(LOG_INFO, "(SPADEV) Directory: \"%s\"", directory);

        this->session_probe.session_probe_launcher =
            std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
    }

    static void init_no_remote_program_no_session_probe(
        const ClientInfo & info,
        const ApplicationParams & application_params,
        char (&program)[512],
        char (&directory)[512])
    {
        if (application_params.target_application && *application_params.target_application) {
            std::string shell_arguments = get_shell_arguments(application_params);

            std::string alternate_shell(application_params.alternate_shell);

            if (!shell_arguments.empty()) {
                str_append(alternate_shell, ' ', shell_arguments);
            }

            strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
            program[sizeof(program) - 1] = 0;
            strncpy(directory, application_params.shell_working_dir, sizeof(directory) - 1);
            directory[sizeof(directory) - 1] = 0;
        }
        else if (application_params.use_client_provided_alternate_shell
              && info.alternate_shell[0]
              && !info.remote_program
        ) {
            strncpy(program, info.alternate_shell, sizeof(program) - 1);
            program[sizeof(program) - 1] = 0;
            strncpy(directory, info.working_dir, sizeof(directory) - 1);
            directory[sizeof(directory) - 1] = 0;
        }
    }

    void send_to_mod_rdpdr_channel(
        const CHANNELS::ChannelDef * rdpdr_channel,
        InStream & chunk, size_t length, uint32_t flags,
        FrontAPI& front,
        ServerTransportContext & stc,
        GeneralCaps const & client_general_caps,
        const char (& client_name)[128])
    {
        if (!this->file_system.enable_rdpdr_data_analysis
        &&   this->channels_authorizations.rdpdr_type_all_is_authorized()
        &&  !this->drive.file_system_drive_manager.has_managed_drive()) {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                LOG_IF(bool(this->verbose & (RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump)),
                    LOG_INFO,
                    "mod_rdp::send_to_mod_rdpdr_channel: recv from Client, "
                    "send Chunked Virtual Channel Data transparently.");

                if (bool(this->verbose & RDPVerbose::rdpdr_dump)) {
                    const bool send              = false;
                    const bool from_or_to_client = false;
                    uint32_t total_length = length;
                    if (total_length > CHANNELS::CHANNEL_CHUNK_LENGTH) {
                        total_length = chunk.get_capacity() - chunk.get_offset();
                    }
                    ::msgdump_d(send, from_or_to_client, length, flags,
                    {chunk.get_data(), total_length});

                    rdpdr::streamLog(chunk, this->rdpdrLogStatus);
                }
            }

            this->send_to_channel(*rdpdr_channel, {chunk.get_data(), chunk.get_capacity()}, length, flags, stc);
            return;
        }

        if (!this->file_system_virtual_channel) {
            this->create_file_system_virtual_channel(front, stc, client_general_caps, client_name);
        }

        FileSystemVirtualChannel& channel = *this->file_system_virtual_channel;

        channel.process_client_message(length, flags, chunk.remaining_bytes());
    }

    void send_to_mod_drdynvc_channel(InStream & chunk, size_t length, uint32_t flags,
                                        FrontAPI& front,
                                        ServerTransportContext & stc)
    {

        if (!this->dynamic_channel_virtual_channel) {
            this->create_dynamic_channel_virtual_channel(front, stc);
        }

        DynamicChannelVirtualChannel& channel = *this->dynamic_channel_virtual_channel;

        channel.process_client_message(length, flags, chunk.remaining_bytes());
    }

    void send_to_channel(
        const CHANNELS::ChannelDef & channel,
        bytes_view chunk, size_t length, uint32_t flags,
        ServerTransportContext & stc)
    {
        if (channel.name == channel_names::rdpsnd && bool(this->verbose & RDPVerbose::rdpsnd)) {
            InStream clone(chunk);
            rdpsnd::streamLogClient(clone, flags);
        }

        if (bool(this->verbose & RDPVerbose::channels)) {
            LOG( LOG_INFO, "mod_rdp::send_to_channel length=%zu chunk_size=%zu", length, chunk.size());
            channel.log(-1u);
        }

        if (channel.flags & GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        }

        if (chunk.size() <= CHANNELS::CHANNEL_CHUNK_LENGTH) {
            CHANNELS::VirtualChannelPDU virtual_channel_pdu;
            virtual_channel_pdu.send_to_server(stc, channel.chanid, length, flags, chunk);
        }
        else {
            uint8_t const * virtual_channel_data = chunk.data();
            size_t          remaining_data_length = length;

            auto get_channel_control_flags = [] (uint32_t flags, size_t data_length,
                                                    size_t remaining_data_length,
                                                    size_t virtual_channel_data_length) -> uint32_t {
                if (remaining_data_length == data_length) {
                    return (flags & (~CHANNELS::CHANNEL_FLAG_LAST));
                }
                if (remaining_data_length == virtual_channel_data_length) {
                    return (flags & (~CHANNELS::CHANNEL_FLAG_FIRST));
                }

                return (flags & (~(CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)));
            };

            do {
                const size_t virtual_channel_data_length =
                    std::min<size_t>(remaining_data_length, CHANNELS::CHANNEL_CHUNK_LENGTH);

                CHANNELS::VirtualChannelPDU virtual_channel_pdu;
                virtual_channel_pdu.send_to_server(stc, channel.chanid, length,
                    get_channel_control_flags(
                        flags, length, remaining_data_length, virtual_channel_data_length),
                    {virtual_channel_data, virtual_channel_data_length});

                remaining_data_length -= virtual_channel_data_length;
                virtual_channel_data  += virtual_channel_data_length;
            }
            while (remaining_data_length);
        }

        LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO, "mod_rdp::send_to_channel done");
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name,
        InStream & chunk, size_t length, uint32_t flags,
        FrontAPI& front,
        ServerTransportContext & stc,
        GeneralCaps const & client_general_caps,
        ModRdpVariables vars,
        RailCaps const & client_rail_caps,
        const char (& client_name)[128]
    ) {
        const CHANNELS::ChannelDef * mod_channel = this->mod_channel_list.get_by_name(front_channel_name);
        if (!mod_channel) {
            return;
        }

        if (bool(this->verbose & RDPVerbose::channels)) {
            mod_channel->log(unsigned(mod_channel - &this->mod_channel_list[0]));
        }

        switch (front_channel_name) {
            case channel_names::cliprdr:
                IF_ENABLE_METRICS(set_client_cliprdr_metrics(chunk.clone(), length, flags));
                this->send_to_mod_cliprdr_channel(chunk, length, flags, front, stc);
                break;
            case channel_names::rail:
                IF_ENABLE_METRICS(client_rail_channel_data(length));
                this->send_to_mod_rail_channel(chunk, length, flags, front, stc, vars, client_rail_caps);
                break;
            case channel_names::rdpdr:
                IF_ENABLE_METRICS(set_client_rdpdr_metrics(chunk.clone(), length, flags));
                this->send_to_mod_rdpdr_channel(mod_channel, chunk, length, flags, front, stc, client_general_caps, client_name);
                break;
            case channel_names::drdynvc:
                IF_ENABLE_METRICS(client_other_channel_data(length));
                this->send_to_mod_drdynvc_channel(chunk, length, flags, front, stc);
                break;
            default:
                IF_ENABLE_METRICS(client_other_channel_data(length));
                this->send_to_channel(*mod_channel, {chunk.get_data(), chunk.get_capacity()}, length, flags, stc);
        }
    }

    // This function can be called several times. If a remaining session_probe is running on the
    // target serveur, the session probe channels is already there before the session probe launcher is created
    void do_enable_session_probe(
        FrontAPI& front,
        ServerTransportContext & stc,
        rdp_api& rdp,
        GeneralCaps const & client_general_caps,
        ModRdpVariables vars,
        RailCaps const & client_rail_caps,
        const char (& client_name)[128],
        const uint32_t monitor_count,
        const bool bogus_refresh_rect,
        const Language & lang,
        FileValidatorService * file_validator_service)
    {
        assert(this->session_probe.enable_session_probe);
        if (this->session_probe.session_probe_launcher){
            if (!this->clipboard_virtual_channel) {
                this->create_clipboard_virtual_channel(front, stc, file_validator_service);
            }
            ClipboardVirtualChannel& cvc = *this->clipboard_virtual_channel;
            cvc.set_session_probe_launcher(this->session_probe.session_probe_launcher.get());

            if (!this->file_system_virtual_channel) {
                this->create_file_system_virtual_channel(front, stc, client_general_caps, client_name);
            }

            FileSystemVirtualChannel& fsvc = *this->file_system_virtual_channel;

            fsvc.set_session_probe_launcher(this->session_probe.session_probe_launcher.get());

            this->drive.file_system_drive_manager.set_session_probe_launcher(this->session_probe.session_probe_launcher.get());

            if (!this->session_probe_virtual_channel) {
                this->create_session_probe_virtual_channel(
                    vars, front, stc, rdp, lang, bogus_refresh_rect,
                    monitor_count, client_general_caps, client_name);
            }
            this->session_probe_virtual_channel->set_session_probe_launcher(this->session_probe.session_probe_launcher.get());
            if (!this->session_probe.start_launch_timeout_timer_only_after_logon) {
                this->session_probe_virtual_channel->start_launch_timeout_timer();
            }
            this->session_probe.session_probe_launcher->set_clipboard_virtual_channel(&cvc);
            this->session_probe.session_probe_launcher->set_session_probe_virtual_channel(this->session_probe_virtual_channel.get());

            if (this->remote_app.enable_remote_program) {

                if (!this->remote_programs_virtual_channel) {
                    this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
                }

                RemoteProgramsVirtualChannel& rpvc = *this->remote_programs_virtual_channel;
                rpvc.set_session_probe_virtual_channel(this->session_probe_virtual_channel.get());
                rpvc.set_session_probe_launcher(this->session_probe.session_probe_launcher.get());
                this->session_probe.session_probe_launcher->set_remote_programs_virtual_channel(&rpvc);
            }
        }
        else // not this->session_probe.session_probe_launcher
        {
            if (!this->session_probe_virtual_channel) {
                this->create_session_probe_virtual_channel(
                    vars, front, stc, rdp, lang, bogus_refresh_rect,
                    monitor_count, client_general_caps, client_name);
            }

            if (!this->session_probe.start_launch_timeout_timer_only_after_logon) {
                this->session_probe_virtual_channel->start_launch_timeout_timer();
            }

            if (this->remote_app.enable_remote_program) {
                if (!this->remote_programs_virtual_channel) {
                    this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
                }

                RemoteProgramsVirtualChannel& rpvc = *this->remote_programs_virtual_channel;
                rpvc.set_session_probe_virtual_channel(this->session_probe_virtual_channel.get());
            }
        }
    }

    void auth_rail_exec(
        uint16_t flags, const char* original_exe_or_file,
        const char* exe_or_file, const char* working_dir,
        const char* arguments, const char* account, const char* password,
        FrontAPI& front,
        ServerTransportContext & stc,
        ModRdpVariables vars,
        RailCaps const & client_rail_caps
    ) {
        if (this->remote_app.enable_remote_program) {
            if (!this->remote_programs_virtual_channel) {
                this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
            }

            RemoteProgramsVirtualChannel& rpvc = *this->remote_programs_virtual_channel;

            rpvc.auth_rail_exec(flags, original_exe_or_file, exe_or_file,  working_dir, arguments, account, password);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::auth_rail_exec(): Current session has no Remote Program Virtual Channel");
        }
    }

    void auth_rail_exec_cancel(
        uint16_t flags, const char* original_exe_or_file, uint16_t exec_result,
        FrontAPI& front,
        ServerTransportContext & stc,
        ModRdpVariables vars,
        RailCaps const & client_rail_caps
    ) {
        if (this->remote_app.enable_remote_program) {
            if (!this->remote_programs_virtual_channel) {
                this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
            }

            RemoteProgramsVirtualChannel& rpvc = *this->remote_programs_virtual_channel;

            rpvc.auth_rail_exec_cancel(flags, original_exe_or_file, exec_result);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::auth_rail_exec(): Current session has no Remote Program Virtual Channel");
        }
    }

    void sespro_rail_exec_result(
        uint16_t flags, const char* exe_or_file, uint16_t exec_result, uint32_t raw_result,
        FrontAPI& front,
        ServerTransportContext & stc,
        ModRdpVariables vars,
        RailCaps const & client_rail_caps
    ) {
        if (this->remote_app.enable_remote_program) {
            if (!this->remote_programs_virtual_channel) {
                this->create_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
            }
            RemoteProgramsVirtualChannel& rpvc = *this->remote_programs_virtual_channel;
            rpvc.sespro_rail_exec_result(flags, exe_or_file, exec_result, raw_result);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::sespro_rail_exec_result(): Current session has no Remote Program Virtual Channel");
        }
    }
#else
    template<class... Ts>
    mod_rdp_channels(const ChannelsAuthorizations & channels_authorizations, Ts&&...) noexcept
    : channels_authorizations(channels_authorizations)
    , enable_auth_channel(false)
    {}
#endif
};

class mod_rdp : public mod_api, public rdp_api
{
#ifndef __EMSCRIPTEN__
    struct SessionProbeChannelCallbacks : public SessionProbeVirtualChannel::Callbacks
    {
        mod_rdp & mod;
        gdi::OsdApi& osd;
        SessionProbeChannelCallbacks(mod_rdp & mod, gdi::OsdApi& osd) : mod(mod), osd(osd) {}
        void freeze_screen() override { mod.freeze_screen(); }
        void disable_input_event() override { mod.disable_input_event(); }
        void enable_input_event() override { mod.enable_input_event(); }
        void enable_graphics_update() override { mod.enable_graphics_update(); }
        void disable_graphics_update() override { mod.disable_graphics_update(); }
        void display_osd_message(std::string_view message) override  { osd.display_osd_message(message); }
    } spvc_callbacks;
#endif

    mod_rdp_channels channels;

    CryptContext decrypt {};

    RedirectionInfo & redir_info;

    const bool disconnect_on_logon_user_change;
    const RdpLogonInfo logon_info;
    const Recv_CS_BitmapCodecCaps front_bitmap_codec_caps;

    std::array<uint8_t, 28>& server_auto_reconnect_packet_ref;

    uint32_t monitor_count = 0;

    Transport & trans;
    CryptContext encrypt {};
    RdpNegociationResult negociation_result;

    uint32_t front_multifragment_maxsize = 0;

    int share_id = 0;
    bool enable_fastpath_client_input_event = false;
    bool remote_apps_not_enabled = false;

    ModRDPParams::ServerInfo * server_info_ref;

    FrontAPI& front;

    rdp_orders orders;
    RfxDecoder rfxDecoder;

    char client_name[128] = {};

    const int key_flags;
    int  last_key_flags_sent;
    bool first_scancode = true;

    enum : uint8_t {
        EARLY,
        WAITING_SYNCHRONIZE,
        WAITING_CTL_COOPERATE,
        WAITING_GRANT_CONTROL_COOPERATE,
        WAITING_FONT_MAP,
        UP_AND_RUNNING,
        DISCONNECTED,
    } connection_finalization_state = EARLY;

    Pointer cursors[32];

    const RDPVerbose verbose;
    const BmpCache::Verbose cache_verbose;

    LicenseApi& license_store;

    std::string& close_box_extra_message_ref;

    const bool enable_fastpath;                    // choice of programmer
    const bool enable_fastpath_server_update;      // choice of programmer
    const bool enable_glyph_cache;
    const bool use_native_pointer;
    const bool enable_new_pointer;
    const bool enable_persistent_disk_bitmap_cache;
    const bool enable_cache_waiting_list;
    const bool persist_bitmap_cache_on_disk;
    const bool enable_server_cert_external_validation;
    const bool enable_remotefx;
    bool haveRemoteFx{false};
    bool haveSurfaceFrameAck{false};
    std::vector<uint8_t> remoteFx_codec_id;

    const PrimaryDrawingOrdersSupport primary_drawing_orders_support;

    uint32_t frameInProgress = false;
    uint32_t currentFrameId = 0;

    bool delayed_start_capture = false;

    const bool experimental_fix_input_event_sync;

    const bool support_connection_redirection_during_recording;

    size_t recv_bmp_update = 0;

    rdp_mppc_unified_dec mppc_dec;

    std::string * error_message;

    gdi::GraphicApi & gd;
    EventsGuard events_guard;
    SessionLogApi& session_log;

#ifndef __EMSCRIPTEN__
    EventRef remoteapp_one_shot_bypass_window_legalnotice;
#endif

    bool deactivation_reactivation_in_progress = false;

    const bool bogus_refresh_rect;

    Language lang;

    bool already_upped_and_running = false;

    bool input_event_disabled     = false;
    bool graphics_update_disabled = false;

    bool mcs_disconnect_provider_ultimatum_pdu_received = false;
    bool errinfo_graphics_subsystem_failed_encountered  = false;
    bool errinfo_encountered                            = false;

    static constexpr std::array<uint32_t, BmpCache::MAXIMUM_NUMBER_OF_CACHES>
    BmpCacheRev2_Cache_NumEntries()
    { return std::array<uint32_t, BmpCache::MAXIMUM_NUMBER_OF_CACHES>{{ 120, 120, 2553, 0, 0 }}; }

    MonotonicTimePoint::duration session_time_start;

    const bool clean_up_32_bpp_cursor;
    const bool large_pointer_support;

    std::unique_ptr<uint8_t[]> multifragment_update_buffer;
    OutStream multifragment_update_data;

    LargePointerCaps        client_large_pointer_caps;
    MultiFragmentUpdateCaps client_multi_fragment_update_caps;

    GeneralCaps const        client_general_caps;
    // BitmapCaps const         client_bitmap_caps;
    // OrderCaps const          client_order_caps;
    // BmpCacheCaps const       client_bmp_cache_caps;
    // BmpCache2Caps const      client_bmp_cache_2_caps;
    // OffScreenCacheCaps const client_off_screen_cache_caps;
    GlyphCacheCaps const     client_glyph_cache_caps;
    RailCaps const           client_rail_caps;
    WindowListCaps const     client_window_list_caps;

    bool is_server_auto_reconnec_packet_received = false;

    bool server_redirection_packet_received = false;

    ModRdpVariables vars;

    bool const accept_monitor_layout_change_if_capture_is_not_started;

#ifndef __EMSCRIPTEN__
    bool const session_probe_start_launch_timeout_timer_only_after_logon;

    RDPMetrics * metrics;
    FileValidatorService * file_validator_service;
#endif

    struct PrivateRdpNegociation
    {
        RdpNegociation rdp_negociation;
        const std::chrono::seconds open_session_timeout;
#ifndef __EMSCRIPTEN__
        CertificateResult result = CertificateResult::wait;
#endif

        template<class... Ts>
        explicit PrivateRdpNegociation(
            std::chrono::seconds open_session_timeout,
            char const* program, char const* directory,
            Ts&&... xs)
        : rdp_negociation(static_cast<Ts&&>(xs)...)
        , open_session_timeout(open_session_timeout)
        {
            this->rdp_negociation.set_program(program, directory);
        }
    };

    std::unique_ptr<PrivateRdpNegociation> private_rdp_negociation;

    uint8_t status_of_keyboard_toggle_keys = 0;

public:
    using Verbose = RDPVerbose;

    explicit mod_rdp(
        Transport & trans
      , gdi::GraphicApi & gd
      , [[maybe_unused]] gdi::OsdApi & osd
      , EventContainer & events
      , SessionLogApi& session_log
      , FrontAPI & front
      , const ClientInfo & info
      , RedirectionInfo & redir_info
      , Random & gen
      , const ChannelsAuthorizations & channels_authorizations
      , const ModRDPParams & mod_rdp_params
      , const TLSClientParams & tls_client_params
      , LicenseApi & license_store
      , ModRdpVariables vars
      , [[maybe_unused]] RDPMetrics * metrics
      , [[maybe_unused]] FileValidatorService * file_validator_service
      , ModRdpFactory& mod_rdp_factory
    )
#ifndef __EMSCRIPTEN__
        : spvc_callbacks(*this, osd)
        , channels(
            channels_authorizations, mod_rdp_params, mod_rdp_params.verbose,
            gen, metrics, osd, events, session_log,
            file_validator_service,
            mod_rdp_factory,
            spvc_callbacks
        )
#else
        : channels(channels_authorizations)
#endif
        , redir_info(redir_info)
        , disconnect_on_logon_user_change(mod_rdp_params.disconnect_on_logon_user_change)
        , logon_info(info.hostname, mod_rdp_params.hide_client_name, mod_rdp_params.target_user, mod_rdp_params.split_domain)
        , front_bitmap_codec_caps(info.bitmap_codec_caps)
        , server_auto_reconnect_packet_ref(mod_rdp_params.server_auto_reconnect_packet_ref)
        , monitor_count(mod_rdp_params.allow_using_multiple_monitors ? info.cs_monitor.monitorCount : 0)
        , trans(trans)
        , server_info_ref(mod_rdp_params.server_info_ref)
        , front(front)
        , orders( mod_rdp_params.target_host, mod_rdp_params.enable_persistent_disk_bitmap_cache
                , mod_rdp_params.persist_bitmap_cache_on_disk
                , mod_rdp_params.remote_app_params.convert_remoteapp_to_desktop, mod_rdp_params.verbose
                , [&session_log](const Error & error){
                    if (error.errnum == ENOSPC) {
                        // error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
                        session_log.report("FILESYSTEM_FULL", "100|unknown");
                    }
                })
        , key_flags(mod_rdp_params.key_flags)
        , last_key_flags_sent(key_flags)
        , verbose(mod_rdp_params.verbose)
        , cache_verbose(mod_rdp_params.cache_verbose)
        , license_store(license_store)
        , close_box_extra_message_ref(mod_rdp_params.close_box_extra_message_ref)
        , enable_fastpath(mod_rdp_params.enable_fastpath)
        , enable_fastpath_server_update(mod_rdp_params.enable_fastpath)
        , enable_glyph_cache(mod_rdp_params.enable_glyph_cache)
        , use_native_pointer(mod_rdp_params.use_native_pointer)
        , enable_new_pointer(mod_rdp_params.enable_new_pointer)
        , enable_persistent_disk_bitmap_cache(mod_rdp_params.enable_persistent_disk_bitmap_cache)
        , enable_cache_waiting_list(mod_rdp_params.enable_cache_waiting_list)
        , persist_bitmap_cache_on_disk(mod_rdp_params.persist_bitmap_cache_on_disk)
        , enable_server_cert_external_validation(mod_rdp_params.enable_server_cert_external_validation)
        , enable_remotefx(mod_rdp_params.enable_remotefx)
        , primary_drawing_orders_support(
            [](auto& order_support, PrimaryDrawingOrdersSupport const& disabled_orders){
                PrimaryDrawingOrdersSupport client_support;
                for (auto idx : order_indexes_supported()) {
                    if (order_support[idx]) {
                        client_support |= idx;
                    }
                }
                return client_support - disabled_orders;
            }(info.order_caps.orderSupport, mod_rdp_params.disabled_orders))
        // info.order_caps.orderSupport
        , experimental_fix_input_event_sync(mod_rdp_params.experimental_fix_input_event_sync)
        , support_connection_redirection_during_recording(mod_rdp_params.support_connection_redirection_during_recording)
        , error_message(mod_rdp_params.error_message)
        , gd(gd)
        , events_guard(events)
        , session_log(session_log)
        , bogus_refresh_rect(mod_rdp_params.bogus_refresh_rect)
        , lang(mod_rdp_params.lang)
        , session_time_start(events.get_monotonic_time().time_since_epoch())
        , clean_up_32_bpp_cursor(mod_rdp_params.clean_up_32_bpp_cursor)
        , large_pointer_support(mod_rdp_params.large_pointer_support)
        , multifragment_update_buffer(std::make_unique<uint8_t[]>(65536))
        , multifragment_update_data({multifragment_update_buffer.get(), 65536})
        , client_large_pointer_caps(info.large_pointer_caps)
        , client_multi_fragment_update_caps(info.multi_fragment_update_caps)
        , client_general_caps(info.general_caps)
        // , client_bitmap_caps(info.bitmap_caps)
        // , client_order_caps(info.order_caps)
        // , client_bmp_cache_caps(info.bmp_cache_caps)
        // , client_bmp_cache_2_caps(info.bmp_cache_2_caps)
        // , client_off_screen_cache_caps(info.off_screen_cache_caps)
        , client_glyph_cache_caps(info.glyph_cache_caps)
        , client_rail_caps(info.rail_caps)
        , client_window_list_caps(info.window_list_caps)
        , vars(vars)
        , accept_monitor_layout_change_if_capture_is_not_started(mod_rdp_params.accept_monitor_layout_change_if_capture_is_not_started)
        #ifndef __EMSCRIPTEN__
        , session_probe_start_launch_timeout_timer_only_after_logon(mod_rdp_params.session_probe_params.start_launch_timeout_timer_only_after_logon)
        , metrics(metrics)
        , file_validator_service(file_validator_service)
        #endif
    {
        #ifdef __EMSCRIPTEN__
        (void)events;
        (void)mod_rdp_factory;
        (void)session_log;
        #endif

        using namespace std::literals::chrono_literals;

        if (this->enable_remotefx){
            LOG(LOG_INFO, "RemoteFX enabled on proxy");
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "Creation of new mod 'RDP'");
            mod_rdp_params.log();
        }

        this->decrypt.encryptionMethod = 2; /* 128 bits */
        this->encrypt.encryptionMethod = 2; /* 128 bits */

        snprintf(this->client_name, sizeof(this->client_name), "%s", info.hostname);

        char program[512] = {};
        char directory[512] = {};

#ifndef __EMSCRIPTEN__
        this->channels.init_remote_program_and_session_probe(
            gd, *this, mod_rdp_params, info, program, directory);
#endif

        this->negociation_result.front_width = info.screen_info.width;
        this->negociation_result.front_height = info.screen_info.height;

        Dimension const& dim = this->get_dim();
        Rect const r(0, 0, dim.w, dim.h);
        RDPOpaqueRect cmd(r, color_encode(BLACK, BitsPerPixel{24}));
        gd.begin_update();
        gd.draw(cmd, r, gdi::ColorCtx::depth24());
        gd.end_update();

        const std::chrono::seconds open_session_timeout
            = (mod_rdp_params.open_session_timeout != 0s)
                ?  mod_rdp_params.open_session_timeout
                :  15s;

        this->private_rdp_negociation = std::make_unique<PrivateRdpNegociation>(
            open_session_timeout, program, directory,
            this->channels.channels_authorizations, this->channels.mod_channel_list,
            this->channels.auth_channel, this->channels.checkout_channel,
            this->decrypt, this->encrypt, this->logon_info,
            this->channels.enable_auth_channel,
            this->trans, this->front, info, this->redir_info,
            gen, this->events_guard.get_time_base(), mod_rdp_params,
            this->session_log, this->license_store,
    #ifndef __EMSCRIPTEN__
            this->channels.drive.file_system_drive_manager.has_managed_drive()
         || this->channels.session_probe.enable_session_probe,
            this->channels.remote_app.convert_remoteapp_to_desktop,
    #else
            false,
            false,
    #endif
            tls_client_params
        );

        RdpNegociation& rdp_negociation = this->private_rdp_negociation->rdp_negociation;

    #ifndef __EMSCRIPTEN__
        if (this->enable_server_cert_external_validation) {
            LOG(LOG_INFO, "**** Enable server cert external validation");
            rdp_negociation.set_cert_callback([this](X509& certificate){
                auto& result = this->private_rdp_negociation->result;

                if (result != CertificateResult::wait) {
                    return result;
                }

                std::string blob_str;
                if (!cert_to_escaped_string(certificate, blob_str)) {
                    LOG(LOG_ERR, "cert_to_string failed");
                    return result;
                }

                // LOG(LOG_INFO, "cert pem: %s", blob_str);

                this->vars.set_acl<cfg::mod_rdp::server_cert>(blob_str);
                this->vars.ask<cfg::mod_rdp::server_cert_response>();

                return result;
            });
        }
    #endif

        LOG(LOG_INFO, "**** Start Negociation");
        rdp_negociation.start_negociation();

        this->events_guard.create_event_fd_timeout(
            "RDP Negociation",
            this->trans.get_fd(),
            this->private_rdp_negociation->open_session_timeout,
            [this](Event& event)
            {
                try {
                    bool const negotiation_finished = this->private_rdp_negociation->rdp_negociation.recv_data(this->buf);

                    // RdpNego::recv_next_data may reconnect and change fd if tls
                    int const fd = this->trans.get_fd();
                    if (fd >= 0) {
                        event.alarm.set_fd(fd, event.alarm.grace_delay);
                    }

                    if (negotiation_finished) {
                        this->negociation_result = this->private_rdp_negociation->rdp_negociation.get_result();

                        if (this->buf.remaining()){
                            this->draw_event();
                        }

                        event.garbage = true;

                        this->events_guard.create_event_fd_timeout(
                            "First Incoming RDP PDU Event",
                            event.alarm.fd, event.alarm.grace_delay,
                            event.alarm.trigger_time + std::chrono::seconds{3600},
                            [this](Event&event)
                            {
                                if (this->buf.remaining()){
                                    this->draw_event();
                                }
                                this->private_rdp_negociation.reset();
                                #ifndef __EMSCRIPTEN__
                                if (this->channels.remote_programs_session_manager) {
                                    this->channels.remote_programs_session_manager->set_drawable(&this->gd);
                                }
                                #endif
                                this->buf.load_data(this->trans);
                                this->draw_event();

                                event.garbage = true;

                                this->events_guard.create_event_fd_timeout(
                                    "Incoming RDP PDU Event",
                                    event.alarm.fd, event.alarm.grace_delay,
                                    event.alarm.trigger_time,
                                    [this](Event& /*event*/)
                                    {
                                        #ifndef __EMSCRIPTEN__
                                        if (this->channels.remote_programs_session_manager) {
                                            this->channels.remote_programs_session_manager->set_drawable(&this->gd);
                                        }
                                        #endif
                                        this->buf.load_data(this->trans);
                                        this->draw_event();
                                    },
                                    [](Event& /*event*/){}
                                );
                            },
                            [](Event& /*event*/){}
                        );
                    }
                }
                catch (Error & error) {
                    event.garbage = true;
                    this->throw_error(error);
                }
            },
            [this](Event&event)
            {
                try {
                    if (this->error_message) {
                        *this->error_message = "Logon timer expired!";
                    }

                    this->session_log.report("CONNECTION_FAILED", "Logon timer expired.");
            #ifndef __EMSCRIPTEN__
                    if (this->channels.session_probe.enable_session_probe) {
                        this->enable_input_event();
                        this->enable_graphics_update();
                    }
            #endif
                    LOG(LOG_ERR,
                        "Logon timer expired on %s. The session will be disconnected.",
                    this->logon_info.hostname());
                    throw Error(ERR_RDP_OPEN_SESSION_TIMEOUT);
                }
                catch (Error & error) {
                    event.garbage = true;
                    this->throw_error(error);
                }
            });
    }   // mod_rdp


    ~mod_rdp() override {
#ifndef __EMSCRIPTEN__
        if (this->channels.session_probe.enable_session_probe) {
            this->enable_input_event();
            this->enable_graphics_update();
        }
#endif

        if (DISCONNECTED == this->connection_finalization_state) {
            this->session_log.report("CLOSE_SESSION_SUCCESSFUL", "OK.");
        }

        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp cache count  = %zu",
                this->orders.recv_bmp_cache_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv order count      = %zu",
                this->orders.recv_order_count);
            LOG(LOG_INFO, "~mod_rdp(): Recv bmp update count = %zu",
                this->recv_bmp_update);
        }

#ifndef __EMSCRIPTEN__
        this->channels.remote_programs_session_manager.reset();
#endif

        if (!this->server_redirection_packet_received) {
            this->redir_info = RedirectionInfo();
        }
    }


    [[noreturn]] void throw_error(Error & error)
    {
        LOG(LOG_INFO, "throw error mod_rdp::fd event exception %u: %s", error.id, error.errmsg());
        switch (error.id) {
        case ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED:
        case ERR_TRANSPORT_TLS_CERTIFICATE_MISSED:
        case ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED:
        case ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE:
        case ERR_NLA_AUTHENTICATION_FAILED:
            throw error;
        default: break;
        }

        const char * statestr = "UNKNOWN_STATE";
        const char * statedescr = "Unknown state.";
        RdpNegociation::State rdp_nego_state =
            this->private_rdp_negociation->rdp_negociation.get_state();

        switch (rdp_nego_state) {
            case RdpNegociation::State::NEGO_INITIATE:
                statestr = "RDP_NEGO_INITIATE";
                statedescr = TR(trkeys::err_mod_rdp_nego, this->lang);
            break;
            case RdpNegociation::State::NEGO:
                statestr = "RDP_NEGO";
                statedescr = TR(trkeys::err_mod_rdp_nego, this->lang);
            break;
            case RdpNegociation::State::BASIC_SETTINGS_EXCHANGE:
                statestr = "RDP_BASIC_SETTINGS_EXCHANGE";
                statedescr = TR(trkeys::err_mod_rdp_basic_settings_exchange, this->lang);
            break;
            case RdpNegociation::State::CHANNEL_CONNECTION_ATTACH_USER:
                statestr = "RDP_CHANNEL_CONNECTION_ATTACH_USER";
                statedescr = TR(trkeys::err_mod_rdp_channel_connection_attach_user, this->lang);
            break;
            case RdpNegociation::State::CHANNEL_JOIN_CONFIRM:
                statestr = "RDP_CHANNEL_JOIN_CONFIRM";
                statedescr = TR(trkeys::mod_rdp_channel_join_confirme, this->lang);
            break;
            case RdpNegociation::State::GET_LICENSE:
                statestr = "RDP_GET_LICENSE";
                statedescr = TR(trkeys::mod_rdp_get_license, this->lang);
            break;
            case RdpNegociation::State::TERMINATED:
                statestr = "RDP_TERMINATED";
                statedescr = TR(trkeys::err_mod_rdp_nego, this->lang);
            break;
        }
        str_append(this->close_box_extra_message_ref, " ", statedescr);
        if (rdp_nego_state != RdpNegociation::State::BASIC_SETTINGS_EXCHANGE)
        {
            str_append(this->close_box_extra_message_ref, " (", statestr, ')');
        }
        LOG(LOG_ERR, "Creation of new mod 'RDP' failed at %s state. %s",
            statestr, statedescr);
        throw Error(ERR_RDP_NEGOTIATION);
    }


    void acl_update(AclFieldMask const& acl_fields) override
    {
        if (this->enable_server_cert_external_validation
         && acl_fields.has<cfg::mod_rdp::server_cert_response>()
        ) {
            auto const& message = this->vars.get<cfg::mod_rdp::server_cert_response>();

            if (message == "Ok" || message == "ok") {
                LOG(LOG_INFO, "Certificate was valid according to authentifier");
            }
            else {
                LOG(LOG_INFO, "Certificate was invalid according to authentifier: %s", message);
                throw Error(ERR_TRANSPORT_TLS_CERTIFICATE_INVALID);
            }

            bool const is_finish =
                this->private_rdp_negociation->rdp_negociation.recv_data(this->buf);
            if (is_finish) {
                this->negociation_result =
                    this->private_rdp_negociation->rdp_negociation.get_result();
                if (this->buf.remaining()) {
                    this->draw_event();
                }
            }
        }

        // AuthCHANNEL CHECK
        if (acl_fields.has<cfg::context::auth_channel_answer>()
         && this->channels.auth_channel != CHANNELS::ChannelNameId()
        ) {
            // Get sesman answer to AUTHCHANNEL_TARGET
            auto & auth_channel_answer = this->vars.get<cfg::context::auth_channel_answer>();
            if (!auth_channel_answer.empty()) {
                this->send_auth_channel_data(auth_channel_answer.c_str());
            }
        }

        // CheckoutCHANNEL CHECK
        if (acl_fields.has<cfg::context::pm_response>()) {
            // Get sesman answer to AUTHCHANNEL_TARGET
            auto & pm_response = this->vars.get<cfg::context::pm_response>();
            if (!pm_response.empty()
            ) {
                // If set, transmit to auth_channel channel
                this->send_checkout_channel_data(pm_response.c_str());
            }
        }

        if (acl_fields.has<cfg::context::rd_shadow_type>()) {
            auto & rd_shadow_type = this->vars.get<cfg::context::rd_shadow_type>();
            if (!rd_shadow_type.empty()) {
                auto & rd_shadow_userdata = this->vars.get<cfg::context::rd_shadow_userdata>();
                LOG(LOG_INFO, "got rd_shadow_type calling create_shadow_session()");
                this->create_shadow_session(rd_shadow_userdata.c_str(), rd_shadow_type.c_str());
            }
        }

        if (acl_fields.has<cfg::context::auth_command>()) {
            if (this->vars.get<cfg::context::auth_command>() == "rail_exec") {
                const uint16_t flags                = this->vars.get<cfg::context::auth_command_rail_exec_flags>();
                const char*    original_exe_or_file = this->vars.get<cfg::context::auth_command_rail_exec_original_exe_or_file>().c_str();
                const char*    exe_or_file          = this->vars.get<cfg::context::auth_command_rail_exec_exe_or_file>().c_str();
                const char*    working_dir          = this->vars.get<cfg::context::auth_command_rail_exec_working_dir>().c_str();
                const char*    arguments            = this->vars.get<cfg::context::auth_command_rail_exec_arguments>().c_str();
                const uint16_t exec_result          = this->vars.get<cfg::context::auth_command_rail_exec_exec_result>();
                const char*    account              = this->vars.get<cfg::context::auth_command_rail_exec_account>().c_str();
                const char*    password             = this->vars.get<cfg::context::auth_command_rail_exec_password>().c_str();

                if (!exec_result) {
                    //LOG(LOG_INFO,
                    //    "RailExec: "
                    //        "original_exe_or_file=\"%s\" "
                    //        "exe_or_file=\"%s\" "
                    //        "working_dir=\"%s\" "
                    //        "arguments=\"%s\" "
                    //        "flags=%u",
                    //    original_exe_or_file, exe_or_file, working_dir, arguments, flags);
                    this->auth_rail_exec(flags, original_exe_or_file, exe_or_file, working_dir, arguments, account, password);
                }
                else {
                    //LOG(LOG_INFO,
                    //    "RailExec: "
                    //        "exec_result=%u "
                    //        "original_exe_or_file=\"%s\" "
                    //        "flags=%u",
                    //    exec_result, original_exe_or_file, flags);
                    this->auth_rail_exec_cancel(flags, original_exe_or_file, exec_result);
                }
            }
        }
    }

    void rdp_input_scancode(long param1, long param2, long device_flags, long time, Keymap2 * /*keymap*/) override
    {
        if ((UP_AND_RUNNING == this->connection_finalization_state)
            && !this->input_event_disabled) {

            if (this->first_scancode && !(device_flags & 0x8000)) {
#ifndef __EMSCRIPTEN__
                if (this->channels.session_probe.enable_session_probe) {
                    if (!this->channels.session_probe_virtual_channel) {
                        ServerTransportContext stc{
                            this->trans, this->encrypt, this->negociation_result};
                        this->channels.create_session_probe_virtual_channel(
                            this->vars,
                            this->front, stc,
                            *this, this->lang,
                            this->bogus_refresh_rect,
                            this->monitor_count,
                            this->client_general_caps,
                            this->client_name);
                    }

                    if (!(this->channels.session_probe.session_probe_launcher
                       && this->channels.session_probe.session_probe_launcher->is_keyboard_sequences_started())
                     || this->channels.session_probe_virtual_channel->has_been_launched()
                    ) {
                        this->first_scancode = false;
                        this->send_input(time, RDP_INPUT_SYNCHRONIZE, 0, this->last_key_flags_sent, 0);
                    }
                }
                else
#endif
                {
                        this->first_scancode = false;
                        this->send_input(time, RDP_INPUT_SYNCHRONIZE, 0, this->last_key_flags_sent, 0);
                }
            }

            this->send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);

#ifndef __EMSCRIPTEN__
            IF_ENABLE_METRICS(key_pressed());

            if (this->channels.remote_programs_session_manager) {
                this->channels.remote_programs_session_manager->input_scancode(param1, param2, device_flags);
            }
#endif
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_UNICODE, flag, unicode, 0);
            IF_ENABLE_METRICS(key_pressed());
        }
    }

    void rdp_input_synchronize( uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        this->status_of_keyboard_toggle_keys = param1;

        (void)time;
        (void)param2;
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_SYNCHRONIZE, device_flags, param1, 0);
        }
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * /*keymap*/) override {
        if (UP_AND_RUNNING == this->connection_finalization_state
         && !this->input_event_disabled
        ) {
            this->send_input(0, RDP_INPUT_MOUSE, device_flags, x, y);

#ifndef __EMSCRIPTEN__
            IF_ENABLE_METRICS(mouse_event(device_flags, x, y));

            if (this->channels.remote_programs_session_manager) {
                this->channels.remote_programs_session_manager->input_mouse(device_flags, x, y);
            }
#endif
        }
    }

    // TODO: move to channels (and also remains here as it is mod API)
    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name,
        InStream & chunk, size_t length, uint32_t flags
    ) override {
        LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO,
            "mod_rdp::send_to_mod_channel: front_channel_channel=\"%s\"", front_channel_name);

        ServerTransportContext stc{
            this->trans, this->encrypt, this->negociation_result};
#ifndef __EMSCRIPTEN__
        this->channels.send_to_mod_channel(
            front_channel_name, chunk, length, flags,
            this->front, stc, this->client_general_caps, this->vars,
            this->client_rail_caps, this->client_name
        );
#else
        if (CHANNELS::ChannelDef const * mod_channel
            = this->channels.mod_channel_list.get_by_name(front_channel_name)
        ) {
            bytes_view data {chunk.get_data(), chunk.get_capacity()};
            CHANNELS::VirtualChannelPDU virtual_channel_pdu;
            virtual_channel_pdu.send_to_server(stc, mod_channel->chanid, length, flags, data);
        }
#endif
    }

    // Method used by session to transmit sesman answer for auth_channel
    // TODO: move to channels
    void send_auth_channel_data(const char * string_data)
    {
#ifndef __EMSCRIPTEN__
        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        StaticOutStream<65536> stream_data;
        uint32_t data_size = std::min(::strlen(string_data) + 1, stream_data.get_capacity());

        stream_data.out_copy_bytes(string_data, data_size);

        ServerTransportContext stc{
            this->trans, this->encrypt, this->negociation_result};
        virtual_channel_pdu.send_to_server(stc, this->channels.auth_channel_chanid
                                          , stream_data.get_offset()
                                          , this->channels.auth_channel_flags
                                          , stream_data.get_produced_bytes());
#else
        (void)string_data;
#endif
    }

    // TODO: move to channels (and also remains here as it is mod API)
    void send_checkout_channel_data(const char * string_data)
    {
#ifndef __EMSCRIPTEN__
        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        StaticOutStream<65536> stream_data;

        uint32_t data_size = std::min(::strlen(string_data), stream_data.get_capacity());

        stream_data.out_uint16_le(1);           // Version
        stream_data.out_uint16_le(data_size);
        stream_data.out_copy_bytes(string_data, data_size);

        ServerTransportContext stc{
            this->trans, this->encrypt, this->negociation_result};

        virtual_channel_pdu.send_to_server(stc, this->channels.checkout_channel_chanid
          , stream_data.get_offset()
          , this->channels.checkout_channel_flags
          , stream_data.get_produced_bytes());
#else
        (void)string_data;
#endif
    }

    void create_shadow_session(const char * userdata, const char * type)
    {
#ifndef __EMSCRIPTEN__
        LOG(LOG_INFO, "mod_rdp::create_shadow_session()");
        if (this->channels.session_probe_virtual_channel) {
            this->channels.session_probe_virtual_channel->create_shadow_session(userdata, type);
        }
#else
        (void)userdata;
        (void)type;
#endif
    }

public:
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

    void connected_fast_path(gdi::GraphicApi & drawable, writable_u8_array_view array)
    {
        InStream stream(array);
        IF_ENABLE_METRICS(server_main_channel_data(stream.in_remain()));

        FastPath::ServerUpdatePDU_Recv su(stream, this->decrypt, array.data());

        while (su.payload.in_remain()) {
            FastPath::Update_Recv upd(su.payload, this->mppc_dec);

            if (bool(this->verbose & RDPVerbose::connection)) {
                const char * m = "UNKNOWN ORDER";
                using FU = FastPath::UpdateType;
                switch (static_cast<FastPath::UpdateType>(upd.updateCode))
                {
                case FU::ORDERS:      m = "ORDERS"; break;
                case FU::BITMAP:      m = "BITMAP"; break;
                case FU::PALETTE:     m = "PALETTE"; break;
                case FU::SYNCHRONIZE: m = "SYNCHRONIZE"; break;
                case FU::SURFCMDS:    m = "SURFCMDS"; break;
                case FU::PTR_NULL:    m = "PTR_NULL"; break;
                case FU::PTR_DEFAULT: m = "PTR_DEFAULT"; break;
                case FU::PTR_POSITION:m = "PTR_POSITION"; break;
                case FU::COLOR:       m = "COLOR"; break;
                case FU::CACHED:      m = "CACHED"; break;
                case FU::POINTER:     m = "POINTER"; break;
                }

                const char * f = "UNKNOWN FRAGMENTATION";
                switch(upd.fragmentation) {
                case FastPath::FASTPATH_FRAGMENT_SINGLE: f = "single"; break;
                case FastPath::FASTPATH_FRAGMENT_FIRST: f = "first"; break;
                case FastPath::FASTPATH_FRAGMENT_LAST: f = "last"; break;
                case FastPath::FASTPATH_FRAGMENT_NEXT: f = "next"; break;
                }
                LOG(LOG_INFO, "FastPath::UpdateType::%s flags=%s len=%lu", m, f, upd.payload.in_remain());
                //upd.log(LOG_INFO);
            }

            if (upd.fragmentation != FastPath::FASTPATH_FRAGMENT_SINGLE) {
                if (upd.fragmentation == FastPath::FASTPATH_FRAGMENT_FIRST) {
                    this->multifragment_update_data.rewind();
                }
                this->multifragment_update_data.out_copy_bytes(upd.payload.get_data(), upd.payload.in_remain());
                if (upd.fragmentation != FastPath::FASTPATH_FRAGMENT_LAST) {
                    continue;
                }
            }

            InStream fud(this->multifragment_update_data.get_produced_bytes());

            InStream& stream = ((upd.fragmentation == FastPath::FASTPATH_FRAGMENT_SINGLE) ? upd.payload : fud);

            LOG_IF(bool(this->verbose & RDPVerbose::graphics),
                LOG_INFO, "fastpath full packet len=%lu", stream.in_remain());

            switch (static_cast<FastPath::UpdateType>(upd.updateCode)) {
            case FastPath::UpdateType::ORDERS:
                drawable.begin_update();
                this->orders.process_orders(
                    stream, true, drawable,
                    this->negociation_result.front_width, this->negociation_result.front_height);
                drawable.end_update();
                break;

            case FastPath::UpdateType::BITMAP:
                drawable.begin_update();
                this->process_bitmap_updates(stream, true, drawable);
                drawable.end_update();
                break;

            case FastPath::UpdateType::PALETTE:
                drawable.begin_update();
                this->process_palette(drawable, stream, true);
                drawable.end_update();
                break;

            case FastPath::UpdateType::SYNCHRONIZE:
                // TODO: we should propagate SYNCHRONIZE to front
                break;

            case FastPath::UpdateType::SURFCMDS:
                this->process_surface_command(stream, drawable);
                break;

            case FastPath::UpdateType::PTR_NULL:
                LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer null (Fast)");
                drawable.set_pointer(0, null_pointer(), gdi::GraphicApi::SetPointerMode::Insert);
                break;

            case FastPath::UpdateType::PTR_DEFAULT:
                LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer default (Fast)");
                drawable.set_pointer(0, system_normal_pointer(), gdi::GraphicApi::SetPointerMode::Insert);
                break;

            case FastPath::UpdateType::PTR_POSITION:
                {
                    LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer position (Fast)");

                    /* xPos(2) + yPos(2) */
                    // TODO: there was a break instead of a throw here, see why.
                    ::check_throw(stream, 4, "mod_rdp::Fast-Path Pointer Position Update", ERR_RDP_DATA_TRUNCATED);

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
                LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer color (Fast)");
//                 this->process_color_pointer_pdu(stream, drawable);
                this->process_new_pointer_pdu(BitsPerPixel{24}, stream, drawable);
                break;

            case FastPath::UpdateType::CACHED:
                LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer cached (Fast)");
                this->process_cached_pointer_pdu(stream, drawable);
                break;

            case FastPath::UpdateType::POINTER:
            {
                LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO,
                    "Process pointer new (Fast)");
                BitsPerPixel data_bpp = checked_int(stream.in_uint16_le()); /* data bpp */
                this->process_new_pointer_pdu(data_bpp, stream, drawable);
            }
            break;

            default:
                LOG( LOG_ERR
                , "mod::rdp: received unexpected fast-path PUD, updateCode = %u"
                , upd.updateCode);
                throw Error(ERR_RDP_FASTPATH);
            }
        }

        // TODO Check that all data in the PDU have been consumed
    }


    // 2.2.9.1.2.1.10.1
    // Surface Command (TS_SURFCMD)
    // The TS_SURFCMD structure is used to specify the Surface Command type and to encapsulate the data
    // for a Surface Command sent from a server to a client. All Surface Commands in section 2.2.9.2
    // conform to this structure.
    //
    // +-------------------------+--------------------+
    // |          cmdType        | cmdData (variable) |
    // +-------------------------+--------------------+
    // |                      ...                     |
    // +----------------------------------------------+
    //
    // cmdType (2 bytes): A 16-bit unsigned integer. Surface Command type.
    // +---------------------------+---------------------------------------+
    // |           Value           |              Meaning                  |
    // +---------------------------+---------------------------------------+
    // | CMDTYPE_SET_SURFACE_BITS  | Indicates a Set Surface Bits Command  |
    // |          0x0001           |     (section 2.2.9.2.1).              |
    // +---------------------------+---------------------------------------+
    // | CMDTYPE_FRAME_MARKER      | Indicates a Frame Marker Command      |
    // |          0x0004           |     (section 2.2.9.2.3).              |
    // +---------------------------+---------------------------------------+
    // |CMDTYPE_STREAM_SURFACE_BITS|Indicates a Stream Surface Bits Command|
    // |          0x0006           |     (section 2.2.9.2.2).              |
    // +---------------------------+---------------------------------------+
    //
    // cmdData (variable): Variable-length data specific to the Surface Command.
    //
    enum {
        CMDTYPE_SET_SURFACE_BITS = 0x0001,
        CMDTYPE_FRAME_MARKER = 0x0004,
        CMDTYPE_STREAM_SURFACE_BITS = 0x0006
    };


    void process_surface_command(InStream & stream, gdi::GraphicApi & drawable) {
        while (stream.in_check_rem(2)) {
            ::check_throw(stream, 2, "mod_rdp::SurfaceCommand", ERR_RDP_DATA_TRUNCATED);

            unsigned expected = 2;

            uint16_t cmdType = stream.in_uint16_le();

            switch(cmdType) {
            case CMDTYPE_SET_SURFACE_BITS:
            case CMDTYPE_STREAM_SURFACE_BITS: {
                RDPSetSurfaceCommand setSurface;

                setSurface.recv(stream);

                if (std::find(this->remoteFx_codec_id.begin(), this->remoteFx_codec_id.end(), setSurface.codecId) != this->remoteFx_codec_id.end()) {
                    LOG_IF(bool(this->verbose & RDPVerbose::surfaceCmd), LOG_INFO, "setSurfaceBits: remoteFX codec");
                    setSurface.codec = RDPSetSurfaceCommand::SETSURFACE_CODEC_REMOTEFX;

                    InStream remoteFxStream(bytes_view(stream.get_current(), setSurface.bitmapDataLength));
                    drawable.draw(setSurface);
                    this->rfxDecoder.recv(remoteFxStream, setSurface, drawable);
                }
                else {
                    LOG(LOG_INFO, "unknown codecId=%u", setSurface.codecId);
                }
                stream.in_skip_bytes(setSurface.bitmapDataLength);
                break;
            }

            case CMDTYPE_FRAME_MARKER: {
                // 2.2.9.2.3 Frame Marker Command (TS_FRAME_MARKER)
                // The Frame Marker Command is used to group multiple surface commands so that these commands
                // can be processed and presented to the user as a single entity, a frame.
                //
                // cmdType (2 bytes): A 16-bit, unsigned integer. Surface Command type. This field MUST be set to
                //         CMDTYPE_FRAME_MARKER (0x0004).
                // frameAction (2 bytes): A 16-bit, unsigned integer. Identifies the beginning and end of a frame.
                // +------------------------------+-------------------------------------+
                // |             Value            |         Meaning                     |
                // +------------------------------+-------------------------------------+
                // | SURFACECMD_FRAMEACTION_BEGIN | Indicates the start of a new frame. |
                // |            0x0000            |                                     |
                // +------------------------------+-------------------------------------+
                // | SURFACECMD_FRAMEACTION_END   | Indicates the end of the current    |
                // |             0x0001           | frame.                              |
                // +------------------------------+-------------------------------------+
                //
                // frameId (4 bytes): A 32-bit, unsigned integer. The ID identifying the frame.
                //
                enum {
                    SURFACECMD_FRAMEACTION_BEGIN = 0x0000,
                    SURFACECMD_FRAMEACTION_END = 0x0001
                };

                expected = 6;
                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR, "Truncated FrameMarker, need=%u remains=%zu", expected, stream.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }

                uint16_t frameAction = stream.in_uint16_le();
                uint32_t frameId = stream.in_uint32_le();
                LOG_IF(bool(this->verbose & RDPVerbose::surfaceCmd), LOG_DEBUG, "setSurfaceBits: frameMarker action=%u frameId=%u",
                        frameAction, frameId);

                switch(frameAction) {
                case SURFACECMD_FRAMEACTION_BEGIN:
                    LOG(LOG_DEBUG, "surfaceCmd frame begin(inProgress=%" PRIu32 " lastFrame=0x%" PRIx32 ")", this->frameInProgress, this->currentFrameId);
                    if (this->frameInProgress) {
                        // some servers don't send frame end markers, so send acks when we receive
                        // a new frame and the previous one was not acked
                        drawable.end_update();

                        if (this->haveSurfaceFrameAck) {
                            LOG(LOG_DEBUG, "surfaceCmd framebegin, sending frameAck id=0x%x", this->currentFrameId);
                            uint32_t localLastFrame = this->currentFrameId;
                            this->send_pdu_type2(
                                PDUTYPE2_FRAME_ACKNOWLEDGE, RDP::STREAM_HI,
                                [localLastFrame](StreamSize<32> /*maxlen*/, OutStream & ostream) {
                                    ostream.out_uint32_le(localLastFrame);
                                }
                            );
                        }
                    }

                    this->frameInProgress = true;
                    this->currentFrameId = frameId;
                    drawable.begin_update();
                    break;
                case SURFACECMD_FRAMEACTION_END:
                    LOG(LOG_DEBUG, "surfaceCmd frameEnd, sending frameAck id=0x%x", frameId);
                    this->frameInProgress = false;
                    drawable.end_update();

                    if (this->haveSurfaceFrameAck) {
                        this->send_pdu_type2(
                            PDUTYPE2_FRAME_ACKNOWLEDGE, RDP::STREAM_HI,
                            [frameId](StreamSize<32> /*maxlen*/, OutStream & ostream) {
                                ostream.out_uint32_le(frameId);
                            }
                        );
                    }
                    break;
                default:
                    LOG(LOG_ERR, "unknown frame marker action %u", frameAction);
                    break;
                }

                break;
            }
            default:
                LOG(LOG_ERR, "unknown surface command 0x%x", cmdType);
                break;
            }
        }
    }

    void connected_slow_path(gdi::GraphicApi & drawable, InStream & stream)
    {
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)

        X224::DT_TPDU_Recv x224(stream);

        IF_ENABLE_METRICS(server_main_channel_data(stream.in_remain()));

        const int mcs_type = MCS::peekPerEncodedMCSType(x224.payload);

        if (mcs_type == MCS::MCSPDU_DisconnectProviderUltimatum){
            LOG(LOG_INFO, "mod::rdp::DisconnectProviderUltimatum received");
            this->mcs_disconnect_provider_ultimatum_pdu_received = true;

            x224.payload.rewind();
            MCS::DisconnectProviderUltimatum_Recv mcs(x224.payload, MCS::PER_ENCODING);
            const char * reason = MCS::get_reason(mcs.reason);
            LOG(LOG_INFO, "mod::rdp::DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);

            this->connection_finalization_state = DISCONNECTED;
            this->session_log.report("CLOSE_SESSION_SUCCESSFUL", "OK.");
            this->log_disconnection(bool(this->verbose & RDPVerbose::sesprobe));
            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
        }


        MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);
        SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->negociation_result.encryptionLevel);
        if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL){
            // TODO: this should move to channels
            LOG_IF(bool(this->verbose & RDPVerbose::channels), LOG_INFO, "received channel data on mcs.chanid=%u", mcs.channelId);

            int num_channel_src = this->channels.mod_channel_list.get_index_by_id(mcs.channelId);
            if (num_channel_src == -1) {
                LOG(LOG_ERR, "mod::rdp::MOD_RDP_CONNECTED::Unknown Channel id=%d", mcs.channelId);
                throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
            }

            const CHANNELS::ChannelDef & mod_channel = this->channels.mod_channel_list[num_channel_src];
            if (bool(this->verbose & RDPVerbose::channels)) {
                mod_channel.log(num_channel_src);
            }

            uint32_t length = sec.payload.in_uint32_le();
            int flags = sec.payload.in_uint32_le();
            size_t chunk_size = sec.payload.in_remain();

#ifndef __EMSCRIPTEN__
            // If channel name is our virtual channel, then don't send data to front
            if ((mod_channel.name == this->channels.auth_channel) && this->channels.enable_auth_channel) {
                ServerTransportContext stc{
                    this->trans, this->encrypt, this->negociation_result};
                this->channels.process_auth_event(this->vars, mod_channel, sec.payload, length, flags, chunk_size, this->front, stc, this->client_general_caps, this->client_name);
            }
            else if (mod_channel.name == this->channels.checkout_channel) {
                this->channels.process_checkout_event(this->vars, mod_channel, sec.payload, length, flags, chunk_size);
            }
            else if (mod_channel.name == channel_names::sespro) {
                ServerTransportContext stc{
                    this->trans, this->encrypt, this->negociation_result};

                if (!this->channels.session_probe_virtual_channel) {
                    this->channels.create_session_probe_virtual_channel(
                        this->vars,
                        this->front, stc,
                        *this, this->lang,
                        this->bogus_refresh_rect,
                        this->monitor_count,
                        this->client_general_caps,
                        this->client_name);
                }
                this->channels.process_session_probe_event(sec.payload, length, flags, chunk_size);
            }
            // Clipboard is a Clipboard PDU
            else if (mod_channel.name == channel_names::cliprdr) {
                IF_ENABLE_METRICS(set_server_cliprdr_metrics(sec.payload.clone(), length, flags));
                ServerTransportContext stc{
                    this->trans, this->encrypt, this->negociation_result};
                this->channels.process_cliprdr_event(sec.payload, length, flags, chunk_size, this->front, stc, this->file_validator_service);
            }
            else if (mod_channel.name == channel_names::rail) {
                IF_ENABLE_METRICS(server_rail_channel_data(length));
                ServerTransportContext stc{
                    this->trans, this->encrypt, this->negociation_result};
                this->channels.process_rail_event(
                    mod_channel, sec.payload, length, flags, chunk_size,
                    this->front, stc, this->vars, this->client_rail_caps);
            }
            else if (mod_channel.name == channel_names::rdpdr) {
                IF_ENABLE_METRICS(set_server_rdpdr_metrics(sec.payload.clone(), length, flags));
                ServerTransportContext stc{
                    this->trans, this->encrypt, this->negociation_result};
                this->channels.process_rdpdr_event(sec.payload, length, flags, chunk_size, this->front, stc, this->client_general_caps, this->client_name);
            }
            else if (mod_channel.name == channel_names::drdynvc) {
                IF_ENABLE_METRICS(server_other_channel_data(length));
                ServerTransportContext stc{
                    this->trans, this->encrypt, this->negociation_result};
                this->channels.process_drdynvc_event(sec.payload, length, flags, chunk_size, this->front, stc);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                    LOG_INFO, "mod_rdp::process unknown channel: mod_channel.name=%" PRIX64 " %s",uint64_t(mod_channel.name), mod_channel.name);
                IF_ENABLE_METRICS(server_other_channel_data(length));
                this->channels.process_unknown_channel_event(mod_channel, sec.payload, length, flags, chunk_size, this->front);
            }
#else
            if (const CHANNELS::ChannelDef * front_channel = front.get_channel_list().get_by_name(mod_channel.name)) {
                front.send_to_channel(*front_channel, {sec.payload.get_current(), chunk_size}, length, flags);
            }
#endif

            sec.payload.in_skip_bytes(sec.payload.in_remain());

        }
        else {

            uint8_t const * next_packet = sec.payload.get_current();
            while (next_packet < sec.payload.get_data_end()) {
                sec.payload.rewind();
                sec.payload.in_skip_bytes(next_packet - sec.payload.get_data());

                if  (peekFlowPDU(sec.payload)){
                    LOG_IF(bool(this->verbose & RDPVerbose::connection), LOG_WARNING, "FlowPDU TYPE");
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

                    LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_WARNING, "LOOPING on PDUs: %u", unsigned(sctrl.totalLength));

                    switch (sctrl.pduType) {
                    case PDUTYPE_DATAPDU:
                        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_WARNING, "PDUTYPE_DATAPDU");
                        switch (this->connection_finalization_state){
                        case EARLY:
                        {
                            ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                            // sdata.log();
                            LOG(LOG_ERR, "sdata.pdutype2=%u", sdata.pdutype2);

                            if (sdata.pdutype2 == PDUTYPE2_SET_ERROR_INFO_PDU)
                            {
                                LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                    LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");
                                uint32_t error_info = this->get_error_info_from_pdu(sdata.payload);
                                this->errinfo_encountered = (0 != error_info);
                                this->process_error_info(error_info);

                                if (error_info == ERRINFO_SERVER_DENIED_CONNECTION) {
                                    str_append(
                                        this->close_box_extra_message_ref, ' ',
                                        TR(trkeys::err_server_denied_connection, this->lang)
                                    );
                                }
                            }

                            LOG(LOG_ERR, "Rdp::finalization is early");
                            throw Error(ERR_SEC);
                        }
                        case WAITING_SYNCHRONIZE:
                            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                                LOG_WARNING, "WAITING_SYNCHRONIZE");

                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();

                                if (sdata.pdutype2 == PDUTYPE2_MONITOR_LAYOUT_PDU) {

                                    MonitorLayoutPDU monitor_layout_pdu;

                                    monitor_layout_pdu.recv(sdata.payload);
                                    monitor_layout_pdu.log(
                                        "Rdp::receiving the server-to-client Monitor Layout PDU");

                                    if ((monitor_layout_pdu.get_monitorCount() !=
                                         (this->monitor_count ? this->monitor_count : 1)) &&
                                        (!this->accept_monitor_layout_change_if_capture_is_not_started || this->front.is_capture_in_progress())) {

                                        LOG(LOG_ERR, "Server do not support the display monitor layout of the client");
                                        throw Error(ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT);
                                    }

                                    this->front.server_relayout(monitor_layout_pdu);

                                    this->monitor_count = ((monitor_layout_pdu.get_monitorCount() == 1) ? 0 : monitor_layout_pdu.get_monitorCount());

#ifndef __EMSCRIPTEN__
                                    if (this->channels.session_probe_virtual_channel) {
                                        this->channels.session_probe_virtual_channel->enable_bogus_refresh_rect_ex_support(this->bogus_refresh_rect && this->monitor_count);
                                    }
#endif
                                }
                                else {
                                    LOG(LOG_INFO, "Resizing to %ux%ux%u", this->negociation_result.front_width, this->negociation_result.front_height, this->orders.get_bpp());

                                    auto resize_result = this->front.server_resize({this->negociation_result.front_width, this->negociation_result.front_height, this->orders.get_bpp()});
                                    switch (resize_result){
                                    case FrontAPI::ResizeResult::fail:
                                    {
                                        LOG(LOG_ERR, "Resize not available on older clients,"
                                            " change client resolution to match server resolution");
                                        throw Error(ERR_RDP_RESIZE_NOT_AVAILABLE);
                                    }
                                    case FrontAPI::ResizeResult::done:
                                    case FrontAPI::ResizeResult::instant_done:
                                    case FrontAPI::ResizeResult::no_need:
                                    case FrontAPI::ResizeResult::remoteapp:
                                        break;
                                    }

                                    this->connection_finalization_state = WAITING_CTL_COOPERATE;
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                }
                            }
                            break;
                        case WAITING_CTL_COOPERATE:
                            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                                LOG_WARNING, "WAITING_CTL_COOPERATE");
                            this->connection_finalization_state = WAITING_GRANT_CONTROL_COOPERATE;
                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();
                                sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                            }
                            break;
                        case WAITING_GRANT_CONTROL_COOPERATE:
                            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                                LOG_WARNING, "WAITING_GRANT_CONTROL_COOPERATE");
                            this->connection_finalization_state = WAITING_FONT_MAP;
                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();
                                sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                            }
                            break;
                        case WAITING_FONT_MAP:
                            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                                LOG_WARNING, "PDUTYPE2_FONTMAP");
                            this->connection_finalization_state = UP_AND_RUNNING;

                            if (!this->deactivation_reactivation_in_progress) {
                                this->session_log.log6(LogId::SESSION_ESTABLISHED_SUCCESSFULLY, {});
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
#ifndef __EMSCRIPTEN__
                                if (this->channels.session_probe.enable_session_probe) {
                                    ServerTransportContext stc{
                                        this->trans, this->encrypt, this->negociation_result};
                                    this->channels.do_enable_session_probe(
                                        this->front,
                                        stc,
                                        *this,
                                        this->client_general_caps,
                                        this->vars,
                                        this->client_rail_caps,
                                        this->client_name,
                                        this->monitor_count,
                                        this->bogus_refresh_rect,
                                        this->lang,
                                        this->file_validator_service);
                                }
#endif
                                this->already_upped_and_running = true;
                            }

#ifndef __EMSCRIPTEN__
                            if (this->channels.session_probe.enable_launch_mask) {
                                this->delayed_start_capture = true;

                                LOG(LOG_INFO, "Mod_rdp: Capture starting is delayed.");
                            }
                            else
#endif
                            if (this->front.can_be_start_capture(false, this->session_log)) {
                                if (this->bogus_refresh_rect && this->monitor_count) {
                                    this->rdp_suppress_display_updates();
                                    this->rdp_allow_display_updates(
                                        0, 0,
                                        this->negociation_result.front_width,
                                        this->negociation_result.front_height);
                                }
                                this->rdp_input_invalidate(Rect(0, 0, this->negociation_result.front_width, this->negociation_result.front_height));
                            }
                            break;
                        case UP_AND_RUNNING:
                            {
                                ShareData_Recv sdata(sctrl.payload, &this->mppc_dec);
                                // sdata.log();

                                switch (sdata.pdutype2) {
                                case PDUTYPE2_UPDATE:
                                    {
                                        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                                            LOG_INFO, "PDUTYPE2_UPDATE");
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
                                            LOG_IF(bool(this->verbose & RDPVerbose::graphics),
                                                LOG_INFO, "RDP_UPDATE_ORDERS");
                                            drawable.begin_update();
                                            this->orders.process_orders(sdata.payload, false,
                                                drawable, this->negociation_result.front_width, this->negociation_result.front_height);
                                            drawable.end_update();
                                            break;
                                        case RDP_UPDATE_BITMAP:
                                            LOG_IF(bool(this->verbose & RDPVerbose::graphics),
                                                LOG_INFO, "RDP_UPDATE_BITMAP");
                                            drawable.begin_update();
                                            this->process_bitmap_updates(sdata.payload, false, drawable);
                                            drawable.end_update();
                                            break;
                                        case RDP_UPDATE_PALETTE:
                                            LOG_IF(bool(this->verbose & RDPVerbose::graphics),
                                                LOG_INFO, "RDP_UPDATE_PALETTE");
                                            drawable.begin_update();
                                            this->process_palette(drawable, sdata.payload, false);
                                            drawable.end_update();
                                            break;
                                        case RDP_UPDATE_SYNCHRONIZE:
                                            LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                                LOG_INFO, "RDP_UPDATE_SYNCHRONIZE");
                                            sdata.payload.in_skip_bytes(2);
                                            break;
                                        default:
                                            LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                                LOG_WARNING, "mod_rdp::MOD_RDP_CONNECTED:RDP_UPDATE_UNKNOWN");
                                            break;
                                        }
                                    }
                                    break;
                                case PDUTYPE2_CONTROL:
                                    LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                        LOG_INFO, "PDUTYPE2_CONTROL");
                                    // TODO CGR: Data should actually be consumed
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_SYNCHRONIZE:
                                    LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                        LOG_INFO, "PDUTYPE2_SYNCHRONIZE");
                                    // TODO CGR: Data should actually be consumed
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_POINTER:
                                    LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
                                        LOG_INFO, "PDUTYPE2_POINTER");
                                    this->process_pointer_pdu(sdata.payload, drawable);
                                    // TODO CGR: Data should actually be consumed
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_PLAY_SOUND:
                                    LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                        LOG_INFO, "PDUTYPE2_PLAY_SOUND");
                                    // TODO CGR: Data should actually be consumed
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                case PDUTYPE2_SAVE_SESSION_INFO:
                                    LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                        LOG_INFO, "PDUTYPE2_SAVE_SESSION_INFO");
                                    // TODO CGR: Data should actually be consumed
                                    this->process_save_session_info(sdata.payload);
                                    break;
                                case PDUTYPE2_SET_ERROR_INFO_PDU:
                                    {
                                        LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                            LOG_INFO, "PDUTYPE2_SET_ERROR_INFO_PDU");
                                        uint32_t error_info = this->get_error_info_from_pdu(sdata.payload);
                                        this->errinfo_encountered = (0 != error_info);
                                        this->process_error_info(error_info);

                                        if (   (ERRINFO_GRAPHICSSUBSYSTEMFAILED == error_info)
                                            && this->is_server_auto_reconnec_packet_received)
                                        {
                                            this->errinfo_graphics_subsystem_failed_encountered = true;
                                            throw Error(ERR_AUTOMATIC_RECONNECTION_REQUIRED);
                                        }
                                    }
                                    break;
                                case PDUTYPE2_SHUTDOWN_DENIED:
                                    //if (bool(this->verbose & RDPVerbose::connection)){ LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED");}
                                    LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_DENIED Received");
                                    break;

                                case PDUTYPE2_SET_KEYBOARD_INDICATORS:
                                    {
                                        LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                            LOG_INFO, "PDUTYPE2_SET_KEYBOARD_INDICATORS");

                                        sdata.payload.in_skip_bytes(2); // UnitId(2)

                                        uint16_t LedFlags = sdata.payload.in_uint16_le();

                                        this->front.set_keyboard_indicators(LedFlags);

                                        assert(sdata.payload.get_current() == sdata.payload.get_data_end());
                                    }
                                    break;

                                default:
                                    LOG(LOG_WARNING, "PDUTYPE2 unsupported tag=%u", sdata.pdutype2);
                                    // TODO CGR: Data should actually be consumed
                                    hexdump(sdata.payload.remaining_bytes());
                                    sdata.payload.in_skip_bytes(sdata.payload.in_remain());
                                    break;
                                }
                            }
                            break;
                        case DISCONNECTED: break;
                        }
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU:
                        {
                            LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");

                            this->orders.reset();

                            this->process_demand_active(sctrl);
                            this->send_confirm_active(drawable);
                            this->send_synchronise();
                            this->send_control(RDP_CTL_COOPERATE);
                            this->send_control(RDP_CTL_REQUEST_CONTROL);

                            /* Including RDP 5.0 capabilities */
                            if (this->negociation_result.use_rdp5){
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

                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                        }
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        LOG_IF(bool(this->verbose & RDPVerbose::connection),
                            LOG_INFO, "PDUTYPE_DEACTIVATEALLPDU");
                        this->deactivation_reactivation_in_progress = true;
                        // TODO CGR: Data should actually be consumed
                            // TODO CGR: Check we are indeed expecting Synchronize... dubious
                            this->connection_finalization_state = WAITING_SYNCHRONIZE;
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        {
                            LOG_IF(bool(this->verbose & RDPVerbose::connection),
                                LOG_INFO, "PDUTYPE_SERVER_REDIR_PKT");
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

    void draw_event()
    {
        while (this->buf.next(TpduBuffer::PDU)) {
            InStream x224_data(this->buf.current_pdu_buffer());

            try{
                gdi::GraphicApi & drawable =
#ifndef __EMSCRIPTEN__
                    this->channels.remote_programs_session_manager
                      ? *this->channels.remote_programs_session_manager
                      :
#endif
                    this->graphics_update_disabled
                      ? gdi::null_gd()
                      : gd;
                if (this->buf.current_pdu_get_type() == Extractors::FASTPATH) {
                    this->connected_fast_path(drawable, this->buf.current_pdu_buffer());
                }
                else {
                    this->connected_slow_path(drawable, x224_data);
                }
            }
            catch(Error const & e){
                LOG(LOG_INFO, "mod_rdp::draw_event() state switch raised exception = %s", e.errmsg());

                if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED)
                {
                    throw;
                }

                if (e.id == ERR_RDP_SERVER_REDIR) {
                    if (!this->support_connection_redirection_during_recording) {
                        this->front.must_be_stop_capture();
                    }
                    throw;
                }

#ifndef __EMSCRIPTEN__
                if (this->channels.session_probe_virtual_channel
                &&  this->channels.session_probe_virtual_channel->is_disconnection_reconnection_required()) {
                    throw Error(ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION);
                }
#endif
                this->front.must_be_stop_capture();

                if (this->remote_apps_not_enabled) {
                    throw Error(ERR_RAIL_NOT_ENABLED);
                }

                if (e.id != ERR_MCS_APPID_IS_MCS_DPUM)
                {
                    char const* reason =
                        ((UP_AND_RUNNING == this->connection_finalization_state) ?
                        "SESSION_EXCEPTION" : "SESSION_EXCEPTION_NO_RECORD");

                    this->session_log.report(reason, e.errmsg());
                }

                if ((e.id == ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED) ||
                    (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_MISSED) ||
                    (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED) ||
                    (e.id == ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE) ||
                    (e.id == ERR_NLA_AUTHENTICATION_FAILED)) {
                    throw;
                }

                if (e.id == ERR_MCS_APPID_IS_MCS_DPUM)
                {
                    throw;
                }

                if (this->mcs_disconnect_provider_ultimatum_pdu_received) {
                    StaticOutStream<256> stream;
                    X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
                    try {
                        this->trans.send(stream.get_produced_bytes());
                        LOG(LOG_INFO, "Connection to server closed");
                    }
                    catch(Error const & e){
                        LOG(LOG_INFO, "Connection to server Already closed: error=%u", e.id);
                    }
                }

#ifndef __EMSCRIPTEN__
                if (this->channels.session_probe.enable_session_probe) {
                    this->enable_input_event();
                    this->enable_graphics_update();
                }
#endif

                if ((e.id == ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT) ||
                    (e.id == ERR_RAIL_CLIENT_EXECUTE) ||
                    (e.id == ERR_RAIL_STARTING_PROGRAM) ||
                    (e.id == ERR_RAIL_UNAUTHORIZED_PROGRAM)) {
                    throw;
                }

                if (UP_AND_RUNNING != this->connection_finalization_state
                && DISCONNECTED != this->connection_finalization_state
                && !this->already_upped_and_running) {
                    const char * statedescr = TR(trkeys::err_mod_rdp_connected, this->lang);
                    str_append(
                        this->close_box_extra_message_ref,
                        ' ',
                        statedescr,
                        " (CONNECTED)");
                    LOG(LOG_ERR, "Creation of new mod 'RDP' failed at CONNECTED state. %s",
                        statedescr);
                    throw Error(ERR_SESSION_UNKNOWN_BACKEND);
                }

                this->set_mod_signal(BACK_EVENT_NEXT);
            }
        }

        this->front.must_flush_capture();
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

    void send_confirm_active(gdi::GraphicApi& drawable) {
        LOG_IF(bool(this->verbose & RDPVerbose::capabilities),
            LOG_INFO, "mod_rdp::send_confirm_active");
        this->send_data_request_ex(
            GCC::MCS_GLOBAL_CHANNEL,
            [this, &drawable](StreamSize<65536> /*maxlen*/, OutStream & stream) {
                RDP::ConfirmActivePDU_Send confirm_active_pdu(stream);

                confirm_active_pdu.emit_begin(this->share_id);

                GeneralCaps general_caps;
                general_caps.os_major = 4;
                general_caps.os_minor = 7;
                general_caps.extraflags  =
                    this->negociation_result.use_rdp5
                    ? NO_BITMAP_COMPRESSION_HDR | AUTORECONNECT_SUPPORTED | LONG_CREDENTIALS_SUPPORTED
                    : 0
                    ;
                // Slow/Fast-path
                general_caps.extraflags |=
                    this->enable_fastpath_server_update
                    ? FASTPATH_OUTPUT_SUPPORTED
                    : 0
                    ;
                general_caps.refreshRectSupport = true;
                general_caps.suppressOutputSupport = true;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    general_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(general_caps);

                BitmapCaps bitmap_caps;
                // TODO Client SHOULD set this field to the color depth requested in the Client Core Data
                bitmap_caps.preferredBitsPerPixel = safe_int(this->orders.get_bpp());
                // bitmap_caps.preferredBitsPerPixel = this->front_bpp
                bitmap_caps.desktopWidth          = this->negociation_result.front_width;
                bitmap_caps.desktopHeight         = this->negociation_result.front_height;
                bitmap_caps.bitmapCompressionFlag = 0x0001; // This field MUST be set to TRUE (0x0001).
                //bitmap_caps.drawingFlags = DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY | DRAW_ALLOW_COLOR_SUBSAMPLING | DRAW_ALLOW_SKIP_ALPHA;
                bitmap_caps.drawingFlags = DRAW_ALLOW_SKIP_ALPHA;

                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    bitmap_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(bitmap_caps);

                OrderCaps order_caps;
                order_caps.numberFonts = 0;
                order_caps.orderFlags  = /*0x2a*/
                                         NEGOTIATEORDERSUPPORT   /* 0x02 */
                                       | ZEROBOUNDSDELTASSUPPORT /* 0x08 */
                                       | COLORINDEXSUPPORT       /* 0x20 */
                                       | ORDERFLAGS_EXTRA_FLAGS  /* 0x80 */
                                       ;

                // intersect with client order capabilities
                // which may not be supported by clients.

                for (auto idx : order_indexes_supported()) {
                    order_caps.orderSupport[idx] = this->primary_drawing_orders_support.test(idx);
                }

                order_caps.textFlags                  = 0x06a1;
                order_caps.orderSupportExFlags        = ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT;
                order_caps.textANSICodePage           = 0x4e4; // Windows-1252 codepage is passed (latin-1)

                LOG_IF(bool(this->verbose & RDPVerbose::capabilities) && !order_caps.orderSupport[TS_NEG_MEMBLT_INDEX],
                    LOG_INFO, "MemBlt Primary Drawing Order is disabled.");

                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    order_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(order_caps);

                BmpCacheCaps bmpcache_caps;
                bmpcache_caps.cache0Entries         = 0x258;
                bmpcache_caps.cache0MaximumCellSize = nb_bytes_per_pixel(this->orders.get_bpp()) * 0x100;
                bmpcache_caps.cache1Entries         = 0x12c;
                bmpcache_caps.cache1MaximumCellSize = nb_bytes_per_pixel(this->orders.get_bpp()) * 0x400;
                bmpcache_caps.cache2Entries         = 0x106;
                bmpcache_caps.cache2MaximumCellSize = nb_bytes_per_pixel(this->orders.get_bpp()) * 0x1000;

                BmpCache2Caps bmpcache2_caps;
                bmpcache2_caps.cacheFlags           = PERSISTENT_KEYS_EXPECTED_FLAG;
                if (this->enable_cache_waiting_list
                 || (this->enable_remotefx && this->haveRemoteFx)
                ) {
                    bmpcache2_caps.cacheFlags |= ALLOW_CACHE_WAITING_LIST_FLAG;
                }
                bmpcache2_caps.numCellCaches        = 3;
                bmpcache2_caps.bitmapCache0CellInfo = this->BmpCacheRev2_Cache_NumEntries()[0];
                bmpcache2_caps.bitmapCache1CellInfo = this->BmpCacheRev2_Cache_NumEntries()[1];
                bmpcache2_caps.bitmapCache2CellInfo = (this->BmpCacheRev2_Cache_NumEntries()[2] | 0x80000000);

                bool use_bitmapcache_rev2 = this->enable_persistent_disk_bitmap_cache || (this->enable_remotefx && this->haveRemoteFx);

                if (use_bitmapcache_rev2) {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bmpcache2_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(bmpcache2_caps);

                    if (!this->deactivation_reactivation_in_progress) {
                        this->orders.create_cache_bitmap(
                            drawable,
                            this->BmpCacheRev2_Cache_NumEntries()[0], nb_bytes_per_pixel(this->orders.get_bpp()) * 16 * 16, false,
                            this->BmpCacheRev2_Cache_NumEntries()[1], nb_bytes_per_pixel(this->orders.get_bpp()) * 32 * 32, false,
                            this->BmpCacheRev2_Cache_NumEntries()[2], nb_bytes_per_pixel(this->orders.get_bpp()) * 64 * 64, this->enable_persistent_disk_bitmap_cache,
                            this->enable_cache_waiting_list,
                            this->cache_verbose);
                    }
                }
                else {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bmpcache_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(bmpcache_caps);

                    if (!this->deactivation_reactivation_in_progress) {
                        this->orders.create_cache_bitmap(
                            drawable,
                            0x258, nb_bytes_per_pixel(this->orders.get_bpp()) * 0x100,  false,
                            0x12c, nb_bytes_per_pixel(this->orders.get_bpp()) * 0x400,  false,
                            0x106, nb_bytes_per_pixel(this->orders.get_bpp()) * 0x1000, false,
                            false,
                            this->cache_verbose);
                    }
                }

                PointerCaps pointer_caps;
                pointer_caps.len                       = 10;
                if (!this->enable_new_pointer) {
                    // TODO std::size(this->cursors) = 32
                    pointer_caps.pointerCacheSize      = 0;
                    pointer_caps.colorPointerCacheSize = 20;
                    pointer_caps.len                   = 8;
                    assert(pointer_caps.colorPointerCacheSize <= sizeof(this->cursors) / sizeof(Pointer));
                }
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    pointer_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(pointer_caps);

                InputCaps input_caps;
                input_caps.inputFlags |= INPUT_FLAG_FASTPATH_INPUT;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    input_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(input_caps);

                BrushCacheCaps brush_caps;
                // brush_caps.brushSupportLevel = BRUSH_COLOR_FULL;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    brush_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(brush_caps);

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

#ifndef __EMSCRIPTEN__
                VirtualChannelCaps virtual_channel_caps;
                virtual_channel_caps.VCChunkSize = CHANNELS::CHANNEL_CHUNK_LENGTH;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    virtual_channel_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(virtual_channel_caps);

                SoundCaps sound_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    sound_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(sound_caps);

                ShareCaps share_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    share_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(share_caps);

                FontCaps font_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    font_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(font_caps);

                ControlCaps control_caps;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    control_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(control_caps);

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


                if (this->channels.remote_app.enable_remote_program) {
                    RailCaps rail_caps = this->client_rail_caps;
                    if (this->channels.remote_app.convert_remoteapp_to_desktop) {
                        rail_caps.RailSupportLevel = 0x83;
                    }
                    rail_caps.RailSupportLevel &= (TS_RAIL_LEVEL_SUPPORTED | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED | TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        rail_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(rail_caps);

                    WindowListCaps window_list_caps = this->client_window_list_caps;
                    if (this->channels.remote_app.convert_remoteapp_to_desktop) {
                        window_list_caps.WndSupportLevel = 0x2;
                        window_list_caps.NumIconCaches = 3;
                        window_list_caps.NumIconCacheEntries = 12;
                    }
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        window_list_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(window_list_caps);
                }
#endif

                bool sendMultiFragmentUpdate = false;
                MultiFragmentUpdateCaps multi_fragment_update_caps;

                LargePointerCaps large_pointer_caps;
                bool sendLargePointer = false;

                if (this->large_pointer_support && this->client_large_pointer_caps.largePointerSupportFlags) {
                    sendLargePointer = true;

                    /* 2.2.7.2.7 Large Pointer Capability Set (TS_LARGE_POINTER_CAPABILITYSET)
                     * To support large pointer shapes, the client and server MUST support multifragment updates and
                     * indicate this support by exchanging the Multifragment Update Capability Set (section 2.2.7.2.6). The
                     * MaxRequestSize field of the Multifragment Update Capability Set MUST be set to at least 38,055
                     * bytes (so that a 96 x 96 pixel 32bpp pointer can be transported).
                     */
                    sendMultiFragmentUpdate = true;
                    if (multi_fragment_update_caps.MaxRequestSize < 38055) {
                        multi_fragment_update_caps.MaxRequestSize = 38055;
                    }
                }

                // if (this->primary_drawing_orders_support.test(TS_NEG_DRAWNINEGRID_INDEX)) {
                //     DrawNineGridCacheCaps ninegrid_caps(DRAW_NINEGRID_SUPPORTED, 0xffff, 256);
                //     confirm_active_pdu.emit_capability_set(ninegrid_caps);
                //     if (bool(this->verbose & RDPVerbose::capabilities)) {
                //         ninegrid_caps.log("Sending to server");
                //     }
                // }

                if (this->enable_remotefx && this->haveRemoteFx) {
                    /**
                     * for remoteFx we need:
                     *     * frameAck
                     *     * announced remoteFx codec
                     *     * appropriate surfaceCommands
                     *     * largePointer
                     *     * a multi_fragment_update that is big enough (at least the value returned by the server)
                     */
                    Emit_CS_BitmapCodecCaps bitmap_codec_caps;

                    if (this->front_bitmap_codec_caps.haveRemoteFxCodec) {
                        for (uint8_t i = 0 ; i < this->front_bitmap_codec_caps.bitmapCodecCount ; i++){
                            if (this->front_bitmap_codec_caps.bitmapCodecArray[i].codecType == CODEC_REMOTEFX){
                                auto id = this->front_bitmap_codec_caps.bitmapCodecArray[i].codecID;
                                auto codecGUID = this->front_bitmap_codec_caps.bitmapCodecArray[i].codecGUID;
                                // CODEC_GUID_IMAGE_REMOTEFX
                                if (memcmp(codecGUID, "\x12\x2F\x77\x76\x72\xBD\x63\x44\xAF\xB3\xB7\x3C\x9C\x6F\x78\x86", 16) == 0) {
                                    bitmap_codec_caps.addCodec(id, CODEC_GUID_IMAGE_REMOTEFX);
                                    this->remoteFx_codec_id.push_back(id);
                                } else
                                // CODEC_GUID_REMOTEFX
                                if(memcmp(codecGUID, "\xD4\xCC\x44\x27\x8A\x9D\x74\x4E\x80\x3C\x0E\xCB\xEE\xA1\x9C\x54", 16) == 0) {
                                    bitmap_codec_caps.addCodec(id, CODEC_GUID_REMOTEFX);
                                    this->remoteFx_codec_id.push_back(id);
                                } else {
                                    // CODEC_REMOTEFX: "\xA6\x51\x43\x9C\x35\x35\xAE\x42\x91\x0C\xCD\xFC\xE5\x76\x0B\x58"
                                    // NSCODEC: "\xB9\x1B\x8D\xCA\x0F\x00\x4F\x15\x58\x9F\xAE\x2D\x1A\x87\xE2\xD6"
                                }
                            }
                        }
                    }
                    else {
                        bitmap_codec_caps.addCodec(3, CODEC_GUID_REMOTEFX);
                        this->remoteFx_codec_id.push_back(3);
                        bitmap_codec_caps.addCodec(5, CODEC_GUID_IMAGE_REMOTEFX);
                        this->remoteFx_codec_id.push_back(5);
                    }

                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bitmap_codec_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(bitmap_codec_caps);

                    if (this->haveSurfaceFrameAck) {
                        FrameAcknowledgeCaps frameAck;
                        frameAck.maxUnacknowledgedFrameCount = 2;
                        if (bool(this->verbose & RDPVerbose::capabilities)) {
                            frameAck.log("Sending to server");
                        }
                        confirm_active_pdu.emit_capability_set(frameAck);
                    }

                    SurfaceCommandsCaps surface_commands_caps;
                    surface_commands_caps.cmdFlags = SURFCMDS_SETSURFACEBITS | SURFCMDS_FRAMEMARKER | SURFCMDS_STREAMSURFACEBITS;
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        surface_commands_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(surface_commands_caps);

                    multi_fragment_update_caps.MaxRequestSize = std::max(
                            this->front_multifragment_maxsize,
                            static_cast<uint32_t>(this->negociation_result.front_width * this->negociation_result.front_height * 4)
                    );
                    sendMultiFragmentUpdate = true;

                    large_pointer_caps.largePointerSupportFlags = LARGE_POINTER_FLAG_96x96;
                    sendLargePointer = true;
                }
                else if (sendLargePointer) {
                    large_pointer_caps = this->client_large_pointer_caps;
                }

                if (sendMultiFragmentUpdate && multi_fragment_update_caps.MaxRequestSize) {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        multi_fragment_update_caps.log("Sending to server");
                    }

                    confirm_active_pdu.emit_capability_set(multi_fragment_update_caps);
                }

                if (sendLargePointer) {
                    // Not yet supported
                    large_pointer_caps.largePointerSupportFlags &= ~LARGE_POINTER_FLAG_384x384;

                    confirm_active_pdu.emit_capability_set(large_pointer_caps);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        large_pointer_caps.log("Sending to server");
                    }
                }
                if (this->multifragment_update_data.get_capacity() < multi_fragment_update_caps.MaxRequestSize) {
                    this->multifragment_update_buffer = std::make_unique<uint8_t[]>(multi_fragment_update_caps.MaxRequestSize);
                    this->multifragment_update_data = OutStream({this->multifragment_update_buffer.get(), multi_fragment_update_caps.MaxRequestSize});
                }

                /** OffscreenBitmapCacheCapabilitySet */
                OffScreenCacheCaps offscreen_cache_caps;
                // offscreen_cache_caps.offscreenSupportLevel = true;
                // offscreen_cache_caps.offscreenCacheSize = 0x1E00;
                // offscreen_cache_caps.offscreenCacheEntries = 0x07D0;

                confirm_active_pdu.emit_capability_set(offscreen_cache_caps);
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    offscreen_cache_caps.log("Sending to server");
                }

                confirm_active_pdu.emit_end();
            },
            [this](StreamSize<256> /*maxlen*/, OutStream & sctrl_header, std::size_t packet_size) {
                // shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
                // containing information about the packet. The type subfield of the pduType
                // field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).
                ShareControl_Send(sctrl_header, PDUTYPE_CONFIRMACTIVEPDU,
                    this->negociation_result.userid + GCC::MCS_USERCHANNEL_BASE, packet_size);
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
        LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
            LOG_INFO, "mod_rdp::process_pointer_pdu");

        int message_type = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */
        switch (message_type) {
        // Cached Pointer Update (section 2.2.9.1.1.4.6)
        case RDP_POINTER_CACHED:
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer cached");
            this->process_cached_pointer_pdu(stream, drawable);
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer cached done");
            break;
        // Color Pointer Update (section 2.2.9.1.1.4.4)
        case RDP_POINTER_COLOR:
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer color");
            this->process_new_pointer_pdu(BitsPerPixel{24}, stream, drawable);
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
                LOG_INFO, "Process pointer color done");
            break;
        // New Pointer Update (section 2.2.9.1.1.4.5)
        case RDP_POINTER_NEW:
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer new");
            if (this->enable_new_pointer) {
                BitsPerPixel data_bpp = checked_int{stream.in_uint16_le()}; /* data bpp */
                this->process_new_pointer_pdu(data_bpp, stream, drawable);
            }
            else {
                LOG(LOG_ERR, "mod_rdp::process_pointer_pdu: New Pointer Updated is disabled!");
            }
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer new done");
            break;
        // 2.2.9.1.1.4.3 System Pointer Update (TS_SYSTEMPOINTERATTRIBUTE)
        // ---------------------------------------------------------------

        // systemPointerType (4 bytes): A 32-bit, unsigned integer.
        //    The type of system pointer.

        // +---------------------------+-----------------------------+
        // |      Value                |      Meaning                |
        // +---------------------------+-----------------------------+
        // | SYSPTR_NULL    0x00000000 | The hidden pointer.         |
        // +---------------------------+-----------------------------+
        // | SYSPTR_DEFAULT 0x00007F00 | The default system pointer. |
        // +---------------------------+-----------------------------+

        case RDP_POINTER_SYSTEM:
        {
            int system_pointer_type = stream.in_uint32_le();
            LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
                LOG_INFO, "Process pointer system::RDP_%s_POINTER",
                (system_pointer_type == RDP_NULL_POINTER) ? "NULL" : "DEFAULT");

            if (system_pointer_type == RDP_NULL_POINTER) {
                drawable.set_pointer(0, null_pointer(), gdi::GraphicApi::SetPointerMode::Insert);
            }
            else {
                drawable.set_pointer(0, normal_pointer(), gdi::GraphicApi::SetPointerMode::Insert);
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
                LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "Process pointer position");

                //  xPos(2) + yPos(2)
                // TODO: see why there was break instead of throw here
                ::check_throw(stream, 6, "mod_rdp::Pointer Position Update", ERR_RDP_DATA_TRUNCATED);

                uint16_t xPos = stream.in_uint16_le();
                uint16_t yPos = stream.in_uint16_le();
                this->front.update_pointer_position(xPos, yPos);
            }
            break;
        default:
            break;
        }
        LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
            LOG_INFO, "mod_rdp::process_pointer_pdu done");
    }

    void process_palette(gdi::GraphicApi & drawable, InStream & stream, bool fast_path) {
        LOG_IF(bool(this->verbose & RDPVerbose::graphics), LOG_INFO, "mod_rdp::process_palette");

        RDP::UpdatePaletteData_Recv(stream, fast_path, this->orders.global_palette);
        drawable.set_palette(this->orders.global_palette);

        LOG_IF(bool(this->verbose & RDPVerbose::graphics), LOG_INFO, "mod_rdp::process_palette done");
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

    static const char* get_error_info_name(uint32_t errorInfo) {
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
            #undef CASE
            default:
                return "?";
        }
    }   // get_error_info_name

    static uint32_t get_error_info_from_pdu(InStream & stream) {
        return stream.in_uint32_le();
    }

    void process_error_info(uint32_t errorInfo) {
        const char* errorInfo_name = get_error_info_name(errorInfo);
        LOG(LOG_INFO, "process error info pdu: code=0x%08X error=%s", errorInfo, errorInfo_name);

        if (errorInfo) {
            str_append(this->close_box_extra_message_ref, " (", errorInfo_name, ')');
        }

        switch (errorInfo){
        case ERRINFO_DISCONNECTED_BY_OTHERCONNECTION:
            this->vars.set<cfg::context::auth_error_message>(
                TR(trkeys::disconnected_by_otherconnection, this->lang));
            break;
        case ERRINFO_REMOTEAPPSNOTENABLED:
            this->remote_apps_not_enabled = true;
            break;
        }
    }   // process_error_info

    void process_logon_info(const char * domain, const char * username, uint32_t native_session_id)
    {
        this->vars.set_acl<cfg::context::native_session_id>(native_session_id);

        char domain_username_format_0[2048];
        char domain_username_format_1[2048];

        snprintf(domain_username_format_0, sizeof(domain_username_format_0),
            "%s@%s", username, domain);
        snprintf(domain_username_format_1, sizeof(domain_username_format_0),
            "%s\\%s", domain, username);

#ifndef __EMSCRIPTEN__
        if (this->channels.file_system.smartcard_passthrough) {
            this->vars.set_acl<cfg::context::smartcard_login>(domain_username_format_0);
        }
#endif

        if (this->disconnect_on_logon_user_change
            && ((0 != ::strcasecmp(domain, this->logon_info.domain().c_str())
             || 0 != ::strcasecmp(username, this->logon_info.username().c_str()))
            && (this->logon_info.domain().c_str()
             || (0 != ::strcasecmp(domain_username_format_0, this->logon_info.username().c_str())
              && 0 != ::strcasecmp(domain_username_format_1, this->logon_info.username().c_str())
              && 0 != ::strcasecmp(username, this->logon_info.username().c_str()))))
        ) {
            if (this->error_message) {
                *this->error_message = "Unauthorized logon user change detected!";
            }

            this->connection_finalization_state = DISCONNECTED;

            this->session_log.report("OPEN_SESSION_FAILED", "Unauthorized logon user change detected.");

            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s%s%s) -> (%s%s%s). "
                    "The session will be disconnected.",
                this->logon_info.hostname(), this->logon_info.domain().c_str(),
                (*this->logon_info.domain().c_str() ? "\\" : ""),
                this->logon_info.username().c_str(), domain,
                ((domain && *domain) ? "\\" : ""),
                username);
            throw Error(ERR_RDP_LOGON_USER_CHANGED);
        }

#ifndef __EMSCRIPTEN__
        if (this->channels.session_probe_virtual_channel &&
            this->session_probe_start_launch_timeout_timer_only_after_logon) {
            this->channels.session_probe_virtual_channel->start_launch_timeout_timer();
        }
#endif

        this->session_log.report("OPEN_SESSION_SUCCESSFUL", "OK.");

//        this->fd_event->disable_timeout();

#ifndef __EMSCRIPTEN__
        if (this->channels.session_probe.enable_session_probe &&
            (!this->channels.session_probe_virtual_channel || !this->channels.session_probe_virtual_channel->has_been_launched())) {
            this->disable_input_event();
            if (this->channels.session_probe.enable_launch_mask){
                this->disable_graphics_update();
            }
            else {
                this->enable_graphics_update();
            }
        }
#endif
    }   // process_logon_info

    void process_save_session_info(InStream & stream) {
        RDP::SaveSessionInfoPDUData_Recv ssipdudata(stream);

        switch (ssipdudata.infoType) {
        case RDP::INFOTYPE_LOGON:
        {
            LOG(LOG_INFO, "process save session info : Logon");
            RDP::LogonInfoVersion1_Recv liv1(ssipdudata.payload);

            this->process_logon_info(char_ptr_cast(liv1.Domain), char_ptr_cast(liv1.UserName), liv1.SessionId);

            this->front.send_savesessioninfo();

#ifndef __EMSCRIPTEN__
            this->remoteapp_one_shot_bypass_window_legalnotice.garbage();
#endif
        }
        break;
        case RDP::INFOTYPE_LOGON_LONG:
        {
            LOG(LOG_INFO, "process save session info : Logon long");
            RDP::LogonInfoVersion2_Recv liv2(ssipdudata.payload);

            this->process_logon_info(char_ptr_cast(liv2.Domain), char_ptr_cast(liv2.UserName), liv2.SessionId);

            this->front.send_savesessioninfo();

#ifndef __EMSCRIPTEN__
            this->remoteapp_one_shot_bypass_window_legalnotice.garbage();
#endif
        }
        break;
        case RDP::INFOTYPE_LOGON_PLAINNOTIFY:
        {
            LOG(LOG_INFO, "process save session info : Logon plainnotify");
            RDP::PlainNotify_Recv pn(ssipdudata.payload);

#ifndef __EMSCRIPTEN__
            if (this->channels.session_probe.enable_session_probe &&
                (!this->channels.session_probe_virtual_channel || !this->channels.session_probe_virtual_channel->has_been_launched())) {
                this->disable_input_event();
                if (this->channels.session_probe.enable_launch_mask){
                    this->disable_graphics_update();
                }
                else {
                    this->enable_graphics_update();
                }
            }

            if (this->channels.session_probe_virtual_channel &&
                this->session_probe_start_launch_timeout_timer_only_after_logon) {
                this->channels.session_probe_virtual_channel->start_launch_timeout_timer();
            }
#endif
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

                OutStream stream(this->server_auto_reconnect_packet_ref);

                auto_reconnect.emit(stream);

                this->is_server_auto_reconnec_packet_received = true;

#ifndef __EMSCRIPTEN__
            this->remoteapp_one_shot_bypass_window_legalnotice.garbage();
#endif
            }

            if (lie.FieldsPresent & RDP::LOGON_EX_LOGONERRORS) {
                LOG(LOG_INFO, "process save session info : Logon Errors Info");

                RDP::LogonErrorsInfo_Recv lei(lif.payload);

#ifndef __EMSCRIPTEN__
                if ((RDP::LOGON_MSG_SESSION_CONTINUE != lei.ErrorNotificationType) &&
                    (RDP::LOGON_WARNING >= lei.ErrorNotificationData) &&
                    this->channels.remote_app.enable_remote_program
                ) {
                    if ((0 != lei.ErrorNotificationType) ||
                        (RDP::LOGON_FAILED_OTHER != lei.ErrorNotificationData) ||
                        (!this->channels.remote_app.bypass_legal_notice_delay.count())
                    ) {
                        this->on_remoteapp_redirect_user_screen(lei.ErrorNotificationData);
                    }
                    else {
                        this->remoteapp_one_shot_bypass_window_legalnotice = this->events_guard.create_event_timeout(
                            "Bypass Legal Notice Timer",
                            this->channels.remote_app.bypass_legal_notice_delay,
                            [this, failed = false](Event&event) mutable
                            {
                                if (!failed) {
                                    LOG(LOG_INFO, "RDP::process_save_session_info: One-shot bypass Windows's Legal Notice");
                                    this->send_input(0, RDP_INPUT_SCANCODE, 0x0, 0x1C, 0x0);
                                    this->send_input(0, RDP_INPUT_SCANCODE, 0x8000, 0x1C, 0x0);

                                    if (this->channels.remote_app.bypass_legal_notice_timeout.count()) {
                                        event.alarm.reset_timeout(
                                            this->events_guard.get_monotonic_time()
                                            + this->channels.remote_app.bypass_legal_notice_timeout);
                                        failed = true;
                                    }
                                    else {
                                        event.garbage = true;
                                    }
                                }
                                else {
                                    event.garbage = true;
                                    this->on_remoteapp_redirect_user_screen(RDP::LOGON_FAILED_OTHER);
                                }
                            });
                    }
                }
                else if (RDP::LOGON_MSG_SESSION_CONTINUE == lei.ErrorNotificationType) {
                    this->remoteapp_one_shot_bypass_window_legalnotice.garbage();
                }
#endif
            }
        }
        break;
        }

        stream.in_skip_bytes(stream.in_remain());
    }

    [[noreturn]]
    void on_remoteapp_redirect_user_screen(uint32_t ErrorNotificationData) {
        LOG(LOG_ERR, "Can not redirect user's focus to the WinLogon screen in RemoteApp mode!");
        std::string errmsg = str_concat(
            "(RemoteApp) ",
            RDP::LogonErrorsInfo_Recv::ErrorNotificationDataToShortMessage(ErrorNotificationData));
        this->vars.set<cfg::context::auth_error_message>(errmsg);
        throw Error(ERR_RAIL_LOGON_FAILED_OR_WARNING);
    }

    // Note: this template is used for Caps we are merely using but doing nothing with
    template<typename CAPS> CAPS receive_caps(InStream & stream, uint16_t capset_length)
    {
        CAPS caps;
        caps.recv(stream, capset_length);
        if (bool(this->verbose & RDPVerbose::capabilities)) {
            caps.log("Receiving from server");
        }
        return caps;
    }

// 2.2.1.13.1.1 Demand Active PDU Data (TS_DEMAND_ACTIVE_PDU)
// ==========================================================

//    shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
// containing information about the packet. The type subfield of the pduType
// field of the Share Control Header MUST be set to PDUTYPE_DEMANDACTIVEPDU (1).

//    shareId (4 bytes): A 32-bit, unsigned integer. The share identifier for
// the packet (see [T128] section 8.4.2 for more information regarding share
// IDs).

//    lengthSourceDescriptor (2 bytes): A 16-bit, unsigned integer. The size
//  in bytes of the sourceDescriptor field.

//    lengthCombinedCapabilities (2 bytes): A 16-bit, unsigned integer.
// The combined size in bytes of the numberCapabilities, pad2Octets, and
// capabilitySets fields.

//    sourceDescriptor (variable): A variable-length array of bytes containing
// a source descriptor (see [T128] section 8.4.1 for more information regarding
// source descriptors).

// numberCapabilities (2 bytes): A 16-bit, unsigned integer. The number of capability sets included in the
// Demand Active PDU.

// pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// capabilitySets (variable): An array of Capability Set (section 2.2.1.13.1.1.1) structures. The number
//  of capability sets is specified by the numberCapabilities field.

// sessionId (4 bytes): A 32-bit, unsigned integer. The session identifier. This field is ignored by the client.

    void process_demand_active(ShareControl_Recv & sctrl)
    {
        LOG(LOG_INFO, "mod_rdp::recv_demand_active");
        this->share_id = sctrl.payload.in_uint32_le();

        uint16_t lengthSourceDescriptor = sctrl.payload.in_uint16_le();
        uint16_t lengthCombinedCapabilities = sctrl.payload.in_uint16_le();

        // TODO before skipping we should check we do not go outside current stream
        sctrl.payload.in_skip_bytes(lengthSourceDescriptor);

        // Read number of capabilities and capabilities content
        this->process_server_caps(sctrl.payload, lengthCombinedCapabilities);

        uint32_t sessionId = sctrl.payload.in_uint32_le();
        (void)sessionId;
    }

    void process_server_caps(InStream & stream, uint16_t len) {
        // TODO check stream consumed and len
        (void)len;
        LOG_IF(bool(this->verbose & RDPVerbose::capabilities), LOG_INFO, "mod_rdp::process_server_caps");

        uint16_t ncapsets = stream.in_uint16_le();
        stream.in_skip_bytes(2); /* pad */

        for (uint16_t n = 0; n < ncapsets; n++) {
            //  capabilitySetType(2) + lengthCapability(2)
            ::check_throw(stream, 6, "mod_rdp::Demand active PDU (1)", ERR_RDP_DATA_TRUNCATED);
            uint16_t capset_type = stream.in_uint16_le();
            uint16_t capset_length = stream.in_uint16_le();

            ::check_throw(stream, capset_length - 4, "mod_rdp::Demand active PDU (2)", ERR_RDP_DATA_TRUNCATED);
            uint8_t const * next = stream.get_current() + capset_length - 4;

//            InStream substream(stream.get_current(), capset_length - 4);
            switch (capset_type) {
            case CAPSTYPE_GENERAL:
                this->receive_caps<GeneralCaps>(stream, capset_length);
                break;
            case CAPSTYPE_BITMAP:
            {
                auto bitmap_caps = this->receive_caps<BitmapCaps>(stream, capset_length);
                this->orders.set_bpp(checked_int(bitmap_caps.preferredBitsPerPixel));
                this->negociation_result.front_width = bitmap_caps.desktopWidth;
                this->negociation_result.front_height = bitmap_caps.desktopHeight;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    LOG(LOG_INFO, "Bitmap Depth from Server: %d", bitmap_caps.preferredBitsPerPixel);
                }
            }
            break;
            case CAPSTYPE_ORDER:
                this->receive_caps<OrderCaps>(stream, capset_length);
                break;
            case CAPSTYPE_INPUT:
            {
                auto input_caps = this->receive_caps<InputCaps>(stream, capset_length);
                this->enable_fastpath_client_input_event =
                    (this->enable_fastpath && ((input_caps.inputFlags & (INPUT_FLAG_FASTPATH_INPUT | INPUT_FLAG_FASTPATH_INPUT2)) != 0));
                if (this->server_info_ref) {
                    this->server_info_ref->input_flags = input_caps.inputFlags;
                    this->server_info_ref->keyboard_layout = input_caps.keyboardLayout;
                }
            }
            break;
            case CAPSTYPE_RAIL:
                this->receive_caps<RailCaps>(stream, capset_length);
                break;
            case CAPSTYPE_WINDOW:
                this->receive_caps<WindowListCaps>(stream, capset_length);
                break;
            case CAPSTYPE_POINTER:
                this->receive_caps<PointerCaps>(stream, capset_length);
                break;
            case CAPSETTYPE_MULTIFRAGMENTUPDATE:
            {
                auto multi_fragment_update_caps = this->receive_caps<MultiFragmentUpdateCaps>(stream, capset_length);
                this->front_multifragment_maxsize = multi_fragment_update_caps.MaxRequestSize;
            }
            break;
            case CAPSETTYPE_LARGE_POINTER:
                this->receive_caps<LargePointerCaps>(stream, capset_length);
                break;
            case CAPSTYPE_SOUND:
                this->receive_caps<SoundCaps>(stream, capset_length);
                break;
            case CAPSTYPE_FONT:
                this->receive_caps<FontCaps>(stream, capset_length);
                break;
            case CAPSTYPE_ACTIVATION:
                this->receive_caps<ActivationCaps>(stream, capset_length);
                break;
            case CAPSTYPE_VIRTUALCHANNEL:
                this->receive_caps<VirtualChannelCaps>(stream, capset_length);
                break;
            case CAPSTYPE_DRAWGDIPLUS:
                this->receive_caps<DrawGdiPlusCaps>(stream, capset_length);
                break;
            case CAPSTYPE_COLORCACHE:
                this->receive_caps<ColorCacheCaps>(stream, capset_length);
                break;
            case CAPSTYPE_BITMAPCACHE_HOSTSUPPORT:
                this->receive_caps<BitmapCacheHostSupportCaps>(stream, capset_length);
                break;
            case CAPSTYPE_SHARE:
                this->receive_caps<ShareCaps>(stream, capset_length);
                break;
            case CAPSETTYPE_COMPDESK:
                this->receive_caps<CompDeskCaps>(stream, capset_length);
                break;
            case CAPSETTYPE_SURFACE_COMMANDS:
                this->receive_caps<SurfaceCommandsCaps>(stream, capset_length);
                break;
            case CAPSETTYPE_BITMAP_CODECS:
            {
                Recv_SC_BitmapCodecCaps bitmap_codecs_caps;
                bitmap_codecs_caps.recv(stream, capset_length);
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    bitmap_codecs_caps.log("Receiving from server");
                }
                this->haveRemoteFx = bitmap_codecs_caps.haveRemoteFxCodec;
                if (this->haveRemoteFx){
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        LOG(LOG_INFO, "RemoteFx Enabled on server");
                    }
                }
            }
            break;
            case CAPSETTYPE_FRAME_ACKNOWLEDGE:
                this->receive_caps<FrameAcknowledgeCaps>(stream, capset_length);
                this->haveSurfaceFrameAck = true;
                break;
            default:
                LOG_IF(bool(this->verbose & RDPVerbose::capabilities), LOG_WARNING,
                    "Unprocessed Capability Set is encountered. capabilitySetType=%s(%u)",
                    ::get_capabilitySetType_name(capset_type), capset_type);
                break;
            }
            stream.in_skip_bytes(next - stream.get_current());
        }

        LOG_IF(bool(this->verbose & RDPVerbose::capabilities), LOG_INFO, "mod_rdp::process_server_caps done");
    }   // process_server_caps

    void send_control(int action) {
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "mod_rdp::send_control");

        this->send_data_request_ex(
            GCC::MCS_GLOBAL_CHANNEL,
            [this, action](StreamSize<256> /*maxlen*/, OutStream & stream) {
                ShareData sdata(stream);
                sdata.emit_begin(PDUTYPE2_CONTROL, this->share_id, RDP::STREAM_MED);

                // Payload
                stream.out_uint16_le(action);
                stream.out_uint16_le(0); /* userid */
                stream.out_uint32_le(0); /* control id */

                // Packet trailer
                sdata.emit_end();
            },
            [this](StreamSize<256> /*maxlen*/, OutStream & sctrl_header, std::size_t packet_size) {
                ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->negociation_result.userid + GCC::MCS_USERCHANNEL_BASE, packet_size);

            }
        );

        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "mod_rdp::send_control done");
    }

    void send_persistent_key_list() {
#ifndef __EMSCRIPTEN__
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "mod_rdp::send_persistent_key_list");

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
            }
        }
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

                        // Send persistent bitmap cache enumeration PDU's not implemented
                        // yet because it should be implemented before in process_data case.
                        // The problem is that we don't save the bitmap key list attached
                        // with rdp_bmpcache2 capability message so we can't develop this
                        // function yet
                        this->send_pdu_type2(
                            PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST, RDP::STREAM_MED,
                            [&pklpdu](StreamSize<2048> /*maxlen*/, OutStream & pdu_data_stream) {
                                pklpdu.emit(pdu_data_stream);
                            }
                        );

                        pklpdu.reset();

                        pdu_number_of_entries = 0;
                    }
                }
            }
        }

        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "mod_rdp::send_persistent_key_list done");
#endif
    }   // send_persistent_key_list

    // TODO CGR: duplicated code in front
    void send_synchronise()
    {
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "mod_rdp::send_synchronise");

        this->send_pdu_type2(
            PDUTYPE2_SYNCHRONIZE, RDP::STREAM_MED,
            [](StreamSize<4> /*maxlen*/, OutStream & stream) {
                stream.out_uint16_le(1); /* type */
                stream.out_uint16_le(1002);
            }
        );

        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "mod_rdp::send_synchronise done");
    }

    void send_fonts(int seq)
    {
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "mod_rdp::send_fonts");

        this->send_pdu_type2(
            PDUTYPE2_FONTLIST, RDP::STREAM_MED,
            [seq](StreamSize<8> /*maxlen*/, OutStream & stream){
                // Payload
                stream.out_uint16_le(0); /* number of fonts */
                stream.out_uint16_le(0); /* pad? */
                stream.out_uint16_le(seq); /* unknown */
                stream.out_uint16_le(0x32); /* entry size */
            }
        );

        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace), LOG_INFO, "mod_rdp::send_fonts done");
    }

    void send_input(int time, int message_type, int device_flags, int param1, int param2) override
    {
        [[maybe_unused]] std::size_t channel_data_size = this->enable_fastpath_client_input_event
            ? this->send_input_fastpath(time, message_type, device_flags, param1, param2)
            : this->send_input_slowpath(time, message_type, device_flags, param1, param2);

        if (message_type == RDP_INPUT_SYNCHRONIZE) {
            this->last_key_flags_sent = param1;
        }
    }

    void rdp_gdi_up_and_running() override {}
    void rdp_gdi_down() override {}

    void rdp_input_invalidate(Rect r) override {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
                "mod_rdp::rdp_input_invalidate");

            if (!r.isempty()){
                RDP::RefreshRectPDU rrpdu(
                    this->share_id,
                    this->negociation_result.userid,
                    this->negociation_result.encryptionLevel,
                    this->encrypt);

                rrpdu.addInclusiveRect(r.x, r.y, r.x + r.cx - 1, r.y + r.cy - 1);

                rrpdu.emit(this->trans);
            }

            LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
                "mod_rdp::rdp_input_invalidate done");
        }
    }

    void rdp_input_invalidate2(array_view<Rect> vr) override {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
                "mod_rdp::rdp_input_invalidate 2");

            if (!vr.empty()) {
                RDP::RefreshRectPDU rrpdu(
                    this->share_id,
                    this->negociation_result.userid,
                    this->negociation_result.encryptionLevel,
                    this->encrypt);

                for (Rect const & rect : vr) {
                    if (!rect.isempty()){
                        rrpdu.addInclusiveRect(rect.x, rect.y, rect.x + rect.cx - 1, rect.y + rect.cy - 1);
                    }
                }

                rrpdu.emit(this->trans);
            }

            LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
                "mod_rdp::rdp_input_invalidate 2 done");
        }
    }

    void rdp_allow_display_updates(uint16_t left, uint16_t top,
            uint16_t right, uint16_t bottom) override {
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "mod_rdp::rdp_allow_display_updates");

        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_pdu_type2(
                PDUTYPE2_SUPPRESS_OUTPUT, RDP::STREAM_MED,
                [left, top, right, bottom](StreamSize<32> /*maxlen*/, OutStream & stream) {
                    RDP::SuppressOutputPDUData sopdud(left, top, right, bottom);

                    sopdud.emit(stream);
                }
            );
        }

        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "mod_rdp::rdp_allow_display_updates done");
    }

    void rdp_suppress_display_updates() override {
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "mod_rdp::rdp_suppress_display_updates");

        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_pdu_type2(
                PDUTYPE2_SUPPRESS_OUTPUT, RDP::STREAM_MED,
                [](StreamSize<32> /*maxlen*/, OutStream & stream) {
                    RDP::SuppressOutputPDUData sopdud;

                    sopdud.emit(stream);
                }
            );
        }

        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "mod_rdp::rdp_suppress_display_updates done");
    }

    void refresh(Rect r) override {
        this->rdp_input_invalidate(r);
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
        LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
            LOG_INFO, "mod_rdp::process_cached_pointer_pdu");

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
            drawable.set_pointer(pointer_idx, cursor, gdi::GraphicApi::SetPointerMode::Cached);
        }
        else {
            LOG(LOG_WARNING,  "mod_rdp::process_cached_pointer_pdu: invalid cache cell index, use system default. index=%u",
                pointer_idx);
        }

        LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
            LOG_INFO, "mod_rdp::process_cached_pointer_pdu done");
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

    void process_new_pointer_pdu(BitsPerPixel data_bpp, InStream & stream, gdi::GraphicApi & drawable) {
        LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer), LOG_INFO, "mod_rdp::process_new_pointer_pdu");

        //::hexdump_d(stream.get_current(), stream.in_remain());

        unsigned pointer_idx = stream.in_uint16_le();
        LOG_IF(bool(this->verbose & RDPVerbose::graphics_pointer),
            LOG_INFO, "mod_rdp::process_new_pointer_pdu xorBpp=%u pointer_idx=%u",
            data_bpp, pointer_idx);

        if (pointer_idx >= (sizeof(this->cursors) / sizeof(this->cursors[0]))) {
            LOG(LOG_ERR,
                "mod_rdp::process_new_pointer_pdu pointer cache idx overflow (%u)",
                pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }

        Pointer& cursor = this->cursors[pointer_idx];
        cursor = pointer_loader_new(data_bpp, stream, this->orders.global_palette, this->clean_up_32_bpp_cursor, this->use_native_pointer);
        drawable.set_pointer(pointer_idx, cursor, gdi::GraphicApi::SetPointerMode::New);
    }   // process_new_pointer_pdu

private:
    void process_bitmap_updates(InStream & stream, bool fast_path, gdi::GraphicApi & drawable) {
        LOG_IF(bool(this->verbose & RDPVerbose::graphics),
            LOG_INFO, "mod_rdp::process_bitmap_updates");

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
//         // The TS_UPDATE_BITMAP_DATA structure encapsulates the bitmap data that
        // defines a Bitmap Update (section 2.2.9.1.1.3.1.2).

        // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update
        // type. This field MUST be set to UPDATETYPE_BITMAP (0x0001).

        // numberRectangles (2 bytes): A 16-bit, unsigned integer.
        // The number of screen rectangles present in the rectangles field.
        size_t numberRectangles = stream.in_uint16_le();
        LOG_IF(bool(this->verbose & RDPVerbose::graphics),
            LOG_INFO, "/* ---------------- Sending %zu rectangles ----------------- */",
            numberRectangles);

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

            // TODO : verify code below, why is there no check at all on BITMAP_COMPRESSION_NO_HDR flag ?
            // CGR: both flags seems to be redundant. Was there an old version of RDP
            // where compression header was present but compresion not enabled ?
            // That's the only potential use I see for this flag

            // BITMAP_COMPRESSION 0x0001
            // Indicates that the bitmap data is compressed. This implies
            // that the bitmapComprHdr field is present if the
            // NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not set.

            LOG_IF(bool(this->verbose & RDPVerbose::graphics), LOG_INFO,
                "/* Rect [%zu] bpp=%" PRIu16 " width=%" PRIu16 " height=%" PRIu16
                " b(%" PRId16 ", %" PRId16 ", %" PRIu16 ", %" PRIu16 ") */",
                i, bmpdata.bits_per_pixel, bmpdata.width, bmpdata.height,
                boundary.x, boundary.y, boundary.cx, boundary.cy);

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
            Bitmap bitmap( this->orders.get_bpp()
                         , checked_int(bmpdata.bits_per_pixel)
                         , &this->orders.global_palette
                         , bmpdata.width
                         , bmpdata.height
                         , data
                         , bmpdata.bitmap_size()
                         , (bmpdata.flags & BITMAP_COMPRESSION)
                         );

            if (   bmpdata.cb_scan_width
                   && ((bmpdata.cb_scan_width - bitmap.line_size()) >= nb_bytes_per_pixel(bitmap.bpp()))) {
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
        LOG_IF(bool(this->verbose & RDPVerbose::graphics),
            LOG_INFO, "mod_rdp::process_bitmap_updates done");
    }   // process_bitmap_updates

public:
    [[nodiscard]] bool is_up_and_running() const override {
        return (UP_AND_RUNNING == this->connection_finalization_state);
    }

    void disconnect() override {
        if (this->is_up_and_running()) {
            LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
                LOG_INFO, "mod_rdp::disconnect()");
            this->session_log.report("CLOSE_SESSION_SUCCESSFUL", "OK.");
            // this->send_shutdown_request();
            // this->draw_event(time(nullptr));
            this->send_disconnect_ultimatum();
        }

        this->log_disconnection(false);
    }

private:
    void log_disconnection(bool enable_verbose)
    {
        if (this->session_time_start.count()) {
            auto delay = this->events_guard.get_monotonic_time().time_since_epoch()
                        - this->session_time_start;
            long seconds = std::chrono::duration_cast<std::chrono::seconds>(delay).count();
            this->session_time_start = MonotonicTimePoint::duration(0);

            char duration_str[128];
            int len = snprintf(duration_str, sizeof(duration_str), "%02ld:%02ld:%02ld",
                seconds / 3600, (seconds % 3600) / 60, seconds % 60);

            // force kbd flush before disconnect event
            this->front.possible_active_window_change();

            this->session_log.log6(LogId::SESSION_DISCONNECTION,
                {KVLog("duration"_av, {duration_str, std::size_t(len)}),});

            LOG_IF(enable_verbose, LOG_INFO,
                "type=SESSION_DISCONNECTION duration=%s", duration_str);
        }
    }

    void send_disconnect_ultimatum() {
        LOG_IF(bool(this->verbose & RDPVerbose::basic_trace),
            LOG_INFO, "SEND MCS DISCONNECT PROVIDER ULTIMATUM PDU");

        if (!this->mcs_disconnect_provider_ultimatum_pdu_received &&
            !this->errinfo_graphics_subsystem_failed_encountered) {
            this->connection_finalization_state = DISCONNECTED;
            write_packets(
                this->trans,
                [](StreamSize<256> /*maxlen*/, OutStream & mcs_data) {
                    MCS::DisconnectProviderUltimatum_Send(mcs_data, 3, MCS::PER_ENCODING);
                },
                X224::write_x224_dt_tpdu_fn{}
            );
        }
    }

public:
    void disable_input_event()
    {
        LOG(LOG_INFO, "Mod_rdp: disable input event.");
        this->input_event_disabled = true;
    }

    void enable_input_event()
    {
        LOG(LOG_INFO, "Mod_rdp: enable input event.");
        this->input_event_disabled = false;
    }

    void disable_graphics_update()
    {
        LOG(LOG_INFO, "Mod_rdp: Disable graphics update.");
        this->graphics_update_disabled = true;

#ifndef __EMSCRIPTEN__
        if (this->channels.remote_programs_session_manager) {
            this->channels.remote_programs_session_manager->disable_graphics_update(true);
        }
#endif
    }

    void enable_graphics_update()
    {
        LOG(LOG_INFO, "Mod_rdp: Enable graphics update.");
        bool need_full_screen_update = this->graphics_update_disabled;
        this->graphics_update_disabled = false;

#ifndef __EMSCRIPTEN__
        if (this->channels.remote_programs_session_manager) {
            this->channels.remote_programs_session_manager->disable_graphics_update(false);
        }
#endif
        if (need_full_screen_update) {
            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                "SessionProbeVirtualChannel::process_event: "
                    "Force full screen update. Rect=(0, 0, %u, %u)",
                this->negociation_result.front_width, this->negociation_result.front_height);
                if (this->bogus_refresh_rect && this->monitor_count) {
                    this->rdp_suppress_display_updates();
                    this->rdp_allow_display_updates(0, 0,
                        this->negociation_result.front_width, this->negociation_result.front_height);
                }
            this->rdp_input_invalidate(Rect(0, 0,
                this->negociation_result.front_width, this->negociation_result.front_height));
        }
    }

    void freeze_screen()
    {
        Rect const r(0, 0, this->negociation_result.front_width, this->negociation_result.front_height);
        RDPPatBlt cmd(r, 0xA0, color_encode(BLACK, BitsPerPixel{24}), color_encode(WHITE, BitsPerPixel{24}),
            RDPBrush(0, 0, 3, 0xaa, byte_ptr("\x55\xaa\x55\xaa\x55\xaa\x55"))
        );
        this->gd.begin_update();
        this->gd.draw(cmd, r, gdi::ColorCtx::depth24());
        this->gd.end_update();
    }

public:
#ifndef __EMSCRIPTEN__
    void DLP_antivirus_check_channels_files()
    {
        this->channels.DLP_antivirus_check_channels_files();
    }
#endif

    [[nodiscard]] windowing_api* get_windowing_api() const {
#ifndef __EMSCRIPTEN__
        if (this->channels.remote_programs_session_manager && !this->channels.remote_app.convert_remoteapp_to_desktop) {
            return this->channels.remote_programs_session_manager.get();
        }
#endif

        return nullptr;
    }

    [[nodiscard]] Dimension get_dim() const override
    { return Dimension(this->negociation_result.front_width, this->negociation_result.front_height); }

    bool is_auto_reconnectable() const override {
        return this->is_server_auto_reconnec_packet_received
            && this->is_up_and_running()
            && !this->mcs_disconnect_provider_ultimatum_pdu_received
#ifndef __EMSCRIPTEN__
            && (!this->channels.session_probe.session_probe_launcher || this->channels.session_probe.session_probe_launcher->is_stopped())
#endif
            ;
    }

    bool server_error_encountered() const override
    {
        return this->errinfo_encountered;
    }

    void reset_keyboard_status() override
    {
        LOG(LOG_INFO, "mod_rdp::reset_keyboard_status()");

        this->rdp_input_scancode(0xF, 0, 0x8000, 0, nullptr);
        this->rdp_input_synchronize(0, 0, this->status_of_keyboard_toggle_keys, 0);
        this->rdp_input_scancode(0xF, 0, 0x8000, 0, nullptr);

        this->rdp_input_scancode(0x2A, 0, 0x0000, 0, nullptr);
        this->rdp_input_scancode(0x2A, 0, 0x8000, 0, nullptr);

        this->rdp_input_scancode(0x1D, 0, 0x0000, 0, nullptr);
        this->rdp_input_scancode(0x1D, 0, 0x8000, 0, nullptr);

        this->rdp_input_scancode(0x38, 0, 0x0000, 0, nullptr);
        this->rdp_input_scancode(0x38, 0, 0x8000, 0, nullptr);

        this->rdp_input_scancode(0x5B, 0, 0x8100, 0, nullptr);
    }

    void auth_rail_exec(uint16_t flags, const char* original_exe_or_file,
            const char* exe_or_file, const char* working_dir,
            const char* arguments, const char* account, const char* password) override {
#ifndef __EMSCRIPTEN__
        ServerTransportContext stc{
            this->trans, this->encrypt, this->negociation_result};
        this->channels.auth_rail_exec(flags, original_exe_or_file, exe_or_file,
                                working_dir, arguments, account, password,
                                this->front, stc, this->vars, this->client_rail_caps);
#else
        (void)flags;
        (void)original_exe_or_file;
        (void)exe_or_file;
        (void)working_dir;
        (void)arguments;
        (void)account;
        (void)password;
#endif
    }

    void auth_rail_exec_cancel(uint16_t flags, const char* original_exe_or_file, uint16_t exec_result) override {
#ifndef __EMSCRIPTEN__
            ServerTransportContext stc{
                this->trans, this->encrypt, this->negociation_result};
        this->channels.auth_rail_exec_cancel(
            flags, original_exe_or_file, exec_result,
            this->front, stc, this->vars, this->client_rail_caps);
#else
        (void)flags;
        (void)original_exe_or_file;
        (void)exec_result;
#endif
    }

    void sespro_rail_exec_result(uint16_t flags, const char* exe_or_file, uint16_t exec_result, uint32_t raw_result) override {
#ifndef __EMSCRIPTEN__
        ServerTransportContext stc{
            this->trans, this->encrypt, this->negociation_result};
        this->channels.sespro_rail_exec_result(
            flags, exe_or_file, exec_result, raw_result,
            this->front, stc, this->vars, this->client_rail_caps);
#else
        (void)flags;
        (void)exe_or_file;
        (void)exec_result;
        (void)raw_result;
#endif
    }

    void sespro_ending_in_progress() override
    {
        this->vars.set<cfg::context::auth_error_message>(
            TR(trkeys::session_logoff_in_progress, this->lang));
        this->set_mod_signal(BACK_EVENT_NEXT);
    }

    void sespro_launch_process_ended() override {
        if (this->delayed_start_capture) {
            this->delayed_start_capture = false;

            if (this->front.can_be_start_capture(false, this->session_log)) {
                if (this->bogus_refresh_rect && this->monitor_count) {
                    this->rdp_suppress_display_updates();
                    this->rdp_allow_display_updates(0, 0, this->negociation_result.front_width, this->negociation_result.front_height);
                }
                this->rdp_input_invalidate(Rect(0, 0, this->negociation_result.front_width, this->negociation_result.front_height));
            }
        }
    }

private:
    template<class... WriterData>
    void send_data_request_ex(uint16_t channelId, WriterData... writer_data)
    {
        write_packets(
            this->trans,
            writer_data...,
            SEC::write_sec_send_fn{0, this->encrypt, this->negociation_result.encryptionLevel},
            [this, channelId](StreamSize<256> /*maxlen*/, OutStream & mcs_header, std::size_t packet_size) {
                MCS::SendDataRequest_Send mcs(
                    mcs_header, this->negociation_result.userid,
                    channelId, 1, 3, packet_size, MCS::PER_ENCODING
                );
                (void)mcs;
            },
            X224::write_x224_dt_tpdu_fn{},
            [this](StreamSize<0> /*maxlen*/, OutStream &, std::size_t total_pdu_sz) {
                (void)this;
                (void)total_pdu_sz;
                IF_ENABLE_METRICS(client_main_channel_data(total_pdu_sz));
            }
        );
    }

    template<class DataWriter>
    void send_pdu_type2(uint8_t pdu_type2, uint8_t stream_id, DataWriter data_writer)
    {
        using packet_size_t = decltype(details_::packet_size(data_writer));

        this->send_data_request_ex(
            GCC::MCS_GLOBAL_CHANNEL,
            [this, &data_writer, pdu_type2, stream_id](
                StreamSize<256 + packet_size_t{}>, OutStream & stream) {
                ShareData sdata(stream);
                sdata.emit_begin(pdu_type2, this->share_id, stream_id);
                {
                    OutStream substream({stream.get_current(), packet_size_t{}});
                    data_writer(packet_size_t{}, substream);
                    stream.out_skip_bytes(substream.get_offset());
                }
                sdata.emit_end();
            },
            [this](StreamSize<256> /*maxlen*/, OutStream & sctrl_header, std::size_t packet_size) {
                ShareControl_Send(
                    sctrl_header, PDUTYPE_DATAPDU,
                    this->negociation_result.userid + GCC::MCS_USERCHANNEL_BASE, packet_size
                );
            }
        );
    }

    std::size_t send_input_slowpath(int time, int message_type, int device_flags, int param1, int param2)
    {
        std::size_t channel_data_size = 0;

        LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
            "mod_rdp::send_input_slowpath");

        if (message_type == RDP_INPUT_SYNCHRONIZE) {
            LOG(LOG_INFO, "mod_rdp::send_input_slowpath: Synchronize Event toggleFlags=0x%X",
                static_cast<unsigned>(param1));
        }

        this->send_pdu_type2(
            PDUTYPE2_INPUT, RDP::STREAM_HI,
            [&](StreamSize<16> /*maxlen*/, OutStream & stream){
                // Payload
                stream.out_uint16_le(1); /* number of events */
                stream.out_uint16_le(0);
                stream.out_uint32_le(time);
                stream.out_uint16_le(message_type);
                stream.out_uint16_le(device_flags);
                stream.out_uint16_le(param1);
                stream.out_uint16_le(param2);

                channel_data_size = stream.tailroom();
            }
        );

        LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
            "mod_rdp::send_input_slowpath done");

        return channel_data_size;
    }

    std::size_t send_input_fastpath(int time, int message_type, uint16_t device_flags, int param1, int param2)
    {
        std::size_t channel_data_size = 0;

        (void)time;
        LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
            "mod_rdp::send_input_fastpath");

        write_packets(
            this->trans,
            [&](StreamSize<256> /*maxlen*/, OutStream & stream) {

                switch (message_type) {
                case RDP_INPUT_SCANCODE:
                    FastPath::KeyboardEvent_Send(stream, device_flags, param1);
                    break;

                case RDP_INPUT_UNICODE:
                    FastPath::UniCodeKeyboardEvent_Send(stream, device_flags, param1);
                    break;

                case RDP_INPUT_SYNCHRONIZE:
                    FastPath::SynchronizeEvent_Send(stream, param1);
                    break;

                case RDP_INPUT_MOUSE:
                    FastPath::MouseEvent_Send(stream, device_flags, param1, param2);
                    break;

                default:
                    LOG(LOG_ERR, "unsupported fast-path input message type 0x%x", unsigned(message_type));
                    throw Error(ERR_RDP_FASTPATH);
                }

                channel_data_size = stream.tailroom();
            },
            [&](StreamSize<256> /*maxlen*/, OutStream & fastpath_header, writable_bytes_view packet) {
                FastPath::ClientInputEventPDU_Send out_cie(
                    fastpath_header, packet.data(), packet.size(), 1,
                    this->encrypt, this->negociation_result.encryptionLevel,
                    this->negociation_result.encryptionMethod
                );
                (void)out_cie;
            }
        );

        LOG_IF(bool(this->verbose & RDPVerbose::input), LOG_INFO,
            "mod_rdp::send_input_fastpath done");

        return channel_data_size;
    }
};

#undef IF_ENABLE_METRICS
