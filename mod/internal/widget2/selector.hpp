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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_HPP

#include "widget_composite.hpp"
#include "multiline.hpp"
#include "button.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "number_edit.hpp"
#include <region.hpp>
//#include "pager.hpp"
//#include "selectline.hpp"

class WidgetSelectLine : public Widget2
{
public:
    uint current_index;
    int current_bg_color;
    int bg_color;
    uint h_border;
    int bg_color1;
    int bg_color2;
    int current_fg_color;
    int fg_color1;
    int fg_color2;
    int h_text;
    int x_text;
    int y_text;
    std::vector<WidgetLabel*> labels;

    WidgetSelectLine(ModApi* drawable, const Rect& rect,
                     Widget2* parent, NotifyApi* notifier, int group_id = 0,
                     int fgcolor1 = BLACK, int fgcolor2 = BLACK,
                     int current_fgcolor = BLACK,
                     int bgcolor1 = WABGREEN, int bgcolor2 = GREEN,
                     int current_bgcolor = DARK_GREEN,
                     int xtext = 0, int ytext = 0,
                     int bgcolor = GREY, uint border_height = 1)
    : Widget2(drawable, rect, parent, notifier, group_id)
    , current_index(-1u)
    , current_bg_color(current_bgcolor)
    , bg_color(bgcolor)
    , h_border(border_height)
    , bg_color1(bgcolor1)
    , bg_color2(bgcolor2)
    , current_fg_color(current_fgcolor)
    , fg_color1(fgcolor1)
    , fg_color2(fgcolor2)
    , h_text(0)
    , x_text(xtext)
    , y_text(ytext)
    , labels()
    {
        if (this->drawable) {
            int w;
            this->drawable->text_metrics("Lp", w, this->h_text);
        }
    }

    virtual ~WidgetSelectLine()
    {
        for (size_t i = 0; i < this->labels.size(); ++i) {
            delete this->labels[i];
        }
    }

    void add_line(const char * line)
    {
        uint16_t lcy = this->h_text + this->y_text * 2;
        bool b = this->labels.size() & 1;
        WidgetLabel * label = new WidgetLabel(
            this->drawable,
            0,
            this->labels.size() * (lcy + this->h_border),
            this, NULL, line, false, 0,
            b ? this->fg_color1 : this->fg_color2,
            b ? this->bg_color1 : this->bg_color2,
            this->x_text, this->y_text
        );
        label->rect.cx = this->cx();
        label->rect.cy = lcy;
        this->labels.push_back(label);
    }

    void clear()
    {
        for (size_t i = 0; i < this->labels.size(); ++i) {
            delete this->labels[i];
        }
        this->labels.clear();
    }

    void init_current_index(uint idx)
    {
        this->current_index = idx;
        this->labels[idx]->bg_color = this->current_bg_color;
        this->labels[idx]->fg_color = this->current_fg_color;
    }

    void set_current_index(uint idx)
    {
        if (idx != this->current_index) {
            if (this->current_index < this->labels.size()) {
                this->labels[this->current_index]->bg_color = this->current_index & 1
                ? this->bg_color1 : this->bg_color2;
                this->labels[this->current_index]->fg_color = this->current_index & 1
                ? this->fg_color1 : this->fg_color2;
                this->refresh(this->labels[this->current_index]->rect);
            }
            this->current_index = idx;
            this->labels[idx]->bg_color = this->current_bg_color;
            this->labels[idx]->fg_color = this->current_fg_color;
            this->refresh(this->labels[this->current_index]->rect);
        }
    }

