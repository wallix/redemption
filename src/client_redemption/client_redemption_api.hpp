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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "core/front_api.hpp"

#include <chrono>
#include <string>
#include <ctime>

class ClientInputSocketAPI
{
    // TODO private
public:
    mod_api * _callback = nullptr;

    virtual bool start_to_listen(int client_sck, mod_api * mod) = 0;
    virtual void disconnect() = 0;

    virtual ~ClientInputSocketAPI() = default;
};

class ClientRedemptionAPI : public FrontAPI
{

public:
    virtual ~ClientRedemptionAPI() = default;

    bool can_be_start_capture() override { return true; }
    void send_to_channel( const CHANNELS::ChannelDef &  /*channel*/, uint8_t const *
                         /*data*/, std::size_t  /*length*/, std::size_t  /*chunk_size*/, int  /*flags*/) override {}

    virtual bool is_connected() {return false;}
    virtual int wait_and_draw_event(std::chrono::milliseconds timeout) = 0;
    virtual void callback(bool /*is_timeout*/) {}

    // CONTROLLER
    virtual void close() = 0;
    virtual void connect(const std::string& /*ip*/, const std::string& /*name*/, const std::string& /*pwd*/, const int /*port*/) {}
    virtual void disconnect(std::string const & /*unused*/, bool /*unused*/) {}
    virtual void update_keylayout() {}

    // Replay functions
    virtual void replay( const std::string & /*unused*/) {}
    virtual bool load_replay_mod(timeval /*unused*/, timeval /*unused*/) { return false; }
    virtual timeval reload_replay_mod(int /*unused*/, timeval /*unused*/) { return timeval{}; }
    virtual void delete_replay_mod() {}
    virtual void instant_play_client(std::chrono::microseconds /*time*/) {}

};



