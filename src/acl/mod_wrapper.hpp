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

#include "RAIL/client_execute.hpp"
#include "acl/auth_api.hpp"
#include "acl/mod_pack.hpp"
#include "acl/module_manager/enums.hpp"
#include "acl/time_before_closing.hpp"
#include "keyboard/keymap.hpp"
#include "configs/config.hpp"
#include "gdi/osd_api.hpp"
#include "gdi/protected_graphics.hpp"
#include "gdi/text_metrics.hpp"
#include "gdi/subrect4.hpp"
#include "mod/null/null.hpp"
#include "utils/translation.hpp"
#include "utils/timebase.hpp"
#include "utils/ref.hpp"
#include "core/callback.hpp"
#include "core/client_info.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/slowpath.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"


class SocketTransport;
class rdp_api;

class ModWrapper final : public gdi::OsdApi, private Callback
{
    struct Graphics final : gdi::ProtectedGraphics
    {
        using gdi::ProtectedGraphics::ProtectedGraphics;
    } gfilter;

    bool connected = false;

public:
    // TODO should be private
    ModuleName current_mod = ModuleName::UNKNOWN;
    // TODO should be private
    bool target_info_is_shown = false;

private:
    bool enable_osd = false;

    ClientInfo const & client_info;
    ClientExecute & rail_client_execute;
    BGRPalette const & palette;

    rdp_api * rdpapi = nullptr;
    windowing_api * winapi = nullptr;

    Inifile & ini;

    std::string osd_message;
    bool allow_disable_osd_message;
    Rect clip;
    RDPColor color;
    RDPColor background_color;
    bool is_disable_by_input = false;
    bool bogus_refresh_rect_ex;
    const Font & glyphs;
    Keymap & keymap;

    SocketTransport * psocket_transport = nullptr;
    null_mod no_mod;
    not_null_ptr<mod_api> modi = &no_mod;

    MonotonicTimePoint end_time_session {};
    TimeBase const& time_base;

    gdi::MultiLineTextMetrics line_metrics;

public:
   explicit ModWrapper(
        CRef<TimeBase> time_base, BGRPalette const & palette, gdi::GraphicApi& graphics,
        Keymap & keymap, ClientInfo const & client_info, const Font & glyphs,
        ClientExecute & rail_client_execute, Inifile & ini)
    : gfilter(graphics, static_cast<RdpInput&>(*this), Rect{})
    , client_info(client_info)
    , rail_client_execute(rail_client_execute)
    , palette(palette)
    , ini(ini)
    , allow_disable_osd_message(false)
    , bogus_refresh_rect_ex(false)
    , glyphs(glyphs)
    , keymap(keymap)
    , time_base(time_base)
    {}

    ~ModWrapper()
    {
        if (this->modi != &this->no_mod){
            delete this->modi;
        }
    }

    void disconnect()
    {
        if (this->modi != &this->no_mod) {
            try {
                this->get_mod().disconnect();
            }
            catch (Error const& e) {
                LOG(LOG_ERR, "disconnect raised exception %d", static_cast<int>(e.id));
            }

            delete this->modi;
            this->modi = &this->no_mod;
            this->rdpapi = nullptr;
            this->winapi = nullptr;
            this->connected = false;
            this->psocket_transport = nullptr;
            this->current_mod = ModuleName::UNKNOWN;
        }
    }

    Callback & get_callback() noexcept
    {
        return static_cast<Callback&>(*this);
    }

    bool is_connected() const
    {
        return this->connected;
    }

    rdp_api* get_rdp_api() const
    {
        return this->rdpapi;
    }

    gdi::GraphicApi & get_graphics()
    {
        return this->gfilter;
    }

    void display_osd_message(std::string_view message,
                             gdi::OsdMsgUrgency omu = gdi::OsdMsgUrgency::NORMAL) override
    {
        if (message != this->osd_message) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            str_assign(this->osd_message,
                       message,
                       '\n',
                       TR(trkeys::disable_osd, language(this->ini)));
            this->allow_disable_osd_message = true;
            this->is_disable_by_input = true;
            this->prepare_osd_message(omu);
            this->draw_osd_message();
        }
    }

    void acl_update(AclFieldMask const& acl_fields)
    {
        this->get_mod().acl_update(acl_fields);
    }

    mod_api& get_mod()
    {
        return *this->modi;
    }

    [[nodiscard]] mod_api const& get_mod() const
    {
        return *this->modi;
    }

    BackEvent_t get_mod_signal()
    {
        return this->get_mod().get_mod_signal();
    }

    bool is_up_and_running() const
    {
        return (this->modi != &this->no_mod) && this->get_mod().is_up_and_running();
    }

    void set_mod(ModuleName next_state, ModPack mod_pack)
    {
        // LOG(LOG_INFO, "=================== Setting new mod %s (was %s)  psocket_transport = %p",
        this->keymap.reset_decoded_keys();

        this->clear_osd_message();

        if (this->modi != &this->no_mod) {
            delete this->modi;
        }

        this->current_mod = next_state;

        this->modi = mod_pack.mod;

        this->rdpapi = mod_pack.rdpapi;
        this->winapi = mod_pack.winapi;
        this->connected = mod_pack.connected;
        this->psocket_transport = mod_pack.psocket_transport;
        this->enable_osd = mod_pack.enable_osd;

        this->modi->init();
    }

    [[nodiscard]] SocketTransport* get_mod_transport() const noexcept
    {
        return this->psocket_transport;
    }

    void set_time_close(MonotonicTimePoint t)
    {
        this->end_time_session = t;
    }

