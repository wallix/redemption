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

   widget, drawable
   this is an object that can be drawn on trough an internal_mod
   all windows, bitmaps, even the screen are of this type

*/

#include "widget.hpp"
#include "internal/internal_mod.hpp"

Widget::Widget(GraphicalContext * mod, int width, int height, Widget & parent, int type) : parent(parent) {
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
    /* for button or combo */
    this->state = 0; /* for button 0 = normal 1 = down */
    /* for popup */
    this->popped_from = 0;
    /* for combo or popup */
    this->item_index = 0;
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

Widget::~Widget(){
    if (this->type != WND_TYPE_SCREEN){
        vector<Widget*>::iterator it;
        for (it = this->child_list.begin(); it != this->child_list.end(); it++){
            if (*it == this){
                this->parent.child_list.erase(it);
                break;
            }
        }
    }
    if (this->caption1){
        free(this->caption1);
        this->caption1 = 0;
    }

}

void window::focus(const Rect & clip)
{
    this->has_focus = true;
}

void window::blur(const Rect & clip)
{
    this->has_focus = false;
}

/*****************************************************************************/
Widget* Widget::Widget_get_child_by_id(int id) {
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
int Widget::delete_all_childs()
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
struct Widget* Widget::widget_at_pos(int x, int y) {
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

    TODO(" we should be able to pass only one pointer  either window if we are dealing with a window or this->parent if we are dealing with any other kind of widget")
const Region Widget::get_visible_region(Widget * window, Widget * widget, const Rect & rect)
{
    Region region;
    region.rects.push_back(rect);
    /* loop through all windows in z order */
    for (size_t i = 0; i < this->mod->nb_windows(); i++) {
        Widget *p = this->mod->window(i);
        if (p == window || p == widget) {
            break;
        }
        region.subtract_rect(p->rect);
    }
    return region;
}

void window::draw(const Rect & clip)
{
    Rect r(0, 0, this->rect.cx, this->rect.cy);
    const Rect scr_r = this->to_screen_rect(r);
    const Region region = this->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.draw_window(scr_r,
            this->bg_color, this->caption1,
            this->has_focus,
            region_clip);
    }
}

void widget_edit::draw(const Rect & clip)
{

    Rect r(0, 0, this->rect.cx, this->rect.cy);
    const Rect scr_r = this->to_screen_rect(r);
    const Region region = this->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.draw_edit(scr_r,
            this->password_char,
            this->buffer,
            this->edit_pos,
            this->has_focus,
            region_clip);
    }
}

void widget_screen::draw(const Rect & clip)
{
    const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
    const Region region = this->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.draw(RDPOpaqueRect(scr_r, this->bg_color),
            region_clip);
    }
}

void widget_combo::draw(const Rect & clip)
{
    const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
    const Region region = this->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.draw_combo(scr_r,
            this->string_list[this->item_index],
            this->state,
            this->has_focus,
            region_clip);
    }
}

void widget_button::draw(const Rect & clip)
{
    Rect r(0, 0, this->rect.cx, this->rect.cy);

    const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
    const Region region = this->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.draw_button(scr_r,
            this->caption1,
            this->state,
            this->has_focus,
            region_clip);
    }
}


void widget_popup::draw(const Rect & clip)
{
    const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
    const Region region = this->get_visible_region(this, &this->parent, scr_r);
    TODO("Font Height is currently hardcoded to 16 in drop box")
    const int height = 16;

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.draw(
            RDPOpaqueRect(Rect(scr_r.x, scr_r.y, this->rect.cx, this->rect.cy), WHITE),
            region_clip);

        TODO(" this should be a two stages process  first prepare drop box data  then call draw_xxx that use that data to draw. For now everything is mixed up  (and that is not good)")
        /* draw the list items */
        if (this->popped_from != 0) {
            size_t list_count = this->popped_from->string_list.size();

            // draw the selected line
            this->mod->gd.draw(
                    RDPOpaqueRect(Rect(scr_r.x, scr_r.y + height * this->item_index, this->rect.cx, height), WABGREEN), region_clip);

            for (unsigned i = 0; i < list_count; i++) {
                const char * p = this->popped_from->string_list[i];
                this->mod->gd.server_draw_text(scr_r.x + 2, scr_r.y + i * height,
                    p,
                    (i == this->item_index)?WABGREEN:WHITE,
                    (i == this->item_index)?WHITE:BLACK,
                    region_clip);
            }
        }

    }
}

void Widget::draw(const Rect & clip)
{
}

void widget_label::draw(const Rect & clip)
{
    const Rect scr_r = this->to_screen_rect(Rect(0, 0, this->rect.cx, this->rect.cy));
    const Region region = this->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

        this->mod->gd.server_draw_text(scr_r.x, scr_r.y, this->caption1, GREY, BLACK,
            region_clip);
    }

}

