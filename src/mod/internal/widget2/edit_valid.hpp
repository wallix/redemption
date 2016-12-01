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

#pragma once

#include "widget.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "password.hpp"
#include "flat_button.hpp"
#include "keyboard/keymap2.hpp"
#include "utils/colors.hpp"
#include "gdi/graphic_api.hpp"

class WidgetEditValid : public Widget2 {
public:
    WidgetFlatButton button;
    WidgetEdit * editbox;
    WidgetLabel * label;

    bool use_label_;

    WidgetEditValid(gdi::GraphicApi & drawable, int16_t x, int16_t y, uint16_t cx,
                    Widget2 & parent, NotifyApi* notifier, const char * text,
                    int group_id, int fgcolor, int bgcolor,
                    int focus_color, Font const & font,
                    const char * title, bool use_title, std::size_t edit_position = -1,
                    // TODO re-enable
                    int /*xtext*/ = 0, int /*ytext*/ = 0, bool pass = false)
        : Widget2(drawable, Rect(0, 0, cx, 1), parent, notifier, group_id)
        , button(drawable, 0, 0, *this, this, "\xe2\x9e\x9c", true,
                 group_id, bgcolor, focus_color, focus_color, font, 6, 2)
        , editbox(pass ? new WidgetPassword(drawable, 0, 0, cx - this->button.cx(), *this,
                                            this, text, group_id, fgcolor, bgcolor,
                                            focus_color, font, edit_position, 1, 2)
                  : new WidgetEdit(drawable, 0, 0, cx - this->button.cx(), *this, this,
                                   text, group_id, fgcolor, bgcolor, focus_color, font,
                                   edit_position, 1, 2))
        , label(title ? new WidgetLabel(drawable, 0, 0, *this, nullptr, title, true,
                                        group_id, MEDIUM_GREY, bgcolor, font, 1, 2)
                : nullptr)
        , use_label_(use_title)
    {
        this->button.set_x(this->editbox->right() - 1);
        this->editbox->set_cy(this->button.cy());
        this->set_cy(this->editbox->cy());
        this->set_cx(this->button.right() - this->x());
        this->set_xy(x, y);
        this->editbox->draw_border_focus = false;

        if (this->label) {
            this->label->set_cx(this->editbox->cx() - 1);
            this->label->set_cy(this->editbox->cy() - 1);
            this->label->set_x(this->label->x() + 1);
            this->label->set_y(this->label->y() + 1);
        }
    }

    ~WidgetEditValid() override {
        if (this->editbox) {
            delete this->editbox;
            this->editbox = nullptr;
        }
        if (this->label) {
            delete this->label;
            this->label = nullptr;
        }
    }

    void use_title(bool use) {
        this->use_label_ = use;
    }

    virtual void set_text(const char * text/*, int position = 0*/)
    {
        this->editbox->set_text(text);
    }

    const char * get_text() const
    {
        return this->editbox->get_text();
    }

    void set_x(int16_t x) override {
        Widget2::set_x(x);
        this->editbox->set_x(x);
        this->button.set_x(this->editbox->right() - 1);

        if (this->label) {
            this->label->set_x(x + 1);
        }
    }

    void set_y(int16_t y) override {
        Widget2::set_y(y);
        this->editbox->set_y(y);
        this->button.set_y(y);

        if (this->label) {
            this->label->set_y(y + 1);
        }
    }

    void draw(const Rect& clip) override {
        this->editbox->draw(clip);
        if (this->label && this->use_label_) {
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
            this->drawable.draw(RDPOpaqueRect(clip.intersect(this->button.get_rect()),
                                              this->button.fg_color), clip);
        }
    }
    void draw_border(const Rect& clip, int color)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y(), this->cx() - 1, 1
        )), color), this->get_rect());
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + 1, 1, this->cy() - 2
        )), color), this->get_rect());
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x() + this->cx() - 1, this->y(), 1, this->cy()
        )), color), this->get_rect());
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + this->cy() - 1, this->cx(), 1
        )), color), this->get_rect());
    }
    void focus(int reason) override {
        this->editbox->focus(reason);
        Widget2::focus(reason);
    }
    void blur() override {
        this->editbox->blur();
        Widget2::blur();
    }

    Widget2 * widget_at_pos(int16_t x, int16_t y) override {
        // TODO y is not used: suspicious
        (void)y;
        Widget2 * w = this->editbox;
        if (x > this->editbox->right()) {
            w = &this->button;
        }
        return w;
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        if (x > this->editbox->right()) {
            this->button.rdp_input_mouse(device_flags, x, y, keymap);
            this->refresh(this->button.get_rect());
        }
        else {
            if ((device_flags == MOUSE_FLAG_BUTTON1)
                && this->button.state) {
                this->button.state = 0;
                this->refresh(this->button.get_rect());
            }
            this->editbox->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                                    long int param4, Keymap2* keymap) override {
        this->editbox->rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
        if (event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_SUBMIT);
        }
        if ((event == NOTIFY_TEXT_CHANGED) &&
            (widget == this->editbox) &&
            this->label && this->use_label_) {
            if (this->editbox->num_chars == 1) {
                this->editbox->draw(this->get_rect());
            }
        }
        if (NOTIFY_COPY == event || NOTIFY_CUT == event || NOTIFY_PASTE == event) {
            if (this->notifier) {
                this->notifier->notify(widget, event);
            }
        }
    }
};

