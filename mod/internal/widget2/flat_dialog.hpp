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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_DIALOG_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_DIALOG_HPP

#include "composite.hpp"
#include "flat_button.hpp"
#include "multiline.hpp"

class FlatDialog : public WidgetParent
{
public:
    WidgetImage img;
    WidgetLabel title;
    WidgetMultiLine dialog;
    WidgetFlatButton ok;
    WidgetFlatButton * cancel;

    int bgcolor;

    FlatDialog(DrawApi& drawable, int16_t width, int16_t height,
               Widget2 & parent, NotifyApi* notifier,
               const char* caption, const char * text, int group_id = 0,
               const char * ok_text = "Ok", const char * cancel_text = "Cancel",
               int fgcolor = WHITE, int bgcolor = DARK_BLUE_BIS
               )
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , img(drawable, 0, 0, SHARE_PATH "/" LOGIN_WAB_BLUE, *this, NULL, -8)
        , title(drawable, 0, 0, *this, NULL, caption, true, -9,
                              BLACK, LIGHT_BLUE, 5)
        , dialog(drawable, 0, 0, *this, NULL, text, true, -10, fgcolor, bgcolor, 10, 2)
        , ok(drawable, 0, 0, *this, this, ok_text ? ok_text : "Ok", true, -12, fgcolor, bgcolor, 6, 2)
        , cancel(cancel_text ? new WidgetFlatButton(drawable, 0, 0, *this, this, cancel_text, true, -11, fgcolor, bgcolor, 6, 2) : NULL)
        , bgcolor(bgcolor)
    {
        this->impl = new CompositeTable;

        this->add_widget(&this->title);
        this->add_widget(&this->dialog);
        this->add_widget(&this->ok);

        const int total_width = std::max(this->dialog.cx(), this->title.cx());
        const int total_height = this->title.cy() + this->dialog.cy() + this->ok.cy() + 20;
        this->title.rect.x = (this->cx() - total_width) / 2;
        this->title.rect.cx = total_width;
        this->dialog.rect.x = this->title.rect.x;
        this->dialog.rect.y = this->title.cy() + 10;

        if (this->cancel) {
            this->add_widget(this->cancel);

            this->cancel->set_button_x(this->dialog.dx() + this->dialog.cx() - (this->cancel->cx() + 10));
            this->ok.set_button_x(this->cancel->dx() - (this->ok.cx() + 10));

            this->ok.set_button_y(this->dialog.dy() + this->dialog.cy() + 15);
            this->cancel->set_button_y(this->ok.dy());
        }
        else {
            this->ok.set_button_x(this->dialog.dx() + this->dialog.cx() - (this->ok.cx() + 10));
            this->ok.set_button_y(this->dialog.dy() + this->dialog.cy() + 15);
        }
        this->impl->move_xy(0, (height - total_height) /2);

        this->img.rect.x = (this->cx() - this->img.cx()) / 2;
        this->img.rect.y = (3*(height - total_height) / 2 - this->img.cy()) / 2 + total_height;
        this->add_widget(&this->img);

    }

    virtual ~FlatDialog()
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
            WidgetParent::notify(widget, event);
        }
    }

    virtual void draw(const Rect& clip)
    {
        this->impl->draw(clip);
        this->draw_inner_free(clip.intersect(this->rect), this->bgcolor);
    }

    virtual void draw_inner_free(const Rect& clip, int bg_color) {
        Region region;
        region.rects.push_back(clip);

        this->impl->draw_inner_free(clip, bg_color, region);

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable.draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }
    }

};

#endif
