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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#if !defined(__DIALOG_HPP__)
#define __DIALOG_HPP__

#include <stdlib.h>
#include "widget.hpp"
#include "widget_window_login.hpp"
#include "log.hpp"

struct window_dialog : public window
{
    Session * session;
    ModContext * context;

    window_dialog(internal_mod * mod, const Rect & r,
                  ModContext & context, Session* session,
                  Widget & parent, Widget & owner, Widget & notify_to,
                  int bg_color,
                  const char * title, Inifile * ini, int regular,
                  const char * message, const char * refuse)
    : window(mod, r, parent, bg_color, title)
    {
        struct Widget* but;
        this->session = session;
        this->context = &context;
        this->esc_button = NULL;

        but = new widget_button(this->mod, Rect(200, 160, 60, 25), *this, 3, 1, "OK");
        this->child_list.push_back(but);
        this->default_button = but;

        if (refuse) {
            but = new widget_button(this->mod, Rect(300, 160, 60, 25), *this, 2, 1, refuse);
            this->child_list.push_back(but);
            this->esc_button = but;
            this->default_button = but;
        }
        int count = 0;
        bool done = false;
        while (!done) {
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;
            but = new widget_label(this->mod, Rect(50, 60 + 16 * count, 500, 40), *this, tmp);
            this->child_list.push_back(but);
            count++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (this->modal_dialog != 0 && msg != 100) {
            return;
        }
        #warning use symbolic button ids instead of constants 2 and 3
        if (msg == 1) { /* click */
            switch (sender->id) {
                case 2: /* cancel button -> Esc */
                    LOG(LOG_INFO, "click on Cancel");
                    this->mod->mod_event(WM_KEYUP, 0, 0, 1, 0);
                break;
                case 3: /* ok button -> Enter */
                    LOG(LOG_INFO, "click on OK");
                    this->mod->mod_event(WM_KEYUP, 0, 0, 28, 0);
                break;
                default:
                break;
            }
        }
        return;
    }

    virtual ~window_dialog(){
    }
};

struct dialog_mod : public internal_mod {
    struct window_dialog * close_window;
    Session * session;
    Widget* button_down;

    int dragging;
    Rect dragging_rect;
    int draggingdx; // distance between mouse and top angle of dragged window
    int draggingdy; // distance between mouse and top angle of dragged window
    struct Widget* dragging_window;

    dialog_mod(wait_obj * event,
               int (& keys)[256], int & key_flags, Keymap * &keymap,
               ModContext & context,
               Front & front, Session * session,
               const char *message, const char * refuse)
            :
            internal_mod(keys, key_flags, keymap, front)
    {

        /* dragging info */
        this->dragging = 0;
        this->event = event;
        // dragging_rect is (0,0,0,0)
        this->draggingdx = 0; // distance between mouse and top angle of dragged window
        this->draggingdy = 0; // distance between mouse and top angle of dragged window
        this->dragging_window = 0;
        this->button_down = 0;

        int log_width = 600;
        int log_height = 200;
        int regular = 1;
        this->session = session;

        this->signal = 0;

        /* draw login window */
        Rect r(
            this->screen.rect.cx / 2 - log_width / 2,
            this->screen.rect.cy / 2 - log_height / 2,
            log_width,
            log_height);

        #warning valgrind say there is a memory leak here
        this->close_window = new window_dialog(this,
            r, context, session,
            this->screen, // parent
            this->screen, // owner
            this->screen, // notify_to
            GREY,
            "Information",
            session->ini,
            regular,
            message, refuse);

        this->screen.child_list.push_back(this->close_window);
        assert(this->close_window->mod == this);

        this->close_window->focus(this->close_window->rect);
        this->close_window->has_focus = true;
        this->screen.Widget_invalidate(this->screen.rect.wh());
    }
    ~dialog_mod() {
    }

