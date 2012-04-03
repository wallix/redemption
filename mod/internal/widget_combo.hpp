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

#if !defined(__MOD_INTERNAL_WIDGET_COMBO__)
#define __MOD_INTERNAL_WIDGET_COMBO__

#include "widget.hpp"
#include "internal/internal_mod.hpp"


struct widget_combo : public Widget
{
    widget_combo(internal_mod * mod, const Rect & r,
                Widget * parent, int id, int tab_stop)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_COMBO){
        this->rect.x = r.x;
        this->rect.y = r.y;
        this->id = id;
        this->tab_stop = tab_stop;
        this->item_index = 0;
    }

    ~widget_combo() {}

    virtual void draw(const Rect & clip)
    {
        const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
        const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->mod->draw_combo(scr_r,
                this->string_list[this->item_index],
                this->state,
                this->has_focus,
                region_clip);
        }
    }


    virtual void def_proc(const int msg, const int param1, const int param2, Keymap2 * keymap)
    {
        if (msg == WM_KEYDOWN) {
            if ((keymap->top_kevent() == Keymap2::KEVENT_LEFT_ARROW)
            || (keymap->top_kevent() == Keymap2::KEVENT_UP_ARROW)){
                if (this->item_index > 0) {
                    this->item_index--;
                    this->refresh(this->rect.wh());
                    this->notify(this, CB_ITEMCHANGE, 0, 0);
                }
            }
            else if ((keymap->top_kevent() == Keymap2::KEVENT_RIGHT_ARROW)
            || (keymap->top_kevent() == Keymap2::KEVENT_DOWN_ARROW)){
                size_t count = this->string_list.size();
                if ((this->item_index + 1) < count) {
                    this->item_index++;
                    this->refresh(this->rect.wh());
                    this->notify(this, CB_ITEMCHANGE, 0, 0);
                }
            }
        }
    }

};

#endif
