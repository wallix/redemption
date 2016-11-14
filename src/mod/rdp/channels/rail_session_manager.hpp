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
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "utils/protect_graphics.hpp"
#include "utils/rect.hpp"
#include "utils/theme.hpp"
#include "utils/translation.hpp"

#include <set>

class RemoteProgramsSessionManager : public gdi::GraphicBase<RemoteProgramsSessionManager> {
private:
    FrontAPI & front;
    mod_api  & mod;

    Translation::language_t lang;

    uint16_t front_width;
    uint16_t front_height;

    Font  const & font;
    Theme const & theme;

    uint32_t verbose;

    std::set<uint32_t> blocked_windows;

    bool graphics_update_disabled = false;

    Rect splash_screen_rect;
    Rect protected_rect;

    uint32_t splash_screen_window_id = 0;

    gdi::GraphicApi * drawable = nullptr;
    int               bpp      = 0;

public:
    RemoteProgramsSessionManager(FrontAPI& front, mod_api& mod, Translation::language_t lang,
                                 uint16_t front_width, uint16_t front_height,
                                 Font const & font, Theme const & theme,
                                 uint32_t verbose)
    : front(front)
    , mod(mod)
    , lang(lang)
    , front_width(front_width)
    , front_height(front_height)
    , font(font)
    , theme(theme)
    , verbose(verbose)
    , splash_screen_rect((front_width - 640) / 2, (front_height - 480) / 2, 640, 480) {}

    void disable_graphics_update(bool disable) {
        this->graphics_update_disabled = disable;

        LOG(LOG_INFO,
            "RemoteProgramsSessionManager::disable_graphics_update: "
                "graphics_update_disabled=%s",
            (this->graphics_update_disabled ? "yes" : "no"));

        if (!disable && this->splash_screen_window_id) {
            this->splash_screen_destroy();
        }
    }

private:
    bool is_window_blocked(uint32_t window_id) {
        return (this->blocked_windows.find(window_id) != this->blocked_windows.end());
    }

public:
    void set_drawable(gdi::GraphicApi * drawable, int bpp) {
        this->drawable = drawable;
        this->bpp      = bpp;
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
            if (!this->is_window_blocked(order.header.WindowId())) {
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(WindowIcon): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::CachedIcon const & order) {
        if (this->drawable) {
            if (!this->is_window_blocked(order.header.WindowId())) {
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(CachedIcon): Order bloacked.");
            }
        }
    }

    void draw_impl(RDP::RAIL::DeletedWindow const & order) {
        const uint32_t window_id         = order.header.WindowId();
        const bool     window_is_blocked = this->is_window_blocked(window_id);

        if (this->drawable) {
            if (!window_is_blocked) {
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedWindow): Order bloacked.");
            }
        }

        if (window_is_blocked) {
            this->blocked_windows.erase(window_id);

            LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(DeletedWindow): Remove window 0x%X from blocked windows list.", window_id);
        }
    }

    void draw_impl(RDP::RAIL::NewOrExistingWindow const & order) {
        const char *   title_info        = order.TitleInfo();
        const uint32_t window_id         = order.header.WindowId();
              bool     window_is_blocked = this->is_window_blocked(window_id);

        const char session_probe_window_title[] = "SesProbe";

        if (this->graphics_update_disabled &&
            (RDP::RAIL::WINDOW_ORDER_STATE_NEW & order.header.FieldsPresentFlags())) {
            REDASSERT(!window_is_blocked);

            const size_t title_info_length = ::strlen(title_info);

            if ((title_info_length >= sizeof(session_probe_window_title) - 1) &&
                !::strcmp(title_info + (title_info_length - sizeof(session_probe_window_title) + 1), session_probe_window_title)) {
                this->blocked_windows.insert(window_id);

                REDASSERT(this->is_window_blocked(window_id));

                window_is_blocked = true;

                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingWindow): Added window 0x%X to blocked windows list.", window_id);

                {
                    RAILPDUHeader rpduh;

                    ClientSystemCommandPDU cscpdu;
                    cscpdu.WindowId(window_id);
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

                this->splash_screen_create(window_id);
                this->splash_screen_draw();
            }
        }

        if (this->drawable) {
            if (!window_is_blocked) {
                this->drawable->draw(order);
            }
            else {
                LOG(LOG_INFO, "RemoteProgramsSessionManager::draw_impl(NewOrExistingWindow): Order bloacked.");
            }
        }
    }

    void splash_screen_create(uint32_t window_id) {
        REDASSERT(!this->splash_screen_window_id);

        this->splash_screen_window_id = window_id;

        this->protected_rect = this->splash_screen_rect;

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
            order.header.WindowId(this->splash_screen_window_id);

            order.OwnerWindowId(0x0);
            order.Style(0x14EE0000);
            order.ExtendedStyle(0x40310);
            order.ShowState(5);
            order.TitleInfo("Splash screen");
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
                LOG(LOG_INFO, "RemoteProgramsSessionManager::splash_screen_create: Send NewOrExistingWindow to client: size=%zu", out_s.get_offset() - 1);
            }

            this->front.draw(order);
        }
    }

    void splash_screen_destroy() {
        REDASSERT(this->splash_screen_window_id);

        {
            RDP::RAIL::DeletedWindow order;

            order.header.FieldsPresentFlags(
                      RDP::RAIL::WINDOW_ORDER_STATE_DELETED
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                );
            order.header.WindowId(this->splash_screen_window_id);

            /*if (this->verbose & MODRDP_LOGLEVEL_RAIL) */{
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "RemoteProgramsSessionManager::splash_screen_destroy: Send DeletedWindow to client: size=%zu", out_s.get_offset() - 1);
            }

            this->front.draw(order);
        }

        this->protected_rect = Rect();

        this->splash_screen_window_id = 0;
    }

    void splash_screen_draw() {
        if (!this->drawable) return;

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
    }
};  // class RemoteProgramsSessionManager