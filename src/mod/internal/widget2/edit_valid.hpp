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

    int border_none_color;

    WidgetEditValid(gdi::GraphicApi & drawable,
                    Widget2 & parent, NotifyApi* notifier, const char * text,
                    int group_id, int fgcolor, int bgcolor,
                    int focus_color, int border_none_color, Font const & font,
                    const char * title, bool use_title, std::size_t edit_position = -1,
                    // TODO re-enable
                    int /*xtext*/ = 0, int /*ytext*/ = 0, bool pass = false)
        : Widget2(drawable, parent, notifier, group_id)
        , button(drawable, *this, this, "\xe2\x9e\x9c",
                 group_id, bgcolor, focus_color, focus_color, 1, font, 6, 2)
        , editbox(pass ? new WidgetPassword(drawable, *this,
                                            this, text, group_id, fgcolor, bgcolor,
                                            focus_color, font, edit_position, 1, 2)
                  : new WidgetEdit(drawable, *this, this,
                                   text, group_id, fgcolor, bgcolor, focus_color, font,
                                   edit_position, 1, 2))
        , label(title ? new WidgetLabel(drawable, *this, nullptr, title,
                                        group_id, MEDIUM_GREY, bgcolor, font, 1, 2)
                : nullptr)
        , use_label_(use_title)
        , border_none_color(border_none_color)
    {
        this->editbox->draw_border_focus = false;
    }

    Dimension get_optimal_dim() override {
        Dimension dim = this->button.get_optimal_dim();

        dim.h += 2 /* border */;

        return dim;
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

    void set_xy(int16_t x, int16_t y) override {
        Widget2::set_xy(x, y);
        this->editbox->set_xy(x + 1, y + 1);
        this->button.set_xy(this->editbox->right(), y + 1);

        if (this->label) {
            this->label->set_xy(x + 2, y + 2);
        }
    }

    void set_wh(uint16_t w, uint16_t h) override {
        Widget2::set_wh(w, h);

        Dimension dim = this->button.get_optimal_dim();
        this->button.set_wh(dim.w, h - 2 /* 2 x border */);

        this->editbox->set_wh(w - this->button.cx() - 2,
                              h - 2 /* 2 x border */);

        this->button.set_xy(this->editbox->right(), this->button.y());

        if (this->label) {
            this->label->set_wh(this->editbox->cx() - 4,
                                this->editbox->cy() - 4 /* 2 x (border + 1) */);
        }
    }

    using Widget2::set_wh;

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->editbox->rdp_input_invalidate(rect_intersect);
            if (this->label && this->use_label_) {
                if (this->editbox->num_chars == 0) {
                    this->label->rdp_input_invalidate(rect_intersect);
                    this->editbox->draw_current_cursor();
                }
            }
            if (this->has_focus) {
                this->button.rdp_input_invalidate(rect_intersect);
                this->draw_border(rect_intersect, this->button.focus_color);
            }
            else {
                this->drawable.draw(
                    RDPOpaqueRect(rect_intersect.intersect(this->button.get_rect()), RDPColor(this->button.fg_color)),
                    rect_intersect, gdi::ColorCtx::depth24()
                );
                this->draw_border(rect_intersect, this->border_none_color);
            }

            this->drawable.end_update();
        }
    }

    void draw_border(const Rect clip, int color)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y(), this->cx() - 1, 1
        )), RDPColor(color)), clip, gdi::ColorCtx::depth24());
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + 1, 1, this->cy() - /*2*/1
        )), RDPColor(color)), clip, gdi::ColorCtx::depth24());
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x() + this->cx() - 1, this->y(), 1, this->cy()
        )), RDPColor(color)), clip, gdi::ColorCtx::depth24());
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + this->cy() - 1, this->cx(), 1
        )), RDPColor(color)), clip, gdi::ColorCtx::depth24());
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
            this->rdp_input_invalidate(this->button.get_rect());
        }
        else {
            if ((device_flags == MOUSE_FLAG_BUTTON1)
                && this->button.state) {
                this->button.state = 0;
                this->rdp_input_invalidate(this->button.get_rect());
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
                this->editbox->rdp_input_invalidate(this->get_rect());
            }
        }
        if (NOTIFY_COPY == event || NOTIFY_CUT == event || NOTIFY_PASTE == event) {
            if (this->notifier) {
                this->notifier->notify(widget, event);
            }
        }
    }

    unsigned get_border_height() const {
        return 1;
    }
};
