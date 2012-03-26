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

#if !defined(__MOD_INTERNAL_WIDGET_LABEL__)
#define __MOD_INTERNAL_WIDGET_LABLE__

#include "widget.hpp"
#include "internal/internal_mod.hpp"

struct widget_label : public Widget {

    widget_label(GraphicalContext * mod, const Rect & r, Widget * parent, const char * title)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_LABEL) {

        assert(type == WND_TYPE_LABEL);

        this->rect.x = r.x;
        this->rect.y = r.y;
        this->caption1 = strdup(title);
    }

    ~widget_label() {
        free(this->caption1);
    }

    virtual void draw(const Rect & clip);

};

#endif
