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

    uint64_t png_interval;
    struct timeval start_static_capture;
    uint64_t inter_frame_interval_static_capture;

    uint64_t frame_interval;
    struct timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    struct timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    StaticCapture sc;
    NativeCapture nc;

private:
    void _init(const timeval & now)
    {
        this->log_prefix[0] = 0;
        this->start_static_capture = now;
        this->png_interval = 3000; // png interval is in 1/10 s, default value, 1 static snapshot every 5 minutes
        this->inter_frame_interval_static_capture       = this->png_interval * 100000; // 1 000 000 us is 1 sec

        // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
        this->start_native_capture = now;
        this->frame_interval = 40;
        this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec

        this->start_break_capture = now;
        this->break_interval = 60 * 10; // break interval is in s, default value 1 break every 10 minutes
        this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec

        LOG(LOG_INFO, "update configuration png_interval=%u frame_interval=%u break_interval=%u",
            this->png_interval, this->frame_interval, this->break_interval);
    }

public:
    TODO(" fat interface : ugly  find another way")
    Capture(const timeval & now, int width, int height, const char * path, const char * codec_id, const char * video_quality, bool bgr = true) :
        sc(width, height, path, width, height, bgr),
        nc(width, height, path)
    {
        this->_init(now);
    }

    Capture(const timeval & now, int width, int height, const char * path, const char * path_meta, const char * codec_id, const char * video_quality, bool bgr, CipherMode::enum_t mode = CipherMode::NO_MODE, const unsigned char * key = 0, const unsigned char * iv = 0) :
    sc(width, height, path, width, height, bgr),
    nc(width, height, path, path_meta, mode, key, iv)
    {
        this->_init(now);
    }

    void update_config(const timeval & now, const Inifile & ini){
        this->sc.update_config(ini);
        if (ini.globals.png_interval != this->png_interval){
            this->start_static_capture = now;
            this->png_interval = ini.globals.png_interval; // png interval is in 1/10 s, default value, 1 static snapshot every 5 minutes
            this->inter_frame_interval_static_capture       = this->png_interval * 100000; // 1 000 000 us is 1 sec
        }

        if (ini.globals.frame_interval != this->frame_interval){
            // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
            this->start_native_capture = now;
            this->frame_interval = ini.globals.frame_interval;
            this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec
        }

        if (ini.globals.break_interval != this->break_interval){
            this->start_break_capture = now;
            this->break_interval = ini.globals.break_interval; // break interval is in s, default value 1 break every 10 minutes
            this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec
        }
        LOG(LOG_INFO, "update configuration png_interval=%u frame_interval=%u break_interval=%u",
            this->png_interval, this->frame_interval, this->break_interval);
    }

    ~Capture(){
    }

    void start(const timeval& now)
    {
        this->nc.send_time_start(now);
    }

    void start_with_invalid_now()
    {
        struct timeval now = {0,0};
        this->nc.send_time_start(now);
    }

    void timestamp()
    {
        this->nc.recorder.timestamp();
    }

    void set_prefix(const char * prefix, size_t len_prefix)
    {
        size_t len = (len_prefix < sizeof(log_prefix))?len_prefix:(sizeof(log_prefix)-1);
        memcpy(this->log_prefix, prefix, len);
        this->log_prefix[len] = 0;
    }

    TODO("looks better to have some function in capture returning native recorder if any and perform meta.emit outside this class. Or some other strategy not implying capture having a dependance on MetaWRM. Logicaly dependence should be between MetaWRM and native recorder")
    void timestamp(uint64_t usecond)
    {
        this->nc.recorder.timestamp(usecond);
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
            if (difftimeval(now, this->start_break_capture) >= this->inter_frame_interval_start_break_capture){
                this->breakpoint();
                this->start_break_capture = now;
            }
        }
        this->nc.recorder.flush();
    }

    void flush()
    {}

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

    void breakpoint(const timeval& now)
    {
        this->nc.recorder.timestamp(now);
        this->nc.breakpoint(this->sc.drawable.data,
                            24,
                            this->sc.drawable.width,
                            this->sc.drawable.height,
                            this->sc.drawable.rowsize,
                            now);
    }

    void breakpoint()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        this->breakpoint(now);
    }

    void dump_png()
    {
        this->sc.dump_png();
    }

    TimerCapture& timer()
    {
        return this->nc.recorder.timer;
    }

    /*Stream& stream()
    {
        return this->nc.recorder.stream;
    }*/

};

#endif
