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

#include <vector>
#include <functional>

#include "utils/sugar/array_view.hpp"
#include "capture/png_params.hpp"
#include "capture/flv_params.hpp"

template<class T>
struct ApiRegisterElement
{
    using list_type = std::vector<std::reference_wrapper<T>>;

    ApiRegisterElement() = default;

    ApiRegisterElement(list_type & l, T & x)
    : l(&l)
    , i(l.size())
    {
        l.push_back(x);
    }

    ApiRegisterElement & operator = (ApiRegisterElement const &) = default;
    ApiRegisterElement & operator = (T & x) { (*this->l)[this->i] = x; return *this; }

    bool operator == (T const & x) const { return &this->get() == &x; }
    bool operator != (T const & x) const { return !(this == x); }

    T & get() { return (*this->l)[this->i]; }
    T const & get() const { return (*this->l)[this->i]; }

private:
    list_type * l = nullptr;
    std::size_t i = ~std::size_t{};
};



#include "capture/utils/graphic_capture_impl.hpp"
#include "capture/utils/wrm_capture_impl.hpp"
#include "capture/utils/kbd_capture_impl.hpp"
#include "capture/utils/image_capture_impl.hpp"
#include "capture/utils/capture_apis_impl.hpp"
#include "capture/utils/capture_impl.hpp"
#include "utils/apps/recording_progress.hpp"

namespace gdi {
    class GraphicApi;
    class CaptureApi;
    class CaptureProbeApi;
    class KbdInputApi;
    class ExternalCaptureApi;
    class UpdateConfigCaptureApi;
}

struct ApisRegister
{
    std::vector<std::reference_wrapper<gdi::GraphicApi>> * graphic_list;
    std::vector<std::reference_wrapper<gdi::CaptureApi>> * graphic_snapshot_list;
    std::vector<std::reference_wrapper<gdi::CaptureApi>> & capture_list;
    std::vector<std::reference_wrapper<gdi::KbdInputApi>> & kbd_input_list;
    std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> & capture_probe_list;
    std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> & external_capture_list;
    std::vector<std::reference_wrapper<gdi::UpdateConfigCaptureApi>> & update_config_capture_list;
};



