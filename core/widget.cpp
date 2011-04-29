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
   this is an object that can be drawn on trough a client_mod
   all windows, bitmaps, even the screen are of this type

*/

#include "widget.hpp"
#include "region.hpp"
#include "keymap.hpp"
#include <string.h>
#include "file_loc.hpp"
#include "constants.hpp"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "client_mod.hpp"


/*****************************************************************************/
/* remove a ch at index position in text, index starts at 0 */
/* if index = -1 remove it from the end */
static void remove_char_at(char* text, int text_size, int index)
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

static void wchar_repeat(wchar_t* dest, int dest_size_in_wchars, wchar_t ch, int repeat)
{
    for (int index = 0; index < repeat; index++) {
        if (index >= dest_size_in_wchars) {
            break;
        }
        dest[index] = ch;
    }
}


Widget::Widget(client_mod * mod, int width, int height, Widget & parent, int type) : parent(parent) {
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
    this->item_height = 0;
    /* for combo or popup */
    this->item_index = 0;
    /* crc */
    this->crc = 0;

    this->has_focus = false;

#warning build the right type of bitmap = class hierarchy
    /* 0 = bitmap 1 = window 2 = screen 3 = button 4 = image 5 = edit
       6 = label 7 = combo 8 = special */
    this->type = type;
    this->rect.x = 0;
    this->rect.y = 0;
    this->rect.cx = width;
    this->rect.cy = height;
}

Widget::~Widget(){
    if (this != &this->mod->screen){
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

void Widget::draw_title_bar(int bg_color, int fg_color, const Rect & clip)
{
    assert(this->type == WND_TYPE_WND);
    this->mod->server_begin_update();
    this->fill_rect(0xCC, Rect(3, 3, this->rect.cx - 5, 18), bg_color, clip);
    this->mod->server_draw_text(this, 4, 4, this->caption1, fg_color, clip);
    this->mod->server_end_update();
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
            this->child_list[index]->Widget_invalidate(this->child_list[index]->rect.wh());
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
        if (this->child_list[i]->rect.rect_contains_pt(x, y)) {
            Widget * res =  this->child_list[i]->widget_at_pos(x, y);
            return res;
        }
    }
    return this;
}

void window::draw(const Rect & clip)
{
    /* draw grey background */

    this->fill_rect(0xCC, Rect(0, 0, this->rect.cx, this->rect.cy), this->bg_color, clip);
    /* top white line */

    this->fill_rect(0xCC, Rect(1, 1, this->rect.cx - 2, 1), WHITE, clip);

    /* left white line */
    this->fill_rect(0xCC, Rect(1, 1, 1, this->rect.cy - 2), WHITE, clip);

    /* bottom dark grey line */
    this->fill_rect(0xCC, Rect(1, this->rect.cy - 2, this->rect.cx - 2, 1), DARK_GREY, clip);

    /* right dark grey line */
    this->fill_rect(0xCC, Rect(this->rect.cx - 2, 1, 1, this->rect.cy - 2), DARK_GREY, clip);

    /* bottom black line */
    this->fill_rect(0xCC, Rect(0, this->rect.cy - 1, this->rect.cx, 1), BLACK, clip);

    /* right black line */
    this->fill_rect(0xCC, Rect(this->rect.cx - 1, 0, 1, this->rect.cy), BLACK, clip);

    if (has_focus) {
        this->draw_title_bar(WABGREEN, WHITE, clip);
    }
    else {
        this->draw_title_bar(DARK_GREY, BLACK, clip);
    }
}


void widget_edit::draw(const Rect & clip)
{
    // LOG(LOG_INFO, "widget_edit::draw\n");
    /* draw gray box */
    this->fill_rect(0xCC, Rect(0, 0, this->rect.cx, this->rect.cy), GREY, clip);
    /* main white background */
    this->fill_rect(0xCC, Rect(1, 1, this->rect.cx - 3, this->rect.cy - 3), WHITE, clip);
    /* dark grey top line */
    this->fill_rect(0xCC, Rect(0, 0, this->rect.cx, 1), DARK_GREY, clip);
    /* dark grey left line */
    this->fill_rect(0xCC, Rect(0, 0, 1, this->rect.cy), DARK_GREY, clip);
    /* white bottom line */
    this->fill_rect(0xCC, Rect(0, this->rect.cy- 1, this->rect.cx, 1), WHITE, clip);
    /* white right line */
    this->fill_rect(0xCC, Rect(this->rect.cx - 1, 0, 1, this->rect.cy), WHITE, clip);
    /* black left line */
    this->fill_rect(0xCC, Rect(1, 1, 1, this->rect.cy - 2), BLACK, clip);
    /* black top line */
    this->fill_rect(0xCC, Rect(1, 1, this->rect.cx - 2, 1), BLACK, clip);
    /* draw text */
    char text[255];
    wchar_t wtext[255];

    if (this->password_char != 0) {
        int i = mbstowcs(0, this->buffer, 0);
        memset(text, this->password_char, i);
        text[i] = 0;
        this->mod->server_draw_text(this, 4, 2, text, BLACK, clip);
    }
    else {
        this->mod->server_draw_text(this, 4, 2, this->buffer, BLACK, clip);
    }
    /* draw xor box(cursor) */
    if (has_focus) {
        if (this->password_char != 0) {
            wchar_repeat(wtext, 255, this->password_char, this->edit_pos);
            wtext[this->edit_pos] = 0;
            wcstombs(text, wtext, 255);
        } else {
            mbstowcs(wtext, this->buffer, 255);
            wtext[this->edit_pos] = 0;
            wcstombs(text, wtext, 255);
        }
        Rect r(4 + this->mod->text_width(text), 3, 2, this->rect.cy - 6);
        this->fill_cursor_rect(r, WHITE, clip);
    }
}

void Widget::fill_rect(int rop, const Rect & r, int fg_color, const Rect & clip)
{
    assert(this->type != WND_TYPE_BITMAP);
    const Rect scr_r = this->to_screen_rect(r);

    const Region region = this->mod->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
        this->mod->server_set_clip(region.rects[ir].intersect(this->to_screen_rect(clip)));
        this->mod->opaque_rect(scr_r, fg_color);
    }
}

