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

#if !defined(__CAPTURE_IMAGE_CAPTURE_HPP__)
#define __CAPTURE_IMAGE_CAPTURE_HPP__

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

class ImageCapture : public RDPDrawable
{
public:
    Transport & trans;

    TODO("RDPDrawable should be provided to Image capture, not instanciated here")

    ImageCapture(Transport & trans, unsigned width, unsigned height)
    : RDPDrawable(width, height, true)
    , trans(trans)
    {
    }

    ~ImageCapture()
    {
    }

    void update_config(const Inifile & ini){}

    virtual void flush()
    {
//                    scale_data(this->data_scale, this->drawable.data,
//                               this->scale_width, this->drawable.width,
//                               this->scale_height, this->drawable.height,
//                               this->drawable.rowsize);
        ::transport_dump_png24(&this->trans, this->drawable.data,
                     this->drawable.width, this->drawable.height,
                     this->drawable.rowsize
                    );
    }

    REDOC("Rescale image :\n"
          "width size is forced to the nearest larger multiple of 4, to be an exact multiple of 32 bits\n"
          "width and height must both be smaller than 4096\n")
    void scale_dump(unsigned scale_width, unsigned scale_height)
    {
        unsigned fixed_scaled_width = (scale_width >= 4096)?4096:(scale_width + 3) & 0xFFC0;
        unsigned fixed_scaled_height = (scale_height >= 4096)?4096:scale_height;
        uint8_t * scaled_data = (uint8_t *)malloc(fixed_scaled_width * fixed_scaled_height * 3);
        scale_data(scaled_data, this->drawable.data,
                   fixed_scaled_width, this->drawable.width,
                   fixed_scaled_height, this->drawable.height,
                   this->drawable.rowsize);
        ::transport_dump_png24(&this->trans, scaled_data,
                     fixed_scaled_width, fixed_scaled_height,
                     fixed_scaled_width * 3
                    );
        free(scaled_data);
    }

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
