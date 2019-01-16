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
#include "core/RDP/capabilities/cap_font.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/capabilities/cap_share.hpp"
#include "core/RDP/capabilities/cap_sound.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/capabilities/compdesk.hpp"
#include "core/RDP/capabilities/control.hpp"
#include "core/RDP/capabilities/drawgdiplus.hpp"
#include "core/RDP/capabilities/drawninegridcache.hpp"
#include "core/RDP/capabilities/frameacknowledge.hpp"
#include "core/RDP/capabilities/input.hpp"
#include "core/RDP/capabilities/largepointer.hpp"
#include "core/RDP/capabilities/multifragmentupdate.hpp"
#include "core/RDP/capabilities/order.hpp"
#include "core/RDP/capabilities/pointer.hpp"
#include "core/RDP/capabilities/rail.hpp"
#include "core/RDP/capabilities/surfacecommands.hpp"
#include "core/RDP/capabilities/window.hpp"

#include "core/RDP/channels/rdpdr.hpp"

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

#include "core/RDPEA/audio_output.hpp"

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"

#ifndef __EMSCRIPTEN__
# include "mod/rdp/rdp_metrics.hpp"
# define IF_ENABLE_METRICS(m) do { if (this->metrics) this->metrics->m; } while (0)
#else
class RDPMetrics;
# define IF_ENABLE_METRICS(m) do {} while(0)
#endif

#include "RAIL/client_execute.hpp"
#include "mod/mod_api.hpp"

#include "mod/rdp/alternate_shell.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/drdynvc_channel.hpp"
#include "mod/rdp/channels/rail_channel.hpp"
#include "mod/rdp/channels/rail_session_manager.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/sespro_alternate_shell_based_launcher.hpp"
#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/sespro_clipboard_based_launcher.hpp"
#include "mod/rdp/rdp_negociation_data.hpp"
#include "mod/rdp/rdp_orders.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/server_transport_context.hpp"

#include "utils/authorization_channels.hpp"
#include "utils/genrandom.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/splitter.hpp"

#include <cstdlib>
#include <deque>


class mod_rdp : public mod_api, public rdp_api
{
private:

    // TODO: AsynchronousTaskContainer ne serait pas une classe d'usage général qui mériterait son propre fichier ?
    struct AsynchronousTaskContainer
    {
    private:
        static auto remover() noexcept
        {
            return [](AsynchronousTaskContainer* pself, AsynchronousTask& task) noexcept {
                (void)task;
                assert(&task == pself->tasks.front().get());
                pself->tasks.pop_front();
                pself->next();
            };
        }

    public:
        explicit AsynchronousTaskContainer(SessionReactor& session_reactor)
          : session_reactor(session_reactor)
        {}

        void add(std::unique_ptr<AsynchronousTask>&& task)
        {
            this->tasks.emplace_back(std::move(task));
            if (this->tasks.size() == 1u) {
                this->tasks.front()->configure_event(this->session_reactor, {this, remover()});
            }
        }

    private:
        void next()
        {
            if (!this->tasks.empty()) {
                this->tasks.front()->configure_event(this->session_reactor, {this, remover()});
            }
        }

        std::deque<std::unique_ptr<AsynchronousTask>> tasks;
    public:
        SessionReactor& session_reactor;
    };

    struct Channels {
    
#ifndef __EMSCRIPTEN__
        RDPMetrics * metrics;
#endif
        CHANNELS::ChannelDefArray mod_channel_list;
        const AuthorizationChannels authorization_channels;
        ReportMessageApi & report_message;
        Random & gen;
        const bool enable_auth_channel;
        const CHANNELS::ChannelNameId auth_channel;
        int auth_channel_flags      = 0;
        int auth_channel_chanid     = 0;
        const CHANNELS::ChannelNameId checkout_channel;
        int checkout_channel_flags  = 0;
        int checkout_channel_chanid = 0;
        const bool disable_clipboard_log_syslog;
        const bool disable_clipboard_log_wrm;
        const bool log_only_relevant_clipboard_activities;
        const bool bogus_ios_rdpdr_virtual_channel;

        std::unique_ptr<SessionProbeLauncher> session_probe_launcher;
        const bool enable_session_probe;

        const bool                        use_session_probe_to_launch_remote_program;
        std::string                       session_probe_arguments;
        bool                              session_probe_customize_executable_name;
        const std::chrono::milliseconds   session_probe_launch_timeout;
        const std::chrono::milliseconds   session_probe_launch_fallback_timeout;
        const bool                        session_probe_start_launch_timeout_timer_only_after_logon;
        const SessionProbeOnLaunchFailure session_probe_on_launch_failure;
        const std::chrono::milliseconds   session_probe_keepalive_timeout;
        const SessionProbeOnKeepaliveTimeout session_probe_on_keepalive_timeout;
        const bool                        session_probe_end_disconnected_session;
        const std::chrono::milliseconds   session_probe_disconnected_application_limit;
        const std::chrono::milliseconds   session_probe_disconnected_session_limit;
        const std::chrono::milliseconds   session_probe_idle_session_limit;
        const bool                        session_probe_enable_log;
        const bool                        session_probe_enable_log_rotation;
        SessionProbeClipboardBasedLauncher::Params session_probe_clipboard_based_launcher;
        const bool                        session_probe_allow_multiple_handshake;
        const bool                        session_probe_enable_crash_dump;
        const uint32_t                    session_probe_handle_usage_limit;
        const uint32_t                    session_probe_memory_usage_limit;
        const bool                        session_probe_ignore_ui_less_processes_during_end_of_session_check;
        const bool                        session_probe_childless_window_as_unidentified_input_field;
        const bool                        session_probe_public_session;
        std::string                       session_probe_target_informations;
        const std::string                 session_probe_extra_system_processes;
        const std::string                 session_probe_outbound_connection_monitoring_rules;
        const std::string                 session_probe_process_monitoring_rules;
        const std::string                 session_probe_windows_of_these_applications_as_unidentified_input_field;

        const bool use_application_driver;
        const bool disable_file_system_log_syslog;
        const bool disable_file_system_log_wrm;
        std::string proxy_managed_drive_prefix;

        // TODO: target_application only exists while in constructor, remove it from here soon, or use std::string
        const char *                      target_application;
        const char *                      primary_user_id;
        const bool                        experimental_fix_too_long_cookie;



        const bool                        mod_rdp_params_session_probe_use_clipboard_based_launcher;
        const bool                        session_probe_use_clipboard_based_launcher;

        uint16_t    client_execute_flags = 0;
        std::string client_execute_exe_or_file;
        std::string client_execute_working_dir;
        std::string client_execute_arguments;

        bool should_ignore_first_client_execute = false;

        uint16_t    real_client_execute_flags = 0;
        std::string real_client_execute_exe_or_file;
        std::string real_client_execute_working_dir;
        std::string real_client_execute_arguments;
        std::string real_alternate_shell;
        std::string real_working_dir;

        data_size_type max_clipboard_data = 0;
        data_size_type max_rdpdr_data     = 0;
        data_size_type max_drdynvc_data   = 0;

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

        std::unique_ptr<SessionProbeVirtualChannel>   session_probe_virtual_channel;
        SessionProbeVirtualChannel * session_probe_virtual_channel_p = nullptr;

        const bool remote_program;
        const bool remote_program_enhanced;

        std::unique_ptr<VirtualChannelDataSender>     remote_programs_to_client_sender;
        std::unique_ptr<VirtualChannelDataSender>     remote_programs_to_server_sender;

        std::unique_ptr<RemoteProgramsVirtualChannel> remote_programs_virtual_channel;

        std::unique_ptr<RemoteProgramsSessionManager> remote_programs_session_manager;

        FileSystemDriveManager file_system_drive_manager;

        RDPECLIP::CliprdrLogState cliprdrLogStatus;
        rdpdr::RdpDrStatus rdpdrLogStatus;

        const bool enable_rdpdr_data_analysis;
        const RDPVerbose verbose;

        Channels(const ModRDPParams & mod_rdp_params, const RDPVerbose verbose, 
                ReportMessageApi & report_message, Random & gen, RDPMetrics * metrics)
            :
            #ifndef __EMSCRIPTEN__
                metrics(metrics),
            #endif
                authorization_channels(
                mod_rdp_params.allow_channels ? *mod_rdp_params.allow_channels : std::string{},
                mod_rdp_params.deny_channels ? *mod_rdp_params.deny_channels : std::string{}
              )
            , report_message(report_message)
            , gen(gen)
            , enable_auth_channel(mod_rdp_params.alternate_shell[0]
                               && !mod_rdp_params.ignore_auth_channel)
            , auth_channel([&]{
                switch (mod_rdp_params.auth_channel) {
                    case CHANNELS::ChannelNameId():
                    case CHANNELS::ChannelNameId("*"):
                        return CHANNELS::ChannelNameId("wablnch");
                    default:
                        return mod_rdp_params.auth_channel;
                }
              }())
            , checkout_channel(mod_rdp_params.checkout_channel)
            , disable_clipboard_log_syslog(mod_rdp_params.disable_clipboard_log_syslog)
            , disable_clipboard_log_wrm(mod_rdp_params.disable_clipboard_log_wrm)
            , log_only_relevant_clipboard_activities(mod_rdp_params.log_only_relevant_clipboard_activities)
            , bogus_ios_rdpdr_virtual_channel(mod_rdp_params.bogus_ios_rdpdr_virtual_channel)

            , enable_session_probe(mod_rdp_params.enable_session_probe)
            , use_session_probe_to_launch_remote_program(mod_rdp_params.use_session_probe_to_launch_remote_program)

            , session_probe_arguments(mod_rdp_params.session_probe_arguments)
            , session_probe_customize_executable_name(mod_rdp_params.session_probe_customize_executable_name)
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
            , session_probe_enable_log(mod_rdp_params.session_probe_enable_log)
            , session_probe_enable_log_rotation(mod_rdp_params.session_probe_enable_log_rotation)
            , session_probe_clipboard_based_launcher(mod_rdp_params.session_probe_clipboard_based_launcher)
            , session_probe_allow_multiple_handshake(mod_rdp_params.session_probe_allow_multiple_handshake)
            , session_probe_enable_crash_dump(mod_rdp_params.session_probe_enable_crash_dump)
            , session_probe_handle_usage_limit(mod_rdp_params.session_probe_handle_usage_limit)
            , session_probe_memory_usage_limit(mod_rdp_params.session_probe_memory_usage_limit)
            , session_probe_ignore_ui_less_processes_during_end_of_session_check(mod_rdp_params.session_probe_ignore_ui_less_processes_during_end_of_session_check)
            , session_probe_childless_window_as_unidentified_input_field(mod_rdp_params.session_probe_childless_window_as_unidentified_input_field)
            , session_probe_public_session(mod_rdp_params.session_probe_public_session)
            , session_probe_extra_system_processes(mod_rdp_params.session_probe_extra_system_processes)
            , session_probe_outbound_connection_monitoring_rules(mod_rdp_params.session_probe_outbound_connection_monitoring_rules)
            , session_probe_process_monitoring_rules(mod_rdp_params.session_probe_process_monitoring_rules)
            , session_probe_windows_of_these_applications_as_unidentified_input_field(mod_rdp_params.session_probe_windows_of_these_applications_as_unidentified_input_field)

            , use_application_driver(mod_rdp_params.alternate_shell
                && !::strncasecmp(mod_rdp_params.alternate_shell, "\\\\tsclient\\SESPRO\\AppDriver.exe", 31))
            , disable_file_system_log_syslog(mod_rdp_params.disable_file_system_log_syslog)
            , disable_file_system_log_wrm(mod_rdp_params.disable_file_system_log_wrm)
            , proxy_managed_drive_prefix(mod_rdp_params.proxy_managed_drive_prefix)
            , target_application(mod_rdp_params.target_application)
            , primary_user_id(mod_rdp_params.primary_user_id)
            , experimental_fix_too_long_cookie(mod_rdp_params.experimental_fix_too_long_cookie)
            , mod_rdp_params_session_probe_use_clipboard_based_launcher(mod_rdp_params.session_probe_use_clipboard_based_launcher)
            , session_probe_use_clipboard_based_launcher(this->mod_rdp_params_session_probe_use_clipboard_based_launcher
                                                        && (!this->target_application || !(*this->target_application))
                                                        && (!mod_rdp_params.use_client_provided_alternate_shell
                                                            || !mod_rdp_params.alternate_shell[0]
                                                            || mod_rdp_params.remote_program))
            , should_ignore_first_client_execute(mod_rdp_params.should_ignore_first_client_execute)
            , remote_program(mod_rdp_params.remote_program)
            , remote_program_enhanced(mod_rdp_params.remote_program_enhanced)
            , enable_rdpdr_data_analysis(mod_rdp_params.enable_rdpdr_data_analysis)
            , verbose(verbose)
        {
            if (mod_rdp_params.proxy_managed_drives && (*mod_rdp_params.proxy_managed_drives)) {
                this->configure_proxy_managed_drives(mod_rdp_params.proxy_managed_drives, mod_rdp_params.proxy_managed_drive_prefix);
            }

        }

        private:
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

