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

    Window(ModApi* drawable, const Rect& rect, Widget2* parent, NotifyApi* notifier,
           const char * caption, int bgcolor = DARK_WABGREEN, int group_id = 0)
    : WidgetComposite(drawable, rect, parent, notifier, group_id)
    , titlebar(drawable, 0, 0, this, NULL, caption, false, -1, BLACK, WABGREEN, 5)
    , button_close(drawable, 0, 0, this, this, "X", true, -2, WHITE, DARK_GREEN, 0, -1)
    , bg_color(bgcolor)
    {
        this->child_list.push_back(&this->titlebar);
        this->child_list.push_back(&this->button_close);

        this->button_close.label.x_text = 3;
        this->button_close.set_button_cx(this->button_close.cx() * 2);
        this->button_close.set_button_cy(this->button_close.cy() - 2);

        if (this->drawable) {
            int w,h;
            this->drawable->text_metrics(this->titlebar.buffer, w,h);
            this->titlebar.rect.cy = std::max<int>(h - 2, this->button_close.cy()) + this->titlebar.y_text * 2;
        }
    }

    void resize_titlebar()
    {
        this->titlebar.rect.cx = this->cx() - this->button_close.cx();
        this->button_close.set_button_x(this->dx() + this->cx() - this->button_close.cx());
    }

    virtual ~Window()
    {}

    virtual void notify(Widget2* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (this->notifier) {
            if (widget == &this->button_close && event == NOTIFY_SUBMIT) {
                this->send_notify(NOTIFY_CANCEL);
            } else {
                this->WidgetComposite::notify(widget, event, param, param2);
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