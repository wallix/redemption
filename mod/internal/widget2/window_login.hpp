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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_LOGIN_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_LOGIN_HPP

#include "edit.hpp"
#include "label.hpp"
#include "button.hpp"
#include "password.hpp"
#include "window.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "msgbox.hpp"

class WindowLogin : public Window
{
public:
    WidgetImage img;
    WidgetEdit login_edit;
    WidgetLabel login_label;
    WidgetPassword password_edit;
    WidgetLabel password_label;
    WidgetButton ok;
    WidgetButton cancel;
    WidgetButton help;
    MessageBox * window_help;

    WindowLogin(ModApi* drawable, int16_t x, int16_t y, Widget* parent,
                NotifyApi* notifier, const char* caption, int id = 0,
                const char * login = 0, const char * password = 0,
                int bgcolor = DARK_WABGREEN, int fgcolor = BLACK)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, id)
    , img(drawable, 0, 0, SHARE_PATH "/" LOGIN_LOGO24, this, NULL, -17)
    , login_edit(drawable, 0, 0, 200, this, NULL, login, -10, WHITE, BLACK, -1u, 1, 1)
    , login_label(drawable, this->img.cx() + 20, 0, this, NULL, "login:", true, -11, bgcolor, fgcolor)
    , password_edit(drawable, 0, 0, 200, this, NULL, password, -12, WHITE, BLACK, -1u, 1, 1)
    , password_label(drawable, this->img.cx() + 20, 0, this, NULL, "password:", true, -13, bgcolor, fgcolor)
    , ok(drawable, 0, 0, this, this, "Ok", true, -14, WHITE, BLACK, 6, 2)
    , cancel(drawable, 0, 0, this, this, "Cancel", true, -15, WHITE, BLACK, 6, 2)
    , help(drawable, 0, 0, this, this, "Help", true, -16, WHITE, BLACK, 6, 2)
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

        x = this->dx() + 10;
        this->img.rect.x = x;
        x += this->img.cx() + std::max(this->login_label.cx(), this->password_label.cx()) + 20;
        this->login_edit.set_edit_x(x);
        this->password_edit.rect.x = x;
        this->rect.cx = x - this->dx() + std::max(this->login_edit.cx(), this->password_edit.cx()) + 10;
        this->help.set_button_x(this->cx() - this->help.cx() - 10);
        this->cancel.set_button_x(this->help.dx() - this->cancel.cx() - 10);
        this->ok.set_button_x(this->cancel.dx() - this->ok.cx() - 10);

        this->login_edit.set_edit_cx(this->cx() - (this->login_edit.dx() - this->dx()) - 10);
        this->password_edit.rect.cx = this->login_edit.cx();

        this->resize_titlebar();

        y = this->dy() + this->titlebar.cy() + 10;
        this->img.rect.y = y;
        this->login_label.rect.y = y + (this->login_edit.cy() - this->login_label.cy()) / 2;
        this->login_edit.set_edit_y(y);
        y += this->login_label.cy() * 2;
        this->password_label.rect.y = y + (this->password_edit.cy() - this->password_label.cy()) / 2;
        this->password_edit.rect.y = y;
        y += this->password_label.cy() * 2;
        this->ok.set_button_y(y);
        this->cancel.set_button_y(y);
        this->help.set_button_y(y);
        this->rect.cy = y + this->ok.cy() + 10 - this->dy();
        int d = (this->cy() - this->titlebar.cy() - (this->img.cy() + 20)) / 2;
        if (d < 0) {
            d = -d;
            this->login_label.rect.y += d;
            this->login_edit.set_edit_y(this->login_edit.dy() + d);
            this->password_label.rect.y += d;
            this->password_edit.set_password_y(this->password_edit.dy() + d);
            this->ok.set_button_y(this->ok.dy() + d);
            this->cancel.set_button_y(this->cancel.dy() + d);
            this->help.set_button_y(this->help.dy() + d);
            this->rect.cy = this->titlebar.cy() + this->img.cy() + 20;
        } else {
            this->img.rect.x += d;
        }
    }

    virtual ~WindowLogin()
    {
        if (this->window_help)
            this->close_window_help();
    }

    virtual void notify(Widget* widget, NotifyApi::notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        if (widget == &this->help) {
            if (this->parent) {
                Widget * p = this->parent;
                while (p->parent)
                    p = p->parent;
                this->window_help = new MessageBox(
                    this->drawable, 0, 0, p, this, "Help",
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
                    -20, DARK_WABGREEN, BLACK
                );

                this->window_help->ok.label.bg_color = WHITE;
                this->window_help->ok.label.fg_color = BLACK;

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
        } else if (widget == this->window_help) {
            this->close_window_help();
            this->window_help = 0;
        } else if (event == NOTIFY_CANCEL) {
            if (this->window_help) {
                this->close_window_help();
                this->window_help = 0;
            }
            this->send_notify(NOTIFY_CANCEL);
        } else {
            Window::notify(widget, event, param, param2);
        }
    }

private:
    void close_window_help()
    {
        std::vector<Widget*>& widgets = static_cast<WidgetComposite*>(this->window_help->parent)->child_list;
        for (size_t i = 0; i < widgets.size(); ++i) {
            if (widgets[i] == this->window_help) {
                widgets[i] = widgets[widgets.size()-1];
                widgets.pop_back();
                break;
            }
        }
        this->window_help->parent->draw(this->window_help->rect);
        delete this->window_help;
    }
};

#endif