                this->file_system_drive_manager.enable_drive(
                    FileSystemDriveManager::DriveName(
                        array_view_const_char{trimmed_range.begin(), trimmed_range.end()}),
                    proxy_managed_drive_prefix, this->verbose);
            }
        }   // configure_proxy_managed_drives

        public:
        std::unique_ptr<VirtualChannelDataSender> create_to_client_sender(
            CHANNELS::ChannelNameId channel_name, FrontAPI& front) const
        {
            if (!this->authorization_channels.is_authorized(channel_name))
            {
                return nullptr;
            }

            const CHANNELS::ChannelDefArray& front_channel_list =
                front.get_channel_list();

            const CHANNELS::ChannelDef* channel = front_channel_list.get_by_name(channel_name);
            if (!channel)
            {
                return nullptr;
            }

            class ToClientSender : public VirtualChannelDataSender
            {
                FrontAPI& front;
                const CHANNELS::ChannelDef& channel;
                const RDPVerbose verbose;

            public:
                explicit ToClientSender(
                    FrontAPI& front,
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

            auto to_client_sender = std::make_unique<ToClientSender>(front, *channel, this->verbose);

            return std::unique_ptr<VirtualChannelDataSender>(std::move(to_client_sender));
        }

        inline void create_clipboard_virtual_channel(FrontAPI & front, ServerTransportContext & stc) {
            assert(!this->clipboard_to_client_sender 
                && !this->clipboard_to_server_sender);

            this->clipboard_to_client_sender = this->create_to_client_sender(channel_names::cliprdr, front);
            this->clipboard_to_server_sender = this->create_to_server_synchronous_sender(channel_names::cliprdr, stc);

            ClipboardVirtualChannel::Params cvc_params(this->report_message);

            cvc_params.exchanged_data_limit      = this->max_clipboard_data;
            cvc_params.verbose                   = this->verbose;
            cvc_params.clipboard_down_authorized = this->authorization_channels.cliprdr_down_is_authorized();
            cvc_params.clipboard_up_authorized   = this->authorization_channels.cliprdr_up_is_authorized();
            cvc_params.clipboard_file_authorized = this->authorization_channels.cliprdr_file_is_authorized();
            cvc_params.dont_log_data_into_syslog = this->disable_clipboard_log_syslog;
            cvc_params.dont_log_data_into_wrm    = this->disable_clipboard_log_wrm;
            cvc_params.log_only_relevant_clipboard_activities = this->log_only_relevant_clipboard_activities;

            this->clipboard_virtual_channel =
                std::make_unique<ClipboardVirtualChannel>(
                    this->clipboard_to_client_sender.get(),
                    this->clipboard_to_server_sender.get(),
                    front,
                    cvc_params);
        }


        std::unique_ptr<VirtualChannelDataSender> create_to_server_synchronous_sender(CHANNELS::ChannelNameId channel_name, ServerTransportContext & stc)
        {
            const CHANNELS::ChannelDef* channel = this->mod_channel_list.get_by_name(channel_name);
            if (!channel)
            {
                return nullptr;
            }

            class ToServerSender : public VirtualChannelDataSender
            {
                ServerTransportContext & stc;
                CHANNELS::ChannelNameId channel_name;
                uint16_t        channel_id;
                bool            show_protocol;

                const RDPVerbose verbose;

            public:
                explicit ToServerSender(
                    ServerTransportContext & stc,
                    CHANNELS::ChannelNameId channel_name,
                    uint16_t channel_id,
                    bool show_protocol,
                    RDPVerbose verbose)
                : stc(stc)
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

                    virtual_channel_pdu.send_to_server(this->stc, this->channel_id, total_length, flags, chunk_data,
                        chunk_data_length);
                }
            };

            std::unique_ptr<ToServerSender> to_server_sender =
                std::make_unique<ToServerSender>(
                    stc,
                    channel_name,
                    channel->chanid,
                    (channel->flags &
                     GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL),
                    this->verbose);

            return std::unique_ptr<VirtualChannelDataSender>(std::move(to_server_sender));
        }


        std::unique_ptr<VirtualChannelDataSender> create_to_server_asynchronous_sender(CHANNELS::ChannelNameId channel_name, ServerTransportContext & stc, AsynchronousTaskContainer & asynchronous_tasks)
        {
            auto to_server_sender =  create_to_server_synchronous_sender(channel_name, stc);

            class ToServerAsynchronousSender : public VirtualChannelDataSender
            {
                std::unique_ptr<VirtualChannelDataSender> to_server_synchronous_sender;

                AsynchronousTaskContainer& asynchronous_tasks;

                RDPVerbose verbose;

            public:
                explicit ToServerAsynchronousSender(
                    std::unique_ptr<VirtualChannelDataSender> to_server_synchronous_sender,
                    AsynchronousTaskContainer& asynchronous_tasks,
                    RDPVerbose verbose)
                : to_server_synchronous_sender(std::move(to_server_synchronous_sender))
                , asynchronous_tasks(asynchronous_tasks)
                , verbose(verbose)
                {}

                VirtualChannelDataSender& SynchronousSender() override {
                    return *(to_server_synchronous_sender.get());
                }

                void operator()(
                    uint32_t total_length, uint32_t flags,
                    const uint8_t* chunk_data, uint32_t chunk_data_length) override
                {
                    this->asynchronous_tasks.add(
                        std::make_unique<RdpdrSendClientMessageTask>(
                            total_length, flags, chunk_data, chunk_data_length,
                            *this->to_server_synchronous_sender,
                            this->verbose
                        )
                    );
                }
            };

            return std::make_unique<ToServerAsynchronousSender>(
                std::move(to_server_sender),
                asynchronous_tasks,
                this->verbose);
        }


        void create_dynamic_channel_virtual_channel(FrontAPI& front, ServerTransportContext & stc) {
            assert(!this->dynamic_channel_to_client_sender && !this->dynamic_channel_to_server_sender);

            this->dynamic_channel_to_client_sender = this->create_to_client_sender(channel_names::drdynvc, front);
            this->dynamic_channel_to_server_sender = this->create_to_server_synchronous_sender(channel_names::drdynvc, stc);

            DynamicChannelVirtualChannel::Params dcvc_params(this->report_message);

            dcvc_params.exchanged_data_limit = this->max_drdynvc_data;
            dcvc_params.verbose              = this->verbose;

            this->dynamic_channel_virtual_channel =
                std::make_unique<DynamicChannelVirtualChannel>(
                    this->dynamic_channel_to_client_sender.get(),
                    this->dynamic_channel_to_server_sender.get(),
                    dcvc_params);
        }

        inline void create_file_system_virtual_channel(
                    FrontAPI& front,
                    ServerTransportContext & stc,
                    AsynchronousTaskContainer & asynchronous_tasks,
                    GeneralCaps const & client_general_caps,
                    const char (& client_name)[128]) {

            assert(!this->file_system_to_client_sender && !this->file_system_to_server_sender);

            this->file_system_to_client_sender = (((client_general_caps.os_major != OSMAJORTYPE_IOS)
                                                    || !this->bogus_ios_rdpdr_virtual_channel)
                                               ? this->create_to_client_sender(channel_names::rdpdr, front)
                                               : nullptr);
            this->file_system_to_server_sender = this->create_to_server_asynchronous_sender(channel_names::rdpdr, stc, asynchronous_tasks);

            FileSystemVirtualChannel::Params fsvc_params(this->report_message);

            fsvc_params.exchanged_data_limit = this->max_rdpdr_data;
            fsvc_params.verbose = this->verbose;

            fsvc_params.client_name = client_name;
            fsvc_params.file_system_read_authorized = this->authorization_channels.rdpdr_drive_read_is_authorized();
            fsvc_params.file_system_write_authorized = this->authorization_channels.rdpdr_drive_write_is_authorized();
            fsvc_params.parallel_port_authorized = this->authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL);
            fsvc_params.print_authorized = this->authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT);
            fsvc_params.serial_port_authorized = this->authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL);
            fsvc_params.smart_card_authorized = this->authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD);
            // TODO: getpid() is global and execution dependent, replace by a constant because it will break tests
            fsvc_params.random_number = ::getpid();

            fsvc_params.dont_log_data_into_syslog = this->disable_file_system_log_syslog;
            fsvc_params.dont_log_data_into_wrm = this->disable_file_system_log_wrm;

            fsvc_params.proxy_managed_drive_prefix = this->proxy_managed_drive_prefix.c_str();

            this->file_system_virtual_channel =  std::make_unique<FileSystemVirtualChannel>(
                    asynchronous_tasks.session_reactor,
                    this->file_system_to_client_sender.get(),
                    this->file_system_to_server_sender.get(),
                    this->file_system_drive_manager,
                    front,
                    fsvc_params);

            if (this->file_system_to_server_sender) {
                if (this->enable_session_probe 
                || this->use_application_driver) {
                    this->file_system_virtual_channel->enable_session_probe_drive();
                }
            }
        }

        std::string get_session_probe_arguments_no_remote_program()
        {
            // Executable file name of SP.
            char exe_var_str[16] {};
            if (this->session_probe_customize_executable_name) {
                ::snprintf(exe_var_str, sizeof(exe_var_str), "-%d", ::getpid());
            }

            // Target informations
            str_assign(this->session_probe_target_informations, this->target_application, ':');
            if (!this->session_probe_public_session) {
                this->session_probe_target_informations += this->primary_user_id;
            }

            if (this->mod_rdp_params_session_probe_use_clipboard_based_launcher
                && this->target_application && *this->target_application
            ) {
                assert(!this->session_probe_use_clipboard_based_launcher);

                LOG(LOG_WARNING,
                    "mod_rdp: "
                        "Clipboard based Session Probe launcher is not compatible with application. "
                        "Falled back to using AlternateShell based launcher.");
            }

            char clipboard_based_launcher_cookie[32];
            {
                SslSha1 sha1;
                sha1.update(this->session_probe_target_informations);

                uint8_t sig[SslSha1::DIGEST_LENGTH];
                sha1.final(sig);

                snprintf(clipboard_based_launcher_cookie, sizeof(clipboard_based_launcher_cookie),
                    "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                    sig[0], sig[1], sig[2], sig[3], sig[4], sig[5], sig[6], sig[7], sig[8], sig[9]);
            }

            const char * cookie = [&]() -> const char * {
                    if (this->session_probe_use_clipboard_based_launcher){
                        return "";
                    }
                    if (this->experimental_fix_too_long_cookie && (this->session_probe_target_informations.length() > 20)){
                      return clipboard_based_launcher_cookie;
                    }
                    return this->session_probe_target_informations.c_str();
                }();

            std::string cookie_param = (cookie && *cookie)
              ? str_concat("/#", cookie, ' ')
              : std::string();

            Channels::replace_probe_arguments(this->session_probe_arguments,
                    "${EXE_VAR}", exe_var_str,
                    "${TITLE_VAR} ", "",
                    "/${COOKIE_VAR} ", cookie_param.c_str(),
                    "${CBSPL_VAR} ", this->session_probe_use_clipboard_based_launcher ? "CD %TMP%&" : ""
                );

            return this->session_probe_arguments;
        }

        inline SessionProbeVirtualChannel& get_session_probe_virtual_channel(
                        FrontAPI& front,
                        ServerTransportContext & stc,
                        AsynchronousTaskContainer & asynchronous_tasks,
                        SessionReactor& session_reactor,
                        mod_api& mod, rdp_api& rdp,
                        const Translation::language_t & lang,
                        const bool bogus_refresh_rect,
                        const bool allow_using_multiple_monitors, // TODO duplicate monitor_count ?
                        const uint32_t monitor_count,
                        GeneralCaps const & client_general_caps,
                        const char (& client_name)[128]
                    ) {
            if (!this->session_probe_virtual_channel) {
                assert(!this->session_probe_to_server_sender);

                this->session_probe_to_server_sender =
                    this->create_to_server_synchronous_sender(channel_names::sespro, stc);

                if (!this->file_system_virtual_channel) {
                    this->create_file_system_virtual_channel(front, stc, asynchronous_tasks, client_general_caps, client_name);
                }

                FileSystemVirtualChannel& file_system_virtual_channel = *this->file_system_virtual_channel;

                SessionProbeVirtualChannel::Params sp_vc_params(this->report_message);

                sp_vc_params.front_width = stc.negociation_result.front_width;
                sp_vc_params.front_height = stc.negociation_result.front_height;
                sp_vc_params.exchanged_data_limit = static_cast<data_size_type>(-1);
                sp_vc_params.verbose  = this->verbose;
                sp_vc_params.real_alternate_shell = this->real_alternate_shell.c_str();
                sp_vc_params.real_working_dir = this->real_working_dir.c_str();
                sp_vc_params.lang = lang;
                sp_vc_params.bogus_refresh_rect_ex = (bogus_refresh_rect && allow_using_multiple_monitors && (monitor_count > 1));
                sp_vc_params.show_maximized = (!remote_program);
                sp_vc_params.target_informations = this->session_probe_target_informations.c_str();

                sp_vc_params.session_probe_launch_timeout = this->session_probe_launch_timeout;
                sp_vc_params.session_probe_launch_fallback_timeout = this->session_probe_launch_fallback_timeout;
                sp_vc_params.session_probe_keepalive_timeout = this->session_probe_keepalive_timeout;
                sp_vc_params.session_probe_on_keepalive_timeout = this->session_probe_on_keepalive_timeout;
                sp_vc_params.session_probe_on_launch_failure = this->session_probe_on_launch_failure;
                sp_vc_params.session_probe_end_disconnected_session = this->session_probe_end_disconnected_session;
                sp_vc_params.session_probe_disconnected_application_limit = this->session_probe_disconnected_application_limit;
                sp_vc_params.session_probe_disconnected_session_limit = this->session_probe_disconnected_session_limit;
                sp_vc_params.session_probe_idle_session_limit =  this->session_probe_idle_session_limit;
                sp_vc_params.session_probe_enable_log = this->session_probe_enable_log;
                sp_vc_params.session_probe_enable_log_rotation = this->session_probe_enable_log_rotation;
                sp_vc_params.session_probe_allow_multiple_handshake = this->session_probe_allow_multiple_handshake;
                sp_vc_params.session_probe_enable_crash_dump = this->session_probe_enable_crash_dump;
                sp_vc_params.session_probe_handle_usage_limit = this->session_probe_handle_usage_limit;
                sp_vc_params.session_probe_memory_usage_limit = this->session_probe_memory_usage_limit;
                sp_vc_params.session_probe_ignore_ui_less_processes_during_end_of_session_check = this->session_probe_ignore_ui_less_processes_during_end_of_session_check;
                sp_vc_params.session_probe_childless_window_as_unidentified_input_field = this->session_probe_childless_window_as_unidentified_input_field;
                sp_vc_params.session_probe_extra_system_processes = this->session_probe_extra_system_processes.c_str();
                sp_vc_params.session_probe_outbound_connection_monitoring_rules = this->session_probe_outbound_connection_monitoring_rules.c_str();
                sp_vc_params.session_probe_process_monitoring_rules = this->session_probe_process_monitoring_rules.c_str();
                sp_vc_params.session_probe_windows_of_these_applications_as_unidentified_input_field = this->session_probe_windows_of_these_applications_as_unidentified_input_field.c_str();

                this->session_probe_virtual_channel = std::make_unique<SessionProbeVirtualChannel>(
                        session_reactor,
                        this->session_probe_to_server_sender.get(),
                        front,
                        mod,
                        rdp,
                        file_system_virtual_channel,
                        this->gen,
                        sp_vc_params);
            }

            return *this->session_probe_virtual_channel;
        }

        inline RemoteProgramsVirtualChannel& get_remote_programs_virtual_channel(
                        FrontAPI& front,
                        ServerTransportContext & stc,
                        const ModRdpVariables & vars,
                        RailCaps const & client_rail_caps) {
            if (!this->remote_programs_virtual_channel) {
                assert(!this->remote_programs_to_client_sender &&
                    !this->remote_programs_to_server_sender);

                this->remote_programs_to_client_sender =
                    this->create_to_client_sender(channel_names::rail, front);
                this->remote_programs_to_server_sender =
                    this->create_to_server_synchronous_sender(channel_names::rail, stc);

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
                    (client_rail_caps.RailSupportLevel & TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED);
                remote_programs_virtual_channel_params.client_supports_enhanced_remoteapp =
                    this->remote_program_enhanced;


                this->remote_programs_virtual_channel =
                    std::make_unique<RemoteProgramsVirtualChannel>(
                        this->remote_programs_to_client_sender.get(),
                        this->remote_programs_to_server_sender.get(),
                        front,
                        vars,
                        remote_programs_virtual_channel_params);
            }

            return *this->remote_programs_virtual_channel;
        }


    private:
    public:
        // TODO: make that private again when callers will be moved to channels
        void send_to_front_channel(FrontAPI & front, CHANNELS::ChannelNameId mod_channel_name, uint8_t const * data
                                  , size_t length, size_t chunk_size, int flags) {
            const CHANNELS::ChannelDef * front_channel = front.get_channel_list().get_by_name(mod_channel_name);
            if (front_channel) {
                front.send_to_channel(*front_channel, data, length, chunk_size, flags);
            }
        }

        void process_cliprdr_event(InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
            FrontAPI& front,
            ServerTransportContext & stc
        ) {
            if (!this->clipboard_virtual_channel) {
                this->create_clipboard_virtual_channel(front, stc);
            }

            ClipboardVirtualChannel& channel = *this->clipboard_virtual_channel;

            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                InStream clone = stream.clone();
                RDPECLIP::streamLogCliprdr(clone, flags, this->cliprdrLogStatus);// FIX
            }

            std::unique_ptr<AsynchronousTask> out_asynchronous_task;
            channel.process_server_message(length, flags, stream.get_current(), chunk_size, out_asynchronous_task);
            assert(!out_asynchronous_task);
        }   // process_cliprdr_event


        void process_auth_event(
            const CHANNELS::ChannelDef & auth_channel,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
            FrontAPI& front,
            mod_api & mod_rdp,
            ServerTransportContext & stc,
            AsynchronousTaskContainer & asynchronous_tasks,
            GeneralCaps const & client_general_caps,
            const char (& client_name)[128],
            AuthApi& authentifier
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
                const bool disable_input_event     = (::strcasecmp(parameters[0].c_str(), "Enable") != 0);
                const bool disable_graphics_update = false;
                mod_rdp.disable_input_event_and_graphics_update(disable_input_event, disable_graphics_update);
            }
            else if (!::strcasecmp(order.c_str(), "Log") && !parameters.empty()) {
                LOG(LOG_INFO, "WABLauncher: %s", parameters[0].c_str());
            }
            else if (!::strcasecmp(order.c_str(), "RemoveDrive") && parameters.empty()) {

                if (!this->file_system_virtual_channel) {
                    this->create_file_system_virtual_channel(front, stc, asynchronous_tasks, client_general_caps, client_name);
                }

                FileSystemVirtualChannel& rdpdr_channel = *this->file_system_virtual_channel;


                rdpdr_channel.disable_session_probe_drive();
            }
            else {
                LOG(LOG_INFO, "Auth channel data=\"%s\"", auth_channel_message);

                authentifier.set_auth_channel_target(auth_channel_message.c_str());
            }
        }

        void process_checkout_event(
            const CHANNELS::ChannelDef & checkout_channel,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
            AuthApi& authentifier
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

    //        send_checkout_channel_data("{ \"response_code\": 0, \"response_message\": \"Succeeded.\" }");
            authentifier.set_pm_request(checkout_channel_message.c_str());
        }


        void process_session_probe_event(
            const CHANNELS::ChannelDef & session_probe_channel,
            InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
            FrontAPI& front,
            mod_api & mod_rdp,
            rdp_api& rdp,
            ServerTransportContext & stc,
            AsynchronousTaskContainer & asynchronous_tasks,
            SessionReactor& session_reactor,
            GeneralCaps const & client_general_caps,
            const char (& client_name)[128],
            const bool allow_using_multiple_monitors,
            const uint32_t monitor_count,
            const bool bogus_refresh_rect,
            const Translation::language_t & lang
        ) {
            (void)session_probe_channel;
            SessionProbeVirtualChannel& channel = this->get_session_probe_virtual_channel(front, stc, asynchronous_tasks, session_reactor, mod_rdp, rdp, lang, bogus_refresh_rect, allow_using_multiple_monitors, monitor_count, client_general_caps, client_name);

            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            channel.process_server_message(length, flags, stream.get_current(), chunk_size, out_asynchronous_task);

            assert(!out_asynchronous_task);
        }

        void process_rail_event(const CHANNELS::ChannelDef & rail_channel,
                InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
                FrontAPI& front,
                ServerTransportContext & stc,
                const ModRdpVariables & vars,
                RailCaps const & client_rail_caps
                ) {
            (void)rail_channel;
            RemoteProgramsVirtualChannel& channel = this->get_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);

            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            channel.process_server_message(length, flags, stream.get_current(), chunk_size, out_asynchronous_task);

            assert(!out_asynchronous_task);
        }

        void send_to_mod_cliprdr_channel(InStream & chunk, size_t length, uint32_t flags,
                                FrontAPI& front,
                                ServerTransportContext & stc) {

            if (!this->clipboard_virtual_channel) {
                this->create_clipboard_virtual_channel(front, stc);
            }
            ClipboardVirtualChannel& channel = *this->clipboard_virtual_channel;

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

        void send_to_mod_rail_channel(InStream & chunk, size_t length, uint32_t flags,
                        FrontAPI& front,
                        ServerTransportContext & stc,
                        const ModRdpVariables & vars,
                        RailCaps const & client_rail_caps) {
            RemoteProgramsVirtualChannel& channel = this->get_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);

            channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());

        }   // send_to_mod_rail_channel

        void process_drdynvc_event(InStream & stream, uint32_t length, uint32_t flags, size_t chunk_size,
                                   FrontAPI& front,
                                   ServerTransportContext & stc,
                                   AsynchronousTaskContainer & asynchronous_tasks) {

            if (!this->dynamic_channel_virtual_channel) {
                this->create_dynamic_channel_virtual_channel(front, stc);
            }

            DynamicChannelVirtualChannel& channel = *this->dynamic_channel_virtual_channel;

            std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            channel.process_server_message(length, flags, stream.get_current(), chunk_size, out_asynchronous_task);

            if (out_asynchronous_task) {
                asynchronous_tasks.add(std::move(out_asynchronous_task));
            }
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
                               AsynchronousTaskContainer & asynchronous_tasks,
                               GeneralCaps const & client_general_caps,
                               const char (& client_name)[128]) {
            if (!this->enable_rdpdr_data_analysis
            &&   this->authorization_channels.rdpdr_type_all_is_authorized()
            &&  !this->file_system_drive_manager.has_managed_drive()) {

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

                this->send_to_front_channel(front, channel_names::rdpdr, stream.get_current(), length, chunk_size, flags);
                return;
            }

            if (!this->file_system_virtual_channel) {
                this->create_file_system_virtual_channel(front, stc, asynchronous_tasks, client_general_caps, client_name);
            }

            FileSystemVirtualChannel& channel = *this->file_system_virtual_channel;

            std::unique_ptr<AsynchronousTask> out_asynchronous_task;
            channel.process_server_message(length, flags, stream.get_current(), chunk_size, out_asynchronous_task);
            if (out_asynchronous_task) {
                asynchronous_tasks.add(std::move(out_asynchronous_task));
            }
        }


        static void replace(std::string & text_with_tags, char const* marker, char const* replacement){
            size_t pos = 0;
            auto const marker_len = strlen(marker);
            auto const replacement_len = strlen(replacement);
            while ((pos = text_with_tags.find(marker, pos)) != std::string::npos) {
                text_with_tags.replace(pos, marker_len, replacement);
                pos += replacement_len;
            }
        };

        static void replace_shell_arguments(std::string & text_with_tags, 
                                char const* marker1, char const* replacement1,
                                char const* marker2, char const* replacement2,
                                char const* marker3, char const* replacement3){
                Channels::replace(text_with_tags, marker1, replacement1);
                Channels::replace(text_with_tags, marker2, replacement2);
                Channels::replace(text_with_tags, marker3, replacement3);
        };

        static void replace_probe_arguments(std::string & text_with_tags, 
                                char const* marker1, char const* replacement1,
                                char const* marker2, char const* replacement2,
                                char const* marker3, char const* replacement3,
                                char const* marker4, char const* replacement4){
                Channels::replace(text_with_tags, marker1, replacement1);
                Channels::replace(text_with_tags, marker2, replacement2);
                Channels::replace(text_with_tags, marker3, replacement3);
                Channels::replace(text_with_tags, marker4, replacement4);
        };

        void init_remote_program_with_session_probe(
                        FrontAPI& front,
                        mod_api & mod_rdp,
                        const ModRDPParams & mod_rdp_params,
                        SessionReactor& session_reactor,
                        Translation::language_t lang,
                        Font const & font,
                        AuthApi & authentifier)
        {

            char session_probe_window_title[32] = { 0 };
            uint32_t const r = this->gen.rand32();

            snprintf(session_probe_window_title,
                sizeof(session_probe_window_title),
                "%X%X%X%X",
                ((r & 0xFF000000) >> 24),
                ((r & 0x00FF0000) >> 16),
                ((r & 0x0000FF00) >> 8),
                  r & 0x000000FF
                );

            bool has_target = (mod_rdp_params.target_application && *mod_rdp_params.target_application);
            bool use_client_provided_remoteapp = (mod_rdp_params.use_client_provided_remoteapp
                                                && mod_rdp_params.client_execute_exe_or_file
                                                && *mod_rdp_params.client_execute_exe_or_file);

            if (has_target) {
                if (this->use_session_probe_to_launch_remote_program) {
                    std::string shell_arguments = mod_rdp_params.shell_arguments;
                    Channels::replace_shell_arguments(shell_arguments,
                                "${APPID}", mod_rdp_params.target_application,
                                "${USER}", mod_rdp_params.target_application_account,
                                "${PASSWORD}", mod_rdp_params.target_application_password);

                    this->real_alternate_shell = std::string(mod_rdp_params.alternate_shell);
                    if (!shell_arguments.empty()) {
                        str_append(this->real_alternate_shell, ' ', shell_arguments);
                    }
                    this->real_working_dir     = mod_rdp_params.shell_working_dir;
                }
                else {
                    Channels::replace_shell_arguments(this->real_client_execute_arguments,
                        "${APPID}", mod_rdp_params.target_application,
                        "${USER}", mod_rdp_params.target_application_account,
                        "${PASSWORD}", mod_rdp_params.target_application_password);
                    this->real_client_execute_flags       = 0;
                }
                this->client_execute_flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
                this->session_probe_launcher = std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
            }
            else {
                if (use_client_provided_remoteapp) {
                        this->real_client_execute_arguments   = mod_rdp_params.client_execute_arguments;
                        this->real_client_execute_flags       = mod_rdp_params.client_execute_flags;
                        this->client_execute_flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
                        this->session_probe_launcher = std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
                }
            }

            if (has_target || use_client_provided_remoteapp){

                if (use_client_provided_remoteapp 
                || !this->use_session_probe_to_launch_remote_program) {
                    this->real_alternate_shell = "[None]";
                    this->real_client_execute_exe_or_file = mod_rdp_params.client_execute_exe_or_file;
                    this->real_client_execute_working_dir = mod_rdp_params.client_execute_working_dir;
                }

                this->client_execute_exe_or_file = mod_rdp_params.session_probe_exe_or_file;

                // Executable file name of SP.
                char exe_var_str[16] {};
                if (this->session_probe_customize_executable_name) {
                    ::snprintf(exe_var_str, sizeof(exe_var_str), "-%d", ::getpid());
                }

                std::string title_param = str_concat("TITLE ", session_probe_window_title, '&');

                // Target informations
                str_assign(this->session_probe_target_informations, this->target_application, ':');
                if (!this->session_probe_public_session) {
                    this->session_probe_target_informations += this->primary_user_id;
                }
                std::string cookie_param = [](std::string s){
                        if (s.size() == 0) { return std::string(); }
                        return std::string("/#")+ s + " ";
                    }(this->session_probe_target_informations); 

                Channels::replace_probe_arguments(this->session_probe_arguments,
                    "${EXE_VAR}", exe_var_str,
                    "${TITLE_VAR} ", title_param.c_str(),
                    "/${COOKIE_VAR} ", cookie_param.c_str(),
                    "${CBSPL_VAR} ", "");

                this->client_execute_arguments   = this->session_probe_arguments;
                this->client_execute_working_dir = "%TMP%";
            }

            this->remote_programs_session_manager =
                std::make_unique<RemoteProgramsSessionManager>(
                    session_reactor, front, mod_rdp, lang,
                    font, mod_rdp_params.theme, authentifier,
                    session_probe_window_title,
                    mod_rdp_params.client_execute,
                    mod_rdp_params.rail_disconnect_message_delay,
                    this->verbose
                );
            
        }
        
        
       void init_remote_program_without_session_probe(
                    FrontAPI& front,
                    mod_api & mod_rdp,
                    const ModRDPParams & mod_rdp_params,
                    SessionReactor& session_reactor,
                    Translation::language_t lang,
                    Font const & font,
                    AuthApi & authentifier)
       {
            char session_probe_window_title[32] = { 0 };
            uint32_t const r = this->gen.rand32();

            snprintf(session_probe_window_title,
                sizeof(session_probe_window_title),
                "%X%X%X%X",
                ((r & 0xFF000000) >> 24),
                ((r & 0x00FF0000) >> 16),
                ((r & 0x0000FF00) >> 8),
                  r & 0x000000FF
                );

            if (mod_rdp_params.target_application 
            && (*mod_rdp_params.target_application)) {
                std::string shell_arguments = get_alternate_shell_arguments(
                    mod_rdp_params.shell_arguments,
                    get_alternate_shell_arguments::App{mod_rdp_params.target_application},
                    get_alternate_shell_arguments::Account{mod_rdp_params.target_application_account},
                    get_alternate_shell_arguments::Password{mod_rdp_params.target_application_password});

                this->client_execute_exe_or_file = mod_rdp_params.alternate_shell;
                this->client_execute_arguments   = std::move(shell_arguments);
                this->client_execute_working_dir = mod_rdp_params.shell_working_dir;
                this->client_execute_flags       = TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY;
            }
            else {
                if (mod_rdp_params.use_client_provided_remoteapp
                    && mod_rdp_params.client_execute_exe_or_file
                    && *mod_rdp_params.client_execute_exe_or_file
                    ) {
                        this->client_execute_flags       = mod_rdp_params.client_execute_flags;
                        this->client_execute_exe_or_file = mod_rdp_params.client_execute_exe_or_file;
                        this->client_execute_arguments   = mod_rdp_params.client_execute_arguments;
                        this->client_execute_working_dir = mod_rdp_params.client_execute_working_dir;
                }
            }

            this->remote_programs_session_manager =
                    std::make_unique<RemoteProgramsSessionManager>(
                        session_reactor, front, mod_rdp, lang,
                        font, mod_rdp_params.theme, authentifier,
                        session_probe_window_title,
                        mod_rdp_params.client_execute,
                        mod_rdp_params.rail_disconnect_message_delay,
                        this->verbose
                    );

        }
        

        void init_no_remote_program_with_session_probe(
                    mod_api & mod_rdp,
                    const ClientInfo & info,
                    const ModRDPParams & mod_rdp_params,
                    char (&program)[512],
                    char (&directory)[512],
                    SessionReactor& session_reactor)
        {
            std::string session_probe_arguments = this->get_session_probe_arguments_no_remote_program();

            if (mod_rdp_params.target_application 
            && (*mod_rdp_params.target_application)) {
                std::string shell_arguments = get_alternate_shell_arguments(
                    mod_rdp_params.shell_arguments,
                    get_alternate_shell_arguments::App{mod_rdp_params.target_application},
                    get_alternate_shell_arguments::Account{mod_rdp_params.target_application_account},
                    get_alternate_shell_arguments::Password{mod_rdp_params.target_application_password});

                std::string alternate_shell(mod_rdp_params.alternate_shell);

                if (!shell_arguments.empty()) {
                    str_append(alternate_shell, ' ', shell_arguments);
                }

                this->real_alternate_shell = std::move(alternate_shell);
                this->real_working_dir     = mod_rdp_params.shell_working_dir;

                alternate_shell = mod_rdp_params.session_probe_exe_or_file;

                if (!::strncmp(alternate_shell.c_str(), "||", 2)) {
                    alternate_shell.erase(0, 2);
                }

                str_append(alternate_shell, ' ', session_probe_arguments);

                strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
                program[sizeof(program) - 1] = 0;
                //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                const char * session_probe_working_dir = "%TMP%";
                strncpy(directory, session_probe_working_dir, sizeof(directory) - 1);
                directory[sizeof(directory) - 1] = 0;

                this->session_probe_launcher =
                    std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
            }
            else {
                if (mod_rdp_params.use_client_provided_alternate_shell
                    && info.alternate_shell[0] && !info.remote_program
                ) {
                    std::string alternate_shell = info.alternate_shell;
                    std::string working_dir = info.working_dir;

                    this->real_alternate_shell = std::move(alternate_shell);
                    this->real_working_dir     = std::move(working_dir);

                    alternate_shell = mod_rdp_params.session_probe_exe_or_file;

                    if (!::strncmp(alternate_shell.c_str(), "||", 2)) {
                        alternate_shell.erase(0, 2);
                    }

                    str_append(alternate_shell, ' ', session_probe_arguments);

                    strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
                    program[sizeof(program) - 1] = 0;
                    //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                    const char * session_probe_working_dir = "%TMP%";
                    strncpy(directory, session_probe_working_dir, sizeof(directory) - 1);
                    directory[sizeof(directory) - 1] = 0;

                    this->session_probe_launcher = std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
                }
                else {
                    std::string alternate_shell(mod_rdp_params.session_probe_exe_or_file);

                    if (!::strncmp(alternate_shell.c_str(), "||", 2)) {
                        alternate_shell.erase(0, 2);
                    }

                    str_append(alternate_shell, ' ', session_probe_arguments);

                    if (this->session_probe_use_clipboard_based_launcher) {
                        this->session_probe_launcher = std::make_unique<SessionProbeClipboardBasedLauncher>(
                                session_reactor,
                                mod_rdp, alternate_shell.c_str(),
                                this->session_probe_clipboard_based_launcher,
                                this->verbose);
                    }
                    else {
                        strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
                        program[sizeof(program) - 1] = 0;
                        //LOG(LOG_INFO, "AlternateShell: \"%s\"", this->program);

                        const char * session_probe_working_dir = "%TMP%";
                        strncpy(directory, session_probe_working_dir, sizeof(directory) - 1);
                        directory[sizeof(directory) - 1] = 0;

                        this->session_probe_launcher =
                            std::make_unique<SessionProbeAlternateShellBasedLauncher>(this->verbose);
                    }
                }
            }
        }
        
        
        void init_no_remote_program_no_session_probe(
                    const ClientInfo & info,
                    const ModRDPParams & mod_rdp_params,
                    char (&program)[512],
                    char (&directory)[512])
        {
            if (mod_rdp_params.target_application  && (*mod_rdp_params.target_application)) {

                std::string shell_arguments = get_alternate_shell_arguments(
                    mod_rdp_params.shell_arguments,
                    get_alternate_shell_arguments::App{mod_rdp_params.target_application},
                    get_alternate_shell_arguments::Account{mod_rdp_params.target_application_account},
                    get_alternate_shell_arguments::Password{mod_rdp_params.target_application_password});

                std::string alternate_shell(mod_rdp_params.alternate_shell);

                if (!shell_arguments.empty()) {
                    str_append(alternate_shell, ' ', shell_arguments);
                }

                strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
                program[sizeof(program) - 1] = 0;
                strncpy(directory, std::string(mod_rdp_params.shell_working_dir).c_str(), sizeof(directory) - 1);
                directory[sizeof(directory) - 1] = 0;
            }
            else {
                if (mod_rdp_params.use_client_provided_alternate_shell 
                && info.alternate_shell[0] 
                && !info.remote_program
                ) {
                    std::string alternate_shell = info.alternate_shell;
                    std::string working_dir = info.working_dir;

                    strncpy(program, alternate_shell.c_str(), sizeof(program) - 1);
                    program[sizeof(program) - 1] = 0;
                    strncpy(directory, working_dir.c_str(), sizeof(directory) - 1);
                    directory[sizeof(directory) - 1] = 0;
                }
            }
        }

        void send_to_mod_rdpdr_channel(const CHANNELS::ChannelDef * rdpdr_channel,
                                       InStream & chunk, size_t length, uint32_t flags,
                                       FrontAPI& front,
                                       ServerTransportContext & stc,
                                       AsynchronousTaskContainer & asynchronous_tasks,
                                       GeneralCaps const & client_general_caps,
                                       const char (& client_name)[128])
        {
            if (!this->enable_rdpdr_data_analysis
            &&   this->authorization_channels.rdpdr_type_all_is_authorized()
            &&  !this->file_system_drive_manager.has_managed_drive()) {

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

                this->send_to_channel(*rdpdr_channel, chunk.get_data(), chunk.get_capacity(), length, flags, stc);
                return;
            }

            if (!this->file_system_virtual_channel) {
                this->create_file_system_virtual_channel(front, stc, asynchronous_tasks, client_general_caps, client_name);
            }

            FileSystemVirtualChannel& channel = *this->file_system_virtual_channel;

            channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());
        }

        void send_to_mod_drdynvc_channel(InStream & chunk, size_t length, uint32_t flags,
                                         FrontAPI& front,
                                         ServerTransportContext & stc)
        {

            if (!this->dynamic_channel_virtual_channel) {
                this->create_dynamic_channel_virtual_channel(front, stc);
            }

            DynamicChannelVirtualChannel& channel = *this->dynamic_channel_virtual_channel;

            channel.process_client_message(length, flags, chunk.get_current(), chunk.in_remain());
        }
    
        void send_to_channel(
            const CHANNELS::ChannelDef & channel,
            uint8_t const * chunk, std::size_t chunk_size,
            size_t length, uint32_t flags,
            ServerTransportContext & stc)
        {
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

                virtual_channel_pdu.send_to_server(stc, channel.chanid, length, flags, chunk, chunk_size);
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
                    if (remaining_data_length == virtual_channel_data_length) {
                        return (flags & (~CHANNELS::CHANNEL_FLAG_FIRST));
                    }

                    return (flags & (~(CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)));
                };

                do {
                    const size_t virtual_channel_data_length =
                        std::min<size_t>(remaining_data_length, CHANNELS::CHANNEL_CHUNK_LENGTH);

                    CHANNELS::VirtualChannelPDU virtual_channel_pdu;

                    LOG(LOG_INFO, "send to server");

                    virtual_channel_pdu.send_to_server(stc, channel.chanid, length, get_channel_control_flags(
                            flags, length, remaining_data_length, virtual_channel_data_length
                        ), virtual_channel_data, virtual_channel_data_length);

                    remaining_data_length -= virtual_channel_data_length;
                    virtual_channel_data  += virtual_channel_data_length;
                }
                while (remaining_data_length);
            }

            if (bool(this->verbose & RDPVerbose::channels)) {
                LOG(LOG_INFO, "mod_rdp::send_to_channel done");
            }
        }
    
        void send_to_mod_channel(
            CHANNELS::ChannelNameId front_channel_name,
            InStream & chunk, size_t length, uint32_t flags,
            FrontAPI& front,
            ServerTransportContext & stc,
            AsynchronousTaskContainer & asynchronous_tasks,
            GeneralCaps const & client_general_caps,
            const ModRdpVariables & vars,
            RailCaps const & client_rail_caps,
            const char (& client_name)[128]
        ) {
            const CHANNELS::ChannelDef * mod_channel = this->mod_channel_list.get_by_name(front_channel_name);
            if (!mod_channel) {
                return;
            }

//            switch (front_channel_name) {
//                case channel_names::cliprdr:
//                    this->create_cliprdr_channel(chunk, length, flags, front, stc);
//                    break;
//                case channel_names::rail:
//                    this->create_mod_rail_channel(chunk, length, flags, front, stc, vars, client_rail_caps);
//                    break;
//                case channel_names::rdpdr:
//                    this->create_mod_rdpdr_channel(mod_channel, chunk, length, flags, front, stc, asynchronous_tasks, 
//                                    client_general_caps, client_name);
//                    break;
//                case channel_names::drdynvc:
//                    break;
//                    this->create_to_mod_drdynvc_channel(chunk, length, flags, front, stc);
//                default:
//                    this->create_channel_by_name(*mod_channel, chunk.get_data(), chunk.get_capacity(), length, flags, stc);
//            }

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
                    this->send_to_mod_rdpdr_channel(mod_channel, chunk, length, flags, front, stc, asynchronous_tasks, 
                                    client_general_caps, client_name);
                    break;
                case channel_names::drdynvc:
                    IF_ENABLE_METRICS(client_other_channel_data(length));
                    this->send_to_mod_drdynvc_channel(chunk, length, flags, front, stc);
                    break;
                default:
                    IF_ENABLE_METRICS(client_other_channel_data(length));
                    this->send_to_channel(*mod_channel, chunk.get_data(), chunk.get_capacity(), length, flags, stc);
            }
        }
    
        // This function can be called several times. If a remaining session_probe is running on the
        // target serveur, the session probe channels is already there before the session probe launcher is created
        void do_enable_session_probe(
            FrontAPI& front,
            ServerTransportContext & stc,
            mod_api & mod_rdp,
            rdp_api& rdp,
            AsynchronousTaskContainer & asynchronous_tasks,
            SessionReactor& session_reactor,
            GeneralCaps const & client_general_caps,
            const ModRdpVariables & vars,
            RailCaps const & client_rail_caps,
            const char (& client_name)[128],
            const bool allow_using_multiple_monitors,
            const uint32_t monitor_count,
            const bool bogus_refresh_rect,
            const Translation::language_t & lang)
        {
            assert(this->enable_session_probe);
            if (this->session_probe_launcher){
                if (!this->clipboard_virtual_channel) {
                    this->create_clipboard_virtual_channel(front, stc);
                }
                ClipboardVirtualChannel& cvc = *this->clipboard_virtual_channel;
                cvc.set_session_probe_launcher(this->session_probe_launcher.get());

                if (!this->file_system_virtual_channel) {
                    this->create_file_system_virtual_channel(front, stc, asynchronous_tasks, client_general_caps, client_name);
                }

                FileSystemVirtualChannel& fsvc = *this->file_system_virtual_channel;

                fsvc.set_session_probe_launcher(this->session_probe_launcher.get());

                this->file_system_drive_manager.set_session_probe_launcher(this->session_probe_launcher.get());
                SessionProbeVirtualChannel& spvc = this->get_session_probe_virtual_channel(front, stc, 
                                                                asynchronous_tasks, session_reactor,
                                                                mod_rdp, rdp, lang, 
                                                                bogus_refresh_rect, 
                                                                allow_using_multiple_monitors, 
                                                                monitor_count, 
                                                                client_general_caps,
                                                                client_name);
                spvc.set_session_probe_launcher(this->session_probe_launcher.get());
                this->session_probe_virtual_channel_p = &spvc;
                if (!this->session_probe_start_launch_timeout_timer_only_after_logon) {
                    spvc.start_launch_timeout_timer();
                }
                this->session_probe_launcher->set_clipboard_virtual_channel(&cvc);
                this->session_probe_launcher->set_session_probe_virtual_channel(this->session_probe_virtual_channel_p);

                if (this->remote_program) {
                    RemoteProgramsVirtualChannel& rpvc = this->get_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
                    rpvc.set_session_probe_virtual_channel(this->session_probe_virtual_channel_p);
                    rpvc.set_session_probe_launcher(this->session_probe_launcher.get());
                    this->session_probe_launcher->set_remote_programs_virtual_channel(&rpvc);
                }
            }
            else // this->channels.this->session_probe_launcher)
            {
                SessionProbeVirtualChannel& spvc = this->get_session_probe_virtual_channel(front, stc, asynchronous_tasks, 
                                                                                           session_reactor, 
                                                                                           mod_rdp, 
                                                                                           rdp, 
                                                                                           lang, 
                                                                                           bogus_refresh_rect, 
                                                                                           allow_using_multiple_monitors, 
                                                                                           monitor_count, 
                                                                                           client_general_caps, 
                                                                                           client_name);
                this->session_probe_virtual_channel_p = &spvc;
                if (!this->session_probe_start_launch_timeout_timer_only_after_logon) {
                    spvc.start_launch_timeout_timer();
                }
                if (this->remote_program) {
                    RemoteProgramsVirtualChannel& rpvc =
                        this->get_remote_programs_virtual_channel(front, stc, vars, client_rail_caps);
                    rpvc.set_session_probe_virtual_channel(this->session_probe_virtual_channel_p);

                }
            }
        }
    
    
    } channels;

    /// shared with RdpNegociation
    //@{

    CryptContext decrypt {};

    RedirectionInfo & redir_info;

    const RdpLogonInfo logon_info;

    std::array<uint8_t, 28>& server_auto_reconnect_packet_ref;
    //@}

    std::string target_host;

    const bool allow_using_multiple_monitors; // TODO duplicate monitor_count ?
    const uint32_t monitor_count;

    Transport & trans;
    CryptContext encrypt {};
    RdpNegociationResult negociation_result;

    ServerTransportContext stc;


    bool remote_apps_not_enabled = false;

    FrontAPI& front;

    rdp_orders orders;

    int share_id;

    char client_name[128] = {};

    const int key_flags;
    int  last_key_flags_sent = 0;
    bool first_scancode = true;

    enum : uint8_t {
        EARLY,
        WAITING_SYNCHRONIZE,
        WAITING_CTL_COOPERATE,
        WAITING_GRANT_CONTROL_COOPERATE,
        WAITING_FONT_MAP,
        UP_AND_RUNNING
    } connection_finalization_state;

    Pointer cursors[32];

    Random& gen;

    const RDPVerbose verbose;
    const BmpCache::Verbose cache_verbose;

    AuthApi & authentifier;
    ReportMessageApi & report_message;

    std::string& close_box_extra_message_ref;

    const bool enable_fastpath;                    // choice of programmer
          bool enable_fastpath_client_input_event; // choice of programmer + capability of server
    const bool enable_fastpath_server_update;      // = choice of programmer
    const bool enable_glyph_cache;
    const bool session_probe_enable_launch_mask;
    const bool enable_mem3blt;
    const bool enable_new_pointer;
    const bool enable_persistent_disk_bitmap_cache;
    const bool enable_cache_waiting_list;
    const bool persist_bitmap_cache_on_disk;
    const bool enable_ninegrid_bitmap;

    bool delayed_start_capture = false;


    const std::chrono::milliseconds   remoteapp_bypass_legal_notice_delay;
    const std::chrono::milliseconds   remoteapp_bypass_legal_notice_timeout;

    const bool                        experimental_fix_input_event_sync;

    size_t recv_bmp_update;

    rdp_mppc_unified_dec mppc_dec;

    std::string * error_message;

    const bool                 disconnect_on_logon_user_change;
    const std::chrono::seconds open_session_timeout;

    SessionReactor& session_reactor;
    SessionReactor::GraphicFdPtr fd_event;

    SessionReactor::TimerPtr remoteapp_one_shot_bypass_window_lecalnotice;

    std::string end_session_reason;
    std::string end_session_message;

    bool enable_polygonsc;
    bool enable_polygoncb;
    bool enable_polyline;
    bool enable_ellipsesc;
    bool enable_ellipsecb;
    bool enable_multidstblt;
    bool enable_multiopaquerect;
    bool enable_multipatblt;
    bool enable_multiscrblt;

    //uint64_t total_data_received;

    bool deactivation_reactivation_in_progress = false;

    const bool bogus_refresh_rect;

    AsynchronousTaskContainer asynchronous_tasks;

    Translation::language_t lang;

    Font const & font;

    bool already_upped_and_running = false;

    bool input_event_disabled     = false;
    bool graphics_update_disabled = false;

    bool mcs_disconnect_provider_ultimatum_pdu_received = false;

    static constexpr std::array<uint32_t, BmpCache::MAXIMUM_NUMBER_OF_CACHES>
    BmpCacheRev2_Cache_NumEntries()
    { return std::array<uint32_t, BmpCache::MAXIMUM_NUMBER_OF_CACHES>{{ 120, 120, 2553, 0, 0 }}; }

    time_t beginning;
    bool   session_disconnection_logged = false;

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

    bool is_server_auto_reconnec_packet_received = false;

    bool server_redirection_packet_received = false;

    ModRdpVariables vars;

