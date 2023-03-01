/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "acl/file_system_license_store.hpp"
#include "acl/module_manager/create_module_rdp.hpp"
#include "acl/module_manager/create_module_vnc.hpp"
#include "capture/cryptofile.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/events.hpp"
#include "gdi/osd_api.hpp"
#include "headlessclient/headless_cli_options.hpp"
#include "headlessclient/headless_configuration.hpp"
#include "headlessclient/headless_front.hpp"
#include "headlessclient/headless_repl.hpp"
#include "headlessclient/headless_session_log.hpp"
#include "keyboard/keylayouts.hpp"
#include "mod/null/null.hpp"
#include "RAIL/client_execute.hpp"
#include "system/scoped_ssl_init.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/fixed_random.hpp"
#include "utils/netutils.hpp"
#include "utils/redirection_info.hpp"
#include "utils/select.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"
#include "utils/theme.hpp"
#include "utils/to_timeval.hpp"
#include "utils/out_param.hpp"


static void show_prompt()
{
    write(1, ">>> ", 4);
}

static bool wait_and_draw_event(SocketTransport& trans, EventManager& event_manager)
{
    const auto now = MonotonicTimePoint::clock::now();
    event_manager.get_writable_time_base().monotonic_time = now;

    int max = 0;
    fd_set rfds;
    io_fd_zero(rfds);

    event_manager.for_each_fd([&](int fd){
        io_fd_set(fd, rfds);
        max = std::max(max, fd);
    });

    const bool nodelay = trans.has_tls_pending_data();

    const auto next_timeout = event_manager.next_timeout();
    timeval timeout = {};
    timeval* timeout_ptr = &timeout;
    if (!nodelay) {
        if (now < next_timeout) {
            timeout = to_timeval(next_timeout - now);
        }
        else {
            timeout_ptr = nullptr;
        }
    }

    const int num = select(max + 1, &rfds, nullptr, nullptr, timeout_ptr);

    if (num < 0) {
        LOG(LOG_ERR, "RDP CLIENT :: errno = %s", strerror(errno));
        return false;
    }

    if (nodelay) {
        io_fd_set(trans.get_fd(), rfds);
    }

    event_manager.get_writable_time_base().monotonic_time = MonotonicTimePoint::clock::now();
    event_manager.execute_events([&rfds](int fd){
        return io_fd_isset(fd, rfds);
    }, false);

    return true;
}

static KeyLayout const& get_layout(KeyLayout::KbdId keylayout)
{
    KeyLayout const* playout = find_layout_by_id(keylayout);
    return playout ? *playout : KeyLayout::null_layout();
}

namespace
{

struct Repl
{
    bool quit = false;
    bool start_connection = false;
    bool disconnection = false;
    bool has_delay_cmd = false;

    std::string delayed_cmd;
    HeadlessRepl repl;

    bool is_eof() const
    {
        return repl.is_eof();
    }

    bool wait_connect(HeadlessFront& front)
    {
        null_mod mod;

        int fd = 0;
        fd_set rfds;
        io_fd_zero(rfds);
        io_fd_set(fd, rfds);

        do {
            show_prompt();

            const int num = select(fd + 1, &rfds, nullptr, nullptr, nullptr);
            // end of file
            if (num < 0) {
                LOG(LOG_ERR, "RDP CLIENT :: errno = %s", strerror(errno));
                quit = true;
                return false;
            }

            if (!execute_command(front, mod)) {
                return false;
            }
        } while (!repl.is_eof());

        return true;
    }

    bool execute_command(HeadlessFront& front, mod_api& mod)
    {
        HeadlessCommand& cmd_ctx = front.command();

        switch (cmd_ctx.execute_command(repl.read_command(), mod)) {
            case HeadlessCommand::Result::Ok:
                break;

            case HeadlessCommand::Result::OutputResult:
                front.print_output_resut();
                break;

            case HeadlessCommand::Result::Fail:
                front.print_fail_result();
                break;

            case HeadlessCommand::Result::Connect:
            case HeadlessCommand::Result::Reconnect:
                disconnection = true;
                start_connection = true;
                return false;

            case HeadlessCommand::Result::Disconnect:
                disconnection = true;
                return false;

            case HeadlessCommand::Result::ConfigStr:
                front.read_config_str();
                break;

            case HeadlessCommand::Result::ConfigFile:
                front.read_config_file();
                break;

            case HeadlessCommand::Result::PrintScreen:
                try {
                    front.dump_png(cmd_ctx.png_path, cmd_ctx.mouse_x, cmd_ctx.mouse_y);
                }
                catch (...) {
                    // ignore error
                }
                break;

            case HeadlessCommand::Result::Delay:
                delayed_cmd = front.command().output_message.as<std::string_view>();
                has_delay_cmd = true;
                break;

            case HeadlessCommand::Result::Quit:
                disconnection = true;
                quit = true;
                return false;
        }

        return true;
    }
};

}


