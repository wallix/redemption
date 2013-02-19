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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_WINDOW_LOGIN_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_WINDOW_LOGIN_HPP_

#include "widget_window.hpp"
#include "widget.hpp"
#include "internal_mod.hpp"
#include "widget_window.hpp"
#include "widget_button.hpp"
#include "widget_edit.hpp"
#include "widget_label.hpp"
#include "widget_image.hpp"
#include "version.hpp"


struct window_help : public window
{
    Widget & notify_to;

    window_help(internal_mod * mod, const Rect & r, Widget * parent, Widget & notify_to, int bg_color, const char * title)
    : window(mod, r, parent, bg_color, title), notify_to(notify_to)
    {
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (msg == 1) { /* click */
            if (sender->id == 1) { /* ok button */
                    this->notify_to.notify(this, 100, 1, 0); /* ok */
            }
        } else if (msg == WM_PAINT) { /* 3 */
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
                const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

                for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
                    this->mod->front.server_draw_text(scr_r.x + 10, scr_r.y + 30 + 16 * count, tmp, GREY, BLACK, region.rects[ir].intersect(this->to_screen_rect(this->rect.wh())));
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
        return;
    }

};


struct window_login : public window
{
    public:
    Widget & notify_to;
    Inifile * ini;
    ModContext & context;
    window * help;

    window_login(internal_mod * mod, const Rect & r, ModContext & context, Widget * parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    :   window(mod, r, parent, bg_color, title),
        notify_to(notify_to),
        context(context)
    {
        this->ini = ini;

        /* create help screen */
        uint32_t grey = 0xc0c0c0;
        this->help = new window_help(this->mod,
            Rect(this->mod->screen.rect.cx / 2 - 340 / 2,
                this->mod->screen.rect.cy / 2 - 300 / 2,
                340,
                300),
            &mod->screen, // parent
            *this, // notify
            grey,
            "Login help");

        if (regular) {
            widget_image * but = new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE, this, 10, 30,
                    SHARE_PATH "/" LOGIN_LOGO24, 24);
            this->child_list.push_back(but);
        }

        if (context.is_asked(STRAUTHID_TARGET_USER)
        ||  context.is_asked(STRAUTHID_TARGET_DEVICE)){
            if (context.is_asked(STRAUTHID_AUTH_USER)){
                ini->account.username[0] = 0;
            }
            else {
                strcpy(ini->account.username, context.get(STRAUTHID_AUTH_USER));
            }
        }
        else if (context.is_asked(STRAUTHID_AUTH_USER)) {
            ini->account.username[0] = 0;
        }
        else {
            char buffer[256];
            if ( strlen(context.get(STRAUTHID_TARGET_PROTOCOL)) > 0) {
                snprintf( buffer, 256, "%s@%s:%s:%s"
                        , context.get(STRAUTHID_TARGET_USER)
                        , context.get(STRAUTHID_TARGET_DEVICE)
                        , context.get(STRAUTHID_TARGET_PROTOCOL)
                        , context.get(STRAUTHID_AUTH_USER)
                        );
            }
            else {
                snprintf( buffer, 256, "%s@%s:%s"
                        , context.get(STRAUTHID_TARGET_USER)
                        , context.get(STRAUTHID_TARGET_DEVICE)
                        , context.get(STRAUTHID_AUTH_USER)
                        );
            }
            strcpy(ini->account.username, buffer);
        }

        Widget * but = new widget_button(this->mod,
              Rect(regular ? 180 : 30, 160, 60, 25),
              this, 3, 1, context.get(STRAUTHID_TRANS_BUTTON_OK));
        this->child_list.push_back(but);
        this->default_button = but;

        but = new widget_button(this->mod,
              Rect(regular ? 250 : ((r.cx - 30) - 60), 160, 60, 25),
              this, 2, 1, context.get(STRAUTHID_TRANS_BUTTON_CANCEL));
        this->child_list.push_back(but);
        this->esc_button = but;

        if (regular) {
            but = new widget_button(this->mod,
                  Rect(320, 160, 60, 25), this, 1, 1, context.get(STRAUTHID_TRANS_BUTTON_HELP));
            this->child_list.push_back(but);
        }

        IniAccounts & acc = this->ini->account;

        struct Widget* login_label = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60, 70, 22),
            this, this->context.get(STRAUTHID_TRANS_LOGIN));

        login_label->id = 100;
        this->child_list.push_back(login_label);

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
        this->child_list.push_back(login_edit);

        struct Widget* password_label = new widget_label(this->mod,
            Rect(this->rect.cx >= 400 ? 155 : 5, 60 + 25, 70, 22),
            this, this->context.get(STRAUTHID_TRANS_PASSWORD));

        password_label->id = 100 + 2;
        this->child_list.push_back(password_label);

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
        this->child_list.push_back(password_edit);

        if (acc.username[0]){
            this->focused_control = password_edit;
            password_edit->has_focus = true;
        }
    }

    IniAccounts * current_account()
    {
        return &(this->ini->account);
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (this->modal_dialog != 0 && msg != 100) {
            return;
        }
        if (msg == 1) { /* click */
            if (sender->id == 1) { /* help button */
                this->help_clicked();
            } else if (sender->id == 2) { /* cancel button */
                this->cancel_clicked();
            } else if (sender->id == 3) { /* ok button */
                this->ok_clicked();
            }
        } else if (msg == 2) { /* mouse move */
        } else if (msg == 100) { /* modal result is done */
            sender->has_focus = false;
            this->has_focus = true;
            for (size_t i = 0 ; i < this->mod->screen.child_list.size() ; i++)
            {
                Widget * b = this->mod->screen.child_list[i];
                if (b == sender){
                    this->mod->screen.child_list.erase(this->mod->screen.child_list.begin()+i);
                    this->mod->front.begin_update();
                    this->mod->screen.refresh(this->mod->screen.rect);
                    this->mod->front.end_update();
                    this->modal_dialog = 0;
                    break;
                }
            }
        }
        return;
    }

    int help_clicked()
    {
        TODO(" add new function in widget_screen : add_window  where window can be modal")
        this->modal_dialog = this->help;
        {
            vector<Widget *>::iterator it = this->mod->screen.child_list.begin();
            this->mod->screen.child_list.insert(it, this->help);
        }

        struct Widget* but = new widget_button(this->mod,
                    Rect(140, 260, 60, 25),
                    this->help, 1, 1, this->context.get(STRAUTHID_TRANS_BUTTON_OK));
        {
            vector<Widget *>::iterator it = this->help->child_list.begin();
            this->help->child_list.insert(it, but);
        }

        TODO(" add new function to window add_button (add_widget ?)")
        /* draw it */
        but->has_focus = true;
        this->help->default_button = but;
        this->help->esc_button = but;

        this->help->refresh(this->help->rect.wh());
        this->help->focus(this->help->rect);
        this->has_focus = false;
        this->help->has_focus = true;
        this->help->refresh(this->help->rect.wh());
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
            else if (0 == strcmp(label->caption1, this->context.get(STRAUTHID_TRANS_LOGIN))){
                context.parse_username(edit->buffer);
            }
            else if (0 == strcmp(label->caption1, this->context.get(STRAUTHID_TRANS_PASSWORD))){
                context.cpy(STRAUTHID_PASSWORD, edit->buffer);
            }
            i += 2;
        }
        this->mod->signal = BACK_EVENT_NEXT;
        this->mod->event.set();
        return 0;

    }

    int cancel_clicked()
    {
        this->mod->signal = BACK_EVENT_STOP;
        this->mod->event.set();
        return 0;
    }

    ~window_login(){
        delete this->help;
    }
};

#endif
