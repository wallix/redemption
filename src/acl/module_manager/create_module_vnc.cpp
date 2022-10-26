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
#include "mod/vnc/vnc_metrics.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "transport/socket_transport.hpp"
#include "acl/module_manager/create_module_vnc.hpp"
#include "acl/module_manager/create_module_rail.hpp"
#include "acl/module_manager/create_module_metrics.hpp"
#include "acl/connect_to_target_host.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "RAIL/client_execute.hpp"


namespace
{

struct VncData
{
    using ModVncMetrics = ModMetrics<VNCMetrics>;

    VncData(EventContainer & events,
        Inifile & ini, SocketTransport::Name name, unique_fd sck,
        SocketTransport::Verbose verbose,
        std::string * error_message
    )
    : events_guard(events)
    , socket_transport( name, std::move(sck)
                      , ini.get<cfg::context::target_host>()
                      , checked_int(ini.get<cfg::context::target_port>())
                      , ini.get<cfg::all_target_mod::connection_establishment_timeout>()
                      , ini.get<cfg::all_target_mod::connection_retry_count>()
                      , ini.get<cfg::all_target_mod::tcp_user_timeout>()
                      , std::chrono::milliseconds(ini.get<cfg::globals::mod_recv_timeout>())
                      , verbose, error_message)
    {}

    ~VncData() = default;

    void set_metrics(std::unique_ptr<ModVncMetrics> && metrics, std::chrono::seconds log_interval)
    {
        assert(!this->metrics);
        this->metrics = std::move(metrics);
        this->metrics->timed_log(this->events_guard, log_interval, "VNC Metrics Timer");
    }

    SocketTransport& get_transport()
    {
        return this->socket_transport;
    }

private:
    std::unique_ptr<ModVncMetrics> metrics;

    EventsGuard events_guard;

    struct FinalSocketTransport final : SocketTransport
    {
        using SocketTransport::SocketTransport;
    };

    FinalSocketTransport socket_transport;
};


class ModVNCWithSocketAndMetrics final : public VncData, public mod_vnc
{
    Inifile & ini;

public:
    ModVNCWithSocketAndMetrics(
        gdi::GraphicApi & drawable,
        Inifile & ini, SocketTransport::Name name, unique_fd sck,
        SocketTransport::Verbose verbose,
        std::string * error_message,
        EventContainer& events,
        SessionLogApi& session_log,
        const char* username,
        const char* password,
        FrontAPI& front,
        uint16_t front_width,
        uint16_t front_height,
        bool clipboard_up,
        bool clipboard_down,
        const char * encodings,
        mod_vnc::ClipboardEncodingType clipboard_server_encoding_type,
        VncBogusClipboardInfiniteLoop bogus_clipboard_infinite_loop,
        KeyLayout const& layout,
        kbdtypes::KeyLocks locks,
        bool server_is_apple,
        bool send_alt_ksym,
        bool cursor_pseudo_encoding_supported,
        ClientExecute* rail_client_execute,
        VNCVerbose vnc_verbose,
        VNCMetrics * metrics
        )
    : VncData(events, ini, name, std::move(sck), verbose, error_message)
    , mod_vnc(
          this->get_transport(), drawable,
          events, username, password, front, front_width, front_height,
          clipboard_up, clipboard_down, encodings,
          clipboard_server_encoding_type, bogus_clipboard_infinite_loop,
          layout, locks, server_is_apple, send_alt_ksym, cursor_pseudo_encoding_supported,
          rail_client_execute, vnc_verbose, metrics, session_log)
    , ini(ini)
    {}

    ~ModVNCWithSocketAndMetrics()
    {
        log_proxy::target_disconnection(
            this->ini.template get<cfg::context::auth_error_message>().c_str(),
            this->ini.template get<cfg::context::session_id>().c_str());
    }
};

} // namespace anonymous

ModPack create_mod_vnc(
    gdi::GraphicApi & drawable,
    Inifile& ini, FrontAPI& front, ClientInfo const& client_info,
    ClientExecute& rail_client_execute,
    KeyLayout const& layout,
    kbdtypes::KeyLocks locks,
    Ref<Font const> glyphs,
    Theme & theme,
    EventContainer& events,
    SessionLogApi& session_log
    )
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC'");

    unique_fd client_sck =
        connect_to_target_host(ini, session_log, trkeys::authentification_vnc_fail, ini.get<cfg::mod_vnc::enable_ipv6>(),
            ini.get<cfg::all_target_mod::connection_establishment_timeout>(), ini.get<cfg::all_target_mod::connection_retry_count>(),
            ini.get<cfg::all_target_mod::tcp_user_timeout>());

    bool const enable_metrics = (ini.get<cfg::metrics::enable_vnc_metrics>()
        && create_metrics_directory(ini.get<cfg::metrics::log_dir_path>().as_string()));

    auto metrics = (!enable_metrics)
        ? std::unique_ptr<VncData::ModVncMetrics>()
        : VncData::ModVncMetrics::make_unique(events, ini, client_info.screen_info);

    const auto vnc_verbose = safe_cast<VNCVerbose>(ini.get<cfg::debug::mod_vnc>());

    std::unique_ptr<RailModuleHostMod> host_mod {
        client_info.remote_program
        ? create_mod_rail(ini,
                          events,
                          drawable,
                          front,
                          client_info,
                          rail_client_execute,
                          glyphs,
                          theme,
                          false)
        : nullptr
    };

    auto new_mod = std::make_unique<ModVNCWithSocketAndMetrics>(
        host_mod ? host_mod->proxy_gd() : drawable,
        ini,
        "VNC Target"_sck_name,
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
        ini.get<cfg::mod_vnc::clipboard_up>(),
        ini.get<cfg::mod_vnc::clipboard_down>(),
        ini.get<cfg::mod_vnc::encodings>().c_str(),
        ini.get<cfg::mod_vnc::server_clipboard_encoding_type>()
            != ClipboardEncodingType::latin1
            ? mod_vnc::ClipboardEncodingType::UTF8
            : mod_vnc::ClipboardEncodingType::Latin1,
        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>(),
        layout,
        locks,
        ini.get<cfg::mod_vnc::server_is_macos>(),
        ini.get<cfg::mod_vnc::server_unix_alt>(),
        ini.get<cfg::mod_vnc::support_cursor_pseudo_encoding>(),
        (client_info.remote_program ? &rail_client_execute : nullptr),
        vnc_verbose,
        enable_metrics ? &metrics->protocol_metrics : nullptr
    );

    if (enable_metrics) {
        new_mod->set_metrics(std::move(metrics), ini.get<cfg::metrics::log_interval>());
    }

    auto tmp_psocket_transport = &new_mod->get_transport();

    if (!client_info.remote_program) {
        return ModPack{new_mod.release(), nullptr, nullptr, false, false, tmp_psocket_transport};
    }

    host_mod->set_mod(std::move(new_mod));
    return ModPack{host_mod.release(), nullptr, nullptr, false, false, tmp_psocket_transport};
}
