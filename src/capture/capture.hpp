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
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan,
              Clément Moroldo, Jennifer Inthavong
*/

#pragma once

#include "capture/capture_params.hpp"
#include "capture/drawable_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/kbd_log_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/pattern_params.hpp"
#include "capture/png_params.hpp"
#include "capture/sequenced_video_params.hpp"
#include "capture/video_params.hpp"
#include "capture/wrm_params.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "gdi/resize_api.hpp"
#include "capture/notify_next_video.hpp"

#include <functional> // std::reference_wrapper
#include <vector>
#include <memory>


struct NotifyTitleChanged : private noncopyable
{
    virtual void notify_title_changed(const timeval & now, array_view_const_char title) = 0;
    virtual ~NotifyTitleChanged() = default;
};

class SessionMeta;
class WrmCaptureImpl;
class PngCapture;
class PngCaptureRT;
class SyslogKbd;
class SessionLogKbd;
class PatternKbd;
class MetaCaptureImpl;
class TitleCaptureImpl;
class PatternsChecker;
class UpdateProgressData;
class RDPDrawable;
class SequencedVideoCaptureImpl;
class FullVideoCaptureImpl;
class VideoCropper;

struct MouseTrace
{
    timeval last_now;
    int     last_x;
    int     last_y;
};

class Capture final
: public gdi::GraphicApi
, public gdi::CaptureApi
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
, public gdi::ExternalCaptureApi
, public gdi::ResizeApi
{
    const bool is_replay_mod;

    using string_view = array_view_const_char;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl( cmd); }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}

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

        explicit TitleChangedFunctions(Capture & capture) : capture(capture) {}

        void notify_title_changed(timeval const & now, string_view title) override;
    } notifier_title_changed{*this};
    //@}

    // Next video
    //@{
    struct NotifyMetaIfNextVideo final : NotifyNextVideo
    {
        SessionMeta * session_meta = nullptr;

        void notify_next_video(const timeval& now, NotifyNextVideo::reason reason) override;
    } notifier_next_video;
    struct NullNotifyNextVideo final : NotifyNextVideo
    {
        void notify_next_video(const timeval& /*now*/, NotifyNextVideo::reason /*unused*/) override {}
    } null_notifier_next_video;
    //@}

public:
    std::unique_ptr<RDPDrawable> gd_drawable_;
    RDPDrawable* gd_drawable = nullptr;

    std::unique_ptr<VideoCropper> video_cropper;
    std::unique_ptr<VideoCropper> video_cropper_real_time;

private:
    class Graphic final : public gdi::GraphicApi
    {
    public:
        void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
        void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
        void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
        void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }
        void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}

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

        // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
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

        void set_row(std::size_t rownum, const uint8_t * data, size_t data_length) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_row(rownum, data, data_length);
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

    private:
        template<class... Ts>
        void draw_impl(Ts const & ... args) {
            for (gdi::GraphicApi & gd : this->gds){
                gd.draw(args...);
            }
        }

        void draw_impl(RDP::FrameMarker const & cmd);

    public:
        MouseTrace const & mouse;
        const std::vector<std::reference_wrapper<gdi::GraphicApi>> & gds;
        const std::vector<std::reference_wrapper<gdi::CaptureApi>> & caps;

        explicit Graphic(
            MouseTrace const & mouse,
            const std::vector<std::reference_wrapper<gdi::GraphicApi>> & gds,
            const std::vector<std::reference_wrapper<gdi::CaptureApi>> & caps)
        : mouse(mouse)
        , gds(gds)
        , caps(caps)
        {}
    };

    std::unique_ptr<Graphic> graphic_api;

    std::unique_ptr<WrmCaptureImpl> wrm_capture_obj;
    std::unique_ptr<PngCapture> png_capture_obj;
    std::unique_ptr<PngCaptureRT> png_capture_real_time_obj;

    std::unique_ptr<SyslogKbd> syslog_kbd_capture_obj;
    std::unique_ptr<SessionLogKbd> session_log_kbd_capture_obj;
    std::unique_ptr<PatternKbd> pattern_kbd_capture_obj;

    std::unique_ptr<SequencedVideoCaptureImpl> sequenced_video_capture_obj;
    std::unique_ptr<FullVideoCaptureImpl> full_video_capture_obj;
    std::unique_ptr<MetaCaptureImpl> meta_capture_obj;
    std::unique_ptr<TitleCaptureImpl> title_capture_obj;
    std::unique_ptr<PatternsChecker> patterns_checker;

    UpdateProgressData * update_progress_data;

    MouseTrace mouse_info;

    std::vector<std::reference_wrapper<gdi::GraphicApi>> gds;
    // Objects willing to be warned of FrameMarker Events
    std::vector<std::reference_wrapper<gdi::CaptureApi>> caps;
    std::vector<std::reference_wrapper<gdi::KbdInputApi>> kbds;
    std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> probes;
    std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> ext_caps;

    bool capture_drawable = false;

    SmartVideoCropping smart_video_cropping;

    uint32_t verbose = 0;

