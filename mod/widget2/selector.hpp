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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_WIDGET2_SELECTOR_HPP)
#define REDEMPTION_MOD_WIDGET2_SELECTOR_HPP

#include "pager.hpp"
#include <modcontext.hpp>

class WidgetSelector : public WidgetPager
{
public:
    enum {
        NOTIFY_FOCUS_BEGIN = ::NOTIFY_FOCUS_BEGIN,
        NOTIFY_FOCUS_END = ::NOTIFY_FOCUS_END,
        NOTIFY_SUBMIT = ::NOTIFY_SUBMIT,
        NOTIFY_CANCEL = ::NOTIFY_CANCEL,
        NOTIFY_FILTER_GROUP = 200,
        NOTIFY_FILTER_DEVICE,
    };

    struct TargetDevice {
        char group[256];
        char target[256];
        char protocol[256];
        char endtime[256];
        TargetDevice(){
            this->group[0] = 0;
            this->target[0] = 0;
            this->protocol[0] = 0;
            this->endtime[0] = 0;
        }
    } grid[50];

    WidgetButton submit;
    WidgetButton cancel;
    WidgetButton apply;

    WidgetLabel device_col;
    WidgetLabel account_col;
    WidgetLabel protocol_col;
    WidgetLabel close_time_col;

    WidgetEdit filter_group;
    WidgetEdit filter_device;

    size_t focus_line;

    size_t current_page;
    size_t total_page;

    ModContext & context;

    WidgetSelector(ModContext & context, ModApi* drawable, uint16_t width, uint16_t height, Widget* parent, NotifyApi* notifier, int id = 0)
    : WidgetPager(drawable, Rect(0, 0, width, height - 20), parent, 0, "Selector", -1u, id)
    , grid()
    , submit(drawable, Rect(), this, 0, "connect", -10)
    , cancel(drawable, Rect(), this, 0, "logout", -11)
    , apply(drawable, Rect(), this, 0, "apply", -12)
    , device_col(drawable, Rect(0, this->titlebar.rect.cy + 2, width/20*3, 20),
                 this, 0, "Device group", -13, 2, 2)
    , account_col(drawable,
                  Rect(this->device_col.rect.cx + 2,
                       this->titlebar.rect.cy + 2,
                       width / 20 * 10,
                       20),
                  this, 0, "Account group", -14, 2, 2)
    , protocol_col(drawable,
                   Rect(this->account_col.rect.cx + this->account_col.rect.x + 2,
                        this->titlebar.rect.cy + 2,
                        this->device_col.rect.cx,
                        20),
                   this, 0, "Protocol", -15, 2, 2)
    , close_time_col(drawable,
                     Rect(this->protocol_col.rect.cx + this->protocol_col.rect.x + 2,
                          this->titlebar.rect.cy + 2,
                          this->device_col.rect.cx,
                          20),
                     this, 0, "Close time", -16, 2, 2)
    , filter_group(drawable,
                   Rect(this->device_col.rect.x,
                        this->device_col.rect.y + this->device_col.rect.cy + 2,
                        this->device_col.rect.cx,
                        this->device_col.rect.cy),
                   this, 0, "", 0, -17, 2, 2)
    , filter_device(drawable,
                    Rect(this->account_col.rect.x,
                         this->account_col.rect.y + this->account_col.rect.cy + 2,
                         this->account_col.rect.cx,
                         this->account_col.rect.cy),
                    this, 0, "", 0, -18, 2, 2)
    , focus_line(0)
    , current_page(0)
    , total_page(0)
    , context(context)
    {
        this->rect.cy += 20;
        int w = width - (this->close_time_col.rect.x + this->close_time_col.rect.cx + 2);
        this->device_col.rect.cx += w;
        this->account_col.rect.x += w;
        this->protocol_col.rect.x += w;
        this->close_time_col.rect.x += w;
        this->filter_group.rect.cx += w;
        this->filter_device.rect.x += w;

        int cx = std::max(
            this->cancel.label.context_text->cx,
            std::max(
                this->submit.label.context_text->cx,
                this->apply.label.context_text->cx
            )
        ) + 8;
        int cy = std::max(
            this->cancel.label.context_text->cy,
            std::max(
                this->submit.label.context_text->cy,
                this->apply.label.context_text->cy
            )
        ) + 6;
        int y = this->rect.cy - cy - 5;
        this->cancel.rect.cx = cx;
        this->cancel.rect.cy = cy;
        this->cancel.rect.y = y;
        this->cancel.rect.x = this->rect.cx - cx - 5;
        this->cancel.label.x_text = (cx - this->cancel.label.context_text->cx) / 2;
        this->cancel.label.y_text = (cy - this->cancel.label.context_text->cy) / 2;
        this->apply.rect.cx = cx;
        this->apply.rect.cy = cy;
        this->apply.rect.y = y;
        this->apply.rect.x = this->cancel.rect.x - cx - 10;
        this->apply.label.x_text = (cx - this->apply.label.context_text->cx) / 2;
        this->apply.label.y_text = (cy - this->apply.label.context_text->cy) / 2;
        this->submit.rect.cx = cx;
        this->submit.rect.cy = cy;
        this->submit.rect.y = y;
        this->submit.rect.x = this->apply.rect.x - cx - 10;
        this->submit.label.x_text = (cx - this->submit.label.context_text->cx) / 2;
        this->submit.label.y_text = (cy - this->submit.label.context_text->cy) / 2;

        const char * context_current_page = this->context.get(STRAUTHID_SELECTOR_CURRENT_PAGE);
        this->current_page = atoi(context_current_page);
        this->current.label.set_text(context_current_page);
        const char * context_total_page = this->context.get(STRAUTHID_SELECTOR_NUMBER_OF_PAGES);
        this->total_page = atoi(context_total_page);

        if (!this->context.is_asked(STRAUTHID_SELECTOR_DEVICE_FILTER)){
            this->filter_device.label.set_text(this->context.get(STRAUTHID_SELECTOR_DEVICE_FILTER));
        }

        if (this->context.is_asked(STRAUTHID_SELECTOR_GROUP_FILTER)){
            this->filter_group.label.set_text(this->context.get(STRAUTHID_SELECTOR_GROUP_FILTER));
        }

        this->nb_page.buffer[0] = '/';
        if (0 == this->total_page) {
            this->nb_page.buffer[1] = '0';
            this->nb_page.buffer[2] = 0;
        } else {
            const size_t max = sizeof(this->nb_page.buffer)/sizeof(this->nb_page.buffer[0]) - 1;
            strncpy(this->nb_page.buffer+1, context_total_page, max-1);
            this->nb_page.buffer[max] = 0;
        }
        if (this->nb_page.drawable) {
            this->nb_page.context_text = this->nb_page.drawable->create_context_text(this->nb_page.buffer);
            this->nb_page.rect = Rect(this->next.rect.x - this->nb_page.context_text->cx - 6,
                                      this->last.rect.y,
                                      this->nb_page.context_text->cx + 4,
                                      this->last.rect.cy);
            int w,h;
            this->drawable->text_metrics("0", w, h);
            size_t size = strlen(context_total_page);
            this->current.rect.cx = w*(size ? size : 1) + 4;
            this->current.rect.x = this->nb_page.rect.x - (this->current.rect.cx + 2);
            this->prev.rect.x = this->current.rect.x - (this->prev.rect.cx + 2);
            this->first.rect.x = this->prev.rect.x - (this->first.rect.cx + 2);
        }
    }

