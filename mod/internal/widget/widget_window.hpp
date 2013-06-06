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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni
*/

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_WINDOW_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_WINDOW_HPP_

#include "internal/widget/widget_widget.hpp"

#include "internal/widget/image.hpp"
#include "internal/widget/button.hpp"
#include "internal/widget/label.hpp"

#include "internal/widget/edit.hpp"
#include "internal/widget/image.hpp"
#include "config.hpp"

struct window : public Widget
{
    window(mod_api * mod, const Rect & r, Widget * parent, int bg_color, const char * title)
    : Widget(mod, r, parent, WND_TYPE_WND) {
        this->bg_color = bg_color;
        this->caption1 = strdup(title);
    }

    /* find the window containing widget */
    virtual window * find_window()
    {
        return this;
    }

    virtual ~window() {
        if (this->caption1){
            free(this->caption1);
            this->caption1 = 0;
        }
    }

    void focus(const Rect & clip)
    {
        this->has_focus = true;
    }

    void blur(const Rect & clip)
    {
        this->has_focus = false;
    }

    void draw(const Rect & clip)
    {
        Rect r(0, 0, this->rect.cx, this->rect.cy);
        const Rect scr_r = this->to_screen_rect(r);
        const Region region = this->get_visible_region(this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            // Window surface and border
            this->mod->draw(RDPOpaqueRect(scr_r, this->bg_color), region_clip);
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x + 1, scr_r.y + 1, scr_r.cx - 2, 1), WHITE), region_clip);
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x + 1, scr_r.y + 1, 1, scr_r.cy - 2), WHITE), region_clip);
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x + 1, scr_r.y + scr_r.cy - 2, scr_r.cx - 2, 1), DARK_GREY), region_clip);
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x+scr_r.cx-2, scr_r.y + 1, 1, scr_r.cy), DARK_GREY), region_clip);
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x, scr_r.y + scr_r.cy - 1, scr_r.cx, 1), BLACK), region_clip);
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x + scr_r.cx - 1, scr_r.y, 1, scr_r.cy), BLACK), region_clip);

            // Title bar
            this->mod->draw(RDPOpaqueRect(Rect(scr_r.x + 3, scr_r.y + 3, scr_r.cx - 5, 18),
                              this->has_focus?WABGREEN:DARK_GREY), region_clip);

            this->mod->server_draw_text(scr_r.x + 4, scr_r.y + 4, this->caption1,
                    this->has_focus?WABGREEN:DARK_GREY,
                    this->has_focus?WHITE:BLACK, region_clip);
        }
    }

    // change control that have focus
    bool switch_focus(Widget * old_focus, Widget * new_focus) {
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

    void def_proc(const int msg, const int param1, const int param2, Keymap2 * keymap)
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

            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_TAB:
                for (size_t i = (size+i_focus+1) % size ; i != i_focus ; i = (i+size+1) % size) {
                    Widget * new_focus = this->child_list[i];
                    if (this->switch_focus(control_with_focus, new_focus)) {
                        break;
                    }
                }
            break;
            case Keymap2::KEVENT_BACKTAB:
                for (size_t i = (size+i_focus-1) % size ; i != i_focus ; i = (i+size-1) % size) {
                    Widget * new_focus = this->child_list[i];
                    if (this->switch_focus(control_with_focus, new_focus)) {
                        break;
                    }
                }
            break;
            case Keymap2::KEVENT_ENTER:
                this->notify(this->default_button->id, 1, 0, 0);
            return;
            case Keymap2::KEVENT_ESC:
                if (this->esc_button) {
                    this->notify(this->esc_button->id, 1, 0, 0);
                }
            break;
            default:
                if (control_with_focus){
                    control_with_focus->def_proc(msg, param1, param2, keymap);
                }
            }
        }
    }
};

struct window_dialog : public window
{
    Inifile * ini;

    window_dialog(mod_api * mod, const Rect & r,
                  Widget * parent, int bg_color,
                  const char * title, Inifile * ini, int regular,
                  const char * message,
                  const char * refuse)
    : window(mod, r, parent, bg_color, title)
    , ini(ini)
    {
        this->esc_button = NULL;

        struct Widget* but = new widget_button(this->mod, Rect(200, r.cy - 40, 60, 25), this, 3, 1, "OK");
        this->default_button = but;

        if (refuse) {
            but = new widget_button(this->mod, Rect(300, r.cy - 40, 60, 25), this, 2, 1, refuse);
            this->esc_button = but;
            this->default_button = but;
        }
        size_t count = 0;
        bool done = false;
        while (!done) {
            if (75 + count * 16 >= r.cy){
                break;
            }
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;
            but = new widget_label(this->mod, Rect(30, 25 + 16 * count, r.cx - 30, 40), this, tmp);
            count++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }
    }

