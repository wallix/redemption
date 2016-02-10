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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#ifndef _REDEMPTION_CAPTURE_CAPTURE_HPP_
#define _REDEMPTION_CAPTURE_CAPTURE_HPP_

#include "transport/out_meta_sequence_transport.hpp"
#include "transport/out_filename_sequence_transport.hpp"

#include "nativecapture.hpp"
#include "staticcapture.hpp"
#include "new_kbdcapture.hpp"

#include "wait_obj.hpp"

#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/input_kbd_api.hpp"
#include "gdi/input_pointer_api.hpp"
#include "gdi/capture_probe_api.hpp"

#include "utils/pattutils.hpp"
#include "dump_png24_from_rdp_drawable_adapter.hpp"
#include "gdi/utils/non_null.hpp"

#include "utils/graphic_capture_impl.hpp"
#include "utils/wrm_capture_impl.hpp"

class Capture final
: public gdi::GraphicApi
, public gdi::CaptureApi
, public gdi::InputKbdApi
, public gdi::InputPointer
, public gdi::CaptureProbeApi
{
    // for snapshot
    using MouseInfo = MouseTrace;

    using Graphic = GraphicCaptureImpl;


    class Static
    {
        OutFilenameSequenceTransport trans;
        StaticCapture sc;

    public:
        Static(
            const timeval & now, bool clear_png, auth_api * authentifier, Graphic & graphic,
            const char * record_tmp_path, const char * basename, int groupid,
            const Inifile & ini)
        : trans(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
            record_tmp_path, basename, ".png", groupid, authentifier)
        , sc(now, this->trans, this->trans.seqgen(), graphic.impl().width(), graphic.impl().height(),
             clear_png, ini, graphic.impl())
        {}

        void attach_apis(ApisRegister & apis_register, const Inifile &) {
            apis_register.capture_list.push_back(this->sc);
            apis_register.graphic_snapshot_list->push_back(this->sc);
        }

        void zoom(unsigned percent) {
            this->sc.zoom(percent);
        }
    };


    using Native = WrmCaptureImpl;


    class Kbd
    {
        NewKbdCapture kc;

    public:
        Kbd(const timeval & now, auth_api * authentifier, const Inifile & ini)
        : kc(now, authentifier, ini.get<cfg::context::pattern_kill>().c_str(),
            ini.get<cfg::context::pattern_notify>().c_str(),
            !bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog),
            /*is_kc_driven_by_ocr=*/false,
            ini.get<cfg::debug::capture>())
        {}

        void attach_apis(ApisRegister & api_register, const Inifile & ini) {
            api_register.capture_list.push_back(this->kc);
            api_register.input_kbd_list.push_back(this->kc);
            api_register.capture_probe_list.push_back(this->kc);
        }

        void enable_keyboard_input_mask(bool enable) {
            this->kc.enable_keyboard_input_mask(enable);
        }
    };


    struct NewCapture : gdi::CaptureDispatcher<NewCapture>
    {
        NewCapture(Capture & cap) : cap(cap) {}

        std::chrono::microseconds snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) override {
            this->cap.capture_event.reset();

            if (this->cap.gd) {
                this->cap.gd->rdp_drawable().set_mouse_cursor_pos(cursor_x, cursor_y);
            }

            this->cap.mouse_info = {now, cursor_x, cursor_y};

            std::chrono::microseconds time = std::chrono::microseconds::max();
            if (!this->caps.empty()) {
                time = NewCapture::base_type::snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
                this->cap.capture_event.update(time.count());
            }
            return time;
        }

        friend gdi::CaptureCoreAccess;

        using captures_list = std::vector<std::reference_wrapper<gdi::CaptureApi>>;
        captures_list caps;

        captures_list & get_capture_list_impl() {
            return this->caps;
        }

        Capture & cap;
    };


    struct NewInputKbd : gdi::InputKbdApi
    {
        bool input_kbd(const timeval & now, array_view<uint8_t const> const & input_data_32) override {
            bool ret = true;
            for (gdi::InputKbdApi & kbd : this->kbds) {
                ret &= kbd.input_kbd(now, input_data_32);
            }
            return ret;
        }

        std::vector<std::reference_wrapper<gdi::InputKbdApi>> kbds;
    };


    struct NewInputPointer : gdi::InputPointer
    {
        void update_pointer_position(uint16_t x, uint16_t y) override {
            for (gdi::InputPointer & mouse : this->mouses) {
                mouse.update_pointer_position(x, y);
            }
        }

        std::vector<std::reference_wrapper<gdi::InputPointer>> mouses;
    };


    struct NewCaptureProbe : gdi::CaptureProbeApi
    {
        void possible_active_window_change() override {
            for (gdi::CaptureProbeApi & cap_prob : this->cds) {
                cap_prob.possible_active_window_change();
            }
        }

        void session_update(const timeval& now, array_const_char const & message) override {
            for (gdi::CaptureProbeApi & cap_prob : this->cds) {
                cap_prob.session_update(now, message);
            }
        }

        std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> cds;
    };



