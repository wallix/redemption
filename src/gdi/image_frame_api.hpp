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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Raphael ZHOU
*/

#pragma once

namespace gdi {

struct ImageFrameApi {
    virtual ~ImageFrameApi() = default;

    virtual uint16_t width() const = 0;

    virtual uint16_t height() const = 0;

    virtual const uint8_t* data() const = 0;

    virtual uint8_t* first_pixel() = 0;

    virtual size_t rowsize() const = 0;

    virtual size_t pix_len() const = 0;

    virtual void prepare_image_frame() = 0;

    virtual unsigned int get_last_update_index() const = 0;
};

}
