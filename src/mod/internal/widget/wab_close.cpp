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

#include "mod/internal/widget/wab_close.hpp"
#include "utils/theme.hpp"
#include "utils/strutils.hpp"
#include "core/app_path.hpp"
#include "core/font.hpp"
#include "gdi/graphic_api.hpp"

WidgetWabClose::WidgetWabClose(
    gdi::GraphicApi & drawable,
    int16_t left, int16_t top, int16_t width, int16_t height,
    Events events, const char * diagnostic_text,
    const char * username, const char * target,
    bool showtimer, const char * extra_message, Font const & font, Theme const & theme,
    Language lang, bool back_selector)
: WidgetComposite(drawable, Focusable::Yes)
, oncancel(events.oncancel)
, connection_closed_label(drawable, TR(trkeys::connection_closed, lang).to_sv(),
                          theme.global.fgcolor, theme.global.bgcolor, font)
, separator(drawable, theme.global.separator_color)
, username_label(drawable, "Username:",
                 theme.global.fgcolor, theme.global.bgcolor, font)
, username_value(drawable, username,
                 theme.global.fgcolor, theme.global.bgcolor, font)
, target_label(drawable, "Target:",
               theme.global.fgcolor, theme.global.bgcolor, font)
, target_value(drawable, target,
               theme.global.fgcolor, theme.global.bgcolor, font)
, diagnostic_label(drawable, "Diagnostic:",
                   theme.global.fgcolor, theme.global.bgcolor, font)
, diagnostic_value(drawable, diagnostic_text,
                   theme.global.fgcolor, theme.global.bgcolor, font)
, timeleft_label(drawable, "Time left:",
                theme.global.fgcolor, theme.global.bgcolor, font)
, timeleft_value(drawable, nullptr,
                 theme.global.fgcolor, theme.global.bgcolor, font)
, cancel(drawable, TR(trkeys::close, lang),
         events.oncancel,
         theme.global.fgcolor, theme.global.bgcolor,
         theme.global.focus_color, 2, font, 6, 2)
, back(back_selector ? new WidgetButton(drawable, TR(trkeys::back_selector, lang),
                                        events.onback_to_selector,
                                        theme.global.fgcolor,
                                        theme.global.bgcolor,
                                        theme.global.focus_color, 2, font,
                                        6, 2) : nullptr)
, img(drawable,
      theme.global.enable_theme ? theme.global.logo_path.c_str() :
      app_path(AppPath::LoginWabBlue),
      theme.global.bgcolor)
, prev_time(0)
, lang(lang)
, showtimer(showtimer)
, font(font)
, diagnostic_text(diagnostic_text)
{
    this->set_bg_color(theme.global.bgcolor);
    this->add_widget(this->img);

    char label[255];
    snprintf(label, sizeof(label), "%s:", TR(trkeys::username, lang).c_str());
    this->username_label.set_text(label);
    snprintf(label, sizeof(label), "%s:", TR(trkeys::target, lang).c_str());
    this->target_label.set_text(label);
    snprintf(label, sizeof(label), "%s:", TR(trkeys::diagnostic, lang).c_str());
    this->diagnostic_label.set_text(label);
    snprintf(label, sizeof(label), "%s:", TR(trkeys::timeleft, lang).c_str());
    this->timeleft_label.set_text(label);

    this->add_widget(this->connection_closed_label);
    this->add_widget(this->separator);

    if (username && *username) {
        this->add_widget(this->username_label);
        this->add_widget(this->username_value);
        this->add_widget(this->target_label);
        this->add_widget(this->target_value);
    }

    this->add_widget(this->diagnostic_label);
    this->add_widget(this->diagnostic_value);

    if (showtimer) {
        this->add_widget(this->timeleft_label);
        this->add_widget(this->timeleft_value);
    }

    this->add_widget(this->cancel, HasFocus::Yes);

    if (this->back) {
        this->add_widget(*this->back);
    }

    this->fixed_format_diagnostic_text =
        (this->diagnostic_text.find('\n') != std::string::npos);

    if (!this->fixed_format_diagnostic_text && extra_message && *extra_message) {
        str_append(this->diagnostic_text, ' ', extra_message);
    }

    this->move_size_widget(left, top, width, height);
}

