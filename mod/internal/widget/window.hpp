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

#include "internal/widget/widget.hpp"

#include "internal/widget/image.hpp"
#include "internal/widget/button.hpp"
#include "internal/widget/label.hpp"

#include "internal/widget/edit.hpp"
#include "internal/widget/image.hpp"
#include "modcontext.hpp"
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



struct window_help : public window
{
    Widget & notify_to;

    window_help(mod_api * mod, const Rect & r, Widget * parent, Widget & notify_to, int bg_color, const char * title)
    : window(mod, r, parent, bg_color, title), notify_to(notify_to)
    {
    }

    void draw(const Rect & clip)
    {
        this->window::draw(clip);
        TODO(" the code below is a bit too much specialized. Change it to some code able to write a paragraph of text in a given rectangle. Later we may even add some formatting support.")
        const char * message =
                "You must be authenticated before using this<br>"
                "session.<br>"
                "<br>"
                "Enter a valid username in the username edit box.<br>"
                "Enter the password in the password edit box.<br>"
                "<br>"
                "Both the username and password are case<br>"
                "sensitive.<br>"
                "<br>"
                "Contact your system administrator if you are<br>"
                "having problems logging on.<br>"
                ;
        int count = 0;
        bool done = false;
        while (!done) {
            const char * str = strstr(message, "<br>");
            char tmp[256];
            tmp[0] = 0;
            strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
            tmp[255] = 0;

            Rect r(0, 0, this->rect.cx, this->rect.cy);
            const Rect scr_r = this->to_screen_rect(r);
            const Region region = this->get_visible_region(this, this->parent, scr_r);

            for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
                this->mod->server_draw_text(scr_r.x + 10, scr_r.y + 30 + 16 * count, tmp, GREY, BLACK, region.rects[ir].intersect(this->to_screen_rect(this->rect.wh())));
            }

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
        if (msg == 1) { /* click */
            if (id == 1) { /* ok button */
                    this->notify_to.notify(id, 100, 1, 0); /* ok */
            }
        } else if (msg == WM_PAINT) { /* 3 */
            this->draw(this->rect);
        }
        return;
    }
};


struct window_login : public window
{
    public:
    Widget & notify_to;
    Inifile * ini;
    ModContext & context;
    TODO("The help window should be created by module, not by other window") 

