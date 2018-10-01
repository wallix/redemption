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

#ifndef Q_MOC_RUN

#include "utils/log.hpp"
#include "mod/rdp/rdp_log.hpp"

#include "configs/config.hpp"

#include "core/front_api.hpp"
#include "core/report_message_api.hpp"

#include "mod/internal/client_execute.hpp"
#include "mod/mod_api.hpp"

#include "mod/internal/replay_mod.hpp"
#include "transport/replay_transport.hpp"

#endif


class ClientRedemptionAPI : public FrontAPI
{
public:
    mod_api            * mod = nullptr;


public:

    ClientRedemptionAPI() {}

    virtual ~ClientRedemptionAPI() = default;
    virtual void send_clipboard_format() {}
    virtual bool is_connected() {return false;}
    void send_to_channel( const CHANNELS::ChannelDef &  /*channel*/, uint8_t const *
                         /*data*/, std::size_t  /*length*/, std::size_t  /*chunk_size*/, int  /*flags*/) override {}
    virtual int wait_and_draw_event(timeval timeout) = 0;

    // CONTROLLER
    virtual bool connect() {return true;}
    virtual void disconnect(std::string const & /*unused*/, bool /*unused*/) {}
    virtual void callback(bool /*is_timeout*/) {}
    virtual void draw_frame(int  /*unused*/) {}
    virtual void closeFromScreen() {}
    virtual void disconnexionReleased() {}
    virtual void update_keylayout() {}
    bool can_be_start_capture() override { return true; }

    // Replay functions
    virtual time_t get_movie_time_length(char const * /*unused*/) { return time_t{}; }
    virtual void instant_play_client(std::chrono::microseconds /*unused*/) {}
    virtual void replay(const std::string & /*unused*/, const std::string & /*unused*/) {}
    virtual bool load_replay_mod(std::string const & /*unused*/, std::string const & /*unused*/, timeval /*unused*/, timeval /*unused*/) { return false; }
    virtual timeval reload_replay_mod(int /*unused*/, timeval /*unused*/) { return timeval{}; }
    virtual bool is_replay_on() { return false; }
    virtual std::string get_mwrm_filename() { return ""; }
    virtual time_t get_real_time_movie_begin() { return time_t{}; }
    virtual void delete_replay_mod() {}
    virtual void replay_set_pause(timeval /*unused*/) {}
    virtual void replay_set_sync() {}
};


class ClientIO
{
public:
    ClientRedemptionAPI * client;

    void set_client(ClientRedemptionAPI * client) {
        this->client = client;
    }
};

