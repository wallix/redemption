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

#if !defined(__STATICCAPTURE_HPP__)
#define __STATICCAPTURE_HPP__

#include <iostream>
#include <stdio.h>
#include "rdtsc.hpp"
#include <sstream>
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <sys/time.h>
#include <time.h>
#include <png.h>
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
#include "bmpcache.hpp"
#include "colors.hpp"

#include "RDP/RDPDrawable.hpp"


class StaticCapture : public RDPDrawable
{
    int framenb;
    uint64_t inter_frame_interval;

    struct timeval start;

    public:
    BGRPalette palette;
    char path[1024];

    StaticCapture(int width, int height, int bpp, const BGRPalette & palette, char * path, const char * codec_id, const char * video_quality)
        : RDPDrawable(width, height, bpp, palette, true),
          framenb(0)
    {
        gettimeofday(&this->start, NULL);
        this->inter_frame_interval = 1000000; // 1 000 000 us is 1 sec (default)
        strcpy(this->path, path);
    }

    ~StaticCapture(){
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(now, this->start) < this->inter_frame_interval){
            return;
        }
        this->dump_png();
    }

    void dump_png(void){
        char rawImagePath[256]     = {0};
        char rawImageMetaPath[256] = {0};
        snprintf(rawImagePath,     254, "%s.%u.%u.png", this->path, getpid(), this->framenb++);
        snprintf(rawImageMetaPath, 254, "%s.meta", rawImagePath);
//        LOG(LOG_INFO, "Dumping to file %s", rawImagePath);
        FILE * fd = fopen(rawImageMetaPath, "w");
        if (fd) {
           fprintf(fd, "%d,%d\n", this->drawable.width, this->drawable.height);
        }
        fclose(fd);
        fd = fopen(rawImagePath, "w");
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
