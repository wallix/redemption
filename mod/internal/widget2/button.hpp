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

class WidgetButton : public Widget2
{
public:
    WidgetLabel label;
    int state;
    int color_border_right_bottom;
    int color_border_right_bottom2;
    int color_border_left_top;

    WidgetButton(ModApi* drawable, int16_t x, int16_t y, Widget2* parent,
                 NotifyApi* notifier, const char * text, bool auto_resize = true,
                 int id = 0, int bgcolor = BLACK, int fgcolor = WHITE,
                 int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, id)
    , label(drawable, 1, 1, this, 0, text, auto_resize, 0, bgcolor, fgcolor, xtext, ytext)
    , state(0)
    {
        this->rect.cx = this->label.cx() + 3;
        this->rect.cy = this->label.cy() + 3;
        this->color_border_right_bottom = BLACK;
        this->color_border_left_top = WHITE;
        this->color_border_right_bottom2 = 0x888888;
    }

    virtual ~WidgetButton()
    {}

    void set_button_x(int x)
    {
        this->rect.x = x;
        this->label.rect.x = x + 1;
    }

    void set_button_y(int y)
    {
        this->rect.y = y;
        this->label.rect.y = y + 1;
    }

    virtual void draw(const Rect& clip)
    {
        if (this->state == 1) {
            ++this->label.rect.x;
            ++this->label.rect.y;
            --this->label.rect.cx;
            --this->label.rect.cy;
            this->label.draw(clip);
            --this->label.rect.x;
            --this->label.rect.y;
            ++this->label.rect.cx;
            ++this->label.rect.cy;
            //top
            this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy(), this->cx() - 1, 1
            )), this->color_border_right_bottom), this->rect);
            this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx() + 1, this->dy() + 1, this->cx() - 3, 1
            )), this->color_border_right_bottom2), this->rect);
            //left
            this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy() + 1, 1, this->cy() - 2
            )), this->color_border_right_bottom), this->rect);
            this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx() + 1, this->dy() + 1, 1, this->cy() - 3
            )), this->color_border_right_bottom2), this->rect);
        }
        else {
            this->label.draw(clip);
            //top
            this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy(), this->cx() - 1, 1
            )), this->color_border_left_top), this->rect);
            //left
            this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
                this->dx(), this->dy() + 1, 1, this->cy() - 2
            )), this->color_border_left_top), this->rect);
        }
        //right
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy(), 1, this->cy()
        )), this->color_border_right_bottom), this->rect);
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 2, this->dy() + 1, 1, this->cy() - 2
        )), this->color_border_right_bottom2), this->rect);
        //bottom
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), this->color_border_right_bottom), this->rect);
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + this->cy() - 2, this->cx() - 2, 1
        )), this->color_border_right_bottom2), this->rect);
    }

    void swap_border_color()
    {
        this->color_border_left_top ^= this->color_border_right_bottom;
        this->color_border_right_bottom ^= this->color_border_left_top;
        this->color_border_left_top ^= this->color_border_right_bottom;
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
                this->Widget2::rdp_input_mouse(device_flags, x, y, keymap);
                break;
        }
    }
};

#endif