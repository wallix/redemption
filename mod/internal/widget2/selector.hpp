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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_HPP

#include "composite.hpp"
#include "multiline.hpp"
#include "button.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "number_edit.hpp"
#include "image.hpp"
#include "region.hpp"
#include "difftimeval.hpp"
#include "tooltip.hpp"
#include "screen.hpp"
class WidgetSelectorImageButton : public Widget2
{
    Bitmap image_inactive;
    Bitmap image_active;
    Bitmap * current_image;
    int state;
    notify_event_t event;

public:
    WidgetSelectorImageButton(DrawApi & drawable, int x, int y, Widget2 & parent,
                              NotifyApi* notifier,
                              uint16_t img_cx, uint16_t img_cy, size_t img_size,
                              const uint8_t * img_data1, const uint8_t * img_data2,
                              int group_id = 0, notify_event_t notify_event = NOTIFY_SUBMIT)
        : Widget2(drawable, Rect(x,y,1,1), parent, notifier, group_id)
        , image_inactive(24, NULL, img_cx, img_cy, img_data1, img_size)
        , image_active(24, NULL, img_cx, img_cy, img_data2, img_size)
        , current_image(&image_inactive)
        , state(0)
        , event(notify_event)
    {
        this->rect.cx = this->image_inactive.cx;
        this->rect.cy = this->image_inactive.cy;
    }

    virtual ~WidgetSelectorImageButton()
    {}

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN) && (this->state & 1) == 0) {
            this->state |= 1;
            this->swap_image();
        }
        else if (device_flags == MOUSE_FLAG_BUTTON1 && this->state & 1) {
            this->state &= ~1;
            this->swap_image();
            if (this->rect.contains_pt(x, y)) {
                this->send_notify(this->event);
            }
        }
        else
            this->Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_ENTER:
                keymap->get_kevent();
                this->send_notify(this->event);
                break;
            case Keymap2::KEVENT_KEY:
                if (keymap->get_char() == ' ') {
                    this->send_notify(this->event);
                }
                break;
            default:
                Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
            }
        }
    }

    virtual void draw(const Rect& clip)
    {
        int16_t mx = std::max<int16_t>(clip.x, 0);
        int16_t my = std::max<int16_t>(clip.y, 0);
        this->drawable.draw(
                             RDPMemBlt(
                                       0,
                                       Rect(mx, my, clip.cx, clip.cy),
                                       0xCC,
                                       mx - this->dx(),
                                       my - this->dy(),
                                       0
                                       ),
                             this->rect,
                             *this->current_image
                             );
    }

    virtual void focus()
    {
        this->swap_image();
        Widget2::focus();
    }

    virtual void blur()
    {
        if (&this->image_active == this->current_image){
            this->swap_image();
        }
        Widget2::blur();
    }

    void swap_image()
    {
        this->current_image
            = (&this->image_active == this->current_image)
            ? &this->image_inactive
            : &this->image_active;
        this->drawable.begin_update();
        WidgetSelectorImageButton::draw(this->rect);
        this->drawable.end_update();
    }
};

class WidgetSelector : public WidgetParent
{
    class Line : public Widget2 {
    public:
        WidgetLabel group;
        WidgetLabel target;
        WidgetLabel protocol;
        WidgetLabel closetime;


        Line(DrawApi & drawable, Widget2& parent, NotifyApi* notifier,
             int x, int y, int lcy,
             int group_w, int target_w, int protocol_w, int closetime_w,
             const char * device_group, const char * target_label,
             const char * protocol, const char * closetime,
             int fgcolor, int bgcolor, int x_text, int y_text)
            : Widget2(drawable, Rect(x, y, group_w + target_w + protocol_w + closetime_w, lcy),
                      parent, notifier)
            , group(WidgetLabel(drawable, x, y,
                                parent, notifier,
                                device_group, false, 0,
                                fgcolor, bgcolor, x_text, y_text))
            , target(WidgetLabel(drawable, x + group_w, y,
                                 parent, notifier,
                                 target_label, false, 0,
                                 fgcolor, bgcolor, x_text, y_text))
            , protocol(WidgetLabel(drawable, x + group_w + target_w, y,
                                   parent, notifier,
                                   protocol, false, 0,
                                   fgcolor, bgcolor, x_text, y_text))
            , closetime(WidgetLabel(drawable, x + group_w + target_w + protocol_w, y,
                                    parent, notifier,
                                    closetime, false, 0,
                                    fgcolor, bgcolor, x_text, y_text))
        {
            this->group.rect.cx     = group_w;
            this->target.rect.cx    = target_w;
            this->protocol.rect.cx  = protocol_w;
            this->closetime.rect.cx = closetime_w;
            this->group.rect.cy     = lcy;
            this->target.rect.cy    = lcy;
            this->protocol.rect.cy  = lcy;
            this->closetime.rect.cy = lcy;
        }

