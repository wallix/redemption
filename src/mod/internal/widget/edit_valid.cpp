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

#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/button.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

WidgetEditValid::WidgetEditValid(
    gdi::GraphicApi & drawable, CopyPaste & copy_paste,
    const char * text, WidgetEventNotifier onsubmit,
    Color fgcolor, Color bgcolor, Color focus_color, Color border_none_color,
    Font const & font, const char * title, bool use_title, std::size_t edit_position,
    // TODO re-enable
    int /*xtext*/, int /*ytext*/, bool pass
)
    : Widget(drawable, Focusable::Yes)
    , button(drawable, "\xe2\x9e\x9c", onsubmit,
             bgcolor, focus_color, focus_color, 1, font, 6, 2)
    , editbox(pass
        ? new WidgetPassword(drawable, copy_paste, text, onsubmit, fgcolor, bgcolor,
                             bgcolor, font, edit_position, 1, 2)
        : new WidgetEdit(drawable, copy_paste, text, onsubmit, fgcolor, bgcolor,
                         bgcolor, font, edit_position, 1, 2))
    , label(title
        ? new WidgetLabel(drawable, title, MEDIUM_GREY, bgcolor, font, 1, 2)
        : nullptr)
    , use_label_(use_title)
    , border_none_color(border_none_color)
{
}

void WidgetEditValid::init_focus()
{
    this->editbox->init_focus();
}

Dimension WidgetEditValid::get_optimal_dim() const
{
    Dimension dim = this->button.get_optimal_dim();

    dim.h += 2 /* border */;

    return dim;
}

WidgetEditValid::~WidgetEditValid()
{
    if (this->editbox) {
        delete this->editbox;
        this->editbox = nullptr;
    }
    if (this->label) {
        delete this->label;
        this->label = nullptr;
    }
}

void WidgetEditValid::use_title(bool use)
{
    this->use_label_ = use;
}

void WidgetEditValid::set_text(const char * text/*, int position = 0*/)
{
    this->editbox->set_text(text);
}

const char * WidgetEditValid::get_text() const
{
    return this->editbox->get_text();
}

void WidgetEditValid::set_xy(int16_t x, int16_t y)
{
    Widget::set_xy(x, y);
    this->editbox->set_xy(x + 1, y + 1);
    this->button.set_xy(this->editbox->eright(), y + 1);

    if (this->label) {
        this->label->set_xy(x + 2, y + 2);
    }
}

void WidgetEditValid::set_wh(uint16_t w, uint16_t h)
{
    Widget::set_wh(w, h);

    Dimension dim = this->button.get_optimal_dim();
    this->button.set_wh(dim.w, h - 2 /* 2 x border */);

    this->editbox->set_wh(w - this->button.cx() - 2,
                            h - 2 /* 2 x border */);

    this->button.set_xy(this->editbox->eright(), this->button.y());

    if (this->label) {
        this->label->set_wh(this->editbox->cx() - 4,
                            this->editbox->cy() - 4 /* 2 x (border + 1) */);
    }
}

void WidgetEditValid::rdp_input_invalidate(Rect clip)
{
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
                RDPOpaqueRect(rect_intersect.intersect(this->button.get_rect()), this->button.fg_color),
                rect_intersect, gdi::ColorCtx::depth24()
            );
            this->draw_border(rect_intersect, this->border_none_color);
        }

        this->drawable.end_update();
    }
}

void WidgetEditValid::draw_border(const Rect clip, Color color)
{
    //top
    this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
        this->x(), this->y(), this->cx() - 1, 1
    )), color), clip, gdi::ColorCtx::depth24());
    //left
    this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
        this->x(), this->y() + 1, 1, this->cy() - /*2*/1
    )), color), clip, gdi::ColorCtx::depth24());
    //right
    this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
        this->x() + this->cx() - 1, this->y(), 1, this->cy()
    )), color), clip, gdi::ColorCtx::depth24());
    //bottom
    this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
        this->x(), this->y() + this->cy() - 1, this->cx(), 1
    )), color), clip, gdi::ColorCtx::depth24());
}

void WidgetEditValid::focus(int reason)
{
    this->editbox->focus(reason);
    Widget::focus(reason);
}

void WidgetEditValid::blur()
{
    this->editbox->blur();
    Widget::blur();
}

Widget * WidgetEditValid::widget_at_pos(int16_t x, int16_t y)
{
    // TODO y is not used: suspicious
    (void)y;
    Widget * w = this->editbox;
    if (x > this->editbox->eright()) {
        w = &this->button;
    }
    return w;
}

void WidgetEditValid::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    if (x > this->editbox->eright()) {
        this->button.rdp_input_mouse(device_flags, x, y);
        this->rdp_input_invalidate(this->button.get_rect());
    }
    else {
        if ((device_flags == MOUSE_FLAG_BUTTON1)
          && this->button.state == WidgetButton::State::Pressed
        ) {
            this->button.state = WidgetButton::State::Normal;
            this->rdp_input_invalidate(this->button.get_rect());
        }
        this->editbox->rdp_input_mouse(device_flags, x, y);
    }
}

void WidgetEditValid::rdp_input_scancode(
    KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    bool has_char1 = (0 == this->editbox->num_chars);
    this->editbox->rdp_input_scancode(flags, scancode, event_time, keymap);

    if (this->label && this->use_label_) {
        bool has_char2 = (0 == this->editbox->num_chars);
        if (has_char1 != has_char2 && has_char1) {
            this->editbox->rdp_input_invalidate(this->editbox->get_rect());
        }
    }
}

void WidgetEditValid::rdp_input_unicode(KbdFlags flag, uint16_t unicode)
{
    this->editbox->rdp_input_unicode(flag, unicode);
}
