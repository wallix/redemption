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
 *   Copyright (C) Wallix 2010-2019
 *   Author(s): Meng Tan
 */


#include "mod/internal/transition_mod.hpp"
#include "mod/internal/widget/tooltip.hpp"
#include "keyboard/keymap2.hpp"
#include "configs/config.hpp"
#include "core/front_api.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"



void TransitionMod::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void TransitionMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
{
    if (device_flags & (MOUSE_FLAG_WHEEL | MOUSE_FLAG_HWHEEL)) {
        x = this->old_mouse_x;
        y = this->old_mouse_y;
    }
    else {
        this->old_mouse_x = x;
        this->old_mouse_y = y;
    }

    if (!this->rail_enabled) {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }
    else {
        bool out_mouse_captured = false;
        if (!this->rail_client_execute.input_mouse(device_flags, x, y, out_mouse_captured)) {
            switch (this->dc_state) {
                case DCState::Wait:
                    if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                        this->dc_state = DCState::FirstClickDown;

                        if (this->first_click_down_timer) {
                            this->first_click_down_timer->set_delay(std::chrono::seconds(1));
                        }
                        else {
                            this->first_click_down_timer = timer_events_
                            .create_timer_executor(this->session_reactor)
                            .set_delay(std::chrono::seconds(1))
                            .on_action(jln::one_shot([this]{
                                this->dc_state = DCState::Wait;
                            }));
                        }
                    }
                break;

                case DCState::FirstClickDown:
                    if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                        this->dc_state = DCState::FirstClickRelease;
                    }
                    else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    }
                    else {
                        this->cancel_double_click_detection();
                    }
                break;

                case DCState::FirstClickRelease:
                    if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                        this->dc_state = DCState::SecondClickDown;
                    }
                    else {
                        this->cancel_double_click_detection();
                    }
                break;

                case DCState::SecondClickDown:
                    if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                        this->dc_state = DCState::Wait;

                        bool out_mouse_captured_2 = false;

                        this->rail_client_execute.input_mouse(PTRFLAGS_EX_DOUBLE_CLICK, x, y, out_mouse_captured_2);

                        this->cancel_double_click_detection();
                    }
                    else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    }
                    else {
                        this->cancel_double_click_detection();
                    }
                break;

                default:
                    assert(false);

                    this->cancel_double_click_detection();
                break;
            }

            if (out_mouse_captured) {
                this->allow_mouse_pointer_change(false);

                this->current_mouse_owner = MouseOwner::ClientExecute;
            }
            else {
                if (MouseOwner::WidgetModule != this->current_mouse_owner) {
                    this->redo_mouse_pointer_change(x, y);
                }

                this->current_mouse_owner = MouseOwner::WidgetModule;
            }
        }

        this->screen.rdp_input_mouse(device_flags, x, y, keymap);

        if (out_mouse_captured) {
            this->allow_mouse_pointer_change(true);
        }
    }
}

void TransitionMod::refresh(Rect r)
{
    this->screen.refresh(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}


void TransitionMod::cancel_double_click_detection()
{
    assert(this->rail_enabled);

    this->first_click_down_timer.reset();

    this->dc_state = DCState::Wait;
}

bool TransitionMod::is_resizing_hosted_desktop_allowed() const
{
    assert(this->rail_enabled);

    return false;
}

TransitionMod::TransitionMod(
    TransitionModVariables vars,
    SessionReactor& session_reactor,
    TimerContainer& timer_events_,
    GraphicEventContainer& graphic_events_,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute, Font const& font,
    Theme const& theme
)
    : front_width(width)
    , front_height(height)
    , front(front)
    , screen(drawable, width, height, font, nullptr, theme)
    , rail_client_execute(rail_client_execute)
    , dvc_manager(false)
    , dc_state(DCState::Wait)
    , rail_enabled(rail_client_execute.is_rail_enabled())
    , current_mouse_owner(MouseOwner::WidgetModule)
    , session_reactor(session_reactor)
    , timer_events_(timer_events_)
    , graphic_events_(graphic_events_)
    , ttmessage(drawable, this->screen, nullptr,
                TR(trkeys::wait_msg, language(vars)),
                theme.tooltip.fgcolor, theme.tooltip.bgcolor,
                theme.tooltip.border_color, font)
    , vars(vars)
{
    this->screen.set_wh(width, height);
    if (this->rail_enabled) {
        this->graphic_event = graphic_events_.create_action_executor(session_reactor)
        .on_action(jln::one_shot([this](gdi::GraphicApi&){
            if (!this->rail_client_execute) {
                this->rail_client_execute.ready(
                    *this, this->front_width, this->front_height, this->font(),
                    this->is_resizing_hosted_desktop_allowed());

                this->dvc_manager.ready(this->front);
            }
        }));
    }

    Dimension dim;
    dim = this->ttmessage.get_optimal_dim();
    this->ttmessage.set_wh(dim);
    this->ttmessage.set_xy(widget_rect.x + (widget_rect.cx - dim.w) / 2,
                           widget_rect.y + (widget_rect.cy - dim.h) / 2);
    this->ttmessage.rdp_input_invalidate(this->ttmessage.get_rect());
    this->set_mod_signal(BACK_EVENT_NEXT);
}

TransitionMod::~TransitionMod()
{
    this->rail_client_execute.reset(true);
    this->screen.clear();
}


void TransitionMod::rdp_input_scancode(long int /*param1*/, long int /*param2*/,
                                       long int /*param3*/, long int /*param4*/,
                                       Keymap2* keymap)
{
    if (keymap->nb_kevent_available() > 0){
        switch (keymap->top_kevent()){
        case Keymap2::KEVENT_ESC:
            keymap->get_kevent();
            this->set_mod_signal(BACK_EVENT_STOP);
            break;
        default:;
        }
    }
}

void TransitionMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name, InStream& chunk,
    size_t length, uint32_t flags)
{
    LOG(LOG_INFO, "ClientChannelMod::send_to_mod_channel()");
    if (this->rail_enabled && this->rail_client_execute){
        if (front_channel_name == CHANNELS::channel_names::rail) {
            this->rail_client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
        else if (front_channel_name == CHANNELS::channel_names::drdynvc) {
            this->dvc_manager.send_to_mod_drdynvc_channel(length, chunk, flags);
        }
    }
}

