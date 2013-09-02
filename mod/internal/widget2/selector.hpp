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
//#include "pager.hpp"
//#include "selectline.hpp"
#include "difftimeval.hpp"
#include "tooltip.hpp"

class WidgetSelectorImageButton : public Widget2
{
    Bitmap image_inactive;
    Bitmap image_active;
    Bitmap * current_image;
    int state;
    notify_event_t event;

public:
    WidgetSelectorImageButton(DrawApi & drawable, int x, int y, Widget2* parent,
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

class WidgetSelector : public WidgetComposite
{
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

        WidgetSelectLine(DrawApi& drawable, const Rect& rect,
                         WidgetSelector* parent, NotifyApi* notifier, int group_id = 0,
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
            , over_index(-1u)
            , click_interval()
        {
            int w;
            this->drawable.text_metrics("Lp", w, this->h_text);
        }

        virtual ~WidgetSelectLine()
        {
            for (size_t i = 0; i < this->labels.size(); ++i) {
                delete this->labels[i];
            }
        }

        const char * get_current_index() const
        {
            if (this->current_index < this->labels.size()) {
                return this->labels[this->current_index]->get_text();
            }
            return "";
        }
        const char * get_over_index() const
        {
            if (this->over_index < this->labels.size()) {
                return this->labels[this->over_index]->get_text();
            }
            return "";
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
            this->current_index = -1u;
        }

        void init_current_index(uint idx)
        {
            if (idx < this->labels.size()) {
                this->current_index = idx;
                this->labels[idx]->bg_color = this->current_bg_color;
                this->labels[idx]->fg_color = this->current_fg_color;
            }
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
            std::size_t size = this->labels.size();

            for (std::size_t i = 0; i < size; ++i) {
                Widget2 * w = this->labels[i];

                w->refresh(clip);

                if (this->h_border) {
                    this->drawable.draw(
                                        RDPOpaqueRect(
                                                      Rect(
                                                           this->rect.x,
                                                           w->dy() + w->cy(),
                                                           this->rect.cx,
                                                           this->h_border
                                                           ),
                                                      this->bg_color
                                                      ), clip
                                        );
                }

            }

            uint lcy = size * (this->h_text + this->y_text * 2 + this->h_border);
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
                        int old_current_index = this->current_index;
                        this->set_current_index(p);
                        this->send_notify(NOTIFY_SELECTION_CHANGED,
                                          old_current_index, this->current_index);
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
                    WidgetSelector * selector = reinterpret_cast<WidgetSelector*>(this->parent);
                    if ((uint)p != this->over_index
                        || (selector->tooltip == NULL)) {
                        this->over_index = p;
                        int w = 0;
                        int h = 0;
                        this->drawable.text_metrics(this->get_over_index(), w, h);
                        this->send_notify(NOTIFY_HIDE_TOOLTIP);
                        if (w >= this->rect.cx) {
                            int sw = selector->rect.cx;
                            int posx = ((x + w) > sw)?(sw - w):x;
                            int posy = (y > h)?(y - h):0;
                            selector->tooltip = new WidgetTooltip(this->drawable,
                                                                  posx,
                                                                  posy,
                                                                  selector, this,
                                                                  this->get_over_index());
                            this->send_notify(NOTIFY_SHOW_TOOLTIP);
                        }
                        // LOG(LOG_INFO, "THIS TOOLTIP %s", this->get_over_index());
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




    TODO("WidgetSelector is not a Composite but has a Composite "
         "We should use Composition instead of inheritance.");
public:
    WidgetLabel device_label;
    WidgetLabel device_target_label;
    WidgetLabel target_label;
    WidgetLabel protocol_label;
    WidgetLabel close_time_label;
    WidgetSelectLine device_lines;
    WidgetSelectLine target_lines;
    WidgetSelectLine protocol_lines;
    WidgetSelectLine close_time_lines;
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

    WidgetTooltip * tooltip;

    Widget2 * w_over;
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
                   uint16_t width, uint16_t height, NotifyApi* notifier,
                   const char * current_page, const char * number_of_page,
                   const char * filter_device = 0, const char * filter_target = 0,
                   const char * filter_proto = 0)
        : WidgetComposite(drawable, Rect(0, 0, width, height), NULL, notifier)
        , device_label(drawable, 20, 10, this, NULL, device_name, true, -10, BLACK, GREY)
        , device_target_label(drawable, 20, 0, this, NULL, "Device Group", true, -10, BLACK, GREY)
        , target_label(drawable, 150, 0, this, NULL, "Account Device", true, -10, BLACK, GREY)
        , protocol_label(drawable, 500, 0, this, NULL, "Protocol", true, -10, BLACK, GREY)
        , close_time_label(drawable, 620, 0, this, NULL, "Close Time", true, -10, BLACK, GREY)
        , device_lines(drawable, Rect(15,0,130,1), this, this, -11,
                       BLACK, BLACK, BLACK, PALE_GREEN, MEDIUM_GREEN, 0X44FFAC, 5, 1, GREY, 1)
        , target_lines(drawable, Rect(145,0,350,1), this, this, -11,
                       BLACK, BLACK, BLACK, PALE_GREEN, MEDIUM_GREEN, 0X44FFAC, 5, 1, GREY, 1)
        , protocol_lines(drawable, Rect(495,0,120,1), this, this, -11,
                         BLACK, BLACK, BLACK, PALE_GREEN, MEDIUM_GREEN, 0X44FFAC, 5, 1, GREY, 1)
        , close_time_lines(drawable, Rect(615,0,170,1), this, this, -11,
                           BLACK, BLACK, BLACK, PALE_GREEN, MEDIUM_GREEN, 0X44FFAC, 5, 1, GREY, 1)
        , filter_device(drawable, 20, 0, 120, this, this, filter_device?filter_device:0, -12, BLACK, WHITE, -1, 1, 1)
        , filter_target(drawable, 150, 0, 340, this, this, filter_target?filter_target:0, -12, BLACK, WHITE, -1, 1, 1)
        , filter_proto(drawable, 500, 0, 110, this, this, filter_proto?filter_proto:0, -12, BLACK, WHITE, -1, 1, 1)
          //BEGIN WidgetPager
        , first_page(drawable, 0, 0, this, notifier,
                     raw_first_page().cx, raw_first_page().cy, raw_first_page().size,
                     raw_first_page().img_blur, raw_first_page().img_focus, -15)
        , prev_page(drawable, 0, 0, this, notifier,
                    raw_prev_page().cx, raw_prev_page().cy, raw_prev_page().size,
                    raw_prev_page().img_blur, raw_prev_page().img_focus, -15)
        , current_page(drawable, 0, 0, this->first_page.cy(), this, notifier,
                       current_page ? current_page : "XXXX", -15, BLACK, WHITE, -1, 1, 1)
        , number_page(drawable, 0, 0, this, NULL,
                      number_of_page ? temporary_number_of_page(number_of_page).buffer : "/XXXX",
                      true, -100, BLACK, GREY)
        , next_page(drawable, 0, 0, this, notifier,
                    raw_next_page().cx, raw_next_page().cy, raw_next_page().size,
                    raw_next_page().img_blur, raw_next_page().img_focus, -15)
        , last_page(drawable, 0, 0, this, notifier,
                    raw_last_page().cx, raw_last_page().cy, raw_last_page().size,
                    raw_last_page().img_blur, raw_last_page().img_focus, -15)
          //END WidgetPager
        , logout(drawable, 0, 0, this, notifier,
                 raw_logout().cx, raw_logout().cy, raw_logout().size,
                 raw_logout().img_blur, raw_logout().img_focus, -16, NOTIFY_CANCEL)
        , apply(drawable, 0, 0, this, notifier,
                raw_apply().cx, raw_apply().cy, raw_apply().size,
                raw_apply().img_blur, raw_apply().img_focus, -12)
        , connect(drawable, 0, 0, this, notifier,
                  raw_connect().cx, raw_connect().cy, raw_connect().size,
                  raw_connect().img_blur, raw_connect().img_focus, -18)
        , tooltip(NULL)
        , w_over(NULL)
    {
        this->current_focus = &this->device_lines;
        this->child_list.push_back(&this->device_label);
        this->child_list.push_back(&this->device_target_label);
        this->child_list.push_back(&this->target_label);
        this->child_list.push_back(&this->protocol_label);
        this->child_list.push_back(&this->close_time_label);
        this->child_list.push_back(&this->filter_device);
        this->child_list.push_back(&this->filter_target);
        this->child_list.push_back(&this->filter_proto);
        this->child_list.push_back(&this->device_lines);
        this->child_list.push_back(&this->target_lines);
        this->child_list.push_back(&this->protocol_lines);
        this->child_list.push_back(&this->close_time_lines);
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

        this->target_lines.tab_flag = IGNORE_TAB;
        this->protocol_lines.tab_flag = IGNORE_TAB;
        this->close_time_lines.tab_flag = IGNORE_TAB;

        int dw = width - (this->close_time_lines.lx() + 15);
        if (dw < 0) {
            this->device_target_label.rect.x -= 15;
            this->filter_device.set_edit_x(this->device_target_label.dx());
            this->device_lines.rect.x -= 15;
            this->filter_device.set_edit_cx(this->device_target_label.cx());
            this->device_lines.rect.cx = this->device_target_label.cx() + 10;

            int w,h;
            this->drawable.text_metrics("INTERNAL", w,h);
            this->protocol_lines.rect.cx = w + 10;
            this->drawable.text_metrics("XXXX-XX-XX XX:XX:XX", w,h);
            this->close_time_lines.rect.cx = w + 10;

            this->close_time_lines.rect.x = width - this->close_time_lines.cx();
            this->close_time_label.rect.x = this->close_time_lines.dx() + 5;
            this->protocol_lines.rect.x = this->close_time_lines.dx() - this->protocol_lines.cx();
            this->protocol_label.rect.x = this->protocol_lines.dx() + 5;
            this->filter_proto.set_edit_x(this->protocol_label.dx());
            this->filter_proto.set_edit_cx(this->protocol_lines.cx() - 10);

            this->target_lines.rect.cx = this->protocol_lines.dx() - this->device_lines.lx();
            this->target_lines.rect.x = this->device_lines.lx();
            this->target_label.rect.x = this->target_lines.dx() + 5;
            this->filter_target.set_edit_x(this->target_label.dx());
            this->filter_target.set_edit_cx(this->target_lines.cx() - 10);
        }
        else if (dw > 0) {
            this->target_lines.rect.cx += dw;
            this->filter_target.set_edit_cx(this->filter_target.cx() + dw);
            this->protocol_label.rect.x += dw;
            this->close_time_label.rect.x += dw;
            this->protocol_lines.rect.x += dw;
            this->close_time_lines.rect.x += dw;

            this->filter_proto.set_edit_x(this->protocol_label.dx());
            this->filter_proto.set_edit_cx(this->protocol_lines.cx() - 10);
        }

        this->device_target_label.rect.y = this->device_label.cy() + this->device_label.dy() + 5;
        this->target_label.rect.y = this->device_target_label.dy();
        this->protocol_label.rect.y = this->device_target_label.dy();
        this->close_time_label.rect.y = this->device_target_label.dy();
        this->filter_device.set_edit_y(this->device_target_label.dy() + this->device_target_label.cy() + 5);
        this->filter_target.set_edit_y(this->filter_device.dy());
        this->filter_proto.set_edit_y(this->filter_device.dy());
        this->device_lines.rect.y = this->filter_device.dy() + this->filter_device.cy() + 5;
        this->target_lines.rect.y = this->device_lines.dy();
        this->protocol_lines.rect.y = this->device_lines.dy();
        this->close_time_lines.rect.y = this->device_lines.dy();

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
    {}

    virtual void draw(const Rect& clip)
    {
        this->WidgetComposite::draw(clip);
        this->WidgetComposite::draw_inner_free(clip.intersect(this->rect), GREY);
    }

    virtual void notify(Widget2* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (NOTIFY_SHOW_TOOLTIP == event) {
            this->child_list.push_back(this->tooltip);
            this->refresh(this->tooltip->rect);
        }
        if (NOTIFY_HIDE_TOOLTIP == event) {
            if (this->tooltip) {
                this->child_list.pop_back();
                this->refresh(this->tooltip->rect);
                delete this->tooltip;
                this->tooltip = NULL;
            }
        }
        if (widget->group_id == this->device_lines.group_id) {
            if (NOTIFY_SUBMIT == event) {
                if (this->notifier) {
                    this->notifier->notify(widget, event, param, param2);
                }
            }
            else {
                this->set_index_list(static_cast<WidgetSelectLine*>(widget)->current_index);
            }
        }
        else if (widget->group_id == this->apply.group_id) {
            if (this->notifier) {
                this->notifier->notify(widget, event, param, param2);
            }
        }
        else {
            WidgetComposite::notify(widget, event, param, param2);
        }
    }

    virtual void focus()
    {
        this->send_notify(NOTIFY_FOCUS_BEGIN);
        this->has_focus = true;
        this->refresh(this->rect);
    }

    virtual void blur()
    {
        this->send_notify(NOTIFY_FOCUS_END);
        this->has_focus = false;
        this->refresh(this->rect);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags & MOUSE_FLAG_MOVE) {
            Widget2 * w = this->widget_at_pos(x, y);
            if (w != this->w_over) {
                this->notify(this, NOTIFY_HIDE_TOOLTIP, 0, 0);
                this->w_over = w;
            }
        }

        Widget2 * w = this->widget_at_pos(x, y);
        if (w){
            if (device_flags & MOUSE_FLAG_BUTTON1) {
                if ((w->focus_flag != IGNORE_FOCUS) && (w != this->current_focus)){
                    if (this->current_focus) {
                        this->current_focus->blur();
                    }
                    this->current_focus = w;
                    this->current_focus->focus();
                }
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }



    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
         this->notify(this, NOTIFY_HIDE_TOOLTIP, 0, 0);
         WidgetComposite::rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void set_index_list(int idx)
    {
        this->device_lines.set_current_index(idx);
        this->target_lines.set_current_index(idx);
        this->protocol_lines.set_current_index(idx);
        this->close_time_lines.set_current_index(idx);
    }

    void add_device(const char * device_group, const char * target_label,
                    const char * protocol, const char * close_time)
    {
        this->device_lines.add_line(device_group);
        this->target_lines.add_line(target_label);
        this->protocol_lines.add_line(protocol);
        this->close_time_lines.add_line(close_time);
        const uint lcy = this->device_lines.labels.size() * (this->device_lines.h_text + this->device_lines.y_text * 2 + this->device_lines.h_border) - this->device_lines.h_border;
        this->device_lines.rect.cy = lcy;
        this->target_lines.rect.cy = lcy;
        this->protocol_lines.rect.cy = lcy;
        this->close_time_lines.rect.cy = lcy;
    }
};

#endif
