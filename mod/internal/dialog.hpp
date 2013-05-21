/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Message Dialog box
*/

#ifndef _REDEMPTION_MOD_INTERNAL_DIALOG_HPP_
#define _REDEMPTION_MOD_INTERNAL_DIALOG_HPP_

#include <stdlib.h>
#include "log.hpp"
#include "widget/window.hpp"

struct dialog_mod : public internal_mod {
    struct window_dialog * dialog_box;
    Widget* button_down;
    bool refuse_flag;
    Inifile & ini;

    dialog_mod(FrontAPI & front, uint16_t width, uint16_t height,
               const char *message, const char * refuse, Inifile & ini)
            : internal_mod(front, width, height)
            , refuse_flag(refuse != NULL)
            , ini(ini)
    {
        this->button_down = 0;

        int log_width = 600;
        int min_log_height = 200;
        int max_log_height = this->get_screen_rect().cy - 40;
        int regular = 1;

        this->signal = BACK_EVENT_NONE;

        size_t number_of_lines = 1;
        uint32_t len = strlen(message);
        for (size_t xx = 0; xx < len - 4 ; xx++) {
            if ((message[xx] == '<')
            && (message[xx+1] == 'b')
            && (message[xx+2] == 'r')
            && (message[xx+3] == '>')){
                xx+=3;
                number_of_lines++;
            }
        }

        int log_height = number_of_lines * 16 + 75;
        if (log_height > min_log_height){
            log_height = std::min(max_log_height, log_height);
        }
        if (log_height < min_log_height){
            log_height = min_log_height;
        }

        /* draw login window */
        Rect r(
            this->get_screen_rect().cx / 2 - log_width / 2,
            this->get_screen_rect().cy / 2 - log_height / 2,
            log_width,
            log_height);

        this->front.begin_update();
        this->dialog_box = new window_dialog(this,
            r,
            &this->screen, // parent
            GREY,
            "Information",
            &ini,
            regular,
            message,
            refuse);

        this->dialog_box->focus(this->dialog_box->rect);
        this->dialog_box->has_focus = true;
        this->screen.refresh(this->get_screen_rect().wh());
        this->front.end_update();

    }
    virtual ~dialog_mod() {
        delete this->dialog_box;
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->front.begin_update();
            this->screen.draw(rect);
            this->front.end_update();
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            if (this->dragging) {
                long dragx = (x < 0)                    ? 0
                           : (x < this->get_screen_rect().cx) ? x
                           : this->get_screen_rect().cx
                           ;

                long dragy = (y < 0)                    ? 0
                           : (y < this->get_screen_rect().cy) ? y
                           : this->get_screen_rect().cy
                           ;

                this->front.begin_update();
                this->server_draw_dragging_rect(this->dragging_rect, this->get_screen_rect());
                this->dragging_rect.x = dragx - this->draggingdx ;
                this->dragging_rect.y = dragy - this->draggingdy;
                this->server_draw_dragging_rect(this->dragging_rect, this->get_screen_rect());
                this->front.end_update();
            }
        }


        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            if (device_flags & MOUSE_FLAG_DOWN){
                /* loop on surface widgets on screen to find active window */
                Widget* wnd = &this->screen;
                for (size_t i = 0; i < wnd->child_list.size(); i++) {
                    if (wnd->child_list[i]->rect.contains_pt(x, y)) {
                        wnd = this->screen.child_list[i];
                        break;
                    }
                }

                /* set focus on window */
                if (wnd && wnd->type == WND_TYPE_WND) {
                    wnd->focus();
                }

                Widget * control = wnd->widget_at_pos(x, y);
                if ((wnd == &this->screen) || (wnd->modal_dialog == 0)) {
                    if (wnd != &this->screen) {
                        // change focus. Is graphical feedback necessary ?
                        if (control != wnd && control->tab_stop) {
                            TODO(" control that had focus previously does not loose it  easy way could be to loop on all controls and clear all existing focus")
                            control->has_focus = true;
                            for (size_t i = 0; i < wnd->child_list.size(); i++) {
                                wnd->child_list[i]->has_focus = false;
                                wnd->child_list[i]->refresh(wnd->child_list[i]->rect.wh());
                            }
                            control->refresh(control->rect.wh());
                        }
                    }

                    switch (control->type) {
                        case WND_TYPE_BUTTON:
                            this->button_down = control;
                            control->state = 1;
                            control->refresh(control->rect.wh());
                        break;
                        case WND_TYPE_WND:
                            /* drag by clicking in title bar and keeping button down */
                            if (y < (control->rect.y + 21)) {
                                this->dragging = 1;
                                this->dragging_window = control;

                                this->draggingdx = x - control->rect.x;
                                this->draggingdy = y - control->rect.y;

                                this->dragging_rect = Rect(
                                    x - this->draggingdx, y - this->draggingdy,
                                    control->rect.cx, control->rect.cy);
                                this->server_draw_dragging_rect(this->dragging_rect, this->get_screen_rect());
                            }
                        break;
                        default:
                        break;
                    }
                }
            }
            else {
                if (this->dragging) {
                    /* if done dragging */
                    /* draw xor box one more time */
                    this->server_draw_dragging_rect(this->dragging_rect, this->get_screen_rect());

                    /* move dragged window to new location */
                    Rect r = this->dragging_window->rect;
                    this->dragging_window->rect.x = this->dragging_rect.x;
                    this->dragging_window->rect.y = this->dragging_rect.y;
                    this->front.begin_update();
                    this->dragging_window->refresh(r);
                    this->screen.refresh(this->get_screen_rect().wh());
                    this->front.end_update();
                    this->dragging_window = 0;
                    this->dragging = 0;
                }
                else {
                    /* loop on surface widgets on screen to find active window */
                    Widget* wnd = &this->screen;
                    for (size_t i = 0; i < wnd->child_list.size(); i++) {
                        if (wnd->child_list[i]->rect.contains_pt(x, y)) {
                            wnd = this->screen.child_list[i];
                            break;
                        }
                    }

                    Widget * control = wnd->widget_at_pos(x, y);

                    if ((wnd == &this->screen)
                    || (wnd->modal_dialog == 0)) {

                        if (wnd != &this->screen) {
                            if (control != wnd && control->tab_stop) {
                            TODO(" previous focus on other control is not yet disabled")
                                control->has_focus = true;
                                control->refresh(control->rect.wh());
                            }
                        }

                        switch (control->type) {
                            case WND_TYPE_BUTTON:
                                if (this->button_down == control){
                                    control->state = 0;
                                    control->refresh(control->rect.wh());
                                    control->notify(control->id, 1, x, y);
                                }
                            break;
                            default:
                            break;
                        }
                        // mouse is up, no more button down, whatever
                        this->button_down = 0;
                    }
                }
            }
        }
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap){
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_KEY:
                {
                    uint32_t c = keymap->get_char();
                    if (c == ' '){
                        this->ini.context_set_value(
                            (this->refuse_flag ? _AUTHID_ACCEPT_MESSAGE : _AUTHID_DISPLAY_MESSAGE),
                            "True");
                        this->event.set();
                        this->signal = BACK_EVENT_NEXT;
                    }
                }
                break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->ini.context_set_value(
                        (this->refuse_flag ? _AUTHID_ACCEPT_MESSAGE : _AUTHID_DISPLAY_MESSAGE),
                        "True");
                    this->event.set();
                    this->signal = BACK_EVENT_NEXT;
                break;
                default:
                break;
            }
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual BackEvent_t draw_event()
    {
        return this->signal;
    }

    int clear_popup()
    {
        return 0;
    }

    void server_draw_dragging_rect(const Rect & r, const Rect & clip)
    {
        this->begin_update();

        RDPBrush brush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55");

        // draw rectangles by drawing each edge top/bottom/left/right
        // 0x66 = xor -> pat_blt( ... 0x5A ...
        // 0xAA = noop -> pat_blt( ... 0xFB ...
        // 0xCC = copy -> pat_blt( ... 0xF0 ...
        // 0x88 = and -> pat_blt( ...  0xC0 ...

        this->draw(RDPPatBlt(Rect(r.x, r.y, r.cx, 5), 0x5A, BLACK, WHITE, brush), clip);
        this->draw(RDPPatBlt(Rect(r.x, r.y + (r.cy - 5), r.cx, 5), 0x5A, BLACK, WHITE, brush), clip);
        this->draw(RDPPatBlt(Rect(r.x, r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, brush), clip);
        this->draw(RDPPatBlt(Rect(r.x + (r.cx - 5), r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, brush), clip);
        this->end_update();
    }
};

#endif
