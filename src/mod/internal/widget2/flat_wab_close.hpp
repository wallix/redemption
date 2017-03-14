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
 *              Meng Tan
 */

#pragma once

#include "composite.hpp"
#include "flat_button.hpp"
#include "image.hpp"
#include "edit.hpp"
#include "label.hpp"
#include "multiline.hpp"
#include "utils/translation.hpp"
#include "widget2_rect.hpp"
#include "utils/theme.hpp"
#include "core/defines.hpp"
#include "gdi/graphic_api.hpp"

#include <vector>

class FlatWabClose : public WidgetParent
{
public:
    CompositeArray composite_array;

    WidgetLabel        connection_closed_label;
    WidgetRect         separator;

    WidgetLabel        username_label;
    WidgetLabel        username_value;
    WidgetLabel        target_label;
    WidgetLabel        target_value;
    WidgetLabel        diagnostic_label;
    WidgetMultiLine    diagnostic_value;
    WidgetLabel        timeleft_label;
    WidgetLabel        timeleft_value;

    WidgetFlatButton   cancel;
    WidgetFlatButton * back;

    WidgetImage        img;

private:
    BGRColor_ bg_color;

    long prev_time;

    Translation::language_t lang;

    bool showtimer;

public:
    FlatWabClose(gdi::GraphicApi & drawable,
                 int16_t left, int16_t top, int16_t width, int16_t height, Widget2& parent,
                 NotifyApi* notifier, const char * diagnostic_text,
                 const char * username, const char * target,
                 bool showtimer, Font const & font, Theme const & theme,
                 Translation::language_t lang, bool back_selector = false)
    : WidgetParent(drawable, parent, notifier)
    , connection_closed_label(drawable, *this, nullptr, TR("connection_closed", lang),
                              -13, theme.global.fgcolor,
                              theme.global.bgcolor, font)
    , separator(drawable, *this, this, -12,
                theme.global.separator_color)
    , username_label(drawable, *this, nullptr, "Username:", -11,
                     theme.global.fgcolor, theme.global.bgcolor, font)
    , username_value(drawable, *this, nullptr, username, -11,
                     theme.global.fgcolor, theme.global.bgcolor, font)
    , target_label(drawable, *this, nullptr, "Target:", -12,
                   theme.global.fgcolor, theme.global.bgcolor, font)
    , target_value(drawable, *this, nullptr, target, -12,
                   theme.global.fgcolor, theme.global.bgcolor, font)
    , diagnostic_label(drawable, *this, nullptr, "Diagnostic:", -15,
                       theme.global.fgcolor, theme.global.bgcolor, font)
    , diagnostic_value(drawable, *this, nullptr, diagnostic_text, -16,
                       theme.global.fgcolor, theme.global.bgcolor, font)
    , timeleft_label(drawable, *this, nullptr, "Time left:", -12,
                     theme.global.fgcolor, theme.global.bgcolor, font)
    , timeleft_value(drawable, *this, nullptr, nullptr, -12,
                     theme.global.fgcolor, theme.global.bgcolor, font)
    , cancel(drawable, *this, this, TR("close", lang), -14,
             theme.global.fgcolor, theme.global.bgcolor,
             theme.global.focus_color, 2, font, 6, 2)
    , back(back_selector ? new WidgetFlatButton(drawable, *this, this,
                                                TR("back_selector", lang), -14,
                                                theme.global.fgcolor,
                                                theme.global.bgcolor,
                                                theme.global.focus_color, 2, font,
                                                6, 2) : nullptr)
    , img(drawable,
          theme.global.logo ? theme.global.logo_path.c_str() :
          SHARE_PATH "/" LOGIN_WAB_BLUE, *this, nullptr, -10)
    , bg_color(theme.global.bgcolor)
    , prev_time(0)
    , lang(lang)
    , showtimer(showtimer)
    {
        this->impl = &composite_array;

        this->add_widget(&this->img);

        char label[255];
        snprintf(label, sizeof(label), "%s:", TR("username", lang));
        this->username_label.set_text(label);
        snprintf(label, sizeof(label), "%s:", TR("target", lang));
        this->target_label.set_text(label);
        snprintf(label, sizeof(label), "%s:", TR("diagnostic", lang));
        this->diagnostic_label.set_text(label);
        snprintf(label, sizeof(label), "%s:", TR("timeleft", lang));
        this->timeleft_label.set_text(label);

        this->add_widget(&this->connection_closed_label);
        this->add_widget(&this->separator);

        if (username && *username) {
            this->add_widget(&this->username_label);
            this->add_widget(&this->username_value);
            this->add_widget(&this->target_label);
            this->add_widget(&this->target_value);
        }

        this->add_widget(&this->diagnostic_label);
        this->add_widget(&this->diagnostic_value);

        if (showtimer) {
            this->add_widget(&this->timeleft_label);
            this->add_widget(&this->timeleft_value);
        }

        this->add_widget(&this->cancel);

        if (this->back) {
            this->add_widget(this->back);
        }

        this->move_size_widget(left, top, width, height);

        this->set_widget_focus(&this->cancel, focus_reason_tabkey);
    }

