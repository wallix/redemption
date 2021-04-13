/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen
*/

#pragma once

#include <memory>

class RdpPointerView;

namespace redclient
{
    struct RGBA8888Image
    {
        unsigned width = 0;
        unsigned height = 0;
        std::unique_ptr<uint8_t[]> buffer;

        unsigned bytes_per_line() const { return this->width * 4u; }
        uint8_t* data() const { return this->buffer.get(); }
    };

    RGBA8888Image pointer_to_rgba8888(RdpPointerView const& pointer);
}
