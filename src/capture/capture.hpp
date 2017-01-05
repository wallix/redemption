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
#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h> // localtime_r
#include <memory>
#include <ctime>
#include <cassert>
#include <chrono>
#include <utility>
#include <sys/time.h>

#include "utils/log.hpp"


#include "utils/sugar/array_view.hpp"
#include "utils/sugar/local_fd.hpp"

#include "utils/difftimeval.hpp"
#include "utils/drawable.hpp"
#include "utils/apps/recording_progress.hpp"
#include "utils/difftimeval.hpp"
#include "utils/dump_png24_from_rdp_drawable_adapter.hpp"
#include "utils/urandom_read.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"
#include "utils/sugar/range.hpp"

#include "transport/transport.hpp"

#include "configs/config.hpp"


#include "gdi/capture_api.hpp"

#include "capture/wrm_params.hpp"
#include "capture/png_params.hpp"
#include "capture/flv_params.hpp"
#include "capture/ocr_params.hpp"
#include "utils/pattutils.hpp"

#include "video_recorder.hpp"
#include "transport/out_meta_sequence_transport.hpp"

#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "capture/GraphicToFile.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/dump_png24.hpp"

#include "capture/session_log_agent.hpp"
#include "capture/title_extractors/agent_title_extractor.hpp"
#include "capture/title_extractors/ocr_title_filter.hpp"
#include "capture/title_extractors/ocr_titles_extractor.hpp"
#include "capture/title_extractors/ppocr_titles_extractor.hpp"
#include "capture/title_extractors/ocr_title_extractor_builder.hpp"
#include "capture/utils/pattern_checker.hpp"
#include "capture/session_meta.hpp"

#include "utils/difftimeval.hpp"
#include "transport/transport.hpp"

#include "openssl_crypto.hpp"

#include "transport/out_file_transport.hpp"
#include "capture/cryptofile.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "gdi/capture_api.hpp"

#include "gdi/graphic_cmd_color_converter.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/kbd_input_api.hpp"

#include "utils/drawable.hpp"
#include "core/wait_obj.hpp"
#include "capture/new_kbdcapture.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"


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
//    bool operator != (T const & x) const { return !(this == x); }

    T & get() { return (*this->l)[this->i]; }
    T const & get() const { return (*this->l)[this->i]; }

private:
    list_type * l = nullptr;
    std::size_t i = ~std::size_t{};
};




namespace gdi {
    class GraphicApi;
    class CaptureApi;
    class CaptureProbeApi;
    class KbdInputApi;
    class ExternalCaptureApi;
    class UpdateConfigCaptureApi;
}


class KbdCaptureImpl
{
public:
    auth_api * authentifier;
    SyslogKbd syslog_kbd;
    SessionLogKbd session_log_kbd;
    PatternKbd pattern_kbd;

    KbdCaptureImpl(const timeval & now, auth_api * authentifier, const Inifile & ini)
    : authentifier(authentifier)
    , syslog_kbd(now)
    , session_log_kbd(*authentifier)
    , pattern_kbd(authentifier,
        ini.get<cfg::context::pattern_kill>().c_str(),
        ini.get<cfg::context::pattern_notify>().c_str(),
        ini.get<cfg::debug::capture>())
    {}
};

struct MouseTrace
{
    timeval last_now;
    int     last_x;
    int     last_y;
};

struct CaptureApisImpl
{
    struct Capture : gdi::CaptureApi
    {
        Capture(const timeval & now, int cursor_x, int cursor_y)
        : mouse_info{now, cursor_x, cursor_y}
        , capture_event{}
        {}

        void set_drawable(Drawable * drawable) {
            this->drawable = drawable;
        }

        MouseTrace const & mouse_trace() const noexcept {
            return this->mouse_info;
        }

        wait_obj & get_capture_event() {
            return this->capture_event;
        }

        std::vector<std::reference_wrapper<gdi::CaptureApi>> caps;

    private:
        std::chrono::microseconds do_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) override {
            this->capture_event.reset();

            if (this->drawable) {
                this->drawable->set_mouse_cursor_pos(cursor_x, cursor_y);
            }

            this->mouse_info = {now, cursor_x, cursor_y};

            std::chrono::microseconds time = std::chrono::microseconds::max();
            if (!this->caps.empty()) {
                for (gdi::CaptureApi & cap : this->caps) {
                    time = std::min(time, cap.snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval));
                }
                this->capture_event.update(time.count());
            }
            return time;
        }

        void do_pause_capture(const timeval& now) override {
            for (gdi::CaptureApi & cap : this->caps) {
                cap.pause_capture(now);
            }
            this->capture_event.reset();
        }

        void do_resume_capture(const timeval& now) override {
            for (gdi::CaptureApi & cap : this->caps) {
                cap.resume_capture(now);
            }
            this->capture_event.set();
        }

        Drawable * drawable = nullptr;
        MouseTrace mouse_info;
        wait_obj capture_event;
    };


    struct KbdInput : gdi::KbdInputApi
    {
        bool kbd_input(const timeval & now, uint32_t uchar) override {
            bool ret = true;
            for (gdi::KbdInputApi & kbd : this->kbds) {
                ret &= kbd.kbd_input(now, uchar);
            }
            return ret;
        }

        void enable_kbd_input_mask(bool enable) override {
            for (gdi::KbdInputApi & kbd : this->kbds) {
                kbd.enable_kbd_input_mask(enable);
            }
        }

        std::vector<std::reference_wrapper<gdi::KbdInputApi>> kbds;
    };


    struct CaptureProbe : gdi::CaptureProbeApi
    {
        void possible_active_window_change() override {
            for (gdi::CaptureProbeApi & cap_prob : this->probes) {
                cap_prob.possible_active_window_change();
            }
        }

        void session_update(const timeval& now, array_view_const_char message) override {
            for (gdi::CaptureProbeApi & cap_prob : this->probes) {
                cap_prob.session_update(now, message);
            }
        }

        std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> probes;
    };


    struct ExternalCapture : gdi::ExternalCaptureApi
    {
        void external_breakpoint() override {
            for (gdi::ExternalCaptureApi & obj : this->objs) {
                obj.external_breakpoint();
            }
        }

        void external_time(const timeval& now) override {
            for (gdi::ExternalCaptureApi & obj : this->objs) {
                obj.external_time(now);
            }
        }

        std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> objs;
    };


    struct UpdateConfigCapture : gdi::UpdateConfigCaptureApi
    {
        void update_config(const Inifile & ini) override {
            for (gdi::UpdateConfigCaptureApi & obj : this->objs) {
                obj.update_config(ini);
            }
        }

        std::vector<std::reference_wrapper<gdi::UpdateConfigCaptureApi>> objs;
    };
};

