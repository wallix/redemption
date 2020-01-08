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
  Copyright (C) Wallix 2019
  Author(s): Christophe Grosjean

*/
#pragma once

#include "mod/null/null.hpp"
#include "mod/mod_api.hpp"
#include "transport/socket_transport.hpp"

#include "acl/end_session_warning.hpp"
#include "RAIL/client_execute.hpp"
#include "acl/auth_api.hpp"
#include "acl/file_system_license_store.hpp"
#include "acl/module_manager/enums.hpp"
#include "configs/config.hpp"
#include "core/log_id.hpp"
#include "core/session_reactor.hpp"
#include "front/front.hpp"
#include "gdi/protected_graphics.hpp"

#include "mod/internal/rail_module_host_mod.hpp"

#include "mod/rdp/rdp_api.hpp"
#include "mod/mod_api.hpp"
#include "mod/null/null.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "mod/xup/xup.hpp"

#include "transport/socket_transport.hpp"

#include "utils/load_theme.hpp"
#include "utils/netutils.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/update_lock.hpp"
#include "utils/log_siem.hpp"
#include "utils/fileutils.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "acl/module_manager/enums.hpp"
#include "core/back_event_t.hpp"

#include "core/session_reactor.hpp"

//struct Filter : public mod_api, public FrontAPI
//{
    // ======================= mod_api::Callback::RdpInput
    //    virtual ~RdpInput() = default;
    //    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) = 0;
    //    virtual void rdp_input_unicode(uint16_t unicode, uint16_t flag) { (void)unicode; (void)flag; }
    //    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) = 0;
    //    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) = 0;
    //    virtual void rdp_input_invalidate(Rect r) = 0;
    //    virtual void rdp_input_invalidate2(array_view<Rect const> vr) {
    //        for (Rect const & rect : vr) {
    //            if (!rect.isempty()) {
    //                this->rdp_input_invalidate(rect);
    //            }
    //        }
    //    }
    //    // Client calls this member function when it became up and running.
    //    virtual void rdp_input_up_and_running() { /* LOG(LOG_ERR, "CB:UP_AND_RUNNING"); */}

    //    virtual void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
    //    { (void)left; (void)top; (void)right; (void)bottom; }

    //    virtual void rdp_suppress_display_updates() {}

    //    virtual void refresh(Rect clip) = 0;    
    // ======================= mod_api::Callback
    //    virtual void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags)
    //    {
    //        (void)front_channel_name;
    //        (void)chunk;
    //        (void)length;
    //        (void)flags;
    //    }
    //    // Interface for session to send back to mod_rdp for tse virtual channel target data (asked previously)
    //    virtual void send_auth_channel_data(const char * data) { (void)data; }
    //    virtual void send_checkout_channel_data(const char * data) { (void)data; }

    //    virtual void create_shadow_session(const char * userdata, const char * type) {
    //        (void)userdata;
    //        (void)type;
    //    }    
    // ======================= mod_api
    //    public:
    //        enum : bool {
    //            CLIENT_UNLOGGED,
    //            CLIENT_LOGGED
    //        };
    //        bool logged_on = CLIENT_UNLOGGED; // TODO suspicious

    //        [[nodiscard]] virtual bool is_up_and_running() const { return false; }

    //        // support auto-reconnection
    //        virtual bool is_auto_reconnectable() { return false; }

    //        virtual void disconnect() {}

    //        virtual void display_osd_message(std::string const & /*unused*/) {}

    //        virtual void move_size_widget(int16_t/* left*/, int16_t/* top*/, uint16_t/* width*/, uint16_t/* height*/) {}

    //        virtual bool disable_input_event_and_graphics_update(
    //                bool disable_input_event, bool disable_graphics_update) {
    //            (void)disable_input_event;
    //            (void)disable_graphics_update;
    //            return false;
    //        }

    //        virtual void send_input(int/* time*/, int/* message_type*/, int/* device_flags*/, int/* param1*/, int/* param2*/) {}

    //        [[nodiscard]] virtual Dimension get_dim() const { return Dimension(); }

    //        virtual void log_metrics() {}

    //        virtual void DLP_antivirus_check_channels_files() {}
    //    };    
    // ============================ FrontAPI
