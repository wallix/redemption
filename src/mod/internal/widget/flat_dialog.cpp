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

#include "core/app_path.hpp"
#include "mod/internal/widget/flat_dialog.hpp"
#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/theme.hpp"
#include "keyboard/keymap2.hpp"
#include "gdi/graphic_api.hpp"

enum {
    WIDGET_MULTILINE_BORDER_X = 10,
    WIDGET_MULTILINE_BORDER_Y = 4
};

FlatDialog::FlatDialog(
    gdi::GraphicApi & drawable, int16_t left, int16_t top, int16_t width, int16_t height,
    Widget & parent, NotifyApi* notifier,
    const char* caption, const char * text,
    WidgetFlatButton * extra_button,
    Theme const & theme, Font const & font, const char * ok_text,
    const char * cancel_text, ChallengeOpt has_challenge
)
    : WidgetParent(drawable, parent, notifier)
    , title(drawable, *this, nullptr, caption, -9,
            theme.global.fgcolor, theme.global.bgcolor, font, 5)
    , separator(drawable, *this, this, -12,
                theme.global.separator_color)
    , dialog(drawable, *this, nullptr, text, -10,
                theme.global.fgcolor, theme.global.bgcolor, font,
                WIDGET_MULTILINE_BORDER_X, WIDGET_MULTILINE_BORDER_Y)
    , challenge(nullptr)
    , ok(drawable, *this, this, ok_text ? ok_text : "Ok", -12,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2)
    , cancel(cancel_text
        ? new WidgetFlatButton(
            drawable, *this, this, cancel_text, -11,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2)
        : nullptr)
    , img(drawable,
          theme.global.logo ? theme.global.logo_path.c_str() :
          app_path(AppPath::LoginWabBlue), *this, nullptr, -8)
    , extra_button(extra_button)
    , font(font)
    , dialog_string(text)
    , bg_color(theme.global.bgcolor)
{
    this->impl = &composite_array;

    this->add_widget(&this->img);

    this->add_widget(&this->title);
    this->add_widget(&this->separator);
    this->add_widget(&this->dialog);

    if (has_challenge) {
        if (CHALLENGE_ECHO == has_challenge) {
            this->challenge = new WidgetEdit(
                this->drawable, *this, this, nullptr, -13,
                theme.edit.fgcolor, theme.edit.bgcolor,
                theme.edit.focus_color, font, -1u, 1, 1
            );
        }
        else {
            this->challenge = new WidgetPassword(
                this->drawable, *this, this, nullptr, -13,
                theme.edit.fgcolor, theme.edit.bgcolor,
                theme.edit.focus_color, font, -1u, 1, 1
            );
        }
        this->add_widget(this->challenge);

        this->set_widget_focus(this->challenge, focus_reason_tabkey);
    }


    this->add_widget(&this->ok);

    if (this->cancel) {
        this->add_widget(this->cancel);
    }

    if (has_challenge) {
        if (extra_button) {
            this->add_widget(extra_button);
        }
    }

    this->move_size_widget(left, top, width, height);

    if (!has_challenge)
        this->set_widget_focus(&this->ok, focus_reason_tabkey);
}

FlatDialog::~FlatDialog()
{
    delete this->challenge;
    delete this->cancel;
    this->clear();
}

void FlatDialog::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
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

    dim = this->dialog.get_optimal_dim();
    std::string formatted_dialog_string;
    gdi::MultiLineTextMetricsEx mltm_ex(this->font, this->dialog_string.c_str(), WIDGET_MULTILINE_BORDER_Y,
        width * 4 / 5 - WIDGET_MULTILINE_BORDER_X * 2, formatted_dialog_string);
    this->dialog.set_wh(mltm_ex.width + WIDGET_MULTILINE_BORDER_X * 2, mltm_ex.height + WIDGET_MULTILINE_BORDER_Y * 2);
    this->dialog.set_text(formatted_dialog_string.c_str());

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

BGRColor FlatDialog::get_bg_color() const
{
    return this->bg_color;
}

void FlatDialog::notify(Widget* widget, NotifyApi::notify_event_t event)
{
    if ((event == NOTIFY_CANCEL) ||
        ((event == NOTIFY_SUBMIT) && (widget == this->cancel))) {
        this->send_notify(NOTIFY_CANCEL);
    }
    else if ((event == NOTIFY_SUBMIT) &&
                ((widget == &this->ok) || (widget == this->challenge))) {
        this->send_notify(NOTIFY_SUBMIT);
    }
    else {
        if (event == NOTIFY_PASTE) {
            if (this->notifier) {
                this->notifier->notify(widget, event);
            }
        }
        else {
            WidgetParent::notify(widget, event);
        }
    }
}

void FlatDialog::rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
{
    if (keymap->nb_kevent_available() > 0){
        switch (keymap->top_kevent()){
        case Keymap2::KEVENT_ESC:
            keymap->get_kevent();
            this->send_notify(NOTIFY_CANCEL);
            break;
        default:
            WidgetParent::rdp_input_scancode(param1, param2, param3, param4, keymap);
            break;
        }
    }
}
