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

#if !defined(__MOD_INTERNAL_WIDGET_WINDOW_LOGIN_HPP__)
#define __MOD_INTERNAL_WIDGET_WINDOW_LOGIN_HPP__

#include "widget.hpp"
#include "internal_mod.hpp"
#include "widget_window.hpp"
#include "widget_button.hpp"
#include "widget_edit.hpp"
#include "widget_label.hpp"
#include "widget_combo.hpp"

struct window_login : public window
{
    protected:
    Widget & notify_to;
    Inifile * ini;
    widget_combo * combo;
    ModContext & context;
    window * help;

    window_login(internal_mod * mod, const Rect & r, ModContext & context, Widget * parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    :   window(mod, r, parent, bg_color, title),
        notify_to(notify_to),
        context(context)
    {
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
        LOG(LOG_INFO, "notify msg = %u", msg);
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

        struct Widget* combo = this->Widget_get_child_by_id(6);

        if (combo != 0) {
            /* get the user typed values */
            i = 100;
            for (;;) {
            TODO(" we should not rely on labels and window ordering for such things but on widget (Widget) identifiers")
                struct Widget* label = this->Widget_get_child_by_id(i);
                LOG(LOG_INFO, "label %i\n", i);
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
            this->mod->signal = BACK_EVENT_2;
            this->mod->event.set();
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
            TODO(" we should not rely on labels and window ordering for such things but on widget (Widget) identifiers")
                if (0 == strcmp(label->caption1, this->context.get(STRAUTHID_TRANS_PASSWORD))){
                        context.cpy(STRAUTHID_PASSWORD, edit->buffer);
                }
                i += 2;
            }
            this->mod->signal = BACK_EVENT_3;
            this->mod->event.set();
        }
        return 0;

    }

    int cancel_clicked()
    {
        this->mod->signal = BACK_EVENT_STOP;
        this->mod->event.set();
        return 0;
    }

};

#endif
