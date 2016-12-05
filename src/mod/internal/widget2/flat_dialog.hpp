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

#pragma once

#include "core/defines.hpp"
#include "composite.hpp"
#include "flat_button.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "widget2_rect.hpp"
#include "edit.hpp"
#include "password.hpp"
#include "utils/theme.hpp"
#include "flat_button.hpp"
#include "gdi/graphic_api.hpp"


enum ChallengeOpt {
    NO_CHALLENGE = 0x00,
    CHALLENGE_ECHO = 0x01,
    CHALLENGE_HIDE = 0x02
};

class FlatDialog : public WidgetParent
{
public:
    int bg_color;

    WidgetImage img;
    WidgetLabel title;
    WidgetMultiLine dialog;
    WidgetEdit * challenge;
    WidgetFlatButton ok;
    WidgetFlatButton * cancel;
    WidgetRect separator;

    CompositeArray composite_array;

    Font const & font;

    WidgetFlatButton * extra_button;

    FlatDialog(gdi::GraphicApi & drawable, int16_t left, int16_t top, int16_t width, int16_t height,
               Widget2 & parent, NotifyApi* notifier,
               const char* caption, const char * text,
               WidgetFlatButton * extra_button,
               Theme const & theme, Font const & font, const char * ok_text = "Ok",
               const char * cancel_text = "Cancel",
               ChallengeOpt has_challenge = NO_CHALLENGE)
        : WidgetParent(drawable, Rect(left, top, width, height), parent, notifier)
        , bg_color(theme.global.bgcolor)
        , img(drawable, 0, 0,
              theme.global.logo ? theme.global.logo_path :
              SHARE_PATH "/" LOGIN_WAB_BLUE, *this, nullptr, -8)
        , title(drawable, 0, 0, *this, nullptr, caption, true, -9,
                theme.global.fgcolor, theme.global.bgcolor, font, 5)
        , dialog(drawable, 0, 0, *this, nullptr, text, true, -10,
                 theme.global.fgcolor, theme.global.bgcolor, font, 10, 2)
        , challenge(nullptr)
        , ok(drawable, 0, 0, *this, this, ok_text ? ok_text : "Ok", true, -12,
             theme.global.fgcolor, theme.global.bgcolor,
             theme.global.focus_color, font, 6, 2)
        , cancel(cancel_text ? new WidgetFlatButton(drawable, 0, 0, *this, this,
                                                    cancel_text, true, -11,
                                                    theme.global.fgcolor,
                                                    theme.global.bgcolor,
                                                    theme.global.focus_color, font,
                                                    6, 2) : nullptr)
        , separator(drawable, Rect(0, 0, width, 2), *this, this, -12,
                    theme.global.separator_color)
        , font(font)
        , extra_button(extra_button)
    {
        this->impl = &composite_array;

        this->add_widget(&this->img);
        this->add_widget(&this->title);
        this->add_widget(&this->dialog);
        this->add_widget(&this->separator);

        const int total_width = std::max(this->dialog.cx(), this->title.cx());
        int total_height = this->title.cy() + this->dialog.cy() + this->ok.cy() + 20;
        int y = 0;
        this->title.set_x(left + (this->cx() - this->title.cx()) / 2);
        // this->title.rect.cx = total_width;
        this->separator.set_x(left + (this->cx() - total_width) / 2);
        this->separator.set_cx(total_width);
        y = this->title.cy();
        this->separator.set_y(top + y + 3);
        this->dialog.set_x(this->separator.x());
        this->dialog.set_y(top + y + 10);

        y = this->dialog.y() + this->dialog.cy() + 10;

        if (has_challenge) {
            if (CHALLENGE_ECHO == has_challenge) {
                this->challenge = new WidgetEdit(this->drawable,
                                                 this->separator.x() - left + 10, y - top,
                                                 total_width - 20, *this, this, nullptr, -13,
                                                 theme.edit.fgcolor,
                                                 theme.edit.bgcolor,
                                                 theme.edit.focus_color, font, -1u, 1, 1);
            } else {
                this->challenge = new WidgetPassword(this->drawable,
                                                     this->separator.x() - left + 10,
                                                     y - top, total_width - 20, *this, this, nullptr,
                                                     -13, theme.edit.fgcolor,
                                                     theme.edit.bgcolor,
                                                     theme.edit.focus_color,
                                                     font, -1u, 1, 1);
            }
            this->add_widget(this->challenge);
            total_height += this->challenge->cy() + 10;
            y += this->challenge->cy() + 10;
            this->set_widget_focus(this->challenge, focus_reason_tabkey);
        }


        this->add_widget(&this->ok);
        y += 5;
        if (this->cancel) {
            this->add_widget(this->cancel);

            this->cancel->set_x(this->dialog.x() + this->dialog.cx() - (this->cancel->cx() + 10));
            this->ok.set_x(this->cancel->x() - (this->ok.cx() + 10));

            this->ok.set_y(y);
            this->cancel->set_y(y);
        }
        else {
            this->ok.set_x(this->dialog.x() + this->dialog.cx() - (this->ok.cx() + 10));
            this->ok.set_y(y);
        }
        this->move_xy(0, (height - total_height) / 2);

        this->img.set_x(left + (this->cx() - this->img.cx()) / 2);
        this->img.set_y(top + (3*(height - total_height) / 2 - this->img.cy()) / 2 + total_height);
        if (this->img.y() + this->img.cy() > top + height) {
            this->img.set_y(top);
        }

        if (!has_challenge)
            this->set_widget_focus(&this->ok, focus_reason_tabkey);
        else {
            if (extra_button) {
                this->add_widget(extra_button);

                extra_button->set_x(left + 60);
                extra_button->set_y(top + height - 60);
            }
        }
    }

