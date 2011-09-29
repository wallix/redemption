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

#warning merge with dialog.hpp

#include "widget_window_login.hpp"

struct wab_close : public window_login
{
    wab_close(internal_mod * mod, const Rect & r, ModContext & context, Widget & parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    : window_login(mod, r, context, parent, notify_to, bg_color, title, ini, regular)
    {
        struct Widget* but;

        if (regular) {
            widget_image * but = new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE,
                *this, 10, 30, SHARE_PATH "/" LOGIN_LOGO24, this->mod->screen.bpp);
            #warning bitmap load below should probably be done before call
            this->child_list.push_back(but);
        }

        struct Widget* b;
        int count = 0;
        /* label */
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 70, 20),
            *this, "Username:");

        this->child_list.push_back(b);
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ?  230 : 70), 60 + 25 * count, 350, 20),
            *this, context.get(STRAUTHID_AUTH_USER));

        b->id = 100 + 2 * count;
        this->child_list.push_back(b);
        count ++;

        char target[255];
        snprintf(target, 255, "%s@%s", context.get(STRAUTHID_TARGET_USER), context.get(STRAUTHID_TARGET_DEVICE));

        b = new widget_label(this->mod,
            Rect(10+((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 70, 20),
            *this, "Target:");

        this->child_list.push_back(b);
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ?  230 : 70), 60 + 25 * count, 350, 20),
            *this, target);

        b->id = 100 + 2 * count;
        this->child_list.push_back(b);
        count ++;

        b = new widget_label(this->mod,
            Rect(150 + ((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 130, 20),
            *this, "Connection closed");

        this->child_list.push_back(b);
        count ++;

        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
            *this, "Diagnostic:");

        this->child_list.push_back(b);

        bool done = false;
        int line = 0;
        const char * message;
        message = context.get(STRAUTHID_AUTH_ERROR_MESSAGE);
        while (!done) {
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;
            b = new widget_label(this->mod, Rect((this->rect.cx >= 400) ?  230 : 70, 60 + 25 * count + 16 * line, 350, 20), *this, tmp);
            this->child_list.push_back(b);
            line++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }

        /* label */
        but = new widget_button(this->mod,
              Rect(50 + (regular ? 250 : ((r.cx - 30) - 60)), 150 + 16 * line, 60, 25),
              *this, 2, 1, "Close");
        this->child_list.push_back(but);
        this->esc_button = but;
        this->default_button = but;

    }

    ~wab_close(){
    }

};

struct close_mod : public internal_mod {
    struct window_login * close_window;
    Widget* button_down;
    int signal;
    Inifile * ini;
    bool closing;

    close_mod(
        wait_obj * event,
        ModContext & context, Front & front, Inifile * ini)
            : internal_mod(front), signal(0), ini(ini), closing(false)
    {
        this->event = event;
        this->event->set();
        this->button_down = 0;

        int width = 600;
        bool done = false;
        const char * message;
        message = context.get(STRAUTHID_AUTH_ERROR_MESSAGE);
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
        int height = 200+line*16;

        int regular = 1;

        if (this->screen.rect.cx < width ) {
            width = std::min(this->screen.rect.cx - 4, 240);
            regular = 0;
        }

        /* draw login window */
        Rect r(
            this->screen.rect.cx / 2 - width / 2,
            this->screen.rect.cy / 2 - height / 2,
            width,
            height);

        this->close_window = new wab_close(this,
            r, context,
            this->screen, // parent
            this->screen, // notify_to
            GREY,
            "Close",
            this->ini,
            regular);

        this->screen.child_list.push_back(this->close_window);
        assert(this->close_window->mod == this);

        if (regular) {
            /* image */
            widget_image * but = new widget_image(this, 4, 4,
                WND_TYPE_IMAGE, this->screen,
                this->screen.rect.cx - 250 - 4,
                this->screen.rect.cy - 120 - 4,
                SHARE_PATH "/" REDEMPTION_LOGO24, this->screen.bpp);

            this->screen.child_list.push_back(but);
        }

        this->close_window->focus(this->close_window->rect);
        this->close_window->has_focus = true;
        this->screen.refresh(this->screen.rect.wh());
    }

    virtual ~close_mod()
    {
        #warning here delete all widgets from this->screen.child_list
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->server_begin_update();
            Rect & r = this->screen.rect;
            this->screen.draw(r);

            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    b->refresh_clip(r2);
                }
            }
            this->server_end_update();
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap)
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
                b->def_proc(WM_MOUSEMOVE, b->from_screenx(x), b->from_screeny(y), keymap);
                if (this->button_down) {
                    this->button_down->state = (b == this->button_down);
                    this->button_down->refresh(this->button_down->rect.wh());
                }
                else {
                    b->notify(&b->parent, 2, x, y);
                }
            }
            this->front.mouse_x = x;
            this->front.mouse_y = y;

        }

        // ---------------------------------------------------------------
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            LOG(LOG_INFO, "button 1 clicked");
            // LBUTTON DOWN
            if (device_flags & MOUSE_FLAG_DOWN){
                if (!this->dragging){
                    LOG(LOG_INFO, "button down");
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
                            this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                        }
                    }
                }
            }
            // LBUTTON UP
            else {
               if (this->dragging) {
                    /* if done dragging */
                    /* draw xor box one more time */
                    this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                    /* move dragged window to new location */
                    Rect r = this->dragging_window->rect;
                    this->dragging_window->rect.x = this->dragging_rect.x;
                    this->dragging_window->rect.y = this->dragging_rect.y;
                    this->dragging_window->refresh_clip(r);
                    this->screen.refresh(this->screen.rect.wh());
                   this->dragging_window = 0;
                    this->dragging = 0;
                }
                else {
                    if (this->button_down && this->closing){
                        this->button_down->state = 0;
                        this->button_down->refresh(this->button_down->rect.wh());
                        this->signal = 4;
                        this->event->set();
                    }
                }
                this->button_down = 0;
            }
        }
        // No other button are used in redemption interface
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, const Keymap * keymap, const key_info* ki){
        LOG(LOG_INFO, "scan code");
        if (ki != 0) {
            int msg = (device_flags & KBD_FLAG_UP)?WM_KEYUP:WM_KEYDOWN;
            switch (msg){
            case WM_KEYUP:
                if (this->close_window->has_focus) {
                    this->close_window->def_proc(msg, param1, device_flags, keymap);
                    this->signal = 4;
                    this->event->set();
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
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual int draw_event()
    {
        this->event->reset();
        return signal;
    }

};

#endif
