/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the impl_graphicied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#pragma once

#include "utils/log.hpp"
#include "core/RDP/remote_programs.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/channel_list.hpp"

#include "client_redemption/client_redemption_config.hpp"
#include "client_redemption/client_input_output_api/client_graphic_api.hpp"
#include "client_redemption/client_input_output_api/client_mouse_keyboard_api.hpp"


// [MS-RDPERP]: Remote Desktop Protocol: Remote Programs Virtual Channel Extension
//
//
// 1.3.2 Message Flows
//
// This topic contains a number of related sections. The list below provides links to each section.
//
//     1.3.2.1 RAIL Session Connection
//     1.3.2.2 RAIL Session Disconnection and Reconnection
//     1.3.2.3 RAIL Server/Client Synchronization
//     1.3.2.4 RAIL Virtual Channel Messages
//     1.3.2.5 RAIL Local Move/Resize
//
//
// 1.3.2.1 RAIL Session Connection
//
// RAIL connection establishment follows the Remote Desktop Protocol: Basic Connectivity and Graphics Remoting connection establishment sequence (as specified in [MS-RDPBCGR] section 1.3.1.1). RAIL-specific information during connection establishment is outlined as follows:
//
//     The client creates and initializes a static virtual channel to be used for RAIL protocol messages. Information regarding this channel is sent to the server in the Client MCS Connect Initial PDU with GCC Conference Create Request (as specified in [MS-RDPBCGR] section 2.2.1.3).
//
//     The Client Info PDU (as specified in [MS-RDPBCGR] section 2.2.1.11) indicates the client's request to establish a RAIL connection.
//
//     The Alternate Shell field of the Client Info PDU, as specified in [MS-RDPBCGR] section 2.2.1.11, is NOT used to communicate the initial application started in the session. Instead, the initial application information is communicated to the server via the Client Execute PDU.
//
//     If the server supports RAIL, the Demand Active PDU has to contain the Remote Programs Capability Set and Window List Capability Set to indicate that it supports RAIL.
//
//     The client sends corresponding Remote Programs Capability Set and Window Capability Set in the Confirm Active PDU.
//
//     If, in the Demand Active PDU, the server does not indicate that it supports RAIL, the client requests a disconnection according to the Remote Desktop Protocol: Basic Connectivity and Graphics Remoting ([MS-RDPBCGR] section 1.3.1.4.1). Likewise, if the client does not indicate that it supports RAIL in the Confirm Active PDU, the server disconnects the client (see [MS-RDPBCGR] section 1.3.1.4.2).
//
// After the RDP connection is established, a RAIL client and server exchange Handshake PDUs over the RAIL Virtual Channel to indicate that each is ready for data on the virtual channel.
//
// +-----------+                                                 +-----------+
// |  Client   |                                                 |  Server   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +-----------------------HANDSHAKE PDU-----------------------> |
//       |                                                             |
//       | <---------------------HANDSHAKE PDU-------------------------+
//       |                                                             |
//
// Figure 1: Handshake PDU
//
//
// 1.3.2.2 RAIL Session Disconnection and Reconnection
//
// RAIL Session Disconnection and RAIL Session Reconnection follow the corresponding Remote Desktop Protocol: Basic Connectivity and Graphics Remoting sequences, as specified in [MS-RDPBCGR] section 1.3.1.4 (Disconnection Sequences) and [MS-RDPBCGR] section 1.3.1.5 (Automatic Reconnection).
//
//
// 1.3.2.3 RAIL Server/Client Synchronization
//
// A RAIL server synchronizes with the RAIL client over the RDP channel upon connection establishment or when a desktop switch occurs.
//
// +-----------+                                                      +-----------+
// |  Client   |                                                      |  Server   |
// |           |                                                      |           |
// +-----+-----+                                                      +-----+-----+
//       |                                                                  |
//       |                                                                  |
//       | <--------Alternate Secondary Order - Begin desktop sync----------+
//       |                                                                  |
//       | <---Alternative Secondary Order - Window, notify icon, desktop---+
//       |                                                                  |
//       | <---------Alternate Secondary Order - End desktop sync-----------+
//       |                                                                  |
//
// Figure 2: RAIL protocol client synchronization
//
// The synchronization begins with a Desktop Information Order with the WINDOW_ORDER_FIELD_DESKTOP_ARC_BEGAN(0x00000008) flag set in the Hdr field (section 2.2.1.3.3.2.2). Upon receipt of this order, the client clears all previously received information from the server. This order is followed by any number of Windowing Alternate Secondary Drawing Orders describing windows, notification icons, and desktop. Finally, the server sends a Desktop Information Order with the WINDOW_ORDER_FIELD_DESKTOP_ARC_COMPLETED (0x00000004) flag set to signal the end of synchronization data (section 2.2.1.3.3.2.1).
//
// After the initial synchronization, Windowing Alternate Secondary Drawing Orders flow from server to client whenever a change occurs in a window, notification icon, or desktop state.
//
// If the server is not capable of monitoring the desktop (for example, secure desktop), it sends a Desktop Information Order with the WINDOW_ORDER_FIELD_DESKTOP_NONE (0x00000001) flag set in the Hdr field (section 2.2.1.3.3.2.2). Upon receipt of this order, the client clears out all previously received information from the server.
//
//
// 1.3.2.4 RAIL Virtual Channel Messages
//
// Client/server or server/client messages can flow over the RAIL anytime after the virtual channel handshake sequence (section 2.2.2.2.1). The client sends the Client Information PDU and the Client System Parameters Update PDU immediately after the handshake to inform the server of its state and system parameters. If the client includes the TS_RAIL_CLIENTSTATUS_ZORDER_SYNC (0x00000004) flag in the Client Information PDU, then the server creates the marker window (section 3.3.1.3) and sends the ID of this window to the client using the Z-Order Sync Information PDU (section 2.2.2.11.1). If the client includes the TS_RAIL_CLIENTSTATUS_WINDOW_RESIZE_MARGIN_SUPPORTED (0x00000010) flag in the Client Information PDU, then the server sends the dimensions of the window resize margins in the Window Information PDU (section 2.2.1.3.1). If the client includes the TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED (0x00000040) flag in the Client Information PDU, then the server sends the registration state and edge messages for application desktop toolbars in the Window Information PDU (section 2.2.1.3.1). The server sends the Server System Parameters Update PDU immediately after the handshake to inform the client of its system parameters. All other virtual channel messages are generated in response to events on the client or server. If the client includes the TS_RAIL_CLIENTSTATUS_POWER_DISPLAY_REQUEST_SUPPORTED (0x00000080) flag in the Client Information PDU, then the server sends display-required power requests to the client using the Power Display Request PDU (section 2.2.2.13.1).
//
//
// 1.3.2.5 RAIL Local Move/Resize
//
// Local move/resize features are RAIL options designed to optimize bandwidth in certain situations where RAIL windows are moved or resized by the user. A RAIL client indicates to the RAIL server whether it supports local move/resize through the Client Capabilities PDU (section 2.2.2.2.2), sent after the Virtual Channel handshake sequence. RAIL servers do not have to explicitly report move/size support to the client.
//
// +-----------+                                                 +-----------+
// |  Client   |                                                 |  Server   |
// |           |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       | <----------------Server Min Max Info PDU--------------------+
//       |                                                             |
//       | <---------------Server Move/Size Start PDU------------------+
//       |                                                             |
//       +-------------------Client Window Move PDU------------------> |
//       |                                                             |
//       | <----------------Server Move/Size End PDU-------------------+
//       |                                                             |
//
// Figure 3: RAIL local move/resize operation
//
// Local move/resize is based on the following logic:
//
//     When the server detects that a window is beginning to be moved or resized, it sends a Server Min Max Info PDU (section 2.2.2.7.1) to the client with the window extents. This is followed by a Server Move/Size Start PDU (section 2.2.2.7.2).
//
//     If the client supports local move/resize, it injects a mouse button-down at the position indicated by the move/size PDU (if the move/size was initiated via mouse) or posts a command to the window (if the move/size was initiated via keyboard) to initiate move/resize of the window by the local window manager.
//
//     At the same time, the client lets the local Window Manager handle all keyboard and mouse events for the RAIL window, instead of redirecting to the server, to ensure that the move/size is entirely happening locally.
//
//     Finally, when the user is done with the move/resize, the local RAIL window receives this notification and forwards a mouse button-up to the server to end move/size on the server. For keyboard-based moves and all resize operations, the client also sends a Client Window Move PDU (section 2.2.2.7.4) to the server to inform the server of the window's new position and size. (For mouse-based moves, the mouse button-up is sufficient to inform the window's final position).
//
//     When the server detects that move/size has ended, it sends a Server Move/Size End PDU (section 2.2.2.7.3) with the final window position and size. The client can adjust its local RAIL window if necessary using this information.