#warning should merge with basic_fill_rect (and probably with fill_rect but there is some rop transposition code to change on the fly if we want to do this)
void Widget::fill_cursor_rect(const Rect & r, int fg_color, const Rect & clip)
{
    assert(this->type != WND_TYPE_BITMAP);
    const Rect scr_r = this->to_screen_rect(r);
    const Region region = this->mod->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir != region.rects.size(); ir++){
        Rect draw_rect = region.rects[ir].intersect(this->to_screen_rect(clip));
        if (!draw_rect.isempty()) {
            this->mod->server_set_clip(draw_rect);
            this->mod->pat_blt(0x5A, scr_r, fg_color, BLACK);
        }
    }
}


void Widget::basic_fill_rect(int rop, const Rect & r, int fg_color, const Rect & clip)
{
    assert(this->type != WND_TYPE_BITMAP);

    const Rect scr_r = this->to_screen_rect(r);
    const Region region = this->mod->get_visible_region(this, &this->parent, scr_r);

    for (size_t ir = 0 ; ir != region.rects.size(); ir++){
        Rect draw_rect = region.rects[ir].intersect(this->to_screen_rect(clip));
        if (!draw_rect.isempty()) {
            this->mod->server_set_clip(draw_rect);
            this->mod->pat_blt(rop, scr_r, fg_color, BLACK);
        }
    }

}


