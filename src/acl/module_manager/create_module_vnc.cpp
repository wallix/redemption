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

#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/report_message_api.hpp"
#include "mod/vnc/vnc.hpp"
#include "mod/metrics_hmac.hpp"
#include "utils/sugar/unique_fd.hpp"

#include "acl/module_manager.hpp"
#include "mod/vnc/vnc.hpp"

struct ModVNCWithMetrics : public mod_vnc
{
    struct ModMetrics : Metrics
    {
        using Metrics::Metrics;

        VNCMetrics protocol_metrics{*this};
    };

    std::unique_ptr<ModMetrics> metrics;
    TimerPtr metrics_timer;
    using mod_vnc::mod_vnc;
};


class ModWithSocketAndMetrics final : public mod_api
{
    SocketTransport socket_transport;
public:
    ModVNCWithMetrics mod;
private:
    ModWrapper & mod_wrapper;
    Inifile & ini;
    bool target_info_is_shown = false;

public:

    ModWithSocketAndMetrics(ModWrapper & mod_wrapper, Inifile & ini, AuthApi & /*authentifier*/,
        const char * name, unique_fd sck, uint32_t verbose,
        std::string * error_message, 
        SessionReactor& session_reactor,
        GraphicEventContainer& graphic_events_,
        const char* username,
        const char* password,
        FrontAPI& front,
        uint16_t front_width,
        uint16_t front_height,
        int keylayout,
        int key_flags,
        bool clipboard_up,
        bool clipboard_down,
        const char * encodings,
        mod_vnc::ClipboardEncodingType clipboard_server_encoding_type,
        VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop,
        ReportMessageApi& report_message,
        bool server_is_apple,
        bool send_alt_ksym,
        bool cursor_pseudo_encoding_supported,
        ClientExecute* rail_client_execute,
        VNCVerbose vnc_verbose,
        VNCMetrics * metrics
        )
    : socket_transport( name, std::move(sck)
                     , ini.get<cfg::context::target_host>().c_str()
                     , ini.get<cfg::context::target_port>()
                     , std::chrono::milliseconds(ini.get<cfg::globals::mod_recv_timeout>())
                     , to_verbose_flags(verbose), error_message)
    , mod(this->socket_transport, session_reactor, graphic_events_, username, password, front, front_width, front_height,
          keylayout, key_flags, clipboard_up, clipboard_down, encodings, 
          clipboard_server_encoding_type, bogus_clipboard_infinite_loop,
          report_message, server_is_apple, send_alt_ksym, cursor_pseudo_encoding_supported, 
          rail_client_execute, vnc_verbose, metrics)
    , mod_wrapper(mod_wrapper)
    , ini(ini)
    {
        this->mod_wrapper.set_mod_transport(&this->socket_transport);
    }

    ~ModWithSocketAndMetrics()
    {
        this->mod_wrapper.set_mod_transport(nullptr);
        log_proxy::target_disconnection(
            this->ini.template get<cfg::context::auth_error_message>().c_str());
    }

    // from RdpInput
    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        //LOG(LOG_INFO, "mod_wrapper::rdp_input_scancode: keyCode=0x%X keyboardFlags=0x%04X this=<%p>", param1, param3, this);
        if (this->mod_wrapper.try_input_scancode(param1, param2, param3, param4, keymap)) {
            this->target_info_is_shown = false;
            return ;
        }

        this->mod.rdp_input_scancode(param1, param2, param3, param4, keymap);

        Inifile const& ini = this->ini;