class GraphicCaptureImpl
{
public:
    using PtrColorConverter = std::unique_ptr<gdi::GraphicApi>;
    using GdRef = std::reference_wrapper<gdi::GraphicApi>;

    struct Graphic final : public gdi::GraphicApi
    {
    public:
        void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPDestBlt          const & cmd, Rect const & clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPMultiDstBlt      const & cmd, Rect const & clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPPatBlt           const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDP::RDPMultiPatBlt const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPOpaqueRect       const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPMultiOpaqueRect  const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPScrBlt           const & cmd, Rect const & clip) override { this->draw_impl(cmd, clip); }
        void draw(RDP::RDPMultiScrBlt const & cmd, Rect const & clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPLineTo           const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPPolygonSC        const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPPolygonCB        const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPPolyline         const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPEllipseSC        const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPEllipseCB        const & cmd, Rect const & clip, gdi::GraphicDepth depth) override { this->draw_impl(cmd, clip, depth); }
        void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
        void draw(RDPMemBlt           const & cmd, Rect const & clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
        void draw(RDPMem3Blt          const & cmd, Rect const & clip, gdi::GraphicDepth depth, Bitmap const & bmp) override { this->draw_impl(cmd, clip, depth, bmp); }
        void draw(RDPGlyphIndex       const & cmd, Rect const & clip, gdi::GraphicDepth depth, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, depth, gly_cache); }

        void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

        void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

        void set_pointer(Pointer    const & pointer) override {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.set_pointer(pointer);
            }
        }

        void set_palette(BGRPalette const & palette) override {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.set_palette(palette);
            }
        }

        void sync() override {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.sync();
            }
        }

        void set_row(std::size_t rownum, const uint8_t * data) override {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.set_row(rownum, data);
            }
        }

        void begin_update() override {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.begin_update();
            }
        }

        void end_update() override {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.end_update();
            }
        }

    protected:
        template<class... Ts>
        void draw_impl(Ts const & ... args) {
            for (gdi::GraphicApi & gd : this->gds){ 
                gd.draw(args...);
            }
        }

    public:
        PtrColorConverter cmd_color_distributor;
        MouseTrace const & mouse;
        std::vector<GdRef> gds;
        std::vector<std::reference_wrapper<gdi::CaptureApi>> snapshoters;

        gdi::GraphicDepth order_depth_ = gdi::GraphicDepth::unspecified();
        gdi::RngByBpp<std::vector<GdRef>::iterator> rng_by_bpp;

        Graphic(MouseTrace const & mouse)
        : mouse(mouse)
        {}

        template<class Cmd, class... Ts>
        void draw_impl(Cmd const & cmd, Ts const & ... args)
        {
            if (gdi::GraphicCmdColor::is_encodable_cmd_color(cmd)) {
                assert(gdi::GraphicDepth::unspecified() != this->order_depth_);
                gdi::draw_cmd_color_convert(this->order_depth_, this->rng_by_bpp, cmd, args...);
            }
            else {
                for (gdi::GraphicApi & gd : this->gds){ 
                    gd.draw(cmd, args...);
                }
            }
        }

        void draw_impl(RDP::FrameMarker const & cmd) {
            for (gdi::GraphicApi & gd : this->gds) {
                gd.draw(cmd);
            }

            if (cmd.action == RDP::FrameMarker::FrameEnd) {
                for (gdi::CaptureApi & cap : this->snapshoters) {
                    cap.snapshot(
                        this->mouse.last_now,
                        this->mouse.last_x,
                        this->mouse.last_y,
                        false
                    );
                }
            }
        }

        void set_depths(gdi::GraphicDepth const & depth) override {
            this->order_depth_ = depth;
        }

        gdi::GraphicDepth const & order_depth() const override {
            return this->order_depth_;
        }

    };

    Graphic graphic_api;
    RDPDrawable drawable;
    uint8_t order_bpp;

public:
    using GraphicApi = Graphic;

    GraphicCaptureImpl(uint16_t width, uint16_t height, uint8_t order_bpp, MouseTrace const & mouse)
    : graphic_api(mouse)
    , drawable(width, height, order_bpp)
    , order_bpp(order_bpp)
    {
    }

    void update_order_bpp(uint8_t order_bpp) {
        if (this->order_bpp != order_bpp) {
            this->order_bpp = order_bpp;
            this->drawable.set_depths(gdi::GraphicDepth::from_bpp(order_bpp));
            this->start();
        }
    }

    void start()
    {
        auto const order_depth = gdi::GraphicDepth::from_bpp(this->order_bpp);
        auto & gds = this->graphic_api.gds;
        this->graphic_api.rng_by_bpp = {order_depth, gds.begin(), gds.end()};
        this->graphic_api.order_depth_ = order_depth;
    }

    Graphic & get_graphic_api() { return this->graphic_api; }

    Drawable & impl() { return this->drawable.impl(); }
    RDPDrawable & rdp_drawable() { return this->drawable; }
};


class PngCapture : public gdi::UpdateConfigCaptureApi, public gdi::CaptureApi
{
public:
    OutFilenameSequenceTransport trans;
    timeval start_capture;
    std::chrono::microseconds frame_interval;

    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    Drawable & drawable;
    std::unique_ptr<uint8_t[]> scaled_buffer;

    PngCapture(
        const timeval & now, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const PngParams & png_params)
    : trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_tmp_path, basename, ".png", groupid, authentifier)
    , start_capture(now)
    , frame_interval(png_params.png_interval)
    , zoom_factor(png_params.zoom)
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    {}

