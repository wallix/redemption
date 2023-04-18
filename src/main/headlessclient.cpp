/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "acl/auth_api.hpp"
#include "acl/file_system_license_store.hpp"
#include "acl/module_manager/create_module_rdp.hpp"
#include "acl/module_manager/create_module_vnc.hpp"
#include "capture/cryptofile.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "core/channel_list.hpp"
#include "core/events.hpp"
#include "core/front_api.hpp"
#include "gdi/osd_api.hpp"
#include "gdi/graphic_dispatcher.hpp"
#include "headlessclient/headless_cli_options.hpp"
#include "headlessclient/headless_configuration.hpp"
#include "headlessclient/headless_wrm_capture.hpp"
#include "headlessclient/headless_graphics.hpp"
#include "headlessclient/headless_command.hpp"
#include "headlessclient/headless_command_reader.hpp"
#include "headlessclient/headless_path.hpp"
#include "headlessclient/headless_repl.hpp"
#include "headlessclient/input_collector.hpp"
#include "keyboard/keymap.hpp"
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
#include "utils/static_string.hpp"
#include "utils/uninit_buffer.hpp"
#include "utils/key_qvalue_pairs.hpp"


static void show_prompt()
{
    write(1, ">>> ", 4);
}

static void update_times(EventManager& em)
{
    auto& tb = em.get_writable_time_base();
    tb = tb.now();
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

    // TODO trans.has_data_to_write();

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
    update_times(event_manager);

    if (num < 0) {
        LOG(LOG_ERR, "RDP CLIENT :: errno = %s", strerror(errno));
        return false;
    }

    if (nodelay) {
        io_fd_set(trans.get_fd(), rfds);
    }

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


int main(int argc, char const** argv)
{
    set_exception_handler_pretty_message();

    HeadlessCliOptions options;
    switch (options.parse(argc, argv)) {
        case HeadlessCliOptions::Result::Error: return 1;
        case HeadlessCliOptions::Result::Exit: return 0;
        case HeadlessCliOptions::Result::Ok: break;
    }

    char const* automation_script = options.headless_script_path;
    bool has_automation_script = automation_script;

    auto input = has_automation_script ? unique_fd(automation_script) : unique_fd(0);

    if (has_automation_script && !input) {
        printf("Automation open error: %s: %s", automation_script, strerror(errno));
        return 1;
    }

    auto* home = getenv("HOME");
    HeadlessRepl repl{(home && *home) ? std::string_view(home) : std::string_view()};

    auto& client_info = repl.client_info;
    auto& ini = repl.ini;
    auto& event_manager = repl.event_manager;

    headless_init_client_info(client_info);
    client_info.screen_info = options.screen_info;
    client_info.keylayout = options.keylayout;

    if (options.print_client_info_section) {
        printf("%s", headless_client_info_config_as_string(client_info).c_str());
        return 0;
    }

    headless_init_ini(ini);
    printf("Config filename: '%s'\n", options.config_filename);
    load_headless_config_from_file(ini, client_info, options.config_filename);

    FixedRandom fixed_random;
    UdevRandom udev_random;
    auto& random = options.use_fixed_random ? static_cast<Random&>(udev_random) : fixed_random;
    RedirectionInfo redir_info;
    std::array<unsigned char, 28> server_auto_reconnect_packet {};
    auto perform_automatic_reconnection = PerformAutomaticReconnection::No;
    Theme theme;
    Font glyph;
    gdi::NullOsd osd;
    FileSystemLicenseStore file_system_license_store {options.license_store_path};
    CryptoContext cctx;

    ScopedSslInit scoped_ssl;

    HeadlessCommand& cmd_ctx = repl.cmd_ctx;
    cmd_ctx.is_kbdmap_en = options.is_cmd_kbdmap_en;
    repl.enable_wrm = options.enable_wrm_capture;
    repl.enable_png = options.enable_png_capture;
    repl.enable_record_transport = options.enable_record_transport_capture;
    repl.png_path = options.output_png_path;
    repl.wrm_path = options.output_wrm_path;
    repl.record_transport_path = options.output_record_transport_path;
    repl.ip_address = options.ip_address;
    repl.username = options.username;
    repl.password = options.password;

    repl.start_connection = *options.ip_address;
    bool interactive = !repl.start_connection || options.interactive;

    null_mod mod;
    FrontAPI& front = repl;
    SessionLogApi& session_log = repl;

    HeadlessCommandReader reader(input.fd());

    while (!repl.quit && (!reader.is_eof() || (repl.start_connection && options.persist))) {
        update_times(event_manager);
        front.must_be_stop_capture();

        /*
         * Wait connect command / ready to connect event
         */
        if (!repl.start_connection && perform_automatic_reconnection == PerformAutomaticReconnection::No) {
            if (!interactive) {
                break;
            }

            fd_set rfds;
            io_fd_zero(rfds);
            io_fd_set(reader.fd(), rfds);

            do {
                show_prompt();

                const int num = select(reader.fd() + 1, &rfds, nullptr, nullptr, nullptr);
                // end of file
                if (num < 0) {
                    LOG(LOG_ERR, "RDP CLIENT :: errno = %s", strerror(errno));
                    repl.quit = true;
                    break;
                }

                if (!repl.execute_command(mod, reader.read_command())) {
                    break;
                }
            } while (!reader.is_eof());

            continue;
        }

        repl.start_connection = false;

        auto& gd = repl.prepare_gd();

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
            ini.set_acl<cfg::context::target_host>(repl.ip_address);
            ini.set_acl<cfg::context::target_password>(repl.password);
            ini.set_acl<cfg::globals::target_user>(repl.username);
        }

        auto& event_container = event_manager.get_events();

        std::unique_ptr<RecorderTransport> recorder_transport;
        auto maybe_make_transport_record = [&](Transport& trans) -> Transport& {
            if (!repl.enable_record_transport) {
                return trans;
            }

            recorder_transport = std::make_unique<RecorderTransport>(
                trans, event_manager.get_time_base(), repl.record_transport_path.c_str()
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
                return (!reader.is_eof() || options.persist)
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
            HeadlessRepl::SessionEventGuard repl_session_guard{repl, *mod};

            InputCollector input_collector;
            Keymap null_keymap(KeyLayout::null_layout());

            EventRef time_ev;
            EventRef input_ev;

            if (has_automation_script) {
                auto timer_input = [&](Event& e) {
                    bool has_timer = repl_session_guard.execute_timer(e, input_collector, null_keymap);
                    input_ev.raw_event()->fd = has_timer ? -1 : input.fd();
                };

                auto read_auto_input = [&, line_number = std::size_t()](Event&) mutable {
                    auto line = reader.read_command();
                    ++line_number;
                    LOG(LOG_INFO, "L.%zu: %.*s", line_number, int(line.size()), line.data());
                    if (repl.execute_command(input_collector, line)) {
                        timer_input(*time_ev.raw_event());
                    }
                };

                time_ev = event_container.event_creator()
                    .create_event_timeout("script-timer", nullptr, MonotonicTimePoint::duration(), timer_input);
                time_ev.raw_event()->active_timer = false;

                input_ev = event_container.event_creator()
                    .create_event_fd_without_timeout("script", nullptr, input.fd(), read_auto_input);
            }
            else {
                show_prompt();

                auto read_input = [&](Event&) {
                    if (repl.execute_command(*mod, reader.read_command())) {
                        show_prompt();
                    }
                };

                input_ev = event_container.event_creator()
                    .create_event_fd_without_timeout("stdin", nullptr, input.fd(), read_input);
            }

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

    update_times(event_manager);
    front.must_be_stop_capture();

    return 0;
}
