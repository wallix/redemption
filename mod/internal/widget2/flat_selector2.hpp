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
 *
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_SELECTOR2_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_SELECTOR2_HPP

#include "composite.hpp"
#include "multiline.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "number_edit.hpp"
#include "image.hpp"
#include "region.hpp"
#include "difftimeval.hpp"
#include "labelgrid.hpp"


#include "flat_button.hpp"
#include "translation.hpp"
#include "theme.hpp"
// #include "radio_list.hpp"

enum {
    NONE = 0x00,
    TICKET_VISIBLE = 0x01,
    TICKET_MANDATORY = 0x03,
    COMMENT_VISIBLE = 0x04,
    COMMENT_MANDATORY = 0x0c
};

class WidgetSelectorFlat2 : public WidgetParent
{

//     class Line : public Widget2 {
//     public:
//         WidgetLabel group;
//         WidgetLabel target;
//         WidgetLabel protocol;
//         WidgetLabel closetime;
//         int border_color;


//         Line(DrawApi & drawable, Widget2& parent, NotifyApi* notifier, int x, int y,
//              int lcy, int group_w, int target_w, int protocol_w, int closetime_w,
//              const char * device_group, const char * target_label, const char * protocol,
//              const char * closetime, int fgcolor, int bgcolor, int x_text, int y_text)
//             : Widget2(drawable,
//                       Rect(x, y, group_w + target_w + protocol_w + closetime_w, lcy),
//                       parent, notifier)
//             , group(WidgetLabel(drawable, x, y, parent, notifier, device_group, false, 0,
//                                 fgcolor, bgcolor, x_text, y_text))
//             , target(WidgetLabel(drawable, x + group_w, y, parent, notifier,
//                                  target_label, false, 0, fgcolor, bgcolor, x_text, y_text))
//             , protocol(WidgetLabel(drawable, x + group_w + target_w, y, parent, notifier,
//                                    protocol, false, 0, fgcolor, bgcolor, x_text, y_text))
//             , closetime(WidgetLabel(drawable, x + group_w + target_w + protocol_w, y,
//                                     parent, notifier, closetime, false, 0,
//                                     fgcolor, bgcolor, x_text, y_text))
//             , border_color(bgcolor)
//         {
//             this->group.rect.cx     = group_w;
//             this->target.rect.cx    = target_w;
//             this->protocol.rect.cx  = protocol_w;
//             this->closetime.rect.cx = closetime_w;
//             this->group.rect.cy     = lcy;
//             this->target.rect.cy    = lcy;
//             this->protocol.rect.cy  = lcy;
//             this->closetime.rect.cy = lcy;
//         }

//         ~Line()
//         {
//         }

//         virtual void draw(const Rect& clip) {
//             Rect new_clip = clip.intersect(this->rect);
//             this->group.refresh(new_clip.intersect(this->group.rect));
//             this->target.refresh(new_clip.intersect(this->target.rect));
//             this->protocol.refresh(new_clip.intersect(this->protocol.rect));
//             this->closetime.refresh(new_clip.intersect(this->closetime.rect));
//         }

//         void set_bg_color(int bg_color) {
//             this->group.bg_color     = bg_color;
//             this->target.bg_color    = bg_color;
//             this->protocol.bg_color  = bg_color;
//             this->closetime.bg_color = bg_color;
//             this->border_color       = bg_color;
//         }
//         void set_fg_color(int fg_color) {
//             this->group.fg_color     = fg_color;
//             this->target.fg_color    = fg_color;
//             this->protocol.fg_color  = fg_color;
//             this->closetime.fg_color = fg_color;
//         }
//     };
// public:
//     typedef enum {
//         COLUMN_UNKNOWN,
//         COLUMN_GROUP,
//         COLUMN_TARGET,
//         COLUMN_PROTOCOL,
//         COLUMN_CLOSETIME,
//     } Column;

// private:
//     class WidgetSelectLine : public Widget2
//     {
//     public:
//         uint current_index;
//         int bg_color;
//         uint h_border;
//         int bg_color1;
//         int bg_color2;
//         int fg_color1;
//         int fg_color2;
//         int current_bgcolor;
//         int current_fgcolor;
//         int focus_bgcolor;
//         int focus_fgcolor;
//         int h_text;
//         int x_text;
//         int y_text;

//         int group_w;
//         int target_w;
//         int protocol_w;
//         int closetime_w;
//         Column col;

//         std::vector<Line*> labels;
//         uint over_index;

//         struct difftimer {
//             uint64_t t;

//             difftimer(uint64_t start = 0)
//                 : t(start)
//             {}

//             uint64_t tick()
//             {
//                 uint64_t ret = this->t;
//                 this->t = ustime();
//                 return this->t - ret;
//             }

//             void update()
//             {
//                 this->t = ustime();
//             }
//         } click_interval;

//         WidgetSelectLine(DrawApi& drawable, WidgetSelectorFlat2& parent,
//                          NotifyApi* notifier, int x, int y,
//                          int group_w, int target_w, int protocol_w, int closetime_w,
//                          int group_id,
//                          int fgcolor1, int fgcolor2,
//                          int current_fgcolor,
//                          int bgcolor1, int bgcolor2,
//                          int current_bgcolor,
//                          int focus_bgcolor, int focus_fgcolor,
//                          int xtext, int ytext,
//                          int bgcolor, uint border_height)
//             : Widget2(drawable, Rect(x, y, group_w + target_w + protocol_w + closetime_w, 1), parent, notifier, group_id)
//             , current_index(-1u)
//             , bg_color(bgcolor)
//             , h_border(border_height)
//             , bg_color1(bgcolor1)
//             , bg_color2(bgcolor2)
//             , fg_color1(fgcolor1)
//             , fg_color2(fgcolor2)
//             , current_bgcolor(current_bgcolor)
//             , current_fgcolor(current_fgcolor)
//             , focus_bgcolor(focus_bgcolor)
//             , focus_fgcolor(focus_fgcolor)
//             , h_text(0)
//             , x_text(xtext)
//             , y_text(ytext)
//             , group_w(group_w)
//             , target_w(target_w)
//             , protocol_w(protocol_w)
//             , closetime_w(closetime_w)
//             , col(COLUMN_UNKNOWN)
//             , labels()
//             , over_index(-1u)
//             , click_interval()
//         {
//             int w = 0;
//             this->drawable.text_metrics("Lp", w, this->h_text);
//         }

//         void set_x(int x) {
//             this->rect.x = x;
//         }

//         void ajust_group_and_target_width(int width) {
//             int both_width = this->group_w + this->target_w;
//             this->group_w = both_width - width;
//             this->target_w = width;
//             for (size_t i = 0; i < this->labels.size(); ++i) {
//                 this->labels[i]->target.rect.cx = width;
//                 this->labels[i]->target.rect.x = this->rect.x + this->group_w;
//                 this->labels[i]->group.rect.cx = this->group_w;
//             }
//         }

//         int get_target_maxwidth() {
//             int max = 0;
//             int h = 0;
//             int w = 0;
//             for (size_t i = 0; i < this->labels.size(); ++i) {
//                 w = 0;
//                 this->drawable.text_metrics(this->labels[i]->target.get_text(), w, h);
//                 if (w > max)
//                     max = w;
//             }
//             return max;
//         }
//         int get_group_maxwidth() {
//             int max = 0;
//             int h = 0;
//             int w = 0;
//             for (size_t i = 0; i < this->labels.size(); ++i) {
//                 w = 0;
//                 this->drawable.text_metrics(this->labels[i]->group.get_text(), w, h);
//                 if (w > max)
//                     max = w;
//             }
//             return max;
//         }
//         void set_group_w(int w) {
//             this->group_w = w;
//             this->rect.cx = get_total_w();
//         }
//         void set_target_w(int w) {
//             this->target_w = w;
//             this->rect.cx = get_total_w();
//         }
//         void set_protocol_w(int w) {
//             this->protocol_w = w;
//             this->rect.cx = get_total_w();
//         }
//         void set_closetime_w(int w) {
//             this->closetime_w = w;
//             this->rect.cx = get_total_w();
//         }

//         int get_target_x() {
//             return this->rect.x + this->group_w;
//         }
//         int get_protocol_x() {
//             return this->get_target_x() + this->target_w;
//         }
//         int get_closetime_x() {
//             return this->get_protocol_x() + this->protocol_w;
//         }
//         int get_total_w() {
//             return this->group_w + this->target_w + this->protocol_w + this->closetime_w;
//         }


//         virtual ~WidgetSelectLine()
//         {
//             for (size_t i = 0; i < this->labels.size(); ++i) {
//                 delete this->labels[i];
//             }
//         }

//         const char * get_current_index(Column col) const
//         {
//             if (this->current_index < this->labels.size()) {
//                 switch (col) {
//                 case COLUMN_GROUP:
//                     return this->labels[this->current_index]->group.get_text();
//                 case COLUMN_TARGET:
//                     return this->labels[this->current_index]->target.get_text();
//                 case COLUMN_PROTOCOL:
//                     return this->labels[this->current_index]->protocol.get_text();
//                 case COLUMN_CLOSETIME:
//                     return this->labels[this->current_index]->closetime.get_text();
//                 default:
//                     return "";
//                     break;
//                 }
//             }
//             return "";
//         }

//         const char * get_over_index() const
//         {
//             if (this->over_index < this->labels.size()) {
//                 switch (this->col) {
//                 case COLUMN_GROUP:
//                     return this->labels[this->over_index]->group.get_text();
//                 case COLUMN_TARGET:
//                     return this->labels[this->over_index]->target.get_text();
//                 case COLUMN_PROTOCOL:
//                     return this->labels[this->over_index]->protocol.get_text();
//                 case COLUMN_CLOSETIME:
//                     return this->labels[this->over_index]->closetime.get_text();
//                 default:
//                     break;
//                 }
//             }

//             return "";
//         }

//         void add_line(const char * device_group, const char * target_label,
//                       const char * protocol, const char * close_time)
//         {
//             uint16_t lcy = this->h_text + this->y_text * 2;
//             bool b = this->labels.size() & 1;

//             Line * line = new Line(this->drawable, *this, NULL,
//                                    0, this->labels.size() * (lcy + this->h_border),
//                                    lcy + this->h_border,
//                                    this->group_w, this->target_w, this->protocol_w, this->closetime_w,
//                                    device_group, target_label, protocol, close_time,
//                                    b ? this->fg_color1 : this->fg_color2,
//                                    b ? this->bg_color1 : this->bg_color2,
//                                    this->x_text, this->y_text);

//             this->labels.push_back(line);
//             const uint lines_h = this->labels.size() * (this->h_text + this->y_text * 2 + this->h_border);
//             this->rect.cy = lines_h;
//         }

//         void clear()
//         {
//             for (size_t i = 0; i < this->labels.size(); ++i) {
//                 delete this->labels[i];
//             }
//             this->rect.cy = 0;
//             this->labels.clear();
//             this->current_index = -1u;
//         }

//         void set_line_focus_color(Line & line) {
//             if (this->has_focus) {
//                 line.set_bg_color(this->focus_bgcolor);
//                 line.set_fg_color(this->focus_fgcolor);
//             }
//             else {
//                 line.set_bg_color(this->current_bgcolor);
//                 line.set_fg_color(this->current_fgcolor);
//             }
//         }

//         void set_current_index(uint idx)
//         {
//             if (this->focus_flag == Widget2::IGNORE_FOCUS) {
//                 return;
//             }
//             if (idx != this->current_index) {
//                 uint previous_index = this->current_index;
//                 this->current_index = idx;
//                 if (previous_index < this->labels.size()) {
//                     this->labels[previous_index]->set_bg_color(previous_index & 1
//                                                                ? this->bg_color1
//                                                                : this->bg_color2);
//                     this->labels[previous_index]->set_fg_color(previous_index & 1
//                                                                ? this->fg_color1
//                                                                : this->fg_color2);
//                     this->refresh(this->labels[previous_index]->rect);
//                 }
//                 if (this->current_index < this->labels.size()) {
//                     this->set_line_focus_color(*this->labels[this->current_index]);
//                     this->refresh(this->labels[this->current_index]->rect);
//                 }
//             }
//         }

//         virtual void draw(const Rect& clip)
//         {
//             std::size_t size = this->labels.size();
//             uint lcy = 0;
//             for (std::size_t i = 0; i < size; ++i) {
//                 Line * line = this->labels[i];
//                 if (i == this->current_index) {
//                     this->set_line_focus_color(*line);
//                 }

//                 line->refresh(clip);

//                 lcy += line->rect.cy;
//             }
//         }

//         Column get_column(int x) {
//             if (x >= this->rect.x) {
//                 if (x <= this->rect.x + this->group_w) {
//                     return COLUMN_GROUP;
//                 }
//                 else if (x <= this->rect.x + this->group_w + this->target_w) {
//                     return COLUMN_TARGET;
//                 }
//                 else if (x <= this->rect.x + this->group_w + this->target_w + this->protocol_w) {
//                     return COLUMN_PROTOCOL;
//                 }
//                 else if (x  <= this->rect.x + this->group_w + this->target_w + this->protocol_w + this->closetime_w) {
//                     return COLUMN_CLOSETIME;
//                 }
//             }
//             return COLUMN_TARGET;
//         }

//         int get_column_cx() {
//             switch (this->col) {
//             case COLUMN_GROUP:
//                 return this->group_w;
//             case COLUMN_TARGET:
//                 return this->target_w;
//             case COLUMN_PROTOCOL:
//                 return this->protocol_w;
//             case COLUMN_CLOSETIME:
//                 return this->closetime_w;
//             default:
//                 break;
//             }
//             return 0;
//         }

//         virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
//         {
//             if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
//                 int lcy = (this->h_text + this->y_text * 2 + this->h_border);
//                 if (int16_t(y) >= this->dy()
//                     && y < int(this->dy() + this->labels.size() * lcy - this->h_border)
//                     && lcy != 0) {
//                     int p = (y - this->dy()) / lcy;
//                     if ((uint)p != this->current_index) {
//                         this->click_interval.update();
//                         this->set_current_index(p);
//                         return ;
//                     }
//                     else {
//                         if (this->click_interval.tick() <= 700000LL) {
//                             this->send_notify(NOTIFY_SUBMIT);
//                             return ;
//                         }
//                     }
//                 }
//             }
//             if (device_flags == MOUSE_FLAG_MOVE) {
//                 int lcy = (this->h_text + this->y_text * 2 + this->h_border);
//                 if (int16_t(y) >= this->dy()
//                     && y < int(this->dy() + this->labels.size() * lcy - this->h_border)
//                     && lcy != 0) {
//                     int p = (y - this->dy()) / lcy;
//                     Column c = this->get_column(x);
//                     if ((uint)p != this->over_index
//                         || (c != this->col)
//                         ) {
//                         this->over_index = p;
//                         this->col = c;
//                         int w = 0;
//                         int h = 0;
//                         this->drawable.text_metrics(this->get_over_index(), w, h);
//                         this->hide_tooltip();
//                         if (w >= this->get_column_cx()) {
//                             this->show_tooltip(this, this->get_over_index(), x, y);
//                         }
//                     }

//                 }
//             }
//             Widget2::rdp_input_mouse(device_flags, x, y, keymap);
//         }

//         virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)

//         {
//             if (keymap->nb_kevent_available() > 0){
//                 switch (keymap->top_kevent()){
//                 case Keymap2::KEVENT_LEFT_ARROW:
//                 case Keymap2::KEVENT_UP_ARROW:
//                     keymap->get_kevent();
//                     if (this->labels.size() > 1) {
//                         this->set_current_index(this->current_index ? this->current_index - 1 : this->labels.size() - 1);
//                     }
//                     break;
//                 case Keymap2::KEVENT_RIGHT_ARROW:
//                 case Keymap2::KEVENT_DOWN_ARROW:
//                     keymap->get_kevent();
//                     if (this->labels.size() > 1) {
//                         this->set_current_index(this->current_index + 1 != this->labels.size() ? this->current_index + 1 : 0);
//                     }
//                     break;
//                 case Keymap2::KEVENT_END:
//                     keymap->get_kevent();
//                     if (this->labels.size() > 1 && this->labels.size() - 1 != this->current_index) {
//                         this->set_current_index(this->labels.size() - 1);
//                     }
//                     break;
//                 case Keymap2::KEVENT_HOME:
//                     keymap->get_kevent();
//                     if (this->labels.size() > 1 && 0 != this->current_index) {
//                         this->set_current_index(0);
//                     }
//                     break;
//                 case Keymap2::KEVENT_ENTER:
//                     keymap->get_kevent();
//                     if (!this->labels.empty()) {
//                         this->send_notify(NOTIFY_SUBMIT);
//                     }
//                     break;
//                 default:
//                     Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
//                     break;
//                 }
//             }
//         }

//     };

public:

    Theme & theme;
    const bool less_than_800;
    WidgetLabel device_label;
    WidgetLabel target_group_label;
    WidgetLabel target_label;
    WidgetLabel protocol_label;
    WidgetLabel close_time_label;

    WidgetLabelGrid selector_lines;

    WidgetEdit filter_target_group;
    WidgetEdit filter_target;
    WidgetEdit filter_protocol;
    //BEGIN WidgetPager

    WidgetFlatButton first_page;
    WidgetFlatButton prev_page;

    WidgetNumberEdit current_page;
    WidgetLabel number_page;
    WidgetFlatButton next_page;
    WidgetFlatButton last_page;
    //END WidgetPager
    WidgetFlatButton logout;
    WidgetFlatButton apply;
    WidgetFlatButton connect;
    //WidgetPager pager;

    // WidgetRadioList radiolist;

    WidgetLabel ticket_label;
    WidgetEdit ticket_edit;
    WidgetLabel comment_label;
    WidgetEdit comment_edit;

    int tc_flag;

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

    enum {
        IDX_TARGETGROUP,
        IDX_TARGET,
        IDX_PROTOCOL,
        IDX_CLOSETIME,
    };

    enum {
        SELECTOR_MARGIN = 15,
        SEPARATOR_SPACE = 5,
    };
public:
    WidgetSelectorFlat2(DrawApi& drawable, const char * device_name, uint16_t width,
                        uint16_t height, Widget2 & parent, NotifyApi* notifier,
                        const char * current_page, const char * number_of_page,
                        const char * filter_target_group, const char * filter_target,
                        const char * filter_protocol, Inifile & ini, int ticcom_flag = 0)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , theme(ini.theme)
        , less_than_800(this->rect.cx < 800)
        , device_label(drawable, 20, 10, *this, NULL, device_name, true, -10,
                       this->theme.global.fgcolor, this->theme.global.bgcolor)
        , target_group_label(drawable, 0, 0, *this, NULL, TR("target_group", ini), true,
                              -10, this->theme.selector_label.fgcolor,
                             this->theme.selector_label.bgcolor, 5)
        , target_label(drawable, 0, 0, *this, NULL, TR("target", ini), true, -10,
                       this->theme.selector_label.fgcolor,
                       this->theme.selector_label.bgcolor, 5)
        , protocol_label(drawable, 0, 0, *this, NULL, TR("protocol", ini), true, -10,
                         this->theme.selector_label.fgcolor,
                         this->theme.selector_label.bgcolor, 5)
        , close_time_label(drawable, 0, 0, *this, NULL, TR("close_time", ini), true, -10,
                           this->theme.selector_label.fgcolor,
                           this->theme.selector_label.bgcolor, 5)
        , selector_lines(drawable, Rect(0, 0, width - (this->less_than_800 ? 0 : 30), 1),
                         *this, this, 0, 4,
                         this->theme.selector_line1.bgcolor,
                         this->theme.selector_line1.fgcolor,
                         this->theme.selector_line2.bgcolor,
                         this->theme.selector_line2.fgcolor,
                         this->theme.selector_focus.bgcolor,
                         this->theme.selector_focus.fgcolor,
                         this->theme.selector_selected.bgcolor,
                         this->theme.selector_selected.fgcolor,
                         2, -11)
        , filter_target_group(drawable, 0, 0, 120, *this, this, filter_target_group?filter_target_group:0,
                        -12, this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                        this->theme.edit.focus_color, -1, 1, 1)
        , filter_target(drawable, 145, 0, 340, *this, this, filter_target?filter_target:0,
                        -12, this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                        this->theme.edit.focus_color, -1, 1, 1)
        , filter_protocol(drawable, 495, 0, 110, *this, this, filter_protocol?filter_protocol:0,
                       -12, this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                       this->theme.edit.focus_color, -1, 1, 1)
          //BEGIN WidgetPager
        , first_page(drawable, 0, 0, *this, notifier, "◀◂", true, -15,
                     this->theme.global.fgcolor, this->theme.global.bgcolor,
                     this->theme.global.focus_color, 6, 2, true)
        , prev_page(drawable, 0, 0, *this, notifier, "◀", true, -15,
                    this->theme.global.fgcolor, this->theme.global.bgcolor,
                    this->theme.global.focus_color, 6, 2, true)
        , current_page(drawable, 0, 0, this->first_page.cy(), *this, notifier,
                       current_page ? current_page : "XXXX", -15,
                       this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                       this->theme.edit.focus_color, -1, 1, 1)
        , number_page(drawable, 0, 0, *this, NULL,
                      number_of_page ? temporary_number_of_page(number_of_page).buffer
                      : "/XXX", true, -100, this->theme.global.fgcolor,
                      this->theme.global.bgcolor)
        , next_page(drawable, 0, 0, *this, notifier, "▶", true, -15,
                    this->theme.global.fgcolor, this->theme.global.bgcolor,
                    this->theme.global.focus_color, 6, 2, true)
        , last_page(drawable, 0, 0, *this, notifier, "▸▶", true, -15,
                    this->theme.global.fgcolor, this->theme.global.bgcolor,
                    this->theme.global.focus_color, 6, 2, true)
          //END WidgetPager
        , logout(drawable, 0, 0, *this, this, TR("logout", ini), true, -16,
                 this->theme.global.fgcolor, this->theme.global.bgcolor,
                 this->theme.global.focus_color, 6, 2)
        , apply(drawable, 0, 0, *this, this, TR("filter", ini), true, -12,
                this->theme.global.fgcolor, this->theme.global.bgcolor,
                this->theme.global.focus_color, 6, 2)
        , connect(drawable, 0, 0, *this, this, TR("connect", ini), true, -18,
                  this->theme.global.fgcolor, this->theme.global.bgcolor,
                  this->theme.global.focus_color, 6, 2)
        // , radiolist(drawable, this->device_label.lx() + 30, this->device_label.dy(), *this, this, -19, this->theme.global.fgcolr, this->theme.global.bgcolor)
        , ticket_label(drawable, 15, 10, *this, NULL, TR("Ticket n°", ini), true, -20,
                       this->theme.global.fgcolor, this->theme.global.bgcolor)
        , ticket_edit(drawable, 15, 0, 200, *this, this,
                      filter_target?filter_target:0, -20, this->theme.edit.fgcolor,
                      this->theme.edit.bgcolor, this->theme.edit.focus_color, -1, 1, 1)
        , comment_label(drawable, 15, 10, *this, NULL, TR("comment", ini), true, -20,
                       this->theme.global.fgcolor, this->theme.global.bgcolor)
        , comment_edit(drawable, 15, 0, 200, *this, this,
                    filter_target_group?filter_target_group:0, -20, this->theme.edit.fgcolor,
                    this->theme.edit.bgcolor, this->theme.edit.focus_color, -1, 1, 1)
        , tc_flag(ticcom_flag)
    {
        this->impl = new CompositeTable;

        this->add_widget(&this->device_label);
        this->add_widget(&this->target_group_label);
        this->add_widget(&this->target_label);
        this->add_widget(&this->protocol_label);
        this->add_widget(&this->close_time_label);
        this->add_widget(&this->filter_target_group);
        this->add_widget(&this->filter_target);
        this->add_widget(&this->filter_protocol);
        this->add_widget(&this->apply);
        this->add_widget(&this->selector_lines);

        // if (this->tc_flag & TICKET_VISIBLE) {
        //     this->add_widget(&this->ticket_label);
        //     this->add_widget(&this->ticket_edit);
        // }
        // if (this->tc_flag & COMMENT_VISIBLE) {
        //     this->add_widget(&this->comment_label);
        //     this->add_widget(&this->comment_edit);
        // }

        // this->add_widget(&this->first_page);
        // this->add_widget(&this->prev_page);
        // this->add_widget(&this->current_page);
        // this->add_widget(&this->number_page);
        // this->add_widget(&this->next_page);
        // this->add_widget(&this->last_page);
        // this->add_widget(&this->logout);
        // this->add_widget(&this->connect);

        // this->add_widget(&this->radiolist);

        // radiolist.add_elem("Alphabetical");
        // radiolist.add_elem("Last Used First");
        // radiolist.add_elem("Most Used First");
        // radiolist.select(0);

        ColumnWidthStrategy column_width_strategies[] = {
            { 50, 150 }, { 150, 800 }, { 50, 100 }, { 140, 140 }
        };

        uint16_t result_row_height[GRID_NB_ROWS_MAX]      = { 0 };
        uint16_t result_column_width[GRID_NB_COLUMNS_MAX] = { 0 };

        compute_format(this->selector_lines, column_width_strategies,
                       result_row_height, result_column_width);
        apply_format(this->selector_lines, result_row_height, result_column_width);

        LOG(LOG_INFO, "result column width %u, %u ,%u %u",
            result_column_width[0],
            result_column_width[1],
            result_column_width[2],
            result_column_width[3]);
        this->rearrange(result_column_width);

        // int dw = width - (this->selector_lines.rect.x + this->selector_lines.get_total_w() + 15);
        // if (dw < 0) {

        //     this->target_group_label.rect.x -= 15;
        //     this->filter_target_group.set_edit_x(this->target_group_label.dx() + 2);
        //     this->selector_lines.set_x(this->selector_lines.rect.x - 15);
        //     this->filter_target_group.set_edit_cx(this->target_group_label.cx());
        //     this->selector_lines.set_group_w(this->target_group_label.cx() + 10);

        //     int w,h;
        //     this->drawable.text_metrics("INTERNAL", w, h);
        //     int protocol_width = w + 10;
        //     this->drawable.text_metrics("XXXX-XX-XX XX:XX:XX", w, h);
        //     int closetime_width = w + 5;
        //     int total_w = this->selector_lines.group_w + protocol_width + closetime_width;

        //     this->selector_lines.set_target_w(width - total_w);
        //     this->selector_lines.set_protocol_w(protocol_width);
        //     this->selector_lines.set_closetime_w(closetime_width);

        //     this->close_time_label.rect.x = this->selector_lines.get_closetime_x();
        //     this->protocol_label.rect.x = this->selector_lines.get_protocol_x();
        //     this->filter_protocol.set_edit_x(this->protocol_label.dx());
        //     this->filter_protocol.set_edit_cx(this->selector_lines.protocol_w - 10);

        //     this->target_label.rect.x = this->selector_lines.get_target_x();
        //     this->filter_target.set_edit_x(this->target_label.dx());
        //     this->filter_target.set_edit_cx(this->selector_lines.target_w - 10);

        // }
        // else if (dw > 0) {
        //     this->selector_lines.set_target_w(this->selector_lines.target_w + dw);
        //     this->filter_target.set_edit_cx(this->filter_target.cx() + dw);

        //     this->protocol_label.rect.x += dw;
        //     this->close_time_label.rect.x += dw;

        //     this->filter_protocol.set_edit_x(this->protocol_label.dx());
        //     this->filter_protocol.set_edit_cx(this->selector_lines.protocol_w - 10);
        // }

        // // Adjust labels according to filter boxes
        // this->target_group_label.rect.y = this->device_label.cy() + this->device_label.dy() + 15;
        // this->target_label.rect.y = this->target_group_label.dy();
        // this->protocol_label.rect.y = this->target_group_label.dy();
        // this->close_time_label.rect.y = this->target_group_label.dy();
        // this->filter_target_group.set_edit_y(this->target_group_label.dy() + this->target_group_label.cy() + 5);

        // this->target_group_label.rect.cx = this->selector_lines.group_w;
        // this->target_label.rect.cx = this->selector_lines.target_w;
        // this->protocol_label.rect.cx = this->selector_lines.protocol_w;
        // this->close_time_label.rect.cx = this->selector_lines.closetime_w;

        // this->filter_target.set_edit_y(this->filter_target_group.dy());
        // this->filter_protocol.set_edit_y(this->filter_target_group.dy());
        // this->selector_lines.rect.y = this->filter_target_group.dy() + this->filter_target_group.cy() + 5;

        // // set right bottom navigation buttons positions
        // this->connect.set_button_y(this->cy() - (this->logout.cy() + 10));
        // this->logout.set_button_y(this->connect.dy());

        // this->last_page.set_button_y(this->connect.dy() - (this->last_page.cy() + 10));
        // this->next_page.set_button_y(this->last_page.dy());
        // this->number_page.rect.y = this->last_page.dy() +
        //     (this->next_page.cy() - this->number_page.cy()) / 2;
        // this->current_page.set_edit_y(this->last_page.dy() +
        //                               (this->next_page.cy() - this->current_page.cy()) / 2);
        // this->prev_page.set_button_y(this->last_page.dy());
        // this->first_page.set_button_y(this->last_page.dy());


        // this->last_page.set_button_x(this->cx() - (this->last_page.cx() + 20));
        // this->next_page.set_button_x(this->last_page.dx() - (this->next_page.cx() + 15));
        // this->number_page.rect.x = this->next_page.dx() - (this->number_page.cx() + 15);
        // this->current_page.set_edit_x(this->number_page.dx() - (this->current_page.cx()));
        // this->prev_page.set_button_x(this->current_page.dx() - (this->prev_page.cx() + 15));
        // this->first_page.set_button_x(this->prev_page.dx() - (this->first_page.cx() + 15));

        // int nav_w = this->last_page.lx() - this->first_page.dx();

        // this->connect.set_button_x(this->last_page.lx() - nav_w/4 - this->connect.cx()/2);
        // this->logout.set_button_x(this->first_page.dx() + nav_w/4 - this->logout.cx()/2);

        // // Comment fields
        // {
        //     int tc_label_max = 0;
        //     if (this->tc_flag & TICKET_VISIBLE) {
        //         tc_label_max = this->ticket_label.lx();
        //     }
        //     if ((this->tc_flag & COMMENT_VISIBLE) &&
        //         (tc_label_max < this->comment_label.lx())) {
        //         tc_label_max = this->comment_label.lx();
        //     }

        //     this->ticket_edit.set_edit_x(tc_label_max + 10);
        //     this->comment_edit.set_edit_x(tc_label_max + 10);

        //     this->ticket_edit.set_edit_cx((this->logout.dx() - tc_label_max - 45) / 2);
        //     this->comment_edit.set_edit_cx(this->logout.dx() - tc_label_max - 45);

        //     this->ticket_label.rect.y = this->current_page.dy();
        //     this->comment_label.rect.y = this->logout.dy();
        //     this->ticket_edit.set_edit_y(this->current_page.dy());
        //     this->comment_edit.set_edit_y(this->comment_label.dy());
        // }
        // // top right filter button
        // this->apply.set_button_y(this->device_label.dy());
        // this->apply.set_button_x(this->cx() - (this->apply.cx() + 20));

    }