#ifndef __EMSCRIPTEN__
    RDPMetrics * metrics;
#endif

public:
    using Verbose = RDPVerbose;

    explicit mod_rdp(
        Transport & trans
      , SessionReactor& session_reactor
      , FrontAPI & front
      , const ClientInfo & info
      , RedirectionInfo & redir_info
      , Random & gen
      , TimeObj & timeobj
      , const ModRDPParams & mod_rdp_params
      , AuthApi & authentifier
      , ReportMessageApi & report_message
      , ModRdpVariables vars
      , [[maybe_unused]] RDPMetrics * metrics
    )
        : channels(mod_rdp_params, mod_rdp_params.verbose, report_message, gen, metrics)
        , redir_info(redir_info)
        , logon_info(info.hostname, mod_rdp_params.hide_client_name, mod_rdp_params.target_user)
        , server_auto_reconnect_packet_ref(mod_rdp_params.server_auto_reconnect_packet_ref)
        , target_host(mod_rdp_params.target_host)
        , allow_using_multiple_monitors(mod_rdp_params.allow_using_multiple_monitors)
        , monitor_count(info.cs_monitor.monitorCount)
        , trans(trans)
        , stc(trans, this->encrypt, this->negociation_result)
        , front(front)
        , orders( mod_rdp_params.target_host, mod_rdp_params.enable_persistent_disk_bitmap_cache
                , mod_rdp_params.persist_bitmap_cache_on_disk, mod_rdp_params.verbose
                , report_error_from_reporter(report_message))
        , share_id(0)
        , key_flags(mod_rdp_params.key_flags)
        , last_key_flags_sent(key_flags)
        , connection_finalization_state(EARLY)
        , gen(gen)
        , verbose(mod_rdp_params.verbose)
        , cache_verbose(mod_rdp_params.cache_verbose)
        , authentifier(authentifier)
        , report_message(report_message)
        , close_box_extra_message_ref(mod_rdp_params.close_box_extra_message_ref)
        , enable_fastpath(mod_rdp_params.enable_fastpath)
        , enable_fastpath_client_input_event(false)
        , enable_fastpath_server_update(mod_rdp_params.enable_fastpath)
        , enable_glyph_cache(mod_rdp_params.enable_glyph_cache)
        , session_probe_enable_launch_mask(mod_rdp_params.session_probe_enable_launch_mask)
        , enable_mem3blt(mod_rdp_params.enable_mem3blt)
        , enable_new_pointer(mod_rdp_params.enable_new_pointer)
        , enable_persistent_disk_bitmap_cache(mod_rdp_params.enable_persistent_disk_bitmap_cache)
        , enable_cache_waiting_list(mod_rdp_params.enable_cache_waiting_list)
        , persist_bitmap_cache_on_disk(mod_rdp_params.persist_bitmap_cache_on_disk)
        , enable_ninegrid_bitmap(mod_rdp_params.enable_ninegrid_bitmap)
        , remoteapp_bypass_legal_notice_delay(mod_rdp_params.remoteapp_bypass_legal_notice_delay)
        , remoteapp_bypass_legal_notice_timeout(mod_rdp_params.remoteapp_bypass_legal_notice_timeout)
        , experimental_fix_input_event_sync(mod_rdp_params.experimental_fix_input_event_sync)
        , recv_bmp_update(0)
        , error_message(mod_rdp_params.error_message)
        , disconnect_on_logon_user_change(mod_rdp_params.disconnect_on_logon_user_change)
        , open_session_timeout(mod_rdp_params.open_session_timeout)
        , session_reactor(session_reactor)
        , enable_polygonsc(false)
        , enable_polygoncb(false)
        , enable_polyline(false)
        , enable_ellipsesc(false)
        , enable_ellipsecb(false)
        , enable_multidstblt(false)
        , enable_multiopaquerect(false)
        , enable_multipatblt(false)
        , enable_multiscrblt(false)
        //, total_data_received(0)
        , bogus_refresh_rect(mod_rdp_params.bogus_refresh_rect)
        , asynchronous_tasks(session_reactor)
        , lang(mod_rdp_params.lang)
        , font(mod_rdp_params.font)
        , beginning(timeobj.get_time().tv_sec)
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
        , vars(vars)
        #ifndef __EMSCRIPTEN__
        , metrics(metrics)
        #endif
    {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "Creation of new mod 'RDP'");
            mod_rdp_params.log();
        }

        this->decrypt.encryptionMethod = 2; /* 128 bits */
        this->stc.encrypt.encryptionMethod = 2; /* 128 bits */

        this->configure_extra_orders(mod_rdp_params.extra_orders);

        snprintf(this->client_name, sizeof(this->client_name), "%s", info.hostname);

        char program[512] = {};
        char directory[512] = {};

        // TODO: to make init code clearer we would prefer to have to consecutive inits
        // - one for remote_program initialisation
        // - one for session probe initialisation
        
        // Something like:
        
        // if probe: init_session_probe(... session_reactor);
        // if remote_prog: init_remote_program(... lang, font, identifier, program, directory);

        // This could probably work like two consecutive filters
        // one to prepare part of the context, the other used to prepare the remaining context.
        // There should be a way to prepare some objects useful for the remaining work to do

        if (this->channels.remote_program) {
            if (this->channels.enable_session_probe) {
                this->channels.init_remote_program_with_session_probe(front, *this, mod_rdp_params, this->session_reactor, this->lang, this->font, this->authentifier);
            }
            else {
                this->channels.init_remote_program_without_session_probe(front, *this, mod_rdp_params, this->session_reactor, this->lang, this->font, this->authentifier);
            }
        }
        else { // ! this->remote_program
            if (this->channels.enable_session_probe) {
                this->channels.init_no_remote_program_with_session_probe(*this, info, mod_rdp_params, program, directory, this->session_reactor);
            } // ! this->enable_session_probe
            else  {
                this->channels.init_no_remote_program_no_session_probe(info, mod_rdp_params, program, directory);
            } // this->enable_session_probe
        } // this->remote_program

        this->negociation_result.front_width = safe_int(info.screen_info.width);
        this->negociation_result.front_height = safe_int(info.screen_info.height);

        this->init_negociate_event_(info, timeobj, mod_rdp_params, program, directory);

    }   // mod_rdp


    ~mod_rdp() override {
        if (this->channels.enable_session_probe) {
            const bool disable_input_event     = false;
            const bool disable_graphics_update = false;
            this->disable_input_event_and_graphics_update(disable_input_event, disable_graphics_update);
        }

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

        this->channels.remote_programs_session_manager.reset();

        if (!this->server_redirection_packet_received) {
            this->redir_info = RedirectionInfo();
        }
    }

