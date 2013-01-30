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
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   module object. Some kind of interface between core and modules

*/
#ifndef _REDEMPTION_CORE_CLIENT_MOD_HPP_
#define _REDEMPTION_CORE_CLIENT_MOD_HPP_

#include <string.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "client_info.hpp"
#include "font.hpp"
#include "front.hpp"
#include "mainloop.hpp"
#include "wait_obj.hpp"
#include "keymap2.hpp"
#include "callback.hpp"
#include "modcontext.hpp"

enum BackEvent_t {
    BACK_EVENT_NONE = 0,
    BACK_EVENT_1,
    BACK_EVENT_2,
    BACK_EVENT_3,
    BACK_EVENT_STOP = 4,
    BACK_EVENT_5,
    BACK_EVENT_REFRESH,
};

struct client_mod : public Callback {
    wait_obj event;
    FrontAPI & front;
    RDPPen pen;
    bool pointer_displayed;

    uint16_t front_width;
    uint16_t front_height;

    client_mod(FrontAPI & front, const uint16_t front_width, const uint16_t front_height)
        : event(-1)
        , front(front)
        , front_width(front_width)
        , front_height(front_height)
    {
        this->pointer_displayed = false;
        this->event.set(0);
    }

    virtual ~client_mod()
    {
    }


    // draw_event is run when mod socket received some data (drawing order),
    // these order could also be auto-generated, say to comply to some refresh.

    // draw_event returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual BackEvent_t draw_event(void) = 0;

    virtual void refresh_context(ModContext & context)
    {
        return; // used when context changed to avoid creating a new module
    }

    int server_is_term()
    {
        return g_is_term();
    }

    void server_add_char(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
        struct FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data, data, fi.datasize());
        this->glyph_cache(fi, font, character);
    }

    void send_to_front_channel(const char * const mod_channel_name, uint8_t* data, size_t length, size_t chunk_size, int flags)
    {
        const ChannelDef * front_channel = this->front.get_channel_list().get(mod_channel_name);
        if (front_channel){
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
    }

    bool get_pointer_displayed() {
        return this->pointer_displayed;
    }

    void set_pointer_display() {
        this->pointer_displayed = true;
    }

    void glyph_cache(const FontChar & font_char, int font_index, int char_index)
    {
        RDPGlyphCache cmd(font_index, 1, char_index, font_char.offset, font_char.baseline, font_char.width, font_char.height, font_char.data);
        this->front.draw(cmd);
    }

    void server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
    }

    virtual void rdp_input_invalidate(const Rect & r) = 0;
    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) = 0;
    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) = 0;
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) = 0;

};

#endif