private:
    void update_config(Inifile const & ini) override {
    }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        (void)x;
        (void)y;
        (void)ignore_frame_in_timeval;
        using std::chrono::microseconds;
        uint64_t const duration = difftimeval(now, this->start_capture);
        uint64_t const interval = this->frame_interval.count();
        if (duration >= interval) {
            if (this->drawable.logical_frame_ended
                // Force snapshot if diff_time_val >= 1.5 x frame_interval.
                || (duration >= interval * 3 / 2)) {
                this->drawable.trace_mouse();
                tm ptm;
                localtime_r(&now.tv_sec, &ptm);
                this->drawable.trace_timestamp(ptm);
                if (this->zoom_factor == 100) {
                    // TODO we should have a variant of ::transport_dump_png24
                    // taking a Drawable as input
                    ::transport_dump_png24(
                        this->trans, this->drawable.data(),
                        this->drawable.width(), this->drawable.height(),
                        this->drawable.rowsize(), true);
                }
                else {
                    // TODO all the zoom thing could be hidden behind a
                    // special type of Drawable
                    scale_data(
                        this->scaled_buffer.get(), this->drawable.data(),
                        this->scaled_width, this->drawable.width(),
                        this->scaled_height, this->drawable.height(),
                        this->drawable.rowsize());
                    ::transport_dump_png24(
                        this->trans, this->scaled_buffer.get(),
                        this->scaled_width, this->scaled_height,
                        this->scaled_width * 3, false);
                }
                // TODO: showing hiding mouse/timestamp should be hidden
                // behind a special type of Drawable
                this->trans.next();
                this->drawable.clear_timestamp();
                this->start_capture = now;
                this->drawable.clear_mouse();

                return microseconds(interval ? interval - duration % interval : 0u);
            }
            else {
                // Wait 0.3 x frame_interval.
                return this->frame_interval / 3;
            }
        }
        return microseconds(interval - duration);
    }

    void do_pause_capture(timeval const & now) override {
        // Draw Pause message
        time_t rawtime = now.tv_sec;
        tm ptm;
        localtime_r(&rawtime, &ptm);
        this->drawable.trace_pausetimestamp(ptm);
        if (this->zoom_factor == 100) {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }
        else {
            scale_data(
                this->scaled_buffer.get(), this->drawable.data(),
                this->scaled_width, this->drawable.width(),
                this->scaled_height, this->drawable.height(),
                this->drawable.rowsize());
            ::transport_dump_png24(
                this->trans, this->scaled_buffer.get(),
                this->scaled_width, this->scaled_height,
                this->scaled_width * 3, false);
        }
        this->trans.next();
        this->drawable.clear_pausetimestamp();
        this->start_capture = now;
    }

};

class PngCaptureRT : public gdi::UpdateConfigCaptureApi, public gdi::CaptureApi
{
public:
    OutFilenameSequenceTransport trans;
    uint32_t num_start = 0;
    unsigned png_limit;

    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    Drawable & drawable;

    std::unique_ptr<uint8_t[]> scaled_buffer;

    timeval start_capture;
    std::chrono::microseconds frame_interval;

    bool enable_rt_display = false;

    PngCaptureRT(
        const timeval & now, auth_api * authentifier, Drawable & drawable,
        const char * record_tmp_path, const char * basename, int groupid,
        const PngParams & png_params)
    : trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        record_tmp_path, basename, ".png", groupid, authentifier)
    , png_limit(png_params.png_limit)
    , zoom_factor(std::min(png_params.zoom, 100u))
    , scaled_width(drawable.width())
    , scaled_height(drawable.height())
    , drawable(drawable)
    , start_capture(now)
    , frame_interval(png_params.png_interval)
    {
        const unsigned zoom_width = (this->drawable.width() * this->zoom_factor) / 100;
        const unsigned zoom_height = (this->drawable.height() * this->zoom_factor) / 100;
        this->scaled_width = (zoom_width + 3) & 0xFFC;
        this->scaled_height = zoom_height;
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    ~PngCaptureRT()
    {
        for(uint32_t until_num = this->trans.get_seqno() + 1; this->num_start < until_num; ++this->num_start){
            // unlink may fail, for instance if file does not exist, just don't care
            ::unlink(this->trans.seqgen()->get(this->num_start));
        }
    }

private:
    void update_config(Inifile const & ini) override {
        auto const old_enable_rt_display = this->enable_rt_display;
        this->enable_rt_display = ini.get<cfg::video::rt_display>();

        if (old_enable_rt_display == this->enable_rt_display) {
            return ;
        }

        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable real time: %d", int(this->enable_rt_display));
        }

        if (!this->enable_rt_display) {
            for(uint32_t until_num = this->trans.get_seqno() + 1; this->num_start < until_num; ++this->num_start){
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->trans.seqgen()->get(this->num_start));
            }
        }
    }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        if (this->enable_rt_display) {
            (void)x;
            (void)y;
            (void)ignore_frame_in_timeval;
            using std::chrono::microseconds;
            uint64_t const duration = difftimeval(now, this->start_capture);
            uint64_t const interval = this->frame_interval.count();
            if (duration >= interval) {
                if (this->drawable.logical_frame_ended
                    // Force snapshot if diff_time_val >= 1.5 x frame_interval.
                    || (duration >= interval * 3 / 2)) {
                    this->drawable.trace_mouse();
                    tm ptm;
                    localtime_r(&now.tv_sec, &ptm);
                    this->drawable.trace_timestamp(ptm);
                    if (this->zoom_factor == 100) {
                        ::transport_dump_png24(
                            this->trans, this->drawable.data(),
                            this->drawable.width(), this->drawable.height(),
                            this->drawable.rowsize(), true);
                    }
                    else {
                        scale_data(
                            this->scaled_buffer.get(), this->drawable.data(),
                            this->scaled_width, this->drawable.width(),
                            this->scaled_height, this->drawable.height(),
                            this->drawable.rowsize());
                        ::transport_dump_png24(
                            this->trans, this->scaled_buffer.get(),
                            this->scaled_width, this->scaled_height,
                            this->scaled_width * 3, false);
                    }
                    if (this->png_limit && this->trans.get_seqno() >= this->png_limit) {
                        // unlink may fail, for instance if file does not exist, just don't care
                        ::unlink(this->trans.seqgen()->get(this->trans.get_seqno() - this->png_limit));
                    }
                    this->trans.next();
                    this->drawable.clear_timestamp();
                    this->start_capture = now;
                    this->drawable.clear_mouse();

                    return microseconds(interval ? interval - duration % interval : 0u);
                }
                else {
                    // Wait 0.3 x frame_interval.
                    return this->frame_interval / 3;
                }
            }
            return microseconds(interval - duration);
        }
        return this->frame_interval;
    }

    void do_pause_capture(timeval const & now) override {
        if (this->enable_rt_display) {
            // Draw Pause message
            time_t rawtime = now.tv_sec;
            tm ptm;
            localtime_r(&rawtime, &ptm);
            this->drawable.trace_pausetimestamp(ptm);
            if (this->zoom_factor == 100) {
                ::transport_dump_png24(
                    this->trans, this->drawable.data(),
                    this->drawable.width(), this->drawable.height(),
                    this->drawable.rowsize(), true);
            }
            else {
                scale_data(
                    this->scaled_buffer.get(), this->drawable.data(),
                    this->scaled_width, this->drawable.width(),
                    this->scaled_height, this->drawable.height(),
                    this->drawable.rowsize());
                ::transport_dump_png24(
                    this->trans, this->scaled_buffer.get(),
                    this->scaled_width, this->scaled_height,
                    this->scaled_width * 3, false);
            }
            if (this->png_limit && this->trans.get_seqno() >= this->png_limit) {
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->trans.seqgen()->get(this->trans.get_seqno() - this->png_limit));
            }
            this->trans.next();
            this->drawable.clear_pausetimestamp();
            this->start_capture = now;
        }
    }

    void do_resume_capture(timeval const & now) override {
    }
};


class VideoCapture : public gdi::CaptureApi
{
    Transport & trans;

