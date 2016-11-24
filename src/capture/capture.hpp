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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include "capture/utils/graphic_capture_impl.hpp"
#include "capture/utils/wrm_capture_impl.hpp"
#include "capture/utils/kbd_capture_impl.hpp"
#include "capture/utils/image_capture_impl.hpp"
#include "capture/utils/capture_apis_impl.hpp"
#include "utils/sugar/array_view.hpp"

#include "capture/utils/capture_impl.hpp"

class Capture final
: public gdi::GraphicBase<Capture>
, public gdi::CaptureApi
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
, public gdi::ExternalCaptureApi
, public gdi::UpdateConfigCaptureApi
{
    using Graphic = GraphicCaptureImpl;

    using Static = ImageCaptureImpl;

    using Native = WrmCaptureImpl;

    using Kbd = KbdCaptureImpl;

    using Video = VideoCaptureImpl;

    using Meta = MetaCaptureImpl;

    using Title = TitleCaptureImpl;

    const bool is_replay_mod;

    struct VideoImageCapture
    {
        OutFilenameSequenceTransport trans;
        ImageCapture ic;

        VideoImageCapture(
            const timeval & now, Drawable & drawable,
            const char * path, const char * basename, int groupid)
        : trans(
            FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
            path, basename, ".png", groupid, nullptr)
        , ic(now, drawable, this->trans, std::chrono::seconds{})
        {}
    };

// TODO
public:
    const bool capture_wrm;
    const bool capture_png;
    const bool capture_ocr;
    const bool capture_flv;
    const bool capture_flv_full; // capturewab only
    const bool capture_meta; // capturewab only

private:
    std::unique_ptr<Graphic> gd;
    std::unique_ptr<Native> pnc;
    std::unique_ptr<Static> psc;
    std::unique_ptr<Kbd> pkc;
    std::unique_ptr<Video> pvc;
    std::unique_ptr<Video> pvc_full;
    std::unique_ptr<Meta> pmc;
    std::unique_ptr<Title> ptc;
    std::unique_ptr<VideoImageCapture> pivc;

    CaptureApisImpl::Capture capture_api;
    CaptureApisImpl::KbdInput kbd_input_api;
    CaptureApisImpl::CaptureProbe capture_probe_api;
    CaptureApisImpl::ExternalCapture external_capture_api;
    CaptureApisImpl::UpdateConfigCapture update_config_capture_api;
    Graphic::GraphicApi * graphic_api = nullptr;

    const int delta_time;

    ApisRegister get_apis_register() {
        return {
            this->graphic_api ? &this->graphic_api->gds : nullptr,
            this->graphic_api ? &this->graphic_api->snapshoters : nullptr,
            this->capture_api.caps,
            this->kbd_input_api.kbds,
            this->capture_probe_api.probes,
            this->external_capture_api.objs,
            this->update_config_capture_api.objs,
        };
    }

public:
    Capture(
        const timeval & now,
        int width,
        int height,
        int order_bpp,
        int capture_bpp,
        bool clear_png,
        bool no_timestamp,
        auth_api * authentifier,
        const Inifile & ini,
        CryptoContext & cctx,
        Random & rnd,
        bool full_video,
        bool force_capture_png_if_enable = false,
        const int delta_time = 1000)
    : is_replay_mod(!authentifier)
    , capture_wrm(bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::wrm))
    , capture_png(bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::png)
                  && (!authentifier || ini.get<cfg::video::png_limit>() > 0))
    , capture_ocr(bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::ocr)
                 || ::contains_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
                 || ::contains_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str()))
    , capture_flv(bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::flv))
    // capture wab only
    , capture_flv_full(full_video)
    // capture wab only
    , capture_meta(this->capture_ocr)
    , capture_api(now, width / 2, height / 2)
    , delta_time(delta_time)
    {
        REDASSERT(authentifier ? order_bpp == capture_bpp : true);

        bool const enable_kbd
          = authentifier
          ? !bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)
          || ini.get<cfg::session_log::enable_session_log>()
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str())
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())
          : false
        ;

        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable capture:  wrm=%d  png=%d  kbd=%d  flv=%d  flv_full=%d  ocr=%d  meta=%d",
                this->capture_wrm ? 1 : 0,
                this->capture_png ? 1 : 0,
                enable_kbd ? 1 : 0,
                this->capture_flv ? 1 : 0,
                this->capture_flv_full ? 1 : 0,
                this->capture_ocr ? (ini.get<cfg::ocr::version>() == OcrVersion::v2 ? 2 : 1) : 0,
                this->capture_meta
            );
        }

        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const bool capture_drawable = this->capture_wrm || this->capture_flv
                                   || this->capture_ocr || this->capture_png
                                   || this->capture_flv_full;
        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
        const char * record_path = authentifier ? ini.get<cfg::video::record_path>().c_str() : record_tmp_path;
        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();

        if (this->capture_png || (authentifier && (this->capture_flv || this->capture_ocr))) {
            if (recursive_create_directory(record_tmp_path, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP, -1) != 0) {
                LOG(LOG_INFO, "Failed to create directory: \"%s\"", record_tmp_path);
            }
        }

        char path[1024];
        char basename[1024];
        char extension[128];
        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
        strcpy(basename, ini.get<cfg::globals::movie_path>().c_str());
        strcpy(extension, "");          // extension is currently ignored

        if (!canonical_path(
            ini.get<cfg::globals::movie_path>().c_str()
          , path, sizeof(path)
          , basename, sizeof(basename)
          , extension, sizeof(extension))
        ) {
            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
        }

        LOG(LOG_INFO, "canonical_path : %s%s%s\n", path, basename, extension);

        if (authentifier) {
            cctx.set_master_key(ini.get<cfg::crypto::key0>());
            cctx.set_hmac_key(ini.get<cfg::crypto::key1>());
        }

        if (capture_drawable) {
            this->gd.reset(new Graphic(width, height, order_bpp, this->capture_api.mouse_trace()));
            this->graphic_api = &this->gd->get_graphic_api();
            this->capture_api.set_drawable(&this->gd->impl());

            if (this->capture_png) {
                if (clear_png || force_capture_png_if_enable) {
                    this->psc.reset(new Static(
                        now, clear_png, authentifier, this->gd->impl(),
                        record_tmp_path, basename, groupid, ini
                    ));
                }
                if (!clear_png) {
                    this->pivc.reset(new VideoImageCapture(
                        now, this->gd->impl(),
                        record_tmp_path, basename, groupid
                    ));
                }

            }

            if (this->capture_wrm) {
                if (authentifier) {
                    if (recursive_create_directory(record_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                        LOG(LOG_ERR, "Failed to create directory: \"%s\"", record_path);
                    }

                    if (recursive_create_directory(hash_path, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                        LOG(LOG_ERR, "Failed to create directory: \"%s\"", hash_path);
                    }
                }
                this->pnc.reset(new Native(
                    now, capture_bpp, ini.get<cfg::globals::trace_type>(),
                    cctx, rnd, record_path, hash_path, basename,
                    groupid, authentifier, this->gd->rdp_drawable(), ini, this->delta_time
                ));
            }

            if (this->capture_ocr) {
                this->pmc.reset(new Meta(
                    now, record_tmp_path, basename,
                    authentifier && ini.get<cfg::session_log::enable_session_log>()
                ));
            }

            if (this->capture_flv) {
                this->pvc.reset(new Video(
                    now, record_path, basename, groupid, authentifier,
                    no_timestamp, this->gd->impl(),
                    video_params_from_ini(this->gd->impl().width(), this->gd->impl().height(), ini),
                    std::chrono::seconds(ini.get<cfg::video::flv_break_interval>()),
                    ini.get<cfg::globals::capture_chunk>()
                    ? Video::SynchronizerNext{
                        this->pmc ? &this->pmc->get_session_meta() : nullptr,
                        this->pivc ? &this->pivc->ic : nullptr}
                    : Video::SynchronizerNext{nullptr, nullptr}
                ));
            }

            if (this->capture_flv_full) {
                this->pvc_full.reset(new Video(
                    now, record_path, basename, groupid, authentifier,
                    no_timestamp, this->gd->impl(),
                    video_params_from_ini(this->gd->impl().width(), this->gd->impl().height(), ini),
                    std::chrono::seconds(0),
                    Video::SynchronizerNext{nullptr, nullptr}
                ));
            }

            if (this->capture_ocr) {
                this->ptc.reset(new Title(
                    now, authentifier, this->gd->impl(),
                    this->pmc ? &this->pmc->get_session_meta() : nullptr,
                    this->pvc.get(),
                    ini
                ));
            }
        }

        // TODO this->pkc = Kbd::construct(now, authentifier, ini); ?
        if (enable_kbd) {
            this->pkc.reset(new Kbd(now, authentifier, ini));
        }
        ApisRegister apis_register = this->get_apis_register();

        if (this->gd ) { this->gd ->attach_apis(apis_register, ini); }
        if (this->pnc) { this->pnc->attach_apis(apis_register, ini); }
        if (this->psc) { this->psc->attach_apis(apis_register, ini); }
        if (this->pkc) { this->pkc->attach_apis(apis_register, ini); }
        if (this->pvc) { this->pvc->attach_apis(apis_register, ini); }
        if (this->pvc_full) { this->pvc_full->attach_apis(apis_register, ini); }
        if (this->pmc) { this->pmc->attach_apis(apis_register, ini); }
        if (this->ptc) { this->ptc->attach_apis(apis_register, ini); }

        if (this->gd) { this->gd->start(); }
    }

    ~Capture() {
        if (this->is_replay_mod) {
            this->psc.reset();
            this->pnc.reset();
            if (this->pvc) {
                try {
                    this->pvc->encoding_video_frame();
                }
                catch (Error const &) {
                    this->pvc->request_full_cleaning();
                    if (this->pmc) {
                        this->pmc->request_full_cleaning();
                    }
                }
                this->pvc.reset();
            }
            if (this->pvc_full) {
                try {
                    this->pvc_full->encoding_video_frame();
                }
                catch (Error const &) {
                    this->pvc_full->request_full_cleaning();
                }
                this->pvc_full.reset();
            }
        }
        else {
            this->ptc.reset();
            this->pkc.reset();
            this->pvc.reset();
            this->psc.reset();

            if (this->pnc) {
                timeval now = tvtime();
                this->pnc->send_timestamp_chunk(now, false);
                this->pnc.reset();
            }
        }
    }

    wait_obj & get_capture_event() {
        return this->capture_api.get_capture_event();
    }

    void update_config(Inifile const & ini) override {
        this->update_config_capture_api.update_config(ini);
    }

    void set_row(size_t rownum, const uint8_t * data) override {
        if (this->gd) {
            this->gd->rdp_drawable().set_row(rownum, data);
        }
    }

    void sync() override
    {
        if (this->graphic_api) {
            this->graphic_api->sync();
        }
    }

    bool kbd_input(const timeval& now, uint32_t uchar) override {
        return this->kbd_input_api.kbd_input(now, uchar);
    }

    void enable_kbd_input_mask(bool enable) override {
        this->kbd_input_api.enable_kbd_input_mask(enable);
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

    void set_order_bpp(uint8_t order_bpp) {
        if (this->graphic_api) {
            this->gd->update_order_bpp(order_bpp);
        }
    }

protected:
    std::chrono::microseconds do_snapshot(
        const timeval& now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override {
        return this->capture_api.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    void do_pause_capture(timeval const & now) override {
        this->capture_api.pause_capture(now);
    }

    void do_resume_capture(timeval const & now) override {
        this->capture_api.resume_capture(now);
    }

    friend gdi::GraphicCoreAccess;
    template<class... Ts>
    void draw_impl(const Ts & ... args) {
        if (this->graphic_api) {
            this->graphic_api->draw(args...);
        }
    }

public:
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

    void set_pointer_display() {
        if (this->gd) {
            this->gd->rdp_drawable().show_mouse_cursor(false);
        }
    }

    void external_breakpoint() override {
        this->external_capture_api.external_breakpoint();
    }

    void external_time(const timeval& now) override {
        this->external_capture_api.external_time(now);
    }

    void session_update(const timeval & now, array_view_const_char message) override {
        this->capture_probe_api.session_update(now, message);
    }

    void possible_active_window_change() override {
        this->capture_probe_api.possible_active_window_change();
    }

    // TODO move to ctor
    void zoom(unsigned percent) {
        if (this->psc) {
            this->psc->zoom(percent);
        }
        if (this->pivc) {
            this->pivc->ic.zoom(percent);
        }
    }
};
