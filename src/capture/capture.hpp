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

#include "configs/autogen/enums.hpp"
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

class UpdateProgressData;
class RDPDrawable;
class VideoCropper;
class WrmCaptureImpl;
class SequencedVideoCaptureImpl;
class FullVideoCaptureImpl;

class CaptureParams;
class DrawableParams;
class FullVideoParams;
class KbdLogParams;
class MetaParams;
class OcrParams;
class PatternParams;
class PngParams;
class SequencedVideoParams;
class VideoParams;
class WrmParams;

class Capture final
: public gdi::GraphicApi
, public gdi::CaptureApi
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
, public gdi::ExternalCaptureApi
, public gdi::ResizeApi
{
public:
    Capture(
        const CaptureParams& capture_params,
        const DrawableParams& drawable_params,
        bool capture_wrm, const WrmParams& wrm_params,
        bool capture_png, const PngParams& png_params,
        bool capture_pattern_checker, const PatternParams& pattern_params,
        bool capture_ocr, const OcrParams& ocr_params,
        bool capture_video, const SequencedVideoParams& sequenced_video_params,
        bool capture_video_full, const FullVideoParams& full_video_params,
        bool capture_meta, const MetaParams& meta_params,
        bool capture_kbd, const KbdLogParams& kbd_log_params,
        const VideoParams& video_params,
        UpdateProgressData * update_progress_data,
        Rect const & crop_rect
    );

    ~Capture();

    void resize(uint16_t width, uint16_t height) override;

    enum class RTDisplayResult : char
    {
        Disabled,
        Enabled,
        Unchanged,
    };

    RTDisplayResult set_rt_display(bool enable_rt_display);

    void set_row(size_t rownum, bytes_view data) override;

    void sync() override;

    bool kbd_input(timeval const & now, uint32_t uchar) override;

    void enable_kbd_input_mask(bool enable) override;

    bool has_graphic_api() const;

    void add_graphic(gdi::GraphicApi & gd);

    Microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override;

    void visibility_rects_event(Rect rect) override;

    void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) override;

    void set_palette(const BGRPalette & palette) override;

    void set_pointer_display();

    void external_breakpoint() override;

    void external_time(timeval const & now) override;

    void session_update(const timeval & now, array_view_const_char message) override;

    void possible_active_window_change() override;

    void draw(RDP::FrameMarker    const & cmd) override;
    void draw(RDPDestBlt          const & cmd, Rect clip) override;
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override;
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPScrBlt           const & cmd, Rect clip) override;
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override;
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override;
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override;
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override;
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override;
    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const &/*content*/) override;

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override;
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override;
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override;
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override;
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override;
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override;
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override;
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override;

    void draw(RDPColCache   const & cmd) override;
    void draw(RDPBrushCache const & cmd) override;

    // public visibility for test...
    class SessionMeta;
    class SessionLogAgent;
    class PngCapture;
    class PngCaptureRT;
    class SyslogKbd;
    class SessionLogKbd;
    class PatternKbd;
    class MetaCaptureImpl;
    class TitleCaptureImpl;
    class PatternsChecker;

private:
    template<class... Ts>
    void draw_impl(const Ts & ... args);

    Rect get_join_visibility_rect() const;

    const bool is_replay_mod;

    // Title changed
    //@{
    struct NotifyTitleChanged final
    {
        Capture & capture;

        explicit NotifyTitleChanged(Capture & capture) : capture(capture) {}

        void notify_title_changed(timeval const & now, array_view_const_char title);
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

    std::unique_ptr<RDPDrawable> gd_drawable_;
    RDPDrawable* gd_drawable = nullptr;

    std::unique_ptr<VideoCropper> video_cropper;
    std::unique_ptr<VideoCropper> video_cropper_real_time;

    struct MouseTrace
    {
        timeval last_now;
        int     last_x;
        int     last_y;
    };

    class Graphic;
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

    struct WindowRecord;
    struct WindowVisibilityRectRecord;

    std::vector<WindowRecord> windows;
    std::vector<WindowVisibilityRectRecord> window_visibility_rects;

    bool old_kbd_input_mask_state = false;
};
