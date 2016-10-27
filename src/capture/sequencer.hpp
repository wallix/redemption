/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <ctime>
#include <cassert>

#include <chrono>
#include <utility>

#include "utils/difftimeval.hpp"

#include "gdi/capture_api.hpp"


template<class Action>
class SequencerCapture : public gdi::CaptureApi
{
    timeval start_break;
    std::chrono::microseconds break_interval;

protected:
    Action action;

public:
    template<class... ActionArgs>
    SequencerCapture(
        const timeval & now, std::chrono::microseconds break_interval,
        ActionArgs && ... action_args)
    : start_break(now)
    , break_interval(break_interval)
    , action(std::forward<ActionArgs>(action_args)...)
    {}

    std::chrono::microseconds get_interval() const {
        return this->break_interval;
    }

    void reset_now(const timeval& now) {
        this->start_break = now;
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        assert(this->break_interval.count());
        auto const interval = difftimeval(now, this->start_break);
        if (interval >= uint64_t(this->break_interval.count())) {
            this->action(now);
            this->start_break = now;
        }
        return this->break_interval;
    }
};
