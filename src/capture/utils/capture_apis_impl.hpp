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

#ifndef REDEMPTION_UTILS_CAPTURE_APIS_IMPL_HPP
#define REDEMPTION_UTILS_CAPTURE_APIS_IMPL_HPP

#include <sys/time.h> // timeval

#include <vector>
#include <functional> // reference_wrapper

#include "utils/drawable.hpp"
#include "core/wait_obj.hpp"

#include "gdi/capture_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/input_pointer_api.hpp"
#include "gdi/kbd_input_api.hpp"


struct CaptureApisImpl
{
    struct Capture : gdi::CaptureDispatcher<Capture>
    {
        Capture(const timeval & now, int cursor_x, int cursor_y)
        : mouse_info{now, cursor_x, cursor_y}
        , capture_event{}
        {}

        std::chrono::microseconds snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) override {
            this->capture_event.reset();

            if (this->drawable) {
                this->drawable->set_mouse_cursor_pos(cursor_x, cursor_y);
            }

            this->mouse_info = {now, cursor_x, cursor_y};

            std::chrono::microseconds time = std::chrono::microseconds::max();
            if (!this->caps.empty()) {
                time = Capture::base_type::snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
                this->capture_event.update(time.count());
            }
            return time;
        }

        void pause_capture(const timeval& now) override {
            Capture::base_type::pause_capture(now);
            this->capture_event.reset();
        }

        void resume_capture(const timeval& now) override {
            Capture::base_type::resume_capture(now);
            this->capture_event.set();
        }

        void set_drawable(Drawable * drawable) {
            this->drawable = drawable;
        }

        MouseTrace const & mouse_trace() const noexcept {
            return this->mouse_info;
        }

        wait_obj & get_capture_event() {
            return this->capture_event;
        }

        using captures_list = std::vector<std::reference_wrapper<gdi::CaptureApi>>;

        captures_list caps;

    private:
        friend gdi::CaptureCoreAccess;
        captures_list & get_capture_list_impl() {
            return this->caps;
        }

        Drawable * drawable;
        MouseTrace mouse_info;
        wait_obj capture_event;
    };


    struct KbdInput : gdi::KbdInputApi
    {
        bool kbd_input(const timeval & now, Keys const & k) override {
            bool ret = true;
            for (gdi::KbdInputApi & kbd : this->kbds) {
                ret &= kbd.kbd_input(now, k);
            }
            return ret;
        }

        void enable_kbd_input_mask(bool enable) override {
            for (gdi::KbdInputApi & kbd : this->kbds) {
                kbd.enable_kbd_input_mask(enable);
            }
        }

        std::vector<std::reference_wrapper<gdi::KbdInputApi>> kbds;
    };


    struct InputPointer : gdi::InputPointer
    {
        void update_pointer_position(uint16_t x, uint16_t y) override {
            for (gdi::InputPointer & mouse : this->mouses) {
                mouse.update_pointer_position(x, y);
            }
        }

        std::vector<std::reference_wrapper<gdi::InputPointer>> mouses;
    };


    struct CaptureProbe : gdi::CaptureProbeApi
    {
        void possible_active_window_change() override {
            for (gdi::CaptureProbeApi & cap_prob : this->probes) {
                cap_prob.possible_active_window_change();
            }
        }

        void session_update(const timeval& now, array_const_char const & message) override {
            for (gdi::CaptureProbeApi & cap_prob : this->probes) {
                cap_prob.session_update(now, message);
            }
        }

        std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> probes;
    };
};

#endif
