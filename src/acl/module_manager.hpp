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
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#pragma once

#include "acl/auth_api.hpp"
#include "acl/mm_api.hpp"
#include "acl/module_manager/mm_ini.hpp"
#include "acl/module_manager/enums.hpp"
#include "configs/config.hpp"
#include "core/session_reactor.hpp"
#include "front/front.hpp"
#include "gdi/protected_graphics.hpp"

#include "mod/internal/bouncer2_mod.hpp"
#include "RAIL/client_execute.hpp"
#include "mod/internal/flat_dialog_mod.hpp"
#include "mod/internal/flat_login_mod.hpp"
#include "mod/internal/flat_wab_close_mod.hpp"
#include "mod/internal/flat_wait_mod.hpp"
#include "mod/internal/interactive_target_mod.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "mod/internal/replay_mod.hpp"
#include "mod/internal/selector_mod.hpp"
#include "mod/internal/test_card_mod.hpp"
#include "mod/internal/widget_test_mod.hpp"

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
#include "utils/translation.hpp"


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


inline void add_time_before_closing(std::string & msg, uint32_t elapsed_time, Translator tr)
{
    const auto hours = elapsed_time / (60*60);
    const auto minutes = elapsed_time / 60 - hours * 60;
    const auto seconds = elapsed_time - hours * (60*60) - minutes * 60;

    if (hours) {
        str_append(
            msg,
            std::to_string(hours),
            ' ',
            tr(trkeys::hour),
            (hours > 1) ? "s, " : ", "
        );
    }

    if (minutes || hours) {
        str_append(
            msg,
            std::to_string(minutes),
            ' ',
            tr(trkeys::minute),
            (minutes > 1) ? "s, " : ", "
        );
    }

    str_append(
        msg,
        std::to_string(seconds),
        ' ',
        tr(trkeys::second),
        (seconds > 1) ? "s " : " ",
        tr(trkeys::before_closing)
    );
}

class ModuleManager : public MMIni
{
    class ModOSD : public gdi::ProtectedGraphics, public mod_api
    {
        ModuleManager & mm;

        std::string osd_message;
        Rect clip;
        RDPColor color;
        RDPColor background_color;
        bool is_disable_by_input = false;
        bool bogus_refresh_rect_ex;

    public:
        explicit ModOSD(ModuleManager & mm)
        : gdi::ProtectedGraphics(mm.front, Rect{})
        , mm(mm)
        {}

        bool is_input_owner() const { return this->is_disable_by_input; }

        void disable_osd()
        {
            this->is_disable_by_input = false;
            auto const protected_rect = this->get_protected_rect();
            this->set_protected_rect(Rect{});

            if (this->bogus_refresh_rect_ex) {
                this->mm.mod->rdp_suppress_display_updates();
                this->mm.mod->rdp_allow_display_updates(0, 0,
                    this->mm.front.client_info.screen_info.width,
                    this->mm.front.client_info.screen_info.height);
            }

            if (this->mm.winapi) {
                this->mm.winapi->destroy_auxiliary_window();
            }

            this->mm.mod->rdp_input_invalidate(protected_rect);
        }

        const char* get_message() const {
            return this->osd_message.c_str();
        }

