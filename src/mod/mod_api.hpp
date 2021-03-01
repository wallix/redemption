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
#include "acl/acl_field_mask.hpp"

enum BackEvent_t
{
    BACK_EVENT_NONE = 0,
    BACK_EVENT_NEXT,     // MODULE FINISHED, ASKING FOR NEXT MODULE
    BACK_EVENT_STOP = 4, // MODULE FINISHED, ASKING TO LEAVE SESSION
};

class mod_api : public Callback
{
public:
    BackEvent_t mod_signal = BACK_EVENT_NONE;

    void set_mod_signal(BackEvent_t signal)
    {
        this->mod_signal = signal;
    }

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

    virtual void move_size_widget(int16_t/* left*/, int16_t/* top*/, uint16_t/* width*/, uint16_t/* height*/) {}

    virtual void send_input(int/* time*/, int/* message_type*/, int/* device_flags*/, int/* param1*/, int/* param2*/) {}

    [[nodiscard]] virtual Dimension get_dim() const { return Dimension(); }

    virtual void acl_update(AclFieldMask const& acl_fields) { (void)acl_fields; }

    virtual void reset_keyboard_status() {}
};
