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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "configs/config.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/rail_session_manager.hpp"
#include "mod/rdp/channels/sespro_channel.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "utils/log.hpp"

class FrontAPI;

class RemoteProgramsVirtualChannel final : public BaseVirtualChannel,
    public rdp_api
{
private:
    uint16_t client_order_type = 0;
    uint16_t server_order_type = 0;

       uint16_t param_client_execute_flags;
    std::string param_client_execute_exe_or_file;
    std::string param_client_execute_working_dir;
    std::string param_client_execute_arguments;

       uint16_t param_client_execute_flags_2;
    std::string param_client_execute_exe_or_file_2;
    std::string param_client_execute_working_dir_2;
    std::string param_client_execute_arguments_2;

    RemoteProgramsSessionManager * param_rail_session_manager = nullptr;

    bool param_should_ignore_first_client_execute;

    bool param_use_session_probe_to_launch_remote_program;

    //bool param_client_supports_handshakeex_pdu;
    //bool param_client_supports_enhanced_remoteapp;

    bool first_client_execute_ignored = false;

    bool client_execute_pdu_sent = false;

    SessionProbeVirtualChannel * session_probe_channel = nullptr;

    SessionProbeLauncher* session_probe_stop_launch_sequence_notifier = nullptr;

    bool exe_or_file_exec_ok = false;
    bool session_probe_launch_confirmed = false;

    bool exe_or_file_2_sent = false;

    ModRdpVariables vars;

    class LaunchPendingApp {
        std::string original_exe_or_file;
        std::string exe_or_file;
        uint16_t    flags;

    public:
        explicit LaunchPendingApp(const char* original_exe_or_file_,
                                  const char* exe_or_file_, uint16_t flags_)
        : original_exe_or_file(original_exe_or_file_)
        , exe_or_file(exe_or_file_)
        , flags(flags_)
        {}

        const char* OriginalExeOrFile() const {
            return this->original_exe_or_file.c_str();
        }

        const char* ExeOrFile() const {
            return this->exe_or_file.c_str();
        }

        uint16_t Flags() const {
            return this->flags;
        }
    };

    std::vector<LaunchPendingApp> launch_pending_apps;

public:
    struct Params : public BaseVirtualChannel::Params
    {
        uninit_checked<   uint16_t> client_execute_flags;
        uninit_checked<const char*> client_execute_exe_or_file;
        uninit_checked<const char*> client_execute_working_dir;
        uninit_checked<const char*> client_execute_arguments;

        uninit_checked<   uint16_t> client_execute_flags_2;
        uninit_checked<const char*> client_execute_exe_or_file_2;
        uninit_checked<const char*> client_execute_working_dir_2;
        uninit_checked<const char*> client_execute_arguments_2;

        uninit_checked<RemoteProgramsSessionManager*> rail_session_manager;

        uninit_checked<bool> should_ignore_first_client_execute;

        uninit_checked<bool> use_session_probe_to_launch_remote_program;

        uninit_checked<bool> client_supports_handshakeex_pdu;
        uninit_checked<bool> client_supports_enhanced_remoteapp;

        explicit Params(ReportMessageApi & report_message)
          : BaseVirtualChannel::Params(report_message)
        {}
    };

    RemoteProgramsVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& /*front*/,
        ModRdpVariables vars,
        const Params& params)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         params)
    , param_client_execute_flags(params.client_execute_flags)
    , param_client_execute_exe_or_file(params.client_execute_exe_or_file)
    , param_client_execute_working_dir(params.client_execute_working_dir)
    , param_client_execute_arguments(params.client_execute_arguments)
    , param_client_execute_flags_2(params.client_execute_flags_2)
    , param_client_execute_exe_or_file_2(params.client_execute_exe_or_file_2)
    , param_client_execute_working_dir_2(params.client_execute_working_dir_2)
    , param_client_execute_arguments_2(params.client_execute_arguments_2)
    , param_rail_session_manager(params.rail_session_manager)
    , param_should_ignore_first_client_execute(params.should_ignore_first_client_execute)
    , param_use_session_probe_to_launch_remote_program(params.use_session_probe_to_launch_remote_program)
    //, param_client_supports_handshakeex_pdu(params.client_supports_handshakeex_pdu)
    //, param_client_supports_enhanced_remoteapp(params.client_supports_enhanced_remoteapp)
    , vars(vars) {}

protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "RAIL_LIMIT";
    }

private:
    template<class PDU>
    bool process_client_windowing_pdu(uint32_t flags, InStream& chunk) {
        PDU pdu;

        pdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            pdu.log(LOG_INFO);
        }

        if (this->param_rail_session_manager->is_client_only_window(pdu.WindowId())) {
            return false;
        }

        if (pdu.map_window_id(*this->param_rail_session_manager)) {
            StaticOutStream<65536> out_s;
            RAILPDUHeader          header;

            header.emit_begin(out_s, PDU::orderType());
            pdu.emit(out_s);
            header.emit_end();

            this->send_message_to_server(out_s.get_offset(), flags, out_s.get_data(),
                out_s.get_offset());

            return false;
        }

        return true;
    }

    template<class PDU>
    bool process_server_windowing_pdu(uint32_t flags, InStream& chunk) {
        PDU pdu;

        pdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            pdu.log(LOG_INFO);
        }

        if (this->param_rail_session_manager->is_server_only_window(pdu.WindowId())) {
            return false;
        }

        if (pdu.map_window_id(*this->param_rail_session_manager)) {
            StaticOutStream<65536> out_s;
            RAILPDUHeader          header;

            header.emit_begin(out_s, PDU::orderType());
            pdu.emit(out_s);
            header.emit_end();

            this->send_message_to_client(out_s.get_offset(), flags, out_s.get_data(),
                out_s.get_offset());

            return false;
        }

        return true;
    }

    bool process_client_activate_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_activate_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_client_windowing_pdu<ClientActivatePDU>(flags, chunk);
    }

    bool process_client_compartment_status_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_compartment_statusinformation_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        CompartmentStatusInformationPDU csipdu;

        csipdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            csipdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_client_execute_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_execute_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientExecutePDU cepdu;

        cepdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            cepdu.log(LOG_INFO);
        }

        if (this->param_should_ignore_first_client_execute &&
            !this->first_client_execute_ignored) {
            this->first_client_execute_ignored = true;

            if (bool(this->verbose & RDPVerbose::rail)) {
                LOG(LOG_INFO,
                    "RemoteProgramsVirtualChannel::process_client_execute_pdu: "
                        "First Client Execute PDU ignored.");
            }

            return false;
        }

        const char* exe_of_file = cepdu.ExeOrFile();

        if (::strcasestr(exe_of_file, DUMMY_REMOTEAPP ":") == exe_of_file)
        {
            const char* remoteapplicationprogram =
                (exe_of_file + sizeof(DUMMY_REMOTEAPP ":") - 1);

            if (bool(this->verbose & RDPVerbose::rail)) {
                LOG(LOG_INFO,
                    "RemoteProgramsVirtualChannel::process_client_execute_pdu: "
                        "remoteapplicationprogram=\"%s\"",
                    remoteapplicationprogram);
            }

            this->vars.set_acl<cfg::context::auth_notify>("rail_exec");
            this->vars.set_acl<cfg::context::auth_notify_rail_exec_flags>(cepdu.Flags());
            this->vars.set_acl<cfg::context::auth_notify_rail_exec_exe_or_file>(remoteapplicationprogram);
        }
        else if (0 != ::strcasecmp(exe_of_file, DUMMY_REMOTEAPP)) {
            return true;
        }

        return false;
    }

    bool process_client_get_application_id_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_get_application_id_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_client_windowing_pdu<ClientGetApplicationIDPDU>(flags, chunk);
    }

    bool process_client_handshake_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_handshake_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        HandshakePDU hspdu;

        hspdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            hspdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_client_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientInformationPDU cipdu;

        cipdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            cipdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_client_language_bar_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_language_bar_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        LanguageBarInformationPDU lbipdu;

        lbipdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            lbipdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_client_language_profile_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_language_profile_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        LanguageProfileInformationPDU lpipdu;

        lpipdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            lpipdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_client_notify_event_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_notify_event_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_client_windowing_pdu<ClientNotifyEventPDU>(flags, chunk);
    }

    bool process_client_system_command_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_system_command_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_client_windowing_pdu<ClientSystemCommandPDU>(flags, chunk);
    }

    bool process_client_system_parameters_update_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientSystemParametersUpdatePDU cspupdu;

        cspupdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            cspupdu.log(LOG_INFO);
        }

        if (!this->client_execute_pdu_sent) {
            if (!this->param_client_execute_exe_or_file.empty()) {
                StaticOutStream<16384> out_s;
                RAILPDUHeader header;
                header.emit_begin(out_s, TS_RAIL_ORDER_EXEC);

                ClientExecutePDU cepdu;

                cepdu.Flags(this->param_client_execute_flags);
                cepdu.ExeOrFile(this->param_client_execute_exe_or_file.c_str());
                cepdu.WorkingDir(this->param_client_execute_working_dir.c_str());
                cepdu.Arguments(this->param_client_execute_arguments.c_str());

                cepdu.emit(out_s);

                header.emit_end();

                const size_t   length = out_s.get_offset();
                const uint32_t flags  =   CHANNELS::CHANNEL_FLAG_FIRST
                                        | CHANNELS::CHANNEL_FLAG_LAST;

                {
                    const bool send              = true;
                    const bool from_or_to_client = false;
                    ::msgdump_c(send, from_or_to_client, length, flags,
                        out_s.get_data(), length);
                }
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "Send to server - Client Execute PDU");
                    cepdu.log(LOG_INFO);
                }

                this->send_message_to_server(length, flags, out_s.get_data(),
                    length);
            }

            this->client_execute_pdu_sent = true;
        }

        return true;
    }

    bool process_client_system_menu_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_client_windowing_pdu<ClientSystemMenuPDU>(flags, chunk);
    }

    bool process_client_window_cloak_state_change_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_window_cloak_state_change_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        WindowCloakStateChangePDU wcscpdu;

        wcscpdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            wcscpdu.log(LOG_INFO);
        }

        return this->process_client_windowing_pdu<WindowCloakStateChangePDU>(flags, chunk);
    }

    bool process_client_window_move_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_window_move_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_client_windowing_pdu<ClientWindowMovePDU>(flags, chunk);
    }

