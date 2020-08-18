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
#include "acl/gd_provider.hpp"
#include "acl/mod_pack.hpp"
#include "acl/module_manager/enums.hpp"
#include "acl/session_logfile.hpp"
#include "acl/time_before_closing.hpp"
#include "configs/config.hpp"
#include "core/callback_forwarder.hpp"
#include "core/front_api.hpp"
#include "gdi/clip_from_cmd.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/graphic_api_forwarder.hpp"
#include "gdi/subrect4.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "mod/mod_api.hpp"
#include "mod/null/null.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"


class SocketTransport;
class rdp_api;

struct ModWrapper : public GdProvider
{
    struct InputFilter {
    };

    struct CallbackForwarder<ModWrapper> callback;

    struct GFilter {
        gdi::GraphicApi & sink;
        Callback & callback;
        BGRPalette const & palette;
        Rect protected_rect;

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
    } gfilter;

    struct gdi::GraphicApiForwarder<GFilter> g;

    FrontAPI & front;

    std::string module_name() const
    {
        return this->modi->module_name();
    }

public:
    bool connected = false;
    ModuleIndex current_mod = MODULE_UNKNOWN;
public:

    bool target_info_is_shown = false;
    bool enable_osd = false;

    bool is_connected() const {
        return this->connected;
    }

    rdp_api* get_rdp_api() const {
        return this->rdpapi;
    }

    gdi::GraphicApi & get_graphics() override
    {
        return this->g;
    }

    bool is_ready_to_draw() override { return true; }

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

    void display_osd_message(const std::string& message) override
    {
        if (message != this->get_message()) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            this->osd_message = std::move(message);
            this->is_disable_by_input = true;
            str_append(this->osd_message, "  ", TR(trkeys::disable_osd, language(this->ini)));
            this->prepare_osd_message();
            this->draw_osd_message();
        }
    }

    [[nodiscard]] Rect get_protected_rect() const
    { return this->gfilter.protected_rect; }

    void set_protected_rect(Rect const rect)
    { this->gfilter.protected_rect = rect; }

private:
    ClientInfo const & client_info;
    ClientExecute & rail_client_execute;
    public:
    rdp_api*       rdpapi = nullptr;
    RailModuleHostMod* rail_module_host_mod_ptr = nullptr;
    windowing_api* &winapi;
    private:
    Inifile & ini;
    Sesman & sesman;

    std::string osd_message;
    Rect clip;
    RDPColor color;
    RDPColor background_color;
    bool is_disable_by_input = false;
    bool bogus_refresh_rect_ex;
    const Font & glyphs;
    Keymap2 & keymap;

public:
    explicit ModWrapper(FrontAPI & front, BGRPalette const & palette, gdi::GraphicApi& graphics, Keymap2 & keymap, ClientInfo const & client_info, const Font & glyphs, ClientExecute & rail_client_execute, windowing_api* & winapi, Inifile & ini, Sesman & sesman)
    : callback(*this)
    , gfilter(graphics, callback, palette, Rect{})
    , g(gfilter)
    , front(front)
    , client_info(client_info)
    , rail_client_execute(rail_client_execute)
    , winapi(winapi)
    , ini(ini)
    , sesman(sesman)
    , bogus_refresh_rect_ex(false)
    , glyphs(glyphs)
    , keymap(keymap)
    {}

    ~ModWrapper(){
        this->remove_mod();
    }

    Callback & get_callback() noexcept
    {
        return this->callback;
    }

    [[nodiscard]] bool is_input_owner() const { return this->is_disable_by_input; }

    [[nodiscard]] std::string const& get_message() const {
        return this->osd_message;
    }

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

    static constexpr int padw = 16;
    static constexpr int padh = 16;

    void draw_osd_message()
    {
        auto rect = this->get_protected_rect();
        this->set_protected_rect(Rect{});
        this->get_graphics().begin_update();
        this->draw_osd_message_impl(this->get_graphics(), rect);
        this->get_graphics().end_update();
        this->set_protected_rect(rect);
    }


