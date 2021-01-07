/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/RDP/remote_programs.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/front_api.hpp"
#include "gdi/clip_from_cmd.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/text_metrics.hpp"
#include "gdi/protected_graphics.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "utils/rect.hpp"
#include "utils/theme.hpp"
#include "utils/timebase.hpp"
#include "utils/translation.hpp"
#include "utils/sugar/not_null_ptr.hpp"

#include <string>

class RemoteProgramsSessionManager final
: public gdi::GraphicApi
, public RemoteProgramsWindowIdManager
, public windowing_api
{
    gdi::GraphicApi & front;
    mod_api  & mod;

    Language lang;

    Font  const & font;
    Theme const & theme;

    const RDPVerbose verbose;

    uint32_t blocked_server_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    bool graphics_update_disabled = false;

    Rect protected_rect;

    uint32_t dialog_box_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    gdi::GraphicApi * drawable = nullptr;

    enum class DialogBoxType {
        SPLASH_SCREEN,
        WAITING_SCREEN,

        NONE
    } dialog_box_type = DialogBoxType::NONE;

    Rect disconnect_now_button_rect;
    bool disconnect_now_button_clicked = false;

    bool has_previous_window = false;

    std::string session_probe_window_title;

    uint32_t auxiliary_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    const not_null_ptr<ClientExecute> rail_client_execute;

    bool currently_without_window = false;

    bool has_actively_monitored_desktop = true;

    std::chrono::milliseconds rail_disconnect_message_delay {};

    EventsGuard events_guard;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl( cmd); }
    void draw(RDPDstBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
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
    void draw(RDPSetSurfaceCommand const & /*cmd*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const &/*content*/) override {}

    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

    void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) override
    {
        if (this->drawable) {
            this->drawable->set_pointer(cache_idx, cursor, mode);
        }
    }

    explicit RemoteProgramsSessionManager(
        EventContainer & events,
        gdi::GraphicApi& front, mod_api& mod, Language lang,
        Font const & font, Theme const & theme,
        char const * session_probe_window_title,
        not_null_ptr<ClientExecute> rail_client_execute,
        std::chrono::milliseconds rail_disconnect_message_delay,
        RDPVerbose verbose)
    : front(front)
    , mod(mod)
    , lang(lang)
    , font(font)
    , theme(theme)
    , verbose(verbose)
    , session_probe_window_title(session_probe_window_title)
    , rail_client_execute(rail_client_execute)
    , rail_disconnect_message_delay(rail_disconnect_message_delay)
    , events_guard(events)
    {}

    ~RemoteProgramsSessionManager() = default;

    void begin_update() override
    {
        if (this->drawable) {
            this->drawable->begin_update();
        }
    }

    void end_update() override
    {
        if (this->drawable) {
            this->drawable->end_update();
        }
    }

    void disable_graphics_update(bool disable)
    {
        this->graphics_update_disabled = disable;

        LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO,
            "RemoteProgramsSessionManager::disable_graphics_update: "
                "graphics_update_disabled=%s",
            (this->graphics_update_disabled ? "yes" : "no"));

        if (!disable) {
            if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->dialog_box_window_id) {
                this->dialog_box_destroy();
            }

            if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->auxiliary_window_id) {
                this->destroy_auxiliary_window();
            }
        }
    }

public:
    bool is_server_only_window(uint32_t window_id) const {
        return (this->blocked_server_window_id == window_id);
    }

    void set_drawable(gdi::GraphicApi * drawable) {
        this->drawable = drawable;
    }

    void input_mouse(int device_flags, int x, int y) {
        if (DialogBoxType::WAITING_SCREEN != this->dialog_box_type) {
            return;
        }

        if (device_flags & SlowPath::PTRFLAGS_BUTTON1) {
            this->disconnect_now_button_clicked = false;

            if (device_flags & SlowPath::PTRFLAGS_DOWN) {
                if (this->disconnect_now_button_rect.contains_pt(x, y)) {
                    this->disconnect_now_button_clicked = true;
                }
            }

            this->waiting_screen_draw(this->disconnect_now_button_clicked ? 1 : 0);

            if (!(device_flags & SlowPath::PTRFLAGS_DOWN)
             && this->disconnect_now_button_rect.contains_pt(x, y)
            ) {
                LOG(LOG_INFO, "RemoteApp session initiated disconnect by user");
                throw Error(ERR_DISCONNECT_BY_USER);
            }
        }
    }

    void input_scancode(long param1, long param2, long device_flags) {
        if (DialogBoxType::WAITING_SCREEN != this->dialog_box_type) {
            return;
        }

        (void)param2;
        // 28 for escape key
        if ((28 == param1) && !(device_flags & SlowPath::KBDFLAGS_RELEASE)) {
            LOG(LOG_INFO, "RemoteApp session initiated disconnect by user");
            throw Error(ERR_DISCONNECT_BY_USER);
        }
    }