    // module received an event from client
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        LOG(LOG_INFO, "mod_event(%d, %ld, %ld, %ld %ld)", msg, x, y, param4, param5);
        switch (msg){
        case WM_KEYUP:
        {
            int scan_code = param4 & 0x7F;
            const char * message = NULL;
            switch (scan_code) {
                case 1: /* cancel button */
                    message = "False";
                break;
                case 28: /* ok button */
                    message = "True";
                break;
                default:
                    message = NULL;
                break;
            }
            if (message){
                strcpy(this->close_window->context->get(
                        (this->close_window->esc_button)?STRAUTHID_ACCEPT_MESSAGE
                                                        :STRAUTHID_DISPLAY_MESSAGE),
                                            message);
                this->event->set();
                this->signal = 1;
            }
        }
        break;
        case WM_KEYDOWN:
        break;
        case WM_INVALIDATE:
            this->screen.fill_rect(0xCC, this->screen.rect, this->screen.bg_color, this->screen.rect);
            break;
        case WM_MOUSEMOVE:
            if (this->dragging) {
                long dragx = (x < 0)                         ? 0
                           : (x < this->screen.rect.cx) ? x
                           : this->screen.rect.cx
                           ;

                long dragy = (y < 0)                         ? 0
                           : (y < this->screen.rect.cy) ? y
                           : this->screen.rect.cy
                           ;

                this->server_begin_update();
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                this->dragging_rect.x = dragx - this->draggingdx ;
                this->dragging_rect.y = dragy - this->draggingdy;
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                this->server_end_update();
            }
            else {
                struct Widget *b = this->screen.widget_at_pos(x, y);
                if (b == 0) { /* if b is null, the movement must be over the screen */
                    b = this->get_screen_wdg();
                }
                if (b->pointer != this->current_pointer) {
                    this->set_pointer(b->pointer);
                }
                b->def_proc(WM_MOUSEMOVE, b->from_screenx(x), b->from_screeny(y));
                if (this->button_down) {
                    this->button_down->state = (b == this->button_down);
                    this->button_down->Widget_invalidate(this->button_down->rect.wh());
                }
                else {
                    b->notify(&b->parent, 2, x, y);
                }
            }
        break;
        case WM_LBUTTONDOWN:
        {
            /* loop on surface widgets on screen to find active window */
            Widget* wnd = this->get_screen_wdg();
            for (size_t i = 0; i < wnd->child_list.size(); i++) {
                if (wnd->child_list[i]->rect.rect_contains_pt(x, y)) {
                    wnd = this->screen.child_list[i];
                    break;
                }
            }

            /* set focus on window */
            if (wnd && wnd->type == WND_TYPE_WND) {
                wnd->focus();
            }

            Widget * control = wnd->widget_at_pos(x, y);

            if (wnd != this->get_screen_wdg()) {
                if (wnd->modal_dialog != 0) {
                    /* window has a modal dialog (but we didn't clicked on it) */
                    break;
                }
                // change focus. Is graphical feedback necessary ?
                if (control != wnd && control->tab_stop) {
                    #warning control that had focus previously does not loose it, easy way could be to loop on all controls and clear all existing focus
                    control->has_focus = true;
                    for (size_t i = 0; i < wnd->child_list.size(); i++) {
                        wnd->child_list[i]->has_focus = false;
                        wnd->child_list[i]->Widget_invalidate(wnd->child_list[i]->rect.wh());
                    }
                    control->Widget_invalidate(control->rect.wh());
                }
            }

            switch (control->type) {
                case WND_TYPE_BUTTON:
                    this->button_down = control;
                    control->state = 1;
                    control->Widget_invalidate(control->rect.wh());
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
        }
        break;
        case WM_LBUTTONUP:
        {
            if (this->dragging) {
                /* if done dragging */
                /* draw xor box one more time */
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);

                /* move dragged window to new location */
                Rect r = this->dragging_window->rect;
                this->dragging_window->rect.x = this->dragging_rect.x;
                this->dragging_window->rect.y = this->dragging_rect.y;
                this->dragging_window->Widget_invalidate_clip(r);
                this->screen.Widget_invalidate(this->screen.rect.wh());
                this->dragging_window = 0;
                this->dragging = 0;
                break;
            }

            /* loop on surface widgets on screen to find active window */
            Widget* wnd = this->get_screen_wdg();
            for (size_t i = 0; i < wnd->child_list.size(); i++) {
                if (wnd->child_list[i]->rect.rect_contains_pt(x, y)) {
                    wnd = this->screen.child_list[i];
                    break;
                }
            }

            Widget * control = wnd->widget_at_pos(x, y);

            if (wnd != this->get_screen_wdg()) {
                if (wnd->modal_dialog != 0) {
                    /* window has a modal dialog (but we didn't clicked on it) */
                    break;
                }
                if (control != wnd && control->tab_stop) {
                #warning previous focus on other control is not yet disabled
                    control->has_focus = true;
                    control->Widget_invalidate(control->rect.wh());
                }
            }

            switch (control->type) {
                case WND_TYPE_BUTTON:
                    if (this->button_down == control){
                        control->state = 0;
                        control->Widget_invalidate(control->rect.wh());
                        control->notify(control, 1, x, y);
                    }
                break;
                default:
                break;
            }
            // mouse is up, no more button down, whatever
            this->button_down = 0;
        }
        break;

        default:
            /* internal : redemption interface only use button 1 */
            break;
        }
        return 0;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual int mod_signal()
    {
        return this->signal;
    }

    int clear_popup()
    {
        return 0;
    }

};

#endif
