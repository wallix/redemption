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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__CAPTURE_STATICCAPTURE_HPP__)
#define __CAPTURE_STATICCAPTURE_HPP__

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
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "png.hpp"
#include "error.hpp"
#include "config.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "colors.hpp"

#include "RDP/RDPDrawable.hpp"
#include "scale.hpp"

class StaticCapture : public RDPDrawable
{
public:
    int framenb;
    char path[1024];
    char image_path[1024];
    uint16_t image_basepath_len;

private:
    unsigned int scale_width;
    unsigned int scale_height;
    uint8_t * data_scale;

    void __init(const char * path)
    {
        strcpy(this->path, path);
        this->image_basepath_len = sprintf(this->image_path, "%s-%u-", path, getpid());
    }

public:
    StaticCapture(int width, int height, const char * path, const char * codec_id, const char * video_quality, bool bgr = true)
    : RDPDrawable(width, height, bgr)
    , framenb(0)
    , scale_width(width)
    , scale_height(height)
    , data_scale(0)
    {
        this->__init(path);
    }

    StaticCapture(int width, int height, const char * path, bool bgr = true)
    : RDPDrawable(width, height, bgr)
    , framenb(0)
    , scale_width(width)
    , scale_height(height)
    , data_scale(0)
    {
        this->__init(path);
    }

    StaticCapture(int width, int height, const char * path, unsigned resize_width, unsigned resize_height, bool bgr = true)
    : RDPDrawable(width, height, bgr)
    , framenb(0)
    , scale_width(std::min<unsigned>(width, resize_width))
    , scale_height(std::min<unsigned>(height, resize_height))
    , data_scale(
        (scale_width == this->drawable.width
        && scale_height == this->drawable.height)
        || 0 == scale_width * scale_height
        ? 0 : new uint8_t[scale_width * scale_height * 3]
    )
    {
        this->__init(path);
    }

    ~StaticCapture()
    {
        delete [] this->data_scale;
    }

    unsigned get_resize_width() const
    {
        return this->scale_width;
    }

    unsigned get_resize_height() const
    {
        return this->scale_height;
    }

    void set_resize(unsigned resize_width, unsigned resize_height)
    {
        resize_width = std::min<unsigned>(this->drawable.height, resize_width);
        resize_height = std::min<unsigned>(this->drawable.height, resize_height);
        if (resize_width == this->drawable.width
            && resize_height == this->drawable.height)
        {
            delete this->data_scale;
            this->data_scale = 0;
        }
        else
        {
            if (resize_width * resize_height > this->scale_width * this->scale_height)
            {
                delete [] this->data_scale;
                this->data_scale = new uint8_t[resize_width * resize_height];
            }
            this->scale_width = resize_width;
            this->scale_height = resize_height;
        }
    }

    virtual void flush()
    {
        this->dump_png();
    }

    void dump_png(void)
    {
        sprintf(this->image_path + this->image_basepath_len, "%u.png", this->framenb++);
//        LOG(LOG_INFO, "Dumping to file %s", rawImagePath);
        if (FILE * fd = fopen(this->image_path, "w"))
        {
            if (this->data_scale)
            {
                scale_data(this->data_scale, this->drawable.data,
                           this->scale_width, this->drawable.width,
                           this->scale_height, this->drawable.height,
                           this->drawable.rowsize);
                ::dump_png24(fd, this->data_scale,
                             this->scale_width,
                             this->scale_height,
                             this->scale_width * 3
                            );
            }
            else
            {
                ::dump_png24(fd, this->drawable.data,
                             this->drawable.width, this->drawable.height,
                             this->drawable.rowsize
                            );
            }
            fclose(fd);
        }
    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
    }

};

#endif
