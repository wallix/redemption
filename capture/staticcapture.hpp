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

#include "image_capture.hpp"

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
#include "config.hpp"

struct StaticCaptureConfig {
    unsigned png_limit;
    uint64_t png_interval;
    char path[1024];
    bool bgr;

    StaticCaptureConfig()
    : png_limit(3)
    {
    }
};

class StaticCapture : public ImageCapture
{
public:
    FileSequence & sequence;
    StaticCaptureConfig conf;

    struct timeval start_static_capture;
    uint64_t inter_frame_interval_static_capture;

    StaticCapture(const timeval & now, Transport & trans, FileSequence & sequence, unsigned width, unsigned height, const Inifile & ini)
    : ImageCapture(trans, width, height)
    , sequence(sequence)
    {
        this->start_static_capture = now;
        this->conf.png_interval = 3000; // png interval is in 1/10 s, default value, 1 static snapshot every 5 minutes
        this->inter_frame_interval_static_capture       = this->conf.png_interval * 100000; // 1 000 000 us is 1 sec
        this->update_config(ini);
    }

    ~StaticCapture()
    {
    }

    void update_config(const Inifile & ini){
        if (ini.globals.png_limit < this->conf.png_limit){
            for(size_t i = this->conf.png_limit ; i > ini.globals.png_limit ; i--){
                char path[1024];
                if (this->trans.seqno >= i){
                    this->sequence.get_name(path, sizeof(path), this->trans.seqno - i );
                    ::unlink(path);  // unlink may fail, for instance if file does not exist, just don't care
                }
            }
        }
        this->conf.png_limit = ini.globals.png_limit;

        if (ini.globals.png_interval != this->conf.png_interval){
            // png interval is in 1/10 s, default value, 1 static snapshot every 5 minutes
            this->conf.png_interval = ini.globals.png_interval;
            this->inter_frame_interval_static_capture = this->conf.png_interval * 100000; // 1 000 000 us is 1 sec
        }
    }

    virtual void snapshot(const timeval & now, int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        if ((unsigned)difftimeval(now, this->start_static_capture) 
         >= (unsigned)this->inter_frame_interval_static_capture){
            this->breakpoint(now);
        }
    
    }

    void breakpoint(const timeval & now)
    {
        time_t rawtime = now.tv_sec;
        tm *ptm = localtime(&rawtime);
        this->drawable.trace_timestamp(*ptm);

        if (this->conf.png_limit > 0){
            if (this->trans.seqno >= this->conf.png_limit){
                char path[1024];
                this->sequence.get_name(path, sizeof(path), this->trans.seqno - this->conf.png_limit);
                ::unlink(path); // unlink may fail, for instance if file does not exist, just don't care
            }
            this->ImageCapture::flush();
            this->trans.next();
        }

        this->drawable.clear_timestamp();
        this->start_static_capture = now;
    }

    virtual void flush()
    {
    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
    }

};

#endif
