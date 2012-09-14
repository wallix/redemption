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

#if !defined(__MAIN_RECORDER_LOAD_PNG_CONTEXT_HPP__)
#define __MAIN_RECORDER_LOAD_PNG_CONTEXT_HPP__

#include "wrm_recorder.hpp"

inline void load_png_context(WRMRecorder& recorder, Drawable& drawable)
{
    if (recorder.idx_file > 1 &&
        recorder.meta().files.size() >= recorder.idx_file &&
        !recorder.meta().files[recorder.idx_file - 1].png_filename.empty())
    {
        recorder.redraw_consumer(&drawable);
        recorder.load_context(
            recorder.meta()
            .files[recorder.idx_file - 1]
            .png_filename.c_str()
        );
        recorder.redraw_consumer(0);
    }
}

#endif