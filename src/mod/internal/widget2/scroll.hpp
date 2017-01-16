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


#include "mod/internal/widget2/flat_button.hpp"

/*
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

    WidgetFrame(gdi::GraphicApi & drawable, const Rect rect, Widget2 & parent,
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

//    virtual void draw(const Rect clip) {
    void rdp_input_invalidate(Rect clip) override {
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
*/

/*
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

//    virtual void draw(const Rect clip) {
    void rdp_input_invalidate(Rect clip) override {
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
*/

/*
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

//    virtual void draw(const Rect clip) {
    void rdp_input_invalidate(Rect clip) override {
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
*/

/*
class WidgetHScrollBar : public Widget2 {
    int fg_color;
    int bg_color;
    int focus_color;

    Font const & font;

    Rect left_button_rect;
    Rect right_button_rect;
    Rect scroll_bar_rect;
    Rect cursor_button_rect;

    int current_value = 0;
    int max_value     = 100;

    int step_value = 1;

    bool mouse_down = false;

    enum {
        BUTTON_NONE,

        BUTTON_LEFT,
        BUTTON_CURSOR,
        BUTTON_RIGHT
    } selected_button = BUTTON_NONE;

    notify_event_t event;

    int old_mouse_x         = 0;
    int old_cursor_button_x = 0;

    uint16_t button_width = 0;

public:
    WidgetHScrollBar(gdi::GraphicApi & drawable, Widget2& parent,
                     NotifyApi* notifier,
                     int group_id, int fgcolor, int bgcolor, int focuscolor,
                     Font const & font, int maxvalue = 100)
    : Widget2(drawable, parent, notifier, group_id)
    , fg_color(fgcolor)
    , bg_color(bgcolor)
    , focus_color(focuscolor)
    , font(font)
    , max_value(maxvalue)
    , event(NOTIFY_HSCROLL)
    {
        Dimension dim = this->get_optimal_button_dim();
        this->button_width = dim.w;
    }

private:
    void compute_step_value() {
        this->step_value = (this->scroll_bar_rect.cx - cursor_button_rect.cx) / this->max_value;
    }

public:
    unsigned int get_current_value() const {
        return static_cast<unsigned int>(this->current_value);
    }

    void set_current_value(unsigned int cv) {
        this->current_value = static_cast<int>(
            std::min<int>(cv, this->max_value));
    }

    void set_max_value(unsigned int maxvalue) {
        this->max_value = static_cast<int>(maxvalue);

        this->compute_step_value();
    }

private:
    void update_cursor_button_rects() {
        this->cursor_button_rect.x  = this->x() + this->button_width - 1 +
                                          (this->cx() - this->button_width * 2 + 2 - this->button_width) *
                                              this->current_value / this->max_value;
        this->cursor_button_rect.y  = this->y() + 1;
        this->cursor_button_rect.cx = this->button_width;
        this->cursor_button_rect.cy = this->cy() - 2;
    }

    void update_rects() {
        this->left_button_rect.x  = this->x();
        this->left_button_rect.y  = this->y();
        this->left_button_rect.cx = this->button_width;
        this->left_button_rect.cy = this->cy();

        this->right_button_rect.x  = this->x() + this->cx() - this->button_width;
        this->right_button_rect.y  = this->y();
        this->right_button_rect.cx = this->button_width;
        this->right_button_rect.cy = this->cy();

        this->scroll_bar_rect.x  = this->x() + this->button_width;
        this->scroll_bar_rect.y  = this->y();
        this->scroll_bar_rect.cx = this->cx() - this->button_width * 2;
        this->scroll_bar_rect.cy = this->cy();

        this->update_cursor_button_rects();

        this->compute_step_value();
    }

public:
    // Widget2

//    void draw(const Rect clip) override {
    void rdp_input_invalidate(Rect clip) override {
        WidgetFlatButton::draw(clip, this->left_button_rect, this->drawable,
            false, (this->mouse_down && (this->selected_button == BUTTON_LEFT)),
            "◀", this->fg_color, this->bg_color, this->focus_color,
            Rect(), 0, 2, this->font, 2, 1);

        WidgetFlatButton::draw(clip, this->right_button_rect, this->drawable,
            false, (this->mouse_down && (this->selected_button == BUTTON_RIGHT)),
            "▶", this->fg_color, this->bg_color, this->focus_color,
            Rect(), 0, 2, this->font, 2, 1);

        this->drawable.draw(
                RDPOpaqueRect(
                        clip.intersect(
                                Rect(this->scroll_bar_rect.x,
                                     this->scroll_bar_rect.y + 1,
                                     this->scroll_bar_rect.cx,
                                     this->scroll_bar_rect.cy - 2)
                            ),
                        this->bg_color
                    ),
                this->get_rect()
            );

        this->drawable.draw(
                RDPOpaqueRect(
                        clip.intersect(
                                Rect(this->scroll_bar_rect.x,
                                     this->scroll_bar_rect.y,
                                     this->scroll_bar_rect.cx,
                                     1)
                            ),
                        this->fg_color
                    ),
                this->get_rect()
            );

        this->drawable.draw(
                RDPOpaqueRect(
                        clip.intersect(
                                Rect(this->scroll_bar_rect.x,
                                     this->scroll_bar_rect.y + this->cy() - 1,
                                     this->scroll_bar_rect.cx,
                                     1)
                            ),
                        this->fg_color
                    ),
                this->get_rect()
            );

        WidgetFlatButton::draw(clip, this->cursor_button_rect, this->drawable,
            false, (this->mouse_down && (this->selected_button == BUTTON_CURSOR)),
            "≡", this->fg_color, this->bg_color, this->focus_color,
            Rect(), 0, 1, this->font, 3, 1);
    }

    void set_x(int16_t x) override {
        Widget2::set_x(x);
        this->update_rects();
    }

    void set_y(int16_t y) override {
        Widget2::set_y(y);
        this->update_rects();
    }

    void set_cx(uint16_t cx) override {
        Widget2::set_cx(cx);
        this->update_rects();
    }

    void set_cy(uint16_t cy) override {
        Widget2::set_cy(cy);
        this->update_rects();
    }

private:
    Dimension get_optimal_button_dim() {
        Dimension dim = WidgetFlatButton::get_optimal_dim(1, this->font, "▶", 3, 2);

        dim.w += 1;
        dim.h += 2;

        return dim;
    }

public:
    Dimension get_optimal_dim() override {
        return this->get_optimal_button_dim();
    }

    // RdpInput
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
            this->mouse_down = true;

            if (this->left_button_rect.contains_pt(x, y)) {
                this->selected_button = BUTTON_LEFT;

                const int old_value = this->current_value;

                this->current_value -= this->step_value;

                if (this->current_value < 0) {
                    this->current_value = 0;
                }

                if (old_value != this->current_value) {
                    this->update_cursor_button_rects();

                    this->send_notify(this->event);
                }
            }
            else if (this->cursor_button_rect.contains_pt(x, y)) {
                this->selected_button = BUTTON_CURSOR;

                this->old_mouse_x         = x;
                this->old_cursor_button_x = this->cursor_button_rect.x;
            }
            if (this->right_button_rect.contains_pt(x, y)) {
                this->selected_button = BUTTON_RIGHT;

                const int old_value = this->current_value;

                this->current_value += this->step_value;

                if (this->current_value > this->max_value) {
                    this->current_value = this->max_value;
                }

                if (old_value != this->current_value) {
                    this->update_cursor_button_rects();

                    this->send_notify(this->event);
                }
            }

            this->drawable.begin_update();
            this->draw(this->get_rect());
            this->drawable.end_update();
        }
        else if (device_flags == MOUSE_FLAG_BUTTON1) {
            this->mouse_down          = false;
            this->selected_button     = BUTTON_NONE;
            this->old_mouse_x         = 0;
            this->old_cursor_button_x = 0;

            this->drawable.begin_update();
            this->draw(this->get_rect());
            this->drawable.end_update();
        }
        else if (device_flags == MOUSE_FLAG_MOVE) {
            if (this->mouse_down && (BUTTON_CURSOR == this->selected_button)) {
                const int old_value = this->current_value;

                const int min_button_x = this->x() + this->button_width - 1;
                const int max_button_x = this->x() + this->button_width - 1 +
                                      (this->cx() - this->button_width * 2 + 2 - this->button_width);

                const int min_x = min_button_x + (this->old_mouse_x - this->old_cursor_button_x);
                const int max_x = max_button_x + (this->old_mouse_x - this->old_cursor_button_x);

                if (x < min_x) {
                    this->current_value = 0;
                }
                else if (x >= max_x) {
                    this->current_value = this->max_value;
                }
                else {
                    this->current_value =
                        (x - min_x) * this->max_value / (max_x - min_x);
                }

                if (old_value != this->current_value) {
                    this->update_cursor_button_rects();

                    this->send_notify(this->event);
                }

                this->drawable.begin_update();
                this->draw(this->get_rect());
                this->drawable.end_update();
            }
        }
        else
            this->Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }
};
*/

