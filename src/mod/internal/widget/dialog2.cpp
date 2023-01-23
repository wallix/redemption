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
#include "mod/internal/widget/dialog2.hpp"
#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/edit.hpp"
#include "utils/theme.hpp"
#include "keyboard/keymap.hpp"
#include "gdi/graphic_api.hpp"

enum {
    WIDGET_MULTILINE_BORDER_X = 10,
    WIDGET_MULTILINE_BORDER_Y = 4
};

WidgetDialog2::WidgetDialog2(
    gdi::GraphicApi & drawable, Rect const widget_rect,
    Events events,
    const char* caption, const char * text,
    const char * link_value, const char * link_label,
    CopyPaste & copy_paste, Theme const & theme,
    Font const & font, const char * ok_text
)
    : WidgetParent(drawable, Focusable::Yes)
    , oncancel(events.oncancel)
    , title(drawable, caption,
            theme.global.fgcolor, theme.global.bgcolor, font, 5)
    , separator(drawable, theme.global.separator_color)
    , dialog(drawable, text,
             theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color,
             font, WIDGET_MULTILINE_BORDER_X, WIDGET_MULTILINE_BORDER_Y)
    , ok(drawable, ok_text ? ok_text : "Ok", events.onsubmit,
         theme.global.fgcolor, theme.global.bgcolor,
         theme.global.focus_color, 2, font, 6, 2)
    , img(drawable,
          theme.global.enable_theme ? theme.global.logo_path.c_str() :
          app_path(AppPath::LoginWabBlue),
          theme.global.bgcolor)
    , link_label(drawable, link_label, theme.global.fgcolor, theme.global.bgcolor, font)
    , link_value(link_value)
    , link_show(drawable, link_value,
                theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color,
                font, WIDGET_MULTILINE_BORDER_X, WIDGET_MULTILINE_BORDER_Y)
    , link_copy(
        drawable, [this]{ this->copy_paste.copy(this->link_value); },
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, font, 2, 2, WidgetDelegatedCopy::MouseButton::Both)
    , copy_paste(copy_paste)
    , bg_color(theme.global.bgcolor)
{
    this->impl = &composite_array;

    this->add_widget(this->img);

    this->add_widget(this->title);
    this->add_widget(this->separator);
    this->add_widget(this->dialog);
    this->add_widget(this->link_label);
    this->add_widget(this->link_copy);
    this->add_widget(this->link_show);

    this->add_widget(this->ok, HasFocus::Yes);

    this->move_size_widget(widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy);
}

void WidgetDialog2::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
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

    this->link_show.set_wh(width * 4 / 5 - WIDGET_MULTILINE_BORDER_X * 2, height / 2);
    {
        auto dim = this->link_show.get_optimal_dim();
        if (dim.h < this->link_show.cy()) {
            this->link_show.set_wh(dim.w, dim.h);
        }
    }

    const auto total_width = std::max(std::max(this->dialog.cx(), this->title.cx()),
                                      this->link_show.cx());
    // const int total_width = std::max(this->dialog.cx(), this->title.cx());

    this->separator.set_wh(total_width, 2);
    this->separator.set_xy(left + (width - total_width) / 2, y + 3);

    y            += 10;
    total_height += 10;

    this->dialog.set_xy(this->separator.x(), y);

    y            += this->dialog.cy() + 10;
    total_height += this->dialog.cy() + 10;

    y += 5;

    // to activate
    this->link_show.set_xy(this->separator.x(), y);

    y            += this->link_show.cy() + 10;
    total_height += this->link_show.cy() + 10;

    y += 5;

    dim = this->link_label.get_optimal_dim();

    this->link_label.set_wh(dim);
    this->link_label.set_xy(this->separator.x() + WIDGET_MULTILINE_BORDER_X, y);

    dim = this->link_copy.get_optimal_dim();
    this->link_copy.set_wh(dim);
    this->link_copy.set_xy(this->link_label.x() + this->link_label.cx(), y);

    y            += this->link_label.cy() + 10;
    total_height += this->link_label.cy() + 10;

    y += 5;

    dim = this->ok.get_optimal_dim();
    this->ok.set_wh(dim);

    this->ok.set_xy(this->separator.x() + this->separator.cx() - (this->ok.cx() + 10), y);

    total_height += this->ok.cy();

    this->move_children_xy(0, (height - total_height) / 2);

    dim = this->img.get_optimal_dim();
    this->img.set_wh(dim);

    this->img.set_xy(left + (width - this->img.cx()) / 2,
                        top + (3 * (height - total_height) / 2 - this->img.cy()) / 2 + total_height);
    if (this->img.y() + this->img.cy() > top + height) {
        this->img.set_xy(this->img.x(), top);
    }

}

Widget::Color WidgetDialog2::get_bg_color() const
{
    return this->bg_color;
}

void WidgetDialog2::rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
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
        default:
            WidgetParent::rdp_input_scancode(flags, scancode, event_time, keymap);
            break;
    }
    REDEMPTION_DIAGNOSTIC_POP()
}
