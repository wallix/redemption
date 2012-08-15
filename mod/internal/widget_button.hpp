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

#if !defined(__MOD_INTERNAL_WIDGET_BUTTON__)
#define __MOD_INTERNAL_WIDGET_BUTTON__

#include "widget.hpp"
#include "internal/internal_mod.hpp"

struct widget_button : public Widget
{
    widget_button(internal_mod * mod, const Rect & r, Widget * parent, int id, int tab_stop, const char * caption)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_BUTTON) {

        assert(type == WND_TYPE_BUTTON);

        this->rect.x = r.x;
        this->rect.y = r.y;
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
        const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->mod->draw_button(scr_r,
                this->caption1,
                this->state,
                this->has_focus,
                region_clip);
        }
    }

    void draw_focus_rect(Widget * wdg, const Rect & r, const Rect & clip);

};

#endif