class WidgetScrollBar : public Widget2 {
    bool horizontal;

    int fg_color;
    int bg_color;
    int focus_color;

    Font const & font;

    Rect left_or_top_button_rect;
    Rect right_or_bottom_button_rect;
    Rect scroll_bar_rect;
    Rect cursor_button_rect;

    int current_value = 0;
    int max_value     = 100;

    int step_value = 1;

    bool mouse_down = false;

    enum {
        BUTTON_NONE,

        BUTTON_LEFT_OR_TOP,
        BUTTON_CURSOR,
        BUTTON_RIGHT_OR_BOTTOM
    } selected_button = BUTTON_NONE;

    notify_event_t event;

    int old_mouse_x_or_y         = 0;
    int old_cursor_button_x_or_y = 0;

    uint16_t button_width_or_height = 0;

public:
    WidgetScrollBar(gdi::GraphicApi & drawable, Widget2& parent,
                    NotifyApi* notifier, bool horizontal,
                    int group_id, int fgcolor, int bgcolor, int focuscolor,
                    Font const & font, int maxvalue = 100)
    : Widget2(drawable, parent, notifier, group_id)
    , horizontal(horizontal)
    , fg_color(fgcolor)
    , bg_color(bgcolor)
    , focus_color(focuscolor)
    , font(font)
    , max_value(maxvalue)
    , event(horizontal ? NOTIFY_HSCROLL : NOTIFY_VSCROLL)
    {
        Dimension dim = this->get_optimal_button_dim();
        this->button_width_or_height = (this->horizontal ? dim.w : dim.h);
    }

private:
    void compute_step_value() {
        this->step_value =
            (this->horizontal ?
             ((this->scroll_bar_rect.cx - cursor_button_rect.cx) / this->max_value) :
             ((this->scroll_bar_rect.cy - cursor_button_rect.cy) / this->max_value));
    }

public:
    unsigned int get_current_value() const {
        return static_cast<unsigned int>(this->current_value);
    }

