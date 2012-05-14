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

#if !defined(__MOD_INTERNAL_WIDGET_WINDOW_DIALOG_HPP__)
#define __MOD_INTERNAL_WIDGET_WINDOW_DIALOG_HPP__

#include "widget.hpp"
#include "internal_mod.hpp"
#include "widget_window.hpp"
#include "widget_button.hpp"
#include "widget_label.hpp"

struct window_dialog : public window
{
    ModContext * context;

    window_dialog(internal_mod * mod, const Rect & r,
                  ModContext & context,
                  Widget * parent, int bg_color,
                  const char * title, Inifile * ini, int regular,
                  const char * message,
                  size_t max_message_lines,
                  const char * refuse)
    : window(mod, r, parent, bg_color, title)
    {
        struct Widget* but;
        this->context = &context;
        this->esc_button = NULL;

        but = new widget_button(this->mod, Rect(200, 160, 60, 25), this, 3, 1, "OK");
        this->child_list.push_back(but);
        this->default_button = but;

        if (refuse) {
            but = new widget_button(this->mod, Rect(300, r.y - 20, 60, 25), this, 2, 1, refuse);
            this->child_list.push_back(but);
            this->esc_button = but;
            this->default_button = but;
        }
        size_t count = 0;
        bool done = false;
        while (!done) {
            if (count >= max_message_lines){
                break;
            }
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;
            LOG(LOG_INFO, "line=%s", tmp);
            but = new widget_label(this->mod, Rect(50, 20 + 16 * count, 500, 40), this, tmp);
            this->child_list.push_back(but);
            count++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (this->modal_dialog != 0 && msg != 100) {
            return;
        }
        TODO(" use symbolic button ids instead of constants 2 and 3")
        if (msg == 1) { /* click */
            LOG(LOG_INFO, "windget_window_dialog::notify %d sender->id=%d", msg, sender->id);
            switch (sender->id) {
            case 2: /* cancel button -> Esc */
                this->context->cpy(
                        (this->esc_button)?STRAUTHID_ACCEPT_MESSAGE
                                          :STRAUTHID_DISPLAY_MESSAGE,
                        "False");
                this->mod->event->set();
                this->mod->signal = BACK_EVENT_1;
            break;
            case 3: /* ok button -> Enter */
                this->context->cpy(
                        (this->esc_button)?STRAUTHID_ACCEPT_MESSAGE
                                          :STRAUTHID_DISPLAY_MESSAGE,
                        "True");
                this->mod->event->set();
                this->mod->signal = BACK_EVENT_1;
            break;
            default:
            break;
            }
        }
        return;
    }

    virtual ~window_dialog(){
    }
};


#endif
