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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_WIDGET_SCREEN_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_WIDGET_SCREEN_HPP_

#include "widget_widget.hpp"
#include "mod_api.hpp"

struct widget_screen : public Widget {
    widget_screen(mod_api * mod, int width, int height)
    : Widget(mod, Rect(0, 0, width, height), NULL, WND_TYPE_SCREEN) 
    {
    }

    ~widget_screen() {
    }

    virtual void draw(const Rect & clip)
    {
        Rect r(0, 0, this->rect.cx, this->rect.cy);
        const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
        const Region region = this->get_visible_region(this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));
            if (!region_clip.isempty()){
                this->mod->draw(RDPOpaqueRect(scr_r, this->bg_color), region_clip);
            }
        }

        for (size_t i = 0; i < this->child_list.size(); i++) {
            Widget *b = this->child_list[i];
            Rect r2 = rect.intersect(b->rect.wh());
            if (!r2.isempty()) {
                b->refresh(r2);
            }
        }
    }  
};

#endif
