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
   Author(s): Christophe Grosjean, Javier Caverni

*/

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_BUTTON_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_BUTTON_HPP_

#include "widget.hpp"

struct widget_button : public Widget
{
    widget_button(mod_api * mod, const Rect & r, Widget * parent, int id, int tab_stop, const char * caption)
    : Widget(mod, r, parent, WND_TYPE_BUTTON) {

        this->tab_stop = tab_stop;
        this->id = id;
        this->caption1 = strdup(caption);
    }

    virtual ~widget_button() {
        if (this->caption1){
            free(this->caption1);
            this->caption1 = 0;
        }
    }

    virtual void draw(const Rect & clip)
    {
        Rect r(0, 0, this->rect.cx, this->rect.cy);

        const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
        const Region region = this->get_visible_region(this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->draw_button(scr_r,
                this->caption1,
                this->state,
                this->has_focus,
                region_clip);
        }
    }
    
    void draw_button(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip){

        int bevel = (state == BUTTON_STATE_DOWN)?1:0;

        this->mod->draw(RDPOpaqueRect(r, GREY), clip);
        if (state == BUTTON_STATE_DOWN) {
            this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        } else {
            this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), WHITE), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), WHITE), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->mod->draw(RDPOpaqueRect(Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->mod->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        }

        TODO("There is probably a way to move text_metrics into server_draw_text, or something similar")
        int w = 0;
        int h = 0;
        this->mod->text_metrics(caption, w, h);
        this->mod->server_draw_text(
            r.x + r.cx / 2 - w / 2 + bevel,
            r.y + r.cy / 2 - h / 2 + bevel,
            caption, GREY, BLACK, clip);
        // focus rect
        if (has_focus) {
            this->mod->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->mod->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->mod->draw(
                RDPPatBlt(Rect(r.x + r.cx - 6 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->mod->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + r.cy - 6 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
        }
    }

};

#endif
