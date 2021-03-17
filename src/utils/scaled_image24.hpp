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

#include <memory>
#include <cstdint>


class Transport;
class ImageView;

struct ScaledPng24
{
    ScaledPng24(unsigned width, unsigned height);

    bool is_scaled() const noexcept { return this->scaled_width; }

    void dump_png24(Transport& trans, ImageView const& image_view, bool bgr) const;
    void dump_png24(char const * filename, ImageView const& image_view, bool bgr) const;

private:
    unsigned scaled_width;
    unsigned scaled_height;

    std::unique_ptr<uint8_t[]> scaled_buffer;
};

void scale_image24(
    uint8_t *dest, const uint8_t *src,
    unsigned int dest_width, unsigned int src_width,
    unsigned int dest_height, unsigned int src_height,
    unsigned int src_rowsize
);
