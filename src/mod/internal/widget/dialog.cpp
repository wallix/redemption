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

#include "core/font.hpp"
#include "core/app_path.hpp"
#include "mod/internal/widget/dialog.hpp"
#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/theme.hpp"
#include "keyboard/keymap.hpp"
#include "gdi/graphic_api.hpp"

enum {
    WIDGET_MULTILINE_BORDER_X = 10,
    WIDGET_MULTILINE_BORDER_Y = 4
};

WidgetDialog::WidgetDialog(
    gdi::GraphicApi & drawable, CopyPaste & copy_paste, Rect const widget_rect,
    Events events,
    const char* caption, const char * text,
    WidgetButton * extra_button,
    Theme const & theme, Font const & font, const char * ok_text,
    const char * cancel_text, ChallengeOpt has_challenge
)
    : WidgetComposite(drawable, Focusable::Yes)
    , onctrl_shift(events.onctrl_shift)
    , title(drawable, caption,
            theme.global.fgcolor, theme.global.bgcolor, font, 5)
    , separator(drawable, theme.global.separator_color)
    , dialog(drawable, text,
             theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color,
             font, WIDGET_MULTILINE_BORDER_X, WIDGET_MULTILINE_BORDER_Y)
    , challenge(nullptr)
    , ok(drawable, ok_text ? ok_text : "Ok", events.onsubmit,
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, 2, font, 6, 2)
    , cancel(cancel_text
        ? std::make_unique<WidgetButton>(
            drawable, cancel_text, events.oncancel,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2)
        : std::unique_ptr<WidgetButton>())
    , img(drawable,
          theme.global.enable_theme ? theme.global.logo_path.c_str() :
          app_path(AppPath::LoginWabBlue),
          theme.global.bgcolor)
    , extra_button(extra_button)
    , oncancel(events.oncancel)
{
    this->set_bg_color(theme.global.bgcolor);
    this->add_widget(this->img);

    this->add_widget(this->title);
    this->add_widget(this->separator);
    this->add_widget(this->dialog);

    if (has_challenge) {
        if (CHALLENGE_ECHO == has_challenge) {
            this->challenge = std::make_unique<WidgetEdit>(
                this->drawable, copy_paste, nullptr, events.onsubmit,
                theme.edit.fgcolor, theme.edit.bgcolor,
                theme.edit.focus_color, font, -1u, 1, 1
            );
        }
        else {
            this->challenge = std::make_unique<WidgetPassword>(
                this->drawable, copy_paste, nullptr, events.onsubmit,
                theme.edit.fgcolor, theme.edit.bgcolor,
                theme.edit.focus_color, font, -1u, 1, 1
            );
        }
        this->add_widget(*this->challenge, HasFocus::Yes);
    }

    this->add_widget(this->ok, has_challenge ? HasFocus::No : HasFocus::Yes);

    if (this->cancel) {
        this->add_widget(*this->cancel);
    }

    if (has_challenge) {
        if (extra_button) {
            this->add_widget(*extra_button);
        }
    }

    this->move_size_widget(widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy);
}

WidgetDialog::~WidgetDialog() = default;

void WidgetDialog::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_xy(left, top);
    this->set_wh(width, height);

    int16_t y            = top;
    int16_t total_height = 0;

    Dimension dim = this->title.get_optimal_dim();
    this->title.set_wh(dim);
    this->title.set_xy(left + (width - this->title.cx()) / 2, y);
    y            += this->title.cy();
    total_height += this->title.cy();

    this->dialog.set_wh(width * 4 / 5 - WIDGET_MULTILINE_BORDER_X * 2, height / 2);
    {
        auto dim = this->dialog.get_optimal_dim();
        if (dim.h < this->dialog.cy()) {
            this->dialog.set_wh(dim.w, dim.h);
        }
    }

    const int total_width = std::max(this->dialog.cx(), this->title.cx());

    this->separator.set_wh(total_width, 2);
    this->separator.set_xy(left + (width - total_width) / 2, y + 3);

    y            += 10;
    total_height += 10;

    this->dialog.set_xy(this->separator.x(), y);

    y            += this->dialog.cy() + 10;
    total_height += this->dialog.cy() + 10;

    if (this->challenge) {
        dim = this->challenge->get_optimal_dim();
        this->challenge->set_wh(total_width - 20, dim.h);
        this->challenge->set_xy(this->separator.x() + 10, y);

        y            += this->challenge->cy() + 10;
        total_height += this->challenge->cy() + 10;
    }

    y += 5;

    dim = this->ok.get_optimal_dim();
    this->ok.set_wh(dim);

    if (this->cancel) {
        dim = this->cancel->get_optimal_dim();
        this->cancel->set_wh(dim);
        this->cancel->set_xy(this->dialog.x() + this->dialog.cx() - (this->cancel->cx() + 10), y);

        this->ok.set_xy(this->cancel->x() - (this->ok.cx() + 10), y);
    }
    else {
        this->ok.set_xy(this->dialog.x() + this->dialog.cx() - (this->ok.cx() + 10), y);
    }

    total_height += this->ok.cy();

    this->move_children_xy(0, (height - total_height) / 2);

    dim = this->img.get_optimal_dim();
    this->img.set_wh(dim);

    this->img.set_xy(left + (width - this->img.cx()) / 2,
                        top + (3 * (height - total_height) / 2 - this->img.cy()) / 2 + total_height);
    if (this->img.y() + this->img.cy() > top + height) {
        this->img.set_xy(this->img.x(), top);
    }

    if (this->challenge && this->extra_button) {
        extra_button->set_xy(left + 60, top + height - 60);
    }
}

void WidgetDialog::rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (keymap.last_kevent()) {
        case Keymap::KEvent::Esc:
            this->oncancel();
            break;
        case Keymap::KEvent::LeftArrow:
        case Keymap::KEvent::UpArrow:
        case Keymap::KEvent::PgUp:
            this->dialog.scroll_up();
            break;
        case Keymap::KEvent::RightArrow:
        case Keymap::KEvent::DownArrow:
        case Keymap::KEvent::PgDown:
            this->dialog.scroll_down();
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
            WidgetComposite::rdp_input_scancode(flags, scancode, event_time, keymap);
            break;
    }
    REDEMPTION_DIAGNOSTIC_POP()
}