class ClientChannelRemoteAppManager {

    RDPVerbose verbose;

    ClientRedemptionAPI * client;
    ClientOutputGraphicAPI * impl_graphic;
    ClientInputMouseKeyboardAPI * impl_input;

    std::string source_of_ExeOrFile;
    std::string source_of_WorkingDir;
    std::string source_of_Arguments;


public:

//     struct RailChannelData {
//         uint32_t clientWindowID;
        uint32_t ServerWindowID;

        uint32_t WindowIDToShow = 0;

        bool ExecuteResult = false;

        std::vector<uint32_t> z_order;

        int build_number = 0;

        int width = 0;
        int height = 0;

//     } rail_channel_data;


// public:
    ClientChannelRemoteAppManager(RDPVerbose verbose,
                                  ClientRedemptionAPI * client,
                                  ClientOutputGraphicAPI * impl_graphic,
                                  ClientInputMouseKeyboardAPI * impl_input)
      : verbose(verbose)
      , client(client)
      , impl_graphic(impl_graphic)
      , impl_input(impl_input)
      {}



    void set_configuration(int width, int height, RDPRemoteAppConfig & config) {
        this->width  = width;
        this->height = height;

        this->source_of_ExeOrFile  = config.source_of_ExeOrFile;
        this->source_of_WorkingDir = config.source_of_WorkingDir;
        this->source_of_Arguments  = config.source_of_Arguments;
    }

