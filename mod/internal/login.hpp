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

   Login Window

*/

#if !defined(__LOGIN_HPP__)
#define __LOGIN_HPP__

#include "widget_window_login.hpp"

struct wab_help : public window
{
    Widget & notify_to;
    char help_message[1024];

    wab_help(internal_mod * mod, const Rect & r, Widget & parent, Widget & notify_to, int bg_color, const char * title, const char * help_message)
    : window(mod, r, parent, bg_color, title), notify_to(notify_to)
    {
        strcpy(this->help_message, help_message);
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (msg == 1) { /* click */
            if (sender->id == 1) { /* ok button */
                    this->notify_to.notify(this, 100, 1, 0); /* ok */
            }
        } else if (msg == WM_PAINT) { /* 3 */
            // "Enter target device and login as login@server"
            // "Enter a valid allowed authentication user"
            // "in the username edit box."
            // "Enter the password in the password edit box."
            // "Both the username and password are case"
            // "sensitive."
            // "Contact your system administrator if you are"
            // "having problems logging on.", this->rect.wh());
            char * message = this->help_message;
            int count = 0;
            bool done = false;
            while (!done) {
                char * str = strstr(message, "<br>");
                char tmp[256];
                tmp[0] = 0;
                strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
                tmp[255] = 0;
                this->server_draw_text(this, 10, 30 + 16 * count, tmp, BLACK, this->rect.wh());
                count++;
                if (!str){
                    done = true;
                }
                else {
                    message = str + 4;
                }
            }
        }
        return;
    }

};


struct wab_login : public window_login
{
    wab_login(internal_mod * mod, const Rect & r, ModContext & context, Widget & parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    : window_login(mod, r, context, parent, notify_to, bg_color, title, ini, regular)
    {
        context.get(STRAUTHID_PASSWORD)[0] = 0;

        this->ini = ini;
        struct Widget* but;

        uint32_t grey = 0xc0c0c0;
        /* create help screen */
        this->help = new wab_help(this->mod,
            Rect(this->mod->screen.rect.cx / 2 - 340 / 2,
                this->mod->screen.rect.cy / 2 - 300 / 2,
                340,
                300),
            mod->screen, // parent
            *this, // notify_to
            grey,
            "Login help",
            context.get(STRAUTHID_TRANS_HELP_MESSAGE));

        if (regular) {
            widget_image * but = new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE,
                *this, 10, 30,
                SHARE_PATH "/" LOGIN_LOGO24,
                this->mod->screen.bpp);
            this->child_list.push_back(but);
        }

        /* label */

        #warning WTF isn't strtok first parameter a const char * ?
        char username[sizeof(this->mod->get_client_info().username)];
        memcpy(username, this->mod->get_client_info().username,
                        sizeof(this->mod->get_client_info().username));
        const char * target = strtok(username, ":" );
        if (!target){
            target = "";
        }

        but = new widget_button(this->mod,
            Rect(regular ? 180 : 30, 160, 60, 25),
            *this, 3, 1, context.get(STRAUTHID_TRANS_BUTTON_OK));
        this->child_list.push_back(but);
        this->default_button = but;

        but = new widget_button(this->mod,
              Rect(regular ? 250 : ((r.cx - 30) - 60), 160, 60, 25),
              *this, 2, 1, context.get(STRAUTHID_TRANS_BUTTON_CANCEL));
        this->child_list.push_back(but);
        this->esc_button = but;

        if (regular) {
            but = new widget_button(this->mod,
                  Rect(320, 160, 60, 25), *this, 1, 1, context.get(STRAUTHID_TRANS_BUTTON_HELP));
            this->child_list.push_back(but);
        }

        struct Widget* b;
        int count = 0;
        /* label */
        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
            *this, context.get(STRAUTHID_TRANS_USERNAME));
        this->child_list.push_back(b);

        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ?  230 : 70, 60 + 25 * count, 350, 20),
            *this, context.get(STRAUTHID_AUTH_USER));
        b->id = 100 + 2 * count;
        this->child_list.push_back(b);
        count ++;

        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ? 155 : 5, 60 + 25 * count, 70, 20),
            *this, context.get(STRAUTHID_TRANS_TARGET));
        this->child_list.push_back(b);

        b = new widget_label(this->mod,
            Rect((this->rect.cx >= 400) ?  230 : 70, 60 + 25 * count, 350, 20),
            *this, target);
        b->id = 100 + 2 * count;
        this->child_list.push_back(b);
        count ++;

        b = new widget_label(this->mod,
            Rect(this->rect.cx >= 400 ? 155 : 5, 60 + 25 * count, 70, 20),
            *this, context.get(STRAUTHID_TRANS_PASSWORD));
        b->id = 100 + 2 * count;
        this->child_list.push_back(b);

        /* edit */
        b = new widget_edit(this->mod,
                Rect((this->rect.cx) >= 400 ? 230 : 70, 60 + 25 * count, 350, 20),
                *this,
                100 + 2 * count + 1, /* id */
                1, /* tab stop */
                context.get(STRAUTHID_PASSWORD),
                1, /* pointer */
                0 /* edit pos */);

        #warning integrate that in widget_edit
        b->password_char = '*';

        this->child_list.push_back(b);
        this->focused_control = b;
        b->has_focus = true;
        count++;
    }

    ~wab_login(){
        #warning here destroy all widgets from screen.child_list
        delete this->help;
    }

};