//    public:
//        virtual bool can_be_start_capture() = 0;
//        virtual bool must_be_stop_capture() = 0;
//        [[nodiscard]] virtual bool is_capture_in_progress() const = 0;

//        [[nodiscard]] virtual const CHANNELS::ChannelDefArray & get_channel_list() const = 0;
//        virtual void send_to_channel( const CHANNELS::ChannelDef & channel, bytes_view chunk_data
//                                    , std::size_t total_length, int flags) = 0;

//        enum class ResizeResult {
//            no_need = 0,
//            done    = 1,
//            fail    = -1,
//            instant_done = 2,
//            remoteapp = 3
//        };
//        virtual ResizeResult server_resize(ScreenInfo screen_server) = 0;

//        virtual void server_relayout(MonitorLayoutPDU const& monitor_layout_pdu_ref) { (void)monitor_layout_pdu_ref; }

//        virtual void update_pointer_position(uint16_t x, uint16_t y) { (void)x; (void)y; }

//        virtual void set_keyboard_indicators(uint16_t LedFlags) { (void)LedFlags; }

//        ////////////////////////////////
//        // Session Probe.

//        virtual void session_probe_started(bool /*unused*/) {}
//        virtual void set_keylayout(int LCID) { (void)LCID; }
//        virtual void set_focus_on_password_textbox(bool /*unused*/) {}
//        virtual void set_focus_on_unidentified_input_field(bool /*unused*/) {}
//        virtual void set_consent_ui_visible(bool /*unused*/) {}
//        virtual void session_update(LogId id, KVList kv_list) { (void)id; (void)kv_list; }

//        ////////////////////////////////
//        // RemoteApp.
//        virtual void send_savesessioninfo() {}

//        /// \return  -1 is an error
//        [[nodiscard]] virtual int get_keylayout() const { return -1; }

//        virtual ~FrontAPI() = default;
//        ================================= public gdi::GraphicApi

//    GraphicApi() = default;

//    virtual ~GraphicApi() = default;

//    virtual void set_palette(BGRPalette   const & /*unused*/) {}

//    virtual void draw(RDP::FrameMarker    const & cmd) = 0;
//    virtual void draw(RDPDestBlt          const & cmd, Rect clip) = 0;
//    virtual void draw(RDPMultiDstBlt      const & cmd, Rect clip) = 0;
//    virtual void draw(RDPScrBlt           const & cmd, Rect clip) = 0;
//    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) = 0;

//#ifdef __EMSCRIPTEN__
//    virtual void set_bmp_cache_entries(std::array<uint16_t, 3> const & /*nb_entries*/) = 0;
//    virtual void draw(RDPBmpCache         const & /*cmd*/) = 0;
//    virtual void draw(RDPMemBlt           const & cmd, Rect clip) = 0;
//    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//#else
//    virtual void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) = 0;
//    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, ColorCtx color_ctx, Bitmap const & bmp) = 0;
//#endif

//    virtual void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) = 0;

//    virtual void draw(RDPPatBlt           const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPOpaqueRect       const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPLineTo           const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPPolygonSC        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPPolygonCB        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPPolyline         const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPEllipseSC        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPEllipseCB        const & cmd, Rect clip, ColorCtx color_ctx) = 0;
//    virtual void draw(RDPNineGrid         const & cmd, Rect clip, ColorCtx color_ctx, Bitmap const & bmp) = 0;
//    virtual void draw(RDPGlyphIndex       const & cmd, Rect clip, ColorCtx color_ctx, GlyphCache const & gly_cache) = 0;
//    virtual void draw(RDPSetSurfaceCommand const & cmd) = 0;
//    virtual void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) = 0;