private:
    struct InternalProtectedGraphics final : gdi::ProtectedGraphics
    {
        RemoteProgramsSessionManager & manager_;

        explicit InternalProtectedGraphics(RemoteProgramsSessionManager & self)
        : gdi::ProtectedGraphics(*self.drawable, self.protected_rect)
        , manager_(self)
        {}

        void refresh_rects(array_view<Rect> av) override
        { this->manager_.mod.rdp_input_invalidate2(av); }
    };

    template<class Cmd, class... Args>
    void draw_impl(Cmd const & cmd, Args const &... args) {
        if (this->drawable) {
            InternalProtectedGraphics(*this).draw(cmd, args...);
        }
    }

public:
    void draw(RDP::RAIL::WindowIcon const & order) override {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(WindowIcon): Order bloacked.");
            }
        }
    }

    void draw(RDP::RAIL::CachedIcon const & order) override {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(CachedIcon): Order bloacked.");
            }
        }
    }

    void draw(RDP::RAIL::DeletedWindow const & order) override {
// LOG(LOG_INFO, "RemoteProgramsSessionManager::draw(DeletedWindow)");
        const uint32_t window_id         = order.header.WindowId();
        const bool     window_is_blocked = (window_id == this->blocked_server_window_id);

        if (this->drawable) {
            if (!window_is_blocked) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedWindow): Order bloacked.");
            }
        }

        if (window_is_blocked) {
            this->unregister_server_window(window_id);

            LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedWindow): Remove window 0x%X from blocked windows list.", window_id);

            this->blocked_server_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;
        }
    }

    void draw(RDP::RAIL::NewOrExistingWindow const & order) override {
// LOG(LOG_INFO, "RemoteProgramsSessionManager::draw(NewOrExistingWindow)");
        const std::string& title_info    = order.TitleInfo();
        const uint32_t window_id         = order.header.WindowId();
              bool     window_is_blocked = (window_id == this->blocked_server_window_id);
        const bool     window_is_new     = (RDP::RAIL::WINDOW_ORDER_STATE_NEW & order.header.FieldsPresentFlags());

        if (window_is_new) {
            if (DialogBoxType::WAITING_SCREEN == this->dialog_box_type) {
                this->dialog_box_destroy();
            }

            this->currently_without_window = false;
        }

        //if (bool(this->verbose & RDPVerbose::rail)) {
        //    LOG(LOG_INFO,
        //        "RemoteProgramsSessionManager::draw(NewOrExistingWindow): "
        //            "TitleInfo=\"%s\" WindowIsNew=%s GraphicsUpdateDisabled=%s",
        //        title_info, (window_is_new ? "yes" : "no"),
        //        (this->graphics_update_disabled ? "yes" : "no"));
        //}

        if ((RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == this->blocked_server_window_id) &&
            this->graphics_update_disabled) {
            assert(!window_is_blocked);

            const char*  blocked_window_title        = this->session_probe_window_title.c_str();
            const size_t blocked_window_title_length = this->session_probe_window_title.length();

            const size_t title_info_length = title_info.size();

            if ((title_info_length >= blocked_window_title_length) &&
                !::strcmp(title_info.c_str() + (title_info_length - blocked_window_title_length), blocked_window_title)) {
                if (window_is_new) {
                    {
                        RAILPDUHeader rpduh;

                        ClientSystemCommandPDU cscpdu;
                        cscpdu.WindowId(this->get_client_window_id_ex(window_id));
                        cscpdu.Command(SC_MINIMIZE);

                        StaticOutStream<1024> out_s;

                        rpduh.emit_begin(out_s, TS_RAIL_ORDER_SYSCOMMAND);

                        cscpdu.emit(out_s);

                        rpduh.emit_end();

                        InStream in_s(out_s.get_produced_bytes());

                        this->mod.send_to_mod_channel(channel_names::rail,
                                                      in_s,
                                                      out_s.get_offset(),
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                      | CHANNELS::CHANNEL_FLAG_LAST
                                                      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

                        LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw(NewOrExistingWindow): Window 0x%X is minimized.", window_id);
                    }
                }
                else {
                    {
                        RDP::RAIL::DeletedWindow order;

                        order.header.FieldsPresentFlags(
                            uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)
                          | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW));
                        order.header.WindowId(window_id);

                        if (bool(this->verbose & RDPVerbose::rail)) {
                            StaticOutStream<1024> out_s;
                            order.emit(out_s);
                            order.log(LOG_INFO);
                        }

                        this->front.draw(order);

                        LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw(NewOrExistingWindow): Window 0x%X is deletec.", window_id);
                    }
                }

                this->blocked_server_window_id = window_id;

                window_is_blocked = true;

                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw(NewOrExistingWindow): Window 0x%X is blocked.", window_id);

                this->dialog_box_create(DialogBoxType::SPLASH_SCREEN);

                this->splash_screen_draw();
            }
        }

        if (this->drawable) {
            if (!window_is_blocked) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingWindow): Order bloacked.");
            }
        }
    }

    void draw(RDP::RAIL::NewOrExistingNotificationIcons const & order) override {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingNotificationIcons): Order bloacked.");
            }
        }
    }

    void draw(RDP::RAIL::DeletedNotificationIcons const & order) override {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedNotificationIcons): Order bloacked.");
            }
        }
    }

    void draw(RDP::RAIL::ActivelyMonitoredDesktop const & order) override {
        this->has_actively_monitored_desktop = true;

        bool has_not_window =
            ((RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER & order.header.FieldsPresentFlags()) &&
             !order.NumWindowIds());
        bool has_window     = false;

        if (this->drawable) {
            if (order.ActiveWindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);

                has_window =
                    ((RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER & order.header.FieldsPresentFlags()) &&
                     order.NumWindowIds());
            }
            else {
                LOG_IF(bool(this->verbose & RDPVerbose::rail), LOG_INFO, "RemoteProgramsSessionManager::draw(ActivelyMonitoredDesktop): Order bloacked.");
            }
        }

        if (has_not_window && (DialogBoxType::NONE == this->dialog_box_type)
        && this->has_previous_window)
        {
            this->currently_without_window = true;
            this->events_guard.create_event_timeout(
                "Rail Waiting Screen Event",
                this->rail_disconnect_message_delay,
                [this](Event&event)
                {
                    if (this->currently_without_window
                     && (DialogBoxType::NONE == this->dialog_box_type)
                     && this->has_previous_window
                     && this->has_actively_monitored_desktop
                    ) {
                        LOG(LOG_INFO, "RemoteProgramsSessionManager::draw(ActivelyMonitoredDesktop): Create waiting screen.");
                        this->dialog_box_create(DialogBoxType::WAITING_SCREEN);
                        this->waiting_screen_draw(0);
                    }
                    event.garbage = true;
                });
        }

        if (has_window) {
            this->has_previous_window = true;
        }
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {
        this->has_actively_monitored_desktop = false;

        if (this->drawable) {
            this->drawable->draw(order);
        }
    }

