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

        ClientExecutePDU_Recv cepdur(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_client_execute_pdu: "
                    "flags=0x%X exe_or_file=\"%s\" working_dir=\"%s\" arguments=\"%s\"",
                cepdur.Flags(), cepdur.exe_or_file(), cepdur.working_dir(), cepdur.arguments());
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

        HandshakePDU_Recv hspdur(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_client_handshake_pdu: "
                    "buildNumber=%u", hspdur.buildNumber());
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

        ClientInformationPDU_Recv cipdur(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_client_information_pdu: "
                    "Flags=0x%08X",
                cipdur.Flags());
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

        ClientSystemParametersUpdatePDU_Recv cspupdur(chunk);

        uint32_t SystemParam = cspupdur.SystemParam();

        switch(SystemParam) {
            case SPI_SETDRAGFULLWINDOWS:
            {
                const unsigned expected = 1 /* Body(1) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETDRAGFULLWINDOWS - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint8_t const Body = chunk.in_uint8();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETDRAGFULLWINDOWS - "
                            "Full Window Drag is %s.",
                        (!Body ? "disabled" : "enabled"));
                }
            }
            break;

            case SPI_SETKEYBOARDCUES:
            {
                const unsigned expected = 1 /* Body(1) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETKEYBOARDCUES - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint8_t const Body = chunk.in_uint8();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    if (Body) {
                        LOG(LOG_INFO,
                            "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                                "SPI_SETKEYBOARDCUES - "
                                "Menu Access Keys are always underlined.");
                    }
                    else {
                        LOG(LOG_INFO,
                            "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                                "SPI_SETKEYBOARDCUES - "
                                "Menu Access Keys are underlined only when the menu is activated by the keyboard.");
                    }
                }
            }
            break;

            case SPI_SETKEYBOARDPREF:
            {
                const unsigned expected = 1 /* Body(1) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETKEYBOARDPREF - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint8_t const Body = chunk.in_uint8();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    if (Body) {
                        LOG(LOG_INFO,
                            "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                                "SPI_SETKEYBOARDPREF - "
                                "The user prefers the keyboard over mouse.");
                    }
                    else {
                        LOG(LOG_INFO,
                            "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                                "SPI_SETKEYBOARDPREF - "
                                "The user does not prefer the keyboard over mouse.");
                    }
                }
            }
            break;

            case SPI_SETMOUSEBUTTONSWAP:
            {
                const unsigned expected = 1 /* Body(1) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETMOUSEBUTTONSWAP - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint8_t Body = chunk.in_uint8();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    if (Body) {
                        LOG(LOG_INFO,
                            "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                                "SPI_SETMOUSEBUTTONSWAP - "
                                "Swaps the meaning of the left and right mouse buttons.");
                    }
                    else {
                        LOG(LOG_INFO,
                            "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                                "SPI_SETMOUSEBUTTONSWAP - "
                                "Restores the meaning of the left and right mouse buttons to their original meanings.");
                    }
                }
            }
            break;

            case SPI_SETWORKAREA:
            {
                const unsigned expected = 8 /* Body(8) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETWORKAREA - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint16_t Left   = chunk.in_uint16_le();
                uint16_t Top    = chunk.in_uint16_le();
                uint16_t Right  = chunk.in_uint16_le();
                uint16_t Bottom = chunk.in_uint16_le();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETWORKAREA - "
                            "work area in virtual screen coordinates is (left=%u top=%u right=%u bottom=%u).",
                        Left, Top, Right, Bottom);
                }
            }
            break;

            case RAIL_SPI_DISPLAYCHANGE:
            {
                const unsigned expected = 8 /* Body(8) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "RAIL_SPI_DISPLAYCHANGE - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint16_t Left   = chunk.in_uint16_le();
                uint16_t Top    = chunk.in_uint16_le();
                uint16_t Right  = chunk.in_uint16_le();
                uint16_t Bottom = chunk.in_uint16_le();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "RAIL_SPI_DISPLAYCHANGE - "
                            "New display resolution in virtual screen coordinates is (left=%u top=%u right=%u bottom=%u).",
                        Left, Top, Right, Bottom);
                }
            }
            break;

            case RAIL_SPI_TASKBARPOS:
            {
                const unsigned expected = 8 /* Body(8) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "RAIL_SPI_TASKBARPOS - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        cspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint16_t Left   = chunk.in_uint16_le();
                uint16_t Top    = chunk.in_uint16_le();
                uint16_t Right  = chunk.in_uint16_le();
                uint16_t Bottom = chunk.in_uint16_le();

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "RAIL_SPI_TASKBARPOS - "
                            "Size of the client taskbar is (left=%u top=%u right=%u bottom=%u).",
                        Left, Top, Right, Bottom);
                }
            }
            break;

            case SPI_SETHIGHCONTRAST:
            {
                HighContrastSystemInformationStructure_Recv hcsisr(chunk);

                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "SPI_SETHIGHCONTRAST - "
                            "Parameters for the high-contrast accessibility feature, Flags=0x%X, ColorScheme=\"%s\".",
                        hcsisr.Flags(), hcsisr.ColorScheme());
                }
            }
            break;

            default:
                if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
                    LOG(LOG_INFO,
                        "RemoteProgramsVirtualChannel::process_client_system_parameters_update_pdu: "
                            "Delivering unprocessed client system parameter %s(%u) to server.",
                        get_RAIL_ClientSystemParam_name(SystemParam),
                        SystemParam);
                }
            break;
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

        HandshakePDU_Recv hspdur(chunk);

        if (this->verbose & MODRDP_LOGLEVEL_RAIL) {
            LOG(LOG_INFO,
                "RemoteProgramsVirtualChannel::process_server_handshake_pdu: "
                    "buildNumber=%u", hspdur.buildNumber());
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

        ServerSystemParametersUpdatePDU_Recv sspupdur(chunk);

        uint32_t SystemParam = sspupdur.SystemParam();

        switch(SystemParam) {
            case SPI_SETSCREENSAVEACTIVE:
            {
                const unsigned expected = 1 /* Body(1) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_server_system_parameters_update_pdu: "
                            "SPI_SETSCREENSAVEACTIVE - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        sspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint8_t const Body = chunk.in_uint8();

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
                const unsigned expected = 1 /* Body(1) */;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "RemoteProgramsVirtualChannel::process_server_system_parameters_update_pdu: "
                            "SPI_SETSCREENSAVESECURE - "
                            "expected=%u remains=%zu (0x%04X)",
                        expected, chunk.in_remain(),
                        sspupdur.SystemParam());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }

                uint8_t const Body = chunk.in_uint8();

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
