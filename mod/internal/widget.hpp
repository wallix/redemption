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

#include "log.hpp"
#include "constants.hpp"
#include "rect.hpp"
#include "region.hpp"
#include "bitmap.hpp"
#include "client_mod.hpp"

#include "keymap.hpp"

#include <iostream>
#include <list>

struct Widget {
    class internal_mod * mod;
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


    Widget(internal_mod * mod, int width, int height, Widget & parent, int type);

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

    /******************************************************************/
    /* nil for rect means the whole thing */
    /* returns error */
    virtual int refresh(const Rect & clip);
    virtual int refresh_clip(const Rect & clip);

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

struct widget_button : public Widget
{
    widget_button(internal_mod * mod, const Rect & r, Widget & parent, int id, int tab_stop, const char * caption)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_BUTTON) {

        assert(type == WND_TYPE_BUTTON);

        this->rect.x = r.x;
        this->rect.y = r.y;
        this->tab_stop = tab_stop;
        this->id = id;
        this->caption1 = strdup(caption);
    }

    ~widget_button() {
        if (this->caption1){
            free(this->caption1);
            this->caption1 = 0;
        }
    }

    void draw(const Rect & clip);

    void draw_focus_rect(Widget * wdg, const Rect & r, const Rect & clip);

};

struct widget_edit : public Widget {

    char buffer[256];

    widget_edit(internal_mod * mod, const Rect & r, Widget & parent, int id, int tab_stop, const char * caption, int pointer, int edit_pos)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_EDIT) {

        assert(type == WND_TYPE_EDIT);

        this->rect.x = r.x;
        this->rect.y = r.y;
        this->tab_stop = tab_stop;
        this->id = id;
        this->buffer[0] = 0;
        if (caption){
            strncpy(this->buffer, caption, 255);
            this->buffer[255] = 0;
        }
        this->pointer = pointer;
        this->edit_pos = edit_pos;
        this->caption1 = 0;
    }

    ~widget_edit() {
    }

    virtual void draw(const Rect & clip);
    virtual void def_proc(const int msg, const int param1, const int param2, const Keymap * keymap);


};

struct window : public Widget
{
    window(internal_mod * mod, const Rect & r, Widget & parent, int bg_color, const char * title)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_WND) {

        assert(type == WND_TYPE_WND);

        this->bg_color = bg_color;
        this->rect.x = r.x;
        this->rect.y = r.y;
        this->caption1 = strdup(title);
    }

    /* find the window containing widget */
    virtual window * find_window()
    {
        return this;
    }

    ~window() {
        if (this->caption1){
            free(this->caption1);
            this->caption1 = 0;
        }
    }

    virtual void draw(const Rect & clip);
    virtual void def_proc(const int msg, const int param1, const int param2, const Keymap * keymap);
    virtual void focus(const Rect & clip);
    virtual void blur(const Rect & clip);


};


struct widget_screen : public Widget {
    uint8_t bpp;

    widget_screen(internal_mod * mod, int width, int height, uint8_t bpp)
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


struct widget_label : public Widget {

    widget_label(internal_mod * mod, const Rect & r, Widget & parent, const char * title)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_LABEL) {

        assert(type == WND_TYPE_LABEL);

        this->rect.x = r.x;
        this->rect.y = r.y;
        this->caption1 = strdup(title);
    }

    ~widget_label() {
        free(this->caption1);
    }

    virtual void draw(const Rect & clip);

};

struct widget_popup : public Widget
{

    widget_popup(internal_mod * mod, const Rect & r,
         Widget * popped_from,
         Widget & parent,
         int item_index)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_SPECIAL)
    {
            this->popped_from = popped_from;
            this->rect.x = r.x;
            this->rect.y = r.y;
            this->item_index = item_index;
    }
    ~widget_popup() {}

    virtual void draw(const Rect & clip);
    virtual void def_proc(const int msg, const int param1, const int param2, const Keymap * keymap);

};

struct widget_combo : public Widget
{
    widget_combo(internal_mod * mod, const Rect & r,
                Widget & parent, int id, int tab_stop)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_COMBO){
        this->rect.x = r.x;
        this->rect.y = r.y;
        this->id = id;
        this->tab_stop = tab_stop;
        this->item_index = 0;
    }

    ~widget_combo() {}

    virtual void draw(const Rect & clip);
    virtual void def_proc(const int msg, const int param1, const int param2, const Keymap * keymap);

};

struct widget_image : public Widget {
    Bitmap bmp;

    widget_image(internal_mod * mod, int width, int height, int type, Widget & parent, int x, int y, const char* filename, uint8_t bpp)
    : Widget(mod, width, height, parent, type), bmp(filename) {

        assert(type == WND_TYPE_IMAGE);

        this->rect = Rect(x, y, bmp.cx, bmp.cy);
    }

    ~widget_image() {
    }

    virtual void draw(const Rect & clip);

};

#endif