    ~FlatDialog() override {
        delete this->challenge;
        delete this->cancel;
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_x(left);
        this->set_y(top);
        this->set_cx(width);
        this->set_cy(height);

        this->title.set_y(top);

        const int total_width = std::max(this->dialog.cx(), this->title.cx());
        int total_height = this->title.cy() + this->dialog.cy() + this->ok.cy() + 20;
        int y = 0;
        this->title.set_x(left + (this->cx() - this->title.cx()) / 2);
        this->separator.set_x(left + (this->cx() - total_width) / 2);
        y = this->title.cy();
        this->separator.set_y(top + y + 3);
        this->dialog.set_x(this->separator.x());
        this->dialog.set_y(top + y + 10);

        y = this->dialog.y() + this->dialog.cy() + 10;

        if (this->challenge) {
            this->challenge->set_x(this->separator.x() + 10);
            this->challenge->set_y(y);
            total_height += this->challenge->cy() + 10;
            y += this->challenge->cy() + 10;
        }

        y += 5;

        if (this->cancel) {
            this->cancel->set_x(this->dialog.x() + this->dialog.cx() - (this->cancel->cx() + 10));
            this->ok.set_x(this->cancel->x() - (this->ok.cx() + 10));

            this->ok.set_y(y);
            this->cancel->set_y(y);
        }
        else {
            this->ok.set_x(this->dialog.x() + this->dialog.cx() - (this->ok.cx() + 10));
            this->ok.set_y(y);
        }
        this->move_xy(0, (height - total_height) / 2);

        this->img.set_x(left + (this->cx() - this->img.cx()) / 2);
        this->img.set_y(top + (3*(height - total_height) / 2 - this->img.cy()) / 2 + total_height);
        if (this->img.y() + this->img.cy() > top + height) {
            this->img.set_y(top);
        }

        if (this->challenge && this->extra_button) {
            extra_button->set_x(left + 60);
            extra_button->set_y(top + height - 60);
        }
    }

    int get_bg_color() const override {
        return this->bg_color;
    }

    void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
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

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
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
};

