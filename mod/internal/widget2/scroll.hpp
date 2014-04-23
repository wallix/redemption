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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SCROLLBAR_HPP_)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SCROLLBAR_HPP_
#include "widget.hpp"

class WidgetFrame : public Widget2 {
public:
    Widget2 * wid;

    // widget_y = frame_y + LAMBDA * (frame_height - widget_height)

    // LAMBDA = (widget_y - frame_y) / (frame_height - widget_height)

    WidgetFrame(DrawApi & drawable, const Rect& rect, Widget2 & parent,
                NotifyApi * notifier, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , wid(NULL)
    {
    }

    virtual ~WidgetFrame() {}

    void set_widget(Widget2 * w) {
        if (w) {
            this->wid = w;
            this->wid->set_xy(this->rect.x, this->rect.y);
        }
    }

    bool need_vertical_scroll() {
        return ((this->wid) && (this->wid->rect.cy > this->rect.cy));
    }

    void set_vertical_widget_pos(uint16_t percent) {
        if (this->need_vertical_scroll()) {
            if (percent > 100) {
                percent = 100;
            }
            // widget_y = frame_y + LAMBDA * (frame_height - widget_height)
            int16_t widget_y = this->rect.y - percent * (this->wid->rect.cy - this->rect.cy) / 100;
            LOG(LOG_INFO, "move vertical position from %u to %d", this->wid->rect.y, widget_y);
            this->wid->set_xy(this->wid->rect.x, widget_y);
        }
    }


    bool need_horizontal_scroll() {
        return ((this->wid) && (this->wid->rect.cx > this->rect.cx));
    }

    void set_horizontal_widget_pos(uint16_t percent) {
        if (this->need_horizontal_scroll()) {
            if (percent > 100) {
                percent = 100;
            }
            // widget_x = frame_x - LAMBDA * (widget_width - frame_width)
            int16_t widget_x = this->rect.x - percent * (this->wid->rect.cx - this->rect.cx) / 100;
            LOG(LOG_INFO, "move horizontal position from %d to %d", this->wid->rect.x, widget_x);
            this->wid->set_xy(widget_x, this->wid->rect.y);
        }
    }

    virtual void draw(const Rect& clip) {
        if (this->wid) {
            this->wid->draw(clip.intersect(this->rect));
        }
    }
};


class WidgetVScrollBar : public Widget2 {
public:
    WidgetFrame * frame;
    uint16_t arrow_height;

    uint16_t bar_height; // = frame_height - 2 * arrow_height
    Rect scroll;
    uint16_t scroll_height; // = (frame_height * scroll_height) / widget_height
    int16_t scroll_y;
    /*
      widget_y = frame_y + LAMBDA * (frame_height - widget_height);
      sblock_y = frame_y + arrow_height + LAMBDA * (scroll_height - sblock_height);
    */
    uint16_t widget_height;

    WidgetVScrollBar(DrawApi & drawable, const Rect& rect, Widget2 & parent,
                    NotifyApi * notifier, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , frame(NULL)
        , arrow_height(0)
        , bar_height(this->rect.cy)
        , scroll(Rect(rect.x, rect.y, 1, 1))
        , scroll_height(0)
        , scroll_y(this->rect.y)
        , widget_height(0)
    {
    }

    void set_frame(WidgetFrame * wframe) {
        if (wframe) {
            this->frame = wframe;
            this->set_scroll_height();
        }
    }

    void set_scroll_height() {
        if (this->frame->need_vertical_scroll()) {
            this->widget_height = this->frame->wid->rect.cy;
            this->scroll_height = this->bar_height * this->rect.cy / this->widget_height;
        }
    }

    void set_vertical_scroll(uint percent) {
        this->scroll_y = this->rect.y + percent * (this->bar_height - this->scroll_height) / 100;
        if (this->frame) {
            this->frame->set_vertical_widget_pos(percent);
        }
    }
    virtual void draw(const Rect& clip) {
    }
};

#endif
