/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan, Raphael Zhou
*/

#pragma once

#include "core/callback.hpp"
#include "core/back_event_t.hpp"
#include "utils/log.hpp"
#include "acl/acl_api.hpp"

#include <string>


class mod_api : public Callback, public AclApi
{
public:
    enum : bool {
        CLIENT_UNLOGGED,
        CLIENT_LOGGED
    };
    bool logged_on = CLIENT_UNLOGGED; // TODO suspicious

    BackEvent_t mod_signal = BACK_EVENT_NONE;

    void set_mod_signal(BackEvent_t signal) { this->mod_signal = signal; }

    BackEvent_t get_mod_signal()
    {
        return this->mod_signal;
    }

    [[nodiscard]] virtual bool is_up_and_running() const { return false; }

    virtual void init() {}

    // support auto-reconnection
    virtual bool is_auto_reconnectable() const { return false; }

    virtual bool server_error_encountered() const = 0;

    virtual void disconnect() {}

    virtual void display_osd_message(std::string const & /*unused*/) {}

    virtual void move_size_widget(int16_t/* left*/, int16_t/* top*/, uint16_t/* width*/, uint16_t/* height*/) {}

    virtual void send_input(int/* time*/, int/* message_type*/, int/* device_flags*/, int/* param1*/, int/* param2*/) {}

    [[nodiscard]] virtual Dimension get_dim() const { return Dimension(); }

    virtual void log_metrics() {}

    virtual void DLP_antivirus_check_channels_files() {}

    virtual void acl_update() override {}
};