    window_login(mod_api * mod, const Rect & r, ModContext & context, Widget * parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    :   window(mod, r, parent, bg_color, title),
        notify_to(notify_to),
        context(context)
    {
        this->ini = ini;

        if (regular) {
            TODO("CGR: see other comments about this new. Why it's not actually"
                 " an error but it's not good practice and should change")
            new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE, this, 10, 30,
                    SHARE_PATH "/" LOGIN_LOGO24, 24);
        }

        if (ini->context_is_asked(_AUTHID_TARGET_USER)
        ||  ini->context_is_asked(_AUTHID_TARGET_DEVICE)){
            if (ini->context_is_asked(_AUTHID_AUTH_USER)){
                ini->account.username[0] = 0;
            }
            else {
//                strcpy(ini->account.username, context.get(STRAUTHID_AUTH_USER));
                strncpy(ini->account.username,
                    this->ini->context_get_value(_AUTHID_AUTH_USER, NULL, 0),
                    sizeof(ini->account.username));
                ini->account.username[sizeof(ini->account.username) - 1] = 0;
            }
        }
        else if (ini->context_is_asked(_AUTHID_AUTH_USER)) {
            ini->account.username[0] = 0;
        }
        else {
            TODO("check this! Assembling parts to get user login with target is not obvious"
                 "method used below il likely to show @: if target fields are empty")
            char buffer[256];
            snprintf( buffer, 256, "%s@%s:%s%s%s"
//                    , context.get(STRAUTHID_TARGET_USER)
                    , this->ini->context_get_value(_AUTHID_TARGET_USER, NULL, 0)
//                    , context.get(STRAUTHID_TARGET_DEVICE)
                    , this->ini->context_get_value(_AUTHID_TARGET_DEVICE, NULL, 0)
//                    , context.get(STRAUTHID_TARGET_PROTOCOL)[0]?context.get(STRAUTHID_TARGET_PROTOCOL):""
                    , (this->ini->context_get_value(_AUTHID_TARGET_PROTOCOL, NULL, 0)[0] ?
                           this->ini->context_get_value(_AUTHID_TARGET_PROTOCOL, NULL, 0) : "")
//                    , context.get(STRAUTHID_TARGET_PROTOCOL)[0]?":":""
                    , (this->ini->context_get_value(_AUTHID_TARGET_PROTOCOL, NULL, 0)[0] ? ":" : "")
//                    , context.get(STRAUTHID_AUTH_USER)
                    , this->ini->context_get_value(_AUTHID_AUTH_USER, NULL, 0)
                    );
            strcpy(ini->account.username, buffer);
        }

        Widget * but = new widget_button(this->mod,
              Rect(regular ? 180 : 30, 160, 60, 25),
              this, 3, 1, ini->globals.translation.button_ok);
        this->default_button = but;

        but = new widget_button(this->mod,
              Rect(regular ? 250 : ((r.cx - 30) - 60), 160, 60, 25),
              this, 2, 1, ini->globals.translation.button_cancel);
        this->esc_button = but;

        if (regular) {
            but = new widget_button(this->mod,
                  Rect(320, 160, 60, 25), this, 1, 1, ini->globals.translation.button_help);
        }

        IniAccounts & acc = this->ini->account;

        struct Widget* login_label = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60, 70, 22),
            this, ini->globals.translation.login);

        login_label->id = 100;

        /* edit */
        struct Widget* login_edit = new widget_edit(this->mod,
            Rect((this->rect.cx >= 400) ? 230 : 70, 60, 350, 22),
                this,
                100 + 1, /* id */
                1, /* tab stop */
                acc.username,
                1, /* pointer */
                0 /* edit pos */);

        if (acc.username[0] == 0){
            this->focused_control = login_edit;
            login_edit->has_focus = true;
        }

        struct Widget* password_label = new widget_label(this->mod,
            Rect(this->rect.cx >= 400 ? 155 : 5, 60 + 25, 70, 22),
            this, ini->globals.translation.password);

        password_label->id = 100 + 2;

        /* edit */
        struct Widget* password_edit = new widget_edit(this->mod,
                Rect((this->rect.cx) >= 400 ? 230 : 70, 60 + 25, 350, 22),
                this,
                100 + 2 + 1, /* id */
                1, /* tab stop */
                acc.password,
                1, /* pointer */
                0 /* edit pos */);

        TODO(" move that into widget_edit")
        password_edit->password_char = '*';

        if (acc.username[0]){
            this->focused_control = password_edit;
            password_edit->has_focus = true;
        }
    }

    IniAccounts * current_account()
    {
        return &(this->ini->account);
    }

    virtual void notify(int id, int msg, long param1, long param2)
    {
//        LOG(LOG_INFO, "notify: id=%u msg=%u p1=%lu p2=%lu", id, msg, param1, param2);
        if (this->modal_dialog != 0 && msg != 100) {
            return;
        }
        if (msg == 1) { /* click */
            if (id == 1) { /* help button */
                this->help_clicked();
            } else if (id == 2) { /* cancel button */
                this->cancel_clicked();
            } else if (id == 3) { /* ok button */
                this->ok_clicked();
            }
        } else if (msg == 2) { /* mouse move */
        } else if (msg == 100) { /* modal result is done */
            if (this->modal_dialog && this->modal_dialog->has_focus){
                this->modal_dialog->has_focus = false;
            }
            this->has_focus = true;
            for (size_t i = 0 ; i < this->parent->child_list.size() ; i++)
            {
                Widget * b = this->parent->child_list[i];
                if (b->id == id){
                    this->parent->child_list.erase(this->parent->child_list.begin()+i);
                    this->mod->begin_update();
                    this->parent->refresh(this->parent->rect);
                    this->mod->end_update();
                    this->modal_dialog = 0;
                    break;
                }
            }
        }
        return;
    }

    int help_clicked()
    {
        TODO("Disabled until help window is fixed")
//        this->modal_dialog = this->help;
//        struct Widget* but = new widget_button(this->mod,
//                    Rect(140, 260, 60, 25),
//                    this->help, 1, 1, ini->globals.translation.button_ok);

//        TODO(" add new function to window add_button (add_widget ?)")
//        /* draw it */
//        but->has_focus = true;
//        this->help->default_button = but;
//        this->help->esc_button = but;

//        this->help->refresh(this->help->rect.wh());
//        this->help->focus(this->help->rect);
//        this->has_focus = false;
//        this->help->has_focus = true;
//        this->help->refresh(this->help->rect.wh());
        return 0;
    }

    int ok_clicked()
    {
        int i;

        /* get the user typed values */
        i = 100;
        for (;;) {
        TODO(" we should not rely on labels and window ordering for such things but on widget (Widget) identifiers")
            struct Widget* label = this->Widget_get_child_by_id(i);
            if (label == 0) {
                break;
            }
            struct widget_edit * edit = (widget_edit *)this->Widget_get_child_by_id(i + 1);
            if (edit == 0){
                break;
            }
            else if (0 == strcmp(label->caption1, this->ini->globals.translation.login)){
//                this->context.parse_username(edit->buffer, *this->ini);
                this->ini->parse_username(edit->buffer);
            }
            else if (0 == strcmp(label->caption1, this->ini->globals.translation.password)){
//                this->context.cpy(STRAUTHID_PASSWORD, edit->buffer);
                this->ini->context_set_value(_AUTHID_PASSWORD, edit->buffer);
            }
            i += 2;
        }
        this->mod->mod_event(BACK_EVENT_NEXT);
        return 0;

    }

    int cancel_clicked()
    {
        this->mod->mod_event(BACK_EVENT_STOP);
        return 0;
    }

    virtual ~window_login(){
    }
};


