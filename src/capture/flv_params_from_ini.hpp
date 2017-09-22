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
*   Author(s): Christophe Grosjean
*/

#pragma once

#include "capture/flv_params.hpp"
#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"

#include "configs/config.hpp"


inline FlvParams flv_params_from_ini(uint16_t original_width, uint16_t original_height, const Inifile & ini)
{
    FlvParams flv_params = [&]() {
        switch (ini.get<cfg::globals::video_quality>()) {
            // TODO What to do if aspect ratio is not 4:3 ?
            case Level::low: return FlvParams{
                Level::low,
                ini.get<cfg::video::l_width>(),
                ini.get<cfg::video::l_height>(),
                ini.get<cfg::video::l_framerate>(),
                ini.get<cfg::video::l_qscale>(),
                ini.get<cfg::video::l_bitrate>(),
                ini.get<cfg::globals::codec_id>(),
                ini.get<cfg::globals::notimestamp>(),
                ini.get<cfg::globals::capture_chunk>(),
                ini.get<cfg::video::flv_break_interval>(),
                ini.get<cfg::debug::ffmpeg>()
            };
            case Level::high: return FlvParams{
                Level::high,
                original_width,
                original_height,
                ini.get<cfg::video::h_framerate>(),
                ini.get<cfg::video::h_qscale>(),
                ini.get<cfg::video::h_bitrate>(),
                ini.get<cfg::globals::codec_id>(),
                ini.get<cfg::globals::notimestamp>(),
                ini.get<cfg::globals::capture_chunk>(),
                ini.get<cfg::video::flv_break_interval>(),
                ini.get<cfg::debug::ffmpeg>()
            };
            case Level::medium:
            default: return FlvParams{
                Level::medium,
                ini.get<cfg::video::m_width>(),
                ini.get<cfg::video::m_height>(),
                ini.get<cfg::video::m_framerate>(),
                ini.get<cfg::video::m_qscale>(),
                ini.get<cfg::video::m_bitrate>(),
                ini.get<cfg::globals::codec_id>(),
                ini.get<cfg::globals::notimestamp>(),
                ini.get<cfg::globals::capture_chunk>(),
                ini.get<cfg::video::flv_break_interval>(),
                ini.get<cfg::debug::ffmpeg>()
            };
        }
    }();

    if (flv_params.target_width > original_width && flv_params.target_height > original_height) {
        flv_params.target_width = original_width;
        flv_params.target_height = original_height;
    }

    return flv_params;
}

inline OcrParams ocr_params_from_ini(Inifile & ini)
{
    return OcrParams{
        ini.get<cfg::ocr::version>(),
        ocr::locale::LocaleId(
            static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
        ini.get<cfg::ocr::on_title_bar_only>(),
        ini.get<cfg::ocr::max_unrecog_char_rate>(),
        ini.get<cfg::ocr::interval>(),
        ini.get<cfg::debug::ocr>()
    };
}

inline MetaParams meta_params_from_ini(Inifile & ini)
{
    return MetaParams{
        MetaParams::EnableSessionLog(ini.get<cfg::session_log::enable_session_log>()),
        MetaParams::HideNonPrintable(ini.get<cfg::session_log::hide_non_printable_kbd_input>())
    };
}

inline KbdLogParams kbd_log_params_from_ini(Inifile & ini)
{
    auto const disable_keyboard_log = ini.get<cfg::video::disable_keyboard_log>();
    return KbdLogParams{
        !bool(disable_keyboard_log & KeyboardLogFlags::wrm),
        !bool(disable_keyboard_log & KeyboardLogFlags::syslog),
        ini.get<cfg::session_log::enable_session_log>()
            && ini.get<cfg::session_log::keyboard_input_masking_level>()
            != ::KeyboardInputMaskingLevel::fully_masked,
        !bool(disable_keyboard_log & KeyboardLogFlags::meta)
    };
}

inline PatternParams pattern_params_from_ini(Inifile & ini)
{
    return PatternParams{
        ini.get<cfg::context::pattern_notify>().c_str(),
        ini.get<cfg::context::pattern_kill>().c_str(),
        ini.get<cfg::debug::capture>()
    };
}
