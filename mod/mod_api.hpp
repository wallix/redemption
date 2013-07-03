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

*/

#ifndef _REDEMPTION_MOD_MOD_API_HPP_
#define _REDEMPTION_MOD_MOD_API_HPP_

#include "RDP/RDPGraphicDevice.hpp"
#include "front_api.hpp"
#include "wait_obj.hpp"
#include "callback.hpp"
#include "config.hpp"
#include "font.hpp"
#include "draw_api.hpp"

enum {
    BUTTON_STATE_UP   = 0,
    BUTTON_STATE_DOWN = 1,
};


struct mod_api : public Callback, public DrawApi {
    wait_obj event;
    RDPPen   pen;
    bool     pointer_displayed;

    uint16_t front_width;
    uint16_t front_height;

    mod_api(const uint16_t front_width, const uint16_t front_height)
        : event(-1)
        , front_width(front_width)
        , front_height(front_height)
    {
        this->pointer_displayed = false;
        this->event.set(0);
    }

    virtual ~mod_api()
    {
    }

    TODO("remove event from mod api")
    virtual void mod_event(int event_id) {}

    virtual void send_to_front_channel(const char * const mod_channel_name, uint8_t* data, size_t length, size_t chunk_size, int flags) = 0;

    // draw_event is run when mod socket received some data (drawing order),
    // these order could also be auto-generated, say to comply to some refresh.

    // draw_event returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual BackEvent_t draw_event(void) = 0;

    virtual void refresh_context(Inifile & ini)
    {
        return; // used when context changed to avoid creating a new module
    }

    bool get_pointer_displayed() {
        return this->pointer_displayed;
    }

    void set_pointer_display() {
        this->pointer_displayed = true;
    }
};

#endif
