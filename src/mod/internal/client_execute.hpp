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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/front_api.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/remote_programs.hpp"
#include "utils/stream.hpp"
#include "utils/virtual_channel_data_sender.hpp"

#define INTERNAL_MODULE_WINDOW_ID 40000

class ClientExecute
{
          FrontAPI             * front_   = nullptr;
          mod_api              * mod_     = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;

    uint16_t client_order_type = 0;

    uint16_t    client_execute_flags = 0;
    std::string client_execute_exe_or_file;
    std::string client_execute_working_dir;
    std::string client_execute_arguments;

public:
    ClientExecute(FrontAPI & front) {
        this->front_ = &front;
    }

public:
    void ready(mod_api & mod) {
        this->mod_ = &mod;

        if (!this->channel_) {
            this->channel_ = this->front_->get_channel_list().get_by_name(channel_names::rail);
            if (!this->channel_) return;
        }

        {
            StaticOutStream<256> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_HANDSHAKE);

            HandshakePDU handshake_pdu;
            handshake_pdu.buildNumber(7601);

            handshake_pdu.emit(out_s);

            header.emit_end();

            const size_t   length     = out_s.get_offset();
            const size_t   chunk_size = length;
            const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                        | CHANNELS::CHANNEL_FLAG_LAST;

            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags,
                    out_s.get_data(), length);
            }
            LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server Handshake PDU");
            handshake_pdu.log(LOG_INFO);

            this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                          flags);
        }

        {
            StaticOutStream<256> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_SYSPARAM);

            ServerSystemParametersUpdatePDU server_system_parameters_update_pdu;
            server_system_parameters_update_pdu.SystemParam(SPI_SETSCREENSAVESECURE);
            server_system_parameters_update_pdu.Body(0);
            server_system_parameters_update_pdu.emit(out_s);

            header.emit_end();

            const size_t   length     = out_s.get_offset();
            const size_t   chunk_size = length;
            const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                        | CHANNELS::CHANNEL_FLAG_LAST;

            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags,
                    out_s.get_data(), length);
            }
            LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server System Parameters Update PDU");
            server_system_parameters_update_pdu.log(LOG_INFO);

            this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                          flags);
        }

        {
            StaticOutStream<256> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_SYSPARAM);

            ServerSystemParametersUpdatePDU server_system_parameters_update_pdu;
            server_system_parameters_update_pdu.SystemParam(SPI_SETSCREENSAVEACTIVE);
            server_system_parameters_update_pdu.Body(0);
            server_system_parameters_update_pdu.emit(out_s);

            header.emit_end();

            const size_t length     = out_s.get_offset();
            const size_t chunk_size = length;
            const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                        | CHANNELS::CHANNEL_FLAG_LAST;

            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags,
                    out_s.get_data(), length);
            }
            LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server System Parameters Update PDU");
            server_system_parameters_update_pdu.log(LOG_INFO);

            this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                          flags);
        }
    }   // ready

    explicit operator bool () const noexcept {
        return this->channel_;
    }   // bool

    void reset() {
        this->channel_ = nullptr;
    }

    void process_client_execute_pdu(uint32_t total_length,
            uint32_t flags, InStream& chunk) {
        (void)total_length;

        if (!this->channel_) return;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::process_client_execute_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientExecutePDU cepdu;

        cepdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
            cepdu.log(LOG_INFO);
        }

        this->client_execute_flags       = cepdu.Flags();
        this->client_execute_exe_or_file = cepdu.ExeOrFile();
        this->client_execute_working_dir = cepdu.WorkingDir();
        this->client_execute_arguments   = cepdu.Arguments();
    }   // process_client_execute_pdu

    void process_client_handshake_pdu(uint32_t total_length,
            uint32_t flags, InStream& chunk) {
        (void)total_length;

        if (!this->channel_) return;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::process_client_handshake_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        HandshakePDU hspdu;

        hspdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
            hspdu.log(LOG_INFO);
        }
    }   // process_client_handshake_pdu

    void process_client_information_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (!this->channel_) return;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::process_client_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientInformationPDU cipdu;

        cipdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
            cipdu.log(LOG_INFO);
        }
    }

    void process_client_system_parameters_update_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (!this->channel_) return;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::process_client_system_parameters_update_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientSystemParametersUpdatePDU cspupdu;

        cspupdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
            cspupdu.log(LOG_INFO);
        }

        if (cspupdu.SystemParam() == SPI_SETWORKAREA) {
            {
                RDP::RAIL::ActivelyMonitoredDesktop order;

                order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_HOOKED |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ARC_BEGAN
                    );

                order.ActiveWindowId(0xFFFFFFFF);
                order.NumWindowIds(0);

                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            {
                RDP::RAIL::ActivelyMonitoredDesktop order;

                order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ARC_COMPLETED
                    );

                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            {
                StaticOutStream<256> out_s;
                RAILPDUHeader header;
                header.emit_begin(out_s, TS_RAIL_ORDER_EXEC_RESULT);

                ServerExecuteResultPDU server_execute_result_pdu;
                server_execute_result_pdu.Flags(this->client_execute_flags);
                server_execute_result_pdu.ExecResult(RAIL_EXEC_S_OK);
                server_execute_result_pdu.RawResult(0);
                server_execute_result_pdu.ExeOrFile(this->client_execute_exe_or_file.c_str());
                server_execute_result_pdu.emit(out_s);

                header.emit_end();

                const size_t   length     = out_s.get_offset();
                const size_t   chunk_size = length;
                const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                            | CHANNELS::CHANNEL_FLAG_LAST;

                {
                    const bool send              = true;
                    const bool from_or_to_client = true;
                    ::msgdump_c(send, from_or_to_client, length, flags,
                        out_s.get_data(), length);
                }
                LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server Execute Result PDU");
                server_execute_result_pdu.log(LOG_INFO);

                this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                              flags);
            }

            {
                RDP::RAIL::ActivelyMonitoredDesktop order;

                order.header.FieldsPresentFlags(
                          RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP
                        | RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND
                    );

                order.ActiveWindowId(INTERNAL_MODULE_WINDOW_ID);
                order.NumWindowIds(0);

                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            {
                RDP::RAIL::ActivelyMonitoredDesktop order;

                order.header.FieldsPresentFlags(
                          RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP
                        | RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER
                    );

                order.NumWindowIds(1);
                order.window_ids(0, INTERNAL_MODULE_WINDOW_ID);

                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            {
                RDP::RAIL::NewOrExistingWindow order;

                order.header.FieldsPresentFlags(
                          RDP::RAIL::WINDOW_ORDER_STATE_NEW
                        | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDCLIENTDELTA
                        | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                        | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                        | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                        | RDP::RAIL::WINDOW_ORDER_FIELD_TITLE
                        | RDP::RAIL::WINDOW_ORDER_FIELD_OWNER
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                order.OwnerWindowId(0x0);
                order.Style(0x14EF0000);
                order.ExtendedStyle(0x40310);
                order.ShowState(5);
                order.TitleInfo("WALLIX ADMINBASTION");
                order.ClientOffsetX(160);
                order.ClientOffsetY(179);
                order.WindowOffsetX(154);
                order.WindowOffsetY(154);
                order.WindowClientDeltaX(6);
                order.WindowClientDeltaY(25);
                order.WindowWidth(668);
                order.WindowHeight(331);
                order.VisibleOffsetX(154);
                order.VisibleOffsetY(154);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, 668, 331));

                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send NewOrExistingWindow to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            this->mod_->rdp_input_invalidate(Rect(154, 154, 154 + 668, 154 + 331));
        }
    }

    void send_to_mod_rail_channel(size_t length, InStream & chunk, uint32_t flags) {
        LOG(LOG_INFO,
            "ClientExecute::send_to_mod_rail_channel: "
                "total_length=%zu flags=0x%08X chunk_data_length=%zu",
            length, flags, chunk.get_capacity());

        {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, length, flags,
                chunk.get_data(), chunk.get_capacity());
        }

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderType(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::send_to_mod_rail_channel: "
                        "Truncated orderType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->client_order_type = chunk.in_uint16_le();
        }

        switch (this->client_order_type)
        {
            case TS_RAIL_ORDER_ACTIVATE:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Activate PDU");
                }

