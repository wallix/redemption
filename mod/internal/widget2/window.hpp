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

class Window : public WidgetComposite
{
public:
    WidgetLabel titlebar;
    WidgetButton button_close;
    int bg_color;
    int active_border_color;
    int inactive_border_color;

    Window(ModApi* drawable, const Rect& rect, Widget2* parent, NotifyApi* notifier,
           const char * caption, int bgcolor = DARK_WABGREEN, int group_id = 0)
    : WidgetComposite(drawable, rect, parent, notifier, group_id)
    , titlebar(drawable, 2, 2, this, NULL, caption, false, -1, BLACK, WABGREEN, 5)
    , button_close(drawable, 2, 2, this, this, "X", true, -2, WHITE, DARK_GREEN, 0, -1, NOTIFY_CANCEL)
    , bg_color(bgcolor)
    , active_border_color(0xCCCCCC)
    , inactive_border_color(0x888888)
    {
        this->child_list.push_back(&this->titlebar);
        this->child_list.push_back(&this->button_close);

        this->titlebar.tab_flag = IGNORE_TAB;
        this->button_close.tab_flag = IGNORE_TAB;

        this->tab_flag |= NO_DELEGATE_PARENT;

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
        this->titlebar.rect.cx = this->cx() - this->button_close.cx() - 4;
        this->button_close.set_button_x(this->dx() + this->cx() - this->button_close.cx() - 2);
    }

    virtual ~Window()
    {}

    virtual void draw(const Rect& clip)
    {
        Rect inner_window = clip.intersect(this->rect.shrink(2));
        this->WidgetComposite::draw(inner_window);
        this->WidgetComposite::draw_inner_free(inner_window, this->bg_color);

        this->draw_border(clip, this->has_focus ? this->active_border_color : this->inactive_border_color);
    }

    void draw_border(const Rect& clip, int border_color)
    {
        //top
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx(), 1
        )), border_color), this->rect);
        //left
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + 1, 1, this->cy() - 2
        )), border_color), this->rect);
        //right
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy() + 1, 1, this->cy() - 2
        )), border_color), this->rect);
        //bottom
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), border_color), this->rect);

        //top
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + 1, this->cx() - 2, 1
        )), this->inactive_border_color), this->rect);
        //left
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + 2, 1, this->cy() - 4
        )), this->inactive_border_color), this->rect);
        //right
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 2, this->dy() + 2, 1, this->cy() - 4
        )), this->inactive_border_color), this->rect);
        //bottom
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + this->cy() - 2, this->cx() - 2, 1
        )), this->inactive_border_color), this->rect);
    }

    virtual bool focus(Widget2* old_focused)
    {
        this->draw_border(this->rect, this->active_border_color);
        return WidgetComposite::focus(old_focused);
    }

    virtual void blur()
    {
        this->draw_border(this->rect, this->inactive_border_color);
        WidgetComposite::blur();
    }
};

#endif