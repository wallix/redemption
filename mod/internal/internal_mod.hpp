/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen
 */

#ifndef REDEMPTION_MOD_INTERNAL_INTERNAL_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_INTERNAL_MOD_HPP

#include "../mod/mod_api.hpp"
#include "widget2/screen.hpp"

struct InternalMod : public mod_api {
public:
    FrontAPI & front;

    WidgetScreen screen;
//     int dragging;
//     Rect dragging_rect;
//     int draggingdx; // distance between mouse and top angle of dragged window
//     int draggingdy; // distance between mouse and left angle of dragged window
//     Widget2 * dragging_window;
//     RDPBrush brush;

    InternalMod(FrontAPI & front, uint16_t front_width, uint16_t front_height)
    : mod_api(front_width, front_height)
    , front(front)
    , screen(*this, front_width, front_height)
    {
        this->front.server_resize(front_width, front_height, 24);
        /* dragging info */
//         this->dragging = 0;
        this->event = event;
        // dragging_rect is (0,0,0,0)
//         this->draggingdx = 0; // distance between mouse and top angle of dragged window
//         this->draggingdy = 0; // distance between mouse and left angle of dragged window
//         this->dragging_window = 0;
    }

    virtual ~InternalMod()
    {}

    const Rect & get_screen_rect() const
    {
        return this->screen.rect;
    }

    virtual void send_to_front_channel(const char * const mod_channel_name, uint8_t* data, size_t length, size_t chunk_size, int flags)
    {
        const CHANNELS::ChannelDef * front_channel =
            this->front.get_channel_list().get_by_name(mod_channel_name);
        if (front_channel){
            this->front.send_to_channel(*front_channel, data, length, chunk_size, flags);
        }
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

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        this->front.draw(cmd, clip, gly_cache);
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        TODO("bgcolor <-> fgcolor")
        this->front.server_draw_text(x, y, text, bgcolor, fgcolor, clip);
    }

    virtual void text_metrics(const char * text, int & width, int & height)
    {
        this->front.text_metrics(text, width, height);
    }


    uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }

    virtual void rdp_input_invalidate(const Rect& r)
    {
        this->screen.rdp_input_invalidate(r);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }
};

#endif
