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

struct StaticCaptureConfig {
    unsigned png_limit;
    unsigned png_interval;
    char path[1024];
    bool bgr;

    StaticCaptureConfig()
    : png_limit(10000)
    {
    }
};

struct StaticCaptureState {
    unsigned framenb;
    unsigned to_remove[32768];
    char image_path[1024];
    uint16_t image_basepath_len;
    
    StaticCaptureState()
    : framenb(0)
    {
    
    }
};

class StaticCapture : public RDPDrawable
{
public:
    StaticCaptureConfig conf;
    StaticCaptureState state;

private:

public:
    StaticCapture(unsigned width, unsigned height, const char * path, bool bgr)
    : RDPDrawable(width, height, RDPDrawableConfig(bgr))
    {
        strcpy(this->conf.path, path);        
        this->conf.bgr = bgr;
        this->state.image_basepath_len = sprintf(this->state.image_path, "%s-%u-", conf.path, getpid());
    }

    ~StaticCapture()
    {
    }

    void update_config(const Inifile & ini){
        if (ini.globals.png_limit != this->conf.png_limit){
            if (ini.globals.png_limit < this->conf.png_limit){
                TODO("remove old images if there is too many of them")
            }
            this->conf.png_limit = ini.globals.png_limit;
        }
    }

    virtual void flush()
    {
        this->dump_png();
    }

    void dump_png(void)
    {
        if (this->conf.png_limit > 0){
            if (this->conf.png_limit < (sizeof(this->state.to_remove)/sizeof(unsigned)) && this->state.framenb >= this->conf.png_limit){
                sprintf(this->state.image_path + this->state.image_basepath_len, "%u.png", 
                    this->state.to_remove[(this->state.framenb - this->conf.png_limit) % this->conf.png_limit]);           
                LOG(LOG_INFO, "Removing file %s framenb=%u limit=%u", 
                    this->state.image_path, this->state.framenb, this->conf.png_limit);
                unlink(this->state.image_path);
            }

            sprintf(this->state.image_path + this->state.image_basepath_len, "%u.png", this->state.framenb++);
            LOG(LOG_INFO, "Dumping to file %s %ux%u framenb=%u limit=%u", 
                this->state.image_path, this->drawable.width, this->drawable.height, 
                this->state.framenb, this->conf.png_limit);
            if (FILE * fd = fopen(this->state.image_path, "w")){
//                    scale_data(this->data_scale, this->drawable.data,
//                               this->scale_width, this->drawable.width,
//                               this->scale_height, this->drawable.height,
//                               this->drawable.rowsize);
//                               
//                    ::dump_png24(fd, this->data_scale,
//                                 this->scale_width,
//                                 this->scale_height,
//                                 this->scale_width * 3
//                                );
                LOG(LOG_INFO, "Dump png %ux%u", this->drawable.width, this->drawable.height,
                             this->drawable.rowsize);

                ::dump_png24(fd, this->drawable.data,
                             this->drawable.width, this->drawable.height,
                             this->drawable.rowsize
                            );
                fclose(fd);
            }
            if (this->conf.png_limit < (sizeof(this->state.to_remove)/sizeof(unsigned))){
                this->state.to_remove[(this->state.framenb-1) % this->conf.png_limit] = this->state.framenb-1;
            }
        }
    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
    }

};

#endif
