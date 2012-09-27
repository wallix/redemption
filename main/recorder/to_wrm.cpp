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

#include "to_wrm.hpp"
#include "timer_compute.hpp"
#include "wrm_recorder.hpp"
#include "capture.hpp"

void to_wrm(WRMRecorder& recorder, const char* outfile,
            std::size_t start, std::size_t stop, std::size_t interval,
            uint frame_limit,
            bool screenshot_start, bool screenshot_wrm,
            const char* metaname,
            CipherMode::enum_t mode,
            const unsigned char * key, const unsigned char * iv
)
{
    unsigned capture_flags = 15;
    unsigned png_interval = 100;
    unsigned png_limit = 60;
    Capture capture(recorder.meta().width,
                    recorder.meta().height,
                    outfile, metaname, "", "", capture_flags, png_limit, png_interval,
                    true,
                    mode, key, iv);
    recorder.consumer(&capture);

    uint frame = 0;

    uint64_t i = 0; // 20 images per second
    while (recorder.next(recorder.next_playback_timestamp)){}
}