private:




public:
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

    void rdp_input_scancode( long param1, long param2, long device_flags, long time, Keymap2 * /*keymap*/) override {
        if ((UP_AND_RUNNING == this->connection_finalization_state) 
            && !this->input_event_disabled) {

            if (this->first_scancode && !(device_flags & 0x8000)) {
                if (this->channels.enable_session_probe) {
                    auto & session_probe = this->channels.get_session_probe_virtual_channel(this->front, stc, 
                                            this->asynchronous_tasks, this->session_reactor,
                                            *this,*this, this->lang, this->bogus_refresh_rect, 
                                            this->allow_using_multiple_monitors, this->monitor_count, 
                                            this->client_general_caps, this->client_name);
                    if (!this->channels.session_probe_launcher->is_keyboard_sequences_started()
                        || session_probe.has_been_launched()) {
                        LOG(LOG_INFO, "mod_rdp::rdp_input_scancode: First Keyboard Event. Resend the Synchronize Event to server.");
                        this->first_scancode = false;
                        this->send_input(time, RDP_INPUT_SYNCHRONIZE, 0, this->last_key_flags_sent, 0);
                    }
                }
                else {
                        LOG(LOG_INFO, "mod_rdp::rdp_input_scancode: First Keyboard Event. Resend the Synchronize Event to server.");
                        this->first_scancode = false;
                        this->send_input(time, RDP_INPUT_SYNCHRONIZE, 0, this->last_key_flags_sent, 0);
                }
            }

            this->send_input(time, RDP_INPUT_SCANCODE, device_flags, param1, param2);

            IF_ENABLE_METRICS(key_pressed());

            if (this->channels.remote_programs_session_manager) {
                this->channels.remote_programs_session_manager->input_scancode(param1, param2, device_flags);
            }
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_UNICODE, flag, unicode, 0);
            IF_ENABLE_METRICS(key_pressed());
        }
    }

    void rdp_input_synchronize( uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        (void)time;
        (void)param2;
        if (UP_AND_RUNNING == this->connection_finalization_state) {
            this->send_input(0, RDP_INPUT_SYNCHRONIZE, device_flags, param1, 0);
        }
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * /*keymap*/) override {
        //if (!(MOUSE_FLAG_MOVE & device_flags)) {
        //    LOG(LOG_INFO, "rdp_input_mouse x=%d y=%d device_flags=%d", x, y, device_flags);
        //}
        if ((UP_AND_RUNNING == this->connection_finalization_state) &&
            !this->input_event_disabled) {
            this->send_input(0, RDP_INPUT_MOUSE, device_flags, x, y);

            if (device_flags & MOUSE_FLAG_MOVE) {
                IF_ENABLE_METRICS(mouse_move(x, y));
            }

            if (device_flags & MOUSE_FLAG_DOWN) {
                if (device_flags & MOUSE_FLAG_BUTTON2) {
                    IF_ENABLE_METRICS(right_click_pressed());
                } else if (device_flags & MOUSE_FLAG_BUTTON1) {
                    IF_ENABLE_METRICS(left_click_pressed());
                }
            }

            if (this->channels.remote_programs_session_manager) {
                this->channels.remote_programs_session_manager->input_mouse(device_flags, x, y);
            }
        }
    }

