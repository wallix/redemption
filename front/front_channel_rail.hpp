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

#ifndef _REDEMPTION_FRONT_FRONT_CHANNEL_RAIL_HPP_
#define _REDEMPTION_FRONT_FRONT_CHANNEL_RAIL_HPP_

void Front::process_rail_event(Callback & cb, uint8_t const * chunk, size_t chunk_size, uint16_t length, uint16_t flags) {
    StaticStream stream(chunk, chunk_size);

    uint16_t orderType   = stream.in_uint16_le();
    uint16_t orderLength = stream.in_uint16_le();

    //LOG(LOG_INFO, "Front::process_rail_event: orderType=%u orderLength=%u", orderType, orderLength);
    //hexdump_d(stream.get_data(), stream.size());

    switch (orderType) {
        case TS_RAIL_ORDER_EXEC:
        {
            ClientExecutePDU_Recv cepdur(stream);

            LOG(LOG_INFO,
                "Front::process_rail_event: Client Execute PDU "
                    "exe_or_file=\"%s\" working_dir=\"%s\" arguments=\"%s\"",
                cepdur.exe_or_file(), cepdur.working_dir(), cepdur.arguments());
        }
        break;

        case TS_RAIL_ORDER_SYSPARAM:
        {
            ClientSystemParametersUpdatePDU_Recv cspupdur(stream);

            switch(cspupdur.SystemParam()) {
                case SPI_SETDRAGFULLWINDOWS:
                {
                    const unsigned expected = 1 /* Body(1) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint8_t Body = stream.in_uint8();

                    LOG(LOG_INFO,
                        "Client System Parameters Update PDU: Full Window Drag is %s.",
                        (!Body ? "disabled" : "enabled"));
                }
                break;

                case SPI_SETKEYBOARDCUES:
                {
                    const unsigned expected = 1 /* Body(1) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint8_t Body = stream.in_uint8();

                    if (Body) {
                        LOG(LOG_INFO,
                            "Client System Parameters Update PDU: "
                                "Menu Access Keys are always underlined.");
                    }
                    else {
                        LOG(LOG_INFO,
                            "Client System Parameters Update PDU: "
                                "Menu Access Keys are underlined only when the menu is activated by the keyboard.");
                    }
                }
                break;

                case SPI_SETKEYBOARDPREF:
                {
                    const unsigned expected = 1 /* Body(1) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint8_t Body = stream.in_uint8();

                    if (Body) {
                        LOG(LOG_INFO,
                            "Client System Parameters Update PDU: "
                                "The user prefers the keyboard over mouse.");
                    }
                    else {
                        LOG(LOG_INFO,
                            "Client System Parameters Update PDU: "
                                "The user does not prefer the keyboard over mouse.");
                    }
                }
                break;

                case SPI_SETMOUSEBUTTONSWAP:
                {
                    const unsigned expected = 1 /* Body(1) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint8_t Body = stream.in_uint8();

                    if (Body) {
                        LOG(LOG_INFO,
                            "Client System Parameters Update PDU: "
                                "Swaps the meaning of the left and right mouse buttons.");
                    }
                    else {
                        LOG(LOG_INFO,
                            "Client System Parameters Update PDU: "
                                "Restores the meaning of the left and right mouse buttons to their original meanings.");
                    }
                }
                break;

                case SPI_SETWORKAREA:
                {
                    const unsigned expected = 8 /* Body(8) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint16_t Left   = stream.in_uint16_le();
                    uint16_t Top    = stream.in_uint16_le();
                    uint16_t Right  = stream.in_uint16_le();
                    uint16_t Bottom = stream.in_uint16_le();

                    LOG(LOG_INFO,
                        "Client System Parameters Update PDU: "
                            "work area in virtual screen coordinates is (left=%u left=%u left=%u left=%u).",
                        Left, Top, Right, Bottom);
                }
                break;

                case RAIL_SPI_DISPLAYCHANGE:
                {
                    const unsigned expected = 8 /* Body(8) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint16_t Left   = stream.in_uint16_le();
                    uint16_t Top    = stream.in_uint16_le();
                    uint16_t Right  = stream.in_uint16_le();
                    uint16_t Bottom = stream.in_uint16_le();

                    LOG(LOG_INFO,
                        "Client System Parameters Update PDU: "
                            "New display resolution in virtual screen coordinates is (left=%u left=%u left=%u left=%u).",
                        Left, Top, Right, Bottom);
                }
                break;

                case RAIL_SPI_TASKBARPOS:
                {
                    const unsigned expected = 8 /* Body(8) */;
                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Client System Parameters Update PDU: expected=%u remains=%u (0x%04X)",
                            expected, stream.in_remain(),
                            cspupdur.SystemParam());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }

                    uint16_t Left   = stream.in_uint16_le();
                    uint16_t Top    = stream.in_uint16_le();
                    uint16_t Right  = stream.in_uint16_le();
                    uint16_t Bottom = stream.in_uint16_le();

                    LOG(LOG_INFO,
                        "Client System Parameters Update PDU: "
                            "New display resolution in virtual screen coordinates is (left=%u left=%u left=%u left=%u).",
                        Left, Top, Right, Bottom);
                }
                break;

                case SPI_SETHIGHCONTRAST:
                {
                    HighContrastSystemInformationStructure_Recv hcsisr(stream);

                    LOG(LOG_INFO,
                        "Client System Parameters Update PDU: "
                            "parameters for the high-contrast accessibility feature, Flags=0x%X, ColorScheme=\"%s\".",
                        hcsisr.Flags(), hcsisr.ColorScheme());
                }
                break;
            }
        }
        break;

        case TS_RAIL_ORDER_CLIENTSTATUS:
        {
            ClientInformationPDU_Recv cipdur(stream);

            LOG(LOG_INFO, "Front::process_rail_event: Client Information PDU Flags=0x%08X",
                cipdur.Flags());
        }
        break;

        case TS_RAIL_ORDER_HANDSHAKE:
        {
            HandshakePDU_Recv hpdur(stream);

            LOG(LOG_INFO, "Front::process_rail_event: Handshake PDU buildNumber=%u", hpdur.buildNumber());
        }
        break;

        default:
            LOG(LOG_INFO, "Front::process_rail_event: orderType=%u orderLength=%u", orderType, orderLength);

            stream.rewind();

            cb.send_to_mod_channel(channel_names::rail, stream, length, flags);
        break;
    }
}

#endif  // #ifndef _REDEMPTION_FRONT_FRONT_CHANNEL_RAIL_HPP_