    FlvParams flv_params;

    const Drawable & drawable;
    std::unique_ptr<video_recorder> recorder;

    timeval start_video_capture;
    std::chrono::microseconds inter_frame_interval;
    bool no_timestamp;

public:
    VideoCapture(
        const timeval & now,
        Transport & trans,
        const Drawable & drawable,
        bool no_timestamp,
        FlvParams flv_params)
    : trans(trans)
    , flv_params(std::move(flv_params))
    , drawable(drawable)
    , start_video_capture(now)
    , inter_frame_interval(1000000L / this->flv_params.frame_rate)
    , no_timestamp(no_timestamp)
    {
        if (this->flv_params.verbosity) {
            LOG(LOG_INFO, "Video recording %d x %d, rate: %d, qscale: %d, brate: %d, codec: %s",
                this->flv_params.target_width, this->flv_params.target_height,
                this->flv_params.frame_rate, this->flv_params.qscale, this->flv_params.bitrate,
                this->flv_params.codec.c_str());
        }

        this->next_video();
    }

    void next_video() {
        if (this->recorder) {
            this->recorder.reset();
            this->trans.next();
        }

        io_video_recorder_with_transport io{this->trans};
        this->recorder.reset(new video_recorder(
            io.write_fn(), io.seek_fn(), io.params(),
            drawable.width(), drawable.height(),
            drawable.pix_len(),
            drawable.data(),
            this->flv_params.bitrate,
            this->flv_params.frame_rate,
            this->flv_params.qscale,
            this->flv_params.codec.c_str(),
            this->flv_params.target_width,
            this->flv_params.target_height,
            this->flv_params.verbosity
        ));
    }

    void preparing_video_frame() {
        auto & drawable = const_cast<Drawable&>(this->drawable);
        drawable.trace_mouse();
        if (!this->no_timestamp) {
            time_t rawtime = this->start_video_capture.tv_sec;
            tm tm_result;
            localtime_r(&rawtime, &tm_result);
            drawable.trace_timestamp(tm_result);
        }
        this->recorder->preparing_video_frame(true);
        if (!this->no_timestamp) { drawable.clear_timestamp(); }
        drawable.clear_mouse();
    }

    void encoding_video_frame() {
        this->recorder->encoding_video_frame();
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
    ) override {
        uint64_t tick = difftimeval(now, this->start_video_capture);
        uint64_t const inter_frame_interval = this->inter_frame_interval.count();
        if (tick >= inter_frame_interval) {
            auto encoding_video_frame = [this](time_t rawtime){
                auto & drawable = const_cast<Drawable&>(this->drawable);
                drawable.trace_mouse();
                if (!this->no_timestamp) {
                    tm tm_result;
                    localtime_r(&rawtime, &tm_result);
                    drawable.trace_timestamp(tm_result);
                    this->recorder->encoding_video_frame();
                    drawable.clear_timestamp();
                }
                else {
                    this->recorder->encoding_video_frame();
                }
                drawable.clear_mouse();
            };

            if (ignore_frame_in_timeval) {
                auto const nframe = tick / inter_frame_interval;
                encoding_video_frame(this->start_video_capture.tv_sec);
                auto const usec = inter_frame_interval * nframe;
                auto sec = usec / 1000000LL;
                this->start_video_capture.tv_usec += usec - sec * inter_frame_interval;
                if (this->start_video_capture.tv_usec >= 1000000LL){
                    this->start_video_capture.tv_usec -= 1000000LL;
                    ++sec;
                }
                this->start_video_capture.tv_sec += sec;
                tick -= inter_frame_interval * nframe;
            }
            else {
                do {
                    encoding_video_frame(this->start_video_capture.tv_sec);
                    this->start_video_capture.tv_usec += inter_frame_interval;
                    if (this->start_video_capture.tv_usec >= 1000000LL){
                        this->start_video_capture.tv_sec += 1;
                        this->start_video_capture.tv_usec -= 1000000LL;
                    }
                    tick -= inter_frame_interval;
                } while (tick >= inter_frame_interval);
            }
        }

        return std::chrono::microseconds(inter_frame_interval - tick);
    }
};

struct PreparingWhenFrameMarkerEnd final : gdi::CaptureApi
{
    PreparingWhenFrameMarkerEnd(VideoCapture & vc)
    : vc(vc)
    {}

private:
    VideoCapture & vc;

    std::chrono::microseconds do_snapshot(
        const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        vc.preparing_video_frame();
        return std::chrono::microseconds{};
    }
};

class FullVideoCaptureImpl
{
    OutFilenameSequenceSeekableTransport trans;
public:
    VideoCapture vc;
    PreparingWhenFrameMarkerEnd preparing_vc{vc};

    FullVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        const Drawable & drawable,
        FlvParams flv_params)
    : trans(
        FilenameGenerator::PATH_FILE_EXTENSION,
        record_path, basename, ("." + flv_params.codec).c_str(), groupid)
    , vc(now, this->trans, drawable, no_timestamp, std::move(flv_params))
    {
        ::unlink((std::string(record_path) + basename + "." + flv_params.codec).c_str());
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->trans.request_full_cleaning();
    }
};

struct NotifyNextVideo : private noncopyable
{
    enum class reason { sequenced, external };
    virtual void notify_next_video(const timeval& now, reason) = 0;
    virtual ~NotifyNextVideo() = default;
};

class SequencedVideoCaptureImpl
{
    class VideoSequencer : public gdi::CaptureApi
    {
        timeval start_break;
        std::chrono::microseconds break_interval;

    protected:
        SequencedVideoCaptureImpl & impl;

    public:
        VideoSequencer(const timeval & now, std::chrono::microseconds break_interval, SequencedVideoCaptureImpl & impl)
        : start_break(now)
        , break_interval(break_interval)
        , impl(impl)
        {}

        std::chrono::microseconds get_interval() const {
            return this->break_interval;
        }

        void reset_now(const timeval& now) {
            this->start_break = now;
        }

