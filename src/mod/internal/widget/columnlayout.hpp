/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 *
 */

#pragma once

#include "widget.hpp"
#include "layout.hpp"
#include "gdi/graphic_api.hpp"

struct WidgetColumnLayout : public WidgetLayout {
    WidgetColumnLayout(gdi::GraphicApi & drawable, int x, int y, Widget & parent,
                       NotifyApi * notifier, int group_id = 0)
        : WidgetLayout(drawable, Rect(x, y, 1, 1), parent, notifier, group_id)
    {
    }
    virtual ~WidgetColumnLayout() {}

    virtual void rearrange(size_t origin = 0) {
        size_t index = origin;
        int pos_y = this->rect.y;
        if (index > 0) {
            pos_y = this->items[index - 1]->bottom();
        }
        for (; index < this->nb_items; index++) {
            Widget2 * w = this->items[index];
            w->set_xy(this->rect.x, pos_y);
            pos_y += this->items[index]->cy();
            if (w->cx() > this->rect.cx) {
                this->rect.cx = w->cx();
            }
        }
        this->rect.cy = pos_y - this->rect.y;
    }

};