struct window_dialog : public window
{
    ModContext * context;
    Inifile * ini;

    window_dialog(mod_api * mod, const Rect & r,
                  ModContext & context,
                  Widget * parent, int bg_color,
                  const char * title, Inifile * ini, int regular,
                  const char * message,
                  const char * refuse)
    : window(mod, r, parent, bg_color, title)
    , context(&context)
    , ini(ini)
    {
//        this->context = &context;
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
/*
                this->context->cpy(
                        (this->esc_button)?STRAUTHID_ACCEPT_MESSAGE
                                          :STRAUTHID_DISPLAY_MESSAGE,
                        "False");
*/
                this->ini->context_set_value(
                    (this->esc_button ? _AUTHID_ACCEPT_MESSAGE : _AUTHID_DISPLAY_MESSAGE),
                    "False");
                this->mod->mod_event(BACK_EVENT_NEXT);
            break;
            case 3: /* ok button -> Enter */
/*
                this->context->cpy(
                        (this->esc_button)?STRAUTHID_ACCEPT_MESSAGE
                                          :STRAUTHID_DISPLAY_MESSAGE,
                        "True");
*/
                this->ini->context_set_value(
                    (this->esc_button ? _AUTHID_ACCEPT_MESSAGE : _AUTHID_DISPLAY_MESSAGE),
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
    ModContext & context;
    window * help;

    wab_close(mod_api * mod, const Rect & r, ModContext & context, Inifile & ini, Widget * parent, int bg_color, const char * title, int regular)
    : window(mod, r, parent, bg_color, title),
      context(context)
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
//            this, context.is_asked(_STRAUTHID_AUTH_USER)? "" : context.get(STRAUTHID_AUTH_USER));
            this, ini.context_is_asked(_STRAUTHID_AUTH_USER)? "" : ini.context_get_value(_AUTHID_AUTH_USER, NULL, 0));

        b->id = 100 + 2 * count;
        count ++;

        char target[255];
        if (ini.context_is_asked(_AUTHID_TARGET_USER)
        ||  ini.context_is_asked(_AUTHID_TARGET_DEVICE)){
            target[0] = 0;
        }
        else {
            snprintf(target, 255, "%s@%s",
//                context.get(STRAUTHID_TARGET_USER),
                ini.context_get_value(_AUTHID_TARGET_USER, NULL, 0),
//                context.get(STRAUTHID_TARGET_DEVICE));
                ini.context_get_value(_AUTHID_TARGET_DEVICE, NULL, 0));
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
//        message = context.get(STRAUTHID_AUTH_ERROR_MESSAGE);
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
