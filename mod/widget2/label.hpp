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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_WIDGET2_LABEL_HPP_HPP)
#define REDEMPTION_MOD_WIDGET2_LABEL_HPP_HPP

#include "widget.hpp"
#include "utf.hpp"

class WidgetLabel : public Widget
{
    char buffer[256];
    ModApi::ContextText * context_text;

public:
    WidgetLabel(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier, const char * text, int id = 0)
    : Widget(drawable, rect, parent, Widget::TYPE_LABEL, notifier, id)
    , context_text(0)
    {
        this->set_text(text);
    }

    virtual ~WidgetLabel()
    {
        delete context_text;
    }

    void set_text(const char * text)
    {
        this->context_text = 0;
        if (text) {
            const size_t max = sizeof(this->buffer)/sizeof(this->buffer[0]) - 1;
            strncpy(buffer, text, max);
            this->buffer[max] = 0;
            if (this->drawable) {
                this->context_text = this->drawable->create_context_text(this->buffer);
            }
        } else {
            this->buffer[0] = 0;
        }
    }

    const char * get_text() const
    {
        return this->buffer;
    }

    virtual void draw(const Rect& rect, int16_t x_screen, int16_t y_screen, const Rect& clip_screen)
    {
        this->Widget::draw(rect, x_screen, y_screen, clip_screen);
        this->context_text->draw_in(this->drawable, rect, x_screen, y_screen, clip_screen, ~this->bg_color);
    }
};

#endif