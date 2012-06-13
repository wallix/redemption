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

#include <iostream>
#include <stdio.h>
#include <sstream>
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


class StaticCapture : public RDPDrawable
{
public:
    int framenb;
    char path[1024];
    char image_path[1024];
    uint16_t image_basepath_len;

public:
    StaticCapture(int width, int height, const char * path, const char * codec_id, const char * video_quality, bool bgr = true)
        : RDPDrawable(width, height, bgr),
          framenb(0)
    {
        strcpy(this->path, path);
        this->image_basepath_len = sprintf(this->image_path, "%s-%u-", path, getpid());
    }

    ~StaticCapture(){
    }

    virtual void flush()
    {
        this->dump_png();
    }

    void dump_png(void){
        char rawImageMetaPath[256] = {0};
        sprintf(this->image_path + this->image_basepath_len, "%u.png", this->framenb++);
        snprintf(rawImageMetaPath, 254, "%s.meta", this->image_path);
//        LOG(LOG_INFO, "Dumping to file %s", rawImagePath);
        FILE * fd = fopen(rawImageMetaPath, "w");
        if (fd) {
           fprintf(fd, "%d,%d\n", this->drawable.width, this->drawable.height);
        }
        fclose(fd);
        fd = fopen(this->image_path, "w");
        if (fd) {
            ::dump_png24(fd, this->drawable.data, this->drawable.width, this->drawable.height, this->drawable.rowsize);
        }
        fclose(fd);
    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
       return;
    }

};

#endif