//    // NOTE maybe in an other interface
//    virtual void draw(const RDP::RAIL::NewOrExistingWindow            & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::WindowIcon                     & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::CachedIcon                     & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::DeletedWindow                  & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::DeletedNotificationIcons       & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & /*cmd*/) = 0;
//    virtual void draw(const RDP::RAIL::NonMonitoredDesktop            & /*cmd*/) = 0;

//    // TODO The 2 methods below should not exist and cache access be done before calling drawing orders
//    virtual void draw(RDPColCache   const & /*cache*/) {}
//    virtual void draw(RDPBrushCache const & /*cache*/) {}

//    virtual void begin_update() {}
//    virtual void end_update() {}

//    virtual void sync() {}

//    enum class SetPointerMode : uint8_t
//    {
//        New,
//        Cached,
//        Insert,
//    };

//    /// \c cache_idx is ignored with \c SetPointerMode::Insert
//    virtual void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) = 0;

//    // TODO berk, data within size
//    virtual void set_row(std::size_t rownum, bytes_view data) { (void)rownum; (void)data; }
//}

//struct GraphicFilter : public gdi::GraphicApi
//{
//    gdi::GraphicApi & sink;

//    GraphicFilter(gdi::GraphicApi & sink) : sink(sink) {}

//    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
//    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
//    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
//    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
//    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
//    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
//    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
//    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
//    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
//    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }
//    void draw(RDPNineGrid const &  /*unused*/, Rect  /*unused*/, gdi::ColorCtx  /*unused*/, Bitmap const &  /*unused*/) override {}
//    void draw(RDPSetSurfaceCommand const & /*cmd*/) override { }
//    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const &/*content*/) override { }

////    // NOTE maybe in an other interface
////    virtual void draw(const RDP::RAIL::NewOrExistingWindow            & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::WindowIcon                     & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::CachedIcon                     & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::DeletedWindow                  & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::DeletedNotificationIcons       & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
////    virtual void draw(const RDP::RAIL::NonMonitoredDesktop            & /*cmd*/) = 0;
//    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

//    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
//    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

//};

struct ModWrapper
{
    struct GraphicFilter : public gdi::GraphicApi
    {
        Rect protected_rect;
        gdi::GraphicApi & sink;
        BGRPalette const & palette;
        Callback * mod;

        GraphicFilter(gdi::GraphicApi & sink, const BGRPalette & palette, Rect rect) 
            : sink(sink), palette(palette), protected_rect(rect) {}

