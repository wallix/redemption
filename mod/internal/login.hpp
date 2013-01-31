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

   Login Window

*/

#ifndef _REDEMPTION_MOD_INTERNAL_LOGIN_HPP_
#define _REDEMPTION_MOD_INTERNAL_LOGIN_HPP_

#include "widget_window_login.hpp"


struct login_mod : public internal_mod {
    struct window_login * login_window;
//    Widget* popup_wnd;
    Widget* button_down;


    login_mod(ModContext & context, FrontAPI & front, uint16_t width, uint16_t height, Inifile * ini)
            : internal_mod(front, width, height)
    {

        uint32_t nb = (this->screen.rect.cy - 230) / 20;
        nb = (nb > 50)?50:nb;
        char buffer[128];
        sprintf(buffer, "%u", nb);

        context.cpy(STRAUTHID_SELECTOR_LINES_PER_PAGE, buffer);

        this->signal = BACK_EVENT_NONE;
        this->button_down = 0;

        int log_width = 600;
        int log_height = 200;
        int regular = 1;

        if (this->screen.rect.cx < log_width ) {
            log_width = std::min(this->screen.rect.cx - 4, 240);
            regular = 0;
        }

        /* draw login window */
        Rect r(
            this->screen.rect.cx / 2 - log_width / 2,
            this->screen.rect.cy / 2 - log_height / 2,
            log_width,
            log_height);

        this->login_window = new window_login(this,
            r, context,
            &this->screen, // parent
            this->screen, // notify_to
            GREY,
            VERSION,
            ini,
            regular);

        this->screen.child_list.push_back(this->login_window);
        assert(this->login_window->mod == this);

//        LOG(LOG_INFO, "loading image...");

        if (regular) {
            /* image */
            widget_image * but = new widget_image(this, 4, 4,
                WND_TYPE_IMAGE,
                &this->screen,
                this->screen.rect.cx - 250 - 4,
                this->screen.rect.cy - 120 - 4,
                SHARE_PATH "/" REDEMPTION_LOGO24,
                24);

            this->screen.child_list.push_back(but);
        }

//        LOG(LOG_INFO, "focus on login_window");

        this->login_window->focus(this->login_window->rect);

        this->front.begin_update();
        this->screen.refresh(this->screen.rect.wh());
//        LOG(LOG_INFO, "rdp_input_invalidate screen done");
        this->front.end_update();
    }

    virtual ~login_mod()
    {
        TODO(" here destroy all widgets from screen.child_list")
    }

    /*****************************************************************************/
    int clear_popup()
    {
        return 0;
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->front.begin_update();
            this->screen.draw(rect);
            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    b->refresh(r2);
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
                           : (x < this->screen.rect.cx) ? x
                           : this->screen.rect.cx
                           ;

                long dragy = (y < 0)                         ? 0
                           : (y < this->screen.rect.cy) ? y
                           : this->screen.rect.cy
                           ;

                this->front.begin_update();
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                this->dragging_rect.x = dragx - this->draggingdx ;
                this->dragging_rect.y = dragy - this->draggingdy;
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                this->front.end_update();
            }
            else {
                this->front.begin_update();

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
                this->front.end_update();
            }
        }

        // -------------------------------------------------------
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            if (device_flags & MOUSE_FLAG_DOWN) {
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

                if (wnd != &this->screen) {
                    if (!wnd->modal_dialog) {
                        // change focus. Is graphical feedback necessary ?
                        if (control != wnd && control->tab_stop) {
                            this->front.begin_update();

                            TODO(" control that had focus previously does not loose it  easy way could be to loop on all controls and clear all existing focus")
                            control->has_focus = true;
                            for (size_t i = 0; i < wnd->child_list.size(); i++) {
                                wnd->child_list[i]->has_focus = false;
                                wnd->child_list[i]->refresh(wnd->child_list[i]->rect.wh());
                            }
                            control->refresh(control->rect.wh());
                            this->front.end_update();

                        }
                    }
                }

                if ((wnd != &this->screen) && !wnd->modal_dialog){
                    this->front.begin_update();
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
                                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                            }
                        break;
                        default:
                        break;
                    }
                    this->front.end_update();
                }
            }
            else { // Button UP
                if (this->dragging) {
                     this->front.begin_update();

                    /* if done dragging */
                    /* draw xor box one more time */
                    this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);

                    /* move dragged window to new location */
                    Rect r = this->dragging_window->rect;
                    this->dragging_window->rect.x = this->dragging_rect.x;
                    this->dragging_window->rect.y = this->dragging_rect.y;
                    this->front.begin_update();
                    this->dragging_window->refresh(r);
                    this->screen.refresh(this->screen.rect.wh());
                    this->front.end_update();
                    this->dragging_window = 0;
                    this->dragging = 0;

                    this->front.end_update();
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

                    {
                        if (wnd == &this->screen || (wnd->modal_dialog == 0)){
                            if (wnd != &this->screen) {
                                if (control != wnd && control->tab_stop) {
                                TODO(" previous focus on other control is not yet disabled")
                                    control->has_focus = true;
                                    this->front.begin_update();
                                    control->refresh(control->rect.wh());
                                    this->front.end_update();
                                }
                            }

                            switch (control->type) {
                                case WND_TYPE_BUTTON:
                                    if (this->button_down == control){
                                        control->state = 0;
                                        this->front.begin_update();
                                        control->refresh(control->rect.wh());
                                        control->notify(control, 1, x, y);
                                        this->front.end_update();
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
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, Keymap2 * keymap){
        if (keymap->nb_kevent_available() > 0){
            if (this->login_window->has_focus) {
                this->front.begin_update();
                this->login_window->def_proc(WM_KEYDOWN, param1, device_flags, keymap);
                keymap->get_kevent();
                this->front.end_update();
            }
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    virtual BackEvent_t draw_event()
    {
        // after refreshing button, return module status that may have changed
        return this->signal;
    }

};

#endif
