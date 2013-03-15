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

#if !defined(REDEMPTION_MOD_WIDGET2_DIALOG_HPP)
#define REDEMPTION_MOD_WIDGET2_DIALOG_HPP

#include "window.hpp"
#include "multi_line.hpp"
#include "yes_no.hpp"

class WidgetDialog : public Window
{
public:
    WidgetMultiLine multi_line;
    WidgetYesNo yesno;

    WidgetDialog(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier, const char* caption, const char * message, int id = 0, int bgcolor1 = 0, int bgcolor2 = 0)
    : Window(drawable, rect, parent, 0, caption, id)
    , multi_line(drawable,
                 Rect(20,
                      this->titlebar.rect.cy + 2,
                      (uint16_t)std::max(rect.cx - 40, 0),
                      rect.cy - (this->titlebar.rect.cy + 2 + 40)),
                 this, 0, message, 0, bgcolor1, bgcolor2)
    , yesno(drawable, 200, rect.cy - 40, this, notifier, "Ok", "Cancel", 1)
    {
        if (drawable) {
            this->yesno.rect.x = (rect.cx - this->yesno.rect.cx) / 2;
            this->yesno.rect.y = rect.cy - this->yesno.rect.cy - 2;
            this->multi_line.rect.cy -= this->yesno.rect.cy - 40 + 4;
        }
    }

    virtual ~WidgetDialog()
    {}
};

#endif