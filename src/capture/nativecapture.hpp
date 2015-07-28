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

#include "CaptureDevice.hpp"
#include "difftimeval.hpp"
#include "GraphicToFile.hpp"

class NativeCapture : public RDPGraphicDevice, public RDPCaptureDevice
{
public:
    uint64_t frame_interval;
    timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    GraphicToFile recorder;
    uint32_t nb_file;
    uint64_t time_to_wait;

    bool disable_keyboard_log_wrm;

    bool externally_generated_breakpoint;

    typedef GraphicToFile::SendInput SendInput;

    NativeCapture( const timeval & now, Transport & trans, int width, int height, int capture_bpp, BmpCache & bmp_cache
                 , GlyphCache & gly_cache, PointerCache & ptr_cache, RDPDrawable & drawable, const Inifile & ini
                 , bool externally_generated_breakpoint = false, SendInput send_input = SendInput::NO)
    : recorder( now, &trans, width, height, capture_bpp, bmp_cache, gly_cache, ptr_cache, drawable, ini, send_input
              , ini.get<cfg::debug::capture>())
    , nb_file(0)
    , time_to_wait(0)
    , disable_keyboard_log_wrm(bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::wrm))
    , externally_generated_breakpoint(externally_generated_breakpoint)
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

    ~NativeCapture() override {
        this->recorder.flush();
    }

    void update_config(const Inifile & ini)
    {
        if (ini.get<cfg::video::frame_interval>() != this->frame_interval){
            // frame interval is in 1/100 s, default value, 1 timestamp mark every 40/100 s
            this->frame_interval = ini.get<cfg::video::frame_interval>();
            this->inter_frame_interval_native_capture       =  this->frame_interval * 10000; // 1 000 000 us is 1 sec
        }

        if (ini.get<cfg::video::break_interval>() != this->break_interval){
            this->break_interval = ini.get<cfg::video::break_interval>(); // break interval is in s, default value 1 break every 10 minutes
            this->inter_frame_interval_start_break_capture  = 1000000 * this->break_interval; // 1 000 000 us is 1 sec
        }
    }

    void snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval,
                          bool const & requested_to_stop) override {
        if (difftimeval(now, this->start_native_capture)
                >= this->inter_frame_interval_native_capture) {
            this->recorder.timestamp(now);
            this->time_to_wait = this->inter_frame_interval_native_capture;
            this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
            this->start_native_capture = now;
            if (!this->externally_generated_breakpoint &&
                (difftimeval(now, this->start_break_capture) >=
                 this->inter_frame_interval_start_break_capture)) {
                this->recorder.breakpoint();
                this->start_break_capture = now;
            }
        }
        else {
            this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
        }
    }

    void flush() override {
        this->recorder.flush();
    }

    void input(const timeval & now, Stream & input_data_32) override {
        if (!this->disable_keyboard_log_wrm) {
            return this->recorder.input(now, input_data_32);
        }

        return true;
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        this->recorder.draw(cmd, clip, bmp);
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        this->recorder.draw(cmd, clip, bmp);
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        this->recorder.draw(cmd, clip, gly_cache);
    }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
            size_t size, const Bitmap & bmp) override {
        this->recorder.draw(bitmap_data, data, size, bmp);
    }

    void draw(const RDP::FrameMarker & order) override {
        this->recorder.draw(order);
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        this->recorder.draw(cmd, clip);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        this->recorder.draw(order);
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        this->recorder.draw(order);
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        this->recorder.draw(order);
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        this->recorder.draw(order);
    }

    using RDPGraphicDevice::draw;

    void server_set_pointer(const Pointer & cursor) override {
        this->recorder.server_set_pointer(cursor);
    }

    // toggles externally genareted breakpoint.
    void external_breakpoint() override {
        REDASSERT(this->externally_generated_breakpoint);

        this->recorder.breakpoint();
    }

    void external_time(const timeval & now) override {
        this->recorder.flush();
        this->recorder.timestamp(now);
    }

    void session_update(const timeval & now, const char * message) override {
        this->recorder.session_update(now, message);
    }
};

#endif
