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
#include "gdi/input_kbd_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/synchronise_api.hpp"
#include "utils/dump_png24_from_rdp_drawable_adapter.hpp"

class NativeCapture : public RDPCaptureDevice, public gdi::InputKbdApi, public gdi::CaptureApi, public gdi::SynchroniseApi
{
// TODO private
public:
    uint64_t frame_interval;
    timeval start_native_capture;
    uint64_t inter_frame_interval_native_capture;

    uint64_t break_interval;
    timeval start_break_capture;
    uint64_t inter_frame_interval_start_break_capture;

    DumpPng24FromRDPDrawableAdapter dump_png24_api;
    GraphicToFile recorder;
    uint32_t nb_file;
    uint64_t time_to_wait;

    bool disable_keyboard_log_wrm;

    bool externally_generated_breakpoint;

    typedef GraphicToFile::SendInput SendInput;

    bool keyboard_input_mask_enabled = false;

    NativeCapture( const timeval & now, Transport & trans, int width, int height, int capture_bpp, BmpCache & bmp_cache
                 , GlyphCache & gly_cache, PointerCache & ptr_cache, RDPDrawable & drawable, const Inifile & ini
                 , bool externally_generated_breakpoint = false, SendInput send_input = SendInput::NO)
    : dump_png24_api{drawable}
    , recorder( now, &trans, width, height, capture_bpp, bmp_cache, gly_cache, ptr_cache, this->dump_png24_api, ini, send_input
              , ini.get<cfg::debug::capture>())
    , nb_file(0)
    , time_to_wait(0)
    , disable_keyboard_log_wrm(bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::wrm))
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

    virtual void pause_capture(const timeval& now) {}
    virtual void resume_capture(const timeval& now) {}

    std::chrono::microseconds snapshot(const timeval & now, int x, int y, bool ignore_frame_in_timeval) override {
        bool requested_to_stop;
        this->snapshot(now, x, y, ignore_frame_in_timeval, requested_to_stop);
        return std::chrono::microseconds{this->time_to_wait};
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

    void sync() override {
        this->recorder.flush();
    }

    bool input_kbd(const timeval& now, array_const_u8 const & input_data_32) override {
        return this->input(now, input_data_32.data(), input_data_32.size());
    }

    bool input(const timeval & now, uint8_t const * input_data_32, std::size_t data_sz) override {
        // TODO useless disable_keyboard_log_wrm (see capture::input_kbd::kbds)
        if (!this->disable_keyboard_log_wrm) {
            if (this->keyboard_input_mask_enabled) {
                StaticOutStream<256> decoded_data;
                for (unsigned char_count = data_sz / sizeof(uint32_t);
                     char_count > 0; char_count--) {
                    if (decoded_data.has_room(sizeof(uint32_t))) {
                        decoded_data.out_uint32_le('*');
                    }
                }
                return this->recorder.input(now, decoded_data.get_data(), decoded_data.get_offset());
            }

            return this->recorder.input(now, input_data_32, data_sz);
        }

        return true;
    }

    void enable_keyboard_input_mask(bool enable) {
        this->keyboard_input_mask_enabled = enable;
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