    void set_current_value(unsigned int cv) {
        this->current_value = static_cast<int>(
            std::min<int>(cv, this->max_value));
    }

    void set_max_value(unsigned int maxvalue) {
        this->max_value = static_cast<int>(maxvalue);

        this->compute_step_value();
    }

private:
    void update_cursor_button_rects() {
        if (this->horizontal) {
            if (!this->cx()) {
                return;
            }

            this->cursor_button_rect.x  = this->x() + this->button_width_or_height - 1 +
                                              (this->cx() - this->button_width_or_height * 2 + 2 - this->button_width_or_height) *
                                                  this->current_value / this->max_value;
            this->cursor_button_rect.y  = this->y() + 1;
            this->cursor_button_rect.cx = this->button_width_or_height;
            this->cursor_button_rect.cy = this->cy() - 2;
        }
        else {
            if (!this->cy()) {
                return;
            }

            this->cursor_button_rect.x  = this->x() + 1;
            this->cursor_button_rect.y  = this->y() + this->button_width_or_height - 1 +
                                              (this->cy() - this->button_width_or_height * 2 + 2 - this->button_width_or_height) *
                                                  this->current_value / this->max_value;
            this->cursor_button_rect.cx = this->cx() - 2;
            this->cursor_button_rect.cy = this->button_width_or_height;
        }
    }