private:
    void dialog_box_create(DialogBoxType type) {
        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->dialog_box_window_id) return;

        this->dialog_box_window_id = this->register_client_window();

        Rect mod_window_rect = this->rail_client_execute->get_window_rect();

        Rect dialog_box_rect(
                mod_window_rect.x + (mod_window_rect.cx - 640) / 2,
                mod_window_rect.y + (mod_window_rect.cy - 480) / 2,
                640,
                480
            );

        this->protected_rect = dialog_box_rect;

        Point window_offset = this->rail_client_execute->get_window_offset();

        const Rect adjusted_protected_rect = this->protected_rect.offset(
                    window_offset.x, window_offset.y);

        {
            RDP::RAIL::NewOrExistingWindow order;

            order.header.FieldsPresentFlags(
                      RDP::RAIL::WINDOW_ORDER_STATE_NEW
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                    | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                    | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_TITLE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_OWNER
                );
            order.header.WindowId(this->dialog_box_window_id);

            order.OwnerWindowId(0x0);
            order.Style(0x14EE0000);
            order.ExtendedStyle(0x40310 | 0x8);
            order.ShowState(5);
            order.TitleInfo("Dialog box");
            order.ClientOffsetX(adjusted_protected_rect.x + 6);
            order.ClientOffsetY(adjusted_protected_rect.y + 25);
            order.WindowOffsetX(adjusted_protected_rect.x);
            order.WindowOffsetY(adjusted_protected_rect.y);
            order.WindowClientDeltaX(6);
            order.WindowClientDeltaY(25);
            order.WindowWidth(adjusted_protected_rect.cx);
            order.WindowHeight(adjusted_protected_rect.cy);
            order.VisibleOffsetX(adjusted_protected_rect.x);
            order.VisibleOffsetY(adjusted_protected_rect.y);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_protected_rect.cx, adjusted_protected_rect.cy));

            if (bool(this->verbose & RDPVerbose::rail)) {
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "RemoteProgramsSessionManager::dialog_box_create: Send NewOrExistingWindow to client: size=%zu", out_s.get_offset() - 1);
            }

            this->drawable->draw(order);
        }

        if (DialogBoxType::WAITING_SCREEN == type) {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                    RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER
                );

            order.NumWindowIds(1);
            order.window_ids(0, this->dialog_box_window_id);

            if (bool(this->verbose & RDPVerbose::rail)) {
                StaticOutStream<256> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "RemoteProgramsSessionManager::dialog_box_create: Send ActivelyMonitoredDesktop to client: size=%zu", out_s.get_offset() - 1);
            }

            this->drawable->draw(order);
        }

        this->dialog_box_type = type;

        this->disconnect_now_button_rect = Rect();
    }

    void dialog_box_destroy() {
        assert(this->dialog_box_window_id);

        {
            RDP::RAIL::DeletedWindow order;

            order.header.FieldsPresentFlags(
                uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)
              | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW));
            order.header.WindowId(this->dialog_box_window_id);

            if (bool(this->verbose & RDPVerbose::rail)) {
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "RemoteProgramsSessionManager::dialog_box_destroy: Send DeletedWindow to client: size=%zu", out_s.get_offset() - 1);
            }

            this->front.draw(order);
        }

        this->mod.rdp_input_invalidate(this->protected_rect);

        this->protected_rect = Rect();

        this->dialog_box_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

        this->dialog_box_type = DialogBoxType::NONE;

        this->disconnect_now_button_rect    = Rect();
        this->disconnect_now_button_clicked = false;
    }

    void splash_screen_draw() {
        if (!this->drawable) return;

        this->drawable->begin_update();

        {
            RDPOpaqueRect order(this->protected_rect, encode_color24()(BLACK));

            this->drawable->draw(order, this->protected_rect, gdi::ColorCtx::depth24());
        }

        {
            Rect rect = this->protected_rect.shrink(1);

            RDPOpaqueRect order(rect, encode_color24()(this->theme.global.bgcolor));
            if (bool(this->verbose & RDPVerbose::rail)) {
                order.log(LOG_INFO, rect);
            }

            this->drawable->draw(order, rect, gdi::ColorCtx::depth24());
        }

        gdi::TextMetrics tm(this->font, TR(trkeys::starting_remoteapp, this->lang));
        gdi::server_draw_text(*this->drawable,
                              this->font,
                              this->protected_rect.x + (this->protected_rect.cx - tm.width) / 2,
                              this->protected_rect.y + (this->protected_rect.cy - tm.height) / 2,
                              TR(trkeys::starting_remoteapp, this->lang),
                              encode_color24()(this->theme.global.fgcolor),
                              encode_color24()(this->theme.global.bgcolor),
                              gdi::ColorCtx::depth24(),
                              this->protected_rect
                              );

        this->drawable->end_update();
    }

    void waiting_screen_draw(int state) {
        if (!this->drawable) return;

        this->drawable->begin_update();

        {
            RDPOpaqueRect order(this->protected_rect, encode_color24()(BLACK));

            this->drawable->draw(order, this->protected_rect, gdi::ColorCtx::depth24());
        }

        {
            Rect rect = this->protected_rect.shrink(1);

            RDPOpaqueRect order(rect, encode_color24()(this->theme.global.bgcolor));
            if (bool(this->verbose & RDPVerbose::rail)) {
                order.log(LOG_INFO, rect);
            }

            this->drawable->draw(order, rect, gdi::ColorCtx::depth24());
        }

        const gdi::TextMetrics tm_msg(this->font, TR(trkeys::closing_remoteapp, this->lang));

        const int xtext = 6;
        const int ytext = 2;

        const Dimension dim_button = WidgetFlatButton::get_optimal_dim(2, this->font, TR(trkeys::disconnect_now, this->lang), xtext, ytext);

        const uint32_t interspace = 60;

        const uint32_t height = tm_msg.height + interspace + dim_button.h;

        int ypos = this->protected_rect.y + (this->protected_rect.cy - height) / 2;

        gdi::server_draw_text(*this->drawable,
                              this->font,
                              this->protected_rect.x + (this->protected_rect.cx - tm_msg.width) / 2,
                              ypos,
                              TR(trkeys::closing_remoteapp, this->lang),
                              encode_color24()(this->theme.global.fgcolor),
                              encode_color24()(this->theme.global.bgcolor),
                              gdi::ColorCtx::depth24(),
                              this->protected_rect
                              );

        ypos += (tm_msg.height + interspace);

        this->disconnect_now_button_rect.x  = this->protected_rect.x + (this->protected_rect.cx - dim_button.w) / 2;
        this->disconnect_now_button_rect.y  = ypos;
        this->disconnect_now_button_rect.cx = dim_button.w;
        this->disconnect_now_button_rect.cy = dim_button.h;

        WidgetFlatButton::draw(this->protected_rect,
                               this->disconnect_now_button_rect,
                               *this->drawable,
                               false,   // logo
                               true,    // has_focus
                               TR(trkeys::disconnect_now, this->lang),
                               encode_color24()(this->theme.global.fgcolor),
                               encode_color24()(this->theme.global.bgcolor),
                               encode_color24()(this->theme.global.focus_color),
                               gdi::ColorCtx::depth24(),
                               Rect(),
                               state,
                               2,
                               this->font,
                               xtext,
                               ytext
                               );

        this->drawable->end_update();
    }

    ///////////////////
    // windowing_api
    //