        void set_message(std::string message, bool is_disable_by_input)
        {
            this->osd_message = std::move(message);
            this->is_disable_by_input = is_disable_by_input;
            this->bogus_refresh_rect_ex = (this->mm.ini.get<cfg::globals::bogus_refresh_rect>()
             && this->mm.ini.get<cfg::globals::allow_using_multiple_monitors>()
             && (this->mm.front.client_info.cs_monitor.monitorCount > 1));

            if (is_disable_by_input) {
                str_append(this->osd_message, "  ", TR(trkeys::disable_osd, language(this->mm.ini)));
            }

            gdi::TextMetrics tm(this->mm.load_font(), this->osd_message.c_str());
            int w = tm.width + padw * 2;
            int h = tm.height + padh * 2;
            this->color = color_encode(BGRColor(BLACK), this->mm.front.client_info.screen_info.bpp);
            this->background_color = color_encode(BGRColor(LIGHT_YELLOW), this->mm.front.client_info.screen_info.bpp);

            if (this->mm.front.client_info.remote_program &&
                (this->mm.winapi == static_cast<windowing_api*>(&this->mm.client_execute))) {

                Rect current_work_area_rect = this->mm.client_execute.get_current_work_area_rect();

                this->clip = Rect(
                    current_work_area_rect.x +
                        (current_work_area_rect.cx < w ? 0 : (current_work_area_rect.cx - w) / 2),
                    0, w, h);
            }
            else {
                this->clip = Rect(this->mm.front.client_info.screen_info.width < w ? 0 : (this->mm.front.client_info.screen_info.width - w) / 2, 0, w, h);
            }

            this->set_protected_rect(this->clip);

            if (this->mm.winapi) {
                this->mm.winapi->create_auxiliary_window(this->clip);
            }
        }

        static constexpr int padw = 16;
        static constexpr int padh = 16;

        void draw_osd_message()
        {
            this->mm.front.begin_update();
            this->draw_osd_message_impl(this->mm.front);
            this->mm.front.end_update();
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
                    this->mm.mod->rdp_input_invalidate2({p, e});
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
                    this->mm.mod->rdp_input_invalidate(r);
                }
                else {
                    ret = true;
                }
            }
            return ret;
        }

    private:
        void draw_event(time_t now, gdi::GraphicApi & gd) override
        {
            update_lock lock{gd};
            this->draw_osd_message_impl(gd);
            this->mm.mod->draw_event(now, gd);
        }

        void draw_osd_message_impl(gdi::GraphicApi & drawable)
        {
            if (this->clip.isempty()) {
                return ;
            }

            auto const color_ctx = gdi::ColorCtx::from_bpp(this->mm.front.client_info.screen_info.bpp, this->mm.front.get_palette());

            drawable.draw(RDPOpaqueRect(this->clip, this->background_color), this->clip, color_ctx);

            RDPLineTo line_left(1, this->clip.x, this->clip.y, this->clip.x, this->clip.y + this->clip.cy - 1,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_left, this->clip, color_ctx);
            RDPLineTo line_bottom(1, this->clip.x, this->clip.y + this->clip.cy - 1, this->clip.x + this->clip.cx - 1, this->clip.y + this->clip.cy - 1,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_bottom, this->clip, color_ctx);

            RDPLineTo line_right(1, this->clip.x + this->clip.cx - 1, this->clip.y + this->clip.cy - 1, this->clip.x + this->clip.cx - 1, this->clip.y,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_right, this->clip, color_ctx);
            RDPLineTo line_top(1, this->clip.x + this->clip.cx - 1, this->clip.y, this->clip.x, this->clip.y,
                encode_color24()(BLACK), 0x0D, RDPPen(0, 0, encode_color24()(BLACK)));
            drawable.draw(line_top, this->clip, color_ctx);

            gdi::server_draw_text(
                drawable, this->mm.load_font(),
                this->get_protected_rect().x + padw, padh,
                this->osd_message.c_str(),
                this->color, this->background_color, color_ctx, this->clip
            );

            this->clip = Rect();
        }

        void refresh_rects(array_view<Rect const> av) override
        {
            this->mm.mod->rdp_input_invalidate2(av);
        }

        void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
        {
            if (!this->try_input_scancode(param1, param2, param3, param4, keymap)) {
                this->mm.mod->rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
        }

        void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
            this->mm.mod->rdp_input_unicode(unicode, flag);
        }

        void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
        {
            if (!this->try_input_mouse(device_flags, x, y, keymap)) {
                this->mm.mod->rdp_input_mouse(device_flags, x, y, keymap);
            }
        }

        void rdp_input_invalidate(Rect r) override
        {
            if (!this->try_input_invalidate(r)) {
                this->mm.mod->rdp_input_invalidate(r);
            }
        }

        void rdp_input_invalidate2(array_view<Rect const> vr) override
        {
            if (!this->try_input_invalidate2(vr)) {
                this->mm.mod->rdp_input_invalidate2(vr);
            }
        }

        void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
        { this->mm.mod->rdp_input_synchronize(time, device_flags, param1, param2); }

        void rdp_input_up_and_running() override
        { this->mm.mod->rdp_input_up_and_running(); }

        void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) override
        { this->mm.mod->rdp_allow_display_updates(left, top, right, bottom); }

        void rdp_suppress_display_updates() override
        { this->mm.mod->rdp_suppress_display_updates(); }

        void refresh(Rect r) override
        {
            this->mm.mod->refresh(r);
        }

        void send_to_mod_channel(
            CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
            std::size_t length, uint32_t flags
        ) override
        { this->mm.mod->send_to_mod_channel(front_channel_name, chunk, length, flags); }

        void send_auth_channel_data(const char * data) override
        { this->mm.mod->send_auth_channel_data(data); }

        void send_checkout_channel_data(const char * data) override
        { this->mm.mod->send_checkout_channel_data(data); }

        void refresh_context() override
        { this->mm.mod->refresh_context(); }

        bool is_up_and_running() const override
        { return this->mm.mod->is_up_and_running(); }

        void disconnect(time_t now) override
        { this->mm.mod->disconnect(now); }

        void display_osd_message(std::string const & message) override
        { this->mm.mod->display_osd_message(message); }

        Dimension get_dim() const override
        {
            return this->mm.mod->get_dim();
        }
    };

