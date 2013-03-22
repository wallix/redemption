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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_LABEL_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_LABEL_HPP_

#include "widget.hpp"
#include "mod_api.hpp"

struct widget_label : public Widget {

    widget_label(mod_api * mod, const Rect & r, Widget * parent, const char * title)
    : Widget(mod, r, parent, WND_TYPE_LABEL) {

        this->rect.x = r.x;
        this->rect.y = r.y;
        this->caption1 = strdup(title);
    }

    virtual ~widget_label() {
        if (this->caption1){
            free(this->caption1);
            this->caption1 = 0;
        }
    }

    void draw(const Rect & clip)
    {
        const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
        const Region region = this->get_visible_region(this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->mod->server_draw_text(scr_r.x, scr_r.y, this->caption1, GREY, BLACK, region_clip);
        }

    }

};

#endif
