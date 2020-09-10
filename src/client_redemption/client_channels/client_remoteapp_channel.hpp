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

#include "gdi/graphic_api.hpp"
#include "mod/rdp/rdp_verbose.hpp"

class ClientCallback;
class ClientRedemptionConfig;
class ClientRedemptionAPI;
class ClientChannelMod;
class RDPRemoteAppConfig;
class InStream;

namespace RDP
{
    namespace RAIL
    {
        class NewOrExistingWindow;
        class ActivelyMonitoredDesktop;
        class DeletedWindow;
    }
}


class ClientRemoteAppGraphicAPI
{
protected:
    ClientCallback * controller;
    ClientRedemptionConfig * config;

public:
    const int screen_max_width;
    const int screen_max_height;

protected:
    ClientRedemptionAPI * client_replay;

public:
    ClientRemoteAppGraphicAPI(ClientCallback * controller, ClientRedemptionConfig * config, int max_width, int max_height)
      : controller(controller)
      , config(config)
      , screen_max_width(max_width)
      , screen_max_height(max_height)
      , client_replay(nullptr)
    {}

    virtual ~ClientRemoteAppGraphicAPI() = default;


    // remote app
    virtual void create_remote_app_screen(uint32_t /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/) {}
    virtual void move_screen(uint32_t /*unused*/, int /*unused*/, int /*unused*/) {}
    virtual void set_screen_size(uint32_t /*unused*/, int /*unused*/, int /*unused*/) {}
    virtual void set_pixmap_shift(uint32_t /*unused*/, int /*unused*/, int /*unused*/) {}
    virtual int get_visible_width(uint32_t /*unused*/) {return 0;}
    virtual int get_visible_height(uint32_t /*unused*/) {return 0;}
    virtual int get_mem_width(uint32_t /*unused*/) {return 0;}
    virtual int get_mem_height(uint32_t /*unused*/) {return 0;}
    virtual void set_mem_size(uint32_t /*unused*/, int /*unused*/, int /*unused*/) {}
    virtual void show_screen(uint32_t /*unused*/) {}
    virtual void clear_remote_app_screen() {}
};

class ClientRemoteAppChannel {

    RDPVerbose verbose;

    ClientCallback * callback;
    ClientChannelMod * channel_mod;
    ClientRemoteAppGraphicAPI * impl_graphic;

    std::string source_of_ExeOrFile;
    std::string source_of_WorkingDir;
    std::string source_of_Arguments;

    uint32_t ServerWindowID;
    bool ExecuteResult = false;
    std::vector<uint32_t> z_order;

public:
    int build_number = 0;

private:
    int width = 0;
    int height = 0;

public:
    ClientRemoteAppChannel(RDPVerbose verbose,
                           ClientCallback * callback,
                           ClientChannelMod * channel_mod)
    : verbose(verbose)
    , callback(callback)
    , channel_mod(channel_mod)
    {}

    void set_api(ClientRemoteAppGraphicAPI * impl_graphic) {
        this->impl_graphic = impl_graphic;
    }

    void set_configuration(int width, int height, RDPRemoteAppConfig & config);

    void clear() {
        this->z_order.clear();
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & cmd);
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop  & cmd);
    void draw(const RDP::RAIL::DeletedWindow & cmd);

    void receive(InStream & stream);
};

