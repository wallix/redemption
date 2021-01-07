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

#include "utils/timebase.hpp"
#include "core/events.hpp"
#include "core/RDP/slowpath.hpp"

constexpr uint16_t PTRFLAGS_EX_DOUBLE_CLICK = 0xFFFFu;

class MouseState
{
    enum class DCState : unsigned char
    {
        Wait,
        FirstClickDown,
        FirstClickRelease,
        SecondClickDown,
    };

    EventRef2 first_click_down_timer;
    DCState dc_state = MouseState::DCState::Wait;

public:
    MouseState(EventContainer& events)
    : first_click_down_timer(events)
    {
    }

    [[nodiscard]]
    bool next_event_is_double_click(uint16_t flags)
    {
        switch (this->dc_state) {
            case MouseState::DCState::Wait:
                if (flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = MouseState::DCState::FirstClickDown;
                    this->first_click_down_timer.reset_timeout_or_create_event(
                        1s, "Mouse::DC Event",
                        [this](Event&) {
                            this->dc_state = MouseState::DCState::Wait;
                        }
                    );
                }
            break;

            case MouseState::DCState::FirstClickDown:
                if (flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = MouseState::DCState::FirstClickRelease;
                }
                else if (flags != (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = MouseState::DCState::Wait;
                    this->first_click_down_timer.garbage();
                }
            break;

            case MouseState::DCState::FirstClickRelease:
                if (flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = MouseState::DCState::SecondClickDown;
                }
                else {
                    this->dc_state = MouseState::DCState::Wait;
                    this->first_click_down_timer.garbage();
                }
            break;

            case MouseState::DCState::SecondClickDown:
                if (flags == SlowPath::PTRFLAGS_BUTTON1) {
                    this->dc_state = MouseState::DCState::Wait;
                    this->first_click_down_timer.garbage();
                    return true;
                }
                else if (flags != (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    this->dc_state = MouseState::DCState::Wait;
                    this->first_click_down_timer.garbage();
                }
            break;
        }

        return false;
    }
};