    private:
        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
        ) override {
            assert(this->break_interval.count());
            auto const interval = difftimeval(now, this->start_break);
            if (interval >= uint64_t(this->break_interval.count())) {
                this->impl.next_video_impl(now, NotifyNextVideo::reason::sequenced);
                this->start_break = now;
            }
            return this->break_interval;
        }
    };

    class VideoTransport final : public OutFilenameSequenceSeekableTransport
    {
        using transport_base = OutFilenameSequenceSeekableTransport;

    public:
        VideoTransport(
            const char * const record_path,
            const char * const basename,
            const char * const suffix,
            const int groupid
        )
        : transport_base(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_path, basename, suffix, groupid)
        {
            this->remove_current_path();
        }

        bool next() override {
            if (transport_base::next()) {
                this->remove_current_path();
                return true;
            }
            return false;
        }

    private:
        void remove_current_path() {
            const char * const path = this->seqgen()->get(this->get_seqno());
            ::unlink(path);
        }
    };


    struct ImageToFile
    {
        Transport & trans;
        unsigned zoom_factor;
        unsigned scaled_width;
        unsigned scaled_height;

        const Drawable & drawable;

    private:
        std::unique_ptr<uint8_t[]> scaled_buffer;

    public:
        ImageToFile(Transport & trans, const Drawable & drawable, unsigned zoom)
        : trans(trans)
        , zoom_factor(std::min(zoom, 100u))
        , scaled_width(drawable.width())
        , scaled_height(drawable.height())
        , drawable(drawable)
        {
            const unsigned zoom_width = (this->drawable.width() * this->zoom_factor) / 100;
            const unsigned zoom_height = (this->drawable.height() * this->zoom_factor) / 100;
            this->scaled_width = (zoom_width + 3) & 0xFFC;
            this->scaled_height = zoom_height;
            if (this->zoom_factor != 100) {
                this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
            }
        }

        ~ImageToFile() = default;

        /// \param  percent  0 to 100 or 100 if greater
        void zoom(unsigned percent) {
            percent = std::min(percent, 100u);
            const unsigned zoom_width = (this->drawable.width() * percent) / 100;
            const unsigned zoom_height = (this->drawable.height() * percent) / 100;
            this->zoom_factor = percent;
            this->scaled_width = (zoom_width + 3) & 0xFFC;
            this->scaled_height = zoom_height;
            if (this->zoom_factor != 100) {
                this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
            }
        }

        void flush() {
            if (this->zoom_factor == 100) {
                this->dump24();
            }
            else {
                this->scale_dump24();
            }
        }

        void dump24() const {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }

        void scale_dump24() const {
            scale_data(
                this->scaled_buffer.get(), this->drawable.data(),
                this->scaled_width, this->drawable.width(),
                this->scaled_height, this->drawable.height(),
                this->drawable.rowsize());
            ::transport_dump_png24(
                this->trans, this->scaled_buffer.get(),
                this->scaled_width, this->scaled_height,
                this->scaled_width * 3, false);
        }

        bool has_first_img = false;

        void breakpoint_image(const timeval& now)
        {
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            //const_cast<Drawable&>(this->drawable).trace_mouse();
            const_cast<Drawable&>(this->drawable).trace_timestamp(ptm);
            this->flush();
            const_cast<Drawable&>(this->drawable).clear_timestamp();
            //const_cast<Drawable&>(this->drawable).clear_mouse();
            this->has_first_img = true;
            this->trans.next();
        }
    };

    // first next_video is ignored
    struct FirstImage : gdi::CaptureApi
    {
        using capture_list_t = std::vector<std::reference_wrapper<gdi::CaptureApi>>;

        SequencedVideoCaptureImpl & impl;
        ApiRegisterElement<gdi::CaptureApi> cap_elem;
        ApiRegisterElement<gdi::CaptureApi> gcap_elem;

        using seconds = std::chrono::seconds;
        using microseconds = std::chrono::microseconds;

        const timeval start_capture;

        FirstImage(timeval const & now, SequencedVideoCaptureImpl & impl)
        : impl(impl)
        , start_capture(now)
        {}

        std::chrono::microseconds do_snapshot(
            const timeval& now, int x, int y, bool ignore_frame_in_timeval
        ) override {
            microseconds ret;

            auto const duration = microseconds(difftimeval(now, this->start_capture));
            auto const interval = microseconds(seconds(3))/2;
            if (duration >= interval) {
                auto video_interval = this->impl.video_sequencer.get_interval();
                if (this->impl.ic.drawable.logical_frame_ended || duration > seconds(2) || duration >= video_interval) {
                    this->impl.ic.breakpoint_image(now);
                    assert(this->cap_elem == *this);
                    assert(this->gcap_elem == *this);
                    this->cap_elem = this->impl.video_sequencer;
                    this->gcap_elem = this->impl.video_sequencer;

                    ret = video_interval;
                }
                else {
                    ret = interval / 3;
                }
            }
            else {
                ret = interval - duration;
            }

            return std::min(ret, this->impl.video_sequencer.snapshot(now, x, y, ignore_frame_in_timeval));
        }

        void do_resume_capture(const timeval& now) override { this->impl.video_sequencer.resume_capture(now); }
        void do_pause_capture(const timeval& now) override { this->impl.video_sequencer.pause_capture(now); }
    };

public:
    VideoTransport vc_trans;
    VideoCapture vc;
    PreparingWhenFrameMarkerEnd preparing_vc{vc};

    OutFilenameSequenceTransport ic_trans;
    ImageToFile ic;

    VideoSequencer video_sequencer;
    FirstImage first_image;

    NotifyNextVideo & next_video_notifier;

    void next_video_impl(const timeval& now, NotifyNextVideo::reason reason) {
        this->video_sequencer.reset_now(now);
        if (!this->ic.has_first_img) {
            this->ic.breakpoint_image(now);
            this->first_image.cap_elem = this->video_sequencer;
            this->first_image.gcap_elem = this->video_sequencer;
        }
        this->vc.next_video();
        this->ic.breakpoint_image(now);
        this->next_video_notifier.notify_next_video(now, reason);
    }

public:
    SequencedVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        unsigned image_zoom,
        const Drawable & drawable,
        FlvParams flv_params,
        std::chrono::microseconds video_interval,
        NotifyNextVideo & next_video_notifier)
    : vc_trans(record_path, basename, ("." + flv_params.codec).c_str(), groupid)
    , vc(now, this->vc_trans, drawable, no_timestamp, std::move(flv_params))
    , ic_trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_path, basename, ".png", groupid)
    , ic(this->ic_trans, drawable, image_zoom)
    , video_sequencer(
        now, video_interval > std::chrono::microseconds(0) ? video_interval : std::chrono::microseconds::max(), *this)
    , first_image(now, *this)
    , next_video_notifier(next_video_notifier)
    {}

    void next_video(const timeval& now) {
        this->next_video_impl(now, NotifyNextVideo::reason::external);
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->vc_trans.request_full_cleaning();
        this->ic_trans.request_full_cleaning();
    }
};


class MetaCaptureImpl
{
public:
    local_fd fd;
    OutFileTransport meta_trans;
    SessionMeta meta;
    SessionLogAgent session_log_agent;
    bool enable_agent;

    MetaCaptureImpl(
        const timeval & now,
        std::string record_path,
        const char * const basename,
        bool enable_agent)
    : fd([](const char * filename){
        int fd = ::open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0440);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        return fd;
    }(record_path.append(basename).append(".meta").c_str()))
    , meta_trans(this->fd.get())
    , meta(now, this->meta_trans)
    , session_log_agent(this->meta)
    , enable_agent(enable_agent)
    {
    }

    SessionMeta & get_session_meta() {
        return this->meta;
    }

    void request_full_cleaning() {
        this->meta_trans.request_full_cleaning();
    }
};


