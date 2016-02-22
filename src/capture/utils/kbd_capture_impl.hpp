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

#ifndef REDEMPTION_CAPTURE_UTILS_KBD_CAPTURE_IMPL_HPP
#define REDEMPTION_CAPTURE_UTILS_KBD_CAPTURE_IMPL_HPP

#include "config.hpp"
#include "apis_register.hpp"
#include "new_kbdcapture.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/input_kbd_api.hpp"
#include "gdi/capture_probe_api.hpp"

class KbdCaptureImpl
{
    auth_api * authentifier;
    KbdSyslogNotify sysog_notify;
    KbdSessionLogNotify session_log_notify;
    NewKbdCapture kc;

public:
    KbdCaptureImpl(const timeval & now, auth_api * authentifier, const Inifile & ini)
    : authentifier(authentifier)
    , session_log_notify(*authentifier)
    , kc(now, authentifier,
        ini.get<cfg::context::pattern_kill>().c_str(),
        ini.get<cfg::context::pattern_notify>().c_str(),
        ini.get<cfg::debug::capture>())
    {}

    void attach_apis(ApisRegister & api_register, const Inifile & ini) {
        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog)) {
            this->kc.attach_flusher(this->sysog_notify);
        }

        if (this->authentifier && ini.get<cfg::session_log::enable_session_log>()) {
            this->kc.attach_flusher(this->session_log_notify);
            api_register.capture_probe_list.push_back(this->session_log_notify);
        }

        if (this->kc.count_flusher()) {
            api_register.capture_list.push_back(this->kc);
        }

        if (this->kc.count_flusher() || this->kc.contains_pattern()) {
            api_register.input_kbd_list.push_back(this->kc);
        }

    }

    void enable_keyboard_input_mask(bool enable) {
        this->kc.enable_keyboard_input_mask(enable);
    }
};

#endif
