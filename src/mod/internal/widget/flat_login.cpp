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
 *   Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong
 */

#include "mod/internal/widget/flat_login.hpp"
#include "core/app_path.hpp"
#include "utils/theme.hpp"
#include "keyboard/keymap2.hpp"
#include "gdi/graphic_api.hpp"

enum {
    WIDGET_MULTILINE_BORDER_X = 10,
    WIDGET_MULTILINE_BORDER_Y = 4
};

FlatLogin::FlatLogin(
    gdi::GraphicApi & drawable,
    int16_t left, int16_t top, uint16_t width, uint16_t height, Widget & parent,
    NotifyApi* notifier, const char* caption,
    const char * login, const char * password,
    const char * label_text_login,
    const char * label_text_password,
    const char * label_error_message,
    const char * label_login_message,
    WidgetFlatButton * extra_button,
    Font const & font, Translator tr, Theme const & theme
)
    : WidgetParent(drawable, parent, notifier)
    , error_message_label(drawable, *this, nullptr, label_error_message, -15,
                    theme.global.error_color, theme.global.bgcolor,
                    font)
    , login_label(drawable, *this, nullptr, label_text_login, -11,
                    theme.global.fgcolor, theme.global.bgcolor, font)
    , login_edit(drawable, *this, this,
                    login, -12, theme.edit.fgcolor, theme.edit.bgcolor,
                    theme.edit.focus_color, theme.global.bgcolor, font,
                    label_text_login, (width <= 640), -1u, 1, 1, false)
    , password_label(drawable, *this, nullptr, label_text_password, -13,
                        theme.global.fgcolor, theme.global.bgcolor,
                        font)
    , password_edit(drawable, *this, this,
                    password, -14, theme.edit.fgcolor,
                    theme.edit.bgcolor, theme.edit.focus_color, theme.global.bgcolor,
                    font, label_text_password, (width <= 640),
                    -1u, 1, 1, true)
    , login_message_label(drawable, *this, nullptr, label_login_message, -10,
             theme.global.fgcolor, theme.global.bgcolor, font,
             WIDGET_MULTILINE_BORDER_X, WIDGET_MULTILINE_BORDER_Y)
    , img(drawable,
            theme.global.logo ? theme.global.logo_path.c_str() :
            app_path(AppPath::LoginWabBlue), *this, nullptr, -10)
    , version_label(drawable, *this, nullptr, caption, -15,
                    theme.global.fgcolor, theme.global.bgcolor,
                    font)
    , helpicon(drawable, *this, nullptr, "?", -16,
                theme.global.fgcolor, theme.global.bgcolor,
                theme.global.focus_color, 2, font, 6, 2)
    , extra_button(extra_button)
    , font(font)
    , login_message(label_login_message)
    , tr(tr)
    , bg_color(theme.global.bgcolor)
{
    this->impl = &composite_array;

    this->add_widget(&this->img);
    this->add_widget(&this->helpicon);

    this->add_widget(&this->login_edit);
    this->add_widget(&this->password_edit);

    if (width > 640) {
        this->add_widget(&this->login_label);
        this->add_widget(&this->password_label);

        this->labels_added = true;
    }

    this->add_widget(&this->version_label);

    this->add_widget(&this->error_message_label);

    if (*label_login_message) {
        this->add_widget(&this->login_message_label);
    }

    if (extra_button) {
        this->add_widget(extra_button);
    }

    this->helpicon.tab_flag = IGNORE_TAB;
    this->helpicon.focus_flag = IGNORE_FOCUS;

    this->move_size_widget(left, top, width, height);
}

FlatLogin::~FlatLogin()
{
    this->clear();
}