public:
    gdi::GraphicApi & get_graphic_wrapper()
    {
        gdi::GraphicApi& gd = this->mod_osd.get_protected_rect().isempty()
          ? static_cast<gdi::GraphicApi&>(this->front) : this->mod_osd;
        if (this->rail_module_host_mod_ptr) {
            return this->rail_module_host_mod_ptr->proxy_gd(gd);
        }
        return gd;
    }

    Callback & get_callback() noexcept
    {
        return *this->mod;
    }

private:
    struct sock_mod_barrier {};
    #include "mod/mod_with_socket.hpp"

public:
    void clear_osd_message()
    {
        if (!this->mod_osd.get_protected_rect().isempty()) {
            this->mod_osd.disable_osd();
        }
    }

    void osd_message(std::string message, bool is_disable_by_input)
    {
        if (message != this->mod_osd.get_message()) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            this->mod_osd.set_message(std::move(message), is_disable_by_input);
            this->mod_osd.draw_osd_message();
        }
    }

private:
    RailModuleHostMod* rail_module_host_mod_ptr = nullptr;
    Front & front;
    null_mod no_mod;
    ModOSD mod_osd;
    Random & gen;
    TimeObj & timeobj;

    ClientExecute client_execute;

    std::array<uint8_t, 28> server_auto_reconnect_packet {};

    int old_target_module = MODULE_UNKNOWN;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        new_mod = 0x1,
    };

private:
    rdp_api*       rdpapi = nullptr;
    windowing_api* winapi = nullptr;

    SocketTransport * socket_transport = nullptr;

public:
    ModuleManager(SessionReactor& session_reactor, Front & front, Inifile & ini, Random & gen, TimeObj & timeobj)
        : MMIni(session_reactor, ini)
        , front(front)
        , mod_osd(*this)
        , gen(gen)
        , timeobj(timeobj)
        , client_execute(session_reactor, front, this->front.client_info.window_list_caps,
                         ini.get<cfg::debug::mod_internal>() & 1)
        , verbose(static_cast<Verbose>(ini.get<cfg::debug::auth>()))
    {
        this->mod = &this->no_mod;
    }

    bool has_pending_data() const
    {
        return this->socket_transport && this->socket_transport->has_pending_data();
    }

    SocketTransport* get_socket() const noexcept
    {
        return this->socket_transport;
    }

    void remove_mod() override
    {
        if (this->mod != &this->no_mod) {
            this->clear_osd_message();

            delete this->mod;
            this->mod = &this->no_mod;
            this->rdpapi = nullptr;
            this->winapi = nullptr;
            this->rail_module_host_mod_ptr = nullptr;
        }
    }

    ~ModuleManager() override
    {
        this->remove_mod();
    }

