/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_LOGIN_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_LOGIN_MOD_HPP

#include "version.hpp"
#include "front_api.hpp"
#include "widget2/flat_login.hpp"
#include "internal_mod.hpp"
#include "widget2/notify_api.hpp"
#include "translation.hpp"
#include "copy_paste.hpp"

#include <functional>

class FlatLoginMod : public InternalMod, public NotifyApi
{
    FlatLogin login;

    CopyPaste copy_paste;

public:
    using submit_signature_type = void(char const * username, char const * password);
    using submit_function_type = std::function<submit_signature_type>;

private:
    submit_function_type submit_notify;

public:
    FlatLoginMod(
        submit_function_type submit_notify,
        char const * username, char const * password,
        FrontAPI & front, uint16_t width, uint16_t height,
        Translator const & tr, Font const & font, Theme const & theme = Theme()
    )
        : InternalMod(front, width, height, font, theme)
        , login(*this, width, height, this->screen, this, "Redemption " VERSION,
                username[0] != 0,
                0, nullptr, nullptr, tr("login"), tr("password"), this->font(), tr, this->theme())
        , submit_notify(std::move(submit_notify))
    {
        this->screen.add_widget(&this->login);

        this->login.login_edit.set_text(username);
        this->login.password_edit.set_text(password);

        this->screen.set_widget_focus(&this->login, Widget2::focus_reason_tabkey);

        this->login.set_widget_focus(&this->login.login_edit, Widget2::focus_reason_tabkey);
        if (username[0] != 0){
            this->login.set_widget_focus(&this->login.password_edit, Widget2::focus_reason_tabkey);
        }

        this->screen.refresh(this->screen.rect);
    }

    ~FlatLoginMod() override {
        this->screen.clear();
    }

    void notify(Widget2* sender, notify_event_t event) override {
        switch (event) {
        case NOTIFY_SUBMIT:
            LOG(LOG_INFO, "asking for selector");
            this->submit_notify(this->login.login_edit.get_text(), this->login.password_edit.get_text());
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            break;
        default:
            if (this->copy_paste) {
                copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event);
            }
            break;
        }
    }

    void draw_event(time_t now) override {
        if (!this->copy_paste && event.waked_up_by_time) {
            this->copy_paste.ready(this->front);
        }
        this->event.reset();
    }

    void send_to_mod_channel(const char * front_channel_name, Stream& chunk, size_t length, uint32_t flags) override {
        if (this->copy_paste) {
            this->copy_paste.send_to_mod_channel(chunk, flags);
        }
    }
};

#endif