void widget_combo::draw(const Rect & clip)
{
    /* draw gray box */
    this->fill_rect(0xCC, Rect(0, 0, this->rect.cx, this->rect.cy), GREY, clip);
    /* white background */
    this->fill_rect(0xCC, Rect(1, 1, this->rect.cx - 3, this->rect.cy - 3), WHITE, clip);
    if (has_focus) {
        this->fill_rect(0xCC, Rect(3, 3, (this->rect.cx - 6) - 18, this->rect.cy - 5), DARK_WABGREEN, clip);
    }

    /* dark grey top line */
    this->fill_rect(0xCC, Rect(0, 0, this->rect.cx, 1), DARK_GREY, clip);
    /* dark grey left line */
    this->fill_rect(0xCC, Rect(0, 0, 1, this->rect.cy), DARK_GREY, clip);
    /* white bottom line */
    this->fill_rect(0xCC, Rect(0, this->rect.cy- 1, this->rect.cx, 1), WHITE, clip);
    /* white right line */
    this->fill_rect(0xCC, Rect(this->rect.cx - 1, 0, 1, this->rect.cy), WHITE, clip);
    /* black left line */
    this->fill_rect(0xCC, Rect(1, 1, 1, this->rect.cy - 2), BLACK, clip);
    /* black top line */
    this->fill_rect(0xCC, Rect(1, 1, this->rect.cx - 2, 1), BLACK, clip);

    /* draw text */
    const uint32_t fg_color = has_focus?WHITE:BLACK;
    this->mod->server_draw_text(this, 4, 2, this->string_list[this->item_index], fg_color, clip);
    /* draw button on right */
    Rect r(this->rect.cx - 20, 2, 18, this->rect.cy - 4);
    if (this->state == BUTTON_STATE_UP) { /* 0 */
        /* gray box */
        this->fill_rect(0xCC, Rect(r.x, r.y, r.cx, r.cy), GREY, clip);
        /* white top line */
        this->fill_rect(0xCC, Rect(r.x, r.y, r.cx, 1), WHITE, clip);
        /* white left line */
        this->fill_rect(0xCC, Rect(r.x, r.y, 1, r.cy), WHITE, clip);
        /* dark grey bottom line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY, clip);
        /* dark grey right line */
        this->fill_rect(0xCC, Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY, clip);
        /* black bottom line */
        this->fill_rect(0xCC, Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK, clip);
        /* black right line */
        this->fill_rect(0xCC, Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK, clip);
    }
    else {
        /* gray box */
        this->fill_rect(0xCC, Rect(r.x, r.y, r.cx, r.cy), GREY, clip);
        /* black top line */
        this->fill_rect(0xCC, Rect(r.x, r.y, r.cx, 1), BLACK, clip);
        /* black left line */
        this->fill_rect(0xCC, Rect(r.x, r.y, 1, r.cy), BLACK, clip);
        /* dark grey top line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY, clip);
        /* dark grey left line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY, clip);
        /* dark grey bottom line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + (r.cx - 2), r.cy - 1, 1), DARK_GREY, clip);
        /* dark grey right line */
        this->fill_rect(0xCC, Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY, clip);
        /* black bottom line */
        this->fill_rect(0xCC, Rect(r.x, r.y + (r.cx - 1), r.cy, 1), BLACK, clip);
        /* black right line */
        this->fill_rect(0xCC, Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK, clip);
    }
}

void widget_button::draw_focus_rect(Widget * wdg, const Rect & r, const Rect & clip)
{
    LOG(LOG_INFO, "widget::draw_focus_rect");
    #warning is passing r.x, r.y necessary here for drawing pattern ?
    this->mod->set_domino_brush(r.x, r.y);

    #warning all coordinates provided to front functions should be screen coordinates, converting window relative coordinates to screen coordinates should be responsibility of caller.
    #warning pass in scr_r in screen coordinates instead or r
    Rect scr_r = wdg->to_screen_rect(r);

    Region covering_windows;
    for (size_t i = 0; i < this->mod->nb_windows(); i++) {
        Widget * p = this->mod->window(i);
        if (p == wdg || p == &wdg->parent) {
            break;
        }
        covering_windows.rects.push_back(p->rect);
    }

    /* top */
    struct Region region0;
    region0.rects.push_back(Rect(scr_r.x, scr_r.y, scr_r.cx, 1));
    /* loop through all windows in z order */
    for (size_t ir = 0; ir < covering_windows.rects.size(); ir++) {
        region0.subtract_rect(covering_windows.rects[ir]);
    }
    for (size_t ir = 0 ; ir != region0.rects.size(); ir++){
        Rect draw_rect = region0.rects[ir].intersect(clip);
        if (!draw_rect.isempty()) {
            this->mod->server_set_clip(draw_rect);
            this->mod->pat_blt(0xF0, r.offset(clip.x, clip.y),
                                            wdg->parent.bg_color, BLACK);
        }
    }



    /* bottom */
    struct Region region1;
    region1.rects.push_back(Rect(scr_r.x, scr_r.y + (scr_r.cy - 1), scr_r.cx, 1));
    for (size_t ir = 0; ir < covering_windows.rects.size(); ir++) {
        region1.subtract_rect(covering_windows.rects[ir]);
    }
    for (size_t ir = 0 ; ir != region1.rects.size(); ir++){
        Rect draw_rect = region1.rects[ir].intersect(clip);
        if (!draw_rect.isempty()) {
            this->mod->server_set_clip(draw_rect);
            this->mod->pat_blt(0xF0, r.offset(clip.x, clip.y),
                                            wdg->parent.bg_color, BLACK);
        }
    }

    /* left */
    struct Region region2;
    region2.rects.push_back(Rect(scr_r.x, scr_r.y + 1, 1, scr_r.cy - 2));
    for (size_t ir = 0; ir < covering_windows.rects.size(); ir++) {
        region2.subtract_rect(covering_windows.rects[ir]);
    }
    for (size_t ir = 0 ; ir != region2.rects.size(); ir++){
        Rect draw_rect = region2.rects[ir].intersect(clip);
        if (!draw_rect.isempty()) {
            this->mod->server_set_clip(draw_rect);
            this->mod->pat_blt(0xF0, r.offset(clip.x, clip.y),
                                            wdg->parent.bg_color, BLACK);
        }
    }


    /* right */
    struct Region region3;
    region3.rects.push_back(Rect(scr_r.x + (scr_r.cx - 1), scr_r.y + 1, 1, scr_r.cy - 2));
    for (size_t ir = 0; ir < covering_windows.rects.size(); ir++) {
        region3.subtract_rect(covering_windows.rects[ir]);
    }
    for (size_t ir = 0 ; ir != region3.rects.size(); ir++){
        Rect draw_rect = region3.rects[ir].intersect(clip);
        if (!draw_rect.isempty()) {
            this->mod->server_set_clip(draw_rect);
            this->mod->pat_blt(0xF0, r.offset(clip.x, clip.y),
                                wdg->parent.bg_color, BLACK);
        }
    }
}