        if (ini.get<cfg::globals::enable_osd_display_remote_target>() && (param1 == Keymap2::F12)) {
            bool const f12_released = (param3 & SlowPath::KBDFLAGS_RELEASE);
            if (this->target_info_is_shown && f12_released) {
                // LOG(LOG_INFO, "Hide info");
                this->mod_wrapper.clear_osd_message();
                this->target_info_is_shown = false;
            }
            else if (!this->target_info_is_shown && !f12_released) {
                // LOG(LOG_INFO, "Show info");
                std::string msg;
                msg.reserve(64);
                if (ini.get<cfg::client::show_target_user_in_f12_message>()) {
                    msg  = ini.get<cfg::globals::target_user>();
                    msg += "@";
                }
                msg += ini.get<cfg::globals::target_device>();
                const uint32_t enddate = ini.get<cfg::context::end_date_cnx>();
                if (enddate) {
                    const auto now = time(nullptr);
                    const auto elapsed_time = enddate - now;
                    // only if "reasonable" time
                    if (elapsed_time < 60*60*24*366L) {
                        msg += "  [";
                        msg += time_before_closing(elapsed_time, Translator(ini));
                        msg += ']';
                    }
                }
                this->mod_wrapper.osd_message_fn(std::move(msg), false);
                this->target_info_is_shown = true;
            }
        }
    }

    // from RdpInput
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        if (this->mod_wrapper.try_input_mouse(device_flags, x, y, keymap)) {
            this->target_info_is_shown = false;
            return ;
        }

        this->mod.rdp_input_mouse(device_flags, x, y, keymap);
    }

    // from RdpInput
    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        this->mod.rdp_input_unicode(unicode, flag);
    }

    // from RdpInput
    void rdp_input_invalidate(const Rect r) override
    {
        if (this->mod_wrapper.try_input_invalidate(r)) {
            return ;
        }

        this->mod.rdp_input_invalidate(r);
    }

    // from RdpInput
    void rdp_input_invalidate2(array_view<Rect const> vr) override
    {
        if (this->mod_wrapper.try_input_invalidate2(vr)) {
            return ;
        }

        this->mod.rdp_input_invalidate2(vr);
    }

    // from RdpInput
    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        return this->mod.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    void refresh(Rect clip) override
    {
        return this->mod.refresh(clip);
    }

    // from mod_api
    [[nodiscard]] bool is_up_and_running() const override { return false; }

    // from mod_api
    // support auto-reconnection
    bool is_auto_reconnectable() override {
        return this->mod.is_auto_reconnectable();
    }

    // from mod_api
    void disconnect() override 
    {
        return this->mod.disconnect();
    }

    // from mod_api
    void display_osd_message(std::string const & message) override 
    {
        this->mod_wrapper.osd_message_fn(message, true);
        //return this->mod.display_osd_message(message);
    }

    // from mod_api
    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        return this->mod.move_size_widget(left, top, width, height);
    }

    // from mod_api
    bool disable_input_event_and_graphics_update(bool disable_input_event, bool disable_graphics_update) override 
    {
        return this->mod.disable_input_event_and_graphics_update(disable_input_event, disable_graphics_update);
    }

    // from mod_api
    void send_input(int time, int message_type, int device_flags, int param1, int param2) override 
    {
        return this->mod.send_input(time, message_type, device_flags, param1, param2);
    }

    // from mod_api
    [[nodiscard]] Dimension get_dim() const override 
    {
        return this->mod.get_dim();
    }

    // from mod_api
    void log_metrics() override 
    {
        return this->mod.log_metrics();
    }

    // from mod_api
    void DLP_antivirus_check_channels_files() override
    {
        return this->mod.DLP_antivirus_check_channels_files(); 
    }
};