// transform a rectangle relative to current widget to rectangle relative to screen
Rect const Widget::to_screen_rect(const Rect & r)
{
        Rect a = r;
        Widget *b = this;
        for (; WND_TYPE_SCREEN != b->type ; b = &b->parent) {
            a = a.offset(b->rect.x, b->rect.y);
        }
        return a.intersect(b->rect);
}

// get current widget rectangle relative to screen
Rect const Widget::to_screen_rect()
{
        Rect a = Rect(0, 0, this->rect.cx, this->rect.cy);
        Widget *b = this;
        for (; WND_TYPE_SCREEN != b->type ; b = &b->parent) {
            a = a.offset(b->rect.x, b->rect.y);
        }
        return a.intersect(b->rect);
}

void widget_image::draw(const Rect & clip)
{
    TODO("See why region clipping is not done the same way as everywhere else here")
    Rect image_screen_rect = this->to_screen_rect();
    Rect intersection = image_screen_rect.intersect(this->to_screen_rect(clip));
    const Region region = this->get_visible_region(this, &this->parent, intersection);

    for (size_t ir = 0; ir < region.rects.size(); ir++){
        this->mod->gd.draw(RDPMemBlt(0, image_screen_rect, 0xCC, 0, 0, 0), region.rects[ir], this->bmp);
    }
}

void Widget::refresh(const Rect & clip)
{
    this->mod->gd.front.begin_update();

    this->draw(clip);
    this->notify(this, WM_PAINT, 0, 0);

    size_t count = this->child_list.size();
    for (size_t i = 0; i < count; i++) {
        Widget * b = this->child_list[i];
        b->refresh(b->rect.wh());
    }
    this->mod->gd.server_end_update();
}

void Widget::notify(struct Widget* sender, int msg, long param1, long param2)
{
    this->parent.notify(sender, msg, param1, param2);
}

static inline bool switch_focus(Widget * old_focus, Widget * new_focus) {
    bool res = false;
    if (new_focus->tab_stop){
        if (old_focus) {
            old_focus->has_focus = (old_focus == new_focus);
            old_focus->refresh(old_focus->rect.wh());
        }
        if (old_focus != new_focus){
            new_focus->has_focus = true;
            new_focus->refresh(new_focus->rect.wh());
        }
        res = true;
    }
    return res;
}

void window::def_proc(const int msg, const int param1, const int param2, const Keymap * keymap)
{
    if (msg == WM_KEYDOWN) {

        Widget * control_with_focus = this->default_button;
        // find control that has focus
        size_t size = this->child_list.size();
        size_t i_focus;
        TODO(" we should iterate only on controls that have tabstop setted (or another attribute can_get_focus ?). Or we could also keep index of focused_control in child_list (but do not forget to reset it when we redefine controls).")
        for (i_focus = 0; i_focus < size; i_focus++){
            if (this->child_list[i_focus]->has_focus && this->child_list[i_focus]->tab_stop){
                control_with_focus = this->child_list[i_focus];
                break;
            }
        }

        int scan_code = param1 & 0x7F;
        switch (scan_code){
        case 15:
        { /* tab */
            /* move to next tab stop */
            int shift = keymap->keys[42] || keymap->keys[54];
            // find the next tab_stop
            if (shift) {
                for (size_t i = (size+i_focus-1) % size ; i != i_focus ; i = (i+size-1) % size) {
                    Widget * new_focus = this->child_list[i];
                    if (switch_focus(control_with_focus, new_focus)) {
                        break;
                    }
                }
            } else {
                for (size_t i = (size+i_focus+1) % size ; i != i_focus ; i = (i+size+1) % size) {
                    Widget * new_focus = this->child_list[i];
                    if (switch_focus(control_with_focus, new_focus)) {
                        break;
                    }
                }
            }
        }
        break;
        case 28: /* enter */
            this->notify(this->default_button, 1, 0, 0);
        return;
        case 1: /* esc */
            if (this->esc_button) {
                this->notify(this->esc_button, 1, 0, 0);
            }
        break;
        default:
            if (control_with_focus){
                control_with_focus->def_proc(msg, param1, param2, keymap);
            }
        }
    }
}