private:
    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override
    {
        if (this->is_disable_by_input && keymap.last_kevent() == Keymap::KEvent::Insert) {
            this->disable_osd();
            return;
        }

        this->get_mod().rdp_input_scancode(flags, scancode, event_time, keymap);

        if (this->enable_osd) {
            Inifile const& ini = this->ini;

            if (ini.get<cfg::globals::enable_osd_display_remote_target>() && (scancode == Scancode::F12)) {
                bool const f12_released = bool(flags & KbdFlags::Release);
                if (this->target_info_is_shown && f12_released) {
                    LOG(LOG_INFO, "Hide info");
                    this->clear_osd_message();
                    this->target_info_is_shown = false;
                }
                else if (!this->target_info_is_shown && !f12_released) {
                    LOG(LOG_INFO, "Show info");
                    std::string msg;
                    msg.reserve(64);
                    if (ini.get<cfg::client::show_target_user_in_f12_message>()) {
                        str_append(msg, ini.get<cfg::globals::target_user>(), '@');
                    }
                    msg += ini.get<cfg::globals::target_device>();
                    if (this->end_time_session.time_since_epoch().count()) {
                        const auto elapsed_time
                            = this->end_time_session- this->time_base.monotonic_time;
                        // only if "reasonable" time
                        using namespace std::chrono_literals;
                        if (elapsed_time < MonotonicTimePoint::duration(60s*60*24*366)) {
                            str_append(msg,
                                "  [",
                                time_before_closing(
                                    std::chrono::duration_cast<std::chrono::seconds>(elapsed_time),
                                    Translator(language(ini))),
                                ']');
                        }
                    }
                    if (msg != this->osd_message) {
                        this->clear_osd_message();
                    }
                    if (!msg.empty()) {
                        this->osd_message = std::move(msg);
                        this->allow_disable_osd_message = false;
                        this->is_disable_by_input = false;
                        this->prepare_osd_message(gdi::OsdMsgUrgency::NORMAL);
                        this->draw_osd_message();
                    }

                    this->target_info_is_shown = true;
                }
            }
        }
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        this->get_mod().rdp_input_unicode(flag, unicode);
    }

    void rdp_input_mouse(int device_flags, int x, int y) override
    {
        if (!this->try_input_mouse(device_flags, x, y)) {
            if (this->enable_osd) {
                if (this->try_input_mouse(device_flags, x, y)) {
                    this->target_info_is_shown = false;
                    return ;
                }
            }
            this->get_mod().rdp_input_mouse(device_flags, x, y);
        }
    }

    void rdp_input_invalidate(Rect r) override
    {
        if (this->get_protected_rect().isempty() || !r.has_intersection(this->get_protected_rect())) {
            this->get_mod().rdp_input_invalidate(r);
            return;
        }

        this->get_mod().rdp_input_invalidate2(gdi::subrect4(r, this->get_protected_rect()));
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        this->get_mod().rdp_input_synchronize(locks);
    }

    void rdp_gdi_up_and_running() override
    {
        this->get_mod().rdp_gdi_up_and_running();
    }

    void rdp_gdi_down() override
    {
        this->get_mod().rdp_gdi_down();
    }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) override
    {
        this->get_mod().rdp_allow_display_updates(left, top, right, bottom);
    }

    void rdp_suppress_display_updates() override
    {
        this->get_mod().rdp_suppress_display_updates();
    }

    void refresh(Rect r) override
    {
        LOG(LOG_INFO, "ModWrapper::refresh");
        this->get_mod().refresh(r);
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags) override
    {
        this->get_mod().send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    [[nodiscard]] Rect get_protected_rect() const
    {
        return this->gfilter.get_protected_rect();
    }

    void set_protected_rect(Rect const rect)
    {
        this->gfilter.set_protected_rect(rect);
    }

    static constexpr int padw = 16;
    static constexpr int padh = 16;

    void prepare_osd_message(gdi::OsdMsgUrgency omu)
    {
        this->bogus_refresh_rect_ex
          = (this->ini.get<cfg::globals::bogus_refresh_rect>()
         && this->ini.get<cfg::globals::allow_using_multiple_monitors>()
         && (this->client_info.cs_monitor.monitorCount > 1));

        BGRColor bgr_color;
        BGRColor bgr_background_color;

        switch (omu)
        {
           case gdi::OsdMsgUrgency::NORMAL :
               bgr_color = BLACK;
               bgr_background_color = LIGHT_YELLOW;
               break;
           case gdi::OsdMsgUrgency::INFO :
               bgr_color = BLUE;
               bgr_background_color = LIGHT_YELLOW;
               this->osd_message.insert(0, "INFO: ");
               break;
           case gdi::OsdMsgUrgency::WARNING :
               bgr_color = ORANGE;
               bgr_background_color = LIGHT_YELLOW;
               this->osd_message.insert(0, "WARNING: ");
               break;
           case gdi::OsdMsgUrgency::ALERT :
               bgr_color = RED;
               bgr_background_color = LIGHT_YELLOW;
               this->osd_message.insert(0, "ALERT: ");
               break;
        }

        this->color = color_encode(bgr_color,
                                   this->client_info.screen_info.bpp);
        this->background_color = color_encode(bgr_background_color,
                                              this->client_info.screen_info.bpp);

        if (this->client_info.remote_program
            && (this->winapi == static_cast<windowing_api *>(&this->rail_client_execute)))
        {
            Rect current_work_area_rect =
                this->rail_client_execute.get_current_work_area_rect();

            this->line_metrics =
                gdi::MultiLineTextMetrics(this->glyphs,
                                          this->osd_message.c_str(),
                                          current_work_area_rect.cx - padw);

            int w = this->line_metrics.max_width() + padw * 2;
            int h = this->line_metrics.lines().size()
                * this->glyphs.max_height()
                + padh
                * 2;

            this->clip = Rect(
                current_work_area_rect.x + (
                   current_work_area_rect.cx < w ? 0 : (current_work_area_rect.cx - w) / 2),
                0,
                w,
                h);
        }
        else
        {
            this->line_metrics =
                gdi::MultiLineTextMetrics(this->glyphs,
                                          this->osd_message.c_str(),
                                          this->client_info.screen_info.width - padw);

            int w = this->line_metrics.max_width() + padw * 2;
            int h = this->line_metrics.lines().size()
                * this->glyphs.max_height()
                + padh
                * 2;

            this->clip = Rect(
                this->client_info.screen_info.width < w ?
                    0 : (this->client_info.screen_info.width - w) / 2,
                0,
                w,
                h);
        }

        this->set_protected_rect(this->clip);

        if (this->winapi) {
            this->winapi->create_auxiliary_window(this->clip);
        }
    }

    void draw_osd_message()
    {
        auto rect = this->get_protected_rect();
        this->set_protected_rect(Rect{});
        this->get_graphics().begin_update();
        this->draw_osd_message_impl(this->get_graphics(), rect);
        this->get_graphics().end_update();
        this->set_protected_rect(rect);
    }

    void draw_osd_message_impl(gdi::GraphicApi & drawable, Rect osd_rect)
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

        auto lines = this->line_metrics.lines();
        int16_t dy = padh;
        uint32_t i = 0;

        while (i < lines.size() - 1)
        {
            gdi::server_draw_text(
                drawable,
                this->glyphs,
                osd_rect.x + padw,
                dy,
                lines[i++].str,
                this->color,
                this->background_color,
                color_ctx,
                this->clip);
            dy += padh;
        }

        if (this->allow_disable_osd_message)
        {
            gdi::server_draw_text(
                drawable,
                this->glyphs,
                osd_rect.x + padw,
                dy + 4,
                lines[i].str,
                color_encode(BGRColor(BLACK),
                             this->client_info.screen_info.bpp),
                this->background_color,
                color_ctx,
                this->clip);
        }

        this->clip = Rect();
    }

    void disable_osd()
    {
        this->is_disable_by_input = false;
        auto const protected_rect = this->get_protected_rect();
        this->set_protected_rect(Rect{});

        if (this->bogus_refresh_rect_ex) {
            this->get_mod().rdp_suppress_display_updates();
            this->get_mod().rdp_allow_display_updates(0, 0,
                this->client_info.screen_info.width,
                this->client_info.screen_info.height);
        }

        if (this->winapi) {
            this->winapi->destroy_auxiliary_window();
        }

        this->get_mod().rdp_input_invalidate(protected_rect);
    }

    void clear_osd_message()
    {
        if (!this->get_protected_rect().isempty()) {
            this->disable_osd();
        }
    }

    bool try_input_mouse(int device_flags, int x, int y)
    {
        if (this->is_disable_by_input
         && this->get_protected_rect().contains_pt(x, y)
         && device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)
        ) {
            this->disable_osd();
            return true;
        }
        return false;
    }
};
