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

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/front_api.hpp"
#include "core/RDP/remote_programs.hpp"
#include "gdi/clip_from_cmd.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/internal/widget2/flat_button.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "utils/protect_graphics.hpp"
#include "utils/rect.hpp"
#include "utils/theme.hpp"
#include "utils/translation.hpp"

#include <set>
#include <string>

class RemoteProgramsSessionManager : public gdi::GraphicBase<RemoteProgramsSessionManager>,
    public RemoteProgramsWindowIdManager,
    public windowing_api {

private:
    FrontAPI & front;
    mod_api  & mod;

    Translation::language_t lang;

    uint16_t front_width;
    uint16_t front_height;

    Font  const & font;
    Theme const & theme;

    const implicit_bool_flags<RDPVerbose> verbose;

    uint32_t blocked_server_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    bool graphics_update_disabled = false;

    Rect dialog_box_rect;
    Rect protected_rect;

    uint32_t dialog_box_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    gdi::GraphicApi * drawable = nullptr;
    int               bpp      = 0;

    enum DialogBoxType {
        SPLASH_SCREEN,
        WAITING_SCREEN,

        NONE
    } dialog_box_type = DialogBoxType::NONE;

    Rect disconnect_now_button_rect;
    bool disconnect_now_button_clicked = false;

    auth_api* acl = nullptr;

    bool has_previous_window = false;

    std::string session_probe_window_title;

    uint32_t auxiliary_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

public:
    RemoteProgramsSessionManager(FrontAPI& front, mod_api& mod, Translation::language_t lang,
                                 uint16_t front_width, uint16_t front_height,
                                 Font const & font, Theme const & theme, auth_api* acl,
                                 char const* session_probe_window_title, RDPVerbose verbose)
    : front(front)
    , mod(mod)
    , lang(lang)
    , front_width(front_width)
    , front_height(front_height)
    , font(font)
    , theme(theme)
    , verbose(verbose)
    , dialog_box_rect((front_width - 640) / 2, (front_height - 480) / 2, 640, 480)
    , acl(acl)
    , session_probe_window_title(session_probe_window_title) {}

    void disable_graphics_update(bool disable) {
        this->graphics_update_disabled = disable;

        LOG(LOG_INFO,
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

    void set_drawable(gdi::GraphicApi * drawable, int bpp) {
        this->drawable = drawable;
        this->bpp      = bpp;
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

            if (!(device_flags & SlowPath::PTRFLAGS_DOWN) &&
                (this->disconnect_now_button_rect.contains_pt(x, y))) {
                LOG(LOG_INFO, "RemoteApp session initiated disconnect by user");
                if (this->acl) {
                    this->acl->disconnect_target();
                }
                throw Error(ERR_DISCONNECT_BY_USER);
            }
        }
    }

    void input_scancode(long param1, long param2, long device_flags) {
        if (DialogBoxType::WAITING_SCREEN != this->dialog_box_type) {
            return;
        }

        (void)param2;
        if ((28 == param1) && !(device_flags & SlowPath::KBDFLAGS_RELEASE)) {
            LOG(LOG_INFO, "RemoteApp session initiated disconnect by user");
            if (this->acl) {
                this->acl->disconnect_target();
            }
            throw Error(ERR_DISCONNECT_BY_USER);
        }
    }

private:
    friend gdi::GraphicCoreAccess;

    template<class Cmd>
    void draw_impl(Cmd const & cmd) {
        if (this->drawable) {
            this->drawable->draw(cmd);
        }
    }

    template<class Cmd, class... Args>
    void draw_impl(Cmd const & cmd, Rect const & clip, Args const &... args) {
        if (this->drawable) {
            Rect const rect = ::clip_from_cmd(cmd).intersect(clip);
            if (this->protected_rect.contains(rect) || rect.isempty()) {
                //nada
            }
            else if (rect.has_intersection(this->protected_rect)) {
                this->drawable->begin_update();
                // TODO used multi orders
                for (const Rect & subrect : subrect4(rect, this->protected_rect)) {
                    if (!subrect.isempty()) {
                        this->drawable->draw(cmd, subrect, args...);
                    }
                }
                this->drawable->end_update();
            }
            else {
                this->drawable->draw(cmd, clip, args...);
            }
        }
    }

    void draw_impl(RDPBitmapData const & bitmap_data, Bitmap const & bmp) {
        if (this->drawable) {
            Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                        , bitmap_data.dest_right - bitmap_data.dest_left + 1
                        , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

            if (rectBmp.has_intersection(this->protected_rect)) {
                this->drawable->begin_update();
                for (const Rect & subrect : subrect4(rectBmp, this->protected_rect)) {
                    if (!subrect.isempty()) {
                        this->drawable->draw(
                            RDPMemBlt(0, subrect, 0xCC, subrect.x - rectBmp.x, subrect.y - rectBmp.y, 0),
                            subrect, bmp
                        );
                    }
                }
                this->drawable->end_update();
            }
            else {
                this->drawable->draw(bitmap_data, bmp);
            }
        }
    }

    void draw_impl(const RDPScrBlt & cmd, const Rect & clip) {
        const Rect drect = cmd.rect.intersect(clip);
        const int deltax = cmd.srcx - cmd.rect.x;
        const int deltay = cmd.srcy - cmd.rect.y;
        const int srcx = drect.x + deltax;
        const int srcy = drect.y + deltay;
        const Rect srect(srcx, srcy, drect.cx, drect.cy);

        const bool has_dest_intersec_fg = drect.has_intersection(this->protected_rect);
        const bool has_src_intersec_fg = srect.has_intersection(this->protected_rect);

        if (!has_dest_intersec_fg && !has_src_intersec_fg) {
            this->drawable->draw(cmd, clip);
        }
        else {
            this->drawable->begin_update();
            subrect4_t rects = subrect4(drect, this->protected_rect);
            auto e = std::remove_if(rects.begin(), rects.end(), [](const Rect & rect) { return !rect.isempty(); });
            auto av = make_array_view(rects.begin(), e);
            this->mod.rdp_input_invalidate2(av);
            this->drawable->end_update();
        }
    }

    void draw_impl(RDP::RAIL::WindowIcon const & order) {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(WindowIcon): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::CachedIcon const & order) {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(CachedIcon): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::DeletedWindow const & order) {
        const uint32_t window_id         = order.header.WindowId();
        const bool     window_is_blocked = (window_id == this->blocked_server_window_id);

        if (this->drawable) {
            if (!window_is_blocked) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedWindow): Order bloacked.");
            }
        }

        if (window_is_blocked) {
            this->unregister_server_window(window_id);

            LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedWindow): Remove window 0x%X from blocked windows list.", window_id);

            this->blocked_server_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;
        }
    }

    void draw_impl(RDP::RAIL::NewOrExistingWindow const & order) {
        const char *   title_info        = order.TitleInfo();
        const uint32_t window_id         = order.header.WindowId();
              bool     window_is_blocked = (window_id == this->blocked_server_window_id);
        const bool     window_is_new     = (RDP::RAIL::WINDOW_ORDER_STATE_NEW & order.header.FieldsPresentFlags());

        const char*  blocked_window_title        = this->session_probe_window_title.c_str();
        const size_t blocked_window_title_length = this->session_probe_window_title.length();

        if (window_is_new &&
            (DialogBoxType::WAITING_SCREEN == this->dialog_box_type)) {

            this->dialog_box_destroy();
        }

        if (this->graphics_update_disabled && window_is_new) {
            REDASSERT(!window_is_blocked);

            const size_t title_info_length = ::strlen(title_info);

            if ((title_info_length >= blocked_window_title_length) &&
                !::strcmp(title_info + (title_info_length - blocked_window_title_length), blocked_window_title)) {

                REDASSERT(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == this->blocked_server_window_id);

                {
                    RAILPDUHeader rpduh;

                    ClientSystemCommandPDU cscpdu;
                    cscpdu.WindowId(this->get_client_window_id_ex(window_id));
                    cscpdu.Command(SC_MINIMIZE);

                    StaticOutStream<1024> out_s;

                    rpduh.emit_begin(out_s, TS_RAIL_ORDER_SYSCOMMAND);

                    cscpdu.emit(out_s);

                    rpduh.emit_end();

                    const size_t totalLength = out_s.get_offset();

                    InStream in_s(out_s.get_data(), totalLength);

                    this->mod.send_to_mod_channel(channel_names::rail,
                                                  in_s,
                                                  totalLength,
                                                    CHANNELS::CHANNEL_FLAG_FIRST
                                                  | CHANNELS::CHANNEL_FLAG_LAST);
                }

                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingWindow): Window 0x%X is minimized.", window_id);

                this->blocked_server_window_id = window_id;

                window_is_blocked = true;

                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingWindow): Window 0x%X is blocked.", window_id);

                this->dialog_box_create(RemoteProgramsSessionManager::SPLASH_SCREEN);

                this->splash_screen_draw();
            }
        }

        if (this->drawable) {
            if (!window_is_blocked) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingWindow): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::NewOrExistingNotificationIcons const & order) {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingNotificationIcons): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::DeletedNotificationIcons const & order) {
        if (this->drawable) {
            if (order.header.WindowId() != this->blocked_server_window_id) {
                order.map_window_id(*this);
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedNotificationIcons): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::ActivelyMonitoredDesktop const & order) {
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
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedNotificationIcons): Order bloacked.");
            }
        }

        if (has_not_window && (DialogBoxType::NONE == this->dialog_box_type) &&
            this->has_previous_window) {

            this->dialog_box_create(DialogBoxType::WAITING_SCREEN);

            this->waiting_screen_draw(0);
        }

        if (has_window) {
            this->has_previous_window = true;
        }
    }

    void dialog_box_create(DialogBoxType type) {
        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->dialog_box_window_id) return;

        this->dialog_box_window_id = this->register_client_window();

        this->protected_rect = this->dialog_box_rect;

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
            order.ExtendedStyle(0x40310);
            order.ShowState(5);
            order.TitleInfo("Dialog box");
            order.ClientOffsetX(this->protected_rect.x + 6);
            order.ClientOffsetY(this->protected_rect.y + 25);
            order.WindowOffsetX(this->protected_rect.x);
            order.WindowOffsetY(this->protected_rect.y);
            order.WindowClientDeltaX(6);
            order.WindowClientDeltaY(25);
            order.WindowWidth(this->protected_rect.cx);
            order.WindowHeight(this->protected_rect.cy);
            order.VisibleOffsetX(this->protected_rect.x);
            order.VisibleOffsetY(this->protected_rect.y);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, this->protected_rect.cx, this->protected_rect.cy));

            /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
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

            /*if (this->verbose & MODINTERNAL_LOGLEVEL_CLIENTEXECUTE) */{
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
        REDASSERT(this->dialog_box_window_id);

        {
            RDP::RAIL::DeletedWindow order;

            order.header.FieldsPresentFlags(
                      RDP::RAIL::WINDOW_ORDER_STATE_DELETED
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                );
            order.header.WindowId(this->dialog_box_window_id);

            /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
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

        this->dialog_box_type = RemoteProgramsSessionManager::NONE;

        this->disconnect_now_button_rect    = Rect();
        this->disconnect_now_button_clicked = false;
    }

    void splash_screen_draw() {
        if (!this->drawable) return;

        this->drawable->begin_update();

        {
            RDPOpaqueRect order(this->protected_rect, 0x000000);

            this->drawable->draw(order, this->protected_rect);
        }

        {
            Rect rect = this->protected_rect.shrink(1);

            RDPOpaqueRect order(rect, color_encode(this->theme.global.bgcolor, this->bpp));
            order.log(LOG_INFO, rect);

            this->drawable->draw(order, rect);
        }

        gdi::TextMetrics tm(this->font, TR("starting_remoteapp", this->lang));
        gdi::server_draw_text(*this->drawable,
                              this->font,
                              (this->front_width - tm.width) / 2,
                              (this->front_height - tm.height) / 2,
                              TR("starting_remoteapp", this->lang),
                              color_encode(this->theme.global.fgcolor, this->bpp),
                              color_encode(this->theme.global.bgcolor, this->bpp),
                              this->protected_rect
                              );

        this->drawable->end_update();
    }

    void waiting_screen_draw(int state) {
        if (!this->drawable) return;

        this->drawable->begin_update();

        {
            RDPOpaqueRect order(this->protected_rect, 0x000000);

            this->drawable->draw(order, this->protected_rect);
        }

        {
            Rect rect = this->protected_rect.shrink(1);

            RDPOpaqueRect order(rect, color_encode(this->theme.global.bgcolor, this->bpp));
            order.log(LOG_INFO, rect);

            this->drawable->draw(order, rect);
        }

        const gdi::TextMetrics tm_msg(this->font, TR("closing_remoteapp", this->lang));

        const int xtext = 6;
        const int ytext = 2;

        const Dimension dim_button = WidgetFlatButton::get_optimal_dim(this->font, TR("disconnect_now", this->lang), xtext, ytext);

        const uint32_t interspace = 60;

        const uint32_t height = tm_msg.height + interspace + dim_button.h;

        int ypos = (this->front_height - height) / 2;

        gdi::server_draw_text(*this->drawable,
                              this->font,
                              (this->front_width - tm_msg.width) / 2,
                              ypos,
                              TR("closing_remoteapp", this->lang),
                              color_encode(this->theme.global.fgcolor, this->bpp),
                              color_encode(this->theme.global.bgcolor, this->bpp),
                              this->protected_rect
                              );

        ypos += (tm_msg.height + interspace);

        this->disconnect_now_button_rect.x  = (this->front_width - dim_button.w) / 2;
        this->disconnect_now_button_rect.y  = ypos;
        this->disconnect_now_button_rect.cx = dim_button.w;
        this->disconnect_now_button_rect.cy = dim_button.h;

        WidgetFlatButton::draw(this->protected_rect,
                               this->disconnect_now_button_rect,
                               *this->drawable,
                               false,   // logo
                               true,    // has_focus
                               TR("disconnect_now", this->lang),
                               color_encode(this->theme.global.fgcolor, this->bpp),
                               color_encode(this->theme.global.bgcolor, this->bpp),
                               color_encode(this->theme.global.focus_color, this->bpp),
                               Rect(),
                               state,
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
    void create_auxiliary_window(Rect const& window_rect) override {
        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->auxiliary_window_id) return;

        this->auxiliary_window_id = this->register_client_window();

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
            order.header.WindowId(this->auxiliary_window_id);

            order.OwnerWindowId(0x0);
            order.Style(0x14EE0000);
            order.ExtendedStyle(0x40310 | 0x8);
            order.ShowState(5);
            order.TitleInfo("Dialog box");
            order.ClientOffsetX(window_rect.x + 6);
            order.ClientOffsetY(window_rect.y + 25);
            order.WindowOffsetX(window_rect.x);
            order.WindowOffsetY(window_rect.y);
            order.WindowClientDeltaX(6);
            order.WindowClientDeltaY(25);
            order.WindowWidth(window_rect.cx);
            order.WindowHeight(window_rect.cy);
            order.VisibleOffsetX(window_rect.x);
            order.VisibleOffsetY(window_rect.y);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, window_rect.cx, window_rect.cy));

            /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
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
                      RDP::RAIL::WINDOW_ORDER_STATE_DELETED
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                );
            order.header.WindowId(this->auxiliary_window_id);

            /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
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
