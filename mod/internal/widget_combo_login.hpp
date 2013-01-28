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
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Login Window

*/

#if !defined(__MOD_INTERNAL_WIDGET_COMBO_LOGIN_HPP__)
#define __MOD_INTERNAL_WIDGET_COMBO_LOGIN_HPP__

#include "widget_window.hpp"
#include "widget_window_login.hpp"
#include "widget_image.hpp"
#include "widget_combo_help.hpp"
#include "widget_popup.hpp"
#include "version.hpp"

struct combo_login : public window_login
{
    combo_login(internal_mod * mod, const Rect & r, ModContext & context, Widget * parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    : window_login(mod, r, context, parent, notify_to, bg_color, title, ini, regular)
    {
        this->ini = ini;
        struct Widget* but;

        /* create help screen */
        uint32_t grey = 0xc0c0c0;
        this->help = new combo_help(this->mod,
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

        /* label */
        but = new widget_label(this->mod, Rect((regular ? 155 : 5), 35, 60, 20), this,  "Module");
        this->child_list.push_back(but);

        Rect rect(regular ? 230 : 70, 35, 350, 20);
        this->combo = new widget_combo(this->mod, rect, this, 6, 1);

        this->combo->string_list.push_back(strdup(ini->account.accountname));

        this->combo->item_index = 0;

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

        this->child_list.push_back(this->combo);

        but = new widget_button(this->mod,
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

        IniAccounts & acc = this->ini->account;

        struct Widget* login_label = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60, 70, 22),
            this, this->context.get(STRAUTHID_TRANS_LOGIN));

        login_label->id = 100;
        list.push_back(login_label);

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
        list.push_back(login_edit);

        struct Widget* password_label = new widget_label(this->mod,
            Rect(this->rect.cx >= 400 ? 155 : 5, 60 + 25, 70, 22),
            this, this->context.get(STRAUTHID_TRANS_PASSWORD));

        password_label->id = 100 + 2;
        list.push_back(password_label);

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
        list.push_back(password_edit);

        if (acc.username[0]){
            this->focused_control = password_edit;
            password_edit->has_focus = true;
        }

        for (i_tmp = 0; i_tmp < tmp.size() ; i_tmp++){
            list.push_back(tmp[i_tmp]);
            if (tmp[i_tmp] == this->combo){
                list.pop_back();
            }
        }
    }

    ~combo_login(){
        delete this->help;
    }

};


#endif
