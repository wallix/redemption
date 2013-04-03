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

#if !defined(REDEMPTION_MOD_WIDGET2_BUTTON_HPP)
#define REDEMPTION_MOD_WIDGET2_BUTTON_HPP

#include "label.hpp"

class WidgetButton : public Widget
{
public:
    WidgetLabel label;
    int state;
    int border_top_right;
    int border_bottom_left;

    WidgetButton(ModApi* drawable, int16_t x, int16_t y, Widget* parent,
                 NotifyApi* notifier, const char * text, bool auto_resize = true,
                 int id = 0, int bgcolor = BLACK, int fgcolor = WHITE,
                 int xtext = 0, int ytext = 0)
    : Widget(drawable, Rect(x,y,1,1), parent, notifier, id)
    , label(drawable, 2, 2, this, 0, text, auto_resize, 0, bgcolor, fgcolor, xtext, ytext)
    , state(0)
    {
        this->rect.cx = this->label.cx() + 4;
        this->rect.cy = this->label.cy() + 4;
        const int c1 = (this->label.bg_color & 0xFF0000) >> 16;
        const int c2 = (this->label.bg_color & 0x00FF00) >> 8;
        const int c3 = (this->label.bg_color & 0x0000FF);
        this->border_top_right = (std::max(c1 - 0x35, 0) << 16) + (std::max(c2 - 0x35, 0) << 8) + std::max(c3 - 0x35, 0);
        this->border_bottom_left = (std::max(c1 - 0x70, 0) << 16) + (std::max(c2 - 0x70, 0) << 8) + std::max(c3 - 0x70, 0);
    }

    virtual ~WidgetButton()
    {}

    virtual void draw(const Rect& clip)
    {
        this->label.draw(clip);
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 2, this->dy(), this->cx() - 2, 2
        )), this->border_top_right), this->rect);
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 2, this->dy() + 2, 2, this->cy() - 2)
        ), this->border_top_right), this->rect);
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), 2, this->cy()
        )), this->border_bottom_left), this->rect);
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 2, this->dy() + this->cy() - 2, this->cx() - 4, 2
        )), this->border_bottom_left), this->rect);
    }

    void swap_border_color()
    {
        this->border_bottom_left ^= this->border_top_right;
        this->border_top_right ^= this->border_bottom_left;
        this->border_bottom_left ^= this->border_top_right;
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        switch (device_flags) {
            case CLIC_BUTTON1_DOWN:
                this->state = 1;
                this->swap_border_color();
                break;
            case CLIC_BUTTON1_UP:
                if (this->state & 1) {
                    this->state = 0;
                    this->swap_border_color();
                    this->send_notify(NOTIFY_SUBMIT);
                }
                break;
            case FOCUS_END:
                if (this->state & 1) {
                    this->state = 0;
                    this->swap_border_color();
                }
                break;
            default:
                this->Widget::rdp_input_mouse(device_flags, x, y, keymap);
                break;
        }
    }
};

#endif