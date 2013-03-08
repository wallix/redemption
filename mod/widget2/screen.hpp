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

#if !defined(REDEMPTION_MOD_WIDGET2_SCREEN_HPP_)
#define REDEMPTION_MOD_WIDGET2_SCREEN_HPP_

#include "widget_composite.hpp"

class WidgetScreen : public WidgetComposite
{
public:
    WidgetScreen(ModApi * drawable, uint16_t width, uint16_t height, NotifyApi * notifier)
    : WidgetComposite(drawable, Rect(0, 0, width, height), 0, Widget::TYPE_SCREEN, notifier)
    {
        this->has_focus = true;
    }

    void paint()
    {

    }
};

#endif