    void clear() {
        this->z_order.clear();
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) {

        uint32_t win_id = cmd.header.WindowId();

        switch (win_id) {

            case RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_0:
            case RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_1:
            case RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_2:
            case RemoteProgramsWindowIdManager::INVALID_WINDOW_ID:
                break;

//             case 0x800000:
//                 this->z_order.clear();
//                 this->impl_graphic->clear_remote_app_screen();
//                 //LOG(LOG_INFO, "RAIL::DeletedWindow  Last App has been close - mod rdp disconnection.");
//                 this->client->disconnect("", false);
//                 break;

            default:
                if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_OWNER) {
                        //cmd.log(LOG_INFO);

                    if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET) {}

                    if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE) {
                        if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET) {

                                this->impl_graphic->create_remote_app_screen(win_id, cmd.WindowWidth(), cmd.WindowHeight(), cmd.WindowOffsetX(), cmd.WindowOffsetY());
                                this->impl_input->refreshPressed();
                        }
                    }

                    if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_SHOW) {
                        if (cmd.ShowState()) {
                            this->impl_graphic->show_screen(win_id);
                        }
                    }
                } else {
                    if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET) {

                        int x_offset = cmd.WindowOffsetX();
                        int y_offset = cmd.WindowOffsetY();

                        if (cmd.WindowOffsetY() < 0) {
                            this->impl_graphic->set_screen_size(win_id, this->impl_graphic->get_visible_width(win_id), this->impl_graphic->get_mem_height(win_id) + cmd.WindowOffsetY());
                            y_offset = 0;
                        } else {
                            if (this->impl_graphic->get_visible_height(win_id) != this->impl_graphic->get_mem_height(win_id)) {
                                this->impl_graphic->set_screen_size(win_id, this->impl_graphic->get_visible_width(win_id), this->impl_graphic->get_mem_height(win_id));
                            }

                            if ((cmd.WindowOffsetY() + this->impl_graphic->get_visible_height(win_id)) > this->impl_graphic->screen_max_height && !(cmd.WindowOffsetY() == 0)) {
                                this->impl_graphic->set_screen_size(win_id, this->impl_graphic->get_visible_width(win_id), 1+this->impl_graphic->screen_max_height - y_offset);
                                y_offset = this->impl_graphic->screen_max_height - this->impl_graphic->get_visible_height(win_id);
                            } else {
                                if (this->impl_graphic->get_visible_height(win_id) != this->impl_graphic->get_mem_height(win_id)) {
                                    int current_height = this->impl_graphic->screen_max_height - y_offset;
                                    if (current_height > this->impl_graphic->get_mem_height(win_id)) {
                                        current_height = this->impl_graphic->get_mem_height(win_id);
                                    }
                                    this->impl_graphic->set_screen_size(win_id,  this->impl_graphic->get_visible_width(win_id), current_height);
                                }
                            }
                        }

                        if (cmd.WindowOffsetX() < 0) {
                            this->impl_graphic->set_screen_size(win_id, this->impl_graphic->get_mem_width(win_id) + cmd.WindowOffsetX(), this->impl_graphic->get_visible_height(win_id));
                            x_offset = 0;

                        } else {
                            if (this->impl_graphic->get_visible_width(win_id) != this->impl_graphic->get_mem_width(win_id)) {
                                this->impl_graphic->set_screen_size(win_id, this->impl_graphic->get_mem_width(win_id), this->impl_graphic->get_visible_height(win_id));
                            }

                            if ((cmd.WindowOffsetX() + this->impl_graphic->get_visible_width(win_id)) > this->impl_graphic->screen_max_width) {
                                this->impl_graphic->set_screen_size(win_id, 1+this->impl_graphic->screen_max_width - x_offset, this->impl_graphic->get_visible_height(win_id));
                                x_offset = this->impl_graphic->screen_max_width - this->impl_graphic->get_visible_width(win_id);
                            } else {
                                if (this->impl_graphic->get_visible_width(win_id) != this->impl_graphic->get_mem_width(win_id)) {
                                    int current_width= this->impl_graphic->screen_max_width - y_offset;
                                    if (current_width > this->impl_graphic->get_mem_width(win_id)) {
                                        current_width = this->impl_graphic->get_mem_width(win_id);
                                    }
                                    this->impl_graphic->set_screen_size(win_id, current_width, this->impl_graphic->get_visible_height(win_id));
                                }
                            }
                        }

                        this->impl_graphic->set_pixmap_shift(win_id, x_offset, y_offset);
                        this->impl_graphic->move_screen(win_id, x_offset, y_offset);
                    }

                    if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE) {

                        this->impl_graphic->set_screen_size(win_id, cmd.WindowWidth(), cmd.WindowHeight());
                        this->impl_graphic->set_mem_size(win_id, cmd.WindowWidth(), cmd.WindowHeight());

                        this->impl_input->refreshPressed();
                    }
                }
                break;
        }
    }


    void draw(const RDP::RAIL::ActivelyMonitoredDesktop  & cmd) {

        if (cmd.header.FieldsPresentFlags() & RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER) {
            this->z_order.clear();
            if (this->ExecuteResult) {
                for (size_t i = 0; i < cmd.NumWindowIds(); i++) {
                    this->z_order.push_back(cmd.window_ids(i));
                }
            }
        }
    }


    void draw(const RDP::RAIL::DeletedWindow            & cmd) {
        int elem_to_erase = -1;

        for (size_t i = 0; i < this->z_order.size(); i++) {
            if (this->z_order[i] == cmd.header.WindowId()) {
                elem_to_erase = i;
            }
        }

        if (elem_to_erase != -1) {
            this->z_order.erase(this->z_order.begin()+elem_to_erase);
        }

        if ( this->z_order.size() <= 1) {
            this->impl_graphic->clear_remote_app_screen();
            this->client->disconnect("", false);
        } else {
            this->impl_input->refreshPressed();
        }
    }


    void receive(InStream & stream) {
        if (!this->impl_graphic || !this->impl_input) {
            return;
        }

        LOG(LOG_INFO,  "ClientChannelRemoteAppManager::receive");
        RAILPDUHeader header;
        header.receive(stream);

        switch (header.orderType()) {

            case TS_RAIL_ORDER_HANDSHAKE:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_HANDSHAKE");
                }
                {
                //this->impl_graphic->screen->hide();

                HandshakePDU hspdu;
                hspdu.receive(stream);
                this->build_number = hspdu.buildNumber();

                }
                break;

            case TS_RAIL_ORDER_SYSPARAM:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                }
                {
                ServerSystemParametersUpdatePDU sspu;
                sspu.receive(stream);
                sspu.log(LOG_INFO);

                if (sspu.SystemParam() == SPI_SETSCREENSAVEACTIVE) {

                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(9);                //(20*4) + (15*2) + 4);

                    out_stream.out_uint32_le(SPI_SETDRAGFULLWINDOWS);
                    out_stream.out_uint8(1);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(18);

                    out_stream.out_uint32_le(SPI_SETHIGHCONTRAST);
                    out_stream.out_uint32_le(0x7e);
                    out_stream.out_uint32_le(2);
                    out_stream.out_uint16_le(0);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(9);

                    out_stream.out_uint32_le(SPI_SETKEYBOARDCUES);
                    out_stream.out_uint8(0);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(9);

                    out_stream.out_uint32_le(SPI_SETKEYBOARDPREF);
                    out_stream.out_uint8(0);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(16);

                    out_stream.out_uint32_le(SPI_SETWORKAREA);
                    out_stream.out_uint16_le(0);
                    out_stream.out_uint16_le(0);
                    out_stream.out_uint16_le(this->width);
                    out_stream.out_uint16_le(this->height);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(9);

                    out_stream.out_uint32_le(SPI_SETMOUSEBUTTONSWAP);
                    out_stream.out_uint8(0);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(16);

                    out_stream.out_uint32_le(RAIL_SPI_TASKBARPOS);
                    out_stream.out_uint16_le(0);
                    out_stream.out_uint16_le(560);
                    out_stream.out_uint16_le(800);
                    out_stream.out_uint16_le(600);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(12);

                    out_stream.out_uint32_le(SPI_SETCARETWIDTH);
                    out_stream.out_uint32_le(1);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(12);

                    out_stream.out_uint32_le(SPI_SETSTICKYKEYS);
                    out_stream.out_uint32_le(1);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(12);

                    out_stream.out_uint32_le(SPI_SETTOGGLEKEYS);
                    out_stream.out_uint32_le(1);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_SYSPARAM);
                    out_stream.out_uint16_le(28);

                    out_stream.out_uint32_le(SPI_SETFILTERKEYS);
                    out_stream.out_uint32_le(1);
                    out_stream.out_uint32_le(1);
                    out_stream.out_uint32_le(1);
                    out_stream.out_uint32_le(1);
                    out_stream.out_uint32_le(1);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSPARAM");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;
                    out_stream.out_uint16_le(TS_RAIL_ORDER_CLIENTSTATUS);
                    out_stream.out_uint16_le(8);
                    out_stream.out_uint32_le( TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE
                                            | TS_RAIL_CLIENTSTATUS_AUTORECONNECT
                                            //| TS_RAIL_CLIENTSTATUS_ZORDER_SYNC
                                            //| TS_RAIL_CLIENTSTATUS_WINDOW_RESIZE_MARGIN_SUPPORTED
                                            //| TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED
                                            );

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                  CHANNELS::CHANNEL_FLAG_FIRST |
                                                  CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_CLIENTSTATUS");
                    }
                    }
                    {
                    StaticOutStream<32> out_stream;

                    out_stream.out_uint16_le(TS_RAIL_ORDER_HANDSHAKE);
                    out_stream.out_uint16_le(8);
                    out_stream.out_uint32_le(this->build_number);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                  , chunk_to_send
                                                  , out_stream.get_offset()
                                                  , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |
                                                    CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                  );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_HANDSHAKE");
                    }
                    }
