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
#include "mod/mod_api.hpp"
#include "utils/stream.hpp"
#include "utils/virtual_channel_data_sender.hpp"

#define INTERNAL_MODULE_WINDOW_ID    40000
#define INTERNAL_MODULE_WINDOW_TITLE "WALLIX ADMINBASTION"

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

    bool server_execute_result_sent = false;

    Rect window_rect;

public:
    ClientExecute(FrontAPI & front) {
        this->front_ = &front;
    }


    Rect adjust_rect(Rect rect) {
        if (!this->front_->get_channel_list().get_by_name(channel_names::rail)) return rect;

        this->window_rect.x  = rect.x + rect.cx * 10 / 100;
        this->window_rect.y  = rect.y + rect.cy * 10 / 100;
        this->window_rect.cx = rect.cx * 80 / 100;
        this->window_rect.cy = rect.cy * 80 / 100;

        Rect result_rect = this->window_rect.shrink(1);
        result_rect.cx--;
        result_rect.cy--;

        return result_rect;
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
        if (!this->channel_) return;

        {
            RDP::RAIL::DeletedWindow order;

            order.header.FieldsPresentFlags(
                      RDP::RAIL::WINDOW_ORDER_STATE_DELETED
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                );
            order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

            StaticOutStream<256> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::reset: Send DeletedWindow to client: size=%zu", out_s.get_offset() - 1);

            this->front_->draw(order);
        }


        this->channel_ = nullptr;
    }

    void enable() {
        {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                    RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND
                );

            order.ActiveWindowId(INTERNAL_MODULE_WINDOW_ID);

            StaticOutStream<256> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::enable: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

            this->front_->draw(order);
        }

        {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                    RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER
                );

            order.NumWindowIds(1);
            order.window_ids(0, INTERNAL_MODULE_WINDOW_ID);

            StaticOutStream<256> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::enable: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

            this->front_->draw(order);
        }
    }

    void process_client_activate_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::process_client_activate_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientActivatePDU capdu;

        capdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
            capdu.log(LOG_INFO);
        }

        if ((capdu.WindowId() == INTERNAL_MODULE_WINDOW_ID) &&
            (capdu.Enabled() == 0)) {
            {
                RDP::RAIL::ActivelyMonitoredDesktop order;

                order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND
                    );

                order.ActiveWindowId(0xFFFFFFFF);

                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_activate_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            {
                RDP::RAIL::ActivelyMonitoredDesktop order;

                order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                        RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER
                    );

                order.NumWindowIds(1);
                order.window_ids(0, INTERNAL_MODULE_WINDOW_ID);

                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_activate_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }
        }
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

    void process_client_get_application_id_pdu(uint32_t total_length,
            uint32_t flags, InStream& chunk) {
        (void)total_length;

        if (!this->channel_) return;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* orderLength(2) */)) {
                LOG(LOG_ERR,
                    "ClientExecute::process_client_get_application_id_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientGetApplicationIDPDU cgaipdu;

        cgaipdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
            cgaipdu.log(LOG_INFO);
        }

        {
            StaticOutStream<1024> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_GET_APPID_RESP);

            ServerGetApplicationIDResponsePDU server_get_application_id_response_pdu;
            server_get_application_id_response_pdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
            server_get_application_id_response_pdu.ApplicationId(INTERNAL_MODULE_WINDOW_TITLE);
            server_get_application_id_response_pdu.emit(out_s);

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
            LOG(LOG_INFO,
                "ClientExecute::process_client_get_application_id_pdu: "
                    "Send to client - Server Get Application ID Response PDU");
            server_get_application_id_response_pdu.log(LOG_INFO);

            this->front_->send_to_channel(*(this->channel_), out_s.get_data(), length, chunk_size,
                                          flags);

            server_execute_result_sent = true;
        }
    }

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

        ClientSystemCommandPDU cscpdu;

        cscpdu.receive(chunk);

        /*if (this->verbose & MODRDP_LOGLEVEL_RAIL)*/ {
            cscpdu.log(LOG_INFO);
        }

        return true;
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

