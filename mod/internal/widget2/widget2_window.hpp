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

class Window : public WidgetComposite
{
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

    Window(DrawApi& drawable, const Rect& rect, Widget2* parent, NotifyApi* notifier,
           const char * caption, int bgcolor = DARK_WABGREEN, int group_id = 0)
    : WidgetComposite(drawable, rect, parent, notifier, group_id)
    , titlebar(drawable, 2, 2, this, NULL, caption, false, -1, WHITE, WABGREEN, 5)
    , button_close(drawable, 2, 2, this, this, "X", true, -2, WHITE, DARK_GREEN, 0, -1, NOTIFY_CANCEL)
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
        this->child_list.push_back(&this->titlebar);
        this->child_list.push_back(&this->button_close);

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
    {}

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
        this->WidgetComposite::draw(inner_window);
        this->WidgetComposite::draw_inner_free(inner_window, this->bg_color);

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

    virtual void focus()
    {
        // LOG(LOG_INFO, "window focus %p", this);
        if (!this->has_focus){
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
            if (this->current_focus) {
                this->current_focus->focus();
            }
            this->refresh(this->rect);
        }
    }

    virtual void blur()
    {
        // LOG(LOG_INFO, "window blur %p", this);
        if (this->has_focus){
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            if (this->current_focus) {
                this->current_focus->blur();
            }
            this->refresh(this->rect);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        Widget2 * w = this->widget_at_pos(x, y);

        // Mouse clic release
        // w could be null if mouse is located at an empty space
        if (device_flags == MOUSE_FLAG_BUTTON1) {
            if (this->current_focus && (w != this->current_focus)) {
                this->current_focus->rdp_input_mouse(device_flags, x, y, keymap);
            }
        }
        if (w){
            // Mouse clic pressed
            if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
                if ((w->focus_flag != IGNORE_FOCUS) && (w != this->current_focus)){
                    if (this->current_focus) {
                        this->current_focus->blur();
                    }
                    this->current_focus = w;
                    this->current_focus->focus();
                }
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
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


};

#endif