public:
    void create_auxiliary_window(Rect const window_rect) override {
        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->auxiliary_window_id) return;

        this->auxiliary_window_id = this->register_client_window();

        {
            Point window_offset = this->rail_client_execute->get_window_offset();

            const Rect adjusted_window_rect = window_rect.offset(
                        window_offset.x, window_offset.y);

            RDP::RAIL::NewOrExistingWindow order;

            order.header.FieldsPresentFlags(
                      RDP::RAIL::WINDOW_ORDER_STATE_NEW
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                    | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                    | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_TITLE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_OWNER
                );
            order.header.WindowId(this->auxiliary_window_id);

            order.OwnerWindowId(0x0);
            order.Style(0x14EE0000);
            order.ExtendedStyle(0x40310 | 0x8);
            order.ShowState(5);
            order.TitleInfo("Dialog box");
            order.ClientOffsetX(adjusted_window_rect.x + 6);
            order.ClientOffsetY(adjusted_window_rect.y + 25);
            order.WindowOffsetX(adjusted_window_rect.x);
            order.WindowOffsetY(adjusted_window_rect.y);
            order.WindowClientDeltaX(6);
            order.WindowClientDeltaY(25);
            order.WindowWidth(adjusted_window_rect.cx);
            order.WindowHeight(adjusted_window_rect.cy);
            order.VisibleOffsetX(adjusted_window_rect.x);
            order.VisibleOffsetY(adjusted_window_rect.y);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

            if (bool(this->verbose & RDPVerbose::rail)) {
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "RemoteProgramsSessionManager::dialog_box_create: Send NewOrExistingWindow to client: size=%zu", out_s.get_offset() - 1);
            }

            this->drawable->draw(order);
        }
    }

    void destroy_auxiliary_window() override {
        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == this->auxiliary_window_id) return;

        {
            RDP::RAIL::DeletedWindow order;

            order.header.FieldsPresentFlags(
                uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)
              | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW));
            order.header.WindowId(this->auxiliary_window_id);

            if (bool(this->verbose & RDPVerbose::rail)) {
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "RemoteProgramsSessionManager::destroy_auxiliary_window: Send DeletedWindow to client: size=%zu", out_s.get_offset() - 1);
            }

            this->front.draw(order);
        }

        this->auxiliary_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;
    }
};  // class RemoteProgramsSessionManager
