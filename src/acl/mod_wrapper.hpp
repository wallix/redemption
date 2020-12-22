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
#include "keyboard/keymap2.hpp"
#include "configs/config.hpp"
#include "gdi/clip_from_cmd.hpp"
#include "gdi/graphic_api_forwarder.hpp"
#include "gdi/subrect4.hpp"
#include "gdi/osd_api.hpp"
#include "gdi/text_metrics.hpp"
#include "mod/null/null.hpp"
#include "utils/translation.hpp"
#include "core/callback.hpp"
#include "core/client_info.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/slowpath.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"


class SocketTransport;
class rdp_api;

class ModWrapper final : public gdi::OsdApi, private Callback
{
    class GFilter
    {
        gdi::GraphicApi & sink;
        Callback & callback;

    public:
        BGRPalette const & palette;
        Rect protected_rect;

    public:
        GFilter(gdi::GraphicApi & sink, Callback & callback, const BGRPalette & palette, Rect rect)
            : sink(sink), callback(callback), palette(palette), protected_rect(rect) {}

        template<class Command, class... Args>
        void draw(Command const & cmd)
            { this->sink.draw(cmd); }
        void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content)
            { this->sink.draw(cmd, content); }

        template<class Command, class... Args>
        void draw(Command const & cmd, Rect clip, Args const &... args)
        {
            auto const & clip_rect = clip_from_cmd(cmd).intersect(clip);
            if (this->protected_rect.contains(clip_rect) || clip_rect.isempty()) {
                // nada: leave the OSD message rect untouched
            }
            else if (clip_rect.has_intersection(this->protected_rect)) {
                // draw the parts of the screen outside OSD message rect
                for (const Rect & subrect : gdi::subrect4(clip_rect, this->protected_rect)) {
                    if (!subrect.isempty()) {
                        this->sink.draw(cmd, subrect, args...);
                    }
                }
            }
            else {
                // The drawing order is fully ouside OSD message rect
                this->sink.draw(cmd, clip, args...);
            }
        }

