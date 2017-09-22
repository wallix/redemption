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