    virtual void draw(const Rect& clip)
    {
        Rect new_clip = clip.intersect(this->rect);
        std::size_t size = this->labels.size();

        for (std::size_t i = 0; i < size; ++i) {
            Widget2 * w = this->labels[i];
            Rect rect = new_clip.intersect(w->rect);

            w->refresh(rect);

            if (this->h_border) {
                this->drawable->draw(
                    RDPOpaqueRect(
                        Rect(
                            this->rect.x,
                            w->dy() + w->cy(),
                            rect.cx,
                            this->h_border
                        ),
                        this->bg_color
                    ), clip
                );
            }
        }

        uint lcy = size * (this->h_text + this->y_text * 2 + this->h_border);
        this->drawable->draw(
            RDPOpaqueRect(
                Rect(
                    this->rect.x,
                    this->rect.y + lcy,
                    this->rect.cx,
                    this->rect.cy - lcy
                ),
                this->bg_color
            ), clip
        );
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == MOUSE_FLAG_BUTTON1) {
            int lcy = (this->h_text + this->y_text * 2 + this->h_border);
            if (int16_t(y) >= this->dy()
             && y < int(this->dy() + this->labels.size() * lcy - this->h_border)
             && (y - this->dy()) % lcy != 0) {
                int p = (y - this->dy()) / lcy;
                if ((uint)p != this->current_index) {
                    int old_current_index = this->current_index;
                    this->set_current_index(p);
                    this->send_notify(NOTIFY_SELECTION_CHANGED,
                                      old_current_index, this->current_index);
                    return ;
                }
            }
        }
        Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    keymap->get_kevent();
                    if (this->labels.size() > 1) {
                        this->set_current_index(this->current_index ? this->current_index - 1 : this->labels.size() - 1);
                        this->send_notify(NOTIFY_SELECTION_CHANGED, this->current_index);
                    }
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->labels.size() > 1) {
                        this->set_current_index(this->current_index + 1 != this->labels.size() ? this->current_index + 1 : 0);
                        this->send_notify(NOTIFY_SELECTION_CHANGED, this->current_index);
                    }
                    break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if (this->labels.size() > 1 && this->labels.size() - 1 != this->current_index) {
                        this->set_current_index(this->labels.size() - 1);
                        this->send_notify(NOTIFY_SELECTION_CHANGED, this->current_index);
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if (this->labels.size() > 1 && 0 != this->current_index) {
                        this->set_current_index(0);
                        this->send_notify(NOTIFY_SELECTION_CHANGED, this->current_index);
                    }
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_SUBMIT);
                    break;
                default:
                    break;
            }
        }
    }
};

class WidgetSelector : public WidgetComposite
{
public:
    WidgetLabel device_label;
    WidgetLabel device_group_label;
    WidgetLabel account_device_label;
    WidgetLabel protocol_label;
    WidgetLabel close_time_label;
    WidgetSelectLine device_group_lines;
    WidgetSelectLine account_device_lines;
    WidgetSelectLine protocol_lines;
    WidgetSelectLine close_time_lines;
    WidgetEdit filter_device_group;
    WidgetEdit filter_account_device;
    WidgetButton first_page;
    WidgetButton prev_page;
    WidgetNumberEdit current_page;
    WidgetLabel number_page;
    WidgetButton next_page;
    WidgetButton last_page;
    WidgetButton logout;
    WidgetButton apply;
    WidgetButton connect;
    //WidgetPager pager;

    Widget2 * widget_focused;

private:
    struct temporary_number_of_page {
        char buffer[15];

