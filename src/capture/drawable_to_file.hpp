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
*   Copyright (C) Wallix 2012-2016
*   Author(s): Christophe Grosjean, Jonathan Poelen
*
*   This module know how to draw and image and flush it to some transport as a PNG file
*   It called it image_capture as at some later time we may choose to make it work
*   with some other target image format.
*/

#ifndef REDEMPTION_DRAWABLE_TO_FILE_HPP
#define REDEMPTION_DRAWABLE_TO_FILE_HPP

#include "png.hpp"
#include "drawable.hpp"

#include <memory>

class DrawableToFile
{
// TODO
protected:
    Transport & trans;
    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

// TODO
protected:
    const Drawable & drawable;

private:
    std::unique_ptr<uint8_t[]> scaled_buffer;

public:
    DrawableToFile(Transport & trans, const Drawable & drawable)
    : trans(trans)
    , zoom_factor(100)
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    {}

    ~DrawableToFile() = default;

    /// \param  percent  0 to 100 or 100 if greater
    void zoom(unsigned percent) {
        percent = std::min(percent, 100u);
        const unsigned zoom_width = (this->drawable.width() * percent) / 100;
        const unsigned zoom_height = (this->drawable.height() * percent) / 100;
        this->zoom_factor = percent;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    bool logical_frame_ended() const {
        return this->drawable.logical_frame_ended;
    }

    void flush() {
        if (this->zoom_factor == 100) {
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

private:
    void dump24() const {
        ::transport_dump_png24(
            this->trans, this->drawable.data(),
            this->drawable.width(), this->drawable.height(),
            this->drawable.rowsize(), true);
    }

    void scale_dump24() const {
        scale_data(
            this->scaled_buffer.get(), this->drawable.data(),
            this->scaled_width, this->drawable.width(),
            this->scaled_height, this->drawable.height(),
            this->drawable.rowsize());
        ::transport_dump_png24(
            this->trans, this->scaled_buffer.get(),
            this->scaled_width, this->scaled_height,
            this->scaled_width * 3, false);
    }

    static void scale_data(uint8_t *dest, const uint8_t *src,
                           unsigned int dest_width, unsigned int src_width,
                           unsigned int dest_height, unsigned int src_height,
                           unsigned int src_rowsize) {
        const uint32_t Bpp = 3;
        unsigned int y_pixels = dest_height;
        unsigned int y_int_part = src_height / dest_height * src_rowsize;
        unsigned int y_fract_part = src_height % dest_height;
        unsigned int yE = 0;
        unsigned int x_int_part = src_width / dest_width * Bpp;
        unsigned int x_fract_part = src_width % dest_width;

        while (y_pixels-- > 0) {
            unsigned int xE = 0;
            const uint8_t * x_src = src;
            unsigned int x_pixels = dest_width;
            while (x_pixels-- > 0) {
                dest[0] = x_src[2];
                dest[1] = x_src[1];
                dest[2] = x_src[0];

                dest += Bpp;
                x_src += x_int_part;
                xE += x_fract_part;
                if (xE >= dest_width) {
                    xE -= dest_width;
                    x_src += Bpp;
                }
            }
            src += y_int_part;
            yE += y_fract_part;
            if (yE >= dest_height) {
                yE -= dest_height;
                src += src_rowsize;
            }
        }
    }
};

#endif
