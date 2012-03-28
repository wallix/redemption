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

#if !defined(__MOD_INTERNAL_WIDGET_POPUP__)
#define __MOD_INTERNAL_WIDGET_POPUP__

#include "widget.hpp"
#include "internal/internal_mod.hpp"


struct widget_popup : public Widget
{

    widget_popup(GraphicalContext * mod, const Rect & r,
         Widget * popped_from,
         Widget * parent,
         int item_index)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_SPECIAL)
    {
            this->popped_from = popped_from;
            this->rect.x = r.x;
            this->rect.y = r.y;
            this->item_index = item_index;
    }
    ~widget_popup() {}

    virtual void draw(const Rect & clip)
    {
        const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
        const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);
        TODO("Font Height is currently hardcoded to 16 in drop box")
        const int height = 16;

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->mod->front.draw(
                RDPOpaqueRect(Rect(scr_r.x, scr_r.y, this->rect.cx, this->rect.cy), WHITE),
                region_clip);

            TODO(" this should be a two stages process  first prepare drop box data  then call draw_xxx that use that data to draw. For now everything is mixed up  (and that is not good)")
            /* draw the list items */
            if (this->popped_from != 0) {
                size_t list_count = this->popped_from->string_list.size();

                // draw the selected line
                this->mod->front.draw(
                        RDPOpaqueRect(Rect(scr_r.x, scr_r.y + height * this->item_index, this->rect.cx, height), WABGREEN), region_clip);

                for (unsigned i = 0; i < list_count; i++) {
                    const char * p = this->popped_from->string_list[i];
                    this->mod->front.server_draw_text(scr_r.x + 2, scr_r.y + i * height,
                        p,
                        (i == this->item_index)?WABGREEN:WHITE,
                        (i == this->item_index)?WHITE:BLACK,
                        region_clip);
                }
            }

        }
    }

    virtual void def_proc(const int msg, const int param1, const int param2, Keymap2 * keymap)
    {
        if (msg == WM_MOUSEMOVE) {
            if (this->popped_from != 0) {
                TODO("The size of a line in drop box is currently hardcoded (16). We should get this size from font information. But the way it was done previously is not sufficient. We should be able to have lines of different sizes in drop box and such")
                unsigned i = param2 / 16;
                if (i != this->item_index && i < this->popped_from->string_list.size())
                {
                    this->item_index = i;
                    this->refresh(this->rect.wh());
                }
            }
        } else if (msg == WM_LBUTTONUP) {

            if (this->popped_from != 0) {
                this->popped_from->item_index = this->item_index;
                this->popped_from->refresh(this->popped_from->rect.wh());
                this->popped_from->notify(this->popped_from, CB_ITEMCHANGE, 0, 0);
            }
        }
    }

};

#endif