    public:
        void draw(RDP::FrameMarker    const & cmd) override { this->sink.draw(cmd); }
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
        void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) 
                                                                                  override { this->draw_impl(cmd, clip, color_ctx, bmp); }
        void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) 
                                                                            override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }
        void draw(RDPNineGrid const &  cmd, Rect clip, gdi::ColorCtx  color_ctx, Bitmap const &  bmp) 
                                                                                  override { }
        void draw(RDPSetSurfaceCommand const & cmd) override { this->sink.draw(cmd); }
        void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override { this->sink.draw(cmd, content); }
        void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->sink.draw(cmd); }
        void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->sink.draw(cmd); }
        void draw(RDPColCache   const & cmd) override { this->sink.draw(cmd); }
        void draw(RDPBrushCache const & cmd) override { this->sink.draw(cmd); }
        void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) 
                                                            override {this->sink.set_pointer(cache_idx, cursor, mode); }
        void set_palette(BGRPalette const & palette) override { this->sink.set_palette(palette); }
        void sync() override {this->sink.sync();}
        void set_row(std::size_t rownum, bytes_view data) override {this->sink.set_row(rownum, data);}
        void begin_update() override {this->sink.begin_update();}
        void end_update() override {this->sink.end_update();}

    private:
        template<class Command, class... Args>
        void draw_impl(Command const & cmd, Rect clip, Args const &... args)
        {
            auto const & clip_rect = clip_from_cmd(cmd).intersect(clip);
            if (this->protected_rect.contains(clip_rect) || clip_rect.isempty()) {
                // nada: leave the OSD message rect untouched
            }
            else if (clip_rect.has_intersection(this->protected_rect)) {
                // draw the parts of the screen outside OSD message rect
                this->sink.begin_update();
                for (const Rect & subrect : gdi::subrect4(clip_rect, this->protected_rect)) {
                    if (!subrect.isempty()) {
                        this->sink.draw(cmd, subrect, args...);
                    }
                }
                this->sink.end_update();
            }
            else {
                // The drawing order is fully ouside OSD message rect
                this->sink.draw(cmd, clip, args...);
            }
        }

        void draw_impl(const RDPBitmapData & bitmap_data, const Bitmap & bmp)
        {
            Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                        , bitmap_data.dest_right - bitmap_data.dest_left + 1
                        , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

            if (this->protected_rect.contains(rectBmp) || rectBmp.isempty()) {
                // nada: leave the OSD message rect untouched
            }
            if (rectBmp.has_intersection(this->protected_rect)) {
                this->sink.begin_update();
                for (const Rect & subrect : gdi::subrect4(rectBmp, this->protected_rect)) {
                    if (!subrect.isempty()) {
                        // draw the parts of the screen outside OSD message rect
                        Bitmap sub_bmp(bmp, Rect(subrect.x - rectBmp.x, subrect.y - rectBmp.y, subrect.cx, subrect.cy));

                        RDPBitmapData sub_bmp_data = bitmap_data;

                        sub_bmp_data.dest_left = subrect.x;
                        sub_bmp_data.dest_top = subrect.y;
                        sub_bmp_data.dest_right = std::min<uint16_t>(sub_bmp_data.dest_left + subrect.cx - 1, bitmap_data.dest_right);
                        sub_bmp_data.dest_bottom = sub_bmp_data.dest_top + subrect.cy - 1;

                        sub_bmp_data.width = sub_bmp.cx();
                        sub_bmp_data.height = sub_bmp.cy();
                        sub_bmp_data.bits_per_pixel = safe_int(sub_bmp.bpp());
                        sub_bmp_data.flags = 0;

                        sub_bmp_data.bitmap_length = sub_bmp.bmp_size();

                        this->sink.draw(sub_bmp_data, sub_bmp);
                    }
                }
                this->sink.end_update();
            }
            else {
                // The drawing order is fully ouside OSD message rect
                this->sink.draw(bitmap_data, bmp);
            }
        }

        void draw_impl(const RDPScrBlt & cmd, const Rect clip)
        {
            const Rect drect = cmd.rect.intersect(clip);
            const int deltax = cmd.srcx - cmd.rect.x;
            const int deltay = cmd.srcy - cmd.rect.y;
            const int srcx = drect.x + deltax;
            const int srcy = drect.y + deltay;
            const Rect srect(srcx, srcy, drect.cx, drect.cy);

            const bool has_dest_intersec_fg = drect.has_intersection(this->protected_rect);
            const bool has_src_intersec_fg = srect.has_intersection(this->protected_rect);

            if (!has_dest_intersec_fg && !has_src_intersec_fg) {
                // neither scr or dest rect intersect with OSD message
                this->sink.draw(cmd, clip);
            }
            else {
                // We don't have src data, ask it to server, no choice
                // FIXME: if only drect has intersection we could perform scr_blt, no real need to ask data to server
                this->sink.begin_update();
                gdi::subrect4_t rects = gdi::subrect4(drect, this->protected_rect);
                auto e = std::remove_if(rects.begin(), rects.end(), [](const Rect & rect) { return rect.isempty(); });
                auto av = make_array_view(rects.begin(), e);
                this->mod->rdp_input_invalidate2(av);
                this->sink.end_update();
            }
        }
    } gfilter;
    
    std::string module_name() 
    {
        return this->modi->module_name();
    }

