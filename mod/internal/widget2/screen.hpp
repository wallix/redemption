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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SCREEN_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SCREEN_HPP

#include "composite.hpp"
#include "tooltip.hpp"

#include <typeinfo>

class WidgetScreen : public WidgetComposite
{

    Widget2 * w_over;
public:
    WidgetTooltip * tooltip;

    WidgetScreen(DrawApi& drawable, uint16_t width, uint16_t height, NotifyApi * notifier = NULL)
        : WidgetComposite(drawable, Rect(0, 0, width, height), *this, notifier)
        , w_over(NULL)
        , tooltip(NULL)
    {
        this->tab_flag = IGNORE_TAB;
    }

    virtual ~WidgetScreen()
    {
        if (this->tooltip) {
            delete this->tooltip;
            this->tooltip = NULL;
        }
    }

    virtual void notify(Widget2* widget, notify_event_t event)
    {
        if (NOTIFY_SHOW_TOOLTIP == event) {
            this->add_widget(this->tooltip);
            this->refresh(this->tooltip->rect);
            this->w_over = widget;
        }
        if (NOTIFY_HIDE_TOOLTIP == event) {
            if (this->tooltip) {
                this->remove_widget(this->tooltip);
                this->refresh(this->tooltip->rect);
                delete this->tooltip;
                this->tooltip = NULL;
            }
        }
        WidgetComposite::notify(widget, event);
    }

    void show_tooltip(Widget2 * widget, const char * text, int x, int y) {
        if (this->tooltip == NULL) {
            int w = 0;
            int h = 0;
            this->drawable.text_metrics(text, w, h);
            int sw = this->rect.cx;
            int posx = ((x + w) > sw)?(sw - w):x;
            int posy = (y > h)?(y - h):0;
            this->tooltip = new WidgetTooltip(this->drawable,
                                              posx,
                                              posy,
                                              *this, this,
                                              text);
            this->notify(widget, NOTIFY_SHOW_TOOLTIP);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (this->tooltip) {
            if (device_flags & MOUSE_FLAG_MOVE) {
                Widget2 * w = this->last_widget_at_pos(x, y);
                if (w != this->w_over) {
                    this->notify(this, NOTIFY_HIDE_TOOLTIP);
                    this->w_over = w;
                }
            }
        }
        WidgetComposite::rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        this->notify(this, NOTIFY_HIDE_TOOLTIP);
        if (this->tab_flag != IGNORE_TAB) {
            WidgetComposite::rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
        else if (this->current_focus) {
            this->current_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }

        for (uint32_t n = keymap->nb_kevent_available(); n ; --n) {
            keymap->get_kevent();
        }
    }

    virtual void draw(const Rect& clip)
    {
        this->WidgetComposite::draw(clip);
        this->WidgetComposite::draw_inner_free(clip, BLACK);
    }
};

#endif
