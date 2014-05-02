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
            // LOG(LOG_INFO, "move vertical position from %d to %d", this->wid->rect.y, widget_y);
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
            // LOG(LOG_INFO, "move horizontal position from %d to %d", this->wid->rect.x, widget_x);
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

    uint16_t widget_height;
    int fg_color;
    int bg_color;
    int16_t y_click;

    WidgetVScrollBar(DrawApi & drawable, Widget2 & parent,
                     NotifyApi * notifier, int fg_color, int bg_color, int group_id = 0)
        : Widget2(drawable, Rect(0, 0, 12, 1), parent, notifier, group_id)
        , frame(NULL)
        , arrow_height(0)
        , bar_height(0)
        , scroll(Rect(this->rect.x + 1, this->rect.y + 1, 10, 1))
        , widget_height(0)
        , fg_color(fg_color)
        , bg_color(bg_color)
        , y_click(-1)
    {
    }

    virtual void set_xy(int16_t x, int16_t y) {
        this->scroll.x = x + 1;
        this->scroll.y = y + 1;
        Widget2::set_xy(x, y);
    }

    void set_frame(WidgetFrame * wframe) {
        if (wframe) {
            this->frame = wframe;
            this->set_xy(this->frame->lx(), this->frame->dy());
            this->rect.cy = this->frame->cy();
            this->bar_height = this->rect.cy - 2;
            this->set_scroll_height();
        }
    }

    void set_scroll_height() {
        if (this->frame->need_vertical_scroll()) {
            this->widget_height = this->frame->wid->rect.cy;
            this->scroll.cy = (this->bar_height * this->rect.cy) / this->widget_height;
            LOG(LOG_INFO, "Set scroll height %d, widget height : %d", this->scroll.cy, this->widget_height);
        }
        else {
            this->widget_height = 0;
        }
    }

    uint get_percent() {
        return (this->scroll.y - this->rect.y) * 100 / (this->bar_height - this->scroll.cy);
    }

    void set_vertical_scroll(uint percent) {
        this->scroll.y = this->rect.y + 1  + percent * (this->bar_height - this->scroll.cy) / 100;
        if (this->frame) {
            this->frame->set_vertical_widget_pos(percent);
        }
    }

    void send_position_to_frame() {
        if (this->frame) {
            this->frame->set_vertical_widget_pos(this->get_percent());
        }
    }

    virtual void draw(const Rect& clip) {
        // bar
        this->drawable.draw(RDPOpaqueRect(this->rect,
                                          this->bg_color),
                            clip);
        // LOG(LOG_INFO, "scroll x : %d, y : %d, width : %d, height : %d",
        //     this->scroll.x, this->scroll.y, this->scroll.cx, this->scroll.cy);
        // scroll
        if (this->widget_height) {
            this->drawable.draw(RDPOpaqueRect(this->scroll,
                                              this->fg_color),
                                clip);
        }
        // LOG(LOG_INFO, "get percent %d",this->get_percent());
    }

    bool move_scroll(int16_t my) {
        int16_t new_y_pos = this->scroll.y + my;
        bool res = false;
        if ((new_y_pos > this->dy()) && (new_y_pos + this->scroll.cy < this->ly())) {
            this->scroll.y = new_y_pos;
            res = true;
        }
        else if ((new_y_pos < this->dy()) && (this->scroll.y != this->dy() + 1)) {
            this->scroll.y = this->dy() + 1;
            res = true;
        }
        else if ((new_y_pos + this->scroll.cy > this->ly()) &&
                 (this->scroll.y + this->scroll.cy + 1 != this->ly())) {
            this->scroll.y = this->ly() - this->scroll.cy - 1;
            res = true;
        }
        return res;
    }

    // - mouve event (mouse moves or a button went up or down)
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            if (this->scroll.contains_pt(x, y)) {
                this->y_click = y;
            }
        }
        if (device_flags == (MOUSE_FLAG_MOVE)) {
            if (this->y_click != -1) {
                bool moved = this->move_scroll(y - this->y_click);
                if (moved) {
                    this->send_position_to_frame();
                    this->y_click = y;
                    TODO("This seems to be greedy");
                    this->frame->refresh(this->frame->rect);
                    this->refresh(this->rect);
                }
            }
        }
        if (device_flags == (MOUSE_FLAG_BUTTON1)) {
            this->y_click = -1;
        }
    }

};

#endif
