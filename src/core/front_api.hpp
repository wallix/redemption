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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat

   Front object API (server),
   used to communicate with RDP client or other drawing backends
*/

#ifndef _REDEMPTION_CORE_FRONT_API_HPP_
#define _REDEMPTION_CORE_FRONT_API_HPP_

#include "RDP/RDPGraphicDevice.hpp"

#include "core/wait_obj.hpp"

class Capability;
class InStream;
class OrderCaps;

namespace CHANNELS {
    class ChannelDefArray;
    class ChannelDef;
}

class FrontAPI : public RDPGraphicDevice{
    public:
    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const = 0;
    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data
                                , std::size_t length, std::size_t chunk_size, int flags) = 0;

    virtual void send_global_palette() = 0;

    virtual int server_resize(int width, int height, int bpp) = 0;
    //virtual void update_config(const timeval & now, const Inifile & ini) {}

    int mouse_x;
    int mouse_y;
    bool notimestamp;
    bool nomouse;

    protected:
    wait_obj event;

    FrontAPI(bool notimestamp, bool nomouse)
        : mouse_x(0)
        , mouse_y(0)
        , notimestamp(notimestamp)
        , nomouse(nomouse) {}

    public:
    virtual wait_obj& get_event() { return this->event; }

    TODO("RZ : Move these methods in OrderCaps class.")
    virtual void intersect_order_caps(int idx, uint8_t * proxy_order_caps) const {}
    virtual void intersect_order_caps_ex(OrderCaps & order_caps) const {}

    ////////////////////////////////
    // Used by transparent proxy.

    virtual void send_data_indication_ex(uint16_t channelId, uint8_t const * data, std::size_t size) {}
    virtual void send_fastpath_data(InStream & data) {}
    virtual bool retrieve_client_capability_set(Capability & caps) { return true; }

    virtual void set_keyboard_indicators(uint16_t LedFlags) {}

    ////////////////////////////////
    // Session Probe.

    virtual void session_probe_started() {}
    virtual void set_keylayout(int LCID) {}
    virtual void set_focus_on_password_textbox(bool set) {}
    virtual void set_consent_ui_visible(bool set) {}
    virtual void session_update(const char * message) {}

    virtual bool disable_input_event_and_graphics_update(bool disable) { return false; }
    
    /// \return  -1 is error
    virtual int get_keylayout() const { return -1; }

    virtual void begin_update() = 0;
    virtual void end_update() = 0;

    virtual void flush() {}
};

#endif