void widget_button::draw(const Rect & clip)
{
    int bevel = (this->state == BUTTON_STATE_DOWN)?1:0;

    int w = this->mod->text_width(this->caption1);
    int h = this->mod->text_height(this->caption1);
    Rect r(0, 0, this->rect.cx, this->rect.cy);
    if (this->state == BUTTON_STATE_DOWN) {
        /* gray box */
        this->fill_rect(0xCC, r, GREY, clip);
        /* black top line */
        this->fill_rect(0xCC, Rect(r.x, r.y, r.cx, 1), BLACK, clip);
        /* black left line */
        this->fill_rect(0xCC, Rect(r.x, r.y, 1, r.cy), BLACK, clip);
        /* dark grey top line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY, clip);
        /* dark grey left line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY, clip);
        /* dark grey bottom line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + (r.cx - 2), r.cy - 1, 1), DARK_GREY, clip);
        /* dark grey right line */
        this->fill_rect(0xCC, Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY, clip);
        /* black bottom line */
        this->fill_rect(0xCC, Rect(r.x, r.y + (r.cx - 1), r.cy, 1), BLACK, clip);
        /* black right line */
        this->fill_rect(0xCC, Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK, clip);
    } else {
        /* gray box */
        this->fill_rect(0xCC, r, GREY, clip);
        /* white top line */
        this->fill_rect(0xCC, Rect(r.x, r.y, r.cx, 1), WHITE, clip);
        /* white left line */
        this->fill_rect(0xCC, Rect(r.x, r.y, 1, r.cy), WHITE, clip);
        /* dark grey bottom line */
        this->fill_rect(0xCC, Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY, clip);
        /* dark grey right line */
        this->fill_rect(0xCC, Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY, clip);
        /* black bottom line */
        this->fill_rect(0xCC, Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK, clip);
        /* black right line */
        this->fill_rect(0xCC, Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK, clip);
    }

    this->mod->server_draw_text(this,
        this->rect.cx / 2 - w / 2 + bevel,
        this->rect.cy / 2 - h / 2 + bevel, this->caption1, BLACK, clip);

    if (has_focus) {
        this->draw_focus_rect(this,
            Rect(4, 4, this->rect.cx - 8, this->rect.cy - 8), this->to_screen_rect(clip));
    }
}


void widget_popup::draw(const Rect & clip)
{
    this->fill_rect(0xCC, Rect(0, 0, this->rect.cx, this->rect.cy), WHITE, clip);

    /* draw the list items */
    if (this->popped_from != 0) {
        int y = 0;
        size_t list_count = this->popped_from->string_list.size();
        for (unsigned i = 0; i < list_count; i++) {
            char * p = this->popped_from->string_list[i];
            int h = this->mod->text_height(p);
            this->item_height = h;
            if (i == this->item_index) { // deleted item
                this->fill_rect(0xCC, Rect(0, y, this->rect.cx, h), WABGREEN, clip);
            }
            this->mod->server_draw_text(this, 2, y, p, (i == this->item_index)?WHITE:BLACK, clip);
            y = y + h;
        }
    }
}