    void update_rects() {
        if (this->horizontal) {
            this->left_or_top_button_rect.x  = this->x();
            this->left_or_top_button_rect.y  = this->y();
            this->left_or_top_button_rect.cx = this->button_width_or_height;
            this->left_or_top_button_rect.cy = this->cy();

            this->right_or_bottom_button_rect.x  = this->x() + this->cx() - this->button_width_or_height;
            this->right_or_bottom_button_rect.y  = this->y();
            this->right_or_bottom_button_rect.cx = this->button_width_or_height;
            this->right_or_bottom_button_rect.cy = this->cy();

            this->scroll_bar_rect.x  = this->x() + this->button_width_or_height;
            this->scroll_bar_rect.y  = this->y();
            this->scroll_bar_rect.cx = this->cx() - this->button_width_or_height * 2;
            this->scroll_bar_rect.cy = this->cy();
        }
        else {
            this->left_or_top_button_rect.x  = this->x();
            this->left_or_top_button_rect.y  = this->y();
            this->left_or_top_button_rect.cx = this->cx();
            this->left_or_top_button_rect.cy = this->button_width_or_height;

            this->right_or_bottom_button_rect.x  = this->x();
            this->right_or_bottom_button_rect.y  = this->y() + this->cy() - this->button_width_or_height;
            this->right_or_bottom_button_rect.cx = this->cx();
            this->right_or_bottom_button_rect.cy = this->button_width_or_height;

            this->scroll_bar_rect.x  = this->x();
            this->scroll_bar_rect.y  = this->y() + this->button_width_or_height;
            this->scroll_bar_rect.cx = this->cx();
            this->scroll_bar_rect.cy = this->cy() - this->button_width_or_height * 2;
        }

        this->update_cursor_button_rects();

        this->compute_step_value();
    }

public:
    // Widget2

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            if (this->horizontal) {
                WidgetFlatButton::draw(rect_intersect, this->left_or_top_button_rect, this->drawable,
                    false, (this->mouse_down && (this->selected_button == BUTTON_LEFT_OR_TOP)),
                    "◀", this->fg_color, this->bg_color, this->focus_color,
                    Rect(), 0, 2, this->font, 0, -1);

                WidgetFlatButton::draw(rect_intersect, this->right_or_bottom_button_rect, this->drawable,
                    false, (this->mouse_down && (this->selected_button == BUTTON_RIGHT_OR_BOTTOM)),
                    "▶", this->fg_color, this->bg_color, this->focus_color,
                    Rect(), 0, 2, this->font, 0, -1);

                this->drawable.draw(
                        RDPOpaqueRect(
                            rect_intersect.intersect(
                                    Rect(this->scroll_bar_rect.x,
                                            this->scroll_bar_rect.y + 1,
                                            this->scroll_bar_rect.cx,
                                            this->scroll_bar_rect.cy - 2)
                                ),
                            this->bg_color
                        ),
                        this->get_rect(),
                        gdi::ColorCtx::depth24()
                    );

                this->drawable.draw(
                        RDPOpaqueRect(
                            rect_intersect.intersect(
                                    Rect(this->scroll_bar_rect.x,
                                            this->scroll_bar_rect.y,
                                            this->scroll_bar_rect.cx,
                                            1)
                                ),
                            this->fg_color
                        ),
                        this->get_rect(),
                        gdi::ColorCtx::depth24()
                    );

                this->drawable.draw(
                        RDPOpaqueRect(
                            rect_intersect.intersect(
                                    Rect(this->scroll_bar_rect.x,
                                            this->scroll_bar_rect.y + this->cy() - 1,
                                            this->scroll_bar_rect.cx,
                                            1)
                                ),
                            this->fg_color
                        ),
                        this->get_rect(),
                        gdi::ColorCtx::depth24()
                    );

                WidgetFlatButton::draw(rect_intersect, this->cursor_button_rect, this->drawable,
                    false, (this->mouse_down && (this->selected_button == BUTTON_CURSOR)),
                    "▤", this->fg_color, this->bg_color, this->focus_color,
                    Rect(), 0, 1, this->font, 0, -1);
            }
            else {
                WidgetFlatButton::draw(rect_intersect, this->left_or_top_button_rect, this->drawable,
                    false, (this->mouse_down && (this->selected_button == BUTTON_LEFT_OR_TOP)),
                    "▲", this->fg_color, this->bg_color, this->focus_color,
                    Rect(), 0, 2, this->font, 0, -1);

                WidgetFlatButton::draw(rect_intersect, this->right_or_bottom_button_rect, this->drawable,
                    false, (this->mouse_down && (this->selected_button == BUTTON_RIGHT_OR_BOTTOM)),
                    "▼", this->fg_color, this->bg_color, this->focus_color,
                    Rect(), 0, 2, this->font, 0, -1);

                this->drawable.draw(
                        RDPOpaqueRect(
                            rect_intersect.intersect(
                                    Rect(this->scroll_bar_rect.x + 1,
                                            this->scroll_bar_rect.y,
                                            this->scroll_bar_rect.cx - 2,
                                            this->scroll_bar_rect.cy)
                                ),
                            this->bg_color
                        ),
                        this->get_rect(),
                        gdi::ColorCtx::depth24()
                    );

                this->drawable.draw(
                        RDPOpaqueRect(
                            rect_intersect.intersect(
                                    Rect(this->scroll_bar_rect.x,
                                            this->scroll_bar_rect.y,
                                            1,
                                            this->scroll_bar_rect.cy)
                                ),
                            this->fg_color
                        ),
                        this->get_rect(),
                        gdi::ColorCtx::depth24()
                    );

                this->drawable.draw(
                        RDPOpaqueRect(
                            rect_intersect.intersect(
                                    Rect(this->scroll_bar_rect.x + this->cx() - 1,
                                            this->scroll_bar_rect.y,
                                            1,
                                            this->scroll_bar_rect.cy)
                                ),
                            this->fg_color
                        ),
                        this->get_rect(),
                        gdi::ColorCtx::depth24()
                    );

                WidgetFlatButton::draw(rect_intersect, this->cursor_button_rect, this->drawable,
                    false, (this->mouse_down && (this->selected_button == BUTTON_CURSOR)),
                    "▥", this->fg_color, this->bg_color, this->focus_color,
                    Rect(), 0, 1, this->font, -1, 0);
            }