struct combo_help : public window
{
    Widget & notify_to;

    combo_help(internal_mod * mod, const Rect & r, Widget & parent, Widget & notify_to, int bg_color, const char * title)
    : window(mod, r, parent, bg_color, title), notify_to(notify_to)
    {
    }

    virtual void notify(struct Widget* sender, int msg, long param1, long param2)
    {
        if (msg == 1) { /* click */
            if (sender->id == 1) { /* ok button */
                    this->notify_to.notify(this, 100, 1, 0); /* ok */
            }
        } else if (msg == WM_PAINT) { /* 3 */
            #warning the code below is a bit too much specialized. Change it to some code able to write a paragraph of text in a given rectangle. Later we may even add some formatting support.
            const char * message =
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
                    "having problems logging on.<br>"
                    ;
            int count = 0;
            bool done = false;
            while (!done) {
                const char * str = strstr(message, "<br>");
                char tmp[256];
                tmp[0] = 0;
                strncat(tmp, message, str?std::min((size_t)(str-message), (size_t)255):255);
                tmp[255] = 0;
                this->server_draw_text(this, 10, 30 + 16 * count, tmp, BLACK, this->rect.wh());
                count++;
                if (!str){
                    done = true;
                }
                else {
                    message = str + 4;
                }
            }
        }
        return;
    }

};


struct combo_login : public window_login
{
    combo_login(internal_mod * mod, const Rect & r, ModContext & context, Widget & parent, Widget & notify_to, int bg_color, const char * title, Inifile * ini, int regular)
    : window_login(mod, r, context, parent, notify_to, bg_color, title, ini, regular)
    {
        this->ini = ini;
        struct Widget* but;

        /* create help screen */
        uint32_t grey = 0xc0c0c0;
        this->help = new combo_help(this->mod,
            Rect(this->mod->screen.rect.cx / 2 - 340 / 2,
                this->mod->screen.rect.cy / 2 - 300 / 2,
                340,
                300),
            mod->screen, // parent
            *this, // notify_to
            grey,
            "Login help");

        if (regular) {
            widget_image * but = new widget_image(this->mod, 4, 4, WND_TYPE_IMAGE, *this, 10, 30,
                    SHARE_PATH "/" LOGIN_LOGO24, this->mod->screen.bpp);
            this->child_list.push_back(but);
        }

        /* label */
        but = new widget_label(this->mod, Rect((regular ? 155 : 5), 35, 60, 20), *this,  "Module");
        this->child_list.push_back(but);

        Rect rect(regular ? 230 : 70, 35, 350, 20);
        this->combo = new widget_combo(this->mod, rect, *this, 6, 1);

        #warning add this to combo through constructor (pass in an array of strings ?) a list of pairs with id and string could be better.
        for (int i = 0; i < 6 ; i++){
            if (ini->account[i].accountdefined){
                this->combo->string_list.push_back(strdup(ini->account[i].accountname));
            }
        }

        #warning select the index of the first account
        for (int i = 0; i < 6 ; i++){
            if (ini->account[i].accountdefined){
                this->combo->item_index = i;
                break;
            }
        }

        this->child_list.push_back(this->combo);

        struct IniAccounts * acc = this->current_account();
        const char * target_protocol = "RDP";
        switch (acc->idlib){
            case ID_LIB_VNC:
                target_protocol = "VNC";
            break;
            case ID_LIB_XUP:
                target_protocol = "XUP";
            break;
            default:;
        }
        this->context.cpy(STRAUTHID_TARGET_PROTOCOL, target_protocol);

        #warning valgrind say there is a memory leak here
        but = new widget_button(this->mod,
              Rect(regular ? 180 : 30, 160, 60, 25),
              *this, 3, 1, context.get(STRAUTHID_TRANS_BUTTON_OK));
        this->child_list.push_back(but);
        this->default_button = but;

        #warning valgrind say there is a memory leak here
        but = new widget_button(this->mod,
              Rect(regular ? 250 : ((r.cx - 30) - 60), 160, 60, 25),
              *this, 2, 1, context.get(STRAUTHID_TRANS_BUTTON_CANCEL));
        this->child_list.push_back(but);
        this->esc_button = but;

        if (regular) {
        #warning valgrind say there is a memory leak here
            but = new widget_button(this->mod,
                  Rect(320, 160, 60, 25), *this, 1, 1, context.get(STRAUTHID_TRANS_BUTTON_HELP));
            this->child_list.push_back(but);
        }

         this->login_window_show_edits();
    }

