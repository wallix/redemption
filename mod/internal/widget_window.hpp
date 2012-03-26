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

    virtual void draw(const Rect & clip);
    virtual void def_proc(const int msg, const int param1, const int param2, const Keymap * keymap);
    virtual void focus(const Rect & clip);
    virtual void blur(const Rect & clip);


};

#endif