public:

    void last_disconnect()
    {
        if (this->has_mod()) {
            try {
                this->get_mod()->disconnect();
            }
            catch (Error const& e) {
                LOG(LOG_INFO, "MMIni::invoke_close_box exception = %u!", e.id);
            }
        }
        this->remove_mod();
    }

    gdi::GraphicApi & get_graphics() 
    {
        return this->gfilter;
    }

    [[nodiscard]] Rect get_protected_rect() const
    { return this->gfilter.protected_rect; }

    void set_protected_rect(Rect const rect)
    { this->gfilter.protected_rect = rect; }

    FrontAPI & front;

private:
    ClientInfo const & client_info;
    ClientExecute & rail_client_execute;
    public:
    rdp_api*       rdpapi = nullptr;
    RailModuleHostMod* rail_module_host_mod_ptr = nullptr;
    windowing_api* &winapi;
    private:
    Inifile & ini;

    std::string osd_message;
    Rect clip;
    RDPColor color;
    RDPColor background_color;
    bool is_disable_by_input = false;
    bool bogus_refresh_rect_ex;
    const Font & glyphs;
    const Theme & theme;

public:
    explicit ModWrapper(FrontAPI & front, BGRPalette const & palette, gdi::GraphicApi& graphics, ClientInfo const & client_info, const Font & glyphs, const Theme & theme, ClientExecute & rail_client_execute, windowing_api* & winapi, Inifile & ini)
    : front(front)
    , gfilter(graphics, palette, Rect{})
    , client_info(client_info)
    , rail_client_execute(rail_client_execute)
    , winapi(winapi)
    , ini(ini)
    , bogus_refresh_rect_ex(false)
    , glyphs(glyphs)
    , theme(theme)
    {}

    ~ModWrapper(){
        this->remove_mod();
    }

    // FIXME: we should always be able to use graphic_wrapper directly
    // finding out the actual internal graphics interface should never be necessary
    gdi::GraphicApi & get_graphic_wrapper()
    {
        gdi::GraphicApi& gd = this->get_graphics();
        if (this->rail_module_host_mod_ptr) {
            return this->rail_module_host_mod_ptr->proxy_gd(gd);
        }
        return gd;
    }

    Callback & get_callback() noexcept
    {
        return *this->get_mod();
    }

    [[nodiscard]] bool is_input_owner() const { return this->is_disable_by_input; }

    [[nodiscard]] const char* get_message() const {
        return this->osd_message.c_str();
    }

    void set_message(std::string message, bool is_disable_by_input)
    {
        this->osd_message = std::move(message);
        this->is_disable_by_input = is_disable_by_input;
        this->bogus_refresh_rect_ex = (this->ini.get<cfg::globals::bogus_refresh_rect>()
         && this->ini.get<cfg::globals::allow_using_multiple_monitors>()
         && (this->client_info.cs_monitor.monitorCount > 1));

        if (is_disable_by_input) {
            str_append(this->osd_message, "  ", TR(trkeys::disable_osd, language(this->ini)));
        }

        gdi::TextMetrics tm(this->glyphs, this->osd_message.c_str());
        int w = tm.width + padw * 2;
        int h = tm.height + padh * 2;
        this->color = color_encode(BGRColor(BLACK), this->client_info.screen_info.bpp);
        this->background_color = color_encode(BGRColor(LIGHT_YELLOW), this->client_info.screen_info.bpp);

        if (this->client_info.remote_program &&
            (this->winapi == static_cast<windowing_api*>(&this->rail_client_execute))) {

            Rect current_work_area_rect = this->rail_client_execute.get_current_work_area_rect();

            this->clip = Rect(
                current_work_area_rect.x +
                    (current_work_area_rect.cx < w ? 0 : (current_work_area_rect.cx - w) / 2),
                0, w, h);
        }
        else {
            this->clip = Rect(this->client_info.screen_info.width < w ? 0 : (this->client_info.screen_info.width - w) / 2, 0, w, h);
        }

        this->set_protected_rect(this->clip);

        if (this->winapi) {
            this->winapi->create_auxiliary_window(this->clip);
        }
    }

    static constexpr int padw = 16;
    static constexpr int padh = 16;

    void draw_osd_message()
    {
        this->get_graphics().begin_update();
        this->draw_osd_message_impl(this->get_graphics());
        this->get_graphics().end_update();
    }