public:
    // TODO: move to channels (and also remains here as it is mod API)
    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name,
        InStream & chunk, size_t length, uint32_t flags
    ) override {
        if (bool(this->verbose & RDPVerbose::channels)) {
            LOG(LOG_INFO,
                "mod_rdp::send_to_mod_channel: front_channel_channel=\"%s\"",
                front_channel_name);
        }
        
        this->channels.send_to_mod_channel(front_channel_name, chunk, length, flags,
                    this->front, this->stc,
                    this->asynchronous_tasks,
                    this->client_general_caps,
                    this->vars,
                    this->client_rail_caps,
                    this->client_name
            );
    }

private:

public:
    // Method used by session to transmit sesman answer for auth_channel
    // TODO: move to channels
    void send_auth_channel_data(const char * string_data) override {
        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        StaticOutStream<65536> stream_data;
        uint32_t data_size = std::min(::strlen(string_data) + 1, stream_data.get_capacity());

        stream_data.out_copy_bytes(string_data, data_size);

        virtual_channel_pdu.send_to_server(this->stc, this->channels.auth_channel_chanid
                                          , stream_data.get_offset()
                                          , this->channels.auth_channel_flags
                                          , stream_data.get_data()
                                          , stream_data.get_offset());
    }

private:
    // TODO: move to channels (and also remains here as it is mod API)
    void send_checkout_channel_data(const char * string_data) override {
        CHANNELS::VirtualChannelPDU virtual_channel_pdu;

        StaticOutStream<65536> stream_data;

        uint32_t data_size = std::min(::strlen(string_data), stream_data.get_capacity());

        stream_data.out_uint16_le(1);           // Version
        stream_data.out_uint16_le(data_size);
        stream_data.out_copy_bytes(string_data, data_size);

        virtual_channel_pdu.send_to_server(this->stc, this->channels.checkout_channel_chanid
          , stream_data.get_offset()
          , this->channels.checkout_channel_flags
          , stream_data.get_data()
          , stream_data.get_offset());
    }

    template<class... WriterData>
    void send_data_request(uint16_t channelId, WriterData... writer_data) {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "send data request");
        }

        write_packets(
            this->stc.trans,
            writer_data...,
            [this, channelId](StreamSize<256>, OutStream & mcs_header, std::size_t packet_size) {
                MCS::SendDataRequest_Send mcs(
                    mcs_header, this->stc.negociation_result.userid,
                    channelId, 1, 3, packet_size, MCS::PER_ENCODING
                );
                (void)mcs;
            },
            X224::write_x224_dt_tpdu_fn{}
        );
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "send data request done");
        }
    }

    template<class... WriterData>
    void send_data_request_ex(uint16_t channelId, WriterData... writer_data) {
        this->send_data_request(
            channelId,
            writer_data...,
            SEC::write_sec_send_fn{0, this->stc.encrypt, this->stc.negociation_result.encryptionLevel}
        );
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

    void connected_fast_path(gdi::GraphicApi & drawable, array_view_u8 array)
    {
        InStream stream(array);
        IF_ENABLE_METRICS(server_main_channel_data(stream.in_remain()));

        FastPath::ServerUpdatePDU_Recv su(stream, this->decrypt, array.data());

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

            InStream fud(this->multifragment_update_data.get_bytes());

            InStream& stream = ((upd.fragmentation == FastPath::FASTPATH_FRAGMENT_SINGLE) ? upd.payload : fud);

            switch (static_cast<FastPath::UpdateType>(upd.updateCode)) {
            case FastPath::UpdateType::ORDERS:
                this->front.begin_update();
                this->orders.process_orders(
                    stream, true, drawable,
                    this->stc.negociation_result.front_width, this->stc.negociation_result.front_height);
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
                drawable.set_pointer(null_pointer());
                break;

            case FastPath::UpdateType::PTR_DEFAULT:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer default (Fast)");
                }
                drawable.set_pointer(system_normal_pointer());
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
//                 this->process_color_pointer_pdu(stream, drawable);
                this->process_new_pointer_pdu(BitsPerPixel{24}, stream, drawable);
                break;

            case FastPath::UpdateType::CACHED:
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer cached (Fast)");
                }
                this->process_cached_pointer_pdu(stream, drawable);
                break;

            case FastPath::UpdateType::POINTER:
            {
                if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                    LOG(LOG_INFO, "Process pointer new (Fast)");

                }
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

        // TODO Chech all data in the PDU is consumed
    }

    void connected_slow_path(time_t now, gdi::GraphicApi & drawable, InStream & stream)
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
            LOG(LOG_ERR, "mod::rdp::DisconnectProviderUltimatum: reason=%s [%u]", reason, mcs.reason);

            this->end_session_reason.clear();
            this->end_session_message.clear();

            if ((!this->channels.session_probe_virtual_channel_p
                || !this->channels.session_probe_virtual_channel_p->is_disconnection_reconnection_required())
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

                ArcsightLogInfo arc_info;
                arc_info.name = "SESSION_DISCONNECTION";
                arc_info.signatureID = ArcsightLogInfo::SESSION_DISCONNECTION;
                arc_info.ApplicationProtocol = "rdp";
                arc_info.endTime = long(seconds);

                this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO, "%s", info);
                }
                this->session_disconnection_logged = true;
            }
            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
        }


        MCS::SendDataIndication_Recv mcs(x224.payload, MCS::PER_ENCODING);
        SEC::Sec_Recv sec(mcs.payload, this->decrypt, this->stc.negociation_result.encryptionLevel);
        if (mcs.channelId != GCC::MCS_GLOBAL_CHANNEL){
            // TODO: this should move to channels
            if (bool(this->verbose & RDPVerbose::channels)) {
                LOG(LOG_INFO, "received channel data on mcs.chanid=%u", mcs.channelId);
            }

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

            // If channel name is our virtual channel, then don't send data to front
            if (mod_channel.name == this->channels.auth_channel && this->channels.enable_auth_channel) {
                this->channels.process_auth_event(mod_channel, sec.payload, length, flags, chunk_size, this->front, *this, this->stc, this->asynchronous_tasks, this->client_general_caps, this->client_name, this->authentifier);
            }
            else if (mod_channel.name == this->channels.checkout_channel) {
                this->channels.process_checkout_event(mod_channel, sec.payload, length, flags, chunk_size, this->authentifier);
            }
            else if (mod_channel.name == channel_names::sespro) {
                this->channels.process_session_probe_event(mod_channel, sec.payload, length, flags, chunk_size, 
                    this->front, *this, *this, this->stc, 
                    this->asynchronous_tasks, this->session_reactor, 
                    this->client_general_caps, this->client_name, 
                    this->allow_using_multiple_monitors, this->monitor_count, 
                    this->bogus_refresh_rect, this->lang);
            }
            
            // Clipboard is a Clipboard PDU
            else if (mod_channel.name == channel_names::cliprdr) {
                IF_ENABLE_METRICS(set_server_cliprdr_metrics(sec.payload.clone(), length, flags));
                this->channels.process_cliprdr_event(sec.payload, length, flags, chunk_size, this->front, this->stc);
            }
            else if (mod_channel.name == channel_names::rail) {
                IF_ENABLE_METRICS(server_rail_channel_data(length));
                this->channels.process_rail_event(mod_channel, sec.payload, length, flags, chunk_size, 
                            this->front, this->stc, 
                            this->vars, this->client_rail_caps);
            }
            else if (mod_channel.name == channel_names::rdpdr) {
                IF_ENABLE_METRICS(set_server_rdpdr_metrics(sec.payload.clone(), length, flags));
                this->channels.process_rdpdr_event(sec.payload, length, flags, chunk_size, this->front, this->stc, this->asynchronous_tasks, this->client_general_caps, this->client_name);
            }
            else if (mod_channel.name == channel_names::drdynvc) {
                IF_ENABLE_METRICS(server_other_channel_data(length));
                this->channels.process_drdynvc_event(sec.payload, length, flags, chunk_size, this->front, this->stc, this->asynchronous_tasks);
            }
            else {
                IF_ENABLE_METRICS(server_other_channel_data(length));
                this->channels.process_unknown_channel_event(mod_channel, sec.payload, length, flags, chunk_size, this->front);
            }

            sec.payload.in_skip_bytes(sec.payload.in_remain());

        }
        else {

            uint8_t const * next_packet = sec.payload.get_current();
            while (next_packet < sec.payload.get_data_end()) {
                sec.payload.rewind();
                sec.payload.in_skip_bytes(next_packet - sec.payload.get_data());

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

                                    if (this->monitor_count &&
                                        (monitor_layout_pdu.get_monitorCount() !=
                                         this->monitor_count)) {

                                        LOG(LOG_ERR, "Server do not support the display monitor layout of the client");
                                        throw Error(ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT);
                                    }
                                }
                                else {
                                    LOG(LOG_INFO, "Resizing to %ux%ux%u", this->stc.negociation_result.front_width, this->stc.negociation_result.front_height, this->orders.bpp);

                                    if (FrontAPI::ResizeResult::fail == this->front.server_resize(this->stc.negociation_result.front_width, this->stc.negociation_result.front_height, this->orders.bpp)){
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

                                ArcsightLogInfo arc_info;
                                arc_info.name = "SESSION_ESTABLISHED";
                                arc_info.signatureID = ArcsightLogInfo::SESSION_ESTABLISHED;
                                arc_info.ApplicationProtocol = "rdp";
                                arc_info.WallixBastionStatus = "SUCCESS";
                                arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                                this->report_message.log6("type=\"SESSION_ESTABLISHED_SUCCESSFULLY\"", arc_info, this->session_reactor.get_current_time());
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
                                if (this->channels.enable_session_probe) {
                                    this->channels.do_enable_session_probe(
                                                this->front,
                                                this->stc,
                                                *this,
                                                *this,
                                                this->asynchronous_tasks,
                                                this->session_reactor,
                                                this->client_general_caps,
                                                this->vars,
                                                this->client_rail_caps,
                                                this->client_name,
                                                this->allow_using_multiple_monitors,
                                                this->monitor_count,
                                                this->bogus_refresh_rect,
                                                this->lang);
                                }
                                this->already_upped_and_running = true;
                            }

                            if (this->channels.enable_session_probe 
                            &&  this->session_probe_enable_launch_mask) {
                                this->delayed_start_capture = true;

                                LOG(LOG_INFO, "Mod_rdp: Capture starting is delayed.");
                            }
                            else if (this->front.can_be_start_capture()) {
                                if (this->bogus_refresh_rect
                                 && this->allow_using_multiple_monitors
                                 && this->monitor_count > 1
                                ) {
                                    this->rdp_suppress_display_updates();
                                    this->rdp_allow_display_updates(0, 0, this->stc.negociation_result.front_width, this->stc.negociation_result.front_height);
                                }
                                this->rdp_input_invalidate(Rect(0, 0, this->stc.negociation_result.front_width, this->stc.negociation_result.front_height));
                            }
                            break;
                        case UP_AND_RUNNING:
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
                                                drawable, this->stc.negociation_result.front_width, this->stc.negociation_result.front_height);
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
                            if (this->stc.negociation_result.use_rdp5){
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

    void draw_event(time_t now, gdi::GraphicApi & gd) override
    {
        //LOG(LOG_INFO, "mod_rdp::draw_event()");

        if (this->channels.remote_programs_session_manager) {
            this->channels.remote_programs_session_manager->set_drawable(&gd);
        }

        this->buf.load_data(this->trans);
        draw_event_impl(now, gd);
    }


    void draw_event_impl(time_t now, gdi::GraphicApi & gd)
    {
        while (this->buf.next_pdu()) {
            InStream x224_data(this->buf.current_pdu_buffer());

            try{
                gdi::GraphicApi & drawable =
                    ( this->channels.remote_programs_session_manager
                    ? (*this->channels.remote_programs_session_manager)
                    : ( this->graphics_update_disabled
                        ? gdi::null_gd()
                        : gd
                    ));
                if (this->buf.current_pdu_is_fast_path()) {
                    this->connected_fast_path(drawable, this->buf.current_pdu_buffer());
                }
                else {
                    this->connected_slow_path(now, drawable, x224_data);
                }
            }
            catch(Error const & e){
                LOG(LOG_INFO, "mod_rdp::draw_event() state switch raised exception");

                if (e.id == ERR_RDP_SERVER_REDIR) {
                    this->front.must_be_stop_capture();
                    throw;
                }

                if (this->channels.session_probe_virtual_channel_p &&
                    this->channels.session_probe_virtual_channel_p->is_disconnection_reconnection_required()) {
                    throw Error(ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION);
                }
                this->front.must_be_stop_capture();

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

                if (this->mcs_disconnect_provider_ultimatum_pdu_received) {
                    StaticOutStream<256> stream;
                    X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
                    try {
                        this->stc.trans.send(stream.get_bytes());
                        LOG(LOG_INFO, "Connection to server closed");
                    }
                    catch(Error const & e){
                        LOG(LOG_INFO, "Connection to server Already closed: error=%u", e.id);
                    };
                }

                this->session_reactor.set_next_event(BACK_EVENT_NEXT);

                if (this->channels.enable_session_probe) {
                    const bool disable_input_event     = false;
                    const bool disable_graphics_update = false;
                    this->disable_input_event_and_graphics_update(
                        disable_input_event, disable_graphics_update);
                }

                if ((e.id == ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT) ||
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
            }
        }
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
                    this->stc.negociation_result.use_rdp5
                    ? NO_BITMAP_COMPRESSION_HDR | AUTORECONNECT_SUPPORTED | LONG_CREDENTIALS_SUPPORTED
                    : 0
                    ;
                // Slow/Fast-path
                general_caps.extraflags |=
                    this->enable_fastpath_server_update
                    ? FASTPATH_OUTPUT_SUPPORTED
                    : 0
                    ;
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    general_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(general_caps);

                BitmapCaps bitmap_caps;
                // TODO Client SHOULD set this field to the color depth requested in the Client Core Data
                bitmap_caps.preferredBitsPerPixel = safe_int(this->orders.bpp);
                //bitmap_caps.preferredBitsPerPixel = this->front_bpp;
                bitmap_caps.desktopWidth          = this->stc.negociation_result.front_width;
                bitmap_caps.desktopHeight         = this->stc.negociation_result.front_height;
                bitmap_caps.bitmapCompressionFlag = 0x0001; // This field MUST be set to TRUE (0x0001).
                //bitmap_caps.drawingFlags = DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY | DRAW_ALLOW_COLOR_SUBSAMPLING | DRAW_ALLOW_SKIP_ALPHA;
                bitmap_caps.drawingFlags = DRAW_ALLOW_SKIP_ALPHA;
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
                if (bool(this->verbose & RDPVerbose::capabilities)) {
                    order_caps.log("Sending to server");
                }
                confirm_active_pdu.emit_capability_set(order_caps);

                BmpCacheCaps bmpcache_caps;
                bmpcache_caps.cache0Entries         = 0x258;
                bmpcache_caps.cache0MaximumCellSize = nb_bytes_per_pixel(this->orders.bpp) * 0x100;
                bmpcache_caps.cache1Entries         = 0x12c;
                bmpcache_caps.cache1MaximumCellSize = nb_bytes_per_pixel(this->orders.bpp) * 0x400;
                bmpcache_caps.cache2Entries         = 0x106;
                bmpcache_caps.cache2MaximumCellSize = nb_bytes_per_pixel(this->orders.bpp) * 0x1000;

                BmpCache2Caps bmpcache2_caps;
                bmpcache2_caps.cacheFlags           = PERSISTENT_KEYS_EXPECTED_FLAG | (this->enable_cache_waiting_list ? ALLOW_CACHE_WAITING_LIST_FLAG : 0);
                bmpcache2_caps.numCellCaches        = 3;
                bmpcache2_caps.bitmapCache0CellInfo = this->BmpCacheRev2_Cache_NumEntries()[0];
                bmpcache2_caps.bitmapCache1CellInfo = this->BmpCacheRev2_Cache_NumEntries()[1];
                bmpcache2_caps.bitmapCache2CellInfo = (this->BmpCacheRev2_Cache_NumEntries()[2] | 0x80000000);

                bool use_bitmapcache_rev2 = this->enable_persistent_disk_bitmap_cache;

                if (use_bitmapcache_rev2) {
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bmpcache2_caps.log("Sending to server");
                    }
                    confirm_active_pdu.emit_capability_set(bmpcache2_caps);

                    if (!this->deactivation_reactivation_in_progress) {
                        this->orders.create_cache_bitmap(
                            this->BmpCacheRev2_Cache_NumEntries()[0], nb_bytes_per_pixel(this->orders.bpp) * 16 * 16, false,
                            this->BmpCacheRev2_Cache_NumEntries()[1], nb_bytes_per_pixel(this->orders.bpp) * 32 * 32, false,
                            this->BmpCacheRev2_Cache_NumEntries()[2], nb_bytes_per_pixel(this->orders.bpp) * 64 * 64, this->enable_persistent_disk_bitmap_cache,
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
                            0x258, nb_bytes_per_pixel(this->orders.bpp) * 0x100,  false,
                            0x12c, nb_bytes_per_pixel(this->orders.bpp) * 0x400,  false,
                            0x106, nb_bytes_per_pixel(this->orders.bpp) * 0x1000, false,
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
                if (!this->enable_new_pointer) {
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

                if (this->channels.remote_program) {
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
                    this->stc.negociation_result.userid + GCC::MCS_USERCHANNEL_BASE, packet_size);
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
//             this->process_color_pointer_pdu(stream, drawable);
            this->process_new_pointer_pdu(BitsPerPixel{24}, stream, drawable);
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
                BitsPerPixel data_bpp = checked_int{stream.in_uint16_le()}; /* data bpp */
                this->process_new_pointer_pdu(data_bpp, stream, drawable);
            }
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer new done");
            }
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
            if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
                LOG(LOG_INFO, "Process pointer system::%s",
                    (system_pointer_type == RDP_NULL_POINTER)?
                    "RDP_NULL_POINTER":"RDP_DEFAULT_POINTER");
            }
            if (system_pointer_type == RDP_NULL_POINTER) {
                drawable.set_pointer(null_pointer());
            }
            else {
                drawable.set_pointer(normal_pointer());
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
            #undef CASE
            default:
                return "?";
        }
    }   // get_error_info_name

    uint32_t get_error_info_from_pdu(InStream & stream) {
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
            ((0 != ::strcasecmp(domain, this->logon_info.domain())
             || 0 != ::strcasecmp(username, this->logon_info.username())) &&
             (this->logon_info.domain()[0] ||
              (0 != ::strcasecmp(domain_username_format_0, this->logon_info.username()) &&
               0 != ::strcasecmp(domain_username_format_1, this->logon_info.username()) &&
               0 != ::strcasecmp(username, this->logon_info.username()))))) {
            if (this->error_message) {
                *this->error_message = "Unauthorized logon user change detected!";
            }

            this->end_session_reason  = "OPEN_SESSION_FAILED";
            this->end_session_message = "Unauthorized logon user change detected.";

            LOG(LOG_ERR,
                "Unauthorized logon user change detected on %s (%s%s%s) -> (%s%s%s). "
                    "The session will be disconnected.",
                this->logon_info.hostname(), this->logon_info.domain(),
                (*this->logon_info.domain() ? "\\" : ""),
                this->logon_info.username(), domain,
                ((domain && *domain) ? "\\" : ""),
                username);
            throw Error(ERR_RDP_LOGON_USER_CHANGED);
        }

        if (this->channels.session_probe_virtual_channel_p &&
            this->channels.session_probe_start_launch_timeout_timer_only_after_logon) {
            this->channels.session_probe_virtual_channel_p->start_launch_timeout_timer();
        }

        this->report_message.report("OPEN_SESSION_SUCCESSFUL", "OK.");
        this->end_session_reason = "CLOSE_SESSION_SUCCESSFUL";
        this->end_session_message = "OK.";

        this->fd_event->disable_timeout();

        if (this->channels.enable_session_probe) {
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

            process_logon_info(char_ptr_cast(liv1.Domain),
                char_ptr_cast(liv1.UserName));

            this->front.send_savesessioninfo();

            this->remoteapp_one_shot_bypass_window_lecalnotice.reset();
        }
        break;
        case RDP::INFOTYPE_LOGON_LONG:
        {
            LOG(LOG_INFO, "process save session info : Logon long");
            RDP::LogonInfoVersion2_Recv liv2(ssipdudata.payload);

            process_logon_info(char_ptr_cast(liv2.Domain),
                char_ptr_cast(liv2.UserName));

            this->front.send_savesessioninfo();

            this->remoteapp_one_shot_bypass_window_lecalnotice.reset();
        }
        break;
        case RDP::INFOTYPE_LOGON_PLAINNOTIFY:
        {
            LOG(LOG_INFO, "process save session info : Logon plainnotify");
            RDP::PlainNotify_Recv pn(ssipdudata.payload);

            if (this->channels.enable_session_probe) {
                const bool disable_input_event     = true;
                const bool disable_graphics_update = this->session_probe_enable_launch_mask;
                this->disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);
            }

            if (this->channels.session_probe_virtual_channel_p &&
                this->channels.session_probe_start_launch_timeout_timer_only_after_logon) {
                this->channels.session_probe_virtual_channel_p->start_launch_timeout_timer();
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

                this->remoteapp_one_shot_bypass_window_lecalnotice.reset();
            }

            if (lie.FieldsPresent & RDP::LOGON_EX_LOGONERRORS) {
                LOG(LOG_INFO, "process save session info : Logon Errors Info");

                RDP::LogonErrorsInfo_Recv lei(lif.payload);

                if ((RDP::LOGON_MSG_SESSION_CONTINUE != lei.ErrorNotificationType) &&
                    (RDP::LOGON_WARNING >= lei.ErrorNotificationData) &&
                    this->channels.remote_program) {
                    if ((0 != lei.ErrorNotificationType) ||
                        (RDP::LOGON_FAILED_OTHER != lei.ErrorNotificationData) ||
                        (!this->remoteapp_bypass_legal_notice_delay.count())) {
                            this->on_remoteapp_redirect_user_screen(this->authentifier, lei.ErrorNotificationData);
                    }
                    else {
                        this->remoteapp_one_shot_bypass_window_lecalnotice = this->session_reactor.create_timer()
                            .on_action(jln::sequencer(
                                    [this](JLN_TIMER_CTX ctx) {
                                        LOG(LOG_INFO, "RDP::process_save_session_info: One-shut bypass Windows's Legal Notice");
                                        this->send_input(0, RDP_INPUT_SCANCODE, 0x0, 0x1C, 0x0);
                                        this->send_input(0, RDP_INPUT_SCANCODE, 0x8000, 0x1C, 0x0);

                                        if (this->remoteapp_bypass_legal_notice_timeout.count()) {
                                            ctx.set_delay(this->remoteapp_bypass_legal_notice_timeout);

                                            return ctx.next();
                                        }
                                        return ctx.terminate();
                                    },
                                    [this](JLN_TIMER_CTX ctx) {
                                        this->on_remoteapp_redirect_user_screen(this->authentifier, RDP::LOGON_FAILED_OTHER);

                                        return ctx.terminate();
                                    }
                                ))
                            .set_delay(this->remoteapp_bypass_legal_notice_delay);
                    }
                }
                else if (RDP::LOGON_MSG_SESSION_CONTINUE == lei.ErrorNotificationType) {
                    this->remoteapp_one_shot_bypass_window_lecalnotice.reset();
                }
            }
        }
        break;
        }

        stream.in_skip_bytes(stream.in_remain());
    }

    [[noreturn]]
    static void on_remoteapp_redirect_user_screen(AuthApi& authentifier, uint32_t ErrorNotificationData) {
        LOG(LOG_ERR, "Can not redirect user's focus to the WinLogon screen in RemoteApp mode!");

        std::string errmsg = "(RemoteApp) ";

        errmsg += RDP::LogonErrorsInfo_Recv::ErrorNotificationDataToShortMessage(ErrorNotificationData);
        authentifier.set_auth_error_message(errmsg.c_str());
        throw Error(ERR_RAIL_LOGON_FAILED_OR_WARNING);
    }

    // TODO CGR: this can probably be unified with process_confirm_active in front
    void process_server_caps(InStream & stream, uint16_t len) {
        // TODO check stream consumed and len
        (void)len;
        if (bool(this->verbose & RDPVerbose::capabilities)){
            LOG(LOG_INFO, "mod_rdp::process_server_caps");
        }

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
                }
                break;
            case CAPSTYPE_BITMAP:
                {
                    BitmapCaps bitmap_caps;
                    bitmap_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        bitmap_caps.log("Received from server");
                    }
                    this->orders.bpp = checked_int(bitmap_caps.preferredBitsPerPixel);
                    this->negociation_result.front_width = bitmap_caps.desktopWidth;
                    this->negociation_result.front_height = bitmap_caps.desktopHeight;
                }
                break;
            case CAPSTYPE_ORDER:
                {
                    OrderCaps order_caps;
                    order_caps.recv(stream, capset_length);
                    if (bool(this->verbose & RDPVerbose::capabilities)) {
                        order_caps.log("Received from server");
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
                    BitmapCodecCaps caps(false);
                    LOG(LOG_INFO, "Dumping CAPSETTYPE_BITMAP_CODECS");
                    hexdump_d(stream.get_current()-4, capset_length);
                    stream.in_skip_bytes(capset_length-4);
//                    caps.recv(stream, capset_length);
//                    if (bool(this->verbose & RDPVerbose::capabilities)) {
//                        caps.log("Receiving from server");
//                    }
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
                ShareControl_Send(sctrl_header, PDUTYPE_DATAPDU, this->stc.negociation_result.userid + GCC::MCS_USERCHANNEL_BASE, packet_size);

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
                    this->stc.negociation_result.userid + GCC::MCS_USERCHANNEL_BASE, packet_size
                );
            }
        );
    }

    void send_persistent_key_list() {
        if (bool(this->verbose & RDPVerbose::basic_trace)) {
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list");
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
            LOG(LOG_INFO, "mod_rdp::send_persistent_key_list done");
        }
    }   // send_persistent_key_list

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

                IF_ENABLE_METRICS(client_main_channel_data(stream.tailroom()));
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
            this->stc.trans,
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
                IF_ENABLE_METRICS(client_main_channel_data(stream.tailroom()));
            },
            [&](StreamSize<256>, OutStream & fastpath_header, uint8_t * packet_data, std::size_t packet_size) {
                FastPath::ClientInputEventPDU_Send out_cie(
                    fastpath_header, packet_data, packet_size, 1,
                    this->stc.encrypt, this->stc.negociation_result.encryptionLevel, this->stc.negociation_result.encryptionMethod
                );
                (void)out_cie;
            }
        );

        if (bool(this->verbose & RDPVerbose::input)) {
            LOG(LOG_INFO, "mod_rdp::send_input_fastpath done");
        }
    }

    void send_input(int time, int message_type, int device_flags, int param1, int param2) override {
        if (!this->enable_fastpath_client_input_event) {
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
                                          this->stc.negociation_result.userid,
                                          this->stc.negociation_result.encryptionLevel,
                                          this->stc.encrypt);

                rrpdu.addInclusiveRect(r.x, r.y, r.x + r.cx - 1, r.y + r.cy - 1);

                rrpdu.emit(this->stc.trans);
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
        if ((UP_AND_RUNNING == this->connection_finalization_state) && !vr.empty()) {
            RDP::RefreshRectPDU rrpdu(this->share_id,
                                      this->stc.negociation_result.userid,
                                      this->stc.negociation_result.encryptionLevel,
                                      this->stc.encrypt);
            for (Rect const & rect : vr) {
                if (!rect.isempty()){
                    rrpdu.addInclusiveRect(rect.x, rect.y, rect.x + rect.cx - 1, rect.y + rect.cy - 1);
                }
            }
            rrpdu.emit(this->stc.trans);
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

    void set_last_tram_len([[maybe_unused]] size_t tram_length) override {
        IF_ENABLE_METRICS(client_main_channel_data(tram_length));
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
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO, "mod_rdp::process_new_pointer_pdu");
        }

//         InStream stream_to_log = stream.clone();
//           ::hexdump(stream.get_data(), stream.in_remain());

        unsigned pointer_idx = stream.in_uint16_le();
        if (bool(this->verbose & RDPVerbose::graphics_pointer)) {
            LOG(LOG_INFO,
                "mod_rdp::process_new_pointer_pdu xorBpp=%u pointer_idx=%u",
                data_bpp, pointer_idx);
        }

        if (pointer_idx >= (sizeof(this->cursors) / sizeof(this->cursors[0]))) {
            LOG(LOG_ERR,
                "mod_rdp::process_new_pointer_pdu pointer cache idx overflow (%u)",
                pointer_idx);
            throw Error(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK);
        }

        Pointer cursor = pointer_loader_new(data_bpp, stream, this->orders.global_palette, this->clean_up_32_bpp_cursor);

        this->cursors[pointer_idx] = cursor;
        drawable.set_pointer(cursor);
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
//         // The TS_UPDATE_BITMAP_DATA structure encapsulates the bitmap data that
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

            // TODO : verify code below, why is there no check at all on BITMAP_COMPRESSION_NO_HDR flag ?
            // CGR: both flags seems to be redundant. Was there an old version of RDP
            // where compression header was present but compresion not enabled ?
            // That's the only potential use I see for this flag

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
        if (bool(this->verbose & RDPVerbose::graphics)){
            LOG(LOG_INFO, "mod_rdp::process_bitmap_updates done");
        }
    }   // process_bitmap_updates

