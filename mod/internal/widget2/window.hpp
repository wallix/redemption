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

#if !defined(REDEMPTION_MOD_WIDGET2_WINDOW_HPP_)
#define REDEMPTION_MOD_WIDGET2_WINDOW_HPP_

#include "widget_composite.hpp"
#include "label.hpp"
#include "colors.hpp"

class Window : public WidgetComposite
{
public:
    WidgetLabel titlebar;

    Window(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier,
           const char * caption, int id = 0)
    : WidgetComposite(drawable, rect, parent, notifier, id)
    , titlebar(drawable, 0,0, this, 0, caption, false, -1, WABGREEN, BLACK, 5, 0)
    {
        this->child_list.push_back(this->titlebar);
    }

    virtual ~Window()
    {}
};

#endif