//                     {
//                     StaticOutStream<32> out_stream;;
//
//                     out_stream.out_uint16_le(TS_RAIL_ORDER_LANGBARINFO);
//                     out_stream.out_uint16_le(8);
//
//                     out_stream.out_uint32_le(0x491);
//
//
//                     InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
//
//                     // this->mod_rail_hosted
//                     this->client->mod->send_to_mod_channel( channel_names::rail
//                                                 , chunk_to_send
//                                                 , out_stream.get_offset()
//                                                 , CHANNELS::CHANNEL_FLAG_LAST |
//                                                   CHANNELS::CHANNEL_FLAG_FIRST |
//                                                   CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
//                                                 );
//
//                     LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_LANGBARINFO");
//                     }

                    {
                    StaticOutStream<1600> out_stream;

                    const char * source_of_ExeOrFile = this->source_of_ExeOrFile.c_str();
                    uint8_t unicode_ExeOrFile[500];
                    const size_t size_of_unicode_ExeOrFile = ::UTF8toUTF16(byte_ptr_cast(source_of_ExeOrFile), unicode_ExeOrFile, 500);

                    const char * source_of_WorkingDir = this->source_of_WorkingDir.c_str();
                    uint8_t unicode_WorkingDir[500];
                    const size_t size_of_unicode_WorkingDir = ::UTF8toUTF16(byte_ptr_cast(source_of_WorkingDir), unicode_WorkingDir, 500);

                    const char * source_of_Arguments = this->source_of_Arguments.c_str();
                    uint8_t unicode_Arguments[500];
                    const size_t size_of_unicode_Arguments = ::UTF8toUTF16(byte_ptr_cast(source_of_Arguments), unicode_Arguments, 500);

                    out_stream.out_uint16_le(TS_RAIL_ORDER_EXEC);
                    out_stream.out_uint16_le(12 + size_of_unicode_ExeOrFile + size_of_unicode_WorkingDir +size_of_unicode_Arguments);

                    out_stream.out_uint16_le(TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY | TS_RAIL_EXEC_FLAG_APP_USER_MODEL_ID | TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS);
                    out_stream.out_uint16_le(size_of_unicode_ExeOrFile);
                    out_stream.out_uint16_le(size_of_unicode_WorkingDir);
                    out_stream.out_uint16_le(size_of_unicode_Arguments);
                    out_stream.out_copy_bytes(unicode_ExeOrFile, size_of_unicode_ExeOrFile);
                    out_stream.out_copy_bytes(unicode_WorkingDir, size_of_unicode_WorkingDir);
                    out_stream.out_copy_bytes(source_of_Arguments, size_of_unicode_Arguments);

                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                    this->client->mod->send_to_mod_channel( channel_names::rail
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );
                    if (bool(this->verbose & RDPVerbose::rail)) {
                        LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_EXEC");
                    }
                    }
                }
                }
                break;

            case TS_RAIL_ORDER_HANDSHAKE_EX:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_HANDSHAKE_EX");
                }
                break;

