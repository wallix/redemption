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
} // namespace

FlatWabCloseMod::FlatWabCloseMod(
    FlatWabCloseModVariables vars, SessionReactor& session_reactor,
    FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, time_t /*now*/, ClientExecute & client_execute,
    Font const& font, Theme const& theme, bool showtimer, bool back_selector
)
    : LocallyIntegrableMod(session_reactor, front, width, height, font, client_execute, theme)
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
        font, theme, language(vars), back_selector)
    , vars(vars)
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

    if (vars.get<cfg::globals::close_timeout>().count()) {
        std::chrono::seconds delay{1};
        std::chrono::seconds start_timer{};
        if (!showtimer) {
            delay = vars.get<cfg::globals::close_timeout>();
            start_timer = delay;
        }
        this->timeout_timer = session_reactor.create_timer(start_timer)
        .set_delay(delay)
        .on_action([this](JLN_TIMER_CTX ctx, std::chrono::seconds& seconds){
            // TODO milliseconds += ctx.time() - previous_time
            ++seconds;
            auto const close_timeout = this->vars.get<cfg::globals::close_timeout>();
            if (seconds < close_timeout) {
                this->close_widget.refresh_timeleft((close_timeout - seconds).count());
                return ctx.ready_to(std::min(std::chrono::seconds{1}, close_timeout));
            }
            ctx.get_reactor().set_event_next(BACK_EVENT_STOP);
            return ctx.terminate();
        });
    }
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
        this->session_reactor.set_next_event(BACK_EVENT_STOP);
    }
    else if (NOTIFY_SUBMIT == event) {
        LOG(LOG_INFO, "asking for selector");
        this->vars.ask<cfg::context::selector>();
        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::target_protocol>();
        this->session_reactor.set_next_event(BACK_EVENT_NEXT);
    }
}

void FlatWabCloseMod::draw_event(time_t /*now*/, gdi::GraphicApi& /*unused*/) {}
