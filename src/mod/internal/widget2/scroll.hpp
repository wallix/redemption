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

#pragma once

#include "widget.hpp"
#include "utils/region.hpp"
#include "gdi/graphic_api.hpp"

class WidgetFrame : public Widget2 {
public:
    Widget2 * wid;
    int16_t diff_y;
    int16_t diff_x;
    // widget_y = frame_y + LAMBDA * (frame_height - widget_height)

    // LAMBDA = (widget_y - frame_y) / (frame_height - widget_height)

    WidgetFrame(gdi::GraphicApi & drawable, const Rect& rect, Widget2 & parent,
                NotifyApi * notifier, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , wid(nullptr)
        , diff_y(0)
        , diff_x(0)
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
            this->diff_y = widget_y - this->wid->rect.y;
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
            this->diff_x = widget_x - this->wid->rect.x;
            this->wid->set_xy(widget_x, this->wid->rect.y);
        }
    }

    virtual void draw(const Rect& clip) {
        if (this->wid) {
            int src_x = this->rect.x;
            int src_y = this->rect.y;
            Rect dest = this->rect;
            Rect refresh = this->rect;
            Rect refreshx = this->rect;
            if (this->diff_y > 0) {
                // DOWN (new > old)
                // source : frame_x, frame_y
                // destination : rect(frame_x, frame_y + diff_y,
                //                    frame_cx, frame_cy - diff_y)
                // refresh : rect(frame_x, frame_y,
                //                frame_cx, diff_y)
                dest.y = this->rect.y + this->diff_y;
                dest.cy = this->rect.cy - this->diff_y;
                refresh.cy = this->diff_y;
            }
            else if (this->diff_y < 0) {
                // UP (new < old)
                // source : frame_x, frame_y - diff_y
                // destination : rect(frame_x, frame_y,
                //                    frame_cx, frame_cy + diff_y)
                // refresh : rect(frame_x, frame_y + frame_cy + diff_y,
                //                frame_cx, -diff_y)
                src_y = this->rect.y - this->diff_y;
                dest.cy = this->rect.cy + this->diff_y;
                refresh.y = this->rect.y + this->rect.cy + this->diff_y;
                refresh.cy = - this->diff_y;
            }

            if (this->diff_x > 0) {
                // RIGHT (new > old)
                // source : frame_x, frame_y
                // destination : rect(frame_x + diff_x, frame_y,
                //                    frame_cx - diff_x, frame_cy)
                // refresh : rect(frame_x, frame_y,
                //                diff_x, frame_cy)
                dest.x = this->rect.x + this->diff_x;
                dest.cx = this->rect.cx - this->diff_x;
                refreshx.cx = this->diff_x;
            }
            else if (this->diff_x < 0) {
                // LEFT (new < old)
                // source : frame_x - diff_x, frame_y
                // destination : rect(frame_x, frame_y,
                //                    frame_cx + diff_x, frame_cy)
                // refresh : rect(frame_x + frame_cx + diff_x, frame_y,
                //                -diff_x, frame_cy)
                src_x = this->rect.x - this->diff_x;
                dest.cx = this->rect.cx + this->diff_x;
                refreshx.x = this->rect.x + this->rect.cx + this->diff_x;
                refreshx.cx = - this->diff_x;
            }

            if (this->diff_y || this->diff_x) {
                // 0xCC is the raster operator code for simple source copy on dest
                this->drawable.draw(RDPScrBlt(dest, 0xCC, src_x, src_y),
                                    clip);
                if (this->diff_x && this->diff_y) {
                    Region region;
                    region.rects.push_back(refreshx);
                    region.subtract_rect(refresh);
                    // if (region.rects.size() > 0) {
                    //     this->wid->draw(clip.intersect(region.rects[0]));
                    // }
                    // there should be only one rect in region.rects
                    for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
                        this->wid->draw(clip.intersect(region.rects[i]));
                    }
                }
                else if (this->diff_x) {
                    refresh = refreshx;
                }
                this->wid->draw(clip.intersect(refresh));
                this->diff_y = 0;
                this->diff_x = 0;
            }
            else {
                this->wid->draw(clip.intersect(this->rect));
            }
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
    int focus_color;
    int16_t y_click;
    int step;

    WidgetVScrollBar(gdi::GraphicApi & drawable, Widget2 & parent, NotifyApi * notifier,
                     int fg_color, int bg_color, int focus_color, int group_id = 0)
        : Widget2(drawable, Rect(0, 0, 14, 1), parent, notifier, group_id)
        , frame(nullptr)
        , arrow_height(0)
        , bar_height(0)
        , scroll(Rect(this->rect.x + 1, this->rect.y + 1, 12, 1))
        , widget_height(0)
        , fg_color(fg_color)
        , bg_color(bg_color)
        , focus_color(focus_color)
        , y_click(-1)
        , step(23)
    {
    }

    virtual ~WidgetVScrollBar() {}

    void set_x(int16_t x) override {
        this->scroll.x = x + 1;
        Widget2::set_x(x);
    }

    void set_y(int16_t y) override {
        this->scroll.y = y + 1;
        Widget2::set_y(y);
    }

    void set_frame(WidgetFrame * wframe) {
        if (wframe) {
            this->frame = wframe;
            this->set_xy(this->frame->right(), this->frame->y());
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
            int color = this->fg_color;
            if (this->has_focus) {
                color = this->focus_color;
            }
            this->drawable.draw(RDPOpaqueRect(this->scroll,
                                              color),
                                clip);
        }
        // LOG(LOG_INFO, "get percent %d",this->get_percent());
    }

    int16_t move_scroll(int16_t my) {
        int16_t new_y_pos = this->scroll.y + my;
        int16_t res = 0;
        if ((new_y_pos > this->y()) && (new_y_pos + this->scroll.cy < this->bottom())) {
            this->scroll.y = new_y_pos;
            res = my;
        }
        else if ((new_y_pos <= this->y()) && (this->scroll.y != this->y() + 1)) {
            res = this->y() + 1 - this->scroll.y;
            this->scroll.y = this->y() + 1;
        }
        else if ((new_y_pos + this->scroll.cy >= this->bottom()) &&
                 (this->scroll.y + this->scroll.cy + 1 != this->bottom())) {
            res = this->bottom() - this->scroll.cy - 1 - this->scroll.y;
            this->scroll.y = this->bottom() - this->scroll.cy - 1;
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
            else if (this->rect.contains_pt(x,y)) {
                // click on bar but outside scrollbar
                int16_t moved = 0;
                if (y > this->scroll.y + this->scroll.cy) { // down
                    moved = this->move_scroll(this->step);
                }
                else if (y < this->scroll.y) { // up
                    moved = this->move_scroll(- this->step);
                }
                if (moved) {
                    this->refresh_all();
                }
            }
        }
        if (device_flags == (MOUSE_FLAG_MOVE)) {
            if (this->y_click != -1) {
                int16_t moved = this->move_scroll(y - this->y_click);
                if (moved) {
                    // this->send_position_to_frame();
                    // this->frame->refresh(this->frame->rect);
                    // this->refresh(this->rect);
                    this->refresh_all();
                    this->y_click = y;
                }
            }
        }
        if (device_flags == (MOUSE_FLAG_BUTTON1)) {
            this->y_click = -1;
        }
    }
    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_DOWN_ARROW:
                keymap->get_kevent();
                if (this->move_scroll(this->step)) {
                    this->refresh_all();
                }
                break;
            case Keymap2::KEVENT_UP_ARROW:
                keymap->get_kevent();
                if (this->move_scroll(- this->step)) {
                    this->refresh_all();
                }
                break;
            default:
                break;
            }
        }
    }

    // Maybe move_scroll and refresh_all should be merged together
    void refresh_all() {
        this->send_position_to_frame();
        this->frame->refresh(this->frame->rect);
        this->refresh(this->rect);
    }
};

