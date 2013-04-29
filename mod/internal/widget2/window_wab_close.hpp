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
#include "button.hpp"
#include "image.hpp"
#include "edit.hpp"
#include "multiline.hpp"

#include <vector>

class WindowWabClose : public Window
{
public:
    WidgetImage img;
    WidgetLabel username_label;
    WidgetLabel target_label;
    WidgetButton cancel;
    WidgetLabel diagnostic;
    WidgetMultiLine diagnostic_lines;

private:
    struct temporary_text {
        char buffer[WidgetLabel::buffer_size];
        const char * text;

        temporary_text(const char * lhs, const char * rhs)
        {
            if (rhs && *rhs) {
                size_t len = std::min(WidgetLabel::buffer_size - 1, strlen(lhs));
                memcpy(this->buffer, lhs, len);
                size_t len2 = std::min(WidgetLabel::buffer_size - 1 - len, strlen(rhs));
                memcpy(&this->buffer[len], rhs, len2);
                this->buffer[len + len2] = '\0';
                this->text = this->buffer;
            }
            else {
                this->text = lhs;
            }
        }
    };

public:
    WindowWabClose(ModApi* drawable, int16_t x, int16_t y, Widget2* parent,
                   NotifyApi* notifier, const char * diagnostic_text, int group_id = 0,
                   const char * username = 0, const char * target = 0,
                   int fgcolor = BLACK, int bgcolor = DARK_WABGREEN)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, "Connection closed", bgcolor, group_id)
    , img(drawable, 0, 0, SHARE_PATH "/" LOGIN_LOGO24, this, NULL, -10)
    , username_label(drawable, this->img.cx() + 20, 0, this, NULL,
                     temporary_text("Username: ", username).text,
                     true, -11, fgcolor, bgcolor)
    , target_label(drawable, this->img.cx() + 20, 0, this, NULL,
                   temporary_text("Target: ", target).text,
                   true, -12, fgcolor, bgcolor)
    , cancel(drawable, 0, 0, this, this, "Cancel", true, -13, BLACK, WHITE, 6, 2)
    , diagnostic(drawable, this->img.cx() + 20, 0, this, NULL,
                 "Diagnostic:", true, -15, fgcolor, bgcolor)
    , diagnostic_lines(drawable, this->img.cx() + 20, 0, this, NULL,
                       diagnostic_text, true, -16, fgcolor, bgcolor)
    {
        this->child_list.push_back(&this->img);
        this->child_list.push_back(&this->username_label);
        this->child_list.push_back(&this->target_label);
        this->child_list.push_back(&this->cancel);
        this->child_list.push_back(&this->diagnostic);
        this->child_list.push_back(&this->diagnostic_lines);

        x = this->dx() + 10;
        this->img.rect.x = x;
        x += this->img.cx() + std::max(std::max(std::max(this->username_label.cx(), this->target_label.cx()), this->diagnostic.cx()), this->diagnostic_lines.cx()) + 20;
        this->rect.cx = x - this->dx();
        this->cancel.set_button_x(x - (this->cancel.cx() + 10));

        this->resize_titlebar();

        y = this->dy() + this->titlebar.cy() + 10;
        this->img.rect.y = y;
        y += 10;
        this->username_label.rect.y = y;
        y += this->username_label.cy() + 10;
        this->target_label.rect.y = y;
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