/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
 *              Jennifer Inthavong
 */

#include "mod/internal/widget/flat_interactive_target.hpp"
#include "keyboard/keymap2.hpp"
#include "utils/theme.hpp"


FlatInteractiveTarget::FlatInteractiveTarget(
    gdi::GraphicApi & drawable, int16_t left, int16_t top, uint16_t width, uint16_t height,
    Widget & parent, NotifyApi* notifier,
    bool ask_device, bool ask_login, bool ask_password,
    Theme const & theme, const char* caption,
    const char * text_device,
    const char * device_str,
    const char * text_login,
    const char * login_str,
    const char * text_password,
    Font const & font,
    WidgetFlatButton * extra_button
)
    : WidgetParent(drawable, parent, notifier)
    , caption_label(drawable, *this, nullptr, caption, -13,
                    theme.global.fgcolor, theme.global.bgcolor, font)
    , separator(drawable, *this, this, -12,
                theme.global.separator_color)
    , device_label(drawable, *this, nullptr, text_device, -13,
                    theme.global.fgcolor, theme.global.bgcolor, font)
    , device(drawable, *this, nullptr, device_str, -13,
                theme.global.fgcolor, theme.global.bgcolor, font)
    , device_edit(drawable, *this, this, nullptr, -14,
                    theme.edit.fgcolor, theme.edit.bgcolor,
                    theme.edit.focus_color, theme.global.bgcolor, font, nullptr, false, -1u, 1, 1)
    , login_label(drawable, *this, nullptr, text_login, -13,
                    theme.global.fgcolor, theme.global.bgcolor, font)
    , login(drawable, *this, nullptr, login_str, -13,
            theme.global.fgcolor, theme.global.bgcolor, font)
    , login_edit(drawable, *this, this, nullptr, -14,
                    theme.edit.fgcolor, theme.edit.bgcolor,
                    theme.edit.focus_color, theme.global.bgcolor, font, nullptr, false, -1u, 1, 1)
    , password_label(drawable, *this, nullptr, text_password, -13,
                        theme.global.fgcolor, theme.global.bgcolor, font)
    , password_edit(drawable, *this, this, nullptr, -14,
                    theme.edit.fgcolor, theme.edit.bgcolor,
                    theme.edit.focus_color, theme.global.bgcolor, font, nullptr, false, -1u, 1, 1, true)
    , extra_button(extra_button)
    , last_interactive((ask_login || ask_password)?&this->password_edit:&this->device_edit)
    , fgcolor(theme.global.fgcolor)
    , bgcolor(theme.global.bgcolor)
    , ask_device(ask_device)
    , ask_login(ask_login)
    , ask_password(ask_password)
{
    this->impl = &composite_array;

    ask_password = (ask_login || ask_password);

    this->ask_password = ask_password;

    Widget * device_show = &this->device;
    if (ask_device) {
        device_show = &this->device_edit;
    }
    Widget * login_show = &this->login;
    if (ask_login) {
        login_show = &this->login_edit;
    }
    Widget * password_show = nullptr;
    if (ask_password) {
        password_show = &this->password_edit;
    }

    this->add_widget(&this->caption_label);
    this->add_widget(&this->separator);

    this->add_widget(&this->device_label);
    this->add_widget(device_show);
    if (ask_device) {
        this->add_widget(&this->device);
        if ((0 == strncmp(device_str, "Error:", 6)) ||
            (0 == strncmp(device_str, "Erreur:", 7))) {
            this->device.fg_color = theme.global.error_color;
        }
    }
    this->add_widget(&this->login_label);
    this->add_widget(login_show);
    if (password_show) {
        this->add_widget(&this->password_label);
        this->add_widget(password_show);
    }

    if (extra_button) {
        this->add_widget(extra_button);
    }

    this->move_size_widget(left, top, width, height);
}

FlatInteractiveTarget::~FlatInteractiveTarget()
{
        this->clear();
}