private:
    void set_mod(not_null_ptr<mod_api> mod, rdp_api* rdpapi = nullptr, windowing_api* winapi = nullptr)
    {
        while (this->front.keymap.nb_char_available()) {
            this->front.keymap.get_char();
        }
        while (this->front.keymap.nb_kevent_available()) {
            this->front.keymap.get_kevent();
        }

        this->clear_osd_message();

        this->mod = mod.get();
        this->rail_module_host_mod_ptr = nullptr;

        this->rdpapi = rdpapi;
        this->winapi = winapi;
    }

public:
    void new_mod(int target_module, time_t now, AuthApi & authentifier, ReportMessageApi & report_message) override
    {
        LOG(LOG_INFO, "----------> ACL new_mod <--------");
        LOG(LOG_INFO, "target_module=%s(%d)", get_module_name(target_module), target_module);

        this->client_execute.enable_remote_program(this->front.client_info.remote_program);

        switch (target_module) {
        case MODULE_INTERNAL_CLOSE:
            detail::log_proxy_logout(this->ini.get<cfg::context::auth_error_message>().c_str());
            break;
        case MODULE_INTERNAL_WIDGET_LOGIN:
            detail::log_proxy_logout();
            break;
        default:
            detail::log_proxy_set_user(this->ini.get<cfg::globals::auth_user>().c_str());
            break;
        }

        this->connected = false;

        if (this->old_target_module != target_module) {
            this->front.must_be_stop_capture();

            auto is_remote_mod = [](int mod_type){
                return
                    (mod_type == MODULE_XUP)
                 || (mod_type == MODULE_RDP)
                 || (mod_type == MODULE_VNC);
            };

            if (is_remote_mod(this->old_target_module)) {
                authentifier.delete_remote_mod();
            }

            if (is_remote_mod(target_module)) {
                authentifier.new_remote_mod();
            }
        }
        this->old_target_module = target_module;

        switch (target_module)
        {
        case MODULE_INTERNAL_BOUNCER2:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'bouncer2_mod'");
            this->set_mod(new Bouncer2Mod(
                this->session_reactor,
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->load_font()
            ));
            if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::internal module 'bouncer2_mod' ready");
            }
            break;
        case MODULE_INTERNAL_TEST:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test'");
            this->set_mod(new ReplayMod(
                this->session_reactor,
                this->front,
                [this]{
                    auto movie_path = this->ini.get<cfg::video::replay_path>().to_string()
                                    + this->ini.get<cfg::globals::target_user>();
                    if (movie_path.size() < 5u || !std::equal(movie_path.end() - 5u, movie_path.end(), ".mwrm")) {
                        movie_path += ".mwrm";
                    }
                    return movie_path;
                }().c_str(),
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->ini.get_ref<cfg::context::auth_error_message>(),
                !this->ini.get<cfg::mod_replay::on_end_of_data>(),
                this->ini.get<cfg::mod_replay::replay_on_loop>(),
                this->ini.get<cfg::video::play_video_with_corrupted_bitmap>(),
                to_verbose_flags(this->ini.get<cfg::debug::capture>())
            ));
            if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::internal module 'test' ready");
            }
            break;
        case MODULE_INTERNAL_WIDGETTEST:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'widgettest'");
            this->set_mod(new WidgetTestMod(
                this->session_reactor,
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->load_font()
            ));
            LOG(LOG_INFO, "ModuleManager::internal module 'widgettest' ready");
            break;
        case MODULE_INTERNAL_CARD:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'test_card'");
            this->set_mod(new TestCardMod(
                this->session_reactor,
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->load_font(),
                false
            ));
            LOG(LOG_INFO, "ModuleManager::internal module 'test_card' ready");
            break;
        case MODULE_INTERNAL_WIDGET_SELECTOR:
        case MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY:
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'selector'");
            if (report_message.get_inactivity_timeout() != this->ini.get<cfg::globals::session_timeout>().count()) {
                report_message.update_inactivity_timeout();
            }

            this->set_mod(new SelectorMod(
                this->ini,
                this->session_reactor,
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->client_execute.adjust_rect(get_widget_rect(
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->front.client_info.cs_monitor
                )),
                this->client_execute,
                this->load_font(),
                this->load_theme()
            ));
            //if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::internal module 'selector' ready");
            //}
            break;
        case MODULE_INTERNAL_CLOSE:
        case MODULE_INTERNAL_CLOSE_BACK: {
            bool const back_to_selector = (target_module == MODULE_INTERNAL_CLOSE_BACK);
            LOG(LOG_INFO, "ModuleManager::Creation of new mod 'INTERNAL::Close%s'",
                back_to_selector ? "Back" : "");

            if (this->ini.get<cfg::context::auth_error_message>().empty()) {
                this->ini.set<cfg::context::auth_error_message>(TR(trkeys::connection_ended, language(this->ini)));
            }

            this->set_mod(new FlatWabCloseMod(
                this->ini,
                this->session_reactor,
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->client_execute.adjust_rect(get_widget_rect(
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->front.client_info.cs_monitor
                )),
                now,
                this->client_execute,
                this->load_font(),
                this->load_theme(),
                true,
                back_to_selector
            ));
            LOG(LOG_INFO, "ModuleManager::internal module Close%s ready",
                back_to_selector ? " Back" : "");
            break;
        }
        case MODULE_INTERNAL_TARGET:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Interactive Target'");
                this->set_mod(new InteractiveTargetMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.screen_info.width,
                        this->front.client_info.screen_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    this->client_execute,
                    this->load_font(),
                    this->load_theme()
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Interactive Target' ready");
            }
            break;
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
        case MODULE_INTERNAL_WIDGET_DIALOG:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Accept Message'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * button = TR(trkeys::refused, language(this->ini));
                const char * caption = "Information";
                this->set_mod(new FlatDialogMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.screen_info.width,
                        this->front.client_info.screen_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    button,
                    now,
                    this->client_execute,
                    this->load_font(),
                    this->load_theme()
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Accept Message' ready");
            }
            break;
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
        case MODULE_INTERNAL_WIDGET_MESSAGE:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Display Message'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * button = nullptr;
                const char * caption = "Information";
                this->set_mod(new FlatDialogMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.screen_info.width,
                        this->front.client_info.screen_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    button,
                    now,
                    this->client_execute,
                    this->load_font(),
                    this->load_theme()
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Display Message' ready");
            }
            break;
        case MODULE_INTERNAL_DIALOG_CHALLENGE:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Dialog Challenge'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * button = nullptr;
                const char * caption = "Challenge";
                ChallengeOpt challenge = CHALLENGE_HIDE;
                if (this->ini.get<cfg::context::authentication_challenge>()) {
                    challenge = CHALLENGE_ECHO;
                }
                this->ini.ask<cfg::context::authentication_challenge>();
                this->ini.ask<cfg::context::password>();
                this->set_mod(new FlatDialogMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.screen_info.width,
                        this->front.client_info.screen_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    button,
                    now,
                    this->client_execute,
                    this->load_font(),
                    this->load_theme(),
                    challenge
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Dialog Challenge' ready");
            }

            break;
        case MODULE_INTERNAL_WAIT_INFO:
            {
                LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Wait Info Message'");
                const char * message = this->ini.get<cfg::context::message>().c_str();
                const char * caption = TR(trkeys::information, language(this->ini));
                bool showform = this->ini.get<cfg::context::showform>();
                uint flag = this->ini.get<cfg::context::formflag>();
                this->set_mod(new FlatWaitMod(
                    this->ini,
                    this->session_reactor,
                    this->front,
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->client_execute.adjust_rect(get_widget_rect(
                        this->front.client_info.screen_info.width,
                        this->front.client_info.screen_info.height,
                        this->front.client_info.cs_monitor
                    )),
                    caption,
                    message,
                    now,
                    this->client_execute,
                    this->load_font(),
                    this->load_theme(),
                    showform,
                    flag
                ));
                LOG(LOG_INFO, "ModuleManager::internal module 'Wait Info Message' ready");
            }
            break;
        case MODULE_INTERNAL_WIDGET_LOGIN: {
            char username[255]; // should use string
            username[0] = 0;
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
            if (!this->ini.is_asked<cfg::globals::auth_user>()){
                if (this->ini.is_asked<cfg::globals::target_user>()
                 || this->ini.is_asked<cfg::globals::target_device>()){
                    utils::strlcpy(
                        username,
                        this->ini.get<cfg::globals::auth_user>().c_str(),
                        sizeof(username));
                }
                else {
                    // TODO check this! Assembling parts to get user login with target is not obvious method used below il likely to show @: if target fields are empty
                    snprintf( username, sizeof(username), "%s@%s:%s%s%s"
                            , this->ini.get<cfg::globals::target_user>().c_str()
                            , this->ini.get<cfg::globals::target_device>().c_str()
                            , this->ini.get<cfg::context::target_protocol>().c_str()
                            , (!this->ini.get<cfg::context::target_protocol>().empty() ? ":" : "")
                            , this->ini.get<cfg::globals::auth_user>().c_str()
                            );
                }

                username[sizeof(username) - 1] = 0;
            }

            this->set_mod(new FlatLoginMod(
                this->ini,
                this->session_reactor,
                username,
                "", // password
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->client_execute.adjust_rect(get_widget_rect(
                    this->front.client_info.screen_info.width,
                    this->front.client_info.screen_info.height,
                    this->front.client_info.cs_monitor
                )),
                now,
                this->client_execute,
                this->load_font(),
                this->load_theme()
            ));
            LOG(LOG_INFO, "ModuleManager::internal module Login ready");
            break;
        }

        case MODULE_XUP: {
            const char * name = "XUP Target";
            if (bool(this->verbose & Verbose::new_mod)) {
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP'\n");
            }

            unique_fd client_sck = this->connect_to_target_host(
                report_message, trkeys::authentification_x_fail, "XUP");

            this->set_mod(new ModWithSocket<xup_mod>(
                *this,
                authentifier,
                name,
                std::move(client_sck),
                this->ini.get<cfg::debug::mod_xup>(),
                nullptr,
                sock_mod_barrier(),
                this->session_reactor,
                this->front,
                this->front.client_info.screen_info.width,
                this->front.client_info.screen_info.height,
                this->ini.get<cfg::context::opt_width>(),
                this->ini.get<cfg::context::opt_height>(),
                // TODO use safe_int
                checked_int(this->ini.get<cfg::context::opt_bpp>())
                // TODO: shouldn't alls mods have access to sesman authentifier ?
            ));

            this->ini.get_ref<cfg::context::auth_error_message>().clear();
            LOG(LOG_INFO, "ModuleManager::Creation of new mod 'XUP' suceeded\n");
            this->connected = true;
            break;
        }

        case MODULE_RDP:
            this->create_mod_rdp(
                authentifier, report_message, this->ini,
                this->front, this->front.client_info,
                this->client_execute, this->front.keymap.key_flags,
                this->server_auto_reconnect_packet);
            break;

        case MODULE_VNC:
            this->create_mod_vnc(
                authentifier, report_message, this->ini,
                this->front, this->front.client_info,
                this->client_execute, this->front.keymap.key_flags);
            break;

        default:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception");
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }

    rdp_api* get_rdp_api() const override {
        return this->rdpapi;
    }