public:
    const bool capture_wrm;
    const bool capture_png;
// for extension
// end extension

    wait_obj capture_event;

private:
    CryptoContext & cctx;

private:
// for extension
// end extension

    MouseInfo mouse_info;

    std::unique_ptr<Graphic> gd;
    std::unique_ptr<Native> pnc;
    std::unique_ptr<Static> psc;
    std::unique_ptr<Kbd> pkc;

    NewCapture capture_api;
    NewInputKbd input_kbd_api;
    NewInputPointer input_pointer_api;
    NewCaptureProbe capture_probe_api;
    Graphic::GraphicApi * graphic_api = nullptr;

    ApisRegister get_apis_register() {
        return {
            this->graphic_api ? &this->graphic_api->gds : nullptr,
            this->graphic_api ? &this->graphic_api->snapshoters : nullptr,
            this->capture_api.caps,
            this->input_kbd_api.kbds,
            this->input_pointer_api.mouses,
            this->capture_probe_api.cds
        };
    };

public:
    Capture(
        const timeval & now,
        int width, int height, int order_bpp, int capture_bpp,
        bool clear_png, bool no_timestamp, auth_api * authentifier,
        const Inifile & ini, Random & rnd, CryptoContext & cctx,
        bool full_video, bool extract_meta_data)
    : capture_wrm(bool(ini.get<cfg::video::capture_flags>() & configs::CaptureFlags::wrm))
    , capture_png(ini.get<cfg::video::png_limit>() > 0)
    , capture_event{}
    , cctx(cctx)
    , mouse_info{now, width / 2, height / 2}
    , capture_api(*this)
    {
        TODO("Remove that after change of capture interface")
        (void)full_video;
        TODO("Remove that after change of capture interface")
        (void)extract_meta_data;
        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const bool capture_drawable = this->capture_wrm || this->capture_png;
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>();
        const char * record_path = ini.get<cfg::video::record_path>();
        const char * hash_path = ini.get<cfg::video::hash_path>();

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, "redemption"); // default value actual one should come from movie_path
        strcpy(extension, "");          // extension is currently ignored
        const bool res = canonical_path(ini.get<cfg::globals::movie_path>().c_str(),
                                        path, sizeof(path),
                                        basename, sizeof(basename),
                                        extension, sizeof(extension));
        if (!res) {
            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
        }


        if (capture_drawable) {
            this->gd.reset(new Graphic(width, height, order_bpp, this->mouse_info));
            this->graphic_api = &this->gd->get_graphic_api();

            if (this->capture_png) {
                if (recursive_create_directory(record_tmp_path, S_IRWXU|S_IRWXG, groupid) != 0) {
                    LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_tmp_path);
                }

                this->psc.reset(new Static(
                    now, clear_png, authentifier, *this->gd,
                    record_tmp_path, basename, groupid, ini
                ));
            }

            if (this->capture_wrm) {
                if (recursive_create_directory(record_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                    LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_path);
                }

                if (recursive_create_directory(hash_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                    LOG(LOG_ERR, "Failed to create directory: \"%s\"", hash_path);
                }

                this->pnc.reset(new Native(
                    now, capture_bpp, ini.get<cfg::globals::trace_type>(),
                    this->cctx, record_path, hash_path, basename,
                    groupid, authentifier, this->gd->rdp_drawable(), ini
                ));
            }
        }

        if (!bool(ini.get<cfg::video::disable_keyboard_log>() & configs::KeyboardLogFlags::syslog) ||
            ini.get<cfg::session_log::enable_session_log>() ||
            ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str()) ||
            ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())) {
            this->pkc.reset(new Kbd(now, authentifier, ini));
        }

        ApisRegister apis_register = this->get_apis_register();

        if (this->gd ) { this->gd->attach_apis (apis_register, ini); }
        if (this->pnc) { this->pnc->attach_apis(apis_register, ini); }
        if (this->psc) { this->psc->attach_apis(apis_register, ini); }
        if (this->pkc) { this->pkc->attach_apis(apis_register, ini); }

        if (this->gd ) { this->gd->start(); }
    }

    ~Capture() override {
        this->pkc.reset();

        this->psc.reset();

        if (this->pnc) {
            timeval now = tvtime();
            this->pnc->send_timestamp_chunk(now, false);
            this->pnc.reset();
        }

        this->gd.reset();
    }

    void request_full_cleaning()
    {
        if (this->pnc) {
            this->pnc->request_full_cleaning();
        }
    }

    void pause_capture(timeval const & now) {
        this->capture_api.pause_capture(now);
        this->capture_event.reset();
    }

    void resume_capture(timeval const & now) {
        this->capture_api.resume_capture(now);
        this->capture_event.set();
    }

    void update_config(const Inifile & ini) {
        this->capture_api.update_config(ini);
    }

    void set_row(size_t rownum, const uint8_t * data) override {
        if (this->gd){
            this->gd->rdp_drawable().set_row(rownum, data);
        }
    }

    std::chrono::microseconds snapshot(
        const timeval & now,
        int x, int y,
        bool ignore_frame_in_timeval
    ) override {
        return this->capture_api.snapshot(now, x, y, ignore_frame_in_timeval);
    }

    void sync() override {
        if (this->graphic_api) {
            this->graphic_api->sync();
        }
    }

    bool input_kbd(const timeval & now, array_const_u8 const & input_data_32) override {
        return this->input_kbd_api.input_kbd(now, input_data_32);
    }

    void update_pointer_position(uint16_t x, uint16_t y) override {
        this->input_pointer_api.update_pointer_position(x, y);
    }

    void enable_keyboard_input_mask(bool enable) {
        if (this->pnc) {
            ApisRegister apis_register = this->get_apis_register();
            this->pnc->enable_keyboard_input_mask(apis_register, enable);
        }

        if (this->pkc) {
            this->pkc->enable_keyboard_input_mask(enable);
        }
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPDestBlt & cmd, const Rect &clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip, bmp);
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd);
        }
    }

    void draw(const RDPColCache & cmd) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd);
        }
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip, gly_cache);
        }
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->graphic_api) {
            this->graphic_api->draw(bitmap_data, bmp);
        }
    }

    void draw(const RDP::FrameMarker & cmd) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd);
        }
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->graphic_api) {
            this->graphic_api->draw(cmd, clip);
        }
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->graphic_api) {
            this->graphic_api->draw(order);
        }
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->graphic_api) {
            this->graphic_api->draw(order);
        }
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->graphic_api) {
            this->graphic_api->draw(order);
        }
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->graphic_api) {
            this->graphic_api->draw(order);
        }
    }

    gdi::GraphicApi * get_graphic_api() const {
        return this->graphic_api;
    }

    void add_graphic(gdi::GraphicApi & gd) {
        if (this->graphic_api) {
            this->get_apis_register().graphic_list->push_back(gd);
            // TODO
            this->gd->start();
        }
    }

    void set_pointer(const Pointer & cursor) override {
        if (this->graphic_api) {
            this->graphic_api->set_pointer(cursor);
        }
    }

    void set_palette(const BGRPalette & palette) override {
        if (this->graphic_api) {
            this->graphic_api->set_palette(palette);
        }
    }

    void set_pointer_display() /*override*/ {
        if (this->gd) {
            this->gd->rdp_drawable().show_mouse_cursor(false);
        }
    }

    // toggles externally genareted breakpoint.
    void external_breakpoint() override {
        this->capture_api.external_breakpoint();
    }

    void external_time(const timeval & now) override {
        this->capture_api.external_time(now);
    }

    void session_update(const timeval & now, array_const_char const & message) override {
        this->capture_probe_api.session_update(now, message);
    }

    void possible_active_window_change() override {
        this->capture_probe_api.possible_active_window_change();
    }

    void zoom(unsigned percent) {
        assert(this->psc);
        this->psc->zoom(percent);
    }
};

#endif
