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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#include "mod/internal/widget/wait.hpp"
#include "keyboard/keymap.hpp"

#include "utils/translation.hpp"
#include "utils/theme.hpp"


enum {
    WIDGET_MULTILINE_BORDER_X = 10,
    WIDGET_MULTILINE_BORDER_Y = 4
};

constexpr unsigned HIDE_BACK_TO_SELECTOR = 0x10000;

WidgetWait::WidgetWait(
    gdi::GraphicApi & drawable, CopyPaste & copy_paste, Rect const widget_rect,
    Events events, const char* caption, const char * text,
    WidgetButton * extra_button,
    Font const & font, Theme const & theme, Language lang,
    bool showform, unsigned flags, std::chrono::minutes duration_max
)
    : WidgetParent(drawable, Focusable::Yes)
    , onaccept(events.onaccept)
    , onrefused(events.onrefused)
    , onctrl_shift(events.onctrl_shift)
    , groupbox(drawable, caption,
               theme.global.fgcolor, theme.global.bgcolor, font)
    , dialog(drawable, text,
             theme.global.fgcolor, theme.global.bgcolor, font,
             WIDGET_MULTILINE_BORDER_X, WIDGET_MULTILINE_BORDER_Y)
    , form(drawable, copy_paste, {events.onconfirm, events.onrefused},
           font, theme, lang, flags & ~HIDE_BACK_TO_SELECTOR, duration_max)
    , goselector(drawable, TR(trkeys::back_selector, lang), events.onaccept,
                 theme.global.fgcolor, theme.global.bgcolor,
                 theme.global.focus_color, 2, font, 6, 2)
    , exit(drawable, TR(trkeys::exit, lang), events.onrefused,
           theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, 2, font,
           6, 2)
    , extra_button(extra_button)
    , bg_color(theme.global.bgcolor)
    , hasform(showform)
    , hide_back_to_selector(flags & HIDE_BACK_TO_SELECTOR)
{
    this->impl = &composite_array;

    this->groupbox.add_widget(this->dialog);

    if (showform) {
        this->groupbox.add_widget(this->form, HasFocus::Yes);
    }

    if (!this->hide_back_to_selector) {
        this->groupbox.add_widget(this->goselector, showform ? HasFocus::No : HasFocus::Yes);
    }
    this->groupbox.add_widget(this->exit);

    this->add_widget(this->groupbox);

    if (extra_button) {
        this->add_widget(*extra_button);
    }

    this->move_size_widget(widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy);
}

void WidgetWait::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_xy(left, top);
    this->set_wh(width, height);

    this->groupbox.set_xy(left, top);
    this->groupbox.set_wh(width, height);

    if (this->hasform) {
        this->form.set_wh(width - 80, 150);
        this->form.set_xy(left, top);
    }

    int y = 20;

    Dimension dim = this->dialog.get_optimal_dim();
    this->dialog.set_wh(dim);
    this->dialog.set_xy(left + 30, top + y + 10);

    y = this->dialog.y() + this->dialog.cy() + 20;

    if (this->hasform) {
        this->form.move_size_widget(left + 40, y, this->form.cx(), this->form.cy());

        y = this->form.ebottom() + 10;
    }

    dim = this->exit.get_optimal_dim();
    this->exit.set_wh(dim);
    this->exit.set_xy(left + width - 40 - this->exit.cx(), y);

    if (!this->hide_back_to_selector) {
        dim = this->goselector.get_optimal_dim();
        this->goselector.set_wh(dim);
        this->goselector.set_xy(this->exit.x() - (this->goselector.cx() + 10), y);
    }

    y += this->exit.cy() + 20;

    this->groupbox.set_wh(this->groupbox.cx(), y - top);
    this->groupbox.move_xy(0, (height - (y - top)) / 2);

    if (this->extra_button) {
        this->extra_button->set_xy(left + 60, top + height - 60);
    }
}

void WidgetWait::rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (keymap.last_kevent()) {
        case Keymap::KEvent::Esc:
            if (this->hide_back_to_selector) {
                this->onrefused();
            }
            else {
                this->onaccept();
            }
            break;

        case Keymap::KEvent::Ctrl:
        case Keymap::KEvent::Shift:
            if (this->extra_button
                && keymap.is_shift_pressed()
                && keymap.is_ctrl_pressed())
            {
                this->onctrl_shift();
            }
            break;

        default:
            WidgetParent::rdp_input_scancode(flags, scancode, event_time, keymap);
            break;
    }
    REDEMPTION_DIAGNOSTIC_POP()
}
