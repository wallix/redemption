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

#include <string>
#include <ctime>


class mod_api : public Callback
{
public:
    enum : bool {
        CLIENT_UNLOGGED,
        CLIENT_LOGGED
    };
    bool logged_on = CLIENT_UNLOGGED; // TODO suspicious

    [[nodiscard]] virtual bool is_up_and_running() const { return false; }

    // support auto-reconnection
    virtual bool is_auto_reconnectable() { return false; }

    virtual void disconnect() {}

    virtual void display_osd_message(std::string const & /*unused*/) {}

    virtual void move_size_widget(int16_t/* left*/, int16_t/* top*/, uint16_t/* width*/, uint16_t/* height*/) {}

    virtual bool disable_input_event_and_graphics_update(
            bool disable_input_event, bool disable_graphics_update) {
        (void)disable_input_event;
        (void)disable_graphics_update;
        return false;
    }

    virtual void send_input(int/* time*/, int/* message_type*/, int/* device_flags*/, int/* param1*/, int/* param2*/) {}

    [[nodiscard]] virtual Dimension get_dim() const { return Dimension(); }

    virtual void log_metrics() {}

    virtual void DLP_antivirus_check_channels_files() {}
};
