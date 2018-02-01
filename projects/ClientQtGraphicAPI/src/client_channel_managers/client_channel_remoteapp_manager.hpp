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

#include "../qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"



class ClientChannelRemoteAppManager {

    RDPVerbose verbose;

    ClientRedemptionIOAPI * client;

    ClientOutputGraphicAPI * impl_graphic;

    ClientInputMouseKeyboardAPI * impl_input;

    struct RailChannelData {
//         uint32_t clientWindowID;
        uint32_t ServerWindowID;

        uint32_t WindowIDToShow = 0;

        bool ExecuteResult = false;

        std::vector<uint32_t> z_order;


        int build_number = 0;

    } rail_channel_data;

public:
    ClientChannelRemoteAppManager(RDPVerbose verbose,
                                  ClientRedemptionIOAPI * client,
                                  ClientOutputGraphicAPI * impl_graphic,
                                  ClientInputMouseKeyboardAPI * impl_input)
      : verbose(verbose)
      , client(client)
      , impl_graphic(impl_graphic)
      , impl_input(impl_input)
      {}

    void clear() {
        this->rail_channel_data.z_order.clear();
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) {

        uint32_t win_id = cmd.header.WindowId();

        switch (win_id) {

            case RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_0:
            case RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_1:
            case RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_2:
            case RemoteProgramsWindowIdManager::INVALID_WINDOW_ID:
                break;

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
                   // }
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
            this->rail_channel_data.z_order.clear();
            if (this->rail_channel_data.ExecuteResult) {
                for (size_t i = 0; i < cmd.NumWindowIds(); i++) {
                    this->rail_channel_data.z_order.push_back(cmd.window_ids(i));
                }
            }
        }
    }


    void draw(const RDP::RAIL::DeletedWindow            & cmd) {

        uint32_t win_id = cmd.header.WindowId();

        this->impl_graphic->dropScreen(win_id);

        int elem_to_erase = -1;

        for (size_t i = 0; i < this->rail_channel_data.z_order.size(); i++) {
            if (this->rail_channel_data.z_order[i] == cmd.header.WindowId()) {
                elem_to_erase = i;
            }
        }

        if (elem_to_erase != -1) {
            this->rail_channel_data.z_order.erase(this->rail_channel_data.z_order.begin()+elem_to_erase);
        }

        if ( this->rail_channel_data.z_order.size() == 0) {
            this->impl_graphic->clear_remote_app_screen();
            this->client->disconnect("");

        } else {

            this->impl_input->refreshPressed();

        }
    }


    void receive(InStream & stream) {
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
                this->rail_channel_data.build_number = hspdu.buildNumber();

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

                if (sspu.SystemParam() ==  SPI_SETSCREENSAVEACTIVE) {

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
                    out_stream.out_uint16_le(this->client->info.width);
                    out_stream.out_uint16_le(this->client->info.height);

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
                    StaticOutStream<32> out_stream;;

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
                    out_stream.out_uint32_le(this->rail_channel_data.build_number);

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

                    const char * source_of_ExeOrFile = "C:\\Windows\\system32\\notepad.exe";
                    uint8_t unicode_ExeOrFile[500];
                    const size_t size_of_unicode_ExeOrFile = ::UTF8toUTF16(reinterpret_cast<const uint8_t *>(source_of_ExeOrFile), unicode_ExeOrFile, 500);

                    const char * source_of_WorkingDir = "C:\\Users\\user1";
                    uint8_t unicode_WorkingDir[500];
                    const size_t size_of_unicode_WorkingDir = ::UTF8toUTF16(reinterpret_cast<const uint8_t *>(source_of_WorkingDir), unicode_WorkingDir, 500);

                    const char * source_of_Arguments = "";
                    uint8_t unicode_Arguments[500];
                    const size_t size_of_unicode_Arguments = ::UTF8toUTF16(reinterpret_cast<const uint8_t *>(source_of_Arguments), unicode_Arguments, 500);

                    out_stream.out_uint16_le(TS_RAIL_ORDER_EXEC);
                    out_stream.out_uint16_le(12 + size_of_unicode_ExeOrFile + size_of_unicode_WorkingDir +size_of_unicode_Arguments);

                    out_stream.out_uint16_le(TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY | TS_RAIL_EXEC_FLAG_APP_USER_MODEL_ID |  TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS);
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
                this->rail_channel_data.ServerWindowID = sgaior.WindowId();
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
                        this->rail_channel_data.ExecuteResult = true;
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