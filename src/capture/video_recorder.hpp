/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#pragma once

#include "utils/image_view.hpp"

#include <memory>
#include <cstdint>

class AclReportApi;


class video_recorder
{
public:
    video_recorder(
        char const* filename, const int groupid, AclReportApi * acl_report,
        ImageView const& image_view, int frame_rate,
        const char * codec_name, char const* codec_options, int log_level
    );

    ~video_recorder();

    void preparing_video_frame();

    void encoding_video_frame(int64_t frame_index);

private:
    class D;
    std::unique_ptr<D> d;
};