        ~Line() {}

        virtual void draw(const Rect& clip) {
            Rect new_clip = clip.intersect(this->rect);
            this->group.refresh(new_clip.intersect(this->group.rect));
            this->target.refresh(new_clip.intersect(this->target.rect));
            this->protocol.refresh(new_clip.intersect(this->protocol.rect));
            this->closetime.refresh(new_clip.intersect(this->closetime.rect));
        }

        void set_bg_color(int bg_color) {
            this->group.bg_color     = bg_color;
            this->target.bg_color    = bg_color;
            this->protocol.bg_color  = bg_color;
            this->closetime.bg_color = bg_color;
        }
        void set_fg_color(int fg_color) {
            this->group.fg_color     = fg_color;
            this->target.fg_color    = fg_color;
            this->protocol.fg_color  = fg_color;
            this->closetime.fg_color = fg_color;
        }
    };

public:
    typedef enum {
        COLUMN_UNKNOWN,
        COLUMN_GROUP,
        COLUMN_TARGET,
        COLUMN_PROTOCOL,
        COLUMN_CLOSETIME,
    } Column;

private:
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

        int group_w;
        int target_w;
        int protocol_w;
        int closetime_w;
        Column col;

        std::vector<Line*> labels;
        uint over_index;




        struct difftimer {
            uint64_t t;

            difftimer(uint64_t start = 0)
                : t(start)
            {}

            uint64_t tick()
            {
                uint64_t ret = this->t;
                this->t = ustime();
                return this->t - ret;
            }

            void update()
            {
                this->t = ustime();
            }
        } click_interval;

        // TOCHANGE
        WidgetSelectLine(DrawApi& drawable, WidgetSelector& parent,
                         NotifyApi* notifier, int x, int y,
                         int group_w, int target_w, int protocol_w, int closetime_w,
                         int group_id = 0,
                         int fgcolor1 = BLACK, int fgcolor2 = BLACK,
                         int current_fgcolor = BLACK,
                         int bgcolor1 = WABGREEN, int bgcolor2 = GREEN,
                         int current_bgcolor = DARK_GREEN,
                         int xtext = 0, int ytext = 0,
                         int bgcolor = GREY, uint border_height = 1)
            : Widget2(drawable, Rect(x, y, group_w + target_w + protocol_w + closetime_w, 1), parent, notifier, group_id)
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
            , group_w(group_w)
            , target_w(target_w)
            , protocol_w(protocol_w)
            , closetime_w(closetime_w)
            , labels()
            , over_index(-1u)
            , click_interval()
        {
            int w;
            this->drawable.text_metrics("Lp", w, this->h_text);
        }

        void set_x(int x) {
            this->rect.x = x;
        }
        void set_group_w(int w) {
            this->group_w = w;
            this->rect.cx = get_total_w();
        }
        void set_target_w(int w) {
            this->target_w = w;
            this->rect.cx = get_total_w();
        }
        void set_protocol_w(int w) {
            this->protocol_w = w;
            this->rect.cx = get_total_w();
        }
        void set_closetime_w(int w) {
            this->closetime_w = w;
            this->rect.cx = get_total_w();
        }

        int get_target_x() {
            return this->rect.x + this->group_w;
        }
        int get_protocol_x() {
            return this->get_target_x() + this->target_w;
        }
        int get_closetime_x() {
            return this->get_protocol_x() + this->protocol_w;
        }
        int get_total_w() {
            return this->group_w + this->target_w + this->protocol_w + this->closetime_w;
        }


        virtual ~WidgetSelectLine()
        {
            for (size_t i = 0; i < this->labels.size(); ++i) {
                delete this->labels[i];
            }
        }

        const char * get_current_index(Column col) const
        {
            if (this->current_index < this->labels.size()) {
                switch (col) {
                case COLUMN_GROUP:
                    return this->labels[this->current_index]->group.get_text();
                case COLUMN_TARGET:
                    return this->labels[this->current_index]->target.get_text();
                case COLUMN_PROTOCOL:
                    return this->labels[this->current_index]->protocol.get_text();
                case COLUMN_CLOSETIME:
                    return this->labels[this->current_index]->closetime.get_text();
                default:
                    return "";
                    break;
                }
            }
            return "";
        }