class Capture final
: public gdi::GraphicBase<Capture>
, public gdi::CaptureApi
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
, public gdi::ExternalCaptureApi
, public gdi::UpdateConfigCaptureApi
{
    using Graphic = GraphicCaptureImpl;

    using Image = PngCapture;

    using ImageRT = PngCaptureRT;

    using Native = WrmCaptureImpl;

    using Kbd = KbdCaptureImpl;

    using Video = SequencedVideoCaptureImpl;

    using FullVideo = FullVideoCaptureImpl;

    using Meta = MetaCaptureImpl;

    using Title = TitleCaptureImpl;

    const bool is_replay_mod;

    // Title changed
    //@{
    using string_view = array_view_const_char;

    struct TitleChangedFunctions final : NotifyTitleChanged
    {
        Capture & capture;

        TitleChangedFunctions(Capture & capture) : capture(capture) {}

        void notify_title_changed(timeval const & now, string_view title) override
        {
            if (this->capture.patterns_checker) {
                this->capture.patterns_checker->operator()(title);
            }
            if (this->capture.pmc) {
                this->capture.pmc->get_session_meta().title_changed(now.tv_sec, title);
            }
            if (this->capture.pvc) {
                this->capture.pvc->next_video(now);
            }
            if (this->capture.update_progress_data) {
                this->capture.update_progress_data->next_video(now.tv_sec);
            }
        }

        bool has_notifier()
        {
            return this->capture.patterns_checker || this->capture.pmc || this->capture.pvc;
        }
    } notifier_title_changed{*this};
    //@}

    // Next video
    //@{
    struct NotifyMetaIfNextVideo final : NotifyNextVideo
    {
        SessionMeta * session_meta = nullptr;

        void notify_next_video(const timeval& now, NotifyNextVideo::reason reason) override
        {
            assert(this->session_meta);
            if (reason == NotifyNextVideo::reason::sequenced) {
                this->session_meta->send_line(now.tv_sec, cstr_array_view("(break)"));
            }
        }
    } notifier_next_video;
    struct NullNotifyNextVideo final : NotifyNextVideo
    {
        void notify_next_video(const timeval&, NotifyNextVideo::reason) override {}
    } null_notifier_next_video;
    //@}

// TODO
public:
    const bool capture_wrm;
    const bool capture_png;
    const bool capture_pattern_checker;
    const bool capture_ocr;
    const bool capture_flv;
    const bool capture_flv_full; // capturewab only
    const bool capture_meta; // capturewab only

private:
    std::unique_ptr<Graphic> gd;
    std::unique_ptr<Native> pnc;
    std::unique_ptr<Image> psc;
    std::unique_ptr<ImageRT> pscrt;
    std::unique_ptr<Kbd> pkc;
    std::unique_ptr<Video> pvc;
    std::unique_ptr<FullVideo> pvc_full;
    std::unique_ptr<Meta> pmc;
    std::unique_ptr<Title> ptc;
    std::unique_ptr<PatternsChecker> patterns_checker;

    UpdateProgressData * update_progress_data;

    CaptureApisImpl::Capture capture_api;
    CaptureApisImpl::KbdInput kbd_input_api;
    CaptureApisImpl::CaptureProbe capture_probe_api;
    CaptureApisImpl::ExternalCapture external_capture_api;
    CaptureApisImpl::UpdateConfigCapture update_config_capture_api;
    Graphic::GraphicApi * graphic_api = nullptr;

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
        const CaptureFlags capture_flags,
        const timeval & now,
        int width,
        int height,
        int order_bpp,
        int capture_bpp,
        const PngParams png_params,
        const FlvParams flv_params,
        bool real_time_image_capture,
        bool no_timestamp,
        auth_api * authentifier,
        const Inifile & ini,
        CryptoContext & cctx,
        Random & rnd,
        bool full_video,
        UpdateProgressData * update_progress_data,
        bool force_capture_png_if_enable)
    : is_replay_mod(!authentifier)
    , capture_wrm(bool(capture_flags & CaptureFlags::wrm))
    , capture_png(bool(capture_flags & CaptureFlags::png) && (!authentifier || png_params.png_limit > 0))
    , capture_pattern_checker(authentifier && (
        ::contains_ocr_pattern(ini.get<cfg::context::pattern_kill>().c_str())
     || ::contains_ocr_pattern(ini.get<cfg::context::pattern_notify>().c_str())))
    , capture_ocr(bool(capture_flags & CaptureFlags::ocr) || this->capture_pattern_checker)
    , capture_flv(bool(capture_flags & CaptureFlags::flv))
    // capture wab only
    , capture_flv_full(full_video)
    // capture wab only
    , capture_meta(this->capture_ocr)
    , update_progress_data(update_progress_data)
    , capture_api(now, width / 2, height / 2)
    {
        REDASSERT(authentifier ? order_bpp == capture_bpp : true);

//        FlvParams flv_params = flv_params_from_ini(width, height, ini);

        bool const enable_kbd
          = authentifier
          ? !bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)
          || ini.get<cfg::session_log::enable_session_log>()
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_kill>().c_str())
          || ::contains_kbd_pattern(ini.get<cfg::context::pattern_notify>().c_str())
          : false
        ;

        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable capture:  wrm=%d  png=%d  kbd=%d  flv=%d  flv_full=%d  pattern=%d  ocr=%d  meta=%d",
                this->capture_wrm ? 1 : 0,
                this->capture_png ? 1 : 0,
                enable_kbd ? 1 : 0,
                this->capture_flv ? 1 : 0,
                this->capture_flv_full ? 1 : 0,
                this->capture_pattern_checker ? 1 : 0,
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
                if (real_time_image_capture) {
                    this->pscrt.reset(new ImageRT(
                        now, authentifier, this->gd->impl(),
                        record_tmp_path, basename, groupid,
                        png_params
                    ));
                }
                else if (force_capture_png_if_enable) {
                    this->psc.reset(new Image(
                        now, authentifier, this->gd->impl(),
                        record_tmp_path, basename, groupid,
                        png_params));
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
                    groupid, authentifier, this->gd->rdp_drawable(), ini
                ));
            }

            if (this->capture_ocr) {
                this->pmc.reset(new Meta(
                    now, record_tmp_path, basename,
                    authentifier && ini.get<cfg::session_log::enable_session_log>()
                ));
            }
            
            if (this->capture_flv) {
                std::reference_wrapper<NotifyNextVideo> notifier = this->null_notifier_next_video;
                if (ini.get<cfg::globals::capture_chunk>() && this->pmc) {
                    this->notifier_next_video.session_meta = &this->pmc->get_session_meta();
                    notifier = this->notifier_next_video;
                }
                this->pvc.reset(new Video(
                    now, record_path, basename, groupid, no_timestamp, png_params.zoom, this->gd->impl(),
                    flv_params,
                    ini.get<cfg::video::flv_break_interval>(), notifier
                ));
            }

            if (this->capture_flv_full) {
                this->pvc_full.reset(new FullVideo(
                    now, record_path, basename, groupid, no_timestamp, this->gd->impl(),
                    flv_params));
            }

            if (this->capture_pattern_checker) {
                this->patterns_checker.reset(new PatternsChecker(
                    *authentifier,
                    ini.get<cfg::context::pattern_kill>().c_str(),
                    ini.get<cfg::context::pattern_notify>().c_str())
                );
                if (!this->patterns_checker->contains_pattern()) {
                    LOG(LOG_WARNING, "Disable pattern_checker");
                    this->patterns_checker.reset();
                }
            }

            if (this->capture_ocr) {
                if (this->notifier_title_changed.has_notifier()) {
                    this->ptc.reset(new Title(
                        now, authentifier, this->gd->impl(), ini,
                        this->notifier_title_changed
                    ));
                }
                else {
                    LOG(LOG_INFO, "Disable title_extractor");
                }
            }
        }

        // TODO this->pkc = Kbd::construct(now, authentifier, ini); ?
        if (enable_kbd) {
            this->pkc.reset(new Kbd(now, authentifier, ini));
        }
        ApisRegister apis_register = this->get_apis_register();

        if (this->gd ) {
            assert(apis_register.graphic_list);
            apis_register.graphic_list->push_back(this->gd->drawable);
        }
        if (this->pnc) {
            apis_register.graphic_list->push_back(this->pnc->graphic_to_file);
            apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this->pnc));
            apis_register.external_capture_list.push_back(this->pnc->nc);
            apis_register.capture_probe_list.push_back(this->pnc->graphic_to_file);

            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::wrm)) {
                this->pnc->kbd_element = {apis_register.kbd_input_list, this->pnc->graphic_to_file};
                this->pnc->graphic_to_file.impl = this->pnc.get();
            }
        }

        if (this->pscrt) {
            this->pscrt->enable_rt_display = ini.get<cfg::video::rt_display>();
            apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this->pscrt));
            apis_register.graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this->pscrt));
            apis_register.update_config_capture_list.push_back(static_cast<gdi::UpdateConfigCaptureApi&>(*this->pscrt));
        }

        if (this->psc) { 
            apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this->psc));
            apis_register.graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this->psc));
        }

        if (this->pkc) {
            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)) {
                apis_register.kbd_input_list.push_back(this->pkc->syslog_kbd);
                apis_register.capture_list.push_back(this->pkc->syslog_kbd);
            }

            if (this->pkc->authentifier && ini.get<cfg::session_log::enable_session_log>() &&
                (ini.get<cfg::session_log::keyboard_input_masking_level>()
                 != ::KeyboardInputMaskingLevel::fully_masked)
            ) {
                apis_register.kbd_input_list.push_back(this->pkc->session_log_kbd);
                apis_register.capture_probe_list.push_back(this->pkc->session_log_kbd);
            }

            if (this->pkc->pattern_kbd.contains_pattern()) {
                apis_register.kbd_input_list.push_back(this->pkc->pattern_kbd);
            }
        }

        if (this->pvc) { 
            apis_register.capture_list.push_back(this->pvc->vc);
            apis_register.graphic_snapshot_list->push_back(this->pvc->preparing_vc);
            this->pvc->first_image.cap_elem = {apis_register.capture_list, this->pvc->first_image};
            this->pvc->first_image.gcap_elem = {*apis_register.graphic_snapshot_list, this->pvc->first_image};
        }
        if (this->pvc_full) { 
            apis_register.capture_list.push_back(this->pvc_full->vc);
            apis_register.graphic_snapshot_list->push_back(this->pvc_full->preparing_vc);
        }
        if (this->pmc) {
            apis_register.capture_list.push_back(this->pmc->meta);
            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta)) {
                apis_register.kbd_input_list.push_back(this->pmc->meta);
                apis_register.capture_probe_list.push_back(this->pmc->meta);
            }

            if (this->pmc->enable_agent) {
                apis_register.capture_probe_list.push_back(this->pmc->session_log_agent);
            }
        }
        if (this->ptc) { 
            apis_register.capture_list.push_back(static_cast<gdi::CaptureApi&>(*this->ptc));
            apis_register.capture_probe_list.push_back(static_cast<gdi::CaptureProbeApi&>(*this->ptc));
        }

        if (this->gd) { this->gd->start(); }
    }

    ~Capture() {
        if (this->is_replay_mod) {
            this->psc.reset();
            if (this->pscrt) { this->pscrt.reset(); }
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
            if (this->pscrt) { this->pscrt.reset(); }

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

    bool kbd_input(timeval const & now, uint32_t uchar) override {
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
        timeval const & now,
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

    void external_time(timeval const & now) override {
        this->external_capture_api.external_time(now);
    }

    void session_update(const timeval & now, array_view_const_char message) override {
        this->capture_probe_api.session_update(now, message);
    }

    void possible_active_window_change() override {
        this->capture_probe_api.possible_active_window_change();
    }
};
