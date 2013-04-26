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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_WIDGET2_YES_NO_HPP)
#define REDEMPTION_MOD_WIDGET2_YES_NO_HPP

#include "widget.hpp"
#include "button.hpp"
#include <rect.hpp>

class WidgetYesNo : public Widget
{
public:
    enum {
        NOTIFY_FOCUS_BEGIN = ::NOTIFY_FOCUS_BEGIN,
        NOTIFY_FOCUS_END = ::NOTIFY_FOCUS_END,
        NOTIFY_YES = ::NOTIFY_SUBMIT,
        NOTIFY_NO = ::NOTIFY_CANCEL
    };

    WidgetButton yes;
    WidgetButton no;

    WidgetYesNo(ModApi* drawable, int x, int y,
                Widget* parent, NotifyApi* notifier,
                const char * yes = "submit",
                const char * no = "cancel", int group_id = 0)
    : Widget(drawable, Rect(), parent, Widget::TYPE_BUTTON, notifier, group_id)
    , yes(drawable, Rect(), this, 0, yes, 0)
    , no(drawable, Rect(), this, 0, no, 1)
    {
        int cx = std::max(this->no.label.context_text.cx, this->yes.label.context_text.cx) + 8;
        int cy = std::max(this->no.label.context_text.cy, this->yes.label.context_text.cy) + 6;
        this->yes.rect.cx = cx;
        this->yes.rect.cy = cy;
        this->yes.rect.y = 0;
        this->yes.rect.x = 0;
        this->yes.label.x_text = (cx - this->yes.label.context_text.cx) / 2;
        this->yes.label.y_text = (cy - this->yes.label.context_text.cy) / 2;
        this->no.rect.cx = cx;
        this->no.rect.cy = cy;
        this->no.rect.y = 0;
        this->no.rect.x = this->yes.rect.cx + 5;
        this->no.label.x_text = (cx - this->no.label.context_text.cx) / 2;
        this->no.label.y_text = (cy - this->no.label.context_text.cy) / 2;
        this->rect.x = x;
        this->rect.y = y;
        this->rect.cx = cx * 2 + 5;
        this->rect.cy = cy;
    }

    virtual ~WidgetYesNo()
    {}

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        int dx = x < 0 ? -x : 0;
        int dy = y < 0 ? -y : 0;

        Rect clipsep = rect.intersect(Rect(this->yes.rect.cx, 0, 5, this->yes.rect.cy));
        if (!clipsep.isempty()) {
            this->Widget::draw(clipsep, x+dx, y+dy, xclip + this->yes.rect.cx, yclip);
        }

        if (rect.x < this->yes.rect.cx) {
            this->yes.draw(Rect(rect.x-dx, rect.y-dy, std::min<>(this->yes.rect.cx, rect.cx), rect.cy), x+dx, y+dy, xclip, yclip);
        }

        clipsep = rect.intersect(this->no.rect.offset(-dx, -dy));
        if (!clipsep.isempty()) {
            clipsep.x -= this->no.rect.x;
            this->no.draw(clipsep, x + this->no.rect.x+dx, y+dy, xclip + this->no.rect.x, yclip);
        }
    }

    virtual void notify(int group_id, EventType event)
    {
        if (event == WIDGET_SUBMIT && (id == this->yes.id || id == this->no.group_id)) {
            this->notify_self(id == this->yes.id ? NOTIFY_YES : NOTIFY_NO);
        } else {
            this->Widget::notify(group_id, event);
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == CLIC_BUTTON1_DOWN || event == CLIC_BUTTON1_UP) {
            if (this->yes.rect.contains_pt(param, param2)) {
                this->yes.send_event(event, param, param2, keymap);
            } else if (this->no.rect.contains_pt(param, param2)) {
                this->no.send_event(event, param + this->no.rect.x + 5, param2, keymap);
            } else {
                this->Widget::send_event(event, param, param2, keymap);
            }
        } else {
            this->Widget::send_event(event, param, param2, keymap);
        }
    }
};

#endif