            this->drawable.end_update();
        }
    }

    void set_xy(int16_t x, int16_t y) override {
        Widget2::set_xy(x, y);
        this->update_rects();
    }

    void set_wh(uint16_t w, uint16_t h) override {
        Widget2::set_wh(w, h);
        this->update_rects();
    }

    using Widget2::set_wh;

private:
    Dimension get_optimal_button_dim() {
        if (this->horizontal) {
            Dimension dim = WidgetFlatButton::get_optimal_dim(1, this->font, "▶", 3, 2);

            dim.w += 1;
            dim.h += 2;

            return dim;
        }
        else {
            Dimension dim = WidgetFlatButton::get_optimal_dim(1, this->font, "▲", 3, 2);

            dim.w += 1;
            dim.h += 2;

            return dim;
        }
    }

public:
    Dimension get_optimal_dim() override {
        return this->get_optimal_button_dim();
    }

    // RdpInput
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
            this->mouse_down = true;

            if (this->left_or_top_button_rect.contains_pt(x, y)) {
                this->selected_button = BUTTON_LEFT_OR_TOP;

                const int old_value = this->current_value;

                this->current_value -= this->step_value;

                if (this->current_value < 0) {
                    this->current_value = 0;
                }

                if (old_value != this->current_value) {
                    this->update_cursor_button_rects();

                    this->send_notify(this->event);
                }
            }
            else if (this->cursor_button_rect.contains_pt(x, y)) {
                this->selected_button = BUTTON_CURSOR;

                this->old_mouse_x_or_y         = (this->horizontal ? x : y);
                this->old_cursor_button_x_or_y = (this->horizontal ? this->cursor_button_rect.x : this->cursor_button_rect.y);
            }
            if (this->right_or_bottom_button_rect.contains_pt(x, y)) {
                this->selected_button = BUTTON_RIGHT_OR_BOTTOM;

                const int old_value = this->current_value;

                this->current_value += this->step_value;

                if (this->current_value > this->max_value) {
                    this->current_value = this->max_value;
                }

                if (old_value != this->current_value) {
                    this->update_cursor_button_rects();

                    this->send_notify(this->event);
                }
            }

            this->rdp_input_invalidate(this->get_rect());
        }
        else if (device_flags == MOUSE_FLAG_BUTTON1) {
            this->mouse_down               = false;
            this->selected_button          = BUTTON_NONE;
            this->old_mouse_x_or_y         = 0;
            this->old_cursor_button_x_or_y = 0;

            this->rdp_input_invalidate(this->get_rect());
        }
        else if (device_flags == MOUSE_FLAG_MOVE) {
            if (this->mouse_down && (BUTTON_CURSOR == this->selected_button)) {
                const int old_value = this->current_value;

                const int min_button_x_or_y = (this->horizontal ? this->x() : this->y()) + this->button_width_or_height - 1;
                const int max_button_x_or_y = (this->horizontal ? this->x() : this->y()) + this->button_width_or_height - 1 +
                                              ((this->horizontal ? this->cx() : this->cy()) - this->button_width_or_height * 2 + 2 - this->button_width_or_height);

                const int min_x_or_y = min_button_x_or_y + (this->old_mouse_x_or_y - this->old_cursor_button_x_or_y);
                const int max_x_or_y = max_button_x_or_y + (this->old_mouse_x_or_y - this->old_cursor_button_x_or_y);

                if ((this->horizontal ? x : y) < min_x_or_y) {
                    this->current_value = 0;
                }
                else if ((this->horizontal ? x : y) >= max_x_or_y) {
                    this->current_value = this->max_value;
                }
                else {
                    this->current_value =
                        ((this->horizontal ? x : y) - min_x_or_y) * this->max_value / (max_x_or_y - min_x_or_y);
                }

                if (old_value != this->current_value) {
                    this->update_cursor_button_rects();

                    this->send_notify(this->event);
                }

                this->rdp_input_invalidate(this->get_rect());
            }
        }
        else
            this->Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }
};