    ~combo_login(){
        delete this->help;
    }

};

struct login_mod : public internal_mod {
    struct window_login * login_window;
    Widget* popup_wnd;
    Widget* button_down;


    login_mod(wait_obj * event,
            int (& keys)[256], int & key_flags, Keymap * &keymap,
            ModContext & context, Front & front, Inifile * ini)
            : internal_mod(keys, key_flags, keymap, front)
    {
        this->event = event;
        this->signal = 0;
        this->button_down = 0;

        int log_width = 600;
        int log_height = 200;
        int regular = 1;

        this->popup_wnd = 0;
        if (this->screen.rect.cx < log_width ) {
            log_width = std::min(this->screen.rect.cx - 4, 240);
            regular = 0;
        }

        /* draw login window */
        Rect r(
            this->screen.rect.cx / 2 - log_width / 2,
            this->screen.rect.cy / 2 - log_height / 2,
            log_width,
            log_height);

        #warning having two completely different LOGIN modules one with password only, the other one with full behavior would probably be much more clean than passing around that wab_auth. See that.
        if (context.wab_auth){
            this->login_window = new wab_login(this,
                r, context,
                this->screen, // parent
                this->screen, // notify_to
                GREY,
                "Login",
                ini,
                regular);
        }
        else {
            this->login_window = new combo_login(this,
                r, context,
                this->screen, // parent
                this->screen, // notify_to
                GREY,
                "Login",
                ini,
                regular);
        }

        this->screen.child_list.push_back(this->login_window);
        assert(this->login_window->mod == this);

//        LOG(LOG_INFO, "loading image...");

        if (regular) {
            /* image */
            widget_image * but = new widget_image(this, 4, 4,
                WND_TYPE_IMAGE,
                this->screen,
                this->screen.rect.cx - 250 - 4,
                this->screen.rect.cy - 120 - 4,
                SHARE_PATH "/" REDEMPTION_LOGO24,
                this->screen.bpp);

            this->screen.child_list.push_back(but);
        }

//        LOG(LOG_INFO, "focus on login_window");

        this->login_window->focus(this->login_window->rect);

        this->screen.refresh(this->screen.rect.wh());
//        LOG(LOG_INFO, "rdp_input_invalidate screen done");
    }

    virtual ~login_mod()
    {
        #warning here destroy all widgets from screen.child_list
    }