WidgetWabClose::~WidgetWabClose()
{
    delete this->back;
}

void WidgetWabClose::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_wh(width, height);
    this->set_xy(left, top);

    int16_t y = 10;

    Dimension dim = this->connection_closed_label.get_optimal_dim();
    this->connection_closed_label.set_wh(dim);
    this->connection_closed_label.set_xy(
        left + (this->cx() - this->connection_closed_label.cx()) / 2, top + y);
    y += this->connection_closed_label.cy();

    // this->separator.set_wh(600, 2);
    // this->separator.set_xy(left + (this->cx() - 600) / 2, top + y + 3);
    this->separator.set_wh(std::max(600, width / 3 * 2), 2);
    this->separator.set_xy(left + (this->cx() - this->separator.cx()) / 2, top + y + 3);
    y += 30;

    uint16_t x = 0;

    if (this->username_value.get_text()[0]) {
        dim = this->username_label.get_optimal_dim();
        this->username_label.set_wh(dim);
        this->username_label.set_xy(left + (width - this->separator.cx()) / 2,
            this->username_label.y());
        x = std::max<uint16_t>(this->username_label.cx(), x);

        dim = this->target_label.get_optimal_dim();
        this->target_label.set_wh(dim);
        this->target_label.set_xy(left + (width - this->separator.cx()) / 2,
            this->target_label.y());
        x = std::max<uint16_t>(this->target_label.cx(), x);
    }

    dim = this->diagnostic_label.get_optimal_dim();
    this->diagnostic_label.set_wh(dim);
    this->diagnostic_label.set_xy(left + (width - this->separator.cx()) / 2,
        this->diagnostic_label.y());
    x = std::max<uint16_t>(this->diagnostic_label.cx(), x);

    if (this->showtimer) {
        dim = this->timeleft_label.get_optimal_dim();
        this->timeleft_label.set_wh(dim);
        this->timeleft_label.set_xy(left + (width - this->separator.cx()) / 2,
            this->timeleft_label.y());
        x = std::max<uint16_t>(this->timeleft_label.cx(), x);
    }

    x += 10;

    if (this->username_value.get_text()[0]) {
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
    if (this->fixed_format_diagnostic_text) {
        this->diagnostic_value.set_wh(dim);
    }
    else {
        std::string formatted_diagnostic_text;
        gdi::MultiLineTextMetrics line_metrics(
            this->font, this->diagnostic_text.c_str(),
            (this->diagnostic_label.cx() > this->cx() - (x + 10))
                ? this->separator.cx()
                : this->separator.cx() - x);
        this->diagnostic_value.set_wh(
            line_metrics.max_width(),
            std::max(int(this->font.max_height() * line_metrics.lines().size()), int(dim.h)));
        this->diagnostic_value.set_text(std::move(line_metrics));
    }

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

std::chrono::seconds WidgetWabClose::refresh_timeleft(std::chrono::seconds remaining)
{
    long tl = remaining.count();
    bool seconds = true;
    if (tl > 60) {
        seconds = false;
        tl = tl / 60;
    }
    if (this->prev_time != tl) {
        char buff[256];
        snprintf(
            buff, sizeof(buff), "%ld %s%s %s. ",
            tl,
            seconds
                ? TR(trkeys::second, this->lang).c_str()
                : TR(trkeys::minute, this->lang).c_str(),
            (tl <= 1) ? "" : "s",
            TR(trkeys::before_closing, this->lang).c_str()
        );

        Rect old = this->timeleft_value.get_rect();
        this->timeleft_value.set_text(nullptr);
        this->rdp_input_invalidate(old);
        this->timeleft_value.set_text(buff);

        Dimension dim = this->timeleft_value.get_optimal_dim();
        this->timeleft_value.set_wh(dim);

        this->rdp_input_invalidate(this->timeleft_value.get_rect());

        this->prev_time = tl;
    }

    return std::chrono::seconds(seconds ? 1 : 60);
}

void WidgetWabClose::rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    if (pressed_scancode(flags, scancode) == Scancode::Esc) {
        oncancel();
    }
    else {
        WidgetComposite::rdp_input_scancode(flags, scancode, event_time, keymap);
    }
}