private:
    void draw_osd_message_impl(gdi::GraphicApi & drawable, Rect osd_rect)
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
            osd_rect.x + padw, padh,
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
    void acl_update()
    {
        this->get_mod()->acl_update();
    }

    mod_api* get_mod()
    {
        return this->modi;
    }

    [[nodiscard]] mod_api const* get_mod() const
    {
        return this->modi;
    }

    BackEvent_t get_mod_signal()
    {
        return this->get_mod()->get_mod_signal();
    }

    void remove_mod()
    {
        if (this->modi != &this->no_mod){
            this->clear_osd_message();
            delete this->modi;
            this->modi = &this->no_mod;
            this->rdpapi = nullptr;
            this->winapi = nullptr;
            this->rail_module_host_mod_ptr = nullptr;
        }
    }

    bool is_up_and_running() const {
        return (this->modi != &this->no_mod) && this->get_mod()->is_up_and_running();
    }

    void set_mod_transport(SocketTransport * psocket_transport)
    {
        this->psocket_transport = psocket_transport;
    }

    void set_mod(ModuleIndex next_state, ModPack mod_pack)
    {
        // The end of session is done when existing RDP or VNC connected module
        // The open counterpart is done before opening socket
        if (next_state != this->current_mod){
            if ((this->current_mod == MODULE_RDP)
            ||  (this->current_mod == MODULE_VNC)){
                sesman.set_disconnect_target();
            }
        }

//        LOG(LOG_INFO, "=================== Setting new mod %s (was %s)  psocket_transport = %p",
//            get_module_name(next_state),
//            get_module_name(this->current_mod),
//            mod_pack.psocket_transport);

        while (this->keymap.nb_char_available()) {
            this->keymap.get_char();
        }
        while (this->keymap.nb_kevent_available()) {
            this->keymap.get_kevent();
        }

        this->current_mod = next_state;
        this->clear_osd_message();

        this->remove_mod();
        this->modi = mod_pack.mod.get();

        this->rail_module_host_mod_ptr = mod_pack.rail_module_host_ptr;
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

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
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

        this->get_mod()->rdp_input_scancode(param1, param2, param3, param4, keymap);

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
                                "  [", time_before_closing(elapsed_time, Translator(ini)), ']');
                        }
                    }
                    if (msg != this->get_message()) {
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

    void rdp_input_unicode(uint16_t unicode, uint16_t flag)
    {
        this->get_mod()->rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (!this->try_input_mouse(device_flags, x, y, keymap)) {
            if (this->enable_osd) {
                if (this->try_input_mouse(device_flags, x, y, keymap)) {
                    this->target_info_is_shown = false;
                    return ;
                }
            }
            this->get_mod()->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_invalidate(Rect r)
    {
        if (this->get_protected_rect().isempty() || !r.has_intersection(this->get_protected_rect())) {
            this->get_mod()->rdp_input_invalidate(r);
            return;
        }
        auto rects = gdi::subrect4(r, this->get_protected_rect());
        this->get_mod()->rdp_input_invalidate2({std::begin(rects), std::end(rects)});
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    { this->get_mod()->rdp_input_synchronize(time, device_flags, param1, param2); }

    void rdp_gdi_up_and_running()
    { this->get_mod()->rdp_gdi_up_and_running(); }

    void rdp_gdi_down()
    { this->get_mod()->rdp_gdi_down(); }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
    {
        this->get_mod()->rdp_allow_display_updates(left, top, right, bottom);
    }

    void rdp_suppress_display_updates()
    { this->get_mod()->rdp_suppress_display_updates(); }

    void refresh(Rect r)
    {
        LOG(LOG_INFO, "ModWrapper::refresh");
        this->get_mod()->refresh(r);
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags)
    {
        this->get_mod()->send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    void send_auth_channel_data(const char * data)
    {
        this->get_mod()->send_auth_channel_data(data);
    }

    void send_checkout_channel_data(const char * data)
    {
        this->get_mod()->send_checkout_channel_data(data);
    }

    void create_shadow_session(const char * userdata, const char * type)
    {
        this->get_mod()->create_shadow_session(userdata, type);
    }

    void disconnect()
    {
        if (this->modi != &this->no_mod) {
            try {
                this->get_mod()->disconnect();
            }
            catch (Error const& e) {
                LOG(LOG_ERR, "disconnect raised exception %d", static_cast<int>(e.id));
            }
        }
    }

    [[nodiscard]] Dimension get_dim() const
    {
        return this->get_mod()->get_dim();
    }

};

