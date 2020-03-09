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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni

   This class implement abstract callback used by front layer
   it is used to avoid explicit dependency between front layer
   and session (to enable to use front layer in tests).

*/

#pragma once

#include "core/callback.hpp"

template<class ForwardTo> class CallbackForwarder : public Callback
{
    ForwardTo & wrap;
    public:
    CallbackForwarder(ForwardTo & wrap) : wrap(wrap){}
    // Callback
    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override
    {
        this->wrap.send_to_mod_channel(front_channel_name, chunk, length, flags);
    }
    // Interface for session to send back to mod_rdp for tse virtual channel target data (asked previously)
    void send_auth_channel_data(const char * data) override
    {
        this->wrap.send_auth_channel_data(data);
    }
    void send_checkout_channel_data(const char * data) override
    {
        this->wrap.send_checkout_channel_data(data);
    }
    void create_shadow_session(const char * userdata, const char * type) override
    {
        this->wrap.create_shadow_session(userdata, type);
    }
    // RdpInput
    std::string module_name() override 
    {
        return "WrappedMod" + this->wrap.module_name();
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        this->wrap.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override
    {
        this->wrap.rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        this->wrap.rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        this->wrap.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    void rdp_input_invalidate(Rect r) override
    {
        this->wrap.rdp_input_invalidate(r);
    }

// Client Notify module that gdi is up and running
    void rdp_gdi_up_and_running(ScreenInfo & screen_info) override
    {
        this->wrap.rdp_gdi_up_and_running(screen_info);
    }

// Client Notify module that gdi is not up and running any more
    void rdp_gdi_down() override
    {
        this->wrap.rdp_gdi_down();
    }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) override
    {
        this->wrap.rdp_allow_display_updates(left, top, right, bottom);
    }
    
    void rdp_suppress_display_updates() override
    {
        this->wrap.rdp_suppress_display_updates();
    }

    void refresh(Rect clip) override
    {
        this->wrap.refresh(clip);
    }
};

