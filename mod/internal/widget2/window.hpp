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
#include "button.hpp"
#include <region.hpp>

class Window : public WidgetComposite
{
public:
    WidgetLabel titlebar;
    WidgetButton button_close;
    int bg_color;

    Window(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier,
           const char * caption, int bgcolor = DARK_WABGREEN, int id = 0)
    : WidgetComposite(drawable, rect, parent, notifier, id)
    , titlebar(drawable, 0, 0, this, NULL, caption, false, -1, WABGREEN, BLACK, 5)
    , button_close(drawable, 0, 0, this, this, "X", true, -2, DARK_GREEN, WHITE, 3, 0)
    , bg_color(bgcolor)
    {
        this->child_list.push_back(&this->titlebar);
        this->child_list.push_back(&this->button_close);
    }

    void resize_titlebar()
    {
        int w,h;
        this->drawable->text_metrics(this->titlebar.buffer, w,h);
        this->titlebar.rect.cx = this->cx();
        this->titlebar.rect.cy = h;
        this->titlebar.y_text = std::max((this->button_close.cy() + 4 - h)/4, 0);
        this->titlebar.rect.cy += this->titlebar.y_text*2;
        this->button_close.rect.x = this->dx() + this->cx() - this->button_close.cx() - 5;
        this->button_close.label.rect.x = this->button_close.dx() + 2;
    }

    virtual ~Window()
    {}

    virtual void notify(Widget* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (this->notifier) {
            if (widget == &this->button_close) {
                this->notifier->notify(this, NOTIFY_CANCEL, 0, 0);
            } else {
                this->WidgetComposite::notify(this, event, param, param2);
            }
        }
    }

    virtual void draw(const Rect& clip)
    {
        this->WidgetComposite::draw(clip);
        Rect new_clip = clip.intersect(this->rect);
        Region region;
        region.rects.push_back(new_clip);

        for (std::size_t i = 0, size = this->child_list.size(); i < size; ++i) {
            Rect rect = new_clip.intersect(this->child_list[i]->rect);

            if (!rect.isempty()) {
                region.subtract_rect(rect);
            }
        }

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable->draw(RDPOpaqueRect(region.rects[i], this->bg_color),
                                 region.rects[i]);
        }
    }
};

#endif