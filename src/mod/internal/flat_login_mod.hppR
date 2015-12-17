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
#include "config_access.hpp"
#include "apply_for_delim.hpp"


using FlatLoginModVariables = vcfg::variables<
    vcfg::var<cfg::context::password,   vcfg::write>,
    vcfg::var<cfg::globals::auth_user,  vcfg::write>,
    vcfg::var<cfg::context::selector,           vcfg::ask | vcfg::write>,
    vcfg::var<cfg::context::target_protocol,    vcfg::ask | vcfg::read>,
    vcfg::var<cfg::globals::target_device,      vcfg::ask | vcfg::read>,
    vcfg::var<cfg::globals::target_user,        vcfg::ask | vcfg::read>,
    vcfg::var<cfg::translation::language>,
    vcfg::var<cfg::font>,
    vcfg::var<cfg::theme>,
    vcfg::var<cfg::context::opt_message, vcfg::read>,
    vcfg::var<cfg::client::keyboard_layout_proposals, vcfg::read>
>;


class FlatLoginMod : public InternalMod, public NotifyApi
{
    class ButtonLanguage : public WidgetFlatButton
    {
        static constexpr size_t locale_name_len = 5;
        struct Loc {
            char const * locale_name;
            int LCID;
        };
        std::vector<Loc> locales;
        unsigned selected_language = 0;
        FrontAPI & front;
        Widget2 & parent;

    public:
        ButtonLanguage(
            std::string const & enable_locales,
            Widget2 & parent,
            DrawApi & drawable,
            FrontAPI & front,
            Font const & font,
            Theme const & theme
        )
        : WidgetFlatButton(drawable, 0, 0, *this, this, nullptr, true, -1,
                           theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, font, 7, 7)
        , front(front)
        , parent(parent)
        {
            using std::begin;
            using std::end;

            auto LCID = front.get_keylayout();

            {
                auto it = std::find_if(begin(keylayouts), end(keylayouts), [&](Keylayout const * k){
                    return k->LCID == LCID;
                });
                if (it == end(keylayouts)) {
                    LCID = keylayout_x00000409.LCID;
                    this->locales.push_back({keylayout_x00000409.locale_name, keylayout_x00000409.LCID});
                }
                else {
                    this->locales.push_back({(*it)->locale_name, (*it)->LCID});
                }
            }

            apply_for_delim(enable_locales.c_str(), ',', [&](char const * & cstr) {
                char const * cend = cstr;
                while (*cend && *cend != ' ' && *cend != '\t' && *cend != ',') {
                    ++cend;
                }

                auto it = std::find_if(begin(keylayouts), end(keylayouts), [&](Keylayout const * k){
                    return strncmp(k->locale_name, cstr, cend-cstr) == 0;
                });
                if (it != end(keylayouts)) {
                    if ((*it)->LCID != LCID) {
                        this->locales.push_back({(*it)->locale_name, (*it)->LCID});
                    }
                }
                else {
                    LOG(LOG_WARNING, "Layout \"%.*s\" not found.", static_cast<int>(cend - cstr), cstr);
                }

                cstr = cend;
            });

            this->label.set_text(this->locales[0].locale_name);
            this->set_button_cx(this->label.rect.cx);
            this->set_button_cy(this->label.rect.cy);
        }

        void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
            if (event == NOTIFY_SUBMIT || event == MOUSE_FLAG_BUTTON1) {
                auto rect = this->rect;

                this->selected_language = (this->selected_language + 1) % this->locales.size();
                this->label.set_text(this->locales[this->selected_language].locale_name);

                this->set_button_cx(this->label.rect.cx);
                this->set_button_cy(this->label.rect.cy);
                rect.cx = std::max(rect.cx, this->rect.cx);
                rect.cy = std::max(rect.cy, this->rect.cy);
                this->parent.draw(rect);

                front.set_keylayout(this->locales[this->selected_language].LCID);
            }
        }
    };

    ButtonLanguage button_language;

    FlatLogin login;

    CopyPaste copy_paste;

    FlatLoginModVariables vars;

public:
    FlatLoginMod(
        FlatLoginModVariables vars,
        char const * username, char const * password,
        FrontAPI & front, uint16_t width, uint16_t height
    )
        : InternalMod(front, width, height, vars.get<cfg::font>(), vars.get<cfg::theme>())
        , button_language(vars.get<cfg::client::keyboard_layout_proposals>().c_str(), this->login, *this, front, this->font(), this->theme())
        , login(*this, width, height, this->screen, this, "Redemption " VERSION,
                username[0] != 0,
                0, nullptr, nullptr,
                TR("login", language(vars)),
                TR("password", language(vars)),
                vars.get<cfg::context::opt_message>().c_str(),
                &this->button_language,
                this->font(), Translator(language(vars)), this->theme())
        , vars(vars)
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
            this->vars.set_acl<cfg::globals::auth_user>(this->login.login_edit.get_text());
            this->vars.ask<cfg::context::selector>();
            this->vars.ask<cfg::globals::target_user>();
            this->vars.ask<cfg::globals::target_device>();
            this->vars.ask<cfg::context::target_protocol>();
            this->vars.set_acl<cfg::context::password>(this->login.password_edit.get_text());
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

    void send_to_mod_channel(const char * front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        if (this->copy_paste && !strcmp(front_channel_name, CHANNELS::channel_names::cliprdr)) {
            this->copy_paste.send_to_mod_channel(chunk, flags);
        }
    }
};

#endif