//                this->process_client_activate_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_CLIENTSTATUS:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Information PDU");
                }

                this->process_client_information_pdu(
                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_COMPARTMENTINFO:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Compartment Status Information PDU");
                }

//                this->process_client_compartment_status_information_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_CLOAK:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Window Cloak State Change PDU");
                }

//                this->process_client_window_cloak_state_change_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_EXEC:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Execute PDU");
                }

                this->process_client_execute_pdu(
                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_GET_APPID_REQ:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Get Application ID PDU");
                }

//                this->process_client_get_application_id_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Handshake PDU");
                }

                this->process_client_handshake_pdu(
                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_LANGBARINFO:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Language Bar Information PDU");
                }

//                this->process_client_language_bar_information_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_LANGUAGEIMEINFO:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Language Profile Information PDU");
                }

//                this->process_client_language_profile_information_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_NOTIFY_EVENT:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Notify Event PDU");
                }

//                this->process_client_notify_event_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSPARAM:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client System Parameters Update PDU");
                }

                this->process_client_system_parameters_update_pdu(
                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSMENU:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client System Menu PDU");
                }

//                this->process_client_system_menu_pdu(
//                    length, flags, chunk);
            break;

            case TS_RAIL_ORDER_WINDOWMOVE:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client Window Move PDU");
                }

//                this->process_client_window_move_pdu(
//                    length, flags, chunk);
            break;

            default:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Delivering unprocessed messages %s(%u) to server.",
                        get_RAIL_orderType_name(this->client_order_type),
                        static_cast<unsigned>(this->client_order_type));
                }
            break;
        }   // switch (this->client_order_type)
    }   // send_to_mod_rail_channel

    uint16_t Flags() const { return this->client_execute_flags; }

    const char * ExeOrFile() const { return this->client_execute_exe_or_file.c_str(); }

    const char * WorkingDir() const { return this->client_execute_working_dir.c_str(); }

    const char * Arguments() const { return this->client_execute_arguments.c_str(); }
};  // class ClientExecute