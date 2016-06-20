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

#include "configs/config.hpp"
#include "apis_register.hpp"
#include "new_kbdcapture.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"

class KbdCaptureImpl
{
    auth_api * authentifier;
    SyslogKbd syslog_kbd;
    SessionLogKbd session_log_kbd;
    PatternKbd pattern_kbd;

public:
    KbdCaptureImpl(const timeval & now, auth_api * authentifier, const Inifile & ini)
    : authentifier(authentifier)
    , syslog_kbd(now)
    , session_log_kbd(*authentifier)
    , pattern_kbd(authentifier,
        ini.get<cfg::context::pattern_kill>().c_str(),
        ini.get<cfg::context::pattern_notify>().c_str(),
        ini.get<cfg::debug::capture>())
    {}

    void attach_apis(ApisRegister & api_register, const Inifile & ini) {
        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)) {
            api_register.kbd_input_list.push_back(this->syslog_kbd);
            api_register.capture_list.push_back(this->syslog_kbd);
        }

        if (this->authentifier && ini.get<cfg::session_log::enable_session_log>() &&
            (ini.get<cfg::session_log::keyboard_input_masking_level>()
             != ::KeyboardInputMaskingLevel::fully_masked)
        ) {
            api_register.kbd_input_list.push_back(this->session_log_kbd);
            api_register.capture_probe_list.push_back(this->session_log_kbd);
        }

        if (this->pattern_kbd.contains_pattern()) {
            api_register.kbd_input_list.push_back(this->pattern_kbd);
        }
    }
};

