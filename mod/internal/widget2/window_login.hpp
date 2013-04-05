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

class WindowHelp : public Window
{
public:
    WidgetMultiLine help;

    WindowHelp(ModApi* drawable, Widget* parent, NotifyApi* notifier, const char* caption, int id = 0, int bgcolor = DARK_WABGREEN, int fgcolor = BLACK)
    : Window(drawable, Rect(), parent, notifier, caption, bgcolor, id)
    , help(drawable, 0, 0, this, notifier,
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
           "having problems logging on.<br>",
           true, -15, bgcolor, fgcolor, 10, 2)
    {
        this->child_list.push_back(&this->help);

        this->rect.cx = this->help.cx();
        this->rect.x = (parent->cx() - this->cx()) / 2;

        this->resize_titlebar();

        this->rect.cy = this->help.cy() + this->titlebar.dy();
        this->rect.y = (parent->cy() - this->cy()) / 2;
        this->help.rect.x = this->dx();
        this->help.rect.y = this->dy() + this->titlebar.dy();
    }

    virtual ~WindowHelp()
    {}
};

class WindowLogin : public Window
{
public:
    WidgetEdit login_edit;
    WidgetLabel login_label;
    WidgetPassword password_edit;
    WidgetLabel password_label;
    WidgetButton ok;
    WidgetButton cancel;
    WidgetButton help;
    WindowHelp * window_help;

    WindowLogin(ModApi* drawable, int16_t x, int16_t y, Widget* parent,
                NotifyApi* notifier, const char* caption, int id = 0,
                const char * login = 0, const char * password = 0,
                int bgcolor = DARK_WABGREEN, int fgcolor = BLACK)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, id)
    , login_edit(drawable, 0, 0, 100, this, NULL, login, -10, WHITE, BLACK, -1u, 1, 1)
    , login_label(drawable, 10, 0, this, NULL, "login:", true, -11, bgcolor, fgcolor)
    , password_edit(drawable, 0, 0, 100, this, NULL, password, -12, WHITE, BLACK, -1u, 1, 1)
    , password_label(drawable, 10, 0, this, NULL, "password:", true, -13, bgcolor, fgcolor)
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

        x = this->dx() + std::max(this->login_label.cx(), this->password_label.cx()) + 20;
        this->login_edit.set_edit_x(x);
        this->password_edit.rect.x = x;
        this->rect.cx = x + std::max(this->login_edit.cx(), this->password_edit.cx()) + 10;
        this->help.set_button_x(this->rect.cx - this->help.cx());
        this->cancel.set_button_x(this->help.dx() - this->cancel.cx() - 10);
        this->ok.set_button_x(this->cancel.dx() - this->ok.cx() - 10);

        this->login_edit.set_edit_cx(this->cx() - (this->login_edit.dx() - this->dx()) - 10);
        this->password_edit.rect.cx = this->login_edit.cx();

        this->resize_titlebar();

        y = this->dy() + this->titlebar.cy() + 10;
        this->login_label.rect.y = y + (this->login_edit.cy() - this->login_label.cy()) / 2;
        this->login_edit.set_edit_y(y);
        y += this->login_label.cy() * 2;
        this->password_label.rect.y = y + (this->password_edit.cy() - this->password_label.cy()) / 2;
        this->password_edit.rect.y = y;
        y += this->password_label.cy() * 2;
        this->ok.set_button_y(y);
        this->cancel.set_button_y(y);
        this->help.set_button_y(y);
        this->rect.cy = y + this->ok.cy() + 5 - this->dy();
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
                this->window_help = new WindowHelp(this->drawable, p, this, "Help");
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
        delete this->window_help;
    }
};

#endif