    virtual ~WidgetSelectorFlat2()
    {
        this->clear();
    }

    void rearrange(uint16_t * columns_width) {

        {
            // filter button position
            this->apply.set_button_y(this->device_label.dy());
            this->apply.set_button_x(this->cx() - (this->apply.cx() + 20));
        }

        uint16_t offset = this->less_than_800 ? 0 : SELECTOR_MARGIN;
        {
            // labels and filters position
            this->target_group_label.rect.cx = columns_width[IDX_TARGETGROUP] +
                this->selector_lines.border * 2;
            this->target_group_label.rect.x = offset;
            this->target_group_label.rect.y = this->device_label.ly() + 15;
            this->filter_target_group.set_edit_x(this->target_group_label.dx());
            this->filter_target_group.set_edit_cx(this->target_group_label.cx() -
                                                  SEPARATOR_SPACE);
            this->filter_target_group.set_edit_y(this->target_group_label.ly() +
                                                 SEPARATOR_SPACE);
            offset += this->target_group_label.rect.cx;


            this->target_label.rect.cx = columns_width[IDX_TARGET] +
                this->selector_lines.border * 2;
            this->target_label.rect.x = offset;
            this->target_label.rect.y = this->target_group_label.rect.y;
            this->filter_target.set_edit_x(this->target_label.dx());
            this->filter_target.set_edit_cx(this->target_label.cx() - SEPARATOR_SPACE);
            this->filter_target.set_edit_y(this->target_label.ly() + SEPARATOR_SPACE);
            offset += this->target_label.rect.cx;


            this->protocol_label.rect.cx = columns_width[IDX_PROTOCOL] +
                this->selector_lines.border * 2;
            this->protocol_label.rect.x = offset;
            this->protocol_label.rect.y = this->target_group_label.rect.y;
            this->filter_protocol.set_edit_x(this->protocol_label.dx());
            this->filter_protocol.set_edit_cx(this->protocol_label.cx() -
                                              SEPARATOR_SPACE);
            this->filter_protocol.set_edit_y(this->protocol_label.ly() +
                                             SEPARATOR_SPACE);
            offset += this->protocol_label.rect.cx;


            this->close_time_label.rect.cx = columns_width[IDX_CLOSETIME] +
                this->selector_lines.border * 2;
            this->close_time_label.rect.x = offset;
            this->close_time_label.rect.y = this->target_group_label.rect.y;
        }
        {
            // selector list position
            this->selector_lines.rect.x = this->less_than_800 ? 0 : SELECTOR_MARGIN;
            this->selector_lines.rect.y = this->filter_target_group.ly() +
                SEPARATOR_SPACE;
            this->selector_lines.rect.cy = this->rect.cy - 300;
        }

        //     this->target_group_label.rect.x -= 15;
        //     this->filter_target_group.set_edit_x(this->target_group_label.dx() + 2);
        //     this->selector_lines.set_x(this->selector_lines.rect.x - 15);
        //     this->filter_target_group.set_edit_cx(this->target_group_label.cx());
        //     this->selector_lines.set_group_w(this->target_group_label.cx() + 10);

        //     int w,h;
        //     this->drawable.text_metrics("INTERNAL", w, h);
        //     int protocol_width = w + 10;
        //     this->drawable.text_metrics("XXXX-XX-XX XX:XX:XX", w, h);
        //     int closetime_width = w + 5;
        //     int total_w = this->selector_lines.group_w + protocol_width + closetime_width;

        //     this->selector_lines.set_target_w(width - total_w);
        //     this->selector_lines.set_protocol_w(protocol_width);
        //     this->selector_lines.set_closetime_w(closetime_width);

        //     this->close_time_label.rect.x = this->selector_lines.get_closetime_x();
        //     this->protocol_label.rect.x = this->selector_lines.get_protocol_x();
        //     this->filter_protocol.set_edit_x(this->protocol_label.dx());
        //     this->filter_protocol.set_edit_cx(this->selector_lines.protocol_w - 10);

        //     this->target_label.rect.x = this->selector_lines.get_target_x();
        //     this->filter_target.set_edit_x(this->target_label.dx());
        //     this->filter_target.set_edit_cx(this->selector_lines.target_w - 10);

        // }
        // else if (dw > 0) {
        //     this->selector_lines.set_target_w(this->selector_lines.target_w + dw);
        //     this->filter_target.set_edit_cx(this->filter_target.cx() + dw);

        //     this->protocol_label.rect.x += dw;
        //     this->close_time_label.rect.x += dw;

        //     this->filter_protocol.set_edit_x(this->protocol_label.dx());
        //     this->filter_protocol.set_edit_cx(this->selector_lines.protocol_w - 10);
        // }

    }

