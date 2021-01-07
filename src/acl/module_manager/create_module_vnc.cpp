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
#include "mod/vnc/vnc.hpp"
#include "mod/metrics_hmac.hpp"
#include "mod/vnc/vnc.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "transport/socket_transport.hpp"
#include "acl/mod_pack.hpp"
#include "acl/mod_wrapper.hpp"
#include "acl/module_manager/create_module_vnc.hpp"
#include "acl/module_manager/create_module_rail.hpp"
#include "acl/connect_to_target_host.hpp"
#include "utils/sugar/unique_fd.hpp"


namespace
{

struct ModVNCWithMetrics : public mod_vnc
{
    struct ModMetrics : Metrics
    {
        using Metrics::Metrics;

        VNCMetrics protocol_metrics{*this};
    };

private:
    std::unique_ptr<ModMetrics> metrics;

    EventsGuard events_guard;

public:
    void set_metrics(std::unique_ptr<ModMetrics> && metrics, std::chrono::seconds log_interval)
    {
        assert(!this->metrics);
        this->metrics = std::move(metrics);
        this->events_guard.create_event_timeout(
            "VNC Metrics Timer",
            log_interval,
            [this,log_interval](Event& event)
            {
                event.alarm.reset_timeout(log_interval);
                this->metrics->log(event.alarm.now);
            });
    }

    ModVNCWithMetrics(Transport & t
           , gdi::GraphicApi & gd
           , EventContainer & events
           , const char * username
           , const char * password
           , FrontAPI & front
           // TODO: front width and front height should be provided through info
           , uint16_t front_width
           , uint16_t front_height
           , int keylayout
           , int key_flags
           , bool clipboard_up
           , bool clipboard_down
           , const char * encodings
           , ClipboardEncodingType clipboard_server_encoding_type
           , VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop
           , bool server_is_macos
           , bool server_is_unix
           , bool cursor_pseudo_encoding_supported
           , ClientExecute* rail_client_execute
           , VNCVerbose verbose
           , VNCMetrics * metrics
           , SessionLogApi& session_log)
    : mod_vnc(
        t, gd, events, username, password, front, front_width, front_height,
        keylayout, key_flags, clipboard_up, clipboard_down, encodings,
        clipboard_server_encoding_type, bogus_clipboard_infinite_loop,
        server_is_macos, server_is_unix, cursor_pseudo_encoding_supported,
        rail_client_execute, verbose, metrics, session_log)
    , events_guard(events)
    {
    }

    ~ModVNCWithMetrics() = default;
};


class ModWithSocketAndMetrics final : public mod_api
{
public:
    SocketTransport socket_transport;
    ModVNCWithMetrics mod;

private:
    ModWrapper & mod_wrapper;
    Inifile & ini;

public:
    ModWithSocketAndMetrics(
        ModWrapper & mod_wrapper, Inifile & ini,
        const char * name, unique_fd sck, SocketTransport::Verbose verbose,
        std::string * error_message,
        EventContainer& events,
        SessionLogApi& session_log,
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
                      , verbose, error_message)
    , mod(
          this->socket_transport, mod_wrapper.get_graphics(),
          events, username, password, front, front_width, front_height,
          keylayout, key_flags, clipboard_up, clipboard_down, encodings,
          clipboard_server_encoding_type, bogus_clipboard_infinite_loop,
          server_is_apple, send_alt_ksym, cursor_pseudo_encoding_supported,
          rail_client_execute, vnc_verbose, metrics, session_log)
    , mod_wrapper(mod_wrapper)
    , ini(ini)
    {
        this->mod_wrapper.target_info_is_shown = false;
    }

    ~ModWithSocketAndMetrics()
    {
        log_proxy::target_disconnection(
            this->ini.template get<cfg::context::auth_error_message>().c_str());
    }

    // from RdpInput
    void rdp_gdi_up_and_running() override
    {
        this->mod.rdp_gdi_up_and_running();
    }