void ModuleManager::create_mod_vnc(ModWrapper & mod_wrapper,
    AuthApi& authentifier, ReportMessageApi& report_message,
    Inifile& ini, gdi::GraphicApi & drawable, FrontAPI& front, ClientInfo const& client_info,
    ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags)
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'");

    unique_fd client_sck = this->connect_to_target_host(
        report_message, trkeys::authentification_vnc_fail);

    try {
        const char * const name = "VNC Target";

        bool const enable_metrics = (ini.get<cfg::metrics::enable_vnc_metrics>()
            && create_metrics_directory(ini.get<cfg::metrics::log_dir_path>().as_string()));

        std::unique_ptr<ModVNCWithMetrics::ModMetrics> metrics;

        if (enable_metrics) {
            metrics = std::make_unique<ModVNCWithMetrics::ModMetrics>(
                ini.get<cfg::metrics::log_dir_path>().as_string(),
                ini.get<cfg::context::session_id>(),
                hmac_user(
                    ini.get<cfg::globals::auth_user>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_account(
                    ini.get<cfg::globals::target_user>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_device_service(
                    ini.get<cfg::globals::target_device>(),
                    ini.get<cfg::context::target_service>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_client_info(
                    ini.get<cfg::globals::host>(),
                    client_info.screen_info,
                    ini.get<cfg::metrics::sign_key>()),
                this->timeobj.get_time(),
                ini.get<cfg::metrics::log_file_turnover_interval>(),
                ini.get<cfg::metrics::log_interval>());
        }

        auto new_mod = std::make_unique<ModWithSocketAndMetrics>(
            mod_wrapper,
            this->ini,
            authentifier,
            name,
            std::move(client_sck),
            ini.get<cfg::debug::mod_vnc>(),
            nullptr,
            this->session_reactor,
            this->graphic_events_,
            ini.get<cfg::globals::target_user>().c_str(),
            ini.get<cfg::context::target_password>().c_str(),
            front,
            client_info.screen_info.width,
            client_info.screen_info.height,
            client_info.keylayout,
            key_flags,
            ini.get<cfg::mod_vnc::clipboard_up>(),
            ini.get<cfg::mod_vnc::clipboard_down>(),
            ini.get<cfg::mod_vnc::encodings>().c_str(),
            ini.get<cfg::mod_vnc::server_clipboard_encoding_type>()
                != ClipboardEncodingType::latin1
                ? mod_vnc::ClipboardEncodingType::UTF8
                : mod_vnc::ClipboardEncodingType::Latin1,
            ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>(),
            report_message,
            ini.get<cfg::mod_vnc::server_is_apple>(),
            ini.get<cfg::mod_vnc::server_unix_alt>(),
            ini.get<cfg::mod_vnc::support_cursor_pseudo_encoding>(),
            (client_info.remote_program ? &rail_client_execute : nullptr),
            to_verbose_flags(ini.get<cfg::debug::mod_vnc>()),
            enable_metrics ? &metrics->protocol_metrics : nullptr
        );

        if (enable_metrics) {
            new_mod->mod.metrics = std::move(metrics);
            new_mod->mod.metrics_timer = session_reactor.create_timer()
                .set_delay(std::chrono::seconds(ini.get<cfg::metrics::log_interval>()))
                .on_action([metrics = new_mod->mod.metrics.get()](JLN_TIMER_CTX ctx){
                    metrics->log(ctx.get_current_time());
                    return ctx.ready();
                })
            ;
        }

        if (client_info.remote_program) {
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'RailModuleHostMod'");

            Rect adjusted_client_execute_rect =
                rail_client_execute.adjust_rect(client_info.cs_monitor.get_widget_rect(
                    client_info.screen_info.width,
                    client_info.screen_info.height
                ));

            std::string target_info = str_concat(
              ini.get<cfg::context::target_str>(),
              ':',
              ini.get<cfg::globals::primary_user_id>());

            rail_client_execute.set_target_info(target_info);

            auto* host_mod = new RailModuleHostMod(
                ini,
                this->session_reactor,
                this->graphic_events_,
                drawable,
                front,
                client_info.screen_info.width,
                client_info.screen_info.height,
                adjusted_client_execute_rect,
                std::move(new_mod),
                rail_client_execute,
                this->glyphs,
                this->theme,
                client_info.cs_monitor,
                false
            );
            this->set_mod(mod_wrapper, host_mod);
            mod_wrapper.rail_module_host_mod_ptr = host_mod;
            LOG(LOG_INFO, "ModuleManager::internal module 'RailModuleHostMod' ready");
        }
        else {
            this->set_mod(mod_wrapper, new_mod.release());
        }
    }
    catch (...) {
        report_message.log6(LogId::SESSION_CREATION_FAILED, this->session_reactor.get_current_time(), {});

        throw;
    }

    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded");
    ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
    this->connected = true;
}
