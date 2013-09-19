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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_DIALOG_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_DIALOG_HPP

#include "widget2_window.hpp"
#include "button.hpp"
#include "multiline.hpp"

class WindowDialog : public Window
{
public:
    WidgetMultiLine dialog;
    WidgetButton ok;
    WidgetButton * cancel;

    WindowDialog(DrawApi& drawable, int16_t x, int16_t y,
                 Widget2 & parent, NotifyApi* notifier,
                 const char* caption, const char * text, int group_id = 0,
                 const char * ok_text = "Ok", const char * cancel_text = "Cancel",
                 int fgcolor = BLACK, int bgcolor = GREY,
                 int fgcolorbtn = BLACK, int bgcolorbtn = WHITE)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, group_id)
    , dialog(drawable, 0, 0, *this, NULL, text, true, -10, fgcolor, bgcolor, 10, 2)
    , ok(drawable, 0, 0, *this, this, ok_text ? ok_text : "Ok", true, -12, fgcolorbtn, bgcolorbtn, 6, 2)
    , cancel(cancel_text ? new WidgetButton(drawable, 0, 0, *this, this, cancel_text, true, -11, fgcolorbtn, bgcolorbtn, 6, 2) : NULL)
    {
        this->add_widget(&this->dialog);
        this->add_widget(&this->ok);

        const int window_size = std::max<int>(this->titlebar_base_width + this->titlebar.x_text + 5 + this->button_close.cx(), this->dialog.cx());

        if (this->cancel) {
            this->add_widget(this->cancel);

            this->set_window_cx(std::max<int>(window_size, this->ok.cx() + this->cancel->cx() + 30));
            this->dialog.rect.x += (this->cx() - this->dialog.cx()) / 2;
            this->cancel->set_button_x(this->dx() + this->cx() - (this->cancel->cx() + 10));
            this->ok.set_button_x(this->cancel->dx() - (this->ok.cx() + 10));

            this->rect.cy = this->dialog.cy() + this->titlebar.cy() + 15 + this->ok.cy();
            this->dialog.rect.y += this->titlebar.cy() + 5;
            this->ok.set_button_y(this->dialog.dy() + this->dialog.cy() + 5);
            this->cancel->set_button_y(this->ok.dy());
        }
        else {
            this->set_window_cx(std::max<int>(window_size, this->ok.cx() + 20));
            this->dialog.rect.x += (this->cx() - this->dialog.cx()) / 2;

            this->set_window_cy(this->titlebar.cy() + this->dialog.cy() + this->ok.cy() + 15);
            this->dialog.rect.y += this->titlebar.cy() + 5;
            this->ok.set_button_x(this->dx() + this->cx() - (this->ok.cx() + 10));
            this->ok.set_button_y(this->dialog.dy() + this->dialog.cy() + 5);
        }
    }

    virtual ~WindowDialog()
    {
        if (this->cancel)
            delete this->cancel;
        this->clear();
    }

    virtual void notify(Widget2* widget, NotifyApi::notify_event_t event) {
        if ((event == NOTIFY_CANCEL) ||
            ((event == NOTIFY_SUBMIT) && (widget == this->cancel))) {
            this->send_notify(NOTIFY_CANCEL);
        }
        else if ((event == NOTIFY_SUBMIT) && (widget == &this->ok)){
            this->send_notify(NOTIFY_SUBMIT);
        }
        else {
            Window::notify(widget, event);
        }
    }

};

#endif
