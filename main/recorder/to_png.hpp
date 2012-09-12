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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__MAIN_RECORDER_TO_PNG__)
#define __MAIN_RECORDER_TO_PNG__

#include <vector>
#include "relative_time_point.hpp"
#include "wrm_recorder.hpp"

void to_png(WRMRecorder& recorder, const char* outfile,
            std::size_t start, std::size_t stop, std::size_t interval,
            unsigned resize_width = -1, unsigned resize_height = -1,
            uint frame_limit = -1,
            bool screenshot_start = true, bool no_screenshot_stop = false,
            bool screenshot_all = false);

void to_png(WRMRecorder& recorder, const char* outfile,
            const std::vector<relative_time_point>& capture_points,
            unsigned resize_width = -1, unsigned resize_height = -1,
            bool no_screenshot_stop = false);

#endif