private:
    unique_fd connect_to_target_host(ReportMessageApi& report_message, trkeys::TrKey const& authentification_fail, char const * protocol)
    {
        auto throw_error = [this, &protocol, &report_message](char const* error_message, int id) {
            LOG_PROXY_SIEM("TARGET_CONNECTION_FAILED",
                R"(target="%s" host="%s" port="%d" reason="%s")",
                this->ini.get<cfg::context::real_target_device>(),
                this->ini.get<cfg::context::target_host>(),
                this->ini.get<cfg::context::target_port>(),
                error_message);

            ArcsightLogInfo arc_info;
            arc_info.name = "CONNECTION";
            arc_info.signatureID = ArcsightLogInfo::CONNECTION;
            arc_info.ApplicationProtocol = protocol;
            arc_info.WallixBastionStatus = "FAIL";
            arc_info.direction_flag = ArcsightLogInfo::SERVER_DST;
            report_message.log6("type=\"CONNECTION_FAILED\"", arc_info, this->session_reactor.get_current_time());

            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::target_fail, language(this->ini)));

            LOG(LOG_ERR, "%s", (id == 1)
                ? "Failed to connect to remote TCP host (1)"
                : "Failed to connect to remote TCP host (2)");
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        };

        LOG_PROXY_SIEM("TARGET_CONNECTION",
            R"(target="%s" host="%s" port="%d")",
            this->ini.get<cfg::context::real_target_device>(),
            this->ini.get<cfg::context::target_host>(),
            this->ini.get<cfg::context::target_port>());

        const char * ip = this->ini.get<cfg::context::target_host>().c_str();
        char ip_addr[256] {};
        in_addr s4_sin_addr;
        if (auto error_message = resolve_ipv4_address(ip, s4_sin_addr)) {
            // TODO: actually this is DNS Failure or invalid address
            throw_error(error_message, 1);
        }

        snprintf(ip_addr, sizeof(ip_addr), "%s", inet_ntoa(s4_sin_addr));

        char const* error_message = nullptr;
        unique_fd client_sck = ip_connect(ip, this->ini.get<cfg::context::target_port>(), 3, 1000, &error_message);

        if (!client_sck.is_open()) {
            throw_error(error_message, 2);
        }

        this->ini.set<cfg::context::auth_error_message>(TR(authentification_fail, language(this->ini)));
        this->ini.set<cfg::context::ip_target>(ip_addr);

        return client_sck;
    }

    void create_mod_rdp(
        AuthApi& authentifier, ReportMessageApi& report_message,
        Inifile& ini, FrontAPI& front, ClientInfo client_info,
        ClientExecute& client_execute, Keymap2::KeyFlags key_flags,
        std::array<uint8_t, 28>& server_auto_reconnect_packet);

    void create_mod_vnc(
        AuthApi& authentifier, ReportMessageApi& report_message,
        Inifile& ini, FrontAPI& front, ClientInfo const& client_info,
        ClientExecute& client_execute, Keymap2::KeyFlags key_flags);

    Font& load_font()
    {
        if (this->_font_is_loaded) {
            return this->_font;
        }

        this->_font = Font(
            app_path(AppPath::DefaultFontFile),
            ini.get<cfg::globals::spark_view_specific_glyph_width>());

        this->_font_is_loaded = true;
        return this->_font;
    }

    Theme& load_theme()
    {
        if (this->_theme_is_loaded) {
            return this->_theme;
        }

        auto & theme_name = this->ini.get<cfg::internal_mod::theme>();
        if (this->ini.get<cfg::debug::config>()) {
            LOG(LOG_INFO, "LOAD_THEME: %s", theme_name);
        }

        ::load_theme(this->_theme, theme_name);

        this->_theme_is_loaded = true;
        return this->_theme;
    }

    Theme _theme;
    Font _font;
    bool _theme_is_loaded = false;
    bool _font_is_loaded = false;
};