void Widget::draw(const Rect & clip)
{
}

void widget_label::draw(const Rect & clip)
{
    this->mod->server_draw_text(this, 0, 0, this->caption1, BLACK, clip);
}

// transform a rectangle relative to current widget to rectangle relative to screen
Rect const Widget::to_screen_rect(const Rect & r)
{
        Rect a = r;
        for (Widget *b = this; WND_TYPE_SCREEN != b->type ; b = &b->parent) {
            a = a.offset(b->rect.x, b->rect.y);
        }
        return a.intersect(this->mod->screen.rect);
}

// get current widget rectangle relative to screen
Rect const Widget::to_screen_rect()
{
        Rect a = Rect(0, 0, this->rect.cx, this->rect.cy);
        for (Widget *b = this; WND_TYPE_SCREEN != b->type ; b = &b->parent) {
            a = a.offset(b->rect.x, b->rect.y);
        }
        return a.intersect(this->mod->screen.rect);
}

void widget_image::draw(const Rect & clip)
{
    Rect image_screen_rect = this->to_screen_rect();
    Rect intersection = image_screen_rect.intersect(this->to_screen_rect(clip));
    const Region region = this->mod->get_visible_region(this, &this->parent, intersection);

    for (size_t ir = 0; ir < region.rects.size(); ir++){
        this->mod->server_set_clip(region.rects[ir]);
        this->mod->server_paint_rect(bmp, image_screen_rect, 0, 0, this->mod->palette332); 
    }
}

int Widget::Widget_invalidate_clip(const Rect & clip)
{
    if (!clip.isempty()) {

        this->mod->server_begin_update();
        this->draw(clip);

        /* notify */
        this->notify(this, WM_PAINT, 0, 0); /* 3 */

        /* draw any child windows in the area */
        int count = this->child_list.size();
        for (int i = 0; i < count; i++) {
            struct Widget* b = this->child_list.at(i);
            struct Rect r2 = clip.intersect(b->rect);
            if (!r2.isempty()) {
                r2 = r2.offset(-(b->rect.x), -(b->rect.y));
                b->Widget_invalidate_clip(r2);
            }
        }
        this->mod->server_end_update();
    }
    return 0;
}

int Widget::Widget_invalidate(const Rect & clip)
{
    struct Widget* b;
    struct Rect r1;
    struct Rect r2;

    this->mod->server_begin_update();

    this->draw(clip);

    this->notify(this, WM_PAINT, 0, 0); /* 3 */

    /* draw any child windows in the area */
    int count = this->child_list.size();
    for (int i = 0; i < count; i++) {
        b = this->child_list[i];
        b->Widget_invalidate(b->rect.wh());
    }
    this->mod->server_end_update();
    return 0;
}

/* all login screen events go here */
void Widget::notify(struct Widget* sender, int msg, long param1, long param2)
{
    this->parent.notify(sender, msg, param1, param2);
}

static inline bool switch_focus(Widget * old_focus, Widget * new_focus) {
    bool res = false;
    if (new_focus->tab_stop){
        if (old_focus) {
            old_focus->has_focus = (old_focus == new_focus);
            old_focus->Widget_invalidate(old_focus->rect.wh());
        }
        if (old_focus != new_focus){
            new_focus->has_focus = true;
            new_focus->Widget_invalidate(new_focus->rect.wh());
        }
        res = true;
    }
    return res;
}

void window::def_proc(int msg, int param1, int param2)
{
    if (msg == WM_KEYDOWN) {

        Widget * control_with_focus = this->default_button;
        // find control that has focus
        size_t size = this->child_list.size();
        size_t i_focus;
        #warning we should iterate only on controls that have tabstop setted (or another attribute can_get_focus ?). Or we could also keep index of focused_control in child_list (but do not forget to reset it when we redefine controls).
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
            int shift = this->mod->keys[42] || this->mod->keys[54];
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
                control_with_focus->def_proc(msg, param1, param2);
            }
        }
    }
}

