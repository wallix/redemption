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

#if !defined(__WIDGET__)
#define __WIDGET__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <list>

#include "log.hpp"
#include "constants.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "bitmap.hpp"
#include "client_mod.hpp"
#include "region.hpp"
#include "keymap.hpp"
//#include "internal/internal_mod.hpp"

#include "keymap.hpp"


/*****************************************************************************/
/* remove a ch at index position in text, index starts at 0 */
/* if index = -1 remove it from the end */
static inline void remove_char_at(char* text, int text_size, int index)
{
    int len = mbstowcs(0, text, 0);
    if (len <= 0) {
        return;
    }
    wchar_t wstr[len + 16];
    mbstowcs(wstr, text, len + 1);
    if ((index < (len - 1)) && (index >= 0)) {
        for (int i = index; i < (len - 1); i++) {
            wstr[i] = wstr[i + 1];
        }
    }
    wstr[len - 1] = 0;
    wcstombs(text, wstr, text_size);
}

#define GraphicalContext internal_mod

struct Widget {
    struct GraphicalContext * mod;
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
    struct Widget & parent; /* window contained in */
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
    /* for combo */
    vector<char*> string_list;
    /* for combo or popup */
    unsigned item_index;
    /* for popup */
    struct Widget* popped_from;
    int item_height;
    /* crc */
    int crc;

    public:


    Widget(GraphicalContext * mod, int width, int height, Widget & parent, int type);

    ~Widget();

    Widget* Widget_get_child_by_id(int id);

    int delete_all_childs();

    struct Widget* widget_at_pos(int x, int y);

    void Widget_set_focus(int focused);

    virtual void notify(struct Widget* sender, int msg, long param1, long param2);
    virtual void focus()
    {
    }
    virtual void blur()
    {
    }

    void draw_title_bar(int bg_color, int fg_color, const Rect & clip);

    /* returns true if they are the same, else returns false */
    int Widget_compare(struct Widget* other_bitmap);

    /******************************************************************/
    /* returns true if they are the same, else returns false */
    int Widget_compare_with_crc(struct Widget* other_bitmap);

    virtual void refresh(const Rect & clip);

    virtual void def_proc(const int msg, const int param1, const int param2, const Keymap * keymap);
    virtual void draw(const Rect & clip);

    Rect const to_screen_rect(const Rect & r);
    Rect const to_screen_rect();

    /* convert the controls coords to screen coords */
    int to_screenx()
    {
        int x = 0;
        for (Widget *b = this; WND_TYPE_SCREEN != b->type ; b = &b->parent) {
             x += b->rect.x;
        }
        return x;
    }

    /*****************************************************************************/
    /* convert the controls coords to screen coords */
    int to_screeny()
    {
        int y = 0;
        for (Widget *b = this; WND_TYPE_SCREEN != b->type ; b = &b->parent) {
             y += b->rect.y;
        }
        return y;
    }

    /*****************************************************************************/
    /* convert the screen coords to controls coords */
    int from_screenx(int x)
    {
        for (Widget *b = this ; WND_TYPE_SCREEN != b->type ; b = &b->parent){
            x -= b->rect.x;
        }
        return x;
    }

    /*****************************************************************************/
    /* convert the screen coords to controls coords */
    int from_screeny(int y)
    {
        for (Widget *b = this ; WND_TYPE_SCREEN != b->type ; b = &b->parent){
            y -= b->rect.y;
        }
        return y;
    }

    /*****************************************************************************/
    /* find the window containing widget or the screen */
    virtual Widget * find_window()
    {
        return this->parent.find_window();
    }

    const Region get_visible_region(Widget * window, Widget * widget, const Rect & rect);
};

struct widget_screen : public Widget {
    uint8_t bpp;

    widget_screen(GraphicalContext * mod, int width, int height, uint8_t bpp)
    : Widget(mod, width, height, *this, WND_TYPE_SCREEN), bpp(bpp) {
        assert(type == WND_TYPE_SCREEN);
    }

    ~widget_screen() {
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    { // notify for screen does nothing
        return;
    }

    /*****************************************************************************/
    /* find the window containing widget */
    virtual widget_screen * find_window()
    {
        return this;
    }

    virtual void draw(const Rect & clip);

};

#endif
