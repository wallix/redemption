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

#if !defined(REDEMPTION_MOD_WIDGET2_PAGER_HPP)
#define REDEMPTION_MOD_WIDGET2_PAGER_HPP

#include <sstream>
#include "window.hpp"
#include "button.hpp"
#include "label.hpp"
#include "edit.hpp"

class WidgetPager : public Window
{
public:
    WidgetButton first;
    WidgetButton prev;
    WidgetEdit current;
    WidgetLabel nb_page;
    WidgetButton next;
    WidgetButton last;

    enum {
        NOTIFY_FOCUS_BEGIN = ::NOTIFY_FOCUS_BEGIN,
        NOTIFY_FOCUS_END = ::NOTIFY_FOCUS_END,
        //NOTIFY_SUBMIT = ::NOTIFY_SUBMIT,
        //NOTIFY_CANCEL = ::NOTIFY_CANCEL,
        NOTIFY_PREV_SUBMIT = 100,
        NOTIFY_CURRENT_SUBMIT = 101,
        NOTIFY_NEXT_SUBMIT = 102
    };

    WidgetPager(ModApi* drawable, const Rect& rect, Widget* parent,
                NotifyApi* notifier, const char* caption, uint page, int id = 0,
                bool bottom = true,
                const char * text_current = "1",
                const char * text_prev = "<",
                const char * text_next = ">",
                const char * text_first = "<<",
                const char * text_last = ">>")
    : Window(drawable, rect, parent, notifier, caption, id)
    , first(drawable, Rect(), this, 0, text_first, -2, 2, 2)
    , prev(drawable, Rect(), this, 0, text_prev, -3, 2, 2)
    , current(drawable, Rect(), this, 0, text_current, 0, -4, 2, 2)
    , nb_page(drawable, Rect(), this, 0, 0, -5, 2, 2)
    , next(drawable, Rect(), this, 0, text_next, -6, 2, 2)
    , last(drawable, Rect(), this, 0, text_last, -7, 2, 2)
    {
        if (page != -1u)
        {
            std::ostringstream os;
            os << '/' << page;
            this->nb_page.set_text(os.str().c_str());
        }
        uint16_t ymax = std::max(
            this->first.label.context_text->cy,
            std::max(
                this->prev.label.context_text->cy,
                std::max(
                    this->current.label.context_text->cy,
                    std::max(
                        this->next.label.context_text->cy,
                        this->last.label.context_text->cy
                    )
                )
            )
        ) + 4;
        if (this->nb_page.context_text && this->nb_page.context_text->cy > ymax) {
            ymax = this->nb_page.context_text->cy;
        }
        int y = caption ? this->titlebar.rect.cy + 2 : 2;
        if (bottom) {
            y = rect.cy - y - ymax;
        }
        int x = this->rect.cx - (this->last.label.context_text->cx + 6);
        this->last.rect = Rect(x, y, this->last.label.context_text->cx + 4, ymax);
        x -= this->next.label.context_text->cx + 6;
        this->next.rect = Rect(x, y, this->next.label.context_text->cx + 4, ymax);
        if (this->nb_page.context_text) {
            x -= this->nb_page.context_text->cx + 6;
            this->nb_page.rect = Rect(x, y, this->nb_page.context_text->cx + 4, ymax);
        }
        x -= this->current.label.context_text->cx + 6;
        this->current.rect = Rect(x, y, this->current.label.context_text->cx + 4, ymax);
        x -= this->prev.label.context_text->cx + 6;
        this->prev.rect = Rect(x, y, this->prev.label.context_text->cx + 4, ymax);
        x -= this->first.label.context_text->cx + 6;
        this->first.rect = Rect(x, y, this->first.label.context_text->cx + 4, ymax);
    }

    virtual ~WidgetPager()
    {}

    //void update_current_page()
    //{
    //    if (this->current.label.context_text->cx + 4 > this->current.rect.cx) {
    //        this->current.rect.x = this->rect.cx - (this->next.label.context_text->cx + this->nb_page.context_text->cx + this->current.label.context_text->cx + 6*3);
    //        this->current.rect.cx = this->current.label.context_text->cx + 4;
    //        this->prev.rect.x = this->current.rect.x - (this->prev.label.context_text->cx + 6);
    //    }
    //}

    virtual void notify(int id, EventType event)
    {
        if (event == WIDGET_SUBMIT
        && (id == this->prev.id
        || id == this->current.id
        || id == this->next.id)) {
            this->notify_self(NOTIFY_PREV_SUBMIT - 1 - id);
        } else {
            this->Window::notify(id, event);
        }
    }
};

#endif