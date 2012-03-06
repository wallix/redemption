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

#if !defined(__NATIVECAPTURE_HPP__)
#define __NATIVECAPTURE_HPP__

#include <iostream>
#include <stdio.h>
#include "rdtsc.hpp"
#include <sstream>
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <time.h>
#include "difftimeval.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "error.hpp"
#include "config.hpp"
#include "bitmap_cache.hpp"
#include "colors.hpp"

#include "GraphicToFile.hpp"

class NativeCapture
{
    public:
    struct timeval start;
    uint64_t inter_frame_interval;
    int width;
    int height;
    int bpp;
    BGRPalette palette;
    int f;
    OutFileTransport trans;
    GraphicsToFile recorder;

    NativeCapture(int width, int height, int bpp, const BGRPalette & palette, BmpCache & bmpcache, char * path)
        : width(width), height(height), bpp(bpp),
        f(-1),
        trans(this->f),
        recorder(&this->trans, NULL) {
        char tmppath[1024] = {};
        sprintf(tmppath, "%s.%u.wrm", path, getpid());
        LOG(LOG_INFO, "Recording to file : %s", tmppath);
        this->f = open(tmppath, O_WRONLY|O_CREAT, 0666);
        if (this->f < 0){
            LOG(LOG_INFO, "Error opening native capture file : %s", strerror(errno));
            throw Error(ERR_RECORDER_NATIVE_CAPTURE_OPEN_FAILED);
        }

        this->trans.fd = this->f;
        this->inter_frame_interval = 40000; // 1 000 000 us is 1 sec (default)
    }

    ~NativeCapture(){
        close(this->f);
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(now, this->start) < this->inter_frame_interval){
            return;
        }
        this->start = now;
        this->recorder.timestamp();
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPBmpCache & cmd)
    {
        this->recorder.draw(cmd);
    }
    void draw(const RDPMemBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    void glyph_index(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

};

#endif
