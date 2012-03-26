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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni

*/

#if !defined(__MOD_INTERNAL_WIDGET_IMPLEMENTATION__)
#define __MOD_INTERNAL_WIDGET_IMPLEMENTATION__

#include "widget.hpp"
#include "internal/internal_mod.hpp"

struct widget_button : public Widget
{
    widget_button(GraphicalContext * mod, const Rect & r, Widget & parent, int id, int tab_stop, const char * caption)
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

    widget_edit(GraphicalContext * mod, const Rect & r, Widget & parent, int id, int tab_stop, const char * caption, int pointer, int edit_pos)
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
    window(GraphicalContext * mod, const Rect & r, Widget & parent, int bg_color, const char * title)
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


struct widget_label : public Widget {

    widget_label(GraphicalContext * mod, const Rect & r, Widget & parent, const char * title)
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

    widget_popup(GraphicalContext * mod, const Rect & r,
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
    widget_combo(GraphicalContext * mod, const Rect & r,
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

    widget_image(GraphicalContext * mod, int width, int height, int type, Widget & parent, int x, int y, const char* filename, uint8_t bpp)
    : Widget(mod, width, height, parent, type), bmp(24, filename) {

        assert(type == WND_TYPE_IMAGE);

        this->rect = Rect(x, y, bmp.cx, bmp.cy);
    }

    ~widget_image() {
    }

    virtual void draw(const Rect & clip);

};

#endif
