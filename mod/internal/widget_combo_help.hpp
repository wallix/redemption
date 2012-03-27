/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Login Window

*/

#if !defined(__WIDGET_COMBO_HELP_HPP__)
#define __WIDGET_COMBO_HELP_HPP__

#include "widget_window_login.hpp"

#include "version.hpp"

struct combo_help : public window
{
    Widget & notify_to;

    combo_help(internal_mod * mod, const Rect & r, Widget * parent, Widget & notify_to, int bg_color, const char * title)
    : window(mod, r, parent, bg_color, title), notify_to(notify_to)
    {
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (msg == 1) { /* click */
            if (sender->id == 1) { /* ok button */
                    this->notify_to.notify(this, 100, 1, 0); /* ok */
            }
        } else if (msg == WM_PAINT) { /* 3 */
            TODO(" the code below is a bit too much specialized. Change it to some code able to write a paragraph of text in a given rectangle. Later we may even add some formatting support.")
            const char * message =
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
                    "having problems logging on.<br>"
                    ;
            int count = 0;
            bool done = false;
            while (!done) {
                const char * str = strstr(message, "<br>");
                char tmp[256];
                tmp[0] = 0;
                strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
                tmp[255] = 0;

                Rect r(0, 0, this->rect.cx, this->rect.cy);
                const Rect scr_r = this->to_screen_rect(r);
                const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

                for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
                    this->mod->front.server_draw_text(scr_r.x + 10, scr_r.y + 30 + 16 * count, tmp, GREY, BLACK, region.rects[ir].intersect(this->to_screen_rect(this->rect.wh())));
                }

                count++;
                if (!str){
                    done = true;
                }
                else {
                    message = str + 4;
                }
            }
        }
        return;
    }

};

#endif