int main(int argc, char const** argv)
{
    set_exception_handler_pretty_message();

    HeadlessCliOptions options;
    switch (options.parse(argc, argv)) {
        case HeadlessCliOptions::Result::Error: return 1;
        case HeadlessCliOptions::Result::Exit: return 0;
        case HeadlessCliOptions::Result::Ok: break;
    }

    ClientInfo client_info;
    headless_init_client_info(client_info);
    client_info.screen_info = options.screen_info;
    client_info.keylayout = options.keylayout;

    if (options.print_client_info_section) {
        printf("%s", headless_client_info_config_as_string(client_info).c_str());
        return 0;
    }

    Inifile ini;
    headless_init_ini(ini);
    printf("Config filename: '%s'\n", options.config_filename);
    load_headless_config_from_file(ini, client_info, options.config_filename);

    Repl repl;
    FixedRandom fixed_random;
    UdevRandom udev_random;
    auto& random = options.use_fixed_random ? static_cast<Random&>(udev_random) : fixed_random;
    RedirectionInfo redir_info;
    std::array<unsigned char, 28> server_auto_reconnect_packet {};
    auto perform_automatic_reconnection = PerformAutomaticReconnection::No;
    Theme theme;
    Font glyph;
    gdi::NullOsd osd;
    EventManager event_manager;
    HeadlessSessionLog session_log;
    FileSystemLicenseStore file_system_license_store {options.license_store_path};
    CryptoContext cctx;

    HeadlessFront front(event_manager.get_writable_time_base(), ini, client_info);

    ScopedSslInit scoped_ssl;

    HeadlessCommand& cmd_ctx = front.command();
    cmd_ctx.is_kbdmap_en = options.is_cmd_kbdmap_en;
    cmd_ctx.enable_wrm = options.enable_wrm_capture;
    cmd_ctx.enable_png = options.enable_png_capture;
    cmd_ctx.enable_record_transport = options.enable_record_transport_capture;
    cmd_ctx.png_path = options.output_png_path;
    cmd_ctx.wrm_path = options.output_wrm_path;
    cmd_ctx.record_transport_path = options.output_record_transport_path;
    cmd_ctx.ip_address = options.ip_address;
    cmd_ctx.username = options.username;
    cmd_ctx.password = options.password;

    std::string ip_address = options.ip_address;
    repl.start_connection = *options.ip_address;
    bool interactive = !repl.start_connection || options.interactive;

    gdi::GraphicDispatcher gds;

    while (!repl.quit && (!repl.is_eof() || (repl.start_connection && options.persist))) {
        front.must_be_stop_capture();

        /*
         * Wait connect command / ready to connect event
         */
        if (!repl.start_connection && perform_automatic_reconnection == PerformAutomaticReconnection::No) {
            if (!interactive) {
                break;
            }

            repl.wait_connect(front);
            continue;
        }

        repl.start_connection = false;

        auto& gd = front.prepare_gd();

        ClientExecute rail_client_execute(
            event_manager.get_time_base(), gd, front,
            client_info.window_list_caps,
            ini.get<cfg::debug::mod_internal>() & 1);

        std::unique_ptr<mod_api> mod;
        SocketTransport* sck_trans = nullptr;

        if (perform_automatic_reconnection == PerformAutomaticReconnection::No) {
            // disable encryption
            ini.set<cfg::globals::trace_type>(TraceType::localfile);

            ini.set<cfg::context::target_port>(cmd_ctx.port);
            ini.set_acl<cfg::context::target_host>(cmd_ctx.ip_address);
            ini.set_acl<cfg::context::target_password>(cmd_ctx.password);
            ini.set_acl<cfg::globals::target_user>(cmd_ctx.username);
        }

        auto& event_container = event_manager.get_events();

        std::unique_ptr<RecorderTransport> recorder_transport;
        auto maybe_make_transport_record = [&](Transport& trans) -> Transport& {
            if (!cmd_ctx.enable_record_transport) {
                return trans;
            }

            recorder_transport = std::make_unique<RecorderTransport>(
                trans, event_manager.get_time_base(), cmd_ctx.record_transport_path.c_str()
            );
            return *recorder_transport;
        };

        try {
            /*
             * Open module
             */
            ModPack mod_pack = cmd_ctx.is_rdp
                ? create_mod_rdp(
                    gd, osd, redir_info, ini, front, client_info, rail_client_execute,
                    kbdtypes::KeyLocks(), glyph, theme, event_container, session_log,
                    file_system_license_store, random, cctx, server_auto_reconnect_packet,
                    std::exchange(perform_automatic_reconnection, PerformAutomaticReconnection::No),
                    maybe_make_transport_record
                )
                : create_mod_vnc(
                    gd, ini, front, client_info, rail_client_execute,
                    get_layout(client_info.keylayout), kbdtypes::KeyLocks(),
                    glyph, theme, event_container, session_log
                );

            mod.reset(mod_pack.mod);
            sck_trans = mod_pack.psocket_transport;

            repl.disconnection = false;

            auto is_actif = [&]{
                return (!repl.is_eof() || options.persist)
                    && !repl.disconnection
                    && mod->get_mod_signal() == BACK_EVENT_NONE;
            };

            /*
             * Wait up and running
             */
            while (is_actif() && !mod->is_up_and_running()) {
                if (!wait_and_draw_event(*sck_trans, event_manager)) {
                    continue;
                }
            }

            if (!is_actif()) {
                continue;
            }

            LOG(LOG_INFO, "Mod is up and running");

            /*
             * Mod is ready
             */

            EventRef delayed_cmd_ref;
            auto mk_delayed_cmd_when_available = [&]{
                if (!repl.has_delay_cmd) {
                    return;
                }

                repl.has_delay_cmd = false;
                if (!repl.delayed_cmd.empty() && cmd_ctx.delay.count() >= 0) {
                    auto delay = std::chrono::duration_cast<MonotonicTimePoint::duration>(cmd_ctx.delay);
                    auto repeat = (cmd_ctx.repeat_delay < 0)
                        ? ~uint64_t(0) // infinite loop
                        : static_cast<uint64_t>(cmd_ctx.repeat_delay);

                    auto fn = [delay, repeat, &repl, &front, &mod](Event& ev) mutable {
                        if (--repeat == 0) {
                            ev.garbage = true;
                            return;
                        }

                        ev.alarm.reset_timeout(delay);
                        repl.execute_command(front, *mod);
                        if (repl.has_delay_cmd) {
                            repl.has_delay_cmd = false;
                            ev.garbage = true;
                        }
                    };

                    delayed_cmd_ref = event_container.event_creator()
                        .create_event_timeout("delayed_cmd", nullptr, delay, fn);
                }
                else {
                    delayed_cmd_ref.garbage();
                }
            };

            mk_delayed_cmd_when_available();

            show_prompt();

            auto read_input = [&](Event&){
                if (repl.execute_command(front, *mod)) {
                    mk_delayed_cmd_when_available();
                    show_prompt();
                }
            };

            EventRef input_ev = event_container.event_creator()
                .create_event_fd_without_timeout("stdin", nullptr, 0, read_input);

            while (is_actif() && wait_and_draw_event(*sck_trans, event_manager)) {
            }

            if (repl.disconnection) {
                mod->disconnect();
            }
        }
        catch (Error const& e) {
            LOG(LOG_ERR, "Headless Client: Exception raised = %s !", e.errmsg());

            if (ERR_AUTOMATIC_RECONNECTION_REQUIRED == e.id) {
                perform_automatic_reconnection = PerformAutomaticReconnection::Yes;
                continue;
            }

            if ((e.id == ERR_TRANSPORT_WRITE_FAILED || e.id == ERR_TRANSPORT_NO_MORE_DATA)
             && sck_trans && mod
             && static_cast<uintptr_t>(sck_trans->get_fd()) == e.data
             && ini.get<cfg::mod_rdp::auto_reconnection_on_losing_target_link>()
             && mod->is_auto_reconnectable()
             && !mod->server_error_encountered()
            ) {
                perform_automatic_reconnection = PerformAutomaticReconnection::Yes;
                continue;
            }

            if (ERR_RDP_SERVER_REDIR != e.id) {
                options.persist = false;
                continue;
            }

            // SET new target in ini
            const char * host = char_ptr_cast(redir_info.host);
            const char * username = char_ptr_cast(redir_info.username);
            if (redir_info.dont_store_username && username[0] != 0) {
                LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                ini.set_acl<cfg::globals::target_user>(username);
            }
            LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
            ini.set_acl<cfg::context::target_host>(host);
            repl.start_connection = true;
        }

        options.persist = false;
    }

    front.must_be_stop_capture();

    return 0;
}