//             case TS_RAIL_ORDER_CLIENTSTATUS:
//                 LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_CLIENTSTATUS");
//                 break;

            case TS_RAIL_ORDER_GET_APPID_RESP:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_GET_APPID_RESP");
                }
                {
                ServerGetApplicationIDResponsePDU sgaior;
                sgaior.receive(stream);
                sgaior.log(LOG_INFO);
                this->ServerWindowID = sgaior.WindowId();
                }
//                 {
//                     StaticOutStream<32> out_stream;
//                     out_stream.out_uint16_le(TS_RAIL_ORDER_SYSCOMMAND);
//                     out_stream.out_uint16_le(9);
//                     out_stream.out_uint32_le(this->ServerWwindowID);
//                     out_stream.out_uint16_le(SC_MINIMIZE);
//
//                     InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
//
//                     this->client->mod->send_to_mod_channel( channel_names::rail
//                                                     , chunk_to_send
//                                                     , out_stream.get_offset()
//                                                     , CHANNELS::CHANNEL_FLAG_LAST |
//                                                     CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
//                                                     );
//                     LOG(LOG_INFO, "CLIENT >> RAIL CHANNEL TS_RAIL_ORDER_SYSCOMMAND");
//                 }
                break;

            case TS_RAIL_ORDER_EXEC_RESULT:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_EXEC_RESULT");
                }
                {
                    ServerExecuteResultPDU res_pdu;
                    res_pdu.receive(stream);
                    res_pdu.log(LOG_INFO);
                    if (res_pdu.ExecResult() == RAIL_EXEC_S_OK) {
                        this->ExecuteResult = true;
                    }
                }
                break;

            case TS_RAIL_ORDER_LANGBARINFO:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_INFO, "SERVER >> RAIL CHANNEL TS_RAIL_ORDER_LANGBARINFO");
                }

                break;

            default:
                if (bool(this->verbose & RDPVerbose::rail)) {
                    LOG(LOG_WARNING, "SERVER >> RAIL CHANNEL DEFAULT 0x%04x %s", header.orderType(), get_RAIL_orderType_name(header.orderType()));
                }
                break;
        }
    }
};
