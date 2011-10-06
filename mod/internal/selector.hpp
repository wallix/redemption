/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   RDP Secondary Connection Device Selector

*/

#if !defined(__SELECTOR_HPP__)
#define __SELECTOR_HPP__

#include <stdio.h>

struct selector_mod : public internal_mod {
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

    char filter_group[256];
    char filter[256];

    int signal;
    size_t focus_line;
    enum {
        FOCUS_ON_FILTER_GROUP = 0,
        FOCUS_ON_FILTER_DEVICE,
        FOCUS_ON_GRID,
        FOCUS_ON_FIRSTPAGE,
        FOCUS_ON_PREVPAGE,
        FOCUS_ON_NEXTPAGE,
        FOCUS_ON_LASTPAGE,
        FOCUS_ON_LOGOUT,
        FOCUS_ON_CANCEL,
        FOCUS_ON_CONNECT,
        MAX_FOCUS_ITEM
    };
    unsigned focus_item;
    unsigned state;
    unsigned filter_group_edit_pos;
    char filter_group_text[256];
    unsigned filter_device_edit_pos;
    char filter_device_text[256];

    size_t showed_page;
    size_t total_page;
    size_t showed;
    size_t total;

    ModContext & context;

    uint32_t color[3];
    selector_mod(wait_obj * event, ModContext & context, Front & front):
            internal_mod(front), signal(0), focus_line(0),
            focus_item(FOCUS_ON_LOGOUT),
            filter_group_edit_pos(0),
            filter_device_edit_pos(0),
            state(BUTTON_STATE_UP),
            showed_page(1), total_page(10),
            showed(100), total(1000),
            context(context)
    {
        this->filter_device_text[0] = 0;
        this->filter_group_text[0] = 0;
        this->filter[0] = 0;
        this->filter_group[0] = 0;
        this->color[0] = RED;
        this->color[1] = GREEN;
        this->color[2] = BLUE;

        const char * groups = context.get(STRAUTHID_TARGET_USER);
        const char * targets = context.get(STRAUTHID_TARGET_DEVICE);
        const char * protocols = context.get(STRAUTHID_TARGET_PROTOCOL);
        const char * endtimes = context.get(STRAUTHID_END_TIME);

        for (size_t index = 0 ; index < 50 ; index++){
            this->grid[index].group[0] = 0;
            this->grid[index].target[0] = 0;
            this->grid[index].protocol[0] = 0;
            this->grid[index].endtime[0] = 0;
        }

        for (size_t index = 0 ; index < 50 ; index++){
            groups = proceed_item(groups, this->grid[index].group);
            targets = proceed_item(targets, this->grid[index].target);
            protocols = proceed_item(protocols, this->grid[index].protocol);
            endtimes = proceed_item(endtimes, this->grid[index].endtime);

            LOG(LOG_INFO, "%s %s %s %s",
                this->grid[index].group,
                this->grid[index].target,
                this->grid[index].protocol,
                this->grid[index].endtime);

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

        this->event = event;
        this->event->set();
    }

    virtual ~selector_mod()
    {
    }


    static inline const char * proceed_item(const char * list, char * grid_item)
    {
        const char * p = list;
        while (*p != ' ' && *p != '\n' && *p){
            p++;
        }
        memcpy(grid_item, list, p-list);
        grid_item[p-list] = 0;
        return p;
    }


    virtual void rdp_input_invalidate(const Rect & rect)
    {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap)
    {
    }

    virtual void rdp_input_scancode(long param1, long param2, long flags, long time, const Keymap * keymap, const key_info* ki)
    {
        LOG(LOG_INFO, "param1=%u param2=%u flags=%x time=%u chr=%u sym=%u key_flags=%u", param1, param2, flags, time, ki?ki->chr:0, ki?ki->sym:0, keymap->key_flags);
        if (flags & 0xC000){ // KEYUP
            switch (this->focus_item){
                case FOCUS_ON_FILTER_DEVICE:
                {
                    LOG(LOG_INFO, "focus on filter device chr=%u", ki?ki->chr:0);
                    size_t lg_dev_text = strlen(this->filter_device_text);
                    if (ki){
                        if (8 == ki->chr && this->filter_device_edit_pos > 0){
                            memcpy(this->filter_device_text + this->filter_device_edit_pos - 1,
                                   this->filter_device_text + this->filter_device_edit_pos,
                                   lg_dev_text - this->filter_device_edit_pos + 1);
                            this->filter_device_edit_pos--;
                        }
                        else if ((ki->chr > 32) && (ki->chr <= 127) && lg_dev_text < 20){
                            memcpy(this->filter_device_text + this->filter_device_edit_pos + 1,
                                   this->filter_device_text + this->filter_device_edit_pos,
                                   lg_dev_text - this->filter_device_edit_pos + 1);
                            *(this->filter_device_text + this->filter_device_edit_pos) = ki->chr;
                            this->filter_device_edit_pos++;
                        }
                    }
                }
                this->event->set();
                break;
                case FOCUS_ON_FILTER_GROUP:
                {
                    LOG(LOG_INFO, "focus on filter group chr=%u", ki?ki->chr:0);
                    size_t lg_group_text = strlen(this->filter_group_text);
                    if (ki){
                        if (8 == ki->chr && this->filter_group_edit_pos > 0){
                            memcpy(this->filter_group_text + this->filter_group_edit_pos - 1,
                                   this->filter_group_text + this->filter_group_edit_pos,
                                   lg_group_text - this->filter_group_edit_pos + 1);
                            this->filter_group_edit_pos--;
                        }
                        else if ((ki->chr > 32) && (ki->chr <= 127) && lg_group_text < 10){
                            memcpy(this->filter_group_text + this->filter_group_edit_pos + 1,
                                   this->filter_group_text + this->filter_group_edit_pos,
                                   lg_group_text - this->filter_group_edit_pos + 1);
                            *(this->filter_group_text + this->filter_group_edit_pos) = ki->chr;
                            this->filter_group_edit_pos++;
                        }
                    }
                }
                this->event->set();
                break;
                default:
                break;
            }
            switch (param1){
                case 15:
                    if (keymap->shift_pressed()){
                        this->focus_item = (this->focus_item - 1 + MAX_FOCUS_ITEM) % MAX_FOCUS_ITEM;
                    }
                    else {
                        this->focus_item = (this->focus_item + 1) % MAX_FOCUS_ITEM;
                    }
                    this->event->set();
                break;
                case 57: // SPACE
                case 28: // ENTER
                    LOG(LOG_INFO, "button up");
                    this->state = BUTTON_STATE_UP;
                    switch (this->focus_item){
                    case FOCUS_ON_CONNECT:
                    {
                        LOG(LOG_INFO, "Connect");
                        char buffer[1024];
                        sprintf(buffer, "%s:%s",
                            this->grid[this->focus_line].target,
                            this->context.get(STRAUTHID_AUTH_USER));
                        this->context.parse_username(buffer);
                        this->signal = 2;
                        this->event->set();
                    }
                    break;
                    case FOCUS_ON_FILTER_GROUP:
                    case FOCUS_ON_FILTER_DEVICE:
                    {
                        LOG(LOG_INFO, "Filter Group");
                        strcpy(this->context.get(STRAUTHID_SELECTOR), "ASK");
                        *this->context.get(STRAUTHID_SELECTOR_GROUP_FILTER) = '!';
                        strcpy(this->context.get(STRAUTHID_SELECTOR_GROUP_FILTER)+1, this->filter_group_text);
                        *this->context.get(STRAUTHID_SELECTOR_DEVICE_FILTER) = '!';
                        strcpy(this->context.get(STRAUTHID_SELECTOR_DEVICE_FILTER)+1, this->filter_device_text);
                        this->signal = 2;
                        this->event->set();
                    }
                    break;
                    default:
                    break;
                    }
                break;
                default:
                break;
            }
        }
        else { // KEYPRESSED
            switch (param1){
            case 80: // ARROW_DOWN
                this->focus_line = (this->focus_line + 1) % this->nblines();
                this->event->set();
            break;
            case 72: // ARROW_UP
                this->focus_line = (this->focus_line + this->nblines() - 1) % this->nblines();
                this->event->set();
            break;
            case 75: // ARROW_LEFT
                if (this->focus_item == FOCUS_ON_FILTER_GROUP){
                    if (this->filter_group_edit_pos > 0) {
                        this->filter_group_edit_pos--;
                    }
                }
                else if (this->focus_item == FOCUS_ON_FILTER_DEVICE){
                    if (this->filter_device_edit_pos > 0) {
                        this->filter_device_edit_pos--;
                    }
                }
                this->event->set();
            break;
            case 77: // ARROW_RIGHT
                if (this->focus_item == FOCUS_ON_FILTER_GROUP){
                    if (this->filter_group_edit_pos < strlen(this->filter_group_text)
                    && this->filter_group_edit_pos < 10) {
                        this->filter_group_edit_pos++;
                    }
                }
                else if (this->focus_item == FOCUS_ON_FILTER_DEVICE){
                    if (this->filter_device_edit_pos < strlen(this->filter_device_text)
                    && this->filter_device_edit_pos < 20) {
                        this->filter_device_edit_pos++;
                    }
                }
                this->event->set();
            break;
            case 57: // SPACE
            case 28: // ENTER
                LOG(LOG_INFO, "button down");
                this->state = BUTTON_STATE_DOWN;
                this->event->set();
            break;
            default:
            break;
            }
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int draw_event()
    {
        this->draw(this->screen.rect);
        this->event->reset();
        LOG(LOG_INFO, "draw_event : signal = %u", this->signal);
        return this->signal;
    }


    void draw_background(){
        this->gd.opaque_rect(RDPOpaqueRect(this->screen.rect, GREY));
    }

    void draw_login(){
        char buffer[256];
        buffer[0] = 0;
        sprintf(buffer, "%s@%s", this->context.get(STRAUTHID_AUTH_USER), this->context.get(STRAUTHID_HOST));
        this->gd.server_draw_text(30, 30, buffer, GREY, BLACK);
    }

    size_t nblines(){
        uint32_t nb = (this->screen.rect.cy - 230) / 20;
        return (nb > 50)?50:nb;
    }

    void draw_array(){
        LOG(LOG_INFO, "drawing array");

        uint32_t w = (this->screen.rect.cx - 40) / 20;

        this->gd.server_draw_text(30       , 50,  "Device Group", GREY, BLACK);

        LOG(LOG_INFO, "filter_group_text=%s", this->filter_group_text);
        this->gd.draw_edit(Rect(30, 70, 3*w - 15, 20), 0, this->filter_group_text, this->filter_group_edit_pos,
            this->focus_item == FOCUS_ON_FILTER_GROUP);
        this->gd.server_draw_text(30 +  3*w, 50,  "Account Device", GREY, BLACK);

        LOG(LOG_INFO, "filter_device_text=%s", this->filter_device_text);
        this->gd.draw_edit(Rect(30 + 3*w, 70, 10*w - 15, 20), 0, this->filter_device_text, this->filter_device_edit_pos,
            this->focus_item == FOCUS_ON_FILTER_DEVICE);
        this->gd.server_draw_text(30 + 13*w, 50,  "Protocol", GREY, BLACK);
        this->gd.server_draw_text(30 + 16*w, 50,  "Deconnexion Time", GREY, BLACK);

        for (size_t line = 0 ; line < this->nblines() ; line++){
            Rect rect(20, 100 + line * 20, this->screen.rect.cx-40, 19);
            uint32_t c = this->color[line%2];
            if (line == this->focus_line){
                c = this->color[2];
            }
            this->gd.opaque_rect(RDPOpaqueRect(rect, c));
            this->gd.server_draw_text(35       , rect.y + 2,  this->grid[line].group, c, BLACK);
            this->gd.server_draw_text(35 +  3*w, rect.y + 2,  this->grid[line].target, c, BLACK);
            this->gd.server_draw_text(35 + 13*w, rect.y + 2,  this->grid[line].protocol, c, BLACK);
            this->gd.server_draw_text(35 + 16*w, rect.y + 2,  this->grid[line].endtime, c, BLACK);
        }
        Rect r(this->screen.rect.cx - 240, this->screen.rect.cy - 130, 30, 20);
        this->gd.draw_button(r, "|<<",
            (this->focus_item == FOCUS_ON_FIRSTPAGE)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_FIRSTPAGE);
        this->gd.draw_button(r.offset(40,0), "<",
            (this->focus_item == FOCUS_ON_PREVPAGE)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_PREVPAGE);


        char buffer[256];
        sprintf(buffer, " %u/%u ", this->showed_page, this->total_page);
        this->gd.server_draw_text(r.x + 80, r.y + 2, buffer, GREY, BLACK);

        this->gd.draw_button(r.offset(130,0), ">",
            (this->focus_item == FOCUS_ON_NEXTPAGE)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_NEXTPAGE);
        this->gd.draw_button(r.offset(170,0), ">>|",
            (this->focus_item == FOCUS_ON_LASTPAGE)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_LASTPAGE);


    }

    void draw_buttons(){
        Rect r(this->screen.rect.cx-240, this->screen.rect.cy- 100, 60, 25);
        this->gd.draw_button(r, "Logout",
            (this->focus_item == FOCUS_ON_LOGOUT)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_LOGOUT);
        this->gd.draw_button(r.offset(70,0), "Cancel",
            (this->focus_item == FOCUS_ON_CANCEL)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_CANCEL);
        this->gd.draw_button(r.offset(140,0), "Connect",
            (this->focus_item == FOCUS_ON_CONNECT)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_CONNECT);
    }


    void draw(const Rect & clip)
    {
        this->gd.server_begin_update();
        this->gd.server_set_clip(clip);

        this->draw_background();
        this->draw_login();
        this->draw_array();
        this->draw_buttons();

        this->gd.server_end_update();
    }

};

#endif
