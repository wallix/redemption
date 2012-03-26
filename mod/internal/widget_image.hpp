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

#if !defined(__MOD_INTERNAL_WIDGET_IMAGE__)
#define __MOD_INTERNAL_WIDGET_IMAGE__

#include "widget.hpp"
#include "internal/internal_mod.hpp"

struct widget_image : public Widget {
    Bitmap bmp;

    widget_image(GraphicalContext * mod, int width, int height, int type, Widget * parent, int x, int y, const char* filename, uint8_t bpp)
    : Widget(mod, width, height, parent, type), bmp(24, filename) {

        assert(type == WND_TYPE_IMAGE);

        this->rect = Rect(x, y, bmp.cx, bmp.cy);
    }

    ~widget_image() {
    }

    virtual void draw(const Rect & clip);

};

#endif
