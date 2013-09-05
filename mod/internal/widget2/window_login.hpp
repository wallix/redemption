/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_LOGIN_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_LOGIN_HPP

#include "version.hpp"
#include "edit.hpp"
#include "label.hpp"
#include "button.hpp"
#include "password.hpp"
#include "widget2_window.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "msgbox.hpp"
#include "screen.hpp"

class WindowLogin : public Window
{
public:
    WidgetImage img;
    WidgetLabel login_label;
    WidgetEdit login_edit;
    WidgetLabel password_label;
    WidgetPassword password_edit;
    WidgetButton ok;
    WidgetButton cancel;
    WidgetButton help;
    MessageBox * window_help;

    WindowLogin(DrawApi& drawable, int16_t x, int16_t y, Widget2 & parent,
                NotifyApi* notifier, const char* caption,
                bool focus_on_password = false,
                int group_id = 0,
                const char * login = 0, const char * password = 0,
                int fgcolor = BLACK, int bgcolor = GREY,
                const char * button_text_ok = "Ok",
                const char * button_text_cancel = "Cancel",
                const char * button_text_help = "Help",
                const char * label_text_login = "Login",
                const char * label_text_password = "Password")
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, group_id)
    , img(drawable, 0, 0, SHARE_PATH "/" LOGIN_LOGO24, *this, NULL, -10)
    , login_label(drawable, this->img.cx() + 20, 0, *this, NULL, label_text_login, true, -11, fgcolor, bgcolor)
    , login_edit(drawable, 0, 0, 350, *this, this, login, -12, BLACK, WHITE, -1u, 1, 1)
    , password_label(drawable, this->img.cx() + 20, 0, *this, NULL, label_text_password, true, -13, fgcolor, bgcolor)
    , password_edit(drawable, 0, 0, 350, *this, this, password, -14, BLACK, WHITE, -1u, 1, 1)
    , ok(drawable, 0, 0, *this, this, button_text_ok, true, -15, BLACK, GREY, 6, 2)
    , cancel(drawable, 0, 0, *this, this, button_text_cancel, true, -16, BLACK, GREY, 6, 2, NOTIFY_CANCEL)
    , help(drawable, 0, 0, *this, this, button_text_help, true, -17, BLACK, GREY, 6, 2)
    , window_help(NULL)
    {
        this->child_list.push_back(&this->login_edit);
        this->child_list.push_back(&this->login_label);
        this->child_list.push_back(&this->password_edit);
        this->child_list.push_back(&this->password_label);
        this->child_list.push_back(&this->ok);
        this->child_list.push_back(&this->cancel);
        this->child_list.push_back(&this->help);
        this->child_list.push_back(&this->img);

        this->ok.border_top_left_color = WHITE;
        this->cancel.border_top_left_color = WHITE;
        this->help.border_top_left_color = WHITE;

        x = this->dx() + 12;
        this->img.rect.x = x;
        x += this->img.cx() + std::max(this->login_label.cx(), this->password_label.cx()) + 20;
        this->login_edit.set_edit_x(x);
        this->password_edit.rect.x = x;
        this->set_window_cx(x - this->dx() + std::max(this->login_edit.cx(), this->password_edit.cx()) + 14);

        x = std::max(std::max(this->help.cx(), this->cancel.cx()), this->ok.cx());
        this->ok.label.x_text += (x - this->ok.cx()) / 2;
        this->cancel.label.x_text += (x - this->cancel.cx()) / 2;
        this->help.label.x_text += (x - this->help.cx()) / 2;
        this->ok.set_button_cx(x);
        this->cancel.set_button_cx(x);
        this->help.set_button_cx(x);

        uint16_t sizex = this->help.cx() + this->cancel.cx() + this->ok.cx() + 40;
        x = this->dx() + (this->cx() - sizex) / 2 + 12;
        this->ok.set_button_x(x);
        x += this->ok.cx() + 10;
        this->cancel.set_button_x(x);
        x += this->cancel.cx() + 10;
        this->help.set_button_x(x);

        this->login_edit.set_edit_cx(this->cx() - (this->login_edit.dx() - this->dx()) - 10);
        this->password_edit.rect.cx = this->login_edit.cx();

        uint16_t rightsize = this->login_edit.cy() * 2 + 10;
        uint16_t maxry = std::max(this->img.cy(), rightsize);
        y = this->titlebar.dy() + this->titlebar.cy();

        this->img.rect.y = y + (maxry - this->img.cy()) / 2;

        y += (maxry - rightsize) / 2;
        this->login_label.rect.y = y + (this->login_edit.cy() - this->login_label.cy()) / 2;
        this->login_edit.set_edit_y(y);
        y += this->login_edit.cy() + 10;
        this->password_label.rect.y = y + (this->password_edit.cy() - this->password_label.cy()) / 2;
        this->password_edit.rect.y = y;

        y = this->titlebar.dy() + this->titlebar.cy() + maxry;
        this->ok.set_button_y(y);
        this->cancel.set_button_y(y);
        this->help.set_button_y(y);

        this->set_window_cy(y + this->ok.cy() + 10 - this->dy());

        this->current_focus = &this->login_edit;
        if (focus_on_password){
            this->current_focus = &this->password_edit;
        }
    }

    virtual ~WindowLogin()
    {
        if (this->window_help){
            delete this->window_help;
            this->window_help = NULL;
        }
        this->child_list.clear();
    }

    virtual void notify(Widget2* widget, NotifyApi::notify_event_t event)
    {
        if (widget == &this->help && event == NOTIFY_SUBMIT) {
            if (&this->parent != this) {
                Widget2 * p = &this->parent;
                if (!this->window_help) {
                    this->window_help =
                        new MessageBox(
                                       this->drawable, 0, 0, *p, this, "Help",
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
                                       "having problems logging on.",
                                       -20, "Ok", this->login_label.fg_color, this->bg_color
                                       );
                    // this->window_help->focus_flag = Widget2::FORCE_FOCUS;

                    this->window_help->ok.label.bg_color = GREY;
                    this->window_help->ok.label.fg_color = BLACK;
                    this->window_help->ok.border_top_left_color = WHITE;

                    int x = (p->cx() - this->window_help->cx()) / 2 - this->window_help->dx();
                    int y = (p->cy() - this->window_help->cy()) / 2 - this->window_help->dy();
                    this->window_help->rect.x += x;
                    this->window_help->rect.y += y;
                    this->window_help->titlebar.rect.x += x;
                    this->window_help->titlebar.rect.y += y;
                    this->window_help->button_close.set_button_x(this->window_help->button_close.dx() + x);
                    this->window_help->button_close.set_button_y(this->window_help->button_close.dy() + y);
                    this->window_help->msg.rect.x += x;
                    this->window_help->msg.rect.y += y;
                    this->window_help->ok.set_button_x(this->window_help->ok.dx() + x);
                    this->window_help->ok.set_button_y(this->window_help->ok.dy() + y);


                    static_cast<WidgetComposite*>(p)->child_list.push_back(this->window_help);
                }
                p->current_focus = this->window_help;

                this->focus_flag = IGNORE_FOCUS;
                // if (this->current_focus){
                //     this->current_focus->blur();
                // }
                // this->current_focus = NULL;
                this->blur();
                this->window_help->current_focus = &this->window_help->ok;
                // this->window_help->ok.focus();
                this->window_help->focus();
                p->refresh(p->rect);
            }
        } else if (widget == this->window_help && event == NOTIFY_CANCEL) {
            this->close_window_help();
        } else if (widget == &this->cancel && event == NOTIFY_CANCEL) {
            if (this->window_help) {
                this->close_window_help();
            }
            this->send_notify(NOTIFY_CANCEL);
        } else {
            Window::notify(widget, event);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) {
        if (this->window_help) {
            return;
        }
        Window::rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_ESC:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_CANCEL);
                    break;
                default:
                    Window::rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
        }
    }

private:
    void close_window_help()
    {
        if (&this->parent != this) {
            WidgetScreen * p = static_cast<WidgetScreen*>(&this->parent);
            p->current_focus = this;
            p->child_list.pop_back();
            delete this->window_help;
            this->window_help = NULL;
            this->current_focus = &this->help;
            this->current_focus->focus();

            this->focus_flag = NORMAL_FOCUS;
            this->focus();
            p->refresh(p->rect);
        }
    }
};

#endif