public:
    void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length) override
    {
        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_client_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (bool(this->verbose & RDPVerbose::rail_dump)) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream  chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderType(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_client_message: "
                        "Truncated orderType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->client_order_type = chunk.in_uint16_le();
        }

        bool send_message_to_server = true;

        switch (this->client_order_type)
        {
            case TS_RAIL_ORDER_ACTIVATE:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Activate PDU");
                }

                send_message_to_server =
                    this->process_client_activate_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_CLIENTSTATUS:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Information PDU");
                }

                send_message_to_server =
                    this->process_client_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_COMPARTMENTINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Compartment Status Information PDU");
                }

                send_message_to_server =
                    this->process_client_compartment_status_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_CLOAK:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Window Cloak State Change PDU");
                }

                send_message_to_server =
                    this->process_client_window_cloak_state_change_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_EXEC:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Execute PDU");
                }

                send_message_to_server =
                    this->process_client_execute_pdu(
                        total_length, flags, chunk);

                if (send_message_to_server) this->client_execute_pdu_sent = true;
            break;

            case TS_RAIL_ORDER_GET_APPID_REQ:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Get Application ID PDU");
                }

                send_message_to_server =
                    this->process_client_get_application_id_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Handshake PDU");
                }

                send_message_to_server =
                    this->process_client_handshake_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_LANGBARINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Language Bar Information PDU");
                }

                send_message_to_server =
                    this->process_client_language_bar_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_LANGUAGEIMEINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Language Profile Information PDU");
                }

                send_message_to_server =
                    this->process_client_language_profile_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_NOTIFY_EVENT:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Notify Event PDU");
                }

                send_message_to_server =
                    this->process_client_notify_event_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSCOMMAND:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client System Command PDU");
                }

                send_message_to_server =
                    this->process_client_system_command_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSPARAM:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client System Parameters Update PDU");
                }

                send_message_to_server =
                    this->process_client_system_parameters_update_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSMENU:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client System Menu PDU");
                }

                send_message_to_server =
                    this->process_client_system_menu_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_WINDOWMOVE:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Window Move PDU");
                }

                send_message_to_server =
                    this->process_client_window_move_pdu(
                        total_length, flags, chunk);
            break;

            default:
                assert(false);

                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Delivering unprocessed messages %s(%u) to server.",
                        get_RAIL_orderType_name(this->client_order_type),
                        static_cast<unsigned>(this->client_order_type));
                }
            break;
        }   // switch (this->client_order_type)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }   // process_client_message

    bool process_server_compartment_status_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_compartment_status_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        CompartmentStatusInformationPDU csipdu;

        csipdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            csipdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_server_execute_result_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        bool is_auth_application = false;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_execute_result_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ServerExecuteResultPDU serpdu;

        serpdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            serpdu.log(LOG_INFO);
        }

        std::vector<LaunchPendingApp>::iterator iter = std::find_if(
                this->launch_pending_apps.begin(),
                this->launch_pending_apps.end(),
                [&serpdu](LaunchPendingApp& app) -> bool {
                        return (!::strcmp(app.ExeOrFile(), serpdu.ExeOrFile()) &&
                            (app.Flags() == serpdu.Flags()));
                    }
            );
        if (this->launch_pending_apps.end() != iter) {
            if (bool(this->verbose & RDPVerbose::rail)) {
                LOG(LOG_INFO,
                    "RemoteProgramsVirtualChannel::process_server_execute_result_pdu: "
                        "Bastion Application found. OriginalExeOrFile=\"%s\" "
                        "ExeOrFile=\"%s\"",
                    iter->OriginalExeOrFile(), serpdu.ExeOrFile());
            }

            serpdu.ExeOrFile(iter->OriginalExeOrFile());

            this->launch_pending_apps.erase(iter);

            is_auth_application = true;
        }

        if (serpdu.ExecResult() != RAIL_EXEC_S_OK) {
            uint16_t ExecResult_ = serpdu.ExecResult();
            LOG(LOG_WARNING,
                "RemoteProgramsVirtualChannel::process_server_execute_result_pdu: "
                    "Flags=0x%X ExecResult=%s(%d) RawResult=%u",
                serpdu.Flags(),
                get_RAIL_ExecResult_name(ExecResult_), ExecResult_,
                serpdu.RawResult());
        }
        else {
            if (!this->session_probe_channel ||
                this->param_client_execute_exe_or_file != serpdu.ExeOrFile()) {

                auto info = key_qvalue_pairs({
                    {"type", "CLIENT_EXECUTE_REMOTEAPP"},
                    {"exe_or_file", serpdu.ExeOrFile()},
                    });

                ArcsightLogInfo arc_info;
                arc_info.name = "CLIENT_EXECUTE_REMOTEAPP";
                arc_info.signatureID = ArcsightLogInfo::CLIENT_EXECUTE_REMOTEAPP;
                arc_info.ApplicationProtocol = "rdp";
                arc_info.filePath = serpdu.ExeOrFile();
                arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                this->report_message.log6(info, arc_info, tvtime());
            }
        }

        if (this->param_client_execute_exe_or_file == serpdu.ExeOrFile()) {
            assert(!is_auth_application);

            if (this->session_probe_channel) {
/*
                if (this->session_probe_stop_launch_sequence_notifier) {
                    this->session_probe_stop_launch_sequence_notifier->stop(serpdu.ExecResult() == RAIL_EXEC_S_OK);
                    this->session_probe_stop_launch_sequence_notifier = nullptr;
                }
*/

                if (serpdu.ExecResult() != RAIL_EXEC_S_OK) {
                    throw Error(ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG);
                }

                if (!this->exe_or_file_2_sent &&
                    !this->param_client_execute_exe_or_file_2.empty()) {
                    this->exe_or_file_exec_ok = true;

                    this->try_launch_application();
                }
            }
            else {
                if (serpdu.ExecResult() != RAIL_EXEC_S_OK) {
                    throw Error(ERR_RAIL_CLIENT_EXECUTE);
                }
            }

            return (!this->session_probe_channel);
        }

        if (this->param_client_execute_exe_or_file_2 == serpdu.ExeOrFile()) {
            assert(!is_auth_application);

            if (this->session_probe_channel) {
                this->session_probe_channel->start_end_session_check();
            }

            if (serpdu.ExecResult() != RAIL_EXEC_S_OK) {
                throw Error((serpdu.ExecResult() == RAIL_EXEC_E_NOT_IN_ALLOWLIST)
                    ? ERR_RAIL_UNAUTHORIZED_PROGRAM
                    : ERR_RAIL_STARTING_PROGRAM);
            }

            return true;
        }

        if (is_auth_application) {
            StaticOutStream<1024> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_EXEC_RESULT);

            serpdu.emit(out_s);

            header.emit_end();

            const size_t   length = out_s.get_offset();
            const uint32_t flags_ =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;
            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags_,
                    out_s.get_data(), length);
            }
            if (bool(this->verbose & RDPVerbose::rail)) {
                LOG(LOG_INFO,
                    "RemoteProgramsVirtualChannel::auth_rail_exec_cancel: "
                        "Send to client - Server Execute Result PDU");
                serpdu.log(LOG_INFO);
            }

            this->send_message_to_client(length, flags_, out_s.get_data(),
                length);

            return false;
        }

        return true;
    }

    bool process_server_get_application_id_response_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_get_application_id_response_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_server_windowing_pdu<ServerGetApplicationIDResponsePDU>(flags, chunk);
    }

    bool process_server_handshake_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_handshake_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        HandshakePDU hspdu;

        hspdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            hspdu.log(LOG_INFO);
        }

        // if (this->param_client_supports_enhanced_remoteapp &&
        //     this->param_client_supports_handshakeex_pdu) {

        //     HandshakeExPDU hsexpdu;

        //     hsexpdu.buildNumber(hspdu.buildNumber());
        //     hsexpdu.railHandshakeFlags(TS_RAIL_ORDER_HANDSHAKEEX_FLAGS_HIDEF);

        //     StaticOutStream<1024> out_s;
        //     RAILPDUHeader header;
        //     header.emit_begin(out_s, TS_RAIL_ORDER_HANDSHAKE_EX);

        //     hsexpdu.emit(out_s);

        //     header.emit_end();

        //     const size_t   length = out_s.get_offset();
        //     const uint32_t flags_ =   CHANNELS::CHANNEL_FLAG_FIRST
        //                             | CHANNELS::CHANNEL_FLAG_LAST;
        //     {
        //         const bool send              = true;
        //         const bool from_or_to_client = true;
        //         ::msgdump_c(send, from_or_to_client, length, flags_,
        //             out_s.get_data(), length);
        //     }
        //     if (bool(this->verbose & RDPVerbose::rail)) {
        //         LOG(LOG_INFO,
        //             "RemoteProgramsVirtualChannel::process_server_handshake_pdu: "
        //                 "Send to client - HandshakeEx PDU");
        //         hsexpdu.log(LOG_INFO);
        //     }

        //     this->send_message_to_client(length, flags_, out_s.get_data(),
        //         length);

        //     return false;
        // }

        return true;
    }

    bool process_server_handshake_ex_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_handshake_ex_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        HandshakeExPDU hsexpdu;

        hsexpdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            hsexpdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_server_language_bar_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_language_bar_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        LanguageBarInformationPDU lbipdu;

        lbipdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            lbipdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_server_min_max_info_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_min_max_info_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_server_windowing_pdu<ServerMinMaxInfoPDU>(flags, chunk);
    }

    bool process_server_move_size_start_or_end_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_move_size_start_or_end_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_server_windowing_pdu<ServerMoveSizeStartOrEndPDU>(flags, chunk);
    }

    bool process_server_system_parameters_update_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_system_parameters_update_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ServerSystemParametersUpdatePDU sspupdu;

        sspupdu.receive(chunk);

        if (bool(this->verbose & RDPVerbose::rail)) {
            sspupdu.log(LOG_INFO);
        }

        return true;
    }

    bool process_server_z_order_sync_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_z_order_sync_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        return this->process_server_windowing_pdu<ServerZOrderSyncInformationPDU>(flags, chunk);
    }

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (bool(this->verbose & RDPVerbose::rail_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderType(2) */)) {
                LOG(LOG_ERR,
                    "RemoteProgramsVirtualChannel::process_server_message: "
                        "Truncated orderType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->server_order_type = chunk.in_uint16_le();
        }

        bool send_message_to_client = true;

        switch (this->server_order_type)
        {
            case TS_RAIL_ORDER_COMPARTMENTINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Compartment Status Information PDU");
                }

                send_message_to_client =
                    this->process_server_compartment_status_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_EXEC_RESULT:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Execute Result PDU");
                }

                send_message_to_client =
                    this->process_server_execute_result_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_GET_APPID_RESP:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Get Application ID Response PDU");
                }

                send_message_to_client =
                    this->process_server_get_application_id_response_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Handshake PDU");
                }

                send_message_to_client =
                    this->process_server_handshake_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE_EX:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server HandshakeEx PDU");
                }

                send_message_to_client =
                    this->process_server_handshake_ex_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_LANGBARINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Language Bar Information PDU");
                }

                send_message_to_client =
                    this->process_server_language_bar_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_LOCALMOVESIZE:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Move/Size Start/End PDU");
                }

                send_message_to_client =
                    this->process_server_move_size_start_or_end_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_MINMAXINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Min Max Info PDU");
                }

                send_message_to_client =
                    this->process_server_min_max_info_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSPARAM:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server System Parameters Update PDU");
                }

                send_message_to_client =
                    this->process_server_system_parameters_update_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_ZORDER_SYNC:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Z-Order Sync Information PDU");
                }

                send_message_to_client =
                    this->process_server_z_order_sync_information_pdu(
                        total_length, flags, chunk);
            break;

            default:
                assert(false);

                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Delivering unprocessed messages %s(%u) to client.",
                        get_RAIL_orderType_name(this->server_order_type),
                        static_cast<unsigned>(this->server_order_type));
                }
            break;
        }   // switch (this->server_order_type)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data,
                chunk_data_length);
        }   // switch (this->server_order_type)
    }   // process_server_message

    void set_session_probe_virtual_channel(SessionProbeVirtualChannel * session_probe_channel) {
        this->session_probe_channel = session_probe_channel;
    }

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->session_probe_stop_launch_sequence_notifier = launcher;
    }

    void auth_rail_exec(uint16_t flags, const char* original_exe_or_file,
            const char* exe_or_file, const char* working_dir,
            const char* arguments, const char* account, const char* password) override {
        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::auth_rail_exec: "
                    "original_exe_or_file=\"%s\" "
                    "exe_or_file=\"%s\" "
                    "working_dir=\"%s\" "
                    "arguments=\"%s\" "
                    "account=\"%s\" "
                    "flags=%u",
                original_exe_or_file, exe_or_file, working_dir, arguments, account, flags);
        }

        launch_pending_apps.emplace_back(LaunchPendingApp(original_exe_or_file, exe_or_file, flags));

        std::string arguments_(arguments);
        {
            const char * appid_marker = "${APPID}";
            size_t pos = arguments_.find(appid_marker, 0);
            if (pos != std::string::npos) {
                arguments_.replace(pos, strlen(appid_marker), original_exe_or_file);
            }
        }
        if (account && *account) {
            const char * user_marker = "${USER}";
            size_t pos = arguments_.find(user_marker, 0);
            if (pos != std::string::npos) {
                arguments_.replace(pos, strlen(user_marker), account);
            }
        }
        if (password && *password) {
            const char * password_marker = "${PASSWORD}";
            size_t pos = arguments_.find(password_marker, 0);
            if (pos != std::string::npos) {
                arguments_.replace(pos, strlen(password_marker), password);
            }
        }

        if (this->param_use_session_probe_to_launch_remote_program &&
            this->session_probe_channel) {
                this->session_probe_channel->rail_exec(
                        exe_or_file,
                        arguments_.c_str(),
                        working_dir,
                        false,  // Show maximized
                        flags
                    );
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::auth_rail_exec: "
                            "Send to Session Probe - Execure");
                }
        }
        else {
            StaticOutStream<1024> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_EXEC);

            ClientExecutePDU cepdu;

            cepdu.Flags(flags);
            cepdu.ExeOrFile(exe_or_file);
            cepdu.WorkingDir(working_dir);
            cepdu.Arguments(arguments_.c_str());

            cepdu.emit(out_s);

            header.emit_end();

            const size_t   length = out_s.get_offset();
            const uint32_t flags_ =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

            {
                const bool send              = true;
                const bool from_or_to_client = false;
                ::msgdump_c(send, from_or_to_client, length, flags_,
                    out_s.get_data(), length);
            }
            if (bool(this->verbose & RDPVerbose::rail)) {
                LOG(LOG_INFO,
                    "RemoteProgramsVirtualChannel::auth_rail_exec: "
                        "Send to server - Client Execute PDU");
                cepdu.log(LOG_INFO);
            }

            this->send_message_to_server(length, flags_, out_s.get_data(),
                length);
        }
    }

    void auth_rail_exec_cancel(uint16_t flags, const char* original_exe_or_file,
            uint16_t exec_result) override {
        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::auth_rail_exec_cancel: "
                    "exec_result=%u "
                    "original_exe_or_file=\"%s\" "
                    "flags=%u",
                exec_result, original_exe_or_file, flags);
        }

        StaticOutStream<1024> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_EXEC_RESULT);

        ServerExecuteResultPDU serpdu;

        serpdu.Flags(flags);
        serpdu.ExecResult(exec_result);
        serpdu.RawResult(0xFFFFFFFF);
        serpdu.ExeOrFile(original_exe_or_file);

        serpdu.emit(out_s);

        header.emit_end();

        const size_t   length = out_s.get_offset();
        const uint32_t flags_ =   CHANNELS::CHANNEL_FLAG_FIRST
                                | CHANNELS::CHANNEL_FLAG_LAST;
        {
            const bool send              = true;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, length, flags_,
                out_s.get_data(), length);
        }
        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::auth_rail_exec_cancel: "
                    "Send to client - Server Execute Result PDU");
            serpdu.log(LOG_INFO);
        }

        this->send_message_to_client(length, flags_, out_s.get_data(),
            length);
    }

    void sespro_rail_exec_result(uint16_t flags, const char* exe_or_file,
        uint16_t exec_result, uint32_t raw_result) override {
        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::sespro_rail_exec_result: "
                    "exec_result=%u "
                    "raw_result=%u "
                    "exe_or_file=\"%s\" "
                    "flags=%u",
                exec_result, raw_result, exe_or_file, flags);
        }

        StaticOutStream<1024> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_EXEC_RESULT);

        ServerExecuteResultPDU serpdu;

        serpdu.Flags(flags);
        serpdu.ExecResult(exec_result);
        serpdu.RawResult(raw_result);
        serpdu.ExeOrFile(exe_or_file);

        serpdu.emit(out_s);

        header.emit_end();

        const size_t   length = out_s.get_offset();
        const uint32_t flags_ =   CHANNELS::CHANNEL_FLAG_FIRST
                                | CHANNELS::CHANNEL_FLAG_LAST;
        {
            const bool send              = true;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, length, flags_,
                out_s.get_data(), length);
        }
        if (bool(this->verbose & RDPVerbose::rail)) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::sespro_rail_exec_result: "
                    "Send to client - Server Execute Result PDU");
            serpdu.log(LOG_INFO);
        }

        this->send_message_to_client(length, flags_, out_s.get_data(),
            length);
    }

    void confirm_session_probe_launch() {
        if (!this->exe_or_file_2_sent &&
            !this->param_client_execute_exe_or_file_2.empty()) {
            this->session_probe_launch_confirmed = true;

            this->try_launch_application();
        }
    }

