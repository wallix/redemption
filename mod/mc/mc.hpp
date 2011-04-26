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

   media center

*/

#if !defined(__MC_HPP__)
#define __MC_HPP__

#include "client_mod.hpp"
#include "widget.hpp"


struct window_custom : public window
{
    Widget & notify_to;

    window_custom(client_mod * mod, const Rect & r, Widget & parent, Widget & notify_to, const char * title)
    : window(mod, r, parent, GREY, title), notify_to(notify_to)
    {
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        LOG(LOG_DEBUG, "received notify message in MC window\n");
        return;
    }
};


struct mc_mod : public client_mod {
    /* mod data */
    int width;
    int height;
    int bpp;
    window_custom * wnd;
    wait_obj * event;
    bool click_ok;

    mc_mod(wait_obj * event, int (& keys)[256], int & key_flags, Keymap * &keymap,
            struct ModContext & context, struct Front & front)
            : client_mod(keys, key_flags, keymap, front)
    {
        LOG(LOG_DEBUG, "new mc_mod\n");
        this->event = event;
        this->wnd = new window_custom(this,
            Rect(10, 10, 400, 400),
            this->screen, // parent
            this->screen, // notify_to
            "Custom window"
        );
        this->click_ok = false;

        /* ok button */
        struct widget_button * but = new widget_button(this,
                Rect(300,200,60,25),
                *this->wnd, 1, 1, "OK");
        this->wnd->child_list.push_back(but);
        but->has_focus = true;

        this->wnd->focus(this->wnd->rect);
        this->wnd->Widget_invalidate(this->wnd->rect);
    }

    virtual ~mc_mod()
    {
            LOG(LOG_DEBUG, "delete mc_mod\n");
    }

    // module received an event from server
    virtual int mod_event(int msg, long param1, long param2, long param3, long param4)
    {
        switch (msg){
        case WM_CHANNELDATA:
            LOG(LOG_WARNING, "channel data event %d\n", msg);
        break;
        case WM_KEYDOWN:
        case WM_KEYUP:
            LOG(LOG_WARNING, "key event %d\n", msg);
        break;
        case WM_MOUSEMOVE:
            LOG(LOG_WARNING, "mouse move event %d\n", msg);
        break;
        case WM_LBUTTONUP:
        {
            struct Widget *b = this->screen.widget_at_pos(param1, param2);
            if (b == this->wnd->child_list[0]){
                this->click_ok = true;
            }
            this->event->set();
        }
        break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_BUTTON3UP:
        case WM_BUTTON3DOWN:
        case WM_BUTTON4UP:
        case WM_BUTTON4DOWN:
        case WM_BUTTON5UP:
        case WM_BUTTON5DOWN:
            LOG(LOG_WARNING, "mouse click event %d\n", msg);
            this->event->set();
        break;
        case WM_INVALIDATE:
        break;
        default:
            LOG(LOG_WARNING, "unexpected message %d\n", msg);
            break;
        }
        return 0;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual int mod_signal()
    {
        LOG(LOG_DEBUG, "mc_mod mod_signal\n");
        this->event->reset();
        if (this->click_ok){
            return 1;
        }
        else {
            return 0;
        }
    }

};

#endif