/*
            if (!server_execute_result_sent) {
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

                server_execute_result_sent = true;
            }
*/

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
                order.TitleInfo(INTERNAL_MODULE_WINDOW_TITLE);
                order.ClientOffsetX(/*160*/this->window_rect.x + 6);
                order.ClientOffsetY(/*179*/this->window_rect.y + 25);
                order.WindowOffsetX(/*154*/this->window_rect.x);
                order.WindowOffsetY(/*154*/this->window_rect.y);
                order.WindowClientDeltaX(6);
                order.WindowClientDeltaY(25);
                order.WindowWidth(/*668*/this->window_rect.cx);
                order.WindowHeight(/*331*/this->window_rect.cy);
                order.VisibleOffsetX(/*154*/this->window_rect.x);
                order.VisibleOffsetY(/*154*/this->window_rect.y);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, 668, 331));

                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send NewOrExistingWindow to client: size=%zu", out_s.get_offset() - 1);

                this->front_->draw(order);
            }

            {
                RDP::RAIL::WindowIcon order;

                order.header.FieldsPresentFlags(
                          RDP::RAIL::WINDOW_ORDER_ICON
                        | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                        | RDP::RAIL::WINDOW_ORDER_FIELD_ICON_BIG
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                order.icon_info.CacheEntry(65535);
                order.icon_info.CacheId(255);
                order.icon_info.Bpp(16);
                order.icon_info.Width(32);
                order.icon_info.Height(32);

                uint8_t const BitsMask[] = {
/* 0000 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0030 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0040 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0050 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0070 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  //................
                    };
                order.icon_info.BitsMask(BitsMask, sizeof(BitsMask));

                uint8_t const BitsColor[] = {
/* 0000 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0010 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0020 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0030 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0040 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0050 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0060 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0070 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe5, 0x75, 0xc3, 0x75, //.u.u.u.u.u.u.u.u
/* 0080 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0090 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00b0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x06, 0x7a, 0xf2, 0x7a, 0xfe, 0x7f, 0x27, 0x7a, //.u.u.u.u.z.z..'z
/* 00c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00f0 */ 0xa1, 0x75, 0xa1, 0x75, 0x49, 0x7a, 0x35, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x27, 0x7a, //.u.uIz5.......'z
/* 0100 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0110 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0120 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, //.u.u.u.u.u.u.u.u
/* 0130 */ 0x28, 0x7a, 0x57, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x7f, 0x07, 0x7a, //(zW............z
/* 0140 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0150 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0160 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x6c, 0x7a, 0x57, 0x7f, //.u.u.u.u.u.ulzW.
/* 0170 */ 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, 0x13, 0x7b, 0x29, 0x7a, 0xa2, 0x75, //...........{)z.u
/* 0180 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0190 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 01a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0x6b, 0x7a, 0xbb, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.ukz......
/* 01b0 */ 0xff, 0x7f, 0xff, 0x7f, 0xbb, 0x7f, 0xf1, 0x7a, 0xc4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.......z.u.u.u.u
/* 01c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 01d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 01e0 */ 0xa1, 0x75, 0xc4, 0x75, 0xd0, 0x7a, 0x9a, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.z..........
/* 01f0 */ 0x36, 0x7f, 0x6b, 0x7a, 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //6.kz.u.u.u.u.u.u
/* 0200 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0210 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0xe5, 0x75, //.u.u.u.u.u.u.u.u
/* 0220 */ 0xd0, 0x7a, 0xde, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, 0x07, 0x7a, //.z.............z
/* 0230 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0240 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0250 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x06, 0x7a, 0x14, 0x7f, 0xbc, 0x7f, //.u.u.u.u.u.z....
/* 0260 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x99, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xae, 0x7a, //...............z
/* 0270 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0280 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0290 */ 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0xe6, 0x75, 0x14, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u........
/* 02a0 */ 0xff, 0x7f, 0xbb, 0x7f, 0x8e, 0x7a, 0xa2, 0x75, 0xf2, 0x7a, 0xff, 0x7f, 0xff, 0x7f, 0xbc, 0x7f, //.....z.u.z......
/* 02b0 */ 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 02c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 02d0 */ 0xa2, 0x75, 0x4a, 0x7a, 0x56, 0x7f, 0xde, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x9a, 0x7f, //.uJzV...........
/* 02e0 */ 0x8c, 0x7a, 0xc4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x4a, 0x7a, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.z.u.u.uJz......
/* 02f0 */ 0x28, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //(z.u.u.u.u.u.u.u
/* 0300 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0x28, 0x7a, //.u.u.u.u.u.u.u(z
/* 0310 */ 0x78, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xbc, 0x7f, 0xaf, 0x7a, 0xa2, 0x75, //x............z.u
/* 0320 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe5, 0x75, 0xdc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 0330 */ 0xd0, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.z.u.u.u.u.u.u.u
/* 0340 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0xaf, 0x7a, 0x79, 0x7f, 0xff, 0x7f, //.u.u.u.u.u.zy...
/* 0350 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xbb, 0x7f, 0x8d, 0x7a, 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.........z.u.u.u
/* 0360 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc2, 0x75, 0x9a, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 0370 */ 0x35, 0x7f, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //5..u.u.u.u.u.u.u
/* 0380 */ 0xa1, 0x75, 0xa1, 0x75, 0xc4, 0x75, 0x8d, 0x7a, 0xbc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.z........
/* 0390 */ 0xff, 0x7f, 0xbc, 0x7f, 0xd0, 0x7a, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.....z.u.u.u.u.u
/* 03a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x57, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.uW.....
/* 03b0 */ 0x78, 0x7f, 0xc2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //x..u.u.u.u.u.u.u
/* 03c0 */ 0xe4, 0x75, 0xf2, 0x7a, 0x9b, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xbc, 0x7f, //.u.z............
/* 03d0 */ 0xae, 0x7a, 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.z.u.u.u.u.u.u.u
/* 03e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x14, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 03f0 */ 0x99, 0x7f, 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //...u.u.u.u.u.u.u
/* 0400 */ 0x6b, 0x7a, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xd0, 0x7a, //kz.............z
/* 0410 */ 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0420 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x13, 0x7b, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u.{....
/* 0430 */ 0x99, 0x7f, 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //...u.u.u.u.u.u.u
/* 0440 */ 0xa2, 0x75, 0x07, 0x7a, 0xd0, 0x7a, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.z.z..........
/* 0450 */ 0xbc, 0x7f, 0x13, 0x7f, 0x06, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.....z.u.u.u.u.u
/* 0460 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x56, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.uV.....
/* 0470 */ 0x79, 0x7f, 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //y..u.u.u.u.u.u.u
/* 0480 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0xd1, 0x7a, 0xbc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.z......
/* 0490 */ 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x7f, 0x13, 0x7f, 0x06, 0x7a, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.........z.u.u.u
/* 04a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x99, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 04b0 */ 0x56, 0x7f, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //V..u.u.u.u.u.u.u
/* 04c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe4, 0x75, 0x6c, 0x7a, 0x99, 0x7f, //.u.u.u.u.u.ulz..
/* 04d0 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, 0x36, 0x7f, 0x49, 0x7a, 0xa1, 0x75, //..........6.Iz.u
/* 04e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe4, 0x75, 0xbc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 04f0 */ 0x13, 0x7f, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //...u.u.u.u.u.u.u
/* 0500 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0510 */ 0x4b, 0x7a, 0x99, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x57, 0x7f, //Kz............W.
/* 0520 */ 0x27, 0x7a, 0xc2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x49, 0x7a, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //'z.u.u.uIz......
/* 0530 */ 0x6b, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //kz.u.u.u.u.u.u.u
/* 0540 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0550 */ 0xa1, 0x75, 0xc3, 0x75, 0x49, 0x7a, 0x34, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.uIz4.........
/* 0560 */ 0xfe, 0x7f, 0x57, 0x7f, 0x6c, 0x7a, 0xa2, 0x75, 0xf2, 0x7a, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, //..W.lz.u.z......
/* 0570 */ 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0580 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0590 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x06, 0x7a, 0x56, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, //.u.u.u.u.zV.....
/* 05a0 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x9a, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xf1, 0x7a, //...............z
/* 05b0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 05c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 05d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x07, 0x7a, 0xd0, 0x7a, //.u.u.u.u.u.u.z.z
/* 05e0 */ 0xdc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xde, 0x7f, 0x07, 0x7a, //...............z
/* 05f0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0600 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0610 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0620 */ 0xc3, 0x75, 0xd0, 0x7a, 0xbb, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xde, 0x7f, //.u.z............
/* 0630 */ 0xd1, 0x7a, 0xe5, 0x75, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.z.u.u.u.u.u.u.u
/* 0640 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0650 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0660 */ 0xa1, 0x75, 0xa1, 0x75, 0xc4, 0x75, 0x6b, 0x7a, 0x78, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.ukzx.......
/* 0670 */ 0xff, 0x7f, 0xdd, 0x7f, 0x35, 0x7f, 0x49, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //....5.Iz.u.u.u.u
/* 0680 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0690 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 06a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x4a, 0x7a, 0x79, 0x7f, 0xfe, 0x7f, //.u.u.u.u.uJzy...
/* 06b0 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x78, 0x7f, 0x29, 0x7a, 0xc3, 0x75, 0xa1, 0x75, //........x.)z.u.u
/* 06c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 06d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 06e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc2, 0x75, 0x28, 0x7a, //.u.u.u.u.u.u.u(z
/* 06f0 */ 0x13, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x79, 0x7f, 0xe5, 0x75, //............y..u
/* 0700 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0710 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0720 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0730 */ 0xa1, 0x75, 0xe5, 0x75, 0x35, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x27, 0x7a, //.u.u5.........'z
/* 0740 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0750 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0760 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0770 */ 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x06, 0x7a, 0xaf, 0x7a, 0xbc, 0x7f, 0xff, 0x7f, 0x27, 0x7a, //.u.u.u.z.z....'z
/* 0780 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0790 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07b0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0xae, 0x7a, 0x06, 0x7a, //.u.u.u.u.u.u.z.z
/* 07c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07f0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75  //.u.u.u.u.u.u.u.u
                    };
                order.icon_info.BitsColor(BitsColor, sizeof(BitsColor));

                {
                    StaticOutStream<8192> out_s;
                    order.emit(out_s);
                    order.log(LOG_INFO);
                    LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);
                }

                this->front_->draw(order);

                order.header.FieldsPresentFlags(
                          RDP::RAIL::WINDOW_ORDER_ICON
                        | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                    );

                {
                    StaticOutStream<8192> out_s;
                    order.emit(out_s);
                    order.log(LOG_INFO);
                    LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);
                }

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

                this->process_client_activate_pdu(
                    length, flags, chunk);
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

                this->process_client_get_application_id_pdu(
                    length, flags, chunk);
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

            case TS_RAIL_ORDER_SYSCOMMAND:
                /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                    LOG(LOG_INFO,
                        "ClientExecute::send_to_mod_rail_channel: "
                            "Client System Command PDU");
                }

                this->process_client_system_command_pdu(
                    length, flags, chunk);
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