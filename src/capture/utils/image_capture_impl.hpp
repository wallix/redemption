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

#ifndef REDEMPTION_UTILS_IMAGE_CAPTURE_IMPL_HPP
#define REDEMPTION_UTILS_IMAGE_CAPTURE_IMPL_HPP

#include "transport/out_filename_sequence_transport.hpp"

#include "staticcapture.hpp"
#include "utils/drawable.hpp"

#include "apis_register.hpp"


class ImageCaptureImpl
{
    OutFilenameSequenceTransport trans;
    StaticCapture sc;

public:
    ImageCaptureImpl(
        const timeval & now, bool clear_png, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const Inifile & ini)
    : trans(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        record_tmp_path, basename, ".png", groupid, authentifier)
    , sc(
        now, this->trans, this->trans.seqgen(),
        drawable.width(), drawable.height(),
        clear_png, ini, drawable)
    {}

    void attach_apis(ApisRegister & apis_register, const Inifile &) {
        apis_register.capture_list.push_back(this->sc);
        apis_register.graphic_snapshot_list->push_back(this->sc);
    }

    void zoom(unsigned percent) {
        this->sc.zoom(percent);
    }

    void next(const timeval & now) {
        this->sc.breakpoint(now);
    }
};

#endif