    ~FlatWabClose() override {
        delete this->back;
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_wh(width, height);
        this->set_xy(left, top);

        int16_t y = 10;

        Dimension dim = this->connection_closed_label.get_optimal_dim();
        this->connection_closed_label.set_wh(dim);
        this->connection_closed_label.set_xy(
            left + (this->cx() - this->connection_closed_label.cx()) / 2, top + y);
        y += this->connection_closed_label.cy();

        this->separator.set_wh(600, 2);
        this->separator.set_xy(left + (this->cx() - 600) / 2, top + y + 3);
        y += 30;

        uint16_t x = 0;

        if (this->username_value.buffer[0]) {
            dim = this->username_label.get_optimal_dim();
            this->username_label.set_wh(dim);
            this->username_label.set_xy(left + (width - 600) / 2,
                this->username_label.y());
            x = std::max<uint16_t>(this->username_label.cx(), x);

            dim = this->target_label.get_optimal_dim();
            this->target_label.set_wh(dim);
            this->target_label.set_xy(left + (width - 600) / 2,
                this->target_label.y());
            x = std::max<uint16_t>(this->target_label.cx(), x);
        }

        dim = this->diagnostic_label.get_optimal_dim();
        this->diagnostic_label.set_wh(dim);
        this->diagnostic_label.set_xy(left + (width - 600) / 2,
            this->diagnostic_label.y());
        x = std::max<uint16_t>(this->diagnostic_label.cx(), x);

        if (this->showtimer) {
            dim = this->timeleft_label.get_optimal_dim();
            this->timeleft_label.set_wh(dim);
            this->timeleft_label.set_xy(left + (width - 600) / 2,
                this->timeleft_label.y());
            x = std::max<uint16_t>(this->timeleft_label.cx(), x);
        }

        x += 10;

        if (this->username_value.buffer[0]) {
            this->username_label.set_xy(this->username_label.x(), top + y);

            dim = this->username_value.get_optimal_dim();
            this->username_value.set_wh(dim);
            this->username_value.set_xy(x + this->diagnostic_label.x(), top + y);

            y += this->username_label.cy() + 20;

            this->target_label.set_xy(this->target_label.x(), top + y);

            dim = this->target_value.get_optimal_dim();
            this->target_value.set_wh(dim);
            this->target_value.set_xy(x + this->diagnostic_label.x(), top + y);

            y += this->target_label.cy() + 20;
        }

        this->diagnostic_label.set_xy(this->diagnostic_label.x(), top + y);

        dim = this->diagnostic_value.get_optimal_dim();
        this->diagnostic_value.set_wh(dim);

        if (this->diagnostic_label.cx() > this->cx() - (x + 10)) {
            y += this->diagnostic_label.cy() + 10;

            this->diagnostic_value.set_xy(x + this->diagnostic_label.x(), top + y);
            y += this->diagnostic_value.cy() + 20;
        }
        else {
            this->diagnostic_value.set_xy(x + this->diagnostic_label.x(), top + y);
            y += std::max(this->diagnostic_value.cy(), this->diagnostic_label.cy()) + 20;
        }

        if (this->showtimer) {
            this->timeleft_label.set_xy(this->timeleft_label.x(), top + y);

            dim = this->timeleft_value.get_optimal_dim();
            this->timeleft_value.set_wh(dim);
            this->timeleft_value.set_xy(x + this->diagnostic_label.x(), top + y);

            y += this->timeleft_label.cy() + 20;
        }

        dim = this->cancel.get_optimal_dim();
        this->cancel.set_wh(dim);

        if (this->back) {
            dim = this->back->get_optimal_dim();
            this->back->set_wh(dim);
        }

        int const back_width = this->back ? this->back->cx() + 10 : 0;

        this->cancel.set_xy(left + (this->cx() - (this->cancel.cx() + back_width)) / 2,
                            top + y);

        if (this->back) {
            this->back->set_xy(this->cancel.x() + this->cancel.cx() + 10, top + y);
        }

        y += this->cancel.cy();

        this->move_children_xy(0, (height - y) / 2);

        dim = this->img.get_optimal_dim();
        this->img.set_wh(dim);

        this->img.set_xy(left + (this->cx() - this->img.cx()) / 2,
                         top + (3*(height - y) / 2 - this->img.cy()) / 2 + y);
        if (this->img.y() + this->img.cy() > top + height) {
            this->img.set_xy(this->img.x(), top);
        }
    }

    BGRColor_ get_bg_color() const override {
        return this->bg_color;
    }

    void refresh_timeleft(long tl) {
        bool seconds = true;
        if (tl > 60) {
            seconds = false;
            tl = tl / 60;
        }
        if (this->prev_time != tl) {
            char buff[256];
            snprintf(buff, sizeof(buff), "%ld %s%s %s. ",
                     tl,
                     seconds?TR("second", this->lang):TR("minute", this->lang),
                     (tl <= 1)?"":"s",
                     TR("before_closing", this->lang)
                     );

            Rect old = this->timeleft_value.get_rect();
            this->drawable.begin_update();
            this->timeleft_value.set_text(nullptr);
            this->rdp_input_invalidate(old);
            this->timeleft_value.set_text(buff);

            Dimension dim = this->timeleft_value.get_optimal_dim();
            this->timeleft_value.set_wh(dim);

            this->rdp_input_invalidate(this->timeleft_value.get_rect());
            this->drawable.end_update();

            this->prev_time = tl;
        }
    }

    void notify(Widget2 * widget, NotifyApi::notify_event_t event) override {
        if (widget == &this->cancel && event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_CANCEL);
        }
        else if (this->back && widget == this->back && event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_SUBMIT);
        }
        else {
            WidgetParent::notify(widget, event);
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