        temporary_number_of_page(const char * s)
        {
            size_t len = std::min(size_t(15 - 2), strlen(s));
            this->buffer[0] = '/';
            memcpy(&this->buffer[1], s, len);
            this->buffer[len + 1] = '\0';
        }
    };

public:
    WidgetSelector(ModApi* drawable, const char * device_name,
                   uint16_t width, uint16_t height, NotifyApi* notifier,
                   const char * current_page, const char * number_of_page)
    : WidgetComposite(drawable, Rect(0,0,width,height), NULL, notifier)
    , device_label(drawable, 20, 10, this, NULL, device_name, true, -10, BLACK, GREY)
    , device_group_label(drawable, 20, 0, this, NULL, "Device Group", true, -10, BLACK, GREY)
    , account_device_label(drawable, 150, 0, this, NULL, "Account Device", true, -10, BLACK, GREY)
    , protocol_label(drawable, 500, 0, this, NULL, "Protocol", true, -10, BLACK, GREY)
    , close_time_label(drawable, 620, 0, this, NULL, "Close Time", true, -10, BLACK, GREY)
    , device_group_lines(drawable, Rect(15,0,130,1), this, this, -11,
                         BLACK, BLACK, BLACK, 0xEEFAEE, 0xCCEEDD, 0XAAFFAA, 5, 1, GREY, 1)
    , account_device_lines(drawable, Rect(145,0,350,1), this, this, -11,
                           BLACK, BLACK, BLACK, 0xEEFAEE, 0xCCEEDD, 0XAAFFAA, 5, 1, GREY, 1)
    , protocol_lines(drawable, Rect(495,0,120,1), this, this, -11,
                     BLACK, BLACK, BLACK, 0xEEFAEE, 0xCCEEDD, 0XAAFFAA, 5, 1, GREY, 1)
    , close_time_lines(drawable, Rect(615,0,170,1), this, this, -11,
                       BLACK, BLACK, BLACK, 0xEEFAEE, 0xCCEEDD, 0XAAFFAA, 5, 1, GREY, 1)
    , filter_device_group(drawable, 20, 0, 120, this, this, NULL, -12, BLACK, WHITE, -1, 1, 1)
    , filter_account_device(drawable, 150, 0, 340, this, this, NULL, -12, BLACK, WHITE, -1, 1, 1)
    //BEGIN WidgetPager
    , first_page(drawable, 0, 0, this, notifier, "<<", true, -15, BLACK, WHITE, 8, 4)
    , prev_page(drawable, 0, 0, this, notifier, "<", true, -15, BLACK, WHITE, 8, 4)
    , current_page(drawable, 0, 0, this->first_page.cy(), this, notifier,
                   current_page, -15, BLACK, WHITE, -1, 1, 1)
    , number_page(drawable, 0, 0, this, NULL, temporary_number_of_page(number_of_page).buffer,
                  true, -15, BLACK, GREY)
    , next_page(drawable, 0, 0, this, notifier, ">>", true, -15, BLACK, WHITE, 8, 4)
    , last_page(drawable, 0, 0, this, notifier, ">", true, -15, BLACK, WHITE, 8, 4)
    //END WidgetPager
    , logout(drawable, 0, 0, this, notifier, "Logout", true, -16, BLACK, WHITE, 8, 4)
    , apply(drawable, 0, 0, this, notifier, "Appy", true, -17, BLACK, WHITE, 8, 4)
    , connect(drawable, 0, 0, this, notifier, "Connect", true, -18, BLACK, WHITE, 8, 4)
    , widget_focused(&this->filter_account_device/*device_group_lines*/)
    {
        this->child_list.push_back(&this->device_label);
        this->child_list.push_back(&this->device_group_label);
        this->child_list.push_back(&this->account_device_label);
        this->child_list.push_back(&this->protocol_label);
        this->child_list.push_back(&this->close_time_label);
        this->child_list.push_back(&this->device_group_lines);
        this->child_list.push_back(&this->account_device_lines);
        this->child_list.push_back(&this->protocol_lines);
        this->child_list.push_back(&this->close_time_lines);
        this->child_list.push_back(&this->filter_device_group);
        this->child_list.push_back(&this->filter_account_device);
        this->child_list.push_back(&this->first_page);
        this->child_list.push_back(&this->prev_page);
        this->child_list.push_back(&this->current_page);
        this->child_list.push_back(&this->number_page);
        this->child_list.push_back(&this->next_page);
        this->child_list.push_back(&this->last_page);
        this->child_list.push_back(&this->logout);
        this->child_list.push_back(&this->apply);
        this->child_list.push_back(&this->connect);
        //this->child_list.push_back(&this->pager);

        this->device_group_label.rect.y = this->device_label.cy() + this->device_label.dy() + 5;
        this->account_device_label.rect.y = this->device_group_label.dy();
        this->protocol_label.rect.y = this->device_group_label.dy();
        this->close_time_label.rect.y = this->device_group_label.dy();
        this->filter_device_group.set_edit_y(this->device_group_label.dy() + this->device_group_label.cy() + 5);
        this->filter_account_device.set_edit_y(this->filter_device_group.dy());
        this->device_group_lines.rect.y = this->filter_device_group.dy() + this->filter_device_group.cy() + 5;
        this->account_device_lines.rect.y = this->device_group_lines.dy();
        this->protocol_lines.rect.y = this->device_group_lines.dy();
        this->close_time_lines.rect.y = this->device_group_lines.dy();

        this->connect.set_button_y(this->cy() - (this->logout.cy() + 10));
        this->apply.set_button_y(this->connect.dy());
        this->logout.set_button_y(this->connect.dy());

        this->connect.set_button_x(this->cx() - (this->connect.cx() + 20));
        this->apply.set_button_x(this->connect.dx() - (this->apply.cx() + 15));
        this->logout.set_button_x(this->apply.dx() - (this->logout.cx() + 15));

        this->last_page.set_button_y(this->connect.dy() - (this->last_page.cy() + 10));
        this->next_page.set_button_y(this->last_page.dy());
        this->number_page.rect.y = this->last_page.dy() + (this->next_page.cy() - this->number_page.cy()) / 2;
        this->current_page.set_edit_y(this->last_page.dy() + (this->next_page.cy() - this->current_page.cy()) / 2);
        this->prev_page.set_button_y(this->last_page.dy());
        this->first_page.set_button_y(this->last_page.dy());

        this->last_page.set_button_x(this->cx() - (this->last_page.cx() + 20));
        this->next_page.set_button_x(this->last_page.dx() - (this->next_page.cx() + 15));
        this->number_page.rect.x = this->next_page.dx() - (this->number_page.cx() + 15);
        this->current_page.set_edit_x(this->number_page.dx() - (this->current_page.cx()));
        this->prev_page.set_button_x(this->current_page.dx() - (this->prev_page.cx() + 15));
        this->first_page.set_button_x(this->prev_page.dx() - (this->first_page.cx() + 15));
    }

