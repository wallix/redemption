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

struct WidgetStackLayout : public WidgetLayout {
    size_t selected;
    BGRColor bgcolor;

    WidgetStackLayout(gdi::GraphicApi & drawable, const Rect rect, Widget2 & parent,
                      NotifyApi * notifier, int group_id = 0, BGRColor bgcolor = BLACK)
        : WidgetLayout(drawable, rect, parent, notifier, group_id)
        , selected(0)
        , bgcolor(bgcolor)
    {
    }

    virtual ~WidgetStackLayout() {}

    virtual void draw(const Rect clip) {
        if (this->selected < this->nb_items) {
            Widget2 *w = this->items[this->selected];
            w->refresh(clip.intersect(this->rect));
        }
    }

    virtual void rearrange(size_t origin = 0) {
        if (origin < this->nb_items) {
            Widget2 * w = this->items[origin];
            w->set_xy(this->rect.x, this->rect.y);
        }
    }
    void hide_current_layout() {
        if (this->selected < this->nb_items) {
            Widget2 *w = this->items[this->selected];
            this->drawable.draw(RDPOpaqueRect(w->rect,
                                              this->bgcolor
                                              ), this->rect
                                );
        }
    }
    void select(size_t num_layout) {
        if (num_layout < this->nb_items) {
            this->hide_current_layout();
            this->selected = num_layout;
            this->draw(this->rect);
        }
    }

    virtual void notify(Widget2 * from, notify_event_t event) {
        if (event == NOTIFY_SELECTION_CHANGED) {
            this->select(from->notify_value);
        }
    }
};


