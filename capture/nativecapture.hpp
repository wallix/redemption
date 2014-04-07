/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Meng Tan
*/

#ifndef _REDEMPTION_CAPTURE_NATIVECAPTURE_HPP_
#define _REDEMPTION_CAPTURE_NATIVECAPTURE_HPP_

#include <errno.h>
#include "bitmap.hpp"
#include "rect.hpp"
#include "difftimeval.hpp"

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"


#include "error.hpp"
#include "config.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "colors.hpp"

#include "GraphicToFile.hpp"
#include "png.hpp"

class NativeCapture : public RDPGraphicDevice
{
public:
    int width;
    int height;
    int bpp;

    uint64_t frame_interval;
    struct timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    struct timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    BmpCache & bmp_cache;
    GraphicToFile recorder;
    uint32_t nb_file;
    uint64_t time_to_wait;

    bool pointer_displayed;

    bool disable_keyboard_log_wrm;

    NativeCapture(const timeval & now, Transport & trans, int width, int height, BmpCache & bmp_cache, RDPDrawable & drawable,
                  const Inifile & ini)
    : width(width)
    , height(height)
    , bpp(24)
    , bmp_cache(bmp_cache)
    , recorder(now, &trans, width, height, 24, bmp_cache, drawable, ini)
    , nb_file(0)
    , time_to_wait(0)
    , pointer_displayed(false)
    , disable_keyboard_log_wrm(ini.video.disable_keyboard_log_wrm)
    {
        // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
        this->start_native_capture = now;
        this->frame_interval = 40;
        this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec

        this->start_break_capture = now;
        this->break_interval = 60 * 10; // break interval is in s, default value 1 break every 10 minutes
        this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec

        this->update_config(ini);
    }

    ~NativeCapture(){
        this->recorder.flush();
    }

    void update_config(const Inifile & ini)
    {
        if (ini.video.frame_interval != this->frame_interval){
            // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
            this->frame_interval = ini.video.frame_interval;
            this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec
        }

        if (ini.video.break_interval != this->break_interval){
            this->break_interval = ini.video.break_interval; // break interval is in s, default value 1 break every 10 minutes
            this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec
        }
    }

    void snapshot( const timeval & now, int x, int y, bool ignore_frame_in_timeval) {
        if (difftimeval(now, this->start_native_capture)
                >= this->inter_frame_interval_native_capture) {
            this->recorder.timestamp(now);
            this->time_to_wait = this->inter_frame_interval_native_capture;
            if (!this->pointer_displayed) {
                this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
            }
            this->start_native_capture = now;
            if (difftimeval(now, this->start_break_capture)
                    >= this->inter_frame_interval_start_break_capture) {
                this->breakpoint();
                this->start_break_capture = now;
            }
        }
        else {
            this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
        }
    }

    virtual void flush()
    {
        this->recorder.flush();
    }

    virtual void input(const timeval & now, Stream & input_data_32) {
        if (!this->disable_keyboard_log_wrm) {
            this->recorder.input(now, input_data_32);
        }
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->recorder.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->recorder.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphCache & cmd)
    {
        this->recorder.draw(cmd);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        this->recorder.draw(cmd, clip, gly_cache);
    }

    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
            size_t size, const Bitmap & bmp) {
        this->recorder.draw(bitmap_data, data, size, bmp);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip)
    {
        this->recorder.draw(cmd, clip);
    }

    virtual void send_pointer(int cache_idx, const Pointer & cursor) {
        this->recorder.send_pointer(cache_idx, cursor);
    }

    virtual void set_pointer(int cache_idx) {
        this->recorder.set_pointer(cache_idx);
    }

    virtual void set_pointer_display() {
        this->pointer_displayed = true;
    }

private:
    void breakpoint()
    {
        this->recorder.breakpoint();
    }
};

#endif