struct NotifyTitleChanged : private noncopyable
{
    virtual void notify_title_changed(const timeval & now, array_view_const_char title) = 0;
    virtual ~NotifyTitleChanged() = default;
};

class TitleCaptureImpl : public gdi::CaptureApi, public gdi::CaptureProbeApi
{
public:
    OcrTitleExtractorBuilder ocr_title_extractor_builder;
    AgentTitleExtractor agent_title_extractor;

    std::reference_wrapper<TitleExtractorApi> title_extractor;

    timeval  last_ocr;
    std::chrono::microseconds usec_ocr_interval;

    NotifyTitleChanged & notify_title_changed;

    TitleCaptureImpl(
        const timeval & now,
        auth_api * authentifier,
        const Drawable & drawable,
        const Inifile & ini,
        NotifyTitleChanged & notify_title_changed)
    : ocr_title_extractor_builder(
        drawable, authentifier != nullptr,
        ini.get<cfg::ocr::version>(),
        static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
        ini.get<cfg::ocr::on_title_bar_only>(),
        ini.get<cfg::ocr::max_unrecog_char_rate>())
    , title_extractor(this->ocr_title_extractor_builder.get_title_extractor())
    , last_ocr(now)
    , usec_ocr_interval(ini.get<cfg::ocr::interval>())
    , notify_title_changed(notify_title_changed)
    {
    }


    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const diff {difftimeval(now, this->last_ocr)};

        using std::chrono::milliseconds;
        using std::chrono::duration_cast;

        if (diff >= this->usec_ocr_interval) {
            this->last_ocr = now;

            auto title = this->title_extractor.get().extract_title();

            if (title.data()/* && title.size()*/) {
                notify_title_changed.notify_title_changed(now, title);
            }

            return this->usec_ocr_interval;
        }
        else {
            return this->usec_ocr_interval - diff;
        }
    }

    void session_update(timeval const & /*now*/, array_view_const_char message) override {
        bool const enable_probe = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
        if (enable_probe) {
            this->title_extractor = this->agent_title_extractor;
        }
        else {
            this->title_extractor = this->ocr_title_extractor_builder.get_title_extractor();
        }

        this->agent_title_extractor.session_update(message);
    }

    void possible_active_window_change() override {}
};

class WrmCaptureImpl : public gdi::KbdInputApi, public gdi::CaptureApi
{
public:
    BmpCache     bmp_cache;
    GlyphCache   gly_cache;
    PointerCache ptr_cache;

    DumpPng24FromRDPDrawableAdapter dump_png24_api;

    struct TransportVariant
    {
        union Variant
        {
            OutMetaSequenceTransportWithSum out_with_sum;
            CryptoOutMetaSequenceTransport out_crypto;
            OutMetaSequenceTransport out;

            struct {} dummy;
            Variant() : dummy() {}
            ~Variant() {}
        } variant;
        ::Transport * trans;

        template<class... Ts>
        TransportVariant(
            TraceType trace_type,
            CryptoContext & cctx,
            Random & rnd,
            const char * path,
            const char * hash_path,
            const char * basename,
            timeval const & now,
            uint16_t width,
            uint16_t height,
            const int groupid,
            auth_api * authentifier
        ) {
            // TODO there should only be one outmeta, not two. Capture code should not really care if file is encrypted or not. Here is not the right level to manage anything related to encryption.
            // TODO Also we may wonder why we are encrypting wrm and not png (This is related to the path split between png and wrm). We should stop and consider what we should actually do
            switch (trace_type) {
                case TraceType::cryptofile:
                    this->trans = new (&this->variant.out_crypto)
                    CryptoOutMetaSequenceTransport(
                        cctx, rnd, path, hash_path, basename, now,
                        width, height, groupid, authentifier);
                    break;
                case TraceType::localfile_hashed:
                    this->trans = new (&this->variant.out_with_sum)
                    OutMetaSequenceTransportWithSum(
                        cctx, path, hash_path, basename, now,
                        width, height, groupid, authentifier);
                    break;
                default:
                case TraceType::localfile:
                    this->trans = new (&this->variant.out)
                    OutMetaSequenceTransport(
                        path, hash_path, basename, now,
                        width, height, groupid, authentifier);
                    break;
            }
        }

        TransportVariant & operator = (TransportVariant const &) = delete;

        ~TransportVariant() {
            this->trans->~Transport();
        }
    } trans_variant;


    struct Serializer final : GraphicToFile {
        Serializer(const timeval & now
                , Transport & trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  capture_bpp
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::DumpPng24Api & dump_png24
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input = SendInput::NO
                , Verbose verbose = Verbose::none)
            : GraphicToFile(now, trans, width, height,
                            capture_bpp,
                            bmp_cache, gly_cache, ptr_cache,
                            dump_png24, wrm_compression_algorithm,
                            send_input, verbose)
            , order_depth_(gdi::GraphicDepth::unspecified())
        {}

        using GraphicToFile::GraphicToFile::draw;
        using GraphicToFile::GraphicToFile::capture_bpp;

        void set_depths(gdi::GraphicDepth const & depth) override {
            this->order_depth_ = depth;
        }

        gdi::GraphicDepth const & order_depth() const override {
            return this->order_depth_;
        }


        void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
            auto compress_and_draw_bitmap_update = [&bitmap_data, this](const Bitmap & bmp) {
                StaticOutStream<65535> bmp_stream;
                bmp.compress(this->capture_bpp, bmp_stream);

                RDPBitmapData target_bitmap_data = bitmap_data;

                target_bitmap_data.bits_per_pixel = bmp.bpp();
                target_bitmap_data.flags          = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;
                target_bitmap_data.bitmap_length  = bmp_stream.get_offset();

                GraphicToFile::draw(target_bitmap_data, bmp);
            };

            if (bmp.bpp() > this->capture_bpp) {
                // reducing the color depth of image.
                Bitmap capture_bmp(this->capture_bpp, bmp);
                compress_and_draw_bitmap_update(capture_bmp);
            }
            else if (!bmp.has_data_compressed()) {
                compress_and_draw_bitmap_update(bmp);
            }
            else {
                GraphicToFile::draw(bitmap_data, bmp);
            }
        }

