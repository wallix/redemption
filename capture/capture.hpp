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

    uint64_t frame_interval;
    struct timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    struct timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    FileSequence * png_sequence;
    OutByFilenameSequenceTransport * png_trans;
    StaticCapture * psc;

    FileSequence * wrm_sequence;
    OutByFilenameSequenceTransport * wrm_trans;
    NativeCapture * pnc;

public:
    Capture(const timeval & now, int width, int height, const char * fullpath, const char * codec_id, const char * video_quality, bool bgr = true) 
      : png_sequence(NULL)
      , png_trans(NULL)
      , psc(NULL)
    {
        LOG(LOG_INFO, "=======================================> Capture : fullpath = %s", fullpath);

        char path[1024];
        char basename[1024];
        strcpy(path, "/tmp/"); 
        strcpy(basename, "redemption"); 
        const char * end_of_path = strrchr(fullpath, '/') + 1;
        if (end_of_path){
            memcpy(path, fullpath, end_of_path - fullpath);
            path[end_of_path - fullpath] = 0;
            const char * start_of_extension = strrchr(end_of_path, '.');
            if (start_of_extension){
                memcpy(basename, end_of_path, start_of_extension - end_of_path);
                basename[start_of_extension - end_of_path] = 0;
            }
            else {
                if (end_of_path[0]){
                    strcpy(basename, end_of_path);
                }
            }
        }
        
        LOG(LOG_INFO, "=======================================> Capture : path = %s basename=%s", path, basename);
        this->png_sequence = new FileSequence("path file pid count extension", path, basename, "png");
        this->png_trans = new OutByFilenameSequenceTransport(*this->png_sequence);
        this->psc = new StaticCapture(now, *this->png_trans, *this->png_sequence, width, height, true);

        this->log_prefix[0] = 0;

        // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
        this->start_native_capture = now;
        this->frame_interval = 40;
        this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec

        this->start_break_capture = now;
        this->break_interval = 60 * 10; // break interval is in s, default value 1 break every 10 minutes
        this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec

        LOG(LOG_INFO, "update configuration frame_interval=%u break_interval=%u",
            this->frame_interval, this->break_interval);

        // fullpath
        this->wrm_sequence = new FileSequence("path file pid count extension", path, basename, "wrm");
        this->wrm_trans = new OutByFilenameSequenceTransport(*this->wrm_sequence);
        this->pnc = new NativeCapture(now, width, height, *wrm_trans);
    }

    ~Capture(){
        delete this->psc;
        TODO("Use a Closure to wrap these 3 fields, after stabilizing API")
        delete this->pnc;
        delete this->png_sequence;
        delete this->png_trans;
    }
    
    void update_config(const timeval & now, const Inifile & ini){
        this->psc->update_config(ini);
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
        LOG(LOG_INFO, "update configuration frame_interval=%u break_interval=%u",
            this->frame_interval, this->break_interval);
    }

    void set_prefix(const char * prefix, size_t len_prefix)
    {
        size_t len = (len_prefix < sizeof(log_prefix))?len_prefix:(sizeof(log_prefix)-1);
        memcpy(this->log_prefix, prefix, len);
        this->log_prefix[len] = 0;
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        this->psc->snapshot(now, x, y, pointer_already_displayed, no_timestamp);
        TODO("this must move to native capture ie: nc.snapshot(now)")
        if (difftimeval(now, this->start_native_capture) >= this->inter_frame_interval_native_capture){
            LOG(LOG_INFO, "recorder timestamp");
            this->pnc->recorder.timestamp(now);
            this->start_native_capture = now;
            if (difftimeval(now, this->start_break_capture) >= this->inter_frame_interval_start_break_capture){
                this->breakpoint();
                this->start_break_capture = now;
            }
        }
        this->pnc->recorder.flush();
    }

    void flush()
    {}

    void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->psc->draw(cmd, clip, bmp);
        this->pnc->draw(cmd, clip, bmp);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }


    void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->psc->draw(cmd, clip);
        this->pnc->draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
//        this->psc->glyph_index(cmd, clip);
//        this->pnc->glyph_index(cmd, clip);
    }

    void breakpoint(const timeval& now)
    {
        this->pnc->recorder.timestamp(now);
        this->pnc->breakpoint(this->psc->drawable.data,
                            24,
                            this->psc->drawable.width,
                            this->psc->drawable.height,
                            this->psc->drawable.rowsize,
                            now);
    }

    void breakpoint()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        this->breakpoint(now);
    }

    timeval& timer()
    {
        return this->pnc->recorder.timer;
    }

};

#endif