    // virtual void fit_columns() {
    //     if (this->selector_lines.labels.size() > 0) {
    //         int group_maxwidth = this->selector_lines.get_group_maxwidth();
    //         int target_maxwidth = this->selector_lines.get_target_maxwidth();
    //         this->fit_target_col(group_maxwidth + 10, target_maxwidth + 10);
    //     }
    // }

    // virtual void fit_target_col(int group_width, int target_width) {
    //     int h = 0;
    //     int w = 0;
    //     this->drawable.text_metrics(this->target_group_label.get_text(), w, h);
    //     w += 10;
    //     group_width = (group_width > w)?group_width:w;
    //     bool group_fits = (group_width <= this->selector_lines.group_w);
    //     bool target_fits = (target_width <= this->selector_lines.target_w);
    //     int group_and_target_width = this->selector_lines.group_w + this->selector_lines.target_w;
    //     int new_target_width = target_width;

    //     if (group_fits && target_fits)
    //         return;
    //     if (!group_fits && !target_fits)
    //         return;
    //     if (group_fits && !target_fits) {
    //         if ((group_and_target_width - target_width) > group_width ) {
    //             new_target_width = target_width;
    //         }
    //         else {
    //             new_target_width = group_and_target_width - group_width;
    //         }
    //     }
    //     else if (!group_fits && target_fits) {
    //         if ((group_and_target_width - group_width) > target_width) {
    //             new_target_width = group_and_target_width - group_width;
    //         }
    //         else {
    //             new_target_width = target_width;
    //         }
    //     }
    //     int diff_target_width = this->target_label.cx() - new_target_width;
    //     this->target_label.rect.cx -= diff_target_width;
    //     this->target_label.rect.x += diff_target_width;
    //     this->target_group_label.rect.cx += diff_target_width;
    //     this->filter_target.set_edit_cx(this->filter_target.cx() - diff_target_width);
    //     this->filter_target.set_edit_x(this->filter_target.dx() + diff_target_width);
    //     this->filter_target_group.set_edit_cx(this->filter_target_group.cx() + diff_target_width);
    //     this->selector_lines.ajust_group_and_target_width(new_target_width);
    // }