void FlatLogin::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_xy(left, top);
    this->set_wh(width, height);

    Dimension dim;

    if (width > 640) {
        if (!this->labels_added) {
            this->add_widget(&this->login_label);
            this->add_widget(&this->password_label);

            this->labels_added = true;
        }

        dim = this->login_label.get_optimal_dim();
        this->login_label.set_wh(dim);

        dim = this->password_label.get_optimal_dim();
        this->password_label.set_wh(dim);
    }
    else {
        if (this->labels_added) {
            this->remove_widget(&this->login_label);
            this->remove_widget(&this->password_label);

            this->labels_added = false;
        }

        this->login_label.set_wh(0, 0);
        this->password_label.set_wh(0, 0);
    }

    this->login_edit.use_title(width < 640);
    dim = this->login_edit.get_optimal_dim();
    this->login_edit.set_wh((width >= 420) ? 400 : width - 20, dim.h);

    const uint16_t offset_y = this->login_edit.cy();

    this->password_edit.use_title(width < 640);
    dim = this->password_edit.get_optimal_dim();
    this->password_edit.set_wh((width >= 420) ? 400 : width - 20, dim.h);

    const int cbloc_w = std::max(this->login_label.cx()    + 10 + this->login_edit.cx(),
                                 this->password_label.cx() + 10 + this->password_edit.cx());


    std::string formatted_login_message;
    gdi::MultiLineTextMetricsEx mltm_ex(this->font, this->login_message.c_str(), WIDGET_MULTILINE_BORDER_Y,
        cbloc_w - WIDGET_MULTILINE_BORDER_X * 2, formatted_login_message);
    this->login_message_label.set_wh(mltm_ex.width + WIDGET_MULTILINE_BORDER_X * 2, mltm_ex.height + WIDGET_MULTILINE_BORDER_Y * 2);
    this->login_message_label.set_text(formatted_login_message.c_str());

    dim = this->error_message_label.get_optimal_dim();
    this->error_message_label.set_wh(dim);


    const int cbloc_h = this->login_message_label.cy() + 60 +
        this->login_label.cy() + offset_y + this->login_label.cy() + offset_y + this->password_label.cy() +
        60;

    const int cbloc_x = (width  - cbloc_w) / 2;
    const int cbloc_y = (height - cbloc_h) / 2;

    this->login_message_label.set_xy(left + (width - this->login_message_label.cx()) / 2, cbloc_y);



    this->error_message_label.set_xy(left + cbloc_x, top + cbloc_y + this->login_message_label.cy() + 60);

    this->login_label.set_xy(left + cbloc_x,
                             this->error_message_label.y() + this->error_message_label.cy() + offset_y + 4);

    this->password_label.set_xy(left + cbloc_x,
                                this->login_label.y() + this->error_message_label.cy() + offset_y + 4);

    const int labels_w = std::max(this->password_label.cx(), this->login_label.cx());

    this->login_edit.set_xy(left + cbloc_x + labels_w + 10, this->login_label.y() - this->login_edit.get_border_height() - 3);
    this->password_edit.set_xy(left + cbloc_x + labels_w + 10, this->password_label.y() - this->password_edit.get_border_height() - 3);

    this->error_message_label.set_xy(this->login_edit.x(),
                                        this->error_message_label.y());

    dim = this->error_message_label.get_optimal_dim();
    this->error_message_label.set_wh(this->login_edit.cx(), dim.h);


    dim = this->version_label.get_optimal_dim();
    this->version_label.set_wh(dim);

    dim = this->img.get_optimal_dim();
    this->img.set_wh(dim);

    // Bottom bloc positioning
    // Logo and Version
    const int bottom_height = (height - cbloc_h) / 2;
    const int bbloc_h       = this->img.cy() + 10 + this->version_label.cy();
    const int y_bbloc       =   ((bbloc_h + 10) > (bottom_height / 2))
                                ? (height - (bbloc_h + 10))
                                : (height / 2 + cbloc_h / 2 + bottom_height / 2);

    this->img.set_xy(left + (width - this->img.cx()) / 2, top + y_bbloc);
    if (this->img.y() + this->img.cy() > top + height) {
        this->img.set_xy(this->img.x(), top);
    }
    this->version_label.set_xy(left + (width - this->version_label.cx()) / 2,
                                top + y_bbloc + this->img.cy() + 10);

    dim = this->helpicon.get_optimal_dim();
    this->helpicon.set_wh(dim);
    this->helpicon.set_xy(left + width - 60, top + height - 60);

    if (this->extra_button) {
        this->extra_button->set_xy(left + 60, top + height - 60);
    }
}

BGRColor FlatLogin::get_bg_color() const
{
    return this->bg_color;
}

void FlatLogin::notify(Widget* widget, NotifyApi::notify_event_t event)
{
    if ((widget == &this->login_edit
            || widget == &this->password_edit)
            && event == NOTIFY_SUBMIT) {
        this->send_notify(NOTIFY_SUBMIT);
    }
    if (NOTIFY_COPY == event || NOTIFY_CUT == event || NOTIFY_PASTE == event) {
        if (this->notifier) {
            this->notifier->notify(widget, event);
        }
    }
}

void FlatLogin::rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
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

void FlatLogin::rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
{
    if (device_flags == MOUSE_FLAG_MOVE) {
        Widget * wid = this->widget_at_pos(x, y);
        if (wid == &this->helpicon) {
            this->show_tooltip(wid, this->tr(trkeys::help_message), x, y, this->get_rect());
        }
    }

    WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
}