public:
    bool is_up_and_running() const override {
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

            ArcsightLogInfo arc_info;
            arc_info.name = "SESSION_DISCONNECTION";
            arc_info.signatureID = ArcsightLogInfo::SESSION_DISCONNECTION;
            arc_info.ApplicationProtocol = "rdp";
            arc_info.endTime = long(seconds);


            this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

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

        if (!this->mcs_disconnect_provider_ultimatum_pdu_received) {
            write_packets(
                this->stc.trans,
                [](StreamSize<256>, OutStream & mcs_data) {
                    MCS::DisconnectProviderUltimatum_Send(mcs_data, 3, MCS::PER_ENCODING);
                },
                X224::write_x224_dt_tpdu_fn{}
            );
        }
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

        if (this->channels.remote_programs_session_manager) {
            this->channels.remote_programs_session_manager->disable_graphics_update(
                disable_graphics_update);
        }

        return need_full_screen_update;
    }

public:
    windowing_api* get_windowing_api() const {
        if (this->channels.remote_programs_session_manager) {
            return this->channels.remote_programs_session_manager.get();
        }

        return nullptr;
    }

    Dimension get_dim() const override
    { return Dimension(this->stc.negociation_result.front_width, this->stc.negociation_result.front_height); }

    bool is_auto_reconnectable() override {
        return (this->is_server_auto_reconnec_packet_received &&
            this->is_up_and_running() &&
            (!this->channels.session_probe_launcher || this->channels.session_probe_launcher->is_stopped()));
    }

    void auth_rail_exec(uint16_t flags, const char* original_exe_or_file,
            const char* exe_or_file, const char* working_dir,
            const char* arguments, const char* account, const char* password) override {
        if (this->channels.remote_program) {
            RemoteProgramsVirtualChannel& rpvc =
                this->channels.get_remote_programs_virtual_channel(this->front, this->stc, this->vars, this->client_rail_caps);

            rpvc.auth_rail_exec(flags, original_exe_or_file, exe_or_file,
                working_dir, arguments, account, password);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::auth_rail_exec(): Current session has no Remote Program Virtual Channel");
        }
    }

    // TODO: this should move to channels, but we need a jump relay as it comes through mod_api
    void auth_rail_exec_cancel(uint16_t flags, const char* original_exe_or_file,
            uint16_t exec_result) override {
        if (this->channels.remote_program) {
            RemoteProgramsVirtualChannel& rpvc =
                this->channels.get_remote_programs_virtual_channel(this->front, this->stc, this->vars, this->client_rail_caps);

            rpvc.auth_rail_exec_cancel(flags, original_exe_or_file, exec_result);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::auth_rail_exec(): Current session has no Remote Program Virtual Channel");
        }
    }

    void sespro_rail_exec_result(uint16_t flags, const char* exe_or_file,
        uint16_t exec_result, uint32_t raw_result) override {
        if (this->channels.remote_program) {
            RemoteProgramsVirtualChannel& rpvc =
                this->channels.get_remote_programs_virtual_channel(this->front, this->stc, this->vars, this->client_rail_caps);

            rpvc.sespro_rail_exec_result(flags, exe_or_file, exec_result, raw_result);
        }
        else {
            LOG(LOG_WARNING, "mod_rdp::sespro_rail_exec_result(): Current session has no Remote Program Virtual Channel");
        }
    }

    void sespro_ending_in_progress() override
    {
        this->end_session_reason.clear();
        this->end_session_message.clear();

        this->authentifier.disconnect_target();
        this->authentifier.set_auth_error_message(TR(trkeys::session_logoff_in_progress, this->lang));

        this->session_reactor.set_event_next(BACK_EVENT_NEXT);
    }

    void sespro_launch_process_ended() override {
        if (this->delayed_start_capture) {
            this->delayed_start_capture = false;

            if (this->front.can_be_start_capture()) {
                if (this->bogus_refresh_rect
                 && this->allow_using_multiple_monitors
                 && this->monitor_count > 1
                ) {
                    this->rdp_suppress_display_updates();
                    this->rdp_allow_display_updates(0, 0, this->stc.negociation_result.front_width, this->stc.negociation_result.front_height);
                }
                this->rdp_input_invalidate(Rect(0, 0, this->stc.negociation_result.front_width, this->stc.negociation_result.front_height));
            }
        }
    }

private:
    void init_negociate_event_(
        const ClientInfo & info, TimeObj & timeobj, const ModRDPParams & mod_rdp_params,
        char const* program, char const* directory);
};

#undef IF_ENABLE_METRICS
