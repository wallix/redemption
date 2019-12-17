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
#include "acl/mod_wrapper.hpp"

class ModOSD : public gdi::ProtectedGraphics, public mod_api
{
    const ModWrapper & mod_wrapper;
    FrontAPI & front;
    BGRPalette const & palette;
    gdi::GraphicApi& graphics;
    ClientInfo const & client_info;
    ClientExecute & rail_client_execute;
    windowing_api* &winapi;
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
    explicit ModOSD(const ModWrapper & mod_wrapper, FrontAPI & front, BGRPalette const & palette, gdi::GraphicApi& graphics, ClientInfo const & client_info, const Font & glyphs, const Theme & theme, ClientExecute & rail_client_execute, windowing_api* & winapi, Inifile & ini)
    : gdi::ProtectedGraphics(graphics, Rect{})
    , mod_wrapper(mod_wrapper)
    , front(front)
    , palette(palette)
    , graphics(graphics)
    , client_info(client_info)
    , rail_client_execute(rail_client_execute)
    , winapi(winapi)
    , ini(ini)
    , bogus_refresh_rect_ex(false)
    , glyphs(glyphs)
    , theme(theme)
    {}

    [[nodiscard]] bool is_input_owner() const { return this->is_disable_by_input; }

    void disable_osd()
    {
        this->is_disable_by_input = false;
        auto const protected_rect = this->get_protected_rect();
        this->set_protected_rect(Rect{});

        if (this->bogus_refresh_rect_ex) {
            this->mod_wrapper.mod->rdp_suppress_display_updates();
            this->mod_wrapper.mod->rdp_allow_display_updates(0, 0,
                this->client_info.screen_info.width,
                this->client_info.screen_info.height);
        }

        if (this->winapi) {
            this->winapi->destroy_auxiliary_window();
        }

        this->mod_wrapper.mod->rdp_input_invalidate(protected_rect);
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
        this->graphics.begin_update();
        this->draw_osd_message_impl(this->graphics);
        this->graphics.end_update();
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
                this->mod_wrapper.mod->rdp_input_invalidate2({p, e});
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
                this->mod_wrapper.mod->rdp_input_invalidate(r);
            }
            else {
                ret = true;
            }
        }
        return ret;
    }

private:
    void draw_osd_message_impl(gdi::GraphicApi & drawable)
    {
        if (this->clip.isempty()) {
            return ;
        }

        auto const color_ctx = gdi::ColorCtx::from_bpp(this->client_info.screen_info.bpp, this->palette);

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

    void refresh_rects(array_view<Rect const> av) override
    {
        this->mod_wrapper.mod->rdp_input_invalidate2(av);
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        if (!this->try_input_scancode(param1, param2, param3, param4, keymap)) {
            this->mod_wrapper.mod->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        this->mod_wrapper.mod->rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        if (!this->try_input_mouse(device_flags, x, y, keymap)) {
            this->mod_wrapper.mod->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_invalidate(Rect r) override
    {
        if (!this->try_input_invalidate(r)) {
            this->mod_wrapper.mod->rdp_input_invalidate(r);
        }
    }

    void rdp_input_invalidate2(array_view<Rect const> vr) override
    {
        if (!this->try_input_invalidate2(vr)) {
            this->mod_wrapper.mod->rdp_input_invalidate2(vr);
        }
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    { this->mod_wrapper.mod->rdp_input_synchronize(time, device_flags, param1, param2); }

    void rdp_input_up_and_running() override
    { this->mod_wrapper.mod->rdp_input_up_and_running(); }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) override
    { this->mod_wrapper.mod->rdp_allow_display_updates(left, top, right, bottom); }

    void rdp_suppress_display_updates() override
    { this->mod_wrapper.mod->rdp_suppress_display_updates(); }

    void refresh(Rect r) override
    {
        this->mod_wrapper.mod->refresh(r);
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags
    ) override
    { this->mod_wrapper.mod->send_to_mod_channel(front_channel_name, chunk, length, flags); }

    void send_auth_channel_data(const char * data) override
    { this->mod_wrapper.mod->send_auth_channel_data(data); }

    void send_checkout_channel_data(const char * data) override
    { this->mod_wrapper.mod->send_checkout_channel_data(data); }

    [[nodiscard]] bool is_up_and_running() const override
    { return this->mod_wrapper.mod->is_up_and_running(); }

    void disconnect() override
    { this->mod_wrapper.mod->disconnect(); }

    void display_osd_message(std::string const & message) override
    { this->mod_wrapper.mod->display_osd_message(message); }

    [[nodiscard]] Dimension get_dim() const override
    {
        return this->mod_wrapper.mod->get_dim();
    }
};