        const char * get_over_index() const
        {
            if (this->over_index < this->labels.size()) {
                switch (this->col) {
                case COLUMN_GROUP:
                    return this->labels[this->over_index]->group.get_text();
                case COLUMN_TARGET:
                    return this->labels[this->over_index]->target.get_text();
                case COLUMN_PROTOCOL:
                    return this->labels[this->over_index]->protocol.get_text();
                case COLUMN_CLOSETIME:
                    return this->labels[this->over_index]->closetime.get_text();
                default:
                    return "";
                    break;
                }
            }

            return "";
        }

        void add_line(const char * device_group, const char * target_label,
                      const char * protocol, const char * close_time)
        {
            uint16_t lcy = this->h_text + this->y_text * 2;
            bool b = this->labels.size() & 1;

            Line * line = new Line(this->drawable, *this, NULL,
                                   0, this->labels.size() * (lcy + this->h_border), lcy,
                                   this->group_w, this->target_w, this->protocol_w, this->closetime_w,
                                   device_group, target_label, protocol, close_time,
                                   b ? this->fg_color1 : this->fg_color2,
                                   b ? this->bg_color1 : this->bg_color2,
                                   this->x_text, this->y_text);

            this->labels.push_back(line);
            const uint lines_h = this->labels.size() * (this->h_text + this->y_text * 2 + this->h_border) - this->h_border;
            this->rect.cy = lines_h;
        }

        void clear()
        {
            for (size_t i = 0; i < this->labels.size(); ++i) {
                delete this->labels[i];
            }
            this->labels.clear();
            this->current_index = -1u;
        }

        void init_current_index(uint idx)
        {
            if (idx < this->labels.size()) {
                this->current_index = idx;
                this->labels[idx]->set_bg_color(this->current_bg_color);
                this->labels[idx]->set_fg_color(this->current_fg_color);
            }
        }

        void set_current_index(uint idx)
        {
            if (idx != this->current_index) {
                if (this->current_index < this->labels.size()) {
                    this->labels[this->current_index]->set_bg_color(this->current_index & 1
                                                                    ? this->bg_color1
                                                                    : this->bg_color2);
                    this->labels[this->current_index]->set_fg_color(this->current_index & 1
                                                                    ? this->fg_color1
                                                                    : this->fg_color2);
                    this->refresh(this->labels[this->current_index]->rect);
                }
                this->current_index = idx;
                this->labels[idx]->set_bg_color(this->current_bg_color);
                this->labels[idx]->set_fg_color(this->current_fg_color);
                this->refresh(this->labels[this->current_index]->rect);
            }
        }