    /*****************************************************************************/
    int clear_popup()
    {
        #warning simplify that
        if (this->popup_wnd != 0) {

            vector<Widget*>::iterator to_erase;
            for (vector<Widget*>::iterator it = this->screen.child_list.begin()
                ; it != this->screen.child_list.end()
                ; it++){
                if (*it == this->popup_wnd){
                    to_erase = it;
                }
            }
            this->screen.child_list.erase(to_erase);

            #warning below inlining of bogus rdp_input_invalidate_clip
            this->server_begin_update();
            this->screen.draw(this->popup_wnd->rect);

            /* notify */
            this->screen.notify(this->get_screen_wdg(), WM_PAINT, 0, 0); /* 3 */

            /* draw any child windows in the area */
            int count = this->screen.child_list.size();
            for (int i = 0; i < count; i++) {
                Widget* b = this->screen.child_list.at(i);
                Rect r2 = this->popup_wnd->rect.intersect(b->rect);
                if (!r2.isempty()) {
                    r2 = r2.offset(-(b->rect.x), -(b->rect.y));
                    b->refresh_clip(r2);
                }
            }

            this->server_end_update();

            delete this->popup_wnd;
            this->popup_wnd = 0;
        }
        return 0;
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->server_begin_update();
            Rect & r = this->screen.rect;
            this->screen.fill_rect(0xCC, r, this->screen.bg_color, r);
            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    b->refresh_clip(r2);
                }
            }
            this->server_end_update();
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y)
    {
        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            this->input_event(WM_MOUSEMOVE, x, y, 0, 0, this->key_flags, this->keys);
            this->front.mouse_x = x;
            this->front.mouse_y = y;

        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            this->input_event(
                WM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            this->input_event(
                WM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            this->input_event(
                WM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
            this->input_event(WM_BUTTON4DOWN, x, y, 0, 0, this->key_flags, this->keys);
            this->input_event(WM_BUTTON4UP, x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
            this->input_event(WM_BUTTON5DOWN, x, y, 0, 0, this->key_flags, this->keys);
            this->input_event(WM_BUTTON5UP, x, y, 0, 0, this->key_flags, this->keys);
        }
    }

    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, const int key_flags, const int (& keys)[256], struct key_info* ki){
        if (ki != 0) {
            int msg = (device_flags & KBD_FLAG_UP)?WM_KEYUP:WM_KEYDOWN;
            this->input_event(msg, ki->chr, ki->sym, param1, device_flags, key_flags, keys);
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    // module received an event from client
    virtual int input_event(const int msg, const long x, const long y, const long param3, const long param4, const int key_flags, const int (& keys)[256])
    {
        LOG(LOG_INFO, "login input_event(%d, %ld, %ld, %ld %ld)", msg, x, y, param3, param4);
        switch (msg){
        case WM_KEYDOWN:
            if (!this->popup_wnd && this->login_window->has_focus) {
                this->login_window->def_proc(msg, param3, param4, this->key_flags, this->keys);
            }
        break;
        case WM_KEYUP:
            if (this->popup_wnd != 0) {
                this->clear_popup();
            }
        break;
        case WM_MOUSEMOVE:
//            LOG(LOG_INFO, "dragging = %d\n", this->dragging);
            if (this->dragging) {
                long dragx = (x < 0)                         ? 0
                           : (x < this->screen.rect.cx) ? x
                           : this->screen.rect.cx
                           ;

                long dragy = (y < 0)                         ? 0
                           : (y < this->screen.rect.cy) ? y
                           : this->screen.rect.cy
                           ;

                this->server_begin_update();
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                this->dragging_rect.x = dragx - this->draggingdx ;
                this->dragging_rect.y = dragy - this->draggingdy;
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                this->server_end_update();
            }
            else {
                struct Widget *b = this->screen.widget_at_pos(x, y);
                if (b == 0) { /* if b is null, the movement must be over the screen */
                    b = this->get_screen_wdg();
                }
                if (b->pointer != this->current_pointer) {
                    this->set_pointer(b->pointer);
                }
                b->def_proc(WM_MOUSEMOVE, b->from_screenx(x), b->from_screeny(y), this->key_flags, this->keys);
                if (this->button_down) {
                    this->button_down->state = (b == this->button_down);
                    this->button_down->refresh(this->button_down->rect.wh());
                }
                else {
                    b->notify(&b->parent, 2, x, y);
                }
            }
        break;
        case WM_LBUTTONDOWN:
        {
            /* loop on surface widgets on screen to find active window */
            Widget* wnd = this->get_screen_wdg();
            for (size_t i = 0; i < wnd->child_list.size(); i++) {
                if (wnd->child_list[i]->rect.rect_contains_pt(x, y)) {
                    wnd = this->screen.child_list[i];
                    break;
                }
            }

            /* set focus on window */
            if (wnd && wnd->type == WND_TYPE_WND) {
                wnd->focus();
            }

            Widget * control = wnd->widget_at_pos(x, y);

            if (wnd != this->get_screen_wdg()) {
                if (wnd->modal_dialog != 0) {
                    /* window has a modal dialog (but we didn't clicked on it) */
                    break;
                }
                // change focus. Is graphical feedback necessary ?
                if (control != wnd && control->tab_stop) {
                    #warning control that had focus previously does not loose it, easy way could be to loop on all controls and clear all existing focus
                    control->has_focus = true;
                    for (size_t i = 0; i < wnd->child_list.size(); i++) {
                        wnd->child_list[i]->has_focus = false;
                        wnd->child_list[i]->refresh(wnd->child_list[i]->rect.wh());
                    }
                    control->refresh(control->rect.wh());
                }
            }

            switch (control->type) {
                case WND_TYPE_BUTTON:
                    this->button_down = control;
                    control->state = 1;
                    control->refresh(control->rect.wh());
                break;
                case WND_TYPE_COMBO:
                    this->button_down = control;
                    control->state = 1;
                    control->refresh(control->rect.wh());
                    this->popup_wnd = new widget_popup(this,
                                Rect(
                                control->to_screenx(),
                                control->to_screeny() + control->rect.cy,
                                control->rect.cx,
                                100),
                            control, // popped_from
                            this->screen, // parent
                            control->item_index); // item_index

                    this->screen.child_list.insert(this->screen.child_list.begin(), this->popup_wnd);
                    this->popup_wnd->refresh(this->popup_wnd->rect.wh());
                break;
                case WND_TYPE_WND:
                    /* drag by clicking in title bar and keeping button down */
                    if (y < (control->rect.y + 21)) {
                        this->dragging = 1;
                        this->dragging_window = control;
                        this->draggingdx = x - control->rect.x;
                        this->draggingdy = y - control->rect.y;
                        this->dragging_rect = Rect(
                            x - this->draggingdx, y - this->draggingdy,
                            control->rect.cx, control->rect.cy);
                        this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);
                    }
                break;
                default:
                break;
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            if (this->dragging) {
                /* if done dragging */
                /* draw xor box one more time */
                this->server_draw_dragging_rect(this->dragging_rect, this->screen.rect);

                /* move dragged window to new location */
                Rect r = this->dragging_window->rect;
                this->dragging_window->rect.x = this->dragging_rect.x;
                this->dragging_window->rect.y = this->dragging_rect.y;
                //this->dragging_window->refresh_clip(r);
                this->screen.refresh(this->screen.rect.wh());
                this->dragging_window = 0;
                this->dragging = 0;
                break;
            }

            /* loop on surface widgets on screen to find active window */
            Widget* wnd = this->get_screen_wdg();
            for (size_t i = 0; i < wnd->child_list.size(); i++) {
                if (wnd->child_list[i]->rect.rect_contains_pt(x, y)) {
                    wnd = this->screen.child_list[i];
                    break;
                }
            }

            Widget * control = wnd->widget_at_pos(x, y);

            // popup is opened
            if (this->popup_wnd) {
                // click inside popup
                if (this->popup_wnd == control){
                    this->popup_wnd->def_proc(WM_LBUTTONUP, x, y, this->key_flags, this->keys);
                }
                // clear popup
                this->clear_popup();
                this->screen.refresh(this->screen.rect.wh());
                break;
            }

            if (wnd != this->get_screen_wdg()) {
                if (wnd->modal_dialog != 0) {
                    /* window has a modal dialog (but we didn't clicked on it) */
                    break;
                }
                if (control != wnd && control->tab_stop) {
                #warning previous focus on other control is not yet disabled
                    control->has_focus = true;
                    control->refresh(control->rect.wh());
                }
            }

            switch (control->type) {
                case WND_TYPE_BUTTON:
                case WND_TYPE_COMBO:
                    if (this->button_down == control){
                        control->state = 0;
                        control->refresh(control->rect.wh());
                        control->notify(control, 1, x, y);
                    }
                break;
                default:
                break;
            }
            // mouse is up, no more button down, whatever
            this->button_down = 0;
        }
        break;

        default:
            /* internal : redemption interface only use button 1 */
            break;
        }
        return 0;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual int draw_event()
    {
        return signal;
    }

};

#endif
