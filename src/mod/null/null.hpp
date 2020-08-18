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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

*/

#pragma once

#include "mod/mod_api.hpp"

// Null module receive every event and does nothing. It allow session code to always have a receiving module active, thus avoidind to test that so back_end is available.

struct null_mod : public mod_api
{
    null_mod()
    {
        this->set_mod_signal(BACK_EVENT_NEXT);
        // throw Error(ERR_BACK_EVENT_NEXT);
    }
    std::string module_name() override {return "Null Mod";}
    void rdp_input_mouse(int,int,int,Keymap2 *) override {}
    void rdp_input_scancode(long,long,long,long,Keymap2 *) override {}
    void rdp_input_synchronize(uint32_t, uint16_t, int16_t, int16_t) override {}
    void rdp_input_invalidate(const Rect) override {}
    void refresh(const Rect) override {}
    bool is_up_and_running() const override { return true; }

    bool is_auto_reconnectable() const override { return false; }
    bool server_error_encountered() const override { return false; }

    void rdp_gdi_up_and_running() override {}
    void rdp_gdi_down() override {}
    void send_to_mod_channel(CHANNELS::ChannelNameId /*front_channel_name*/, InStream & /*chunk*/, std::size_t /*length*/, uint32_t /*flags*/) override {}
    void create_shadow_session(const char * /*userdata*/, const char * /*type*/) override {}
    void send_auth_channel_data(const char * /*data*/) override {}
    void send_checkout_channel_data(const char * /*data*/) override {}

};