    virtual void draw(const Rect& clip)
    {
        this->impl->draw(clip);
        this->draw_inner_free(clip.intersect(this->rect), this->theme.global.bgcolor);
    }

    void ask_for_connection() {
        if (((this->tc_flag & TICKET_MANDATORY) == TICKET_MANDATORY) &&
            (this->ticket_edit.num_chars == 0)) {
            this->set_widget_focus(&this->ticket_edit);
            return;
        }
        if (((this->tc_flag & COMMENT_MANDATORY) == COMMENT_MANDATORY) &&
            (this->comment_edit.num_chars == 0)) {
            this->set_widget_focus(&this->comment_edit);
            return;
        }
        if (this->notifier) {
            this->notifier->notify(&this->connect, NOTIFY_SUBMIT);
        }
    }


    virtual void notify(Widget2* widget, notify_event_t event)
    {
        if (widget->group_id == this->selector_lines.group_id) {
            if (NOTIFY_SUBMIT == event) {
                this->ask_for_connection();
            }
        }
        else if (widget->group_id == this->apply.group_id) {
            if (NOTIFY_SUBMIT == event) {
                if (this->notifier) {
                    this->notifier->notify(widget, event);
                }
            }
        }
        else if (widget->group_id == this->logout.group_id) {
            if (NOTIFY_SUBMIT == event) {
                if (this->notifier) {
                    this->notifier->notify(widget, NOTIFY_CANCEL);
                }
            }
        }
        else if (widget->group_id == this->connect.group_id) {
            if (NOTIFY_SUBMIT == event) {
                this->ask_for_connection();
            }
        }
        else if (widget->group_id == this->ticket_edit.group_id) {
            if (NOTIFY_SUBMIT == event) {
                this->ask_for_connection();
            }
        }
        else {
            WidgetParent::notify(widget, event);
        }
    }
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        // if (device_flags == MOUSE_FLAG_MOVE) {
        //     if (this->selector_lines.col != COLUMN_UNKNOWN) {
        //         Widget2 * w = this->widget_at_pos(x, y);
        //         if (w != &this->selector_lines) {
        //             this->selector_lines.col = COLUMN_UNKNOWN;
        //         }
        //     }
        // }
        // WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
    }

    void add_device(const char * device_group, const char * target_label,
                    const char * protocol, const char * close_time)
    {
        const char * texts[] = { device_group, target_label, protocol, close_time };
        this->selector_lines.add_line(texts);
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
