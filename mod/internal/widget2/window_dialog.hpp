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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_DIALOG_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_DIALOG_HPP

#include "widget2_button.hpp"
#include "widget2_multiline.hpp"
#include "window.hpp"

class WindowDialog : public Window
{
public:
    WidgetMultiLine dialog;
    WidgetButton ok;
    WidgetButton * cancel;

    WindowDialog(ModApi* drawable, int16_t x, int16_t y,
                 Widget2* parent, NotifyApi* notifier,
                 const char* caption, const char * text, int group_id = 0,
                 const char * ok_text = "Ok", const char * cancel_text = "Cancel",
                 int fgcolor = BLACK, int bgcolor = GREY,
                 int fgcolorbtn = BLACK, int bgcolorbtn = WHITE)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, group_id)
    , dialog(drawable, 0, 0, this, NULL, text, true, -10, fgcolor, bgcolor, 10, 2)
    , ok(drawable, 0, 0, this, this, ok_text, true, -12, fgcolorbtn, bgcolorbtn, 6, 2)
    , cancel(cancel_text ? new WidgetButton(drawable, 0, 0, this, this, cancel_text, true, -11, fgcolorbtn, bgcolorbtn, 6, 2, NOTIFY_CANCEL) : NULL)
    {
        this->child_list.push_back(&this->dialog);
        this->child_list.push_back(&this->ok);

        int w,h;
        this->drawable->text_metrics(this->titlebar.buffer, w,h);
        int window_size = std::max<int>(w + this->titlebar.x_text + 5 + this->button_close.cx(), this->dialog.cx());

        if (this->cancel) {
            this->child_list.push_back(this->cancel);

            this->rect.cx = std::max<int>(window_size, this->ok.cx() + this->cancel->cx() + 30);
            this->dialog.rect.x += (this->cx() - this->dialog.cx()) / 2;
            this->cancel->set_button_x(this->dx() + this->cx() - (this->cancel->cx() + 10));
            this->ok.set_button_x(this->cancel->dx() - (this->ok.cx() + 10));

            this->resize_titlebar();

            this->rect.cy = this->dialog.cy() + this->titlebar.cy() + 15 + this->ok.cy();
            this->dialog.rect.y += this->titlebar.cy() + 5;
            this->ok.set_button_y(this->dialog.dy() + this->dialog.cy() + 5);
            this->cancel->set_button_y(this->ok.dy());
        }
        else {
            this->rect.cx = std::max<int>(window_size, this->ok.cx() + 20);
            this->dialog.rect.x += (this->cx() - this->dialog.cx()) / 2;
            this->ok.set_button_x(this->dx() + this->cx() - (this->ok.cx() + 10));

            this->resize_titlebar();

            this->rect.cy = this->dialog.cy() + this->titlebar.cy() + 15 + this->ok.cy();
            this->dialog.rect.y += this->titlebar.cy() + 5;
            this->ok.set_button_y(this->dialog.dy() + this->dialog.cy() + 5);
        }
    }

    virtual ~WindowDialog()
    {
        delete this->cancel;
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_SUBMIT);
                    break;
                case Keymap2::KEVENT_ESC:
                    keymap->get_kevent();
                    this->send_notify(this->cancel ? NOTIFY_CANCEL : NOTIFY_SUBMIT);
                    break;
                default:
                    Window::rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
        }
    }
};

#endif
