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

#if !defined(__CAPTURE_CAPTURE_HPP__)
#define __CAPTURE_CAPTURE_HPP__

#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "meta_wrm.hpp"

class Capture : public RDPGraphicDevice
{
    struct timeval start_static_capture;
    uint64_t inter_frame_interval_static_capture;

    struct timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    StaticCapture sc;
    NativeCapture nc;

    public:

    TODO(" fat interface : ugly  find another way")
    Capture(int width, int height, const char * path, const char * codec_id, const char * video_quality) :
        sc(width, height, path, codec_id, video_quality),
        nc(width, height, path)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        this->start_static_capture = now;
        this->start_native_capture = now;
        this->inter_frame_interval_static_capture = 5000000; // 1 000 000 us is 1 sec (default)
        this->inter_frame_interval_native_capture =   40000; // 1 000 000 us is 1 sec (default)
    }

    ~Capture(){
    }

    void timestamp()
    {
        this->nc.recorder.timestamp();
    }

    void emit_meta(MetaWRM& meta)
    {
        meta.send(this->nc.recorder);
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(now, this->start_static_capture) >= this->inter_frame_interval_static_capture){
            TODO("change code below, it would be better to provide now to drawable instead of tm struct");
            time_t rawtime;
            time(&rawtime);
            tm *ptm = localtime(&rawtime);
            this->sc.drawable.trace_timestamp(*ptm);
            this->sc.flush();
            this->sc.drawable.clear_timestamp();
            this->start_static_capture = now;
        }
        if (difftimeval(now, this->start_native_capture) >= this->inter_frame_interval_native_capture){
            this->nc.recorder.timestamp(now);
            this->start_native_capture = now;
        }
        this->nc.recorder.flush();
    }

    void flush(){}

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->sc.draw(cmd, clip);
        this->nc.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->sc.draw(cmd, clip);
        this->nc.draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->sc.draw(cmd, clip);
        this->nc.draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->sc.draw(cmd, clip, bmp);
        this->nc.draw(cmd, clip, bmp);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->sc.draw(cmd, clip);
        this->nc.draw(cmd, clip);
    }


    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->sc.draw(cmd, clip);
        this->nc.draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
//        this->sc.glyph_index(cmd, clip);
//        this->nc.glyph_index(cmd, clip);
    }

    void breakpoint()
    {
        this->nc.breakpoint(this->sc.drawable.data, this->sc.drawable.pix_len);
    }

};

#endif
