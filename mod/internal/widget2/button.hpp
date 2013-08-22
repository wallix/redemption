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

#if !defined(REDEMPTION_MOD_WIDGET2_BUTTON_HPP)
#define REDEMPTION_MOD_WIDGET2_BUTTON_HPP

#include "widget.hpp"
#include "label.hpp"

class WidgetButton : public Widget2
{
public:
    WidgetLabel label;
    int state;
    notify_event_t event;
    int border_right_bottom_color;
    int border_right_bottom_color2;
    int border_top_left_color;
    int border_top_left_color2;
    bool focus_is_visible;

    WidgetButton(DrawApi & drawable, int16_t x, int16_t y, Widget2* parent,
                 NotifyApi* notifier, const char * text, bool auto_resize = true,
                 int group_id = 0, int fgcolor = BLACK, int bgcolor = WHITE,
                 int xtext = 0, int ytext = 0, notify_event_t notify_event = NOTIFY_SUBMIT)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, group_id)
    , label(drawable, 1, 1, this, 0, text, auto_resize, 0, fgcolor, bgcolor, xtext, ytext)
    , state(0)
    , event(notify_event)
    , border_right_bottom_color(BLACK)
    , border_right_bottom_color2(0x888888)
    , border_top_left_color(0xCCCCCC)
    , border_top_left_color2(BLACK)
    , focus_is_visible(false)
    {
        this->rect.cx = this->label.cx() + 3;
        this->rect.cy = this->label.cy() + 3;
    }

    virtual ~WidgetButton()
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

    virtual void set_wh(uint16_t w, uint16_t h)
    {
        this->set_button_cx(w);
        this->set_button_cy(h);
    }

    void update_draw_state(const Rect& clip)
    {
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
            //top
            this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy(), this->cx() - 1, 1
            )), this->border_right_bottom_color2), this->rect);
            this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx() + 1, this->dy() + 1, this->cx() - 3, 1
            )), this->border_top_left_color2), this->rect);
            //left
            this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy() + 1, 1, this->cy() - 2
            )), this->border_right_bottom_color2), this->rect);
            this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx() + 1, this->dy() + 1, 1, this->cy() - 3
            )), this->border_top_left_color2), this->rect);
        }
        else {
            this->label.draw(clip);
            //top
            this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy(), this->cx() - 1, 1
            )), this->border_top_left_color), this->rect);
            //left
            this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy() + 1, 1, this->cy() - 2
            )), this->border_top_left_color), this->rect);
        }

        if (this->has_focus || this->focus_is_visible) {
            this->draw_focus(clip);
        }
    }

    virtual void draw(const Rect& clip)
    {
        this->update_draw_state(clip);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy(), 1, this->cy()
        )), this->border_right_bottom_color), this->rect);
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 2, this->dy() + 1, 1, this->cy() - 2
        )), this->border_right_bottom_color2), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), this->border_right_bottom_color), this->rect);
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + this->cy() - 2, this->cx() - 2, 1
        )), this->border_right_bottom_color2), this->rect);
    }

    void swap_border_color()
    {
        this->border_top_left_color ^= this->border_right_bottom_color;
        this->border_right_bottom_color ^= this->border_top_left_color;
        this->border_top_left_color ^= this->border_right_bottom_color;
        this->drawable.begin_update();
        this->update_draw_state(this->rect);
        this->drawable.end_update();
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN) && (this->state & 1) == 0) {
            this->state |= 1;
            this->swap_border_color();
            this->focus_is_visible = (this->focus_flag ^ IGNORE_FOCUS);
        }
        else if (device_flags == MOUSE_FLAG_BUTTON1 && this->state & 1) {
            this->state &= ~1;
            this->swap_border_color();
            if (this->rect.contains_pt(x, y)) {
                this->send_notify(this->event);
            }
            else if (this->has_focus) {
                this->draw_focus(this->rect);
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
    struct ImplFocusBorder {
        DrawApi & drawable;
        Rect & rect;
        int bevel;

        ImplFocusBorder(Rect & rect, int bevel, DrawApi & drawable)
        : drawable(drawable)
        , rect(rect)
        , bevel(bevel)
        {}

        Rect top()
        { return Rect(rect.x + 2 + bevel, rect.y + 2 + bevel, rect.cx - 5 - bevel, 2); }

        Rect left()
        { return Rect(rect.x + 2 + bevel, rect.y + 4 + bevel, 2, rect.cy - 7 - bevel); }

        Rect right()
        { return Rect(rect.x + rect.cx - 5, rect.y + 4 + bevel, 2, rect.cy - 7 - bevel); }

        Rect bottom()
        { return Rect(rect.x + 2 + bevel, rect.y + rect.cy - 5, rect.cx - 5 - bevel, 2); }

        void draw_border(const Rect& rect, const Rect& clip)
        {
            drawable.draw(RDPPatBlt(rect, 0xF0, GREY, BLACK, RDPBrush(this->rect.x, this->rect.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
        }
    };

    void draw_focus(const Rect& clip)
    {
        ImplFocusBorder impl(this->rect, (state & 1), this->drawable);
        this->drawable.begin_update();
        impl.draw_border(impl.top(), clip);
        impl.draw_border(impl.left(), clip);
        impl.draw_border(impl.right(), clip);
        impl.draw_border(impl.bottom(), clip);
        this->drawable.end_update();

        this->focus_is_visible = true;
    }

public:
    virtual bool focus(Widget2* old_focused, int policy = 0)
    {
        if (policy != 2) {
            this->draw_focus(this->rect);
        }
        return Widget2::focus(old_focused, policy);
    }

    virtual void blur()
    {
        if (this->focus_is_visible){
            ImplFocusBorder impl(this->rect, (state & 1), this->drawable);
            this->drawable.begin_update();
            this->label.draw(impl.top());
            this->label.draw(impl.left());
            this->label.draw(impl.bottom());
            this->label.draw(impl.right());
            this->drawable.end_update();
            this->focus_is_visible = false;
        }
        Widget2::blur();
    }
};

#endif
