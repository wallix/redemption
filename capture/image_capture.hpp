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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean
   
   This module know how to draw and image and flush it to some transport as a PNG file
   It called it image_capture as at some later time we may choose to make it work
   with some other target image format.
*/

#ifndef _REDEMPTION_CAPTURE_IMAGE_CAPTURE_HPP_
#define _REDEMPTION_CAPTURE_IMAGE_CAPTURE_HPP_

#include <stdio.h>
#include <png.h>

#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include "difftimeval.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "png.hpp"
#include "error.hpp"
#include "config.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "colors.hpp"

#include "RDP/RDPDrawable.hpp"

class ImageCapture
{
public:
    Transport & trans;
    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;
    Drawable & drawable;

    ImageCapture(Transport & trans, unsigned width, unsigned height, Drawable & drawable)
    : trans(trans)
    , zoom_factor(100)
    , scaled_width(width)
    , scaled_height(height)
    , drawable(drawable)
    {
    }

    ~ImageCapture()
    {
    }

    void zoom(unsigned percent)
    {
        const unsigned zoom_width = (this->drawable.width * percent) / 100;
        const unsigned zoom_height = (this->drawable.height * percent) / 100;
        TODO("we should limit percent to avoid images larger than 4096 x 4096")
        this->zoom_factor = percent;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
    }

    void update_config(const Inifile & ini){}

    virtual void flush()
    {
        if (this->zoom_factor == 100){
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

    TODO("move dump png24 to Drawable")
    void dump24(){
        ::transport_dump_png24(&this->trans, this->drawable.data,
                 this->drawable.width, this->drawable.height,
                 this->drawable.rowsize,
                 true
                );
    }

    void scale_dump24()
    {
        uint8_t * scaled_data = (uint8_t *)malloc(this->scaled_width * this->scaled_height * 3);
        scale_data(scaled_data, this->drawable.data,
                   this->scaled_width, this->drawable.width,
                   this->scaled_height, this->drawable.height,
                   this->drawable.rowsize);
        ::transport_dump_png24(&this->trans, scaled_data,
                     this->scaled_width, this->scaled_height,
                     this->scaled_width * 3,
                     true
                    );
        free(scaled_data);
    }

    TODO("move scale_data to Drawable")
    static void scale_data(uint8_t *dest, const uint8_t *src,
                            unsigned int dest_width, unsigned int src_width,
                            unsigned int dest_height, unsigned int src_height,
                            unsigned int src_rowsize)
    {
        const uint32_t Bpp = 3;
        unsigned int y_pixels = dest_height;
        unsigned int y_int_part = src_height / dest_height * src_rowsize;
        unsigned int y_fract_part = src_height % dest_height;
        unsigned int yE = 0;
        unsigned int x_int_part = src_width / dest_width * Bpp;
        unsigned int x_fract_part = src_width % dest_width;

        while (y_pixels-- > 0)
        {
            unsigned int xE = 0;
            const uint8_t * x_src = src;
            unsigned int x_pixels = dest_width;
            while (x_pixels-- > 0)
            {
                TODO("we can perform both scaling and rgb/bgr swapping at the same time")
                dest[0] = x_src[0];
                dest[1] = x_src[1];
                dest[2] = x_src[2];

                dest += Bpp;
                x_src += x_int_part;
                xE += x_fract_part;
                if (xE >= dest_width)
                {
                    xE -= dest_width;
                    x_src += Bpp;
                }
            }
            src += y_int_part;
            yE += y_fract_part;
            if (yE >= dest_height)
            {
                yE -= dest_height;
                src += src_rowsize;
            }
        }
    }
    
};

#endif
