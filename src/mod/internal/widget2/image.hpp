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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */
#pragma once

#include "widget.hpp"
#include "utils/bitmap_from_file.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "gdi/graphic_api.hpp"

class WidgetImage : public Widget2
{
    Bitmap bmp;

public:
    WidgetImage(gdi::GraphicApi & drawable, const char * filename, Widget2 & parent, NotifyApi* notifier, int group_id = 0)
    : Widget2(drawable, parent, notifier, group_id)
    , bmp(bitmap_from_file(filename))
    {
        this->tab_flag   = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;
    }

    ~WidgetImage() override {}

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->drawable.draw(
                RDPMemBlt(
                    0,
                    rect_intersect,
                    0xCC,
                    rect_intersect.x - this->x(),
                    rect_intersect.y - this->y(),
                    0
                ),
                rect_intersect,
                this->bmp
            );

            this->drawable.end_update();
        }
    }

    Dimension get_optimal_dim() override {
        return Dimension(this->bmp.cx(), this->bmp.cy());
    }
};
