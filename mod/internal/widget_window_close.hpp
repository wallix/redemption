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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#if !defined(__WIDGET_WINDOW_CLOSE_HPP__)
#define __WIDGET_WINDOW_CLOSE_HPP__

#include "widget.hpp"
#include "internal_mod.hpp"

struct wab_close : public window
{
    Widget & notify_to;
    Inifile * ini;
    widget_combo * combo;
    ModContext & context;
    window * help;

    wab_close(internal_mod * mod, const Rect & r, ModContext & context, Widget & parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    : window(mod, r, parent, bg_color, title),
      notify_to(notify_to),
      context(context)
    {
        struct Widget* but;

        if (regular) {
            widget_image * but = new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE,
                *this, 10, 30, SHARE_PATH "/" LOGIN_LOGO24, this->mod->screen.bpp);
            TODO(" bitmap load below should probably be done before call")
            this->child_list.push_back(but);
        }

        struct Widget* b;
        int count = 0;
        /* label */
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 70, 20),
            *this, "Username:");

        this->child_list.push_back(b);
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ?  230 : 70), 60 + 25 * count, 350, 20),
            *this, context.is_asked(STRAUTHID_AUTH_USER)?"":context.get(STRAUTHID_AUTH_USER));

        b->id = 100 + 2 * count;
        this->child_list.push_back(b);
        count ++;

        char target[255];
        if (context.is_asked(STRAUTHID_TARGET_USER)
        ||context.is_asked(STRAUTHID_TARGET_DEVICE)){
            target[0] = 0;
        }
        else {
            snprintf(target, 255, "%s@%s",
                context.get(STRAUTHID_TARGET_USER),
                context.get(STRAUTHID_TARGET_DEVICE));
        }

        b = new widget_label(this->mod,
            Rect(10+((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 70, 20),
            *this, "Target:");

        this->child_list.push_back(b);
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ?  230 : 70), 60 + 25 * count, 350, 20),
            *this, target);

        b->id = 100 + 2 * count;
        this->child_list.push_back(b);
        count ++;

        b = new widget_label(this->mod,
            Rect(150 + ((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 130, 20),
            *this, "Connection closed");

        this->child_list.push_back(b);
        count ++;

        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
            *this, "Diagnostic:");

        this->child_list.push_back(b);

        bool done = false;
        int line = 0;
        const char * message;
        message = context.get(STRAUTHID_AUTH_ERROR_MESSAGE);
        while (!done) {
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;
            b = new widget_label(this->mod, Rect((this->rect.cx >= 400) ?  230 : 70, 60 + 25 * count + 16 * line, 350, 20), *this, tmp);
            this->child_list.push_back(b);
            line++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }

        /* label */
        but = new widget_button(this->mod,
              Rect(50 + (regular ? 250 : ((r.cx - 30) - 60)), 150 + 16 * line, 60, 25),
              *this, 2, 1, "Close");
        this->child_list.push_back(but);
        this->esc_button = but;
        this->default_button = but;

    }

    ~wab_close(){
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        return;
    }

 };

#endif