        WrmCaptureImpl * impl = nullptr;
        void enable_kbd_input_mask(bool enable) override {
            this->impl->enable_kbd_input_mask(enable);
        }
        gdi::GraphicDepth order_depth_ = gdi::GraphicDepth::unspecified();
    } graphic_to_file;

    class NativeCaptureLocal : public gdi::CaptureApi, public gdi::ExternalCaptureApi
    {
        timeval start_native_capture;
        uint64_t inter_frame_interval_native_capture;

        timeval start_break_capture;
        uint64_t inter_frame_interval_start_break_capture;

        GraphicToFile & recorder;
        uint64_t time_to_wait;

    public:
        NativeCaptureLocal(
            GraphicToFile & recorder,
            const timeval & now,
            std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
            std::chrono::seconds break_interval
        )
        : start_native_capture(now)
        , inter_frame_interval_native_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(frame_interval).count())
        , start_break_capture(now)
        , inter_frame_interval_start_break_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(break_interval).count())
        , recorder(recorder)
        , time_to_wait(0)
        {}

        ~NativeCaptureLocal() override {
            this->recorder.sync();
        }

        // toggles externally genareted breakpoint.
        void external_breakpoint() override {
            this->recorder.breakpoint();
        }

        void external_time(const timeval & now) override {
            this->recorder.sync();
            this->recorder.timestamp(now);
        }

    private:
        std::chrono::microseconds do_snapshot(
            const timeval & now, int x, int y, bool ignore_frame_in_timeval
        ) override {
            (void)ignore_frame_in_timeval;
            if (difftimeval(now, this->start_native_capture)
                    >= this->inter_frame_interval_native_capture) {
                this->recorder.timestamp(now);
                this->time_to_wait = this->inter_frame_interval_native_capture;
                this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
                this->start_native_capture = now;
                if ((difftimeval(now, this->start_break_capture) >=
                     this->inter_frame_interval_start_break_capture)) {
                    this->recorder.breakpoint();
                    this->start_break_capture = now;
                }
            }
            else {
                this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
            }
            return std::chrono::microseconds{this->time_to_wait};
        }

    } nc;

//    template<>
    struct ApiRegisterElement_KBD
    {
        using list_type = std::vector<std::reference_wrapper<gdi::KbdInputApi>>;

        ApiRegisterElement_KBD() = default;

        ApiRegisterElement_KBD(list_type & l, gdi::KbdInputApi & x)
        : l(&l)
        , i(l.size())
        {
            l.push_back(x);
        }

        ApiRegisterElement_KBD & operator = (ApiRegisterElement_KBD const &) = default;
        ApiRegisterElement_KBD & operator = (gdi::KbdInputApi & x) { (*this->l)[this->i] = x; return *this; }

        bool operator == (gdi::KbdInputApi const & x) const { return &this->get() == &x; }
    //    bool operator != (gdi::KbdInputApi const & x) const { return !(this == x); }

        gdi::KbdInputApi & get() { return (*this->l)[this->i]; }
        gdi::KbdInputApi const & get() const { return (*this->l)[this->i]; }

    private:
        list_type * l = nullptr;
        std::size_t i = ~std::size_t{};
    } kbd_element;

public:
    WrmCaptureImpl(
        const timeval & now,
        const WrmParams wrm_params,
        uint8_t capture_bpp, TraceType trace_type,
        CryptoContext & cctx, Random & rnd,
        const char * record_path, const char * hash_path, const char * basename,
        int groupid, auth_api * authentifier,
        RDPDrawable & drawable, const Inifile & ini
    )
    : bmp_cache(
        BmpCache::Recorder, capture_bpp, 3, false,
        BmpCache::CacheOption(600, 768, false),
        BmpCache::CacheOption(300, 3072, false),
        BmpCache::CacheOption(262, 12288, false))
    , ptr_cache(/*pointerCacheSize=*/0x19)
    , dump_png24_api{drawable}
    , trans_variant(
        trace_type, cctx, rnd, record_path, hash_path, basename, now,
        drawable.width(), drawable.height(), groupid, authentifier)
    , graphic_to_file(
        now, *this->trans_variant.trans, drawable.width(), drawable.height(), capture_bpp,
        this->bmp_cache, this->gly_cache, this->ptr_cache, this->dump_png24_api,
        ini.get<cfg::video::wrm_compression_algorithm>(), GraphicToFile::SendInput::YES,
        to_verbose_flags(ini.get<cfg::debug::capture>())
        | (ini.get<cfg::debug::primary_orders>()
            ? GraphicToFile::Verbose::primary_orders   : GraphicToFile::Verbose::none)
        | (ini.get<cfg::debug::secondary_orders>()
            ? GraphicToFile::Verbose::secondary_orders : GraphicToFile::Verbose::none)
        | (ini.get<cfg::debug::bitmap_update>()
            ? GraphicToFile::Verbose::bitmap_update    : GraphicToFile::Verbose::none)
    )
    , nc(this->graphic_to_file, now, ini.get<cfg::video::frame_interval>(), ini.get<cfg::video::break_interval>())
    , order_depth_(gdi::GraphicDepth::unspecified())
    {}


    void enable_kbd_input_mask(bool enable) override {
        assert(this->kbd_element == *this || this->kbd_element == this->graphic_to_file);
        this->kbd_element = enable
            ? static_cast<gdi::KbdInputApi&>(*this)
            : static_cast<gdi::KbdInputApi&>(this->graphic_to_file);
    }

    void send_timestamp_chunk(timeval const & now, bool ignore_time_interval) {
        this->graphic_to_file.timestamp(now);
        this->graphic_to_file.send_timestamp_chunk(ignore_time_interval);
    }

    void request_full_cleaning() {
        this->trans_variant.trans->request_full_cleaning();
    }

    std::chrono::microseconds do_snapshot(
        const timeval & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        return this->nc.snapshot(now, x, y, ignore_frame_in_timeval);
    }

    void do_resume_capture(const timeval& now) override {
        this->trans_variant.trans->next();
        this->send_timestamp_chunk(now, true);
    }

    // shadow text
    bool kbd_input(const timeval& now, uint32_t) override {
        return this->graphic_to_file.kbd_input(now, '*');
    }

    virtual void set_depths(gdi::GraphicDepth const & depth) {
        this->order_depth_ = depth;
    }

    virtual gdi::GraphicDepth const & order_depth() const {
        return this->order_depth_;
    }

    gdi::GraphicDepth order_depth_;

};

class Capture final
: public gdi::GraphicApi
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

    using string_view = array_view_const_char;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl( cmd); }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::GraphicColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }
private:
    // Title changed
    //@{
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