    virtual ~WidgetSelector()
    {}

    virtual void draw(const Rect& clip)
    {
        this->WidgetComposite::draw(clip);
        Rect new_clip = clip.intersect(this->rect);
        Region region;
        region.rects.push_back(new_clip);

        for (std::size_t i = 0, size = this->child_list.size(); i < size; ++i) {
            Rect rect = new_clip.intersect(this->child_list[i]->rect);

            if (!rect.isempty()) {
                region.subtract_rect(rect);
            }
        }

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable->draw(RDPOpaqueRect(region.rects[i], GREY),
                                 region.rects[i]);
        }
    }

    virtual void notify(Widget2* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (widget->group_id == this->device_group_lines.group_id) {
            if (NOTIFY_SUBMIT == event) {
                this->send_notify(NOTIFY_SUBMIT);
            }
            else {
                this->set_index_list(static_cast<WidgetSelectLine*>(widget)->current_index);
            }
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            Widget2 * w = this->child_at_pos(x,y);
            if (w && (
                w->group_id == this->filter_account_device.group_id
             || w->group_id == this->device_group_lines.group_id
             || w == &this->current_page)) {
                this->widget_focused = w;
                w->rdp_input_mouse(device_flags, x, y, keymap);
                return ;
            }
            else {
                this->widget_focused = NULL;
            }
        }
        WidgetComposite::rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (this->widget_focused) {
            this->drawable->begin_update();
            this->widget_focused->rdp_input_scancode(param1, param2, param3, param4, keymap);
            this->drawable->end_update();
        }
    }

    void set_index_list(int idx)
    {
        this->device_group_lines.set_current_index(idx);
        this->account_device_lines.set_current_index(idx);
        this->protocol_lines.set_current_index(idx);
        this->close_time_lines.set_current_index(idx);
    }

    void add_device(const char * device_group, const char * account_device,
                    const char * protocol, const char * close_time)
    {
        this->device_group_lines.add_line(device_group);
        this->account_device_lines.add_line(account_device);
        this->protocol_lines.add_line(protocol);
        this->close_time_lines.add_line(close_time);
        const uint lcy = this->device_group_lines.labels.size() * (this->device_group_lines.h_text + this->device_group_lines.y_text * 2 + this->device_group_lines.h_border) - this->device_group_lines.h_border;
        this->device_group_lines.rect.cy = lcy;
        this->account_device_lines.rect.cy = lcy;
        this->protocol_lines.rect.cy = lcy;
        this->close_time_lines.rect.cy = lcy;
    }
};

#endif