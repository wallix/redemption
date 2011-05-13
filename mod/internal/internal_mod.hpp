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

*/

#if !defined(__INTERNAL_MOD_HPP__)
#define __INTERNAL_MOD_HPP__

#include "../mod/internal/widget.hpp"
#include "client_mod.hpp"

struct internal_mod : public client_mod {
    public:
    widget_screen screen;
    internal_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front)
            : client_mod(keys, key_flags, keymap, front),
                screen(this,
                 front.orders->rdp_layer->client_info.width,
                 front.orders->rdp_layer->client_info.height,
                 front.orders->rdp_layer->client_info.bpp)
    {
    }

    virtual ~internal_mod()
    {
        this->screen.delete_all_childs();
    }

    size_t nb_windows()
    {
        return this->screen.child_list.size();
    }


    virtual void front_resize() {
        this->screen.rect.cx = this->front->orders->rdp_layer->client_info.width;
        this->screen.rect.cy = this->front->orders->rdp_layer->client_info.height;
        this->screen.bpp = this->front->orders->rdp_layer->client_info.bpp;
    }

    Widget * window(int i)
    {
        return this->screen.child_list[i];
    }

    virtual void invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->server_begin_update();
            Rect r(0, 0, this->get_front_width(), this->get_front_height());
            this->mod_event(WM_INVALIDATE,
                ((r.x & 0xffff) << 16) | (r.y & 0xffff),
                ((r.cx & 0xffff) << 16) | (r.cy & 0xffff),
                0, 0);

            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    b->Widget_invalidate_clip(r2);
                }
            }
            this->server_end_update();
        }
    }

    widget_screen * get_screen_wdg(){
        return &(this->screen);
    }


    virtual int mod_event(int msg, long param1, long param2, long param3, long param4)
    {
        return 0;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual int mod_signal()
    {
        return 0;
    }
};

#endif
