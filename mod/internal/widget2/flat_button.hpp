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

#if !defined(REDEMPTION_MOD_WIDGET2_FLAT_BUTTON_HPP)
#define REDEMPTION_MOD_WIDGET2_FLAT_BUTTON_HPP

#include "widget.hpp"
#include "label.hpp"

class WidgetFlatButton : public Widget2
{
public:
    WidgetLabel label;
    int state;
    notify_event_t event;
    int fg_color;
    int bg_color;
    int focus_color;
    bool logo;

    WidgetFlatButton(DrawApi & drawable, int16_t x, int16_t y, Widget2& parent,
                     NotifyApi* notifier, const char * text, bool auto_resize = true,
                     int group_id = 0, int fgcolor = WHITE, int bgcolor = DARK_BLUE_BIS,
                     int xtext = 0, int ytext = 0, bool logo = false
                     /*, notify_event_t notify_event = NOTIFY_SUBMIT*/)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, group_id)
    , label(drawable, 1, 1, *this, 0, text, auto_resize, 0, fgcolor, bgcolor, xtext, ytext)
    , state(0)
    , event(NOTIFY_SUBMIT)
    , fg_color(fgcolor)
    , bg_color(bgcolor)
    , focus_color(WINBLUE)
    , logo(logo)
    {
        BEGINBODY
        this->rect.cx = this->label.cx() + 3;
        this->rect.cy = this->label.cy() + 3;
        ENDBODY
    }

    virtual ~WidgetFlatButton()
    {}

    void set_button_x(int x)
    {
        this->rect.x = x;
        this->label.rect.x = x + 1;
    }

    void set_button_y(int y)
    {
        this->rect.y = y;
        this->label.rect.y = y + 1;
    }

    void set_button_cx(int w)
    {
        this->rect.cx = w;
        this->label.rect.cx = w - 3;
    }

    void set_button_cy(int h)
    {
        this->rect.cy = h;
        this->label.rect.cy = h - 3;
    }

    virtual void set_xy(int16_t x, int16_t y)
    {
        this->set_button_x(x);
        this->set_button_y(y);
    }

    void update_draw_state(const Rect& clip)
    {
        // Label color
        if (this->logo) {
            if (this->has_focus) {
                this->label.fg_color = this->focus_color;
            }
            else {
                this->label.fg_color = this->fg_color;
            }
        }
        else {
            if (this->has_focus) {
                this->label.bg_color = this->focus_color;
            }
            else {
                this->label.bg_color = this->bg_color;
            }
        }
        // background
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx(), this->cy()
            )), this->label.bg_color), this->rect);

        if (this->state & 1)  {
            ++this->label.rect.x;
            ++this->label.rect.y;
            --this->label.rect.cx;
            --this->label.rect.cy;
            this->label.draw(clip);
            --this->label.rect.x;
            --this->label.rect.y;
            ++this->label.rect.cx;
            ++this->label.rect.cy;

        }
        else {
            this->label.draw(clip);
        }

        if (this->logo)
            return;

        // border
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              this->dx(), this->dy(), this->cx() - 2, 2
              )), this->fg_color), this->rect);
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              this->dx(), this->dy() + 2, 2, this->cy() - 2
              )), this->fg_color), this->rect);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              this->dx() + this->cx() - 2, this->dy(), 2, this->cy()
              )), this->fg_color), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              this->dx(), this->dy() + this->cy() - 2, this->cx(), 2
              )), this->fg_color), this->rect);

    }

    virtual void draw(const Rect& clip)
    {
        this->update_draw_state(clip);
    }

    void swap_border_color()
    {
        this->drawable.begin_update();
        this->draw(this->rect);
        this->drawable.end_update();
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN) && (this->state & 1) == 0) {
            this->state |= 1;
            this->swap_border_color();
        }
        else if (device_flags == MOUSE_FLAG_BUTTON1 && this->state & 1) {
            this->state &= ~1;
            this->swap_border_color();
            if (this->rect.contains_pt(x, y)) {
                this->send_notify(this->event);
            }
        }
        else
            this->Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(this->event);
                    break;
                case Keymap2::KEVENT_KEY:
                    if (keymap->get_char() == ' ') {
                        this->send_notify(this->event);
                    }
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
        }
    }

private:


public:
    virtual void blur()
    {
        Widget2::blur();
    }
};

#endif
