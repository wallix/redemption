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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   widget, drawable
   this is an object that can be drawn on trough an internal_mod
   all windows, bitmaps, even the screen are of this type

*/

#include "widget.hpp"
#include "internal/internal_mod.hpp"

void Widget::draw(const Rect & clip)
{
    const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
    const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->front.draw(RDPOpaqueRect(scr_r, this->bg_color),
            region_clip);
    }
}