        void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp)
        {
            Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                        , bitmap_data.dest_right - bitmap_data.dest_left + 1
                        , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

            if (this->protected_rect.contains(rectBmp) || rectBmp.isempty()) {
                // nada: leave the OSD message rect untouched
            }
            if (rectBmp.has_intersection(this->protected_rect)) {
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
            }
            else {
                // The drawing order is fully ouside OSD message rect
                this->sink.draw(bitmap_data, bmp);
            }
        }

        void draw(const RDPScrBlt & cmd, const Rect clip)
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
                if (has_src_intersec_fg){
                    // We don't have src data, ask it to server, no choice
                    gdi::subrect4_t rects = gdi::subrect4(drect, this->protected_rect);
                    auto e = std::remove_if(rects.begin(), rects.end(), [](const Rect & rect) { return rect.isempty(); });
                    auto av = make_array_view(rects.begin(), e);
                    this->callback.rdp_input_invalidate2(av);
                }
                else {
                    // only drect has intersection, src rect is available
                    for (const Rect & subrect : gdi::subrect4(drect, this->protected_rect)) {
                        if (!subrect.isempty()) {
                            this->sink.draw(cmd, subrect);
                        }
                    }
                }
            }
        }

        void set_pointer(
            uint16_t cache_idx, Pointer const& cursor,
            gdi::GraphicApi::SetPointerMode mode)
        {
            this->sink.set_pointer(cache_idx, cursor, mode);
        }

        void set_palette(BGRPalette const & palette) { this->sink.set_palette(palette); }
        void sync() { this->sink.sync(); }
        void set_row(std::size_t rownum, bytes_view data) { this->sink.set_row(rownum, data); }
        void begin_update() { this->sink.begin_update(); }
        void end_update() { this->sink.end_update(); }
    };

    struct Graphics final : gdi::GraphicApiForwarder<GFilter>
    {
        using gdi::GraphicApiForwarder<GFilter>::GraphicApiForwarder;

        using gdi::GraphicApiForwarder<GFilter>::sink;
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

    rdp_api * rdpapi = nullptr;
    windowing_api * winapi = nullptr;

    Inifile & ini;

    std::string osd_message;
    Rect clip;
    RDPColor color;
    RDPColor background_color;
    bool is_disable_by_input = false;
    bool bogus_refresh_rect_ex;
    const Font & glyphs;
    Keymap2 & keymap;

    SocketTransport * psocket_transport = nullptr;
    null_mod no_mod;
    not_null_ptr<mod_api> modi = &no_mod;


public:
    explicit ModWrapper(BGRPalette const & palette, gdi::GraphicApi& graphics, Keymap2 & keymap, ClientInfo const & client_info, const Font & glyphs, ClientExecute & rail_client_execute, Inifile & ini)
    : gfilter(graphics, static_cast<Callback&>(*this), palette, Rect{})
    , client_info(client_info)
    , rail_client_execute(rail_client_execute)
    , ini(ini)
    , bogus_refresh_rect_ex(false)
    , glyphs(glyphs)
    , keymap(keymap)
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

    void display_osd_message(std::string_view message) override
    {
        if (message != this->osd_message) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            str_assign(this->osd_message,
                       message,
                       " ",
                       TR(trkeys::disable_osd, language(this->ini)));
            this->is_disable_by_input = true;
            this->prepare_osd_message();
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
       //     get_module_name(next_state),
       //     get_module_name(this->current_mod),
       //     mod_pack.psocket_transport);

        while (this->keymap.nb_char_available()) {
            this->keymap.get_char();
        }
        while (this->keymap.nb_kevent_available()) {
            this->keymap.get_kevent();
        }

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

private:
    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        if (this->is_disable_by_input && keymap->nb_kevent_available() > 0
            && keymap->top_kevent() == Keymap2::KEVENT_INSERT
        ) {
            keymap->get_kevent();
            this->disable_osd();
            return;
        }
        if (this->enable_osd) {

            if (this->is_disable_by_input && keymap->nb_kevent_available() > 0
                && keymap->top_kevent() == Keymap2::KEVENT_INSERT
            ) {
                keymap->get_kevent();
                this->disable_osd();
                this->target_info_is_shown = false;
            }
        }

        this->get_mod().rdp_input_scancode(param1, param2, param3, param4, keymap);

        if (this->enable_osd) {
            Inifile const& ini = this->ini;

            if (ini.get<cfg::globals::enable_osd_display_remote_target>() && (param1 == Keymap2::F12)) {
                bool const f12_released = (param3 & SlowPath::KBDFLAGS_RELEASE);
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
                    const uint32_t enddate = ini.get<cfg::context::end_date_cnx>();
                    if (enddate) {
                        const auto now = time(nullptr);
                        const auto elapsed_time = enddate - now;
                        // only if "reasonable" time
                        if (elapsed_time < 60*60*24*366L) {
                            str_append(msg,
                                "  [",
                                time_before_closing(elapsed_time, Translator(language(ini))),
                                ']');
                        }
                    }
                    if (msg != this->osd_message) {
                        this->clear_osd_message();
                    }
                    if (!msg.empty()) {
                        this->osd_message = std::move(msg);
                        this->is_disable_by_input = false;
                        this->prepare_osd_message();
                        this->draw_osd_message();
                    }

                    this->target_info_is_shown = true;
                }
            }
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override
    {
        this->get_mod().rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        if (!this->try_input_mouse(device_flags, x, y, keymap)) {
            if (this->enable_osd) {
                if (this->try_input_mouse(device_flags, x, y, keymap)) {
                    this->target_info_is_shown = false;
                    return ;
                }
            }
            this->get_mod().rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_invalidate(Rect r) override
    {
        if (this->get_protected_rect().isempty() || !r.has_intersection(this->get_protected_rect())) {
            this->get_mod().rdp_input_invalidate(r);
            return;
        }
        auto rects = gdi::subrect4(r, this->get_protected_rect());
        this->get_mod().rdp_input_invalidate2({std::begin(rects), std::end(rects)});
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        this->get_mod().rdp_input_synchronize(time, device_flags, param1, param2);
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
        return this->gfilter.sink.protected_rect;
    }

    void set_protected_rect(Rect const rect)
    {
        this->gfilter.sink.protected_rect = rect;
    }

    static constexpr int padw = 16;
    static constexpr int padh = 16;

    void prepare_osd_message()
    {
        this->bogus_refresh_rect_ex = (this->ini.get<cfg::globals::bogus_refresh_rect>()
         && this->ini.get<cfg::globals::allow_using_multiple_monitors>()
         && (this->client_info.cs_monitor.monitorCount > 1));


        gdi::TextMetrics tm(this->glyphs, this->osd_message.c_str());
        int w = tm.width + padw * 2;
        int h = tm.height + padh * 2;
        this->color = color_encode(BGRColor(BLACK), this->client_info.screen_info.bpp);
        this->background_color = color_encode(BGRColor(LIGHT_YELLOW), this->client_info.screen_info.bpp);

        if (this->client_info.remote_program
        && (this->winapi == static_cast<windowing_api*>(&this->rail_client_execute))) {

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

        auto const color_ctx = gdi::ColorCtx::from_bpp(this->client_info.screen_info.bpp, this->gfilter.sink.palette);

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
            osd_rect.x + padw, padh,
            this->osd_message.c_str(),
            this->color, this->background_color, color_ctx, this->clip
        );

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
};

