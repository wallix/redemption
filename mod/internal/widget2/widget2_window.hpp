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

#if !defined(REDEMPTION_MOD_WIDGET2_WINDOW_HPP_)
#define REDEMPTION_MOD_WIDGET2_WINDOW_HPP_

#include "composite.hpp"
#include "label.hpp"
#include "colors.hpp"
#include "button.hpp"

class Window : public WidgetParent
{
    CompositeInterface * impl;
public:
    WidgetLabel titlebar;
    WidgetButton button_close;
    int bg_color;
    int active_border_top_left_color;
    int active_border_top_left_color_inner;
    int active_border_right_bottom_color;
    int active_border_right_bottom_color_inner;
    int inactive_border_top_left_color;
    int inactive_border_top_left_color_inner;
    int inactive_border_right_bottom_color;
    int inactive_border_right_bottom_color_inner;
    int titlebar_base_width;

    Window(DrawApi& drawable, const Rect& rect, Widget2 & parent, NotifyApi* notifier,
           const char * caption, int bgcolor = DARK_WABGREEN, int group_id = 0)
    : WidgetParent(drawable, rect, parent, notifier, group_id)
    , titlebar(drawable, 2, 2, *this, NULL, caption, false, -1, WHITE, WABGREEN, 5)
    , button_close(drawable, 2, 2, *this, this, "X", true, -2, WHITE, DARK_GREEN, 0, -1)
    , bg_color(bgcolor)
    , active_border_top_left_color(0xEEEEEE)
    , active_border_top_left_color_inner(0xEEEEEE)
    , active_border_right_bottom_color(0x666666)
    , active_border_right_bottom_color_inner(0x888888)
    , inactive_border_top_left_color(0x888888)
    , inactive_border_top_left_color_inner(0x888888)
    , inactive_border_right_bottom_color(0x888888)
    , inactive_border_right_bottom_color_inner(0x888888)
    {
        this->tab_flag = DELEGATE_CONTROL_TAB;
        // this->impl = new CompositeVector;
        this->impl = new CompositeTable;

        this->add_widget(&this->titlebar);
        this->add_widget(&this->button_close);

        // this->titlebar.tab_flag = IGNORE_TAB;
        // this->button_close.tab_flag = IGNORE_TAB;
        // this->button_close.focus_flag = IGNORE_FOCUS;

        this->tab_flag |= NO_DELEGATE_PARENT;

        this->button_close.label.x_text = 2;
        this->button_close.set_button_cx(this->button_close.label.cx()*2);
        this->button_close.set_button_cy(this->button_close.cy() - 2);

        int h;
        this->drawable.text_metrics(this->titlebar.buffer, this->titlebar_base_width, h);
        this->titlebar.rect.cy = std::max<int>(h - 2, this->button_close.cy()) + this->titlebar.y_text * 2;
    }

    virtual ~Window()
    {
        if (this->impl) {
            delete this->impl;
            this->impl = NULL;
        }
    }

    void set_window_x(int x)
    {
        this->titlebar.rect.x = x + 2;
        this->button_close.set_button_x(this->dx() + this->cx() - this->button_close.cx() - 2);
        this->rect.x = x;
    }

    void set_window_y(int y)
    {
        this->titlebar.rect.y = y + 2;
        this->button_close.set_button_y(y + 2);
        this->rect.y = y;
    }

public:

    void resize_titlebar()
    {
        this->titlebar.rect.cx = this->cx() - this->button_close.cx() - 4;
        this->button_close.set_button_x(this->dx() + this->cx() - this->button_close.cx() - 2);
    }

    void set_window_cx(int w)
    {
        this->rect.cx = w;
        this->resize_titlebar();
    }

    void set_window_cy(int h)
    {
        this->rect.cy = h;
    }

    virtual void draw(const Rect& clip)
    {
        Rect inner_window = clip.intersect(this->rect.shrink(2));
        this->impl->draw(inner_window);
        this->draw_inner_free(inner_window, this->bg_color);

        if (this->has_focus) {
            this->draw_border(clip,
                              this->active_border_top_left_color,
                              this->active_border_top_left_color_inner,
                              this->active_border_right_bottom_color,
                              this->active_border_right_bottom_color_inner);
        }
        else {
            this->draw_border(clip,
                              this->inactive_border_top_left_color,
                              this->inactive_border_top_left_color_inner,
                              this->inactive_border_right_bottom_color,
                              this->inactive_border_right_bottom_color_inner);
        }
    }


    virtual void notify(Widget2* widget, NotifyApi::notify_event_t event) {
        if (widget == &this->button_close && event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_CANCEL);
        }
        else {
            WidgetParent::notify(widget, event);
        }
    }

    void draw_border(const Rect& clip,
                     int border_top_left_color, int border_top_left_color_inner,
                     int border_right_bottom_color, int border_right_bottom_color_inner)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx(), 1
        )), border_top_left_color), this->rect);
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + 1, 1, this->cy() - 2
        )), border_top_left_color), this->rect);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy() + 1, 1, this->cy() - 2
        )), border_right_bottom_color), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), border_right_bottom_color), this->rect);

        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + 1, this->cx() - 2, 1
        )), border_top_left_color_inner), this->rect);
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + 2, 1, this->cy() - 4
        )), border_top_left_color_inner), this->rect);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 2, this->dy() + 2, 1, this->cy() - 4
        )), border_right_bottom_color_inner), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + this->cy() - 2, this->cx() - 2, 1
        )), border_right_bottom_color_inner), this->rect);
    }

    virtual void add_widget(Widget2 * w) {
        this->impl->add_widget(w);
    }
    virtual void remove_widget(Widget2 * w) {
        this->impl->remove_widget(w);
    }
    virtual void clear() {
        this->impl->clear();
    }

    virtual void set_xy(int16_t x, int16_t y) {
        int16_t xx = x - this->dx();
        int16_t yy = y - this->dy();
        this->impl->set_xy(xx, yy);
        WidgetParent::set_xy(x, y);
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) {
        if (!this->rect.contains_pt(x, y))
            return 0;
        if (this->current_focus) {
            if (this->current_focus->rect.contains_pt(x, y)) {
                return this->current_focus;
            }
        }
        return this->impl->widget_at_pos(x, y);
    }

    virtual bool next_focus() {
        return this->impl->next_focus(this);
    }

    virtual bool previous_focus() {
        return this->impl->previous_focus(this);
    }

    virtual void draw_inner_free(const Rect& clip, int bg_color) {
        Region region;
        region.rects.push_back(clip);

        this->impl->draw_inner_free(clip, bg_color, region);

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable.draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }
    }


};

#endif
