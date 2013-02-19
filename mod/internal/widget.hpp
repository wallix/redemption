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

*/

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_HPP_

#include <stdlib.h>

#include "log.hpp"
#include "constants.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "bitmap.hpp"
#include "client_mod.hpp"
#include "region.hpp"
#include "keymap2.hpp"

enum {
    WM_PAINT       = 3,
    WM_KEYDOWN     = 15,
    WM_KEYUP       = 16,
    WM_SYNCHRONIZE = 17,
    WM_MOUSEMOVE   = 100,
    WM_LBUTTONUP   = 101,
    WM_LBUTTONDOWN = 102,
    WM_RBUTTONUP   = 103,
    WM_RBUTTONDOWN = 104,
    WM_BUTTON3UP   = 105,
    WM_BUTTON3DOWN = 106,
    WM_BUTTON4UP   = 107,
    WM_BUTTON4DOWN = 108,
    WM_BUTTON5UP   = 109,
    WM_BUTTON5DOWN = 110,
    WM_BUTTON_OK = 300,
    WM_SCREENUPDATE = 0x4444,
    WM_CHANNELDATA = 0x5555,
};

enum {
    NOTIFY_MOUSE_MOVE,
    NOTIFY_MOUSE_DOWN,
    NOTIFY_MOUSE_UP,
    NOTIFY_CLICK,
};


/* drawable types */
enum {
    WND_TYPE_BITMAP  = 0,
    WND_TYPE_WND     = 1,
    WND_TYPE_SCREEN  = 2,
    WND_TYPE_BUTTON  = 3,
    WND_TYPE_IMAGE   = 4,
    WND_TYPE_EDIT    = 5,
    WND_TYPE_LABEL   = 6,
};


struct Widget {
    struct internal_mod * mod;
    bool has_focus;

    /* 0 = bitmap 1 = window 2 = screen 3 = button 4 = image 5 = edit
       6 = label 7 = combo 8 = special */
    int type;

    Rect rect;

    /* msg 1 = click 2 = mouse move 3 = paint 100 = modal result */
    /* see messages in constants.h */

    /* for all but bitmap */
    int pointer;
    int bg_color;
    int tab_stop;
    int id;
    char* caption1;
    /* for window or screen */
    struct Widget* modal_dialog;
    struct Widget* focused_control;
    struct Widget * parent; /* widget container, NULL for screen */
    /* for modal dialog */
    struct Widget* default_button; /* button when enter is pressed */
    struct Widget* esc_button; /* button when esc is pressed */
    /* list of child windows */
    vector<Widget*> child_list;
    /* for edit */
    int edit_pos;
    wchar_t password_char;
    /* for button or combo */
    int state; /* for button 0 = normal 1 = down */
    int item_height;
    /* crc */
    int crc;

    public:


    Widget(struct internal_mod * mod, int width, int height, Widget * parent, int type) : parent(parent) {
        this->mod = mod;
        /* for all but bitmap */
        this->pointer = 0;
        this->bg_color = 0;
        this->tab_stop = 0;
        this->id = 0;
        this->caption1 = 0;
        /* for window or screen */
        this->modal_dialog = 0;
        this->focused_control = 0;
        /* for modal dialog */
        this->default_button = 0; /* button when enter is pressed */
        this->esc_button = 0; /* button when esc is pressed */
        /* for edit */
        this->edit_pos = 0;
        this->password_char = 0;
        /* crc */
        this->crc = 0;

        this->has_focus = false;

    TODO(" build the right type of bitmap = class hierarchy")
        /* 0 = bitmap 1 = window 2 = screen 3 = button 4 = image 5 = edit
           6 = label 7 = combo 8 = special */
        this->type = type;
        this->rect.x = 0;
        this->rect.y = 0;
        this->rect.cx = width;
        this->rect.cy = height;
    }

    virtual ~Widget(){
        if (this->type != WND_TYPE_SCREEN){
            vector<Widget*>::iterator it;
            for (it = this->child_list.begin(); it != this->child_list.end(); it++){
                if (*it == this){
                    this->parent->child_list.erase(it);
                    break;
                }
            }
        }
    }

    /*****************************************************************************/
    Widget* Widget_get_child_by_id(int id) {
        for (size_t i = 0; i < this->child_list.size(); i++) {
            struct Widget * b = this->child_list[i];
            if (b->id == id) {
                return b;
            }
        }
        return 0;
    }


