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
#include <sys/time.h>
#include <time.h>
#include <png.h>

#include "rdtsc.hpp"
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
#include "scale.hpp"

class ImageCapture : public RDPDrawable
{
public:
    Transport & trans;

    ImageCapture(Transport & trans, unsigned width, unsigned height, bool bgr)
    : RDPDrawable(width, height, bgr)
    , trans(trans)
    {
    }

    ~ImageCapture()
    {
    }

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
};

#endif
