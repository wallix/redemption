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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_WAB_CLOSE_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_WAB_CLOSE_HPP

#include "window.hpp"
#include "widget2_button.hpp"
#include "widget2_image.hpp"
#include "widget2_edit.hpp"
#include "widget2_multiline.hpp"

#include <vector>

class WindowWabClose : public Window
{
public:
    WidgetImage img;
    WidgetLabel username_label;
    WidgetLabel username_label_value;
    WidgetLabel target_label;
    WidgetLabel target_label_value;
    WidgetButton cancel;
    WidgetLabel diagnostic;
    WidgetMultiLine diagnostic_lines;

private:
    struct temporary_text {
        char text[WidgetLabel::buffer_size];

        temporary_text(const char * lhs, const char * rhs)
        {
            size_t len = std::min(WidgetLabel::buffer_size - 1, strlen(lhs));
            memcpy(this->text, lhs, len);
            size_t len2 = std::min(WidgetLabel::buffer_size - 1 - len, strlen(rhs));
            memcpy(&this->text[len], rhs, len2);
            this->text[len + len2] = '\0';
        }
    };

public:
    WindowWabClose(ModApi* drawable, int16_t x, int16_t y, Widget2* parent,
                   NotifyApi* notifier, const char * diagnostic_text, int group_id = 0,
                   const char * username = 0, const char * target = 0,
                   int fgcolor = BLACK, int bgcolor = DARK_WABGREEN)
    : Window(drawable, Rect(x,y,600,1), parent, notifier, "Close", bgcolor, group_id)
    , img(drawable, 0, 0, SHARE_PATH "/" LOGIN_LOGO24, this, NULL, -10)
    , username_label(drawable, this->img.cx() + 20, 0, this, NULL,
                     "Username:", true, -11, fgcolor, bgcolor)
    , username_label_value(drawable, 0, 0, this, NULL, username, true, -11, fgcolor, bgcolor)
    , target_label(drawable, this->img.cx() + 20, 0, this, NULL,
                   "Target:", true, -12, fgcolor, bgcolor)
    , target_label_value(drawable, 0, 0, this, NULL, target, true, -12, fgcolor, bgcolor)
    , cancel(drawable, 0, 0, this, this, "Close", true, -13, BLACK, GREY, 6, 2, NOTIFY_CANCEL)
    , diagnostic(drawable, this->img.cx() + 20, 0, this, NULL,
                 "Diagnostic:", true, -15, fgcolor, bgcolor)
    , diagnostic_lines(drawable, this->img.cx() + 20, 0, this, NULL,
                       diagnostic_text, true, -16, fgcolor, bgcolor)
    {
        this->child_list.push_back(&this->img);
        this->child_list.push_back(&this->username_label);
        this->child_list.push_back(&this->username_label_value);
        this->child_list.push_back(&this->target_label);
        this->child_list.push_back(&this->target_label_value);
        this->child_list.push_back(&this->cancel);
        this->child_list.push_back(&this->diagnostic);
        this->child_list.push_back(&this->diagnostic_lines);

        this->cancel.border_top_left_color = WHITE;

        this->img.rect.x = this->dx() + 10;
        this->cancel.set_button_x((this->cx() - this->cancel.cx()) / 2);

        uint16_t cx = std::max(this->username_label.cx(), this->target_label.cx()) + 10;
        this->username_label_value.rect.x = this->username_label.dx() + cx;
        this->target_label_value.rect.x = this->username_label.dx() + cx;

        this->resize_titlebar();

        y = this->dy() + this->titlebar.cy() + 10;
        this->img.rect.y = y;
        y += 10;
        this->username_label.rect.y = y;
        this->username_label_value.rect.y = y;
        y += this->username_label.cy() + 10;
        this->target_label.rect.y = y;
        this->target_label_value.rect.y = y;
        y += this->target_label.cy() + 20;
        this->diagnostic.rect.y = y;
        y += this->diagnostic.cy() + 10;
        this->diagnostic_lines.rect.y = y;
        y += this->diagnostic_lines.cy() + 20;
        this->cancel.set_button_y(y);
        y += this->cancel.cy() + 10;
        this->rect.cy = y - this->dy();
    }

    virtual ~WindowWabClose()
    {}
};

#endif
