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
 *
 */

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_WAB_CLOSE_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_WAB_CLOSE_MOD_HPP

#include "front_api.hpp"
#include "mod_api.hpp"
#include "widget2/flat_wab_close.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"
#include "utils/timeout.hpp"
#include "configs/config_access.hpp"


using FlatWabCloseModVariables = vcfg::variables<
    vcfg::var<cfg::globals::auth_user,          vcfg::get | vcfg::is_asked>,
    vcfg::var<cfg::globals::target_device,      vcfg::get | vcfg::ask | vcfg::is_asked>,
    vcfg::var<cfg::globals::target_user,        vcfg::get | vcfg::ask>,
    vcfg::var<cfg::context::selector,           vcfg::ask>,
    vcfg::var<cfg::context::target_protocol,    vcfg::ask>,
    vcfg::var<cfg::globals::close_timeout,      vcfg::get>,
    vcfg::var<cfg::globals::target_application, vcfg::get>,
    vcfg::var<cfg::context::auth_error_message, vcfg::get>,
    vcfg::var<cfg::context::module,             vcfg::get>,
    vcfg::var<cfg::translation::language,       vcfg::get>,
    vcfg::var<cfg::font,                        vcfg::get>,
    vcfg::var<cfg::theme,                       vcfg::get>
>;

class FlatWabCloseMod : public InternalMod, public NotifyApi
{
    FlatWabClose     close_widget;
    Timeout timeout;
    FlatWabCloseModVariables vars;

private:
    bool showtimer;
    struct temporary_text {
        char text[255];

        explicit temporary_text(FlatWabCloseModVariables vars)
        {
            if (vars.get<cfg::context::module>() == "selector") {
                snprintf(text, sizeof(text), "%s", TR("selector", language(vars)));
            }
            else {
                TODO("target_application only used for user message, the two branches of alternative should be unified et message prepared by sesman")
                if (!vars.get<cfg::globals::target_application>().empty()) {
                    snprintf(text, sizeof(text), "%s",
                             vars.get<cfg::globals::target_application>().c_str());
                }
                else {
                    snprintf(text, sizeof(text), "%s@%s",
                             vars.get<cfg::globals::target_user>().c_str(),
                             vars.get<cfg::globals::target_device>().c_str());
                }
            }
        }
    };

public:
    FlatWabCloseMod(FlatWabCloseModVariables vars,
                    FrontAPI & front, uint16_t width, uint16_t height, time_t now,
                    bool showtimer = false, bool back_selector = false)
        : InternalMod(front, width, height, vars.get<cfg::font>(), vars.get<cfg::theme>())
        , close_widget(*this, width, height, this->screen, this,
                       vars.get<cfg::context::auth_error_message>().c_str(), 0,
                       (vars.is_asked<cfg::globals::auth_user>()
                        || vars.is_asked<cfg::globals::target_device>()) ?
                       nullptr : vars.get<cfg::globals::auth_user>().c_str(),
                       (vars.is_asked<cfg::globals::auth_user>()
                        || vars.is_asked<cfg::globals::target_device>()) ?
                       nullptr : temporary_text(vars).text,
                       showtimer,
                       vars.get<cfg::font>(),
                       vars.get<cfg::theme>(),
                       language(vars),
                       back_selector)
        , timeout(now, vars.get<cfg::globals::close_timeout>())
        , vars(vars)
        , showtimer(showtimer)
    {
        if (vars.get<cfg::globals::close_timeout>()) {
            LOG(LOG_INFO, "WabCloseMod: Ending session in %u seconds", vars.get<cfg::globals::close_timeout>());
        }
        this->front.set_palette(BGRPalette::classic_332());

        this->screen.add_widget(&this->close_widget);
        this->close_widget.set_widget_focus(&this->close_widget.cancel, Widget2::focus_reason_tabkey);
        this->screen.set_widget_focus(&this->close_widget, Widget2::focus_reason_tabkey);

        this->screen.refresh(this->screen.rect);
    }

    ~FlatWabCloseMod() override {
        this->screen.clear();
    }

    void notify(Widget2* sender, notify_event_t event) override {
        if (NOTIFY_CANCEL == event) {
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
        }
        else if (NOTIFY_SUBMIT == event) {
            LOG(LOG_INFO, "asking for selector");
            this->vars.ask<cfg::context::selector>();
            this->vars.ask<cfg::globals::target_user>();
            this->vars.ask<cfg::globals::target_device>();
            this->vars.ask<cfg::context::target_protocol>();
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();
        }
    }

    void draw_event(time_t now) override {
        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            break;
        case Timeout::TIMEOUT_NOT_REACHED:
            if (this->showtimer) {
                this->close_widget.refresh_timeleft(this->timeout.timeleft(now));
            }
            this->event.set(200000);
            break;
        default:
            this->event.reset();
            break;
        }
    }
};

#endif