public:
    Capture(
        const CaptureParams capture_params,
        const DrawableParams drawable_params,
        bool capture_wrm, const WrmParams wrm_params,
        bool capture_png, const PngParams png_params,
        bool capture_pattern_checker, const PatternParams pattern_params,
        bool capture_ocr, const OcrParams ocr_params,
        bool capture_video, const SequencedVideoParams /*sequenced_video_params*/,
        bool capture_video_full, const FullVideoParams full_video_params,
        bool capture_meta, const MetaParams /*meta_params*/,
        bool capture_kbd, const KbdLogParams /*kbd_log_params*/,
        const VideoParams video_params,
        UpdateProgressData * update_progress_data,
        Rect const & crop_rect
    );

    ~Capture();

public:
    void resize(uint16_t width, uint16_t height) override;

    enum class RTDisplayResult : char
    {
        Disabled,
        Enabled,
        Unchanged,
    };

    RTDisplayResult set_rt_display(bool enable_rt_display);

    void set_row(size_t rownum, const uint8_t * data, size_t data_length) override;

    void sync() override
    {
        if (this->capture_drawable) {
            this->graphic_api->sync();
        }
    }

    bool kbd_input(timeval const & now, uint32_t uchar) override {
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

    bool has_graphic_api() const {
        return static_cast<bool>(this->graphic_api);
    }

    void add_graphic(gdi::GraphicApi & gd) {
        if (this->capture_drawable) {
            this->gds.emplace_back(gd);
        }
    }

    Microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override;

    void visibility_rects_event(Rect rect) override;

protected:
    template<class... Ts>
    void draw_impl(const Ts & ... args) {
        if (this->capture_drawable) {
            this->graphic_api->draw(args...);
        }
    }

    void draw_impl(const RDP::RAIL::NewOrExistingWindow & cmd);

    void draw_impl(const RDP::RAIL::DeletedWindow & cmd);

    void draw_impl(const RDP::RAIL::NonMonitoredDesktop & cmd);

    struct WindowRecord {
        uint32_t window_id;
        uint32_t fields_present_flags;
        uint32_t style;
        uint8_t show_state;
        int32_t visible_offset_x;
        int32_t visible_offset_y;

        std::string title_info;

        WindowRecord(uint32_t window_id, uint32_t fields_present_flags,
                     uint32_t style, uint8_t show_state,
                     int32_t visible_offset_x, int32_t visible_offset_y,
                     const char* title_info)
        : window_id(window_id)
        , fields_present_flags(fields_present_flags)
        , style(style)
        , show_state(show_state)
        , visible_offset_x(visible_offset_x)
        , visible_offset_y(visible_offset_y)
        , title_info(title_info) {}
    };

    std::vector<WindowRecord> windows;

    struct WindowVisibilityRectRecord {
        uint32_t window_id;
        Rect rect;

        WindowVisibilityRectRecord(uint32_t window_id, Rect rect)
        : window_id(window_id)
        , rect(rect) {}
    };

    std::vector<WindowVisibilityRectRecord> window_visibility_rects;

    Rect get_joint_visibility_rect() const;

public:
    void set_pointer(const Pointer & cursor) override {
        if (this->capture_drawable) {
            this->graphic_api->set_pointer(cursor);
        }
    }

    void set_palette(const BGRPalette & palette) override {
        if (this->capture_drawable) {
            this->graphic_api->set_palette(palette);
        }
    }

    void set_pointer_display();

    void external_breakpoint() override {
        for (gdi::ExternalCaptureApi & obj : this->ext_caps) {
            obj.external_breakpoint();
        }
    }

    void external_time(timeval const & now) override {
        for (gdi::ExternalCaptureApi & obj : this->ext_caps) {
            obj.external_time(now);
        }
    }

    void session_update(const timeval & now, array_view_const_char message) override {
        for (gdi::CaptureProbeApi & cap_prob : this->probes) {
            cap_prob.session_update(now, message);
        }
    }

    void possible_active_window_change() override {
        for (gdi::CaptureProbeApi & cap_prob : this->probes) {
            cap_prob.possible_active_window_change();
        }
    }
};
