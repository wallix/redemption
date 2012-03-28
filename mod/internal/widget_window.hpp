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

#if !defined(__MOD_INTERNAL_WIDGET_WINDOW__)
#define __MOD_INTERNAL_WIDGET_WINDOW__

#include "widget.hpp"
#include "internal/internal_mod.hpp"

struct window : public Widget
{
    window(GraphicalContext * mod, const Rect & r, Widget * parent, int bg_color, const char * title)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_WND) {

        assert(type == WND_TYPE_WND);

        this->bg_color = bg_color;
        this->rect.x = r.x;
        this->rect.y = r.y;
        this->caption1 = strdup(title);
    }

    /* find the window containing widget */
    virtual window * find_window()
    {
        return this;
    }

    ~window() {
        if (this->caption1){
            free(this->caption1);
            this->caption1 = 0;
        }
    }

    void focus(const Rect & clip)
    {
        this->has_focus = true;
    }

    void blur(const Rect & clip)
    {
        this->has_focus = false;
    }



    void draw(const Rect & clip)
    {
        Rect r(0, 0, this->rect.cx, this->rect.cy);
        const Rect scr_r = this->to_screen_rect(r);
        const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->mod->draw_window(scr_r,
                this->bg_color, this->caption1,
                this->has_focus,
                region_clip);
        }
    }

    void def_proc(const int msg, const int param1, const int param2, Keymap2 * keymap)
    {
        if (msg == WM_KEYDOWN) {

            Widget * control_with_focus = this->default_button;
            // find control that has focus
            size_t size = this->child_list.size();
            size_t i_focus;
            TODO(" we should iterate only on controls that have tabstop setted (or another attribute can_get_focus ?). Or we could also keep index of focused_control in child_list (but do not forget to reset it when we redefine controls).")
            for (i_focus = 0; i_focus < size; i_focus++){
                if (this->child_list[i_focus]->has_focus && this->child_list[i_focus]->tab_stop){
                    control_with_focus = this->child_list[i_focus];
                    break;
                }
            }

            int scan_code = param1 & 0x7F;
            switch (scan_code){
            case 15:
            { /* tab */
                /* move to next tab stop */
                int shift = keymap->is_shift_pressed();
                // find the next tab_stop
                if (shift) {
                    for (size_t i = (size+i_focus-1) % size ; i != i_focus ; i = (i+size-1) % size) {
                        Widget * new_focus = this->child_list[i];
                        if (switch_focus(control_with_focus, new_focus)) {
                            break;
                        }
                    }
                } else {
                    for (size_t i = (size+i_focus+1) % size ; i != i_focus ; i = (i+size+1) % size) {
                        Widget * new_focus = this->child_list[i];
                        if (switch_focus(control_with_focus, new_focus)) {
                            break;
                        }
                    }
                }
            }
            break;
            case 28: /* enter */
                this->notify(this->default_button, 1, 0, 0);
            return;
            case 1: /* esc */
                if (this->esc_button) {
                    this->notify(this->esc_button, 1, 0, 0);
                }
            break;
            default:
                if (control_with_focus){
                    control_with_focus->def_proc(msg, param1, param2, keymap);
                }
            }
        }
    }


};

#endif