    virtual void notify(int id, int msg, long param1, long param2)
    {
        if (this->modal_dialog != 0 && msg != 100) {
            return;
        }
        TODO(" use symbolic button ids instead of constants 2 and 3")
        if (msg == 1) { /* click */
            LOG(LOG_INFO, "widget_window_dialog::notify id=%d msg=%d", id, msg);
            switch (id) {
            case 2: /* cancel button -> Esc */
                this->ini->context_set_value(
                    (this->esc_button ? AUTHID_ACCEPT_MESSAGE : AUTHID_DISPLAY_MESSAGE),
                    "False");
                this->mod->mod_event(BACK_EVENT_NEXT);
            break;
            case 3: /* ok button -> Enter */
                this->ini->context_set_value(
                    (this->esc_button ? AUTHID_ACCEPT_MESSAGE : AUTHID_DISPLAY_MESSAGE),
                    "True");
                this->mod->mod_event(BACK_EVENT_NEXT);
            break;
            default:
            break;
            }
            LOG(LOG_INFO, "widget_window_dialog::notify done");
        }
        return;
    }

    virtual ~window_dialog(){
    }
};


struct wab_close : public window
{
    window * help;

    wab_close(mod_api * mod, const Rect & r, Inifile & ini, Widget * parent, int bg_color, const char * title, int regular)
    : window(mod, r, parent, bg_color, title)
    {
        if (regular) {
            TODO("CGR: WIdget is registered in wab_close and will de deallocated"
                 " through it's child list. I'm not sure it's such a good idea "
                 "as it breaks the usual new/delete pair."
                 "It works but it would be good to find a better way."
                 "A possible solution would be to make child_list some kind of"
                 "class and instanciate widgets through it (as it will also deallocate)")
            new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE,
                this, 10, 30, SHARE_PATH "/" LOGIN_LOGO24, 24);
            TODO(" bitmap load below should probably be done before call")
        }

        struct Widget* b;
        int count = 0;
        /* label */
        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 70, 20),
            this, "Username:");

        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ?  230 : 70), 60 + 25 * count, 350, 20),
            this, ini.context_is_asked(AUTHID_AUTH_USER)? "" : ini.context_get_value(AUTHID_AUTH_USER, NULL, 0));

        b->id = 100 + 2 * count;
        count ++;

        char target[255];
        if (ini.context_is_asked(AUTHID_TARGET_USER)
        ||  ini.context_is_asked(AUTHID_TARGET_DEVICE)){
            target[0] = 0;
        }
        else {
            snprintf(target, 255, "%s@%s",
                ini.context_get_value(AUTHID_TARGET_USER, NULL, 0),
                ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0));
        }

        b = new widget_label(this->mod,
            Rect(10+((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 70, 20),
            this, "Target:");

        b = new widget_label(this->mod,
            Rect(10 + ((this->rect.cx >= 400) ?  230 : 70), 60 + 25 * count, 350, 20),
            this, target);

        b->id = 100 + 2 * count;
        count ++;

        b = new widget_label(this->mod,
            Rect(150 + ((this->rect.cx >= 400) ? 155 : 5), 60 + 25 * count, 130, 20),
            this, "Connection closed");

        count ++;

        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
            this, "Diagnostic:");

        bool done = false;
        int line = 0;
        const char * message;
        message = ini.globals.context.auth_error_message;
        while (!done) {
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;
            b = new widget_label(this->mod, Rect((this->rect.cx >= 400) ?  230 : 70, 60 + 25 * count + 16 * line, 350, 20), this, tmp);
            line++;
            if (!str){
                done = true;
            }
            else {
                message = str + 4;
            }
        }

        /* label */
        struct Widget* but = new widget_button(this->mod,
              Rect(50 + (regular ? 250 : ((r.cx - 30) - 60)), 150 + 16 * line, 60, 25),
              this, 2, 1, "Close");
        this->esc_button = but;
        this->default_button = this->esc_button;

    }

    ~wab_close(){
    }

    virtual void notify(int id, int msg, long param1, long param2)
    {
        return;
    }
 };

#endif
