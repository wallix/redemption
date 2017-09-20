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

#include "mod/internal/flat_wab_close_mod.hpp"
#include "configs/config.hpp"
#include "core/front_api.hpp"

namespace
{
    struct temporary_text
    {
        char text[255];

        explicit temporary_text(FlatWabCloseModVariables vars)
        {
            if (vars.get<cfg::context::module>() == "selector") {
                snprintf(text, sizeof(text), "%s", TR(trkeys::selector, language(vars)));
            }
            else {
                // TODO target_application only used for user message, the two branches of alternative should be unified et message prepared by sesman
                if (!vars.get<cfg::globals::target_application>().empty()) {
                    snprintf(
                        text, sizeof(text), "%s",
                        vars.get<cfg::globals::target_application>().c_str());
                }
                else {
                    snprintf(
                        text, sizeof(text), "%s@%s",
                        vars.get<cfg::globals::target_user>().c_str(),
                        vars.get<cfg::globals::target_device>().c_str());
                }
            }
        }
    };
}

FlatWabCloseMod::FlatWabCloseMod(
    FlatWabCloseModVariables vars, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, time_t now, ClientExecute & client_execute,
    bool showtimer, bool back_selector
)
    : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
    , close_widget(
        front, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy, this->screen, this,
        vars.get<cfg::context::auth_error_message>().c_str(),
        (vars.is_asked<cfg::globals::auth_user>() || vars.is_asked<cfg::globals::target_device>())
            ? nullptr
            : vars.get<cfg::globals::auth_user>().c_str(),
        (vars.is_asked<cfg::globals::auth_user>() || vars.is_asked<cfg::globals::target_device>())
            ? nullptr
            : temporary_text(vars).text,
        showtimer,
        vars.get<cfg::context::close_box_extra_message>().c_str(),
        vars.get<cfg::font>(),
        vars.get<cfg::theme>(),
        language(vars),
        back_selector)
    , timeout(now, vars.get<cfg::globals::close_timeout>().count())
    , vars(vars)
    , showtimer(showtimer)
{
    if (vars.get<cfg::globals::close_timeout>().count()) {
        LOG(LOG_INFO, "WabCloseMod: Ending session in %u seconds",
            static_cast<unsigned>(vars.get<cfg::globals::close_timeout>().count()));
    }
    this->front.set_palette(BGRPalette::classic_332());

    this->screen.add_widget(&this->close_widget);
    this->close_widget.set_widget_focus(&this->close_widget.cancel, Widget::focus_reason_tabkey);
    this->screen.set_widget_focus(&this->close_widget, Widget::focus_reason_tabkey);

    this->screen.rdp_input_invalidate(this->screen.get_rect());
}

FlatWabCloseMod::~FlatWabCloseMod()
{
    this->vars.set<cfg::context::close_box_extra_message>("");

    this->screen.clear();
}

void FlatWabCloseMod::notify(Widget* sender, notify_event_t event)
{
    (void)sender;
    if (NOTIFY_CANCEL == event) {
        this->event.signal = BACK_EVENT_STOP;
        this->event.set_trigger_time(wait_obj::NOW);
    }
    else if (NOTIFY_SUBMIT == event) {
        LOG(LOG_INFO, "asking for selector");
        this->vars.ask<cfg::context::selector>();
        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::target_protocol>();
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set_trigger_time(wait_obj::NOW);
    }
}

void FlatWabCloseMod::draw_event(time_t now, gdi::GraphicApi & gapi)
{
    LocallyIntegrableMod::draw_event(now, gapi);

    switch(this->timeout.check(now)) {
    case Timeout::TIMEOUT_REACHED:
        this->event.signal = BACK_EVENT_STOP;
        this->event.set_trigger_time(wait_obj::NOW);
        break;
    case Timeout::TIMEOUT_NOT_REACHED:
        if (this->showtimer) {
            this->close_widget.refresh_timeleft(this->timeout.timeleft(now));
        }
        this->event.set_trigger_time(200000);
        break;
    case Timeout::TIMEOUT_INACTIVE:
        this->event.reset_trigger_time();
        break;
    }
}