private:
    void draw_osd_message_impl(gdi::GraphicApi & drawable)
    {
        if (this->clip.isempty()) {
            return ;
        }

        auto const color_ctx = gdi::ColorCtx::from_bpp(this->client_info.screen_info.bpp, this->gfilter.palette);

        drawable.draw(RDPOpaqueRect(this->clip, this->background_color), this->clip, color_ctx);

        RDPLineTo line_ileft(1, this->clip.x, this->clip.y, this->clip.x, this->clip.y + this->clip.cy - 1,
            encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
        drawable.draw(line_ileft, this->clip, color_ctx);
        RDPLineTo line_ibottom(1, this->clip.x, this->clip.y + this->clip.cy - 1, this->clip.x + this->clip.cx - 1, this->clip.y + this->clip.cy - 1,
            encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
        drawable.draw(line_ibottom, this->clip, color_ctx);

        RDPLineTo line_iright(1, this->clip.x + this->clip.cx - 1, this->clip.y + this->clip.cy - 1, this->clip.x + this->clip.cx - 1, this->clip.y,
            encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
        drawable.draw(line_iright, this->clip, color_ctx);
        RDPLineTo line_etop(1, this->clip.x + this->clip.cx - 1, this->clip.y, this->clip.x, this->clip.y,
            encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
        drawable.draw(line_etop, this->clip, color_ctx);

        gdi::server_draw_text(
            drawable, this->glyphs,
            this->get_protected_rect().x + padw, padh,
            this->osd_message.c_str(),
            this->color, this->background_color, color_ctx, this->clip
        );

        this->clip = Rect();
    }

private:
    SocketTransport * psocket_transport = nullptr;

public:

    null_mod no_mod;
private:
    mod_api* modi = &no_mod;

public:
    mod_api* get_mod()
    {
        return this->modi;
    }

    [[nodiscard]] mod_api const* get_mod() const
    {
        return this->modi;
    }

    bool has_mod() const {
        return (this->modi != &this->no_mod);
    }

    void remove_mod()
    {
        if (this->has_mod()){
            this->clear_osd_message();
            delete this->modi;
            this->modi = &this->no_mod;
            this->rdpapi = nullptr;
            this->winapi = nullptr;
            this->rail_module_host_mod_ptr = nullptr;
        }
    }

    bool is_up_and_running() const {
        return this->has_mod() && this->get_mod()->is_up_and_running();
    }

    void set_mod_transport(SocketTransport * psocket_transport)
    {
        LOG(LOG_INFO, "set_mod_transport %p", psocket_transport);
        this->psocket_transport = psocket_transport;
    }

    // push_mod or replace_mod
    void set_mod(mod_api* mod)
    {
        // TODO: check we are using no_mod, otherwise it is an error
        this->modi = mod;
        this->gfilter.mod = mod;
    }
    
    [[nodiscard]] SocketTransport* get_mod_transport() const noexcept
    {
//        LOG(LOG_INFO, "get_mod_transport %p", psocket_transport);
        return this->psocket_transport;
    }

    void disable_osd()
    {
        this->is_disable_by_input = false;
        auto const protected_rect = this->get_protected_rect();
        this->set_protected_rect(Rect{});

        if (this->bogus_refresh_rect_ex) {
            this->get_mod()->rdp_suppress_display_updates();
            this->get_mod()->rdp_allow_display_updates(0, 0,
                this->client_info.screen_info.width,
                this->client_info.screen_info.height);
        }

        if (this->winapi) {
            this->winapi->destroy_auxiliary_window();
        }

        this->get_mod()->rdp_input_invalidate(protected_rect);
    }

    void clear_osd_message()
    {
        if (!this->get_protected_rect().isempty()) {
            this->disable_osd();
        }
    }

    void osd_message_fn(std::string message, bool is_disable_by_input)
    {
        if (message != this->get_message()) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            this->set_message(std::move(message), is_disable_by_input);
            this->draw_osd_message();
        }
    }

    bool try_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
        (void)param1;
        (void)param2;
        (void)param3;
        (void)param4;
        if (this->is_disable_by_input
         && keymap->nb_kevent_available() > 0
//             && !(param3 & SlowPath::KBDFLAGS_DOWN)
         && keymap->top_kevent() == Keymap2::KEVENT_INSERT
        ) {
            keymap->get_kevent();
            this->disable_osd();
            return true;
        }
        return false;
    }

    bool try_input_mouse(int device_flags, int x, int y, Keymap2 * /*unused*/)
    {
        if (this->is_disable_by_input
         && this->get_protected_rect().contains_pt(x, y)
         && device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            this->disable_osd();
            return true;
        }
        return false;
    }

    bool try_input_invalidate(const Rect r)
    {
        if (!this->get_protected_rect().isempty() && r.has_intersection(this->get_protected_rect())) {
            auto rects = gdi::subrect4(r, this->get_protected_rect());
            auto p = std::begin(rects);
            auto e = std::remove_if(p, std::end(rects), [](Rect const & rect) {
                return rect.isempty();
            });
            if (p != e) {
                this->get_mod()->rdp_input_invalidate2({p, e});
                this->clip = r.intersect(this->get_protected_rect());
            }
            return true;
        }
        return false;
    }

    bool try_input_invalidate2(array_view<Rect const> vr)
    {
        // TODO PERF multi opaque rect
        bool ret = false;
        for (Rect const & r : vr) {
            if (!this->try_input_invalidate(r)) {
                this->get_mod()->rdp_input_invalidate(r);
            }
            else {
                ret = true;
            }
        }
        return ret;
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
        if (!this->try_input_scancode(param1, param2, param3, param4, keymap)) {
            this->get_mod()->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag)
    {
        this->get_mod()->rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (!this->try_input_mouse(device_flags, x, y, keymap)) {
            this->get_mod()->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_invalidate(Rect r)
    {
        if (!this->try_input_invalidate(r)) {
            this->get_mod()->rdp_input_invalidate(r);
        }
    }

    void rdp_input_invalidate2(array_view<Rect const> vr)
    {
        if (!this->try_input_invalidate2(vr)) {
            this->get_mod()->rdp_input_invalidate2(vr);
        }
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    { this->get_mod()->rdp_input_synchronize(time, device_flags, param1, param2); }

    void rdp_input_up_and_running(ScreenInfo & screen_info)
    { this->get_mod()->rdp_input_up_and_running(screen_info); }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
    { this->get_mod()->rdp_allow_display_updates(left, top, right, bottom); }

    void rdp_suppress_display_updates()
    { this->get_mod()->rdp_suppress_display_updates(); }

    void refresh(Rect r)
    {
        this->get_mod()->refresh(r);
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags)
    {
        LOG(LOG_INFO, ">>>>>>>>++++++++++++ send_to_mod_channel");
        this->get_mod()->send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    void send_auth_channel_data(const char * data)
    { this->get_mod()->send_auth_channel_data(data); }

    void send_checkout_channel_data(const char * data)
    { this->get_mod()->send_checkout_channel_data(data); }

    void disconnect()
    { this->get_mod()->disconnect(); }

    void display_osd_message(std::string const & message)
    { this->get_mod()->display_osd_message(message); }

    [[nodiscard]] Dimension get_dim() const
    {
        return this->get_mod()->get_dim();
    }

};

