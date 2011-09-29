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

#if !defined(__WIDGET_WINDOW_LOGIN_HPP__)
#define __WIDGET_WINDOW_LOGIN_HPP__

#include "widget.hpp"
#include "internal_mod.hpp"

struct window_login : public window
{
    protected:
    Widget & notify_to;
    Inifile * ini;
    widget_combo * combo;
    ModContext & context;
    window * help;

    window_login(internal_mod * mod, const Rect & r, ModContext & context, Widget & parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    :   window(mod, r, parent, bg_color, title),
        notify_to(notify_to),
        context(context)
    {
    }

    IniAccounts * current_account()
    {
        return &(this->ini->account[this->combo->item_index]);
    }

    #warning this should not be here, the whole widget hierarchy should be rethought anyway
    int login_window_show_edits()
    {
        int count;
        struct Widget* b;

        /* free edits and labels, cause we gota re-create them */
        vector<Widget*> tmp;
        vector<Widget*> & list = this->child_list;
        for (size_t i = 0; i < list.size() ; i++){
            if (list[i]->type == WND_TYPE_LABEL || list[i]->type == WND_TYPE_EDIT) {
                delete list[i];
                continue;
            }
            tmp.push_back(list[i]);
       }
       list.clear();

       size_t i_tmp = 0;
       for (; i_tmp < tmp.size() ; i_tmp++){
          if (tmp[i_tmp] == this->combo){
              break;
          }
          list.push_back(tmp[i_tmp]);
       }
       list.push_back(this->combo);

        IniAccounts & acc = this->ini->account[this->combo->item_index];

        if (acc.accountdefined){
            const char * target_protocol = "RDP";
            switch (acc.idlib){
                case ID_LIB_VNC:
                    target_protocol = "VNC";
                break;
                case ID_LIB_XUP:
                    target_protocol = "XUP";
                break;
                default:;
            }
            this->context.cpy(STRAUTHID_TARGET_PROTOCOL, target_protocol);
            this->context.cpy(STRAUTHID_TARGET_DEVICE, acc.ip);
            this->context.cpy(STRAUTHID_TARGET_PORT, acc.port);
        }

        count = 0;
        if (acc.askip){
            /* label */
            b = new widget_label(this->mod,
                Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
                *this, "ip");

            b->id = 100 + 2 * count;
            list.push_back(b);

            /* edit */
            b = new widget_edit(this->mod,
                Rect(this->rect.cx >= 400 ? 230 : 70, 60 + 25 * count, 350, 20),
                *this,
                100 + 2 * count + 1, /* id */
                1, /* tab stop */
                acc.ip,
                1, /* pointer */
                0 /* edit pos */);

            list.push_back(b);
            count++;
        }

        if (acc.askusername){
            b = new widget_label(this->mod,
                Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
                *this, this->context.get(STRAUTHID_TRANS_LOGIN));

            b->id = 100 + 2 * count;
            list.push_back(b);

            /* edit */
            b = new widget_edit(this->mod,
                Rect((this->rect.cx >= 400) ? 230 : 70, 60 + 25 * count, 350, 20),
                    *this,
                    100 + 2 * count + 1, /* id */
                    1, /* tab stop */
                    acc.username,
                    1, /* pointer */
                    0 /* edit pos */);

            list.push_back(b);

            this->focused_control = b;
            b->has_focus = true;
            count++;
        }

        if (acc.askpassword){
            b = new widget_label(this->mod,
                Rect(this->rect.cx >= 400 ? 155 : 5, 60 + 25 * count, 70, 20),
                *this, this->context.get(STRAUTHID_TRANS_PASSWORD));

            b->id = 100 + 2 * count;
            list.push_back(b);

            /* edit */
            b = new widget_edit(this->mod,
                    Rect((this->rect.cx) >= 400 ? 230 : 70, 60 + 25 * count, 350, 20),
                    *this,
                    100 + 2 * count + 1, /* id */
                    1, /* tab stop */
                    acc.password,
                    1, /* pointer */
                    0 /* edit pos */);

            #warning integrate that in widget_edit
            b->password_char = '*';
            list.push_back(b);
            count++;
        }

        for (i_tmp = 0; i_tmp < tmp.size() ; i_tmp++){
            list.push_back(tmp[i_tmp]);
            if (tmp[i_tmp] == this->combo){
                list.pop_back();
            }
        }

        return 0;
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
                    this->mod->server_begin_update();
                    this->mod->screen.refresh_clip(this->mod->screen.rect);
                    this->mod->server_end_update();
                    this->modal_dialog = 0;
                    break;
                }
            }
        } else if (msg == CB_ITEMCHANGE) { /* combo box change */
            this->login_window_show_edits();
            this->refresh(this->rect.wh()); /* rdp_input_invalidate the whole dialog for now */
        }
        return;
    }

    int help_clicked()
    {
        #warning add new function in widget_screen : add_window, where window can be modal
        this->modal_dialog = this->help;
        {
            vector<Widget *>::iterator it = this->mod->screen.child_list.begin();
            this->mod->screen.child_list.insert(it, this->help);
        }

        struct Widget* but = new widget_button(this->mod,
                    Rect(140, 260, 60, 25),
                    *this->help, 1, 1, this->context.get(STRAUTHID_TRANS_BUTTON_OK));
        {
            vector<Widget *>::iterator it = this->help->child_list.begin();
            this->help->child_list.insert(it, but);
        }

        #warning add new function to window add_button (add_widget ?)
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

        struct Widget* combo = this->Widget_get_child_by_id(6);

        if (combo != 0) {
            IniAccounts & acc = this->ini->account[combo->item_index];
            const char * target_protocol = "RDP";
            switch (acc.idlib){
                case ID_LIB_VNC:
                    target_protocol = "VNC";
                break;
                case ID_LIB_XUP:
                    target_protocol = "XUP";
                break;
                default:;
            }
            this->context.cpy(STRAUTHID_TARGET_PROTOCOL, target_protocol);

            if (acc.accountdefined){
                /* get the user typed values */
                i = 100;
                for (;;) {
                #warning we should not rely on labels and window ordering for such things but on widget (Widget) identifiers
                    struct Widget* label = this->Widget_get_child_by_id(i);
                    if (label == 0) {
                        break;
                    }
                    struct widget_edit * edit = (widget_edit *)this->Widget_get_child_by_id(i + 1);
                    if (edit == 0){
                        break;
                    }
                    if (0 == strcmp(label->caption1, "ip")){
                        context.cpy(STRAUTHID_TARGET_DEVICE, acc.askip?edit->buffer:acc.ip);
                    }
                    else if (0 == strcmp(label->caption1, this->context.get(STRAUTHID_TRANS_LOGIN))){
                        context.parse_username(acc.askusername?edit->buffer:acc.username);
                    }
                    else if (0 == strcmp(label->caption1, this->context.get(STRAUTHID_TRANS_PASSWORD))){
                        context.cpy(STRAUTHID_PASSWORD, acc.askpassword?edit->buffer:acc.password);
                    }
                    i += 2;
                }
                this->mod->signal = 2;
                this->mod->event->set();
            }
        }
        else {
            i = 104;
            for (;;) {
                struct Widget* label = this->Widget_get_child_by_id(i);
                if (label == 0) {
                    break;
                }
                struct widget_edit * edit = (widget_edit *)this->Widget_get_child_by_id(i + 1);
                if (label == 0) {
                    break;
                }
            #warning we should not rely on labels and window ordering for such things but on widget (Widget) identifiers
                if (0 == strcmp(label->caption1, this->context.get(STRAUTHID_TRANS_PASSWORD))){
                        context.cpy(STRAUTHID_PASSWORD, edit->buffer);
                }
                i += 2;
            }
            this->mod->signal = 3;
            this->mod->event->set();
        }
        return 0;

    }

    int cancel_clicked()
    {
        this->mod->signal = 4;
        this->mod->event->set();
        return 0;
    }

};

#endif
