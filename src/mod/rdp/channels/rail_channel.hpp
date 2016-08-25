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

#include "mod/rdp/channels/base_channel.hpp"

class RemoteProgramsVirtualChannel : public BaseVirtualChannel
{
private:
    uint16_t client_order_type = 0;
    uint16_t server_order_type = 0;

    FrontAPI& front;

public:
    struct Params : public BaseVirtualChannel::Params {
    };

    RemoteProgramsVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        const Params & params)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         params)
    , front(front) {}

protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "RAIL_LIMIT";
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

        ClientActivatePDU capdu;

        capdu.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            capdu.log(LOG_INFO);
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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            cepdu.log(LOG_INFO);
        }

        return true;
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

        ClientGetApplicationIDPDU cgaipdu;

        cgaipdu.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            cgaipdu.log(LOG_INFO);
        }

        return true;
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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            cipdu.log(LOG_INFO);
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
                    "RemoteProgramsVirtualChannel::process_client_information_pdu: "
                        "Truncated orderLength, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            chunk.in_skip_bytes(2); // orderLength(2)
        }

        ClientNotifyEventPDU cnepdu;

        cnepdu.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            cnepdu.log(LOG_INFO);
        }

        return true;
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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            cspupdu.log(LOG_INFO);
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

        ClientSystemMenuPDU csmpdu;

        csmpdu.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            csmpdu.log(LOG_INFO);
        }

        return true;
    }

public:
    void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length) override
    {
        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_client_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_RAIL_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

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
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Activate PDU");
                }

                send_message_to_server =
                    this->process_client_activate_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_CLIENTSTATUS:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Information PDU");
                }

                send_message_to_server =
                    this->process_client_information_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_EXEC:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Execute PDU");
                }

                send_message_to_server =
                    this->process_client_execute_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_GET_APPID_REQ:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Get Application ID PDU");
                }

                send_message_to_server =
                    this->process_client_get_application_id_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Handshake PDU");
                }

                send_message_to_server =
                    this->process_client_handshake_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_NOTIFY_EVENT:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client Notify Event PDU");
                }

                send_message_to_server =
                    this->process_client_notify_event_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSPARAM:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client System Parameters Update PDU");
                }

                send_message_to_server =
                    this->process_client_system_parameters_update_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSMENU:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_message: "
                            "Client System Menu PDU");
                }

                send_message_to_server =
                    this->process_client_system_menu_pdu(
                        total_length, flags, chunk);
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
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

    bool process_server_execute_result_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            serpdu.log(LOG_INFO);
        }

        return true;
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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            hspdu.log(LOG_INFO);
        }

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

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            hsexpdu.log(LOG_INFO);
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

        ServerMinMaxInfoPDU smmipdu;

        smmipdu.receive(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            smmipdu.log(LOG_INFO);
        }

        return true;
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

        uint32_t SystemParam = sspupdu.SystemParam();

        uint8_t Body = sspupdu.Body();

        switch(SystemParam) {
            case SPI_SETSCREENSAVEACTIVE:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_system_parameters_update_pdu: "
                            "SPI_SETSCREENSAVEACTIVE - "
                            "Screen saver is %s.",
                        (!Body ? "disabled" : "enabled"));
                }
            }
            break;

            case SPI_SETSCREENSAVESECURE:
            {
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_system_parameters_update_pdu: "
                            "SPI_SETSCREENSAVESECURE - "
                            "The desktop is%s to be locked after switching out of screen saver mode.",
                        (!Body ? " not" : ""));
                }
            }
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_system_parameters_update_pdu: "
                            "Delivering unprocessed server system parameter %s(%u) to client.",
                        get_RAIL_ServerSystemParam_name(SystemParam),
                        SystemParam);
                }
            break;
        }

        return true;
    }

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_RAIL_DUMP) {
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
            case TS_RAIL_ORDER_EXEC_RESULT:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Execute Result PDU");
                }

                send_message_to_client =
                    this->process_server_execute_result_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Handshake PDU");
                }

                send_message_to_client =
                    this->process_server_handshake_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_HANDSHAKE_EX:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server HandshakeEx PDU");
                }

                send_message_to_client =
                    this->process_server_handshake_ex_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_MINMAXINFO:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server Min Max Info PDU");
                }

                send_message_to_client =
                    this->process_server_min_max_info_pdu(
                        total_length, flags, chunk);
            break;

            case TS_RAIL_ORDER_SYSPARAM:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_server_message: "
                            "Server System Parameters Update PDU");
                }

                send_message_to_client =
                    this->process_server_system_parameters_update_pdu(
                        total_length, flags, chunk);
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
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
};