    void rdp_gdi_down() override
    {
        this->mod.rdp_gdi_down();
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        this->mod.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    // from RdpInput
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        this->mod.rdp_input_mouse(device_flags, x, y, keymap);
    }

    // from RdpInput
    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        this->mod.rdp_input_unicode(unicode, flag);
    }

    // from RdpInput
    void rdp_input_invalidate(const Rect r) override
    {
        this->mod.rdp_input_invalidate(r);
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
    bool is_auto_reconnectable() const override {
        return this->mod.is_auto_reconnectable();
    }

    bool server_error_encountered() const override {
        return this->mod.server_error_encountered();
    }

    // from mod_api
    void disconnect() override
    {
        return this->mod.disconnect();
    }

    // from mod_api
    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        return this->mod.move_size_widget(left, top, width, height);
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

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override
    {
        this->mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
    }
};

}

ModPack create_mod_vnc(
    ModWrapper & mod_wrapper,
    Inifile& ini, FrontAPI& front, ClientInfo const& client_info,
    ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags,
    Font & glyphs,
    Theme & theme,
    EventContainer& events,
    SessionLogApi& session_log
    )
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'");

    unique_fd client_sck =
        connect_to_target_host(ini, session_log, trkeys::authentification_vnc_fail, ini.get<cfg::mod_vnc::enable_ipv6>());

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
            events.get_current_time(),
            ini.get<cfg::metrics::log_file_turnover_interval>(),
            ini.get<cfg::metrics::log_interval>());
    }

    const auto vnc_verbose = safe_cast<VNCVerbose>(ini.get<cfg::debug::mod_vnc>());

    auto new_mod = std::make_unique<ModWithSocketAndMetrics>(
        mod_wrapper,
        ini,
        name,
        std::move(client_sck),
        safe_cast<SocketTransport::Verbose>(ini.get<cfg::debug::sck_mod>()),
        nullptr,
        events,
        session_log,
        ini.get<cfg::globals::target_user>().c_str(),
        ini.get<cfg::context::target_password>().c_str(),
        front,
        client_info.screen_info.width,
        client_info.screen_info.height,
        front.get_keylayout(),
        key_flags,
        ini.get<cfg::mod_vnc::clipboard_up>(),
        ini.get<cfg::mod_vnc::clipboard_down>(),
        ini.get<cfg::mod_vnc::encodings>().c_str(),
        ini.get<cfg::mod_vnc::server_clipboard_encoding_type>()
            != ClipboardEncodingType::latin1
            ? mod_vnc::ClipboardEncodingType::UTF8
            : mod_vnc::ClipboardEncodingType::Latin1,
        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>(),
        ini.get<cfg::mod_vnc::server_is_macos>(),
        ini.get<cfg::mod_vnc::server_unix_alt>(),
        ini.get<cfg::mod_vnc::support_cursor_pseudo_encoding>(),
        (client_info.remote_program ? &rail_client_execute : nullptr),
        vnc_verbose,
        enable_metrics ? &metrics->protocol_metrics : nullptr
    );

    if (enable_metrics) {
        new_mod->mod.set_metrics(std::move(metrics), ini.get<cfg::metrics::log_interval>());
    }

    auto tmp_psocket_transport = &new_mod->socket_transport;

    if (!client_info.remote_program) {
        auto mod = new_mod.release();
        return ModPack{mod, nullptr, nullptr, false, false, tmp_psocket_transport};
    }

    gdi::GraphicApi& drawable = mod_wrapper.get_graphics();

    auto* host_mod = create_mod_rail(
        ini,
        events,
        drawable,
        front,
        client_info,
        rail_client_execute.adjust_rect(client_info.get_widget_rect()),
        std::move(new_mod),
        rail_client_execute,
        glyphs,
        theme,
        false
    );

    return ModPack{host_mod, nullptr, nullptr, false, false, tmp_psocket_transport};
}
