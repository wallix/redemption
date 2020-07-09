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
 *   Copyright (C) Wallix 2010-2020
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan
 *
 */

#pragma once

#include <functional>
#include "utils/timebase.hpp"
#include "core/events.hpp"
#include "core/RDP/slowpath.hpp"

enum { PTRFLAGS_EX_DOUBLE_CLICK = 0xFFFF };

struct MouseState {

    enum class DCState
    {
        Wait,
        FirstClickDown,
        FirstClickRelease,
        SecondClickDown,
    };

    int first_click_down_timer = 0;
    DCState dc_state = MouseState::DCState::Wait;
    TimeBase & time_base;
    EventContainer & events;
    std::function<bool(int device_flags, int x, int y, Keymap2 * keymap, bool & out_mouse_captured)> chained_input_mouse;

    MouseState(TimeBase & time_base, EventContainer& events)
        : time_base(time_base)
        , events(events)
    {
    }

    ~MouseState()
    {
        this->events.end_of_lifespan(this);
    }

    void cancel_double_click_detection()
    {
        //    assert(this->rail_enabled);
        this->first_click_down_timer = this->events.erase_event(this->first_click_down_timer);
        this->dc_state = MouseState::DCState::Wait;
    }

    bool input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        switch (this->dc_state) {
            case MouseState::DCState::Wait:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = MouseState::DCState::FirstClickDown;

                    if (this->first_click_down_timer) {
                        this->events.reset_timeout(this->time_base.get_current_time()+std::chrono::seconds{1},
                                      this->first_click_down_timer);
                    }
                    else {
                        Event dc_event("Mouse::DC Event", this);
                        this->first_click_down_timer = dc_event.id;
                        dc_event.alarm.set_timeout(
                                            this->time_base.get_current_time()
                                            +std::chrono::seconds{1});
                        dc_event.actions.on_timeout = [this](Event&)
                        {
                            this->dc_state = MouseState::DCState::Wait;
                        };
                        this->events.add(std::move(dc_event));
                    }
                }
            break;

            case MouseState::DCState::FirstClickDown:
                if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = MouseState::DCState::FirstClickRelease;
                }
                else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            case MouseState::DCState::FirstClickRelease:
                if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = MouseState::DCState::SecondClickDown;
                }
                else {
                    this->cancel_double_click_detection();
                }
            break;

            case MouseState::DCState::SecondClickDown:
                if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = MouseState::DCState::Wait;

                    bool out_mouse_captured_2 = false;

                    chained_input_mouse(PTRFLAGS_EX_DOUBLE_CLICK, x, y, keymap, out_mouse_captured_2);

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
        return false;
    }

};