void FlatInteractiveTarget::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_xy(left, top);
    this->set_wh(width, height);

    Dimension dim = this->caption_label.get_optimal_dim();
    this->caption_label.set_wh(dim);

    dim = this->device.get_optimal_dim();
    this->device.set_wh(dim);
    Widget * device_show = &this->device;
    if (this->ask_device) {
        dim = this->device_edit.get_optimal_dim();
        this->device_edit.set_wh(400, dim.h);
        device_show = &this->device_edit;
    }
    dim = this->login.get_optimal_dim();
    this->login.set_wh(dim);
    Widget * login_show = &this->login;
    if (this->ask_login) {
        dim = this->login_edit.get_optimal_dim();
        this->login_edit.set_wh(400, dim.h);
        login_show = &this->login_edit;
    }
    if (this->ask_password) {
        dim = this->password_edit.get_optimal_dim();
        this->password_edit.set_wh(400, dim.h);
    }

    dim = this->device_label.get_optimal_dim();
    this->device_label.set_wh(dim);

    dim = this->login_label.get_optimal_dim();
    this->login_label.set_wh(dim);

    // Center bloc positionning
    // Device, Login and Password boxes
    int margin_w = std::max<int>(this->device_label.cx(),
                                    this->login_label.cx());
    if (this->ask_password) {
        dim = this->password_label.get_optimal_dim();
        this->password_label.set_wh(dim);

        margin_w = std::max<int>(margin_w,
                                    this->password_label.cx());
    }

    int cbloc_w = std::max<int>(this->caption_label.cx(),
                                margin_w + device_show->cx() + 20);
    if (this->ask_device) {
        cbloc_w = std::max<int>(cbloc_w,
                                margin_w + this->device.cx() + 20);
    }
    cbloc_w = std::max<int>(cbloc_w,
                            margin_w + login_show->cx() + 20);
    if (this->ask_password) {
        cbloc_w = std::max<int>(cbloc_w,
                                margin_w + this->password_edit.cx() + 20);
    }

    const int cbloc_h =
        this->caption_label.cy() + 20 + 30 +
        this->device_label.cy() + 30 +
        (this->ask_device ? this->device.cy() + 30 : 0) +
        this->login_label.cy() + 30 +
        (this->ask_password ? this->password_edit.cy() + 20 : 0);

    const int x_cbloc = (width  - cbloc_w) / 2;
    const int y_cbloc = (height - cbloc_h) / 3;

    int y = y_cbloc;

    this->caption_label.set_xy(left + (width - this->caption_label.cx()) / 2, top + y);

    y = this->caption_label.bottom() + 20;

    this->separator.set_xy(left + x_cbloc, y);
    this->separator.set_wh(cbloc_w, 2);

    y = this->separator.bottom() + 20;

    this->device_label.set_xy(left + x_cbloc, y);
    device_show->set_xy(left + x_cbloc + margin_w + 20, y - (this->ask_device ? this->device_edit.get_border_height() : 0));

    y = device_show->bottom() + 20;

    if (this->ask_device) {
        this->device.set_xy(left + x_cbloc + margin_w + 20, y - 10);

        y = this->device.bottom() + 20;
    }

    this->login_label.set_xy(left + x_cbloc, y);
    login_show->set_xy(left + x_cbloc + margin_w + 20, y - (this->ask_login ? this->login_edit.get_border_height() : 0));

    y = login_show->bottom() + 20;

    if (this->ask_password) {
        this->password_label.set_xy(left + x_cbloc, y);
        this->password_edit.set_xy(left + x_cbloc + margin_w + 20, y - this->password_edit.get_border_height());
    }

    this->device_label.set_xy(this->device_label.x(),
        this->device_label.y() + (device_show->cy() - this->device_label.cy()) / 2);
    this->login_label.set_xy(this->login_label.x(),
        this->login_label.y() + (login_show->cy() - this->login_label.cy()) / 2);
    if (this->ask_password) {
        this->password_label.set_xy(this->password_label.x(),
            this->password_label.y() + (this->password_edit.cy() - this->password_label.cy()) / 2);
    }

    if (this->extra_button) {
        this->extra_button->set_xy(left + 60, top + height - 60);
    }
}

BGRColor FlatInteractiveTarget::get_bg_color() const
{
    return this->bgcolor;
}

void FlatInteractiveTarget::notify(Widget* widget, NotifyApi::notify_event_t event)
{
    if ((widget == this->last_interactive)
            && event == NOTIFY_SUBMIT) {
        this->send_notify(NOTIFY_SUBMIT);
    }
    else if (event == NOTIFY_SUBMIT) {
        this->next_focus();
    }
    if (NOTIFY_COPY == event || NOTIFY_CUT == event || NOTIFY_PASTE == event) {
        if (this->notifier) {
            this->notifier->notify(widget, event);
        }
    }
}

void FlatInteractiveTarget::rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
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
