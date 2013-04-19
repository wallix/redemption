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

#if !defined(REDEMPTION_MOD_WIDGET2_WINDOW_HELP_HPP)
#define REDEMPTION_MOD_WIDGET2_WINDOW_HELP_HPP

#include "window.hpp"
#include "multiline.hpp"

class WindowHelp : public Window
{
public:
    WidgetMultiLine multi_line;

    WindowHelp(ModApi* drawable, const Rect& rect, Widget2 * parent, NotifyApi* notifier, const char* caption, int id = 0)
    : Window(drawable, rect, parent, notifier, caption, id)
    , multi_line(drawable, Rect(20,
                                this->titlebar.rect.cy + 2,
                                rect.cx - 40,
                                rect.cy - (this->titlebar.rect.cy + 4)
                               ), this, 0,
                 "You must be authenticated before using this<br>"
                 "session.<br>"
                 "<br>"
                 "Enter a valid username in the username edit box.<br>"
                 "Enter the password in the password edit box.<br>"
                 "<br>"
                 "Both the username and password are case<br>"
                 "sensitive.<br>"
                 "<br>"
                 "Contact your system administrator if you are<br>"
                 "having problems logging on.",
                 id)
    {
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2* keymap)
    {
        if (event == KEYDOWN && keymap->top_kevent() && Keymap2::KEVENT_ESC) {
            this->notify_parent(WIDGET_CANCEL);
        } else {
            this->Window::send_event(event, param, param2, keymap);
        }
    }
};

#endif