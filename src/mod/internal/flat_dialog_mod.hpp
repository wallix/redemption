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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_DIALOG_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_DIALOG_MOD_HPP

#include "utils/translation.hpp"
#include "core/front_api.hpp"
#include "configs/config.hpp"
#include "widget2/language_button.hpp"
#include "widget2/flat_dialog.hpp"
#include "widget2/screen.hpp"
#include "configs/config_access.hpp"
#include "internal_mod.hpp"
#include "utils/timeout.hpp"



using FlatDialogModVariables = vcfg::variables<
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::get>,
    vcfg::var<cfg::context::accept_message,             vcfg::set>,
    vcfg::var<cfg::context::display_message,            vcfg::set>,
    vcfg::var<cfg::context::password,                   vcfg::set>,
    vcfg::var<cfg::debug::pass_dialog_box,              vcfg::get>,
    vcfg::var<cfg::translation::language,               vcfg::get>,
    vcfg::var<cfg::font,                                vcfg::get>,
    vcfg::var<cfg::theme,                               vcfg::get>
>;

class FlatDialogMod : public InternalMod, public NotifyApi
{
    LanguageButton language_button;
    FlatDialog dialog_widget;

    FlatDialogModVariables vars;
    Timeout   timeout;

public:
    FlatDialogMod(FlatDialogModVariables vars, FrontAPI & front, uint16_t width, uint16_t height, Rect const & widget_rect,
                  const char * caption, const char * message, const char * cancel_text,
                  time_t now, ChallengeOpt has_challenge = NO_CHALLENGE)
        : InternalMod(front, width, height, vars.get<cfg::font>(), vars.get<cfg::theme>())
        , language_button(vars.get<cfg::client::keyboard_layout_proposals>().c_str(), this->dialog_widget, *this, front, this->font(), this->theme())
        , dialog_widget(*this, widget_rect.x, widget_rect.y, widget_rect.cx + 1, widget_rect.cy + 1, this->screen, this, caption, message, 0,
                        &this->language_button,
                        vars.get<cfg::theme>(), vars.get<cfg::font>(),
                        TR("OK", language(vars)),
                        cancel_text, has_challenge)
        , vars(vars)
        , timeout(now, vars.get<cfg::debug::pass_dialog_box>())
    {
        this->screen.add_widget(&this->dialog_widget);
        this->dialog_widget.set_widget_focus(&this->dialog_widget.ok, Widget2::focus_reason_tabkey);
        this->screen.set_widget_focus(&this->dialog_widget, Widget2::focus_reason_tabkey);
        this->screen.refresh(this->screen.rect);

        if (this->dialog_widget.challenge) {
            this->dialog_widget.set_widget_focus(this->dialog_widget.challenge, Widget2::focus_reason_tabkey);
            // this->vars.get<cfg::to_send_set::insert>()(AUTHID_AUTHENTICATION_CHALLENGE);
        }
    }

    ~FlatDialogMod() override {
        this->screen.clear();
    }

    void notify(Widget2* sender, notify_event_t event) override {
        switch (event) {
            case NOTIFY_SUBMIT: this->accepted(); break;
            case NOTIFY_CANCEL: this->refused(); break;
            default: ;
        }
    }

private:
    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
    void accepted()
    {
        if (this->dialog_widget.challenge) {
            this->vars.set_acl<cfg::context::password>(this->dialog_widget.challenge->get_text());
        }
        else if (this->dialog_widget.cancel) {
            this->vars.set_acl<cfg::context::accept_message>(true);
        }
        else {
            this->vars.set_acl<cfg::context::display_message>(true);
        }
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
    void refused()
    {
        if (!this->dialog_widget.challenge) {
            if (this->dialog_widget.cancel) {
                this->vars.set_acl<cfg::context::accept_message>(false);
            }
            else {
                this->vars.set_acl<cfg::context::display_message>(false);
            }
        }
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    void draw_event(time_t now) override {
        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->accepted();
            break;
        case Timeout::TIMEOUT_NOT_REACHED:
            this->event.set(1000000);
            break;
        default:
            this->event.reset();
            break;
        }
    }

};

#endif
