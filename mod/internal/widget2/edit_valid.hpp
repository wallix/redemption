/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1 of the License, or
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
 *   Copyright (C) Wallix 1010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_EDIT_VALID_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_EDIT_VALID_HPP

#include "widget.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "password.hpp"
#include "flat_button.hpp"
#include <keymap2.hpp>

class WidgetEditValid : public Widget2 {
public:
    WidgetFlatButton button;
    WidgetEdit * editbox;
    WidgetLabel * label;

    WidgetEditValid(DrawApi& drawable, int16_t x, int16_t y, uint16_t cx,
                    Widget2 & parent, NotifyApi* notifier, const char * text,
                    int group_id, int fgcolor, int bgcolor,
                    int focus_color, std::size_t edit_position = -1,
                    int xtext = 0, int ytext = 0, bool pass = false,
                    const char * title = NULL)
        : Widget2(drawable, Rect(0, 0, cx, 1), parent, notifier, group_id)
        , button(drawable, 0, 0, *this, this, "\xe2\x9e\x9c", true,
                 group_id, bgcolor, focus_color, focus_color, 6, 2)
        , editbox(pass ? new WidgetPassword(drawable, 0, 0, cx - this->button.cx(), *this,
                                            this, text, group_id, fgcolor, bgcolor,
                                            focus_color, edit_position, 1, 2)
                  : new WidgetEdit(drawable, 0, 0, cx - this->button.cx(), *this, this,
                                   text, group_id, fgcolor, bgcolor, focus_color,
                                   edit_position, 1, 2))
        , label(title ? new WidgetLabel(drawable, 0, 0, *this, 0, title, true,
                                        group_id, MEDIUM_GREY, bgcolor, 1, 2)
                : NULL)
    {
        this->button.set_button_x(this->editbox->lx() - 1);
        this->editbox->set_edit_cy(this->button.cy());
        this->rect.cy = this->editbox->cy();
        this->rect.cx = this->button.lx() - this->rect.x;
        this->set_xy(x, y);
        this->editbox->draw_border_focus = false;

        if (this->label) {
            this->label->rect.cx = this->editbox->rect.cx - 1;
            this->label->rect.cy = this->editbox->rect.cy - 1;
            ++this->label->rect.x;
            ++this->label->rect.y;
        }
    }

    virtual ~WidgetEditValid()
    {
        if (this->editbox) {
            delete this->editbox;
            this->editbox = NULL;
        }
        if (this->label) {
            delete this->label;
            this->label = NULL;
        }
    }

    virtual void set_text(const char * text/*, int position = 0*/)
    {
        this->editbox->set_text(text);
    }

    const char * get_text() const
    {
        return this->editbox->get_text();
    }

    virtual void set_edit_x(int x)
    {
        this->rect.x = x;
        this->editbox->set_edit_x(x);
        this->button.set_button_x(this->editbox->lx() - 1);

        if (this->label) {
            this->label->rect.x = x + 1;
        }
    }

    virtual void set_edit_y(int y)
    {
        this->rect.y = y;
        this->editbox->set_edit_y(y);
        this->button.set_button_y(y);

        if (this->label) {
            this->label->rect.y = y + 1;
        }
    }

    // virtual void set_edit_cx(int w)
    // {
    //     this->rect.cx = w;
    //     this->editbox->set_edit_cx(w - this->button.cx());
    //     this->button.set_button_x(this->editbox->lx());
    // }

    virtual void set_xy(int16_t x, int16_t y)
    {
        this->set_edit_x(x);
        this->set_edit_y(y);
    }

    virtual void draw(const Rect& clip)
    {
        this->editbox->draw(clip);
        if (this->label) {
            if (this->editbox->num_chars == 0) {
                this->label->draw(clip);
                this->editbox->draw_current_cursor();
            }
        }
        if (this->has_focus) {
            this->button.draw(clip);
            this->draw_border(clip, this->button.focus_color);
        }
        else {
            this->drawable.draw(RDPOpaqueRect(clip.intersect(this->button.rect),
                                              this->button.fg_color), clip);
        }
    }
    void draw_border(const Rect& clip, int color)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx() - 1, 1
        )), color), this->rect);
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + 1, 1, this->cy() - 2
        )), color), this->rect);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy(), 1, this->cy()
        )), color), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), color), this->rect);
    }
    virtual void focus() {
        this->editbox->focus();
        Widget2::focus();
    }
    virtual void blur() {
        this->editbox->blur();
        Widget2::blur();
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
    {
        Widget2 * w = this->editbox;
        if (x > this->editbox->lx()) {
            w = &this->button;
        }
        return w;
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (x > this->editbox->lx()) {
            this->button.rdp_input_mouse(device_flags, x, y, keymap);
            this->refresh(this->button.rect);
        }
        else {
            if ((device_flags == MOUSE_FLAG_BUTTON1)
                && this->button.state) {
                this->button.state = 0;
                this->refresh(this->button.rect);
            }
            this->editbox->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3,
                                    long int param4, Keymap2* keymap)
    {
        this->editbox->rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    virtual void notify(Widget2* widget, NotifyApi::notify_event_t event)
    {
        if (event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_SUBMIT);
        }
        if ((event == NOTIFY_TEXT_CHANGED) &&
            (widget == this->editbox) &&
            (this->label)) {
            if (this->editbox->num_chars == 1) {
                this->editbox->draw(this->rect);
            }
        }
    }
};

#endif