    /*****************************************************************************/
    // called for screen
    int delete_all_childs()
    {
        {
            size_t index = this->child_list.size();
            while (index > 0) {
                index--;
                this->child_list[index]->refresh(this->child_list[index]->rect.wh());
            }
        }
        {
            size_t index = this->child_list.size();
            while (index > 0) {
                index--;
                delete this->child_list[index];
            }
        }
        this->child_list.clear();
        return 0;
    }

    /*****************************************************************************/
    /* return the window at x, y on the screen */
    /* coordinates are given relative to the container of this */
    struct Widget* widget_at_pos(int x, int y) {
        x -= this->rect.x;
        y -= this->rect.y;
        /* loop through all windows. */
        /* If a widget contains overlapping subwidgets */
        /* consider the right one is the first one found in child_list */
        for (size_t i = 0; i < this->child_list.size(); i++) {
            if (this->child_list[i]->rect.contains_pt(x, y)) {
                Widget * res =  this->child_list[i]->widget_at_pos(x, y);
                return res;
            }
        }
        return this;
    }


    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (this->type != WND_TYPE_SCREEN){
            this->parent->notify(sender, msg, param1, param2);
        }
    }

    virtual void focus()
    {
    }

    virtual void blur()
    {
    }

    virtual void refresh(const Rect & clip)
    {
        this->draw(clip);
        this->notify(this, WM_PAINT, 0, 0);

        size_t count = this->child_list.size();
        for (size_t i = 0; i < count; i++) {
            Widget * b = this->child_list[i];
            b->refresh(b->rect.wh());
        }
    }

    virtual void def_proc(const int msg, const int param1, const int param2, Keymap2 * keymap)
    {
    }

    virtual void draw(const Rect & clip)
    {
    }

    // transform a rectangle relative to current widget to rectangle relative to screen
    Rect const to_screen_rect(const Rect & r)
    {
            Rect a = r;
            Widget *b = this;
            for (; WND_TYPE_SCREEN != b->type ; b = b->parent) {
                a = a.offset(b->rect.x, b->rect.y);
            }
            return a.intersect(b->rect);
    }

    // get current widget rectangle relative to screen
    Rect const to_screen_rect()
    {
            Rect a = Rect(0, 0, this->rect.cx, this->rect.cy);
            Widget *b = this;
            for (; WND_TYPE_SCREEN != b->type ; b = b->parent) {
                a = a.offset(b->rect.x, b->rect.y);
            }
            return a.intersect(b->rect);
    }

    /* convert the controls coords to screen coords */
    int to_screenx()
    {
        int x = 0;
        for (Widget *b = this; WND_TYPE_SCREEN != b->type ; b = b->parent) {
             x += b->rect.x;
        }
        return x;
    }

    /*****************************************************************************/
    /* convert the controls coords to screen coords */
    int to_screeny()
    {
        int y = 0;
        for (Widget *b = this; WND_TYPE_SCREEN != b->type ; b = b->parent) {
             y += b->rect.y;
        }
        return y;
    }

    /*****************************************************************************/
    /* convert the screen coords to controls coords */
    int from_screenx(int x)
    {
        for (Widget *b = this ; WND_TYPE_SCREEN != b->type ; b = b->parent){
            x -= b->rect.x;
        }
        return x;
    }

    /*****************************************************************************/
    /* convert the screen coords to controls coords */
    int from_screeny(int y)
    {
        for (Widget *b = this ; WND_TYPE_SCREEN != b->type ; b = b->parent){
            y -= b->rect.y;
        }
        return y;
    }

    /*****************************************************************************/
    /* find the window containing widget or the screen */
    virtual Widget * find_window()
    {
        return (this->type == WND_TYPE_SCREEN)?this:this->parent->find_window();
    }

        TODO(" we should be able to pass only one pointer  either window if we are dealing with a window or this->parent if we are dealing with any other kind of widget")
    // find out the visible part of a widget
    // the visible part of a widget is a list of rectangles
    // expressed in absolute (screen) coordinates
    // Rules are:
    // 1 - widget is clipped by it's parent (and so on until screen
    // if the parent itself is not fully visible)
    // 2 - offsprings of a widget overlap it
    // 3 - siblings overlap if they are youger (after it in the child list of parent)
    // It is obvious that Rule 1 yield only one rectangle while other rules yields true regions
    const Region get_visible_region(Widget * screen, Widget * window, Widget * widget, const Rect & rect)
    {
        Region region;
        region.rects.push_back(rect);
        /* loop through all windows in z order */
        for (size_t i = 0; i < screen->child_list.size(); i++) {
            Widget *p = screen->child_list[i];
            if (p == window || p == widget) {
                break;
            }
            region.subtract_rect(p->rect);
        }
        return region;
    }
};


#endif