class WidgetHScrollBar : public Widget2 {
public:
    WidgetFrame * frame;
    uint16_t arrow_width;

    uint16_t bar_width; // = frame_width - 2 * arrow_width
    Rect scroll;

    uint16_t widget_width;
    int fg_color;
    int bg_color;
    int focus_color;
    int16_t x_click;
    int step;

    WidgetHScrollBar(gdi::GraphicApi & drawable, Widget2 & parent, NotifyApi * notifier,
                     int fg_color, int bg_color, int focus_color, int group_id = 0)
        : Widget2(drawable, Rect(0, 0, 1, 14), parent, notifier, group_id)
        , frame(nullptr)
        , arrow_width(0)
        , bar_width(0)
        , scroll(Rect(this->rect.x + 1, this->rect.y + 1, 1, 12))
        , widget_width(0)
        , fg_color(fg_color)
        , bg_color(bg_color)
        , focus_color(focus_color)
        , x_click(-1)
        , step(23)
    {
    }

    virtual ~WidgetHScrollBar() {}

    void set_x(int16_t x) override {
        this->scroll.x = x + 1;
        Widget2::set_x(x);
    }

    void set_y(int16_t y) override {
        this->scroll.y = y + 1;
        Widget2::set_y(y);
    }

    void set_frame(WidgetFrame * wframe) {
        if (wframe) {
            this->frame = wframe;
            this->set_xy(this->frame->x(), this->frame->bottom());
            this->rect.cx = this->frame->cx();
            this->bar_width = this->rect.cx - 2;
            this->set_scroll_width();
        }
    }