void widget_edit::def_proc(const int msg, int const param1, int const param2, const Keymap * keymap)
{
    wchar_t c;
    int n;
    int ext;
    int scan_code;
    int num_bytes;
    int num_chars;

    if (msg == WM_KEYDOWN) {
        scan_code = param1 % 128;
        ext = param2 & 0x0100;
        /* left or up arrow */
        if ((scan_code == 75 || scan_code == 72)
        && (ext || keymap->key_flags & 5)) // numlock = 0
        {
            if (this->edit_pos > 0) {
                this->edit_pos--;
                this->refresh(this->rect.wh());
            }
        }
        /* right or down arrow */
        else if ((scan_code == 77 || scan_code == 80)
        && (ext || keymap->key_flags & 5)) // numlock = 0
        {
            if (this->edit_pos < (int)mbstowcs(0, this->buffer, 0)) {
                this->edit_pos++;
                this->refresh(this->rect.wh());
            }
        }
        /* backspace */
        else if (scan_code == 14) {

            n = mbstowcs(0, this->buffer, 0);
            if (n > 0) {
                if (this->edit_pos > 0) {
                    this->edit_pos--;
                    remove_char_at(this->buffer, 255, this->edit_pos);
                    this->refresh(this->rect.wh());
                }
            }
        }
        /* delete */
        else if (scan_code == 83  && (ext || keymap->key_flags & 5)) // numlock = 0
        {
            n = mbstowcs(0, this->buffer, 0);
            if (n > 0) {
                if (this->edit_pos < n) {
                    remove_char_at(this->buffer, 255, this->edit_pos);
                    this->refresh(this->rect.wh());
                }
            }
        }
        /* end */
        else if (scan_code == 79  && (ext || keymap->key_flags & 5)) {
            n = mbstowcs(0, this->buffer, 0);
            if (this->edit_pos < n) {
                this->edit_pos = n;
                this->refresh(this->rect.wh());
            }
        }
        /* home */
        else if ((scan_code == 71)  &&
                 (ext || (keymap->key_flags & 5))) {
            if (this->edit_pos > 0) {
                this->edit_pos = 0;
                this->refresh(this->rect.wh());
            }
        }
        else {
            c = (wchar_t)(keymap->get_key_info_from_scan_code(param2, scan_code)->chr);
            num_chars = mbstowcs(0, this->buffer, 0);
            num_bytes = strlen(this->buffer);

            if ((c >= 32) && (num_chars < 127) && (num_bytes < 250)) {
                char text[256];
                strncpy(text, this->buffer, 255);

                int index = this->edit_pos;
                TODO(" why not always keep wcs instead of constantly converting back and from wcs ?")
                int len = mbstowcs(0, text, 0);
                wchar_t wstr[len + 16];
                mbstowcs(wstr, text, len + 1);
                if ((this->edit_pos >= len) || (this->edit_pos < 0)) {
                    wstr[len] = c;
                }
                else{
                TODO(" is backward loop necessary ? a memcpy could do the trick")
                    int i;
                    for (i = (len - 1); i >= index; i--) {
                        wstr[i + 1] = wstr[i];
                    }
                    wstr[i + 1] = c;
                }
                wstr[len + 1] = 0;
                wcstombs(text, wstr, 255);
                this->edit_pos++;
                strncpy(this->buffer, text, 255);
                this->buffer[255] = 0;
                this->refresh(this->rect.wh());
            }

        }
    }
}

void widget_combo::def_proc(const int msg, const int param1, const int param2, const Keymap * keymap)
{
    int ext;
    int scan_code;

    if (msg == WM_KEYDOWN) {
        scan_code = param1 % 128;
        ext = param2 & 0x0100;
        /* left or up arrow */
        if (((scan_code == 75) || (scan_code == 72)) && (ext || (keymap->key_flags & 5))) {
            if (this->item_index > 0) {
                this->item_index--;
                this->refresh(this->rect.wh());
                this->notify(this, CB_ITEMCHANGE, 0, 0);
            }
        }
        /* right or down arrow */
        else if ((scan_code == 77 || scan_code == 80) && (ext || (keymap->key_flags & 5))) {
                    size_t count = this->string_list.size();
            if ((this->item_index + 1) < count) {
                this->item_index++;
                this->refresh(this->rect.wh());
                this->notify(this, CB_ITEMCHANGE, 0, 0);
            }
        }
    }
}


void widget_popup::def_proc(const int msg, const int param1, const int param2, const Keymap * keymap)
{
    if (msg == WM_MOUSEMOVE) {
        if (this->popped_from != 0) {
            TODO("The size of a line in drop box is currently hardcoded (16). We should get this size from font information. But the way it was done previously is not sufficient. We should be able to have lines of different sizes in drop box and such")
            unsigned i = param2 / 16;
            if (i != this->item_index && i < this->popped_from->string_list.size())
            {
                this->item_index = i;
                this->refresh(this->rect.wh());
            }
        }
    } else if (msg == WM_LBUTTONUP) {

        if (this->popped_from != 0) {
            this->popped_from->item_index = this->item_index;
            this->popped_from->refresh(this->popped_from->rect.wh());
            this->popped_from->notify(this->popped_from, CB_ITEMCHANGE, 0, 0);
        }
    }
}

void Widget::def_proc(const int msg, const int param1, const int param2, const Keymap * keymap)
{
}