    virtual ~WidgetSelector()
    {}

    void refresh_context()
    {
        const char * groups = this->context.get(STRAUTHID_TARGET_USER);
        const char * targets = this->context.get(STRAUTHID_TARGET_DEVICE);
        const char * protocols = this->context.get(STRAUTHID_TARGET_PROTOCOL);
        const char * endtimes = this->context.get(STRAUTHID_END_TIME);

        for (size_t index = 0 ; index < 50 ; index++){
            this->grid[index].group[0] = 0;
            this->grid[index].target[0] = 0;
            this->grid[index].protocol[0] = 0;
            this->grid[index].endtime[0] = 0;
        }

        for (size_t index = 0 ; index < sizeof(this->grid)/sizeof(this->grid[0]); index++){
            groups = proceed_item(groups, this->grid[index].group);
            targets = proceed_item(targets, this->grid[index].target);
            protocols = proceed_item(protocols, this->grid[index].protocol);
            endtimes = proceed_item(endtimes, this->grid[index].endtime, ';');

            if (*groups    == '\n' || !*groups
            ||  *targets   == '\n' || !*targets
            ||  *protocols == '\n' || !*protocols
            ||  *endtimes  == '\n' || !*endtimes
            ){
                break;
            }

            groups++;
            targets++;
            protocols++;
            endtimes++;
        }
    }

    static const char * proceed_item(const char * list, char * grid_item, char sep = ' ')
    {
        const char * p = list;
        while (*p != sep && *p != '\n' && *p){
            p++;
        }
        memcpy(grid_item, list, p-list);
        grid_item[p-list] = 0;
        return p;
    }
};

#endif