    void set_scroll_width() {
        if (this->frame->need_horizontal_scroll()) {
            this->widget_width = this->frame->wid->rect.cx;
            this->scroll.cx = (this->bar_width * this->rect.cx) / this->widget_width;
            LOG(LOG_INFO, "Set scroll width %d, widget width : %d", this->scroll.cx, this->widget_width);
        }
        else {
            this->widget_width = 0;
        }
    }

    uint get_percent() {
        return (this->scroll.x - this->rect.x) * 100 / (this->bar_width - this->scroll.cx);
    }

    void set_horizontal_scroll(uint percent) {
        this->scroll.x = this->rect.x + 1  + percent * (this->bar_width - this->scroll.cx) / 100;
        if (this->frame) {
            this->frame->set_horizontal_widget_pos(percent);
        }
    }

    void send_position_to_frame() {
        if (this->frame) {
            this->frame->set_horizontal_widget_pos(this->get_percent());
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
        if (this->widget_width) {
            int color = this->fg_color;
            if (this->has_focus) {
                color = this->focus_color;
            }
            this->drawable.draw(RDPOpaqueRect(this->scroll,
                                              color),
                                clip);
        }
        // LOG(LOG_INFO, "get percent %d",this->get_percent());
    }

    int16_t move_scroll(int16_t mx) {
        int16_t new_x_pos = this->scroll.x + mx;
        int16_t res = 0;
        if ((new_x_pos > this->x()) && (new_x_pos + this->scroll.cx < this->right())) {
            this->scroll.x = new_x_pos;
            res = mx;
        }
        else if ((new_x_pos <= this->x()) && (this->scroll.x >= this->x() + 1)) {
            res = this->x() + 1 - this->scroll.x;
            this->scroll.x = this->x() + 1;
        }
        else if ((new_x_pos + this->scroll.cx >= this->right()) &&
                 (this->scroll.x + this->scroll.cx + 1 <= this->right())) {
            res = this->right() - this->scroll.cx - 1 - this->scroll.x;
            this->scroll.x = this->right() - this->scroll.cx - 1;
        }
        return res;
    }

    // - mouve event (mouse moves or a button went up or down)
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            // on click
            if (this->scroll.contains_pt(x, y)) {
                this->x_click = x;
            }
            else if (this->rect.contains_pt(x,y)) {
                // click on bar but outside scrollbar
                int16_t moved = 0;
                if (x > this->scroll.x + this->scroll.cx) { // right
                    moved = this->move_scroll(this->step);
                }
                else if (x < this->scroll.x) { // left
                    moved = this->move_scroll(- this->step);
                }
                if (moved) {
                    this->refresh_all();
                }
            }
        }
        if (device_flags == (MOUSE_FLAG_MOVE)) {
            if (this->x_click != -1) {
                // on drag (previously clicked)
                int16_t moved = this->move_scroll(x - this->x_click);
                if (moved) {
                    this->refresh_all();
                    this->x_click = x;
                }
            }
        }
        if (device_flags == (MOUSE_FLAG_BUTTON1)) {
            // on release
            this->x_click = -1;
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_RIGHT_ARROW:
                keymap->get_kevent();
                if (this->move_scroll(this->step)) {
                    this->refresh_all();
                }
                break;
            case Keymap2::KEVENT_LEFT_ARROW:
                keymap->get_kevent();
                if (this->move_scroll(- this->step)) {
                    this->refresh_all();
                }
                break;
            default:
                break;
            }
        }
    }

    void refresh_all() {
        this->send_position_to_frame();
        this->frame->refresh(this->frame->rect);
        this->refresh(this->rect);
    }
};


