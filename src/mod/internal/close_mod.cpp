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

#include "configs/config.hpp"
#include "mod/internal/close_mod.hpp"
#include "utils/timebase.hpp"

static FlatWabClose build_close_widget(
    gdi::GraphicApi & drawable,
    Rect const widget_rect,
    CloseMod & mod,
    WidgetScreen & screen,
    char const* auth_error_message,
    CloseModVariables vars,
    Font const& font, Theme const& theme, bool back_selector)
{
    struct temporary_text
    {
        char text[255];

        explicit temporary_text(CloseModVariables vars)
        {
            if (vars.get<cfg::context::module>() == ModuleName::selector) {
                snprintf(text, sizeof(text), "%s", TR(trkeys::selector, language(vars)).c_str());
            }
            else {
                // TODO target_application only used for user message,
                // the two branches of alternative should be unified et message prepared by sesman
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

    const bool is_asked = (
        vars.is_asked<cfg::globals::auth_user>()
     || vars.is_asked<cfg::globals::target_device>());

    return FlatWabClose(
        drawable, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        screen, &mod, auth_error_message,
        is_asked ? nullptr : vars.get<cfg::globals::auth_user>().c_str(),
        is_asked ? nullptr : temporary_text(vars).text,
        true,
        vars.get<cfg::context::close_box_extra_message>().c_str(),
        font, theme, language(vars), back_selector);
}

CloseMod::CloseMod(
    char const* auth_error_message,
    CloseModVariables vars,
    EventContainer& events,
    gdi::GraphicApi & gd,
    FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme, bool back_selector)
    : RailModBase(
        events, gd, front,
        width, height, rail_client_execute, font, theme)
    , close_widget(
        build_close_widget(gd, widget_rect, *this, this->screen, auth_error_message, vars, font, theme, back_selector))
    , vars(vars)
    , events_guard(events)
{
    if (vars.get<cfg::globals::close_timeout>().count()) {
        LOG(LOG_INFO, "WabCloseMod: Ending session in %u seconds",
            static_cast<unsigned>(vars.get<cfg::globals::close_timeout>().count()));
    }

    this->screen.add_widget(&this->close_widget);
    this->close_widget.set_widget_focus(&this->close_widget.cancel, Widget::focus_reason_tabkey);
    this->screen.set_widget_focus(&this->close_widget, Widget::focus_reason_tabkey);

    if (vars.get<cfg::globals::close_timeout>().count()) {
        this->close_widget.refresh_timeleft(vars.get<cfg::globals::close_timeout>().count());
    }

    this->screen.rdp_input_invalidate(this->screen.get_rect());

    this->events_guard.create_event_timeout(
        "Close Event",
        this->vars.get<cfg::globals::close_timeout>(),
        [this](Event&e)
        {
            this->set_mod_signal(BACK_EVENT_STOP);
            e.garbage = true;
        });

    auto start_time = this->events_guard.get_current_time();
    this->events_guard.create_event_timeout(
        "Close Refresh Message Event",
        start_time,
        [this, tv_sec = start_time.tv_sec](Event& event)
        {
            event.alarm.reset_timeout(1s);
            auto elapsed = event.alarm.now.tv_sec - tv_sec;
            auto remaining = this->vars.get<cfg::globals::close_timeout>()
                           - std::chrono::seconds{elapsed};
            this->close_widget.refresh_timeleft(remaining.count());
        });
}

CloseMod::~CloseMod()
{
    this->vars.set<cfg::context::close_box_extra_message>("");
}

void CloseMod::notify(Widget* sender, notify_event_t event)
{
    (void)sender;
    if (NOTIFY_CANCEL == event) {
        LOG(LOG_INFO, "CloseMod::notify Click on Close Button");
        this->set_mod_signal(BACK_EVENT_STOP);
    }
    else if (NOTIFY_SUBMIT == event) {
        LOG(LOG_INFO, "CloseMod::notify Click on Back to Selector Button");
        this->vars.ask<cfg::context::selector>();
        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::target_protocol>();
        this->set_mod_signal(BACK_EVENT_NEXT);
        // throw Error(ERR_BACK_EVENT_NEXT);
    }
}