public:
    Capture(
        bool capture_wrm,
        const WrmParams wrm_params,
        bool capture_png,
        const PngParams png_params,
        bool capture_pattern_checker,
        bool capture_ocr,
        OcrParams ocr_params,
        bool capture_flv,
        bool capture_flv_full,
        bool capture_meta,
        bool capture_kbd,
        const timeval & now,
        int width,
        int height,
        int order_bpp,
        int capture_bpp,
        const char * record_tmp_path,
        const char * record_path,
        const FlvParams flv_params,
        bool no_timestamp,
        auth_api * authentifier,
        const Inifile & ini,
        CryptoContext & cctx,
        Random & rnd,
        UpdateProgressData * update_progress_data)
    : is_replay_mod(!authentifier)
    , capture_wrm(capture_wrm)
    , capture_png(capture_png)
    , capture_pattern_checker(capture_pattern_checker)
    , capture_ocr(capture_ocr)
    , capture_flv(capture_flv)
    , capture_flv_full(capture_flv_full)
    , capture_meta(capture_meta)
    , update_progress_data(update_progress_data)
    , capture_api(now, width / 2, height / 2)
    , order_depth_(gdi::GraphicDepth::unspecified())
    {
        REDASSERT(authentifier ? order_bpp == capture_bpp : true);

        if (ini.get<cfg::debug::capture>()) {
            LOG(LOG_INFO, "Enable capture:  %s%s  kbd=%d %s%s%s  ocr=%d %s",
                this->capture_wrm ?"wrm ":"",
                this->capture_png ?"png ":"",
                capture_kbd ? 1 : 0,
                this->capture_flv ?"flv ":"",
                this->capture_flv_full ?"flv_full ":"",
                this->capture_pattern_checker ?"pattern ":"",
                this->capture_ocr ? (ini.get<cfg::ocr::version>() == OcrVersion::v2 ? 2 : 1) : 0,
                this->capture_meta?"meta ":""
            );
        }

        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
        const bool capture_drawable = this->capture_wrm || this->capture_flv
                                   || this->capture_ocr || this->capture_png
                                   || this->capture_flv_full;
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
                if (png_params.real_time_image_capture) {
                    this->pscrt.reset(new ImageRT(
                        now, authentifier, this->gd->impl(),
                        record_tmp_path, basename, groupid,
                        png_params
                    ));
                }
                else if (png_params.force_capture_png_if_enable) {
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
                    now, wrm_params, capture_bpp, ini.get<cfg::globals::trace_type>(),
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
        if (capture_kbd) {
            this->pkc.reset(new Kbd(now, authentifier, ini));
        }

            std::vector<std::reference_wrapper<gdi::GraphicApi>> * apis_register_graphic_list
                = this->graphic_api ? &this->graphic_api->gds : nullptr;
            std::vector<std::reference_wrapper<gdi::CaptureApi>> * apis_register_graphic_snapshot_list
                = this->graphic_api ? &this->graphic_api->snapshoters : nullptr;
            std::vector<std::reference_wrapper<gdi::KbdInputApi>> & apis_register_kbd_input_list
                = this->kbd_input_api.kbds;
            std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> & apis_register_capture_probe_list
                = this->capture_probe_api.probes;
            std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> & apis_register_external_capture_list
                = this->external_capture_api.objs;
            std::vector<std::reference_wrapper<gdi::UpdateConfigCaptureApi>> & apis_register_update_config_capture_list
                = this->update_config_capture_api.objs;



        if (this->gd ) {
            assert(apis_register_graphic_list);
            apis_register_graphic_list->push_back(this->gd->drawable);
        }
        if (this->pnc) {
            apis_register_graphic_list->push_back(this->pnc->graphic_to_file);
            this->capture_api.caps.push_back(static_cast<gdi::CaptureApi&>(*this->pnc));
            apis_register_external_capture_list.push_back(this->pnc->nc);
            apis_register_capture_probe_list.push_back(this->pnc->graphic_to_file);

            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::wrm)) {
                this->pnc->kbd_element = {apis_register_kbd_input_list, this->pnc->graphic_to_file};
                this->pnc->graphic_to_file.impl = this->pnc.get();
            }
        }

        if (this->pscrt) {
            this->pscrt->enable_rt_display = ini.get<cfg::video::rt_display>();
            this->capture_api.caps.push_back(static_cast<gdi::CaptureApi&>(*this->pscrt));
            apis_register_graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this->pscrt));
            apis_register_update_config_capture_list.push_back(static_cast<gdi::UpdateConfigCaptureApi&>(*this->pscrt));
        }

        if (this->psc) {
            this->capture_api.caps.push_back(static_cast<gdi::CaptureApi&>(*this->psc));
            apis_register_graphic_snapshot_list->push_back(static_cast<gdi::CaptureApi&>(*this->psc));
        }

        if (this->pkc) {
            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog)) {
                apis_register_kbd_input_list.push_back(this->pkc->syslog_kbd);
                this->capture_api.caps.push_back(this->pkc->syslog_kbd);
            }

            if (this->pkc->authentifier && ini.get<cfg::session_log::enable_session_log>() &&
                (ini.get<cfg::session_log::keyboard_input_masking_level>()
                 != ::KeyboardInputMaskingLevel::fully_masked)
            ) {
                apis_register_kbd_input_list.push_back(this->pkc->session_log_kbd);
                apis_register_capture_probe_list.push_back(this->pkc->session_log_kbd);
            }

            if (this->pkc->pattern_kbd.contains_pattern()) {
                apis_register_kbd_input_list.push_back(this->pkc->pattern_kbd);
            }
        }

        if (this->pvc) {
            this->capture_api.caps.push_back(this->pvc->vc);
            apis_register_graphic_snapshot_list->push_back(this->pvc->preparing_vc);
            this->pvc->first_image.cap_elem = {this->capture_api.caps, this->pvc->first_image};
            this->pvc->first_image.gcap_elem = {*apis_register_graphic_snapshot_list, this->pvc->first_image};
        }
        if (this->pvc_full) {
            this->capture_api.caps.push_back(this->pvc_full->vc);
            apis_register_graphic_snapshot_list->push_back(this->pvc_full->preparing_vc);
        }
        if (this->pmc) {
            this->capture_api.caps.push_back(this->pmc->meta);
            if (!bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta)) {
                apis_register_kbd_input_list.push_back(this->pmc->meta);
                apis_register_capture_probe_list.push_back(this->pmc->meta);
            }

            if (this->pmc->enable_agent) {
                apis_register_capture_probe_list.push_back(this->pmc->session_log_agent);
            }
        }
        if (this->ptc) {
            this->capture_api.caps.push_back(static_cast<gdi::CaptureApi&>(*this->ptc));
            apis_register_capture_probe_list.push_back(static_cast<gdi::CaptureProbeApi&>(*this->ptc));
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
            std::vector<std::reference_wrapper<gdi::GraphicApi>> * graphic_list = this->graphic_api ? &this->graphic_api->gds : nullptr;
            graphic_list->push_back(gd);
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

    void set_depths(gdi::GraphicDepth const & depth) override {
        this->order_depth_ = depth;
    }

    gdi::GraphicDepth const & order_depth() const override {
        return this->order_depth_;
    }

    gdi::GraphicDepth order_depth_;
};