void widget_edit::def_proc(int msg, int param1, int param2)
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
        && (ext || this->mod->key_flags & 5)) // numlock = 0
        {
            if (this->edit_pos > 0) {
                this->edit_pos--;
                this->Widget_invalidate(this->rect.wh());
            }
        }
        /* right or down arrow */
        else if ((scan_code == 77 || scan_code == 80)
        && (ext || this->mod->key_flags & 5)) // numlock = 0
        {
            if (this->edit_pos < (int)mbstowcs(0, this->buffer, 0)) {
                this->edit_pos++;
                this->Widget_invalidate(this->rect.wh());
            }
        }
        /* backspace */
        else if (scan_code == 14) {

            n = mbstowcs(0, this->buffer, 0);
            if (n > 0) {
                if (this->edit_pos > 0) {
                    this->edit_pos--;
                    remove_char_at(this->buffer, 255, this->edit_pos);
                    this->Widget_invalidate(this->rect.wh());
                }
            }
        }
        /* delete */
        else if (scan_code == 83  && (ext || this->mod->key_flags & 5)) // numlock = 0
        {
            n = mbstowcs(0, this->buffer, 0);
            if (n > 0) {
                if (this->edit_pos < n) {
                    remove_char_at(this->buffer, 255, this->edit_pos);
                    this->Widget_invalidate(this->rect.wh());
                }
            }
        }
        /* end */
        else if (scan_code == 79  && (ext || this->mod->key_flags & 5)) {
            n = mbstowcs(0, this->buffer, 0);
            if (this->edit_pos < n) {
                this->edit_pos = n;
                this->Widget_invalidate(this->rect.wh());
            }
        }
        /* home */
        else if ((scan_code == 71)  &&
                 (ext || (this->mod->key_flags & 5))) {
            if (this->edit_pos > 0) {
                this->edit_pos = 0;
                this->Widget_invalidate(this->rect.wh());
            }
        }
        else {
            c = (wchar_t)(this->mod->keymap
                            ->get_key_info_from_scan_code(
                                                param2, scan_code,
                                                this->mod->keys,
                                                this->mod->key_flags)->chr);


            num_chars = mbstowcs(0, this->buffer, 0);
            num_bytes = strlen(this->buffer);

            if ((c >= 32) && (num_chars < 127) && (num_bytes < 250)) {
                char text[256];
                strncpy(text, this->buffer, 255);

                int index = this->edit_pos;
                #warning why not always keep wcs instead of constantly converting back and from wcs ?
                int len = mbstowcs(0, text, 0);
                wchar_t wstr[len + 16];
                mbstowcs(wstr, text, len + 1);
                if ((this->edit_pos >= len) || (this->edit_pos < 0)) {
                    wstr[len] = c;
                }
                else{
                #warning is backward loop necessary ? a memcpy could do the trick
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
                this->Widget_invalidate(this->rect.wh());
            }

        }
    }
}

void widget_combo::def_proc(int msg, int param1, int param2)
{
    int ext;
    int scan_code;

    if (msg == WM_KEYDOWN) {
        scan_code = param1 % 128;
        ext = param2 & 0x0100;
        /* left or up arrow */
        if (((scan_code == 75) || (scan_code == 72))
        && (ext || (this->mod->key_flags & 5))) {
            if (this->item_index > 0) {
                this->item_index--;
                this->Widget_invalidate(this->rect.wh());
                this->notify(this, CB_ITEMCHANGE, 0, 0);
            }
        }
        /* right or down arrow */
        else if ((scan_code == 77 || scan_code == 80) &&
                 (ext || (this->mod->key_flags & 5))) {
                    size_t count = this->string_list.size();
            if ((this->item_index + 1) < count) {
                this->item_index++;
                this->Widget_invalidate(this->rect.wh());
                this->notify(this, CB_ITEMCHANGE, 0, 0);
            }
        }
    }
}


void widget_popup::def_proc(int msg, int param1, int param2)
{
    if (msg == WM_MOUSEMOVE) {
        if (this->item_height > 0 && this->popped_from != 0) {
            unsigned i = param2 / this->item_height;
            if (i != this->item_index && i < this->popped_from->string_list.size())
            {
                this->item_index = i;
                this->Widget_invalidate(this->rect.wh());
            }
        }
    } else if (msg == WM_LBUTTONUP) {

        if (this->popped_from != 0) {
            this->popped_from->item_index = this->item_index;
            this->popped_from->Widget_invalidate(this->popped_from->rect.wh());
            this->popped_from->notify(this->popped_from, CB_ITEMCHANGE, 0, 0);
        }
    }
}

void Widget::def_proc(int msg, int param1, int param2)
{
}
