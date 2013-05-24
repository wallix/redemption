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

#ifndef _REDEMPTION_MOD_INTERNAL_INTERNAL_MOD_HPP_
#define _REDEMPTION_MOD_INTERNAL_INTERNAL_MOD_HPP_

#include "widget/screen.hpp"
#include "mod_api.hpp"

struct internal_mod : public mod_api {
    public:
    FrontAPI & front;
    BackEvent_t signal;

    widget_screen screen;
    int dragging;
    Rect dragging_rect;
    int draggingdx; // distance between mouse and top angle of dragged window
    int draggingdy; // distance between mouse and left angle of dragged window
    struct Widget* dragging_window;
    RDPBrush brush;

    internal_mod(FrontAPI & front, uint16_t front_width, uint16_t front_height)
            : mod_api(front_width, front_height)
            , front(front)
            , signal(BACK_EVENT_NONE)
            , screen(this, front_width, front_height)
    {
        this->front.server_resize(front_width, front_height, 24);
        /* dragging info */
        this->dragging = 0;
        this->event = event;
        // dragging_rect is (0,0,0,0)
        this->draggingdx = 0; // distance between mouse and top angle of dragged window
        this->draggingdy = 0; // distance between mouse and left angle of dragged window
        this->dragging_window = 0;
    }


    virtual void send_to_front_channel(const char * const mod_channel_name, uint8_t* data, size_t length, size_t chunk_size, int flags)
    {
        const CHANNELS::ChannelDef * front_channel = this->front.get_channel_list().get(mod_channel_name);
        if (front_channel){
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
    }

    virtual void mod_event(int event_id)
    {
        this->signal = static_cast<BackEvent_t>(event_id);
        this->event.set();
    }

    virtual void begin_update()
    {
        this->front.begin_update();
    }

    virtual void end_update()
    {
        this->front.end_update();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->front.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->front.draw(cmd, clip);
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        this->front.server_draw_text(x, y, text, fgcolor, bgcolor, clip);
    }

    virtual void text_metrics(const char * text, int & width, int & height)
    {
        this->front.text_metrics(text, width, height);
    }


    virtual ~internal_mod()
    {
    }

    size_t nb_windows()
    {
        return this->screen.child_list.size();
    }


    const Rect & get_screen_rect() const
    {
        return this->screen.rect;
    }

    Widget * window(int i)
    {
        return this->screen.child_list[i];
    }


    uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }

    virtual void rdp_input_invalidate(const Rect & rect) = 0;
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) = 0;
    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap) = 0;

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual BackEvent_t draw_event()
    {
        return BACK_EVENT_NONE;
    }
};

#endif