private:
    void try_launch_application() {
        if (!this->exe_or_file_exec_ok || !this->session_probe_launch_confirmed) {
            return;
        }

        assert(!this->exe_or_file_2_sent &&
            !this->param_client_execute_exe_or_file_2.empty());

        this->exe_or_file_2_sent = true;

        StaticOutStream<16384> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_EXEC);

        ClientExecutePDU cepdu;

        cepdu.Flags(this->param_client_execute_flags_2);
        cepdu.ExeOrFile(this->param_client_execute_exe_or_file_2.c_str());
        cepdu.WorkingDir(this->param_client_execute_working_dir_2.c_str());
        cepdu.Arguments(this->param_client_execute_arguments_2.c_str());

        cepdu.emit(out_s);

        header.emit_end();

        const size_t   length = out_s.get_offset();
        const uint32_t flags  =   CHANNELS::CHANNEL_FLAG_FIRST
                                | CHANNELS::CHANNEL_FLAG_LAST;

        {
            const bool send              = true;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, length, flags,
                out_s.get_data(), length);
        }
        LOG(LOG_INFO,
            "RemoteProgramsVirtualChannel::try_launch_application: "
                "Send to server - Client Execute PDU (2)");
        cepdu.log(LOG_INFO);

        this->send_message_to_server(length, flags, out_s.get_data(),
            length);
    }

    void sespro_ending_in_progress() override {}

    void sespro_launch_process_ended() override {}
};
