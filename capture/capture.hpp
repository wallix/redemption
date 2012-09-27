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

class Capture : public RDPGraphicDevice
{
    char log_prefix[256];
    unsigned & png_interval;

    struct timeval start_static_capture;
    uint64_t inter_frame_interval_static_capture(void)
    {
        return this->png_interval * 1000000;
    }

    struct timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    struct timeval start_break_capture;
    uint64_t inter_frame_interval_break_capture;

public:
    StaticCapture sc;
    NativeCapture nc;

    enum {
        STATIC_CAPTURE = 1,
        NATIVE_CAPTURE = 2
    };

    uint32_t enabled;

public:
    TODO(" fat interface : ugly  find another way")
    Capture(int width, 
            int height, 
            const char * path, 
            const char * path_meta, 
            const char * codec_id, 
            const char * video_quality, 
            unsigned capture_flags, 
            unsigned & png_interval, 
            unsigned & png_limit, 
            bool bgr = true, 
            CipherMode::enum_t mode = CipherMode::NO_MODE, 
            const unsigned char * key = 0, 
            const unsigned char * iv = 0) :
    png_interval(png_interval),
    sc(width, height, path, codec_id, video_quality, png_limit, width, height, bgr),
    nc(width, height, path, path_meta, mode, key, iv),
    enabled(capture_flags)
    {
        this->log_prefix[0] = 0;
        struct timeval now;
        gettimeofday(&now, NULL);
        this->start_static_capture = now;
        this->start_native_capture = now;
        this->start_break_capture = now;
         // 1 000 000 us is 1 sec (default)
        this->inter_frame_interval_native_capture =   40000; // 1 000 000 us is 1 sec (default)
        this->inter_frame_interval_break_capture  = 1000000 * 30; // 1 000 000 us is 1 sec (default)
    }

    ~Capture(){
    }

    void start(const timeval& now)
    {
        LOG(LOG_INFO, "start");
        if (this->enabled & NATIVE_CAPTURE){
            LOG(LOG_INFO, "send time start");
            this->nc.send_time_start_order(now);
            fprintf(this->nc.meta_file, "%s, %ld %ld\n", this->nc.filename, now.tv_sec, now.tv_usec);
        }
    }

    void start_with_invalid_now()
    {
        if (this->enabled & NATIVE_CAPTURE){
            struct timeval now = {0, 0};
            this->nc.send_time_start_order(now);
            fprintf(this->nc.meta_file, "%s, %ld %ld\n", this->nc.filename, now.tv_sec, now.tv_usec);
        }
    }

    void set_prefix(const char * prefix, size_t len_prefix)
    {
        size_t len = (len_prefix < sizeof(log_prefix))?len_prefix:(sizeof(log_prefix)-1);
        memcpy(this->log_prefix, prefix, len);
        this->log_prefix[len] = 0;
    }

    void timestamp(uint64_t usecond)
    {
        if (this->enabled & NATIVE_CAPTURE){
            this->nc.recorder.timestamp(usecond);
        }
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);

        if ((this->enabled & NATIVE_CAPTURE)
        && (difftimeval(now, this->start_native_capture) >= this->inter_frame_interval_native_capture)){
            this->nc.recorder.timestamp(now.tv_sec * 1000000ull + now.tv_usec);
            this->nc.recorder.flush();
            this->start_native_capture = now;
        }

        if ((this->enabled & STATIC_CAPTURE)
        && (difftimeval(now, this->start_static_capture) >= this->inter_frame_interval_static_capture())){
            TODO("change code below, it would be better to provide now to drawable instead of tm struct");
            time_t rawtime;
            time(&rawtime);
            tm *ptm = localtime(&rawtime);
            this->sc.drawable.trace_timestamp(*ptm);
            this->sc.flush();
            this->sc.drawable.clear_timestamp();
            this->start_static_capture = now;
         }
         
        if ((this->enabled & STATIC_CAPTURE) && (this->enabled & NATIVE_CAPTURE)
        && (difftimeval(now, this->start_break_capture) >= this->inter_frame_interval_break_capture)){
            LOG(LOG_INFO, "Breakpoint...");
            this->start_break_capture = now;
            this->nc.recorder.timestamp(now.tv_sec * 1000000ull + now.tv_usec); 
            this->start_native_capture = now;
            this->nc.breakpoint(this->sc.drawable.data,
                            24,
                            this->sc.drawable.width,
                            this->sc.drawable.height,
                            this->sc.drawable.rowsize,
                            now);
        }
    }

    void flush(){
    }

    TODO("use a template to factorize code below")
    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        if (this->enabled & STATIC_CAPTURE){ this->sc.draw(cmd, clip);}
        if (this->enabled & NATIVE_CAPTURE){ this->nc.draw(cmd, clip);}
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        if (this->enabled & STATIC_CAPTURE){ this->sc.draw(cmd, clip);}
        if (this->enabled & NATIVE_CAPTURE){ this->nc.draw(cmd, clip);}
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        if (this->enabled & STATIC_CAPTURE){ this->sc.draw(cmd, clip);}
        if (this->enabled & NATIVE_CAPTURE){ this->nc.draw(cmd, clip);}
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        if (this->enabled & STATIC_CAPTURE){ this->sc.draw(cmd, clip, bmp);}
        if (this->enabled & NATIVE_CAPTURE){ this->nc.draw(cmd, clip, bmp);}
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        if (this->enabled & STATIC_CAPTURE){ this->sc.draw(cmd, clip);}
        if (this->enabled & NATIVE_CAPTURE){ this->nc.draw(cmd, clip);}
    }

    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        if (this->enabled & STATIC_CAPTURE){ this->sc.draw(cmd, clip);}
        if (this->enabled & NATIVE_CAPTURE){ this->nc.draw(cmd, clip);}
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
//        if (this->enabled & STATIC_CAPTURE){ this->oc.draw(cmd, clip);}
//        if (this->enabled & NATIVE_CAPTURE){ this->oc.draw(cmd, clip);}
    }

};

#endif
