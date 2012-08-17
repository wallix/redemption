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

   Close Window with message

*/

#if !defined(__CLOSE_HPP__)
#define __CLOSE_HPP__

#include "widget_window_close.hpp"

struct close_mod : public internal_mod {
    struct window * close_window;
    Widget* button_down;
    bool closing;

    close_mod(ModContext & context, FrontAPI & front, uint16_t width, uint16_t height)
            : internal_mod(front, width, height), closing(false)
    {
        this->button_down = 0;

        int win_width = 600;
        bool done = false;
        const char * message = context.get(STRAUTHID_AUTH_ERROR_MESSAGE);
        int line = 0;
        while (!done) {
            const char * str = strstr(message, "<br>");
            line++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }
        int win_height = 200+line*16;

        int regular = 1;

        if (this->get_screen_rect().cx < win_width ) {
            win_width = std::min(this->get_screen_rect().cx - 4, 240);
            regular = 0;
        }

        Rect r(this->get_screen_rect().cx / 2 - win_width / 2,
               this->get_screen_rect().cy / 2 - win_height / 2,
               win_width,
               win_height);

        this->close_window = new wab_close(this,
            r, context,
            &this->screen, // parent
            GREY,
            "Close",
            regular);

        this->screen.child_list.push_back(this->close_window);
        assert(this->close_window->mod == this);

        if (regular) {
            /* image */
            widget_image * but = new widget_image(this, 4, 4,
                WND_TYPE_IMAGE, &this->screen,
                this->screen.rect.cx - 250 - 4,
                this->screen.rect.cy - 120 - 4,
                SHARE_PATH "/" REDEMPTION_LOGO24, 24);

            this->screen.child_list.push_back(but);
        }

        this->close_window->focus(this->close_window->rect);
        this->close_window->has_focus = true;
    }

    virtual ~close_mod()
    {
        TODO(" here delete all widgets from this->screen.child_list")
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->front.begin_update();
            const Rect & r = this->get_screen_rect();
            this->screen.draw(r);

            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    this->front.begin_update();
                    b->refresh(r2);
                    this->front.end_update();
                }
            }
            this->front.end_update();
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            if (this->dragging) {
                long dragx = (x < 0)                         ? 0
                           : (x < this->get_screen_rect().cx) ? x
                           : this->get_screen_rect().cx
                           ;

                long dragy = (y < 0)                         ? 0
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
            else {
                struct Widget *b = this->screen.widget_at_pos(x, y);
                if (b == 0) { /* if b is null, the movement must be over the screen */
                    b = &this->screen;
                }
//                if (b->pointer != this->current_pointer) {
//                    this->server_set_pointer(b->pointer);
//                }
                b->def_proc(WM_MOUSEMOVE, b->from_screenx(x), b->from_screeny(y), keymap);
                if (this->button_down) {
                    this->button_down->state = (b == this->button_down);
                    this->button_down->refresh(this->button_down->rect.wh());
                }
                else {
                    b->notify(b->parent, 2, x, y);
                }
            }
        }

        // ---------------------------------------------------------------
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            // LBUTTON DOWN
            if (device_flags & MOUSE_FLAG_DOWN){
                if (!this->dragging){
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
                    if (control && control->type == WND_TYPE_BUTTON){
                        control->state = 1;
                        control->refresh(control->rect.wh());
                        this->closing = true;
                        this->button_down = control;
                    }
                    else if (control && control->type == WND_TYPE_WND){
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
                    }
                }
            }
            // LBUTTON UP
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
                    this->front.end_update();
                    this->screen.refresh(this->get_screen_rect().wh());
                    this->dragging_window = 0;
                    this->dragging = 0;
                }
                else {
                    if (this->button_down && this->closing){
                        this->button_down->state = 0;
                        this->button_down->refresh(this->button_down->rect.wh());
                        this->signal = BACK_EVENT_STOP;
                        this->event.set();
                    }
                }
                this->button_down = 0;
            }
        }
        // No other button are used in redemption interface
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap){
        int msg = (device_flags & KBD_FLAG_UP)?WM_KEYUP:WM_KEYDOWN;
        switch (msg){
        case WM_KEYUP:
            if (this->close_window->has_focus) {
                this->close_window->def_proc(msg, param1, device_flags, keymap);
                this->signal = BACK_EVENT_STOP;
                this->event.set();
            } else {
                this->close_window->has_focus = 1;
            }
        break;
        case WM_KEYDOWN:
            if (this->close_window->has_focus) {
                this->close_window->def_proc(msg, param1, device_flags, keymap);
            }
        break;
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual BackEvent_t draw_event()
    {
        LOG(LOG_INFO, "============================== close::DRAW_EVENT =================================");

        this->front.begin_update();
        this->screen.refresh(this->get_screen_rect().wh());
        this->front.end_update();
        this->event.reset();
        return signal;
    }

};

#endif
