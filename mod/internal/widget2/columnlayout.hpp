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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET2_COLUMNLAYOUT_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET2_COLUMNLAYOUT_HPP_
#include "widget.hpp"
#include "layout.hpp"

struct WidgetColumnLayout : public WidgetLayout {
    WidgetColumnLayout(DrawApi & drawable, int x, int y, Widget2 & parent,
                       NotifyApi * notifier, int group_id = 0)
        : WidgetLayout(drawable, Rect(x, y, 1, 1), parent, notifier, group_id)
    {
    }
    virtual ~WidgetColumnLayout() {}

    virtual void rearrange(size_t origin = 0) {
        size_t index = origin;
        int pos_y = this->rect.y;
        if (index > 0) {
            pos_y = this->items[index - 1]->ly();
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

// struct WidgetColumnLayout : public Widget2 {
//     Widget2 * items[COLUMN_SIZE_MAX];
//     size_t    nb_items;

//     WidgetColumnLayout(DrawApi & drawable, int x, int y, Widget2 & parent,
//                        NotifyApi * notifier, int group_id = 0)
//         : Widget2(drawable, Rect(x, y, 1, 1), parent, notifier, group_id)
//         , nb_items(0)
//     {
//     }

//     virtual ~WidgetColumnLayout() {}

//     virtual void draw(const Rect& clip) {
//         for (size_t i = 0; i < this->nb_items; ++i) {
//             Widget2 *w = this->items[i];
//             w->refresh(clip.intersect(w->rect));
//         }
//     }
//     virtual void set_xy(int16_t x, int16_t y) {
//         for (size_t i = 0, max = this->nb_items; i < max; ++i) {
//             Widget2 * w = this->items[i];
//             uint16_t dx = w->rect.x - this->rect.x;
//             uint16_t dy = w->rect.y - this->rect.y;
//             this->items[i]->set_xy(x + dx, y + dy);
//         }
//         this->rect.x = x;
//         this->rect.y = y;
//     }
//     virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
//     {
//         Widget2 * ret = 0;
//         for( size_t i = 0; i < this->nb_items && ret == 0; ++i) {
//             if (this->items[i]->rect.contains_pt(x, y)) {
//                 ret = this->items[i];
//             }
//         }
//         return ret;
//     }

//     size_t get_size() {
//         return this->nb_items;
//     }
//     void add_widget(Widget2 * w) {
//         if (this->nb_items < COLUMN_SIZE_MAX) {
//             int pos_y = this->rect.y;
//             if (this->nb_items > 0) {
//                 pos_y = this->items[this->nb_items - 1]->ly();
//             }
//             w->set_xy(this->rect.x, pos_y);
//             this->items[nb_items] = w;
//             this->nb_items++;
//             this->rect.cy += w->cy();
//             if (w->cx() > this->rect.cx) {
//                 this->rect.cx = w->cx();
//             }
//         }
//     }

//     void rearrange(size_t origin = 0) {
//         size_t index = origin;
//         int pos_y = this->rect.y;
//         if (index > 0) {
//             pos_y = this->items[index - 1]->ly();
//         }
//         for (; index < this->nb_items; index++) {
//             this->items[index]->set_xy(this->rect.x, pos_y);
//             pos_y += this->items[index]->cy();
//         }
//         this->rect.cy = pos_y - this->rect.y;
//     }


// };

#endif