        virtual void draw(const Rect& clip)
        {
            std::size_t size = this->labels.size();
            uint lcy = 0;
            for (std::size_t i = 0; i < size; ++i) {
                Line * line = this->labels[i];

                line->refresh(clip);

                lcy += this->h_text + this->y_text * 2;
                if (this->h_border) {
                    this->drawable.draw(
                                        RDPOpaqueRect(
                                                      Rect(
                                                           this->rect.x,
                                                           this->rect.y + lcy,
                                                           this->rect.cx,
                                                           this->h_border
                                                           ),
                                                      this->bg_color
                                                      ), clip
                                        );
                    lcy += this->h_border;
                }

            }

            this->drawable.draw(
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

        Column get_column(int x) {
            if (x >= this->rect.x) {
                if (x <= this->rect.x + this->group_w) {
                    return COLUMN_GROUP;
                }
                else if (x <= this->rect.x + this->group_w + this->target_w) {
                    return COLUMN_TARGET;
                }
                else if (x <= this->rect.x + this->group_w + this->target_w + this->protocol_w) {
                    return COLUMN_PROTOCOL;
                }
                else if (x  <= this->rect.x + this->group_w + this->target_w + this->protocol_w + this->closetime_w) {
                    return COLUMN_CLOSETIME;
                }
            }
            return COLUMN_TARGET;
        }

        int get_column_cx() {
            switch (this->col) {
            case COLUMN_GROUP:
                return this->group_w;
            case COLUMN_TARGET:
                return this->target_w;
            case COLUMN_PROTOCOL:
                return this->protocol_w;
            case COLUMN_CLOSETIME:
                return this->closetime_w;
            default:
                return 0;
                break;
            }
            return 0;
        }

        virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
        {
            if (device_flags == MOUSE_FLAG_BUTTON1) {
                int lcy = (this->h_text + this->y_text * 2 + this->h_border);
                if (int16_t(y) >= this->dy()
                    && y < int(this->dy() + this->labels.size() * lcy - this->h_border)
                    && lcy != 0) {
                    int p = (y - this->dy()) / lcy;
                    if ((uint)p != this->current_index) {
                        this->click_interval.update();
                        this->set_current_index(p);
                        return ;
                    }
                    else {
                        if (this->click_interval.tick() <= 700000LL) {
                            this->send_notify(NOTIFY_SUBMIT);
                            return ;
                        }
                    }
                }
            }
            if (device_flags == MOUSE_FLAG_MOVE) {
                int lcy = (this->h_text + this->y_text * 2 + this->h_border);
                if (int16_t(y) >= this->dy()
                    && y < int(this->dy() + this->labels.size() * lcy - this->h_border)
                    && lcy != 0) {
                    int p = (y - this->dy()) / lcy;
                    Column c = this->get_column(x);
                    if ((uint)p != this->over_index
                        || (c != this->col)
                        || (!this->tooltip_exist())
                        ) {
                        this->over_index = p;
                        this->col = c;
                        int w = 0;
                        int h = 0;
                        this->drawable.text_metrics(this->get_over_index(), w, h);
                        this->show_tooltip(this, NULL, 0, 0);
                        if (w >= this->get_column_cx()) {
                            this->show_tooltip(this, this->get_over_index(), x, y);
                        }
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
                    }
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->labels.size() > 1) {
                        this->set_current_index(this->current_index + 1 != this->labels.size() ? this->current_index + 1 : 0);
                    }
                    break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if (this->labels.size() > 1 && this->labels.size() - 1 != this->current_index) {
                        this->set_current_index(this->labels.size() - 1);
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if (this->labels.size() > 1 && 0 != this->current_index) {
                        this->set_current_index(0);
                    }
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    if (!this->labels.empty()) {
                        this->send_notify(NOTIFY_SUBMIT);
                    }
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
                }
            }
        }
    };

    CompositeInterface * impl;
public:
    WidgetLabel device_label;
    WidgetLabel device_target_label;
    WidgetLabel target_label;
    WidgetLabel protocol_label;
    WidgetLabel close_time_label;

    WidgetSelectLine selector_lines;

    WidgetEdit filter_device;
    WidgetEdit filter_target;
    WidgetEdit filter_proto;
    //BEGIN WidgetPager
    WidgetSelectorImageButton first_page;
    WidgetSelectorImageButton prev_page;
    WidgetNumberEdit current_page;
    WidgetLabel number_page;
    WidgetSelectorImageButton next_page;
    WidgetSelectorImageButton last_page;
    //END WidgetPager
    WidgetSelectorImageButton logout;
    WidgetSelectorImageButton apply;
    WidgetSelectorImageButton connect;
    //WidgetPager pager;

public:
    struct temporary_number_of_page {
        char buffer[15];

        temporary_number_of_page(const char * s)
        {
            size_t len = std::min(sizeof(this->buffer) - 3, strlen(s));
            this->buffer[0] = '/';
            memcpy(&this->buffer[1], s, len);
            this->buffer[len + 1] = '\0';
        }
    };

#include "rawinfo_selector.hpp"

public:
    WidgetSelector(DrawApi& drawable, const char * device_name,
                   uint16_t width, uint16_t height, Widget2 & parent, NotifyApi* notifier,
                   const char * current_page, const char * number_of_page,
                   const char * filter_device = 0, const char * filter_target = 0,
                   const char * filter_proto = 0)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , device_label(drawable, 20, 10, *this, NULL, device_name, true, -10, BLACK, GREY)
        , device_target_label(drawable, 20, 0, *this, NULL, "Device Group", true, -10, BLACK, GREY)
        , target_label(drawable, 150, 0, *this, NULL, "Account Device", true, -10, BLACK, GREY)
        , protocol_label(drawable, 500, 0, *this, NULL, "Protocol", true, -10, BLACK, GREY)
        , close_time_label(drawable, 620, 0, *this, NULL, "Close Time", true, -10, BLACK, GREY)
        , selector_lines(drawable, *this, this, 15, 0, 130, 350, 120, 170, -11,
                         BLACK, BLACK, BLACK, PALE_GREEN, MEDIUM_GREEN, 0X44FFAC, 5, 1, GREY, 1)
        , filter_device(drawable, 20, 0, 120, *this, this, filter_device?filter_device:0, -12, BLACK, WHITE, -1, 1, 1)
        , filter_target(drawable, 150, 0, 340, *this, this, filter_target?filter_target:0, -12, BLACK, WHITE, -1, 1, 1)
        , filter_proto(drawable, 500, 0, 110, *this, this, filter_proto?filter_proto:0, -12, BLACK, WHITE, -1, 1, 1)
          //BEGIN WidgetPager
        , first_page(drawable, 0, 0, *this, notifier,
                     raw_first_page().cx, raw_first_page().cy, raw_first_page().size,
                     raw_first_page().img_blur, raw_first_page().img_focus, -15)
        , prev_page(drawable, 0, 0, *this, notifier,
                    raw_prev_page().cx, raw_prev_page().cy, raw_prev_page().size,
                    raw_prev_page().img_blur, raw_prev_page().img_focus, -15)
        , current_page(drawable, 0, 0, this->first_page.cy(), *this, notifier,
                       current_page ? current_page : "XXXX", -15, BLACK, WHITE, -1, 1, 1)
        , number_page(drawable, 0, 0, *this, NULL,
                      number_of_page ? temporary_number_of_page(number_of_page).buffer : "/XXXX",
                      true, -100, BLACK, GREY)
        , next_page(drawable, 0, 0, *this, notifier,
                    raw_next_page().cx, raw_next_page().cy, raw_next_page().size,
                    raw_next_page().img_blur, raw_next_page().img_focus, -15)
        , last_page(drawable, 0, 0, *this, notifier,
                    raw_last_page().cx, raw_last_page().cy, raw_last_page().size,
                    raw_last_page().img_blur, raw_last_page().img_focus, -15)
          //END WidgetPager
        , logout(drawable, 0, 0, *this, notifier,
                 raw_logout().cx, raw_logout().cy, raw_logout().size,
                 raw_logout().img_blur, raw_logout().img_focus, -16, NOTIFY_CANCEL)
        , apply(drawable, 0, 0, *this, notifier,
                raw_apply().cx, raw_apply().cy, raw_apply().size,
                raw_apply().img_blur, raw_apply().img_focus, -12)
        , connect(drawable, 0, 0, *this, notifier,
                  raw_connect().cx, raw_connect().cy, raw_connect().size,
                  raw_connect().img_blur, raw_connect().img_focus, -18)
    {
        this->tab_flag = DELEGATE_CONTROL_TAB;
        this->impl = new CompositeVector;

        this->add_widget(&this->device_label);
        this->add_widget(&this->device_target_label);
        this->add_widget(&this->target_label);
        this->add_widget(&this->protocol_label);
        this->add_widget(&this->close_time_label);
        this->add_widget(&this->filter_device);
        this->add_widget(&this->filter_target);
        this->add_widget(&this->filter_proto);
        this->add_widget(&this->selector_lines);
        this->add_widget(&this->first_page);
        this->add_widget(&this->prev_page);
        this->add_widget(&this->current_page);
        this->add_widget(&this->number_page);
        this->add_widget(&this->next_page);
        this->add_widget(&this->last_page);
        this->add_widget(&this->logout);
        this->add_widget(&this->apply);
        this->add_widget(&this->connect);

        int dw = width - (this->selector_lines.rect.x + this->selector_lines.get_total_w() + 15);
        if (dw < 0) {

            this->device_target_label.rect.x -= 15;
            this->filter_device.set_edit_x(this->device_target_label.dx());
            this->selector_lines.set_x(this->selector_lines.rect.x - 15);
            this->filter_device.set_edit_cx(this->device_target_label.cx());
            this->selector_lines.set_group_w(this->device_target_label.cx() + 10);

            int w,h;
            this->drawable.text_metrics("INTERNAL", w,h);
            this->selector_lines.set_protocol_w(w + 10);
            this->drawable.text_metrics("XXXX-XX-XX XX:XX:XX", w,h);
            this->selector_lines.set_closetime_w(w + 10);

            this->close_time_label.rect.x = this->selector_lines.get_closetime_x() + 5;
            this->protocol_label.rect.x = this->selector_lines.get_protocol_x() + 5;
            this->filter_proto.set_edit_x(this->protocol_label.dx());
            this->filter_proto.set_edit_cx(this->selector_lines.protocol_w - 10);

            this->target_label.rect.x = this->selector_lines.get_target_x() + 5;
            this->filter_target.set_edit_x(this->target_label.dx());
            this->filter_target.set_edit_cx(this->selector_lines.target_w - 10);

        }
        else if (dw > 0) {
            this->selector_lines.set_target_w(this->selector_lines.target_w + dw);
            this->filter_target.set_edit_cx(this->filter_target.cx() + dw);

            this->protocol_label.rect.x += dw;
            this->close_time_label.rect.x += dw;

            this->filter_proto.set_edit_x(this->protocol_label.dx());
            this->filter_proto.set_edit_cx(this->selector_lines.protocol_w - 10);
        }

        this->device_target_label.rect.y = this->device_label.cy() + this->device_label.dy() + 5;
        this->target_label.rect.y = this->device_target_label.dy();
        this->protocol_label.rect.y = this->device_target_label.dy();
        this->close_time_label.rect.y = this->device_target_label.dy();
        this->filter_device.set_edit_y(this->device_target_label.dy() + this->device_target_label.cy() + 5);
        this->filter_target.set_edit_y(this->filter_device.dy());
        this->filter_proto.set_edit_y(this->filter_device.dy());
        this->selector_lines.rect.y = this->filter_device.dy() + this->filter_device.cy() + 5;

        this->connect.rect.y = this->cy() - (this->logout.cy() + 10);
        this->apply.rect.y = this->connect.dy();
        this->logout.rect.y = this->connect.dy();

        this->last_page.rect.y = this->connect.dy() - (this->last_page.cy() + 10);
        this->next_page.rect.y = this->last_page.dy();
        this->number_page.rect.y = this->last_page.dy() + (this->next_page.cy() - this->number_page.cy()) / 2;
        this->current_page.set_edit_y(this->last_page.dy() + (this->next_page.cy() - this->current_page.cy()) / 2);
        this->prev_page.rect.y = this->last_page.dy();
        this->first_page.rect.y = this->last_page.dy();

        this->connect.rect.x = this->cx() - (this->connect.cx() + 20);
        this->apply.rect.x = this->connect.dx() - (this->apply.cx() + 15);
        this->logout.rect.x = this->apply.dx() - (this->logout.cx() + 15);

        this->last_page.rect.x = this->cx() - (this->last_page.cx() + 20);
        this->next_page.rect.x = this->last_page.dx() - (this->next_page.cx() + 15);
        this->number_page.rect.x = this->next_page.dx() - (this->number_page.cx() + 15);
        this->current_page.set_edit_x(this->number_page.dx() - (this->current_page.cx()));
        this->prev_page.rect.x = this->current_page.dx() - (this->prev_page.cx() + 15);
        this->first_page.rect.x = this->prev_page.dx() - (this->first_page.cx() + 15);
    }

    virtual ~WidgetSelector()
    {
        this->clear();
        if (this->impl) {
            delete this->impl;
            this->impl = NULL;
        }
    }

    virtual void draw(const Rect& clip)
    {
        this->impl->draw(clip);
        this->draw_inner_free(clip.intersect(this->rect), GREY);
    }

    virtual void notify(Widget2* widget, notify_event_t event)
    {
        if (widget->group_id == this->selector_lines.group_id) {
            if (NOTIFY_SUBMIT == event) {
                if (this->notifier) {
                    this->notifier->notify(widget, event);
                }
            }
        }
        else if (widget->group_id == this->apply.group_id) {
            if (this->notifier) {
                this->notifier->notify(widget, event);
            }
        }
        else {
            WidgetParent::notify(widget, event);
        }
    }


    void add_device(const char * device_group, const char * target_label,
                    const char * protocol, const char * close_time)
    {
        this->selector_lines.add_line(device_group, target_label, protocol, close_time);
    }

    virtual void add_widget(Widget2 * w) {
        this->impl->add_widget(w);
    }
    virtual void remove_widget(Widget2 * w) {
        this->impl->remove_widget(w);
    }
    virtual void clear() {
        this->impl->clear();
    }

    virtual void set_xy(int16_t x, int16_t y) {
        int16_t xx = x - this->dx();
        int16_t yy = y - this->dy();
        this->impl->set_xy(xx, yy);
        WidgetParent::set_xy(x, y);
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) {
        if (!this->rect.contains_pt(x, y))
            return 0;
        if (this->current_focus) {
            if (this->current_focus->rect.contains_pt(x, y)) {
                return this->current_focus;
            }
        }
        return this->impl->widget_at_pos(x, y);
    }

    virtual bool next_focus() {
        return this->impl->next_focus(this);
    }

    virtual bool previous_focus() {
        return this->impl->previous_focus(this);
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
