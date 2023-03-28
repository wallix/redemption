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


static void show_prompt()
{
    write(1, ">>> ", 4);
}

static bool wait_and_draw_event(SocketTransport& trans, EventManager& event_manager, int ignored_fd = -1)
{
    const auto now = MonotonicTimePoint::clock::now();
    event_manager.get_writable_time_base().monotonic_time = now;

    int max = 0;
    fd_set rfds;
    io_fd_zero(rfds);

    event_manager.for_each_fd([&](int fd){
        if (fd != ignored_fd) {
            io_fd_set(fd, rfds);
            max = std::max(max, fd);
        }
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

struct BatchInput : RdpInput
{
    enum class InputType
    {
        Scancode,
        Unicode,
        KeyLock,
        Mouse,
    };

    struct InputData
    {
        InputType type;
        uint16_t flags_or_locks;
        uint16_t sc_or_uc_or_x;
        uint16_t y;
    };

    // TODO
    Keymap const* keymap = nullptr;

    std::vector<InputData> inputs;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override
    {
        (void)keymap;
        (void)event_time;
        this->keymap = &keymap;
        inputs.push_back(InputData{InputType::Scancode, underlying_cast(flags), underlying_cast(scancode), 0});
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        inputs.push_back(InputData{InputType::Unicode, underlying_cast(flag), unicode, 0});
    }

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        inputs.push_back(InputData{InputType::Mouse, device_flags, x, y});
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        inputs.push_back(InputData{InputType::KeyLock, underlying_cast(locks), 0, 0});
    }

    void rdp_input_invalidate(Rect r) override
    {
        (void)r;
    }

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}
};

struct Repl
{
    bool quit = false;
    bool start_connection = false;
    bool disconnection = false;
    bool has_delay_cmd = false;

    int fd = 0;

    MonotonicTimePoint::duration cmd_delay;
    MonotonicTimePoint::duration key_delay;
    MonotonicTimePoint::duration mouse_delay;
    MonotonicTimePoint::duration sleep_delay;
    std::string delayed_cmd;
    HeadlessRepl repl;

    bool is_eof() const
    {
        return repl.is_eof();
    }

    bool wait_connect(HeadlessFront& front)
    {
        null_mod mod;

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

    bool execute_command(HeadlessFront& front, RdpInput& mod)
    {
        return execute_command(front, mod, repl.read_command(fd));
    }

    bool execute_delayed_command(HeadlessFront& front, RdpInput& mod)
    {
        return execute_command(front, mod, delayed_cmd);
    }

    bool execute_command(HeadlessFront& front, RdpInput& mod, chars_view cmd_line)
    {
        HeadlessCommand& cmd_ctx = front.command();

        switch (cmd_ctx.execute_command(cmd_line, mod)) {
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

            case HeadlessCommand::Result::KeyDelay:
                key_delay = cmd_ctx.delay;
                break;

            case HeadlessCommand::Result::MouseDelay:
                mouse_delay = cmd_ctx.delay;
                break;

            case HeadlessCommand::Result::Sleep:
                sleep_delay = cmd_ctx.delay;
                break;

            case HeadlessCommand::Result::RepetitionCommand:
                cmd_delay = cmd_ctx.delay;
                delayed_cmd = cmd_ctx.output_message.as<std::string_view>();
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

    repl.start_connection = *options.ip_address;
    bool interactive = !repl.start_connection || options.interactive;

    char const* automation_script = options.headless_script_path;
    bool has_automation_script = automation_script;

    auto input = has_automation_script ? unique_fd(automation_script) : unique_fd(0);
    repl.fd = input.fd();

    if (has_automation_script && !input) {
        printf("Automation open error: %s: %s", automation_script, strerror(errno));
        return 1;
    }

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
                if (!repl.delayed_cmd.empty() && repl.cmd_delay.count() >= 0) {
                    auto repeat = (cmd_ctx.repeat_delay < 0)
                        ? ~uint64_t(0) // infinite loop
                        : static_cast<uint64_t>(cmd_ctx.repeat_delay);

                    auto fn = [repeat, &repl, &front, &mod](Event& ev) mutable {
                        if (--repeat == 0) {
                            ev.garbage = true;
                            return;
                        }

                        ev.add_timeout_delay(repl.cmd_delay);
                        repl.execute_delayed_command(front, *mod);
                        if (repl.has_delay_cmd) {
                            repl.has_delay_cmd = false;
                            ev.garbage = true;
                        }
                    };

                    delayed_cmd_ref = event_container.event_creator()
                        .create_event_timeout("delayed_cmd", nullptr, repl.cmd_delay, fn);
                }
                else {
                    delayed_cmd_ref.garbage();
                }
            };

            BatchInput batch;
            int ignored_fd = -1;

            EventRef time_ev;
            EventRef input_ev;
            std::size_t i_input = 0;
            std::size_t i_line = 0;

            if (automation_script) {
                auto timer_input = [&](Event& e) {
                    if (repl.sleep_delay.count()) {
                        ignored_fd = input.fd();
                        e.add_timeout_delay(repl.sleep_delay);
                        repl.sleep_delay = repl.sleep_delay.zero();
                        return;
                    }

                    if (!batch.inputs.empty()) {
                        ignored_fd = input.fd();
                        auto& input = batch.inputs[i_input];

                        // send one event to mod
                        switch (input.type) {
                            case BatchInput::InputType::Scancode:
                                mod->rdp_input_scancode(
                                    checked_int(input.flags_or_locks),
                                    checked_int(input.sc_or_uc_or_x),
                                    0,
                                    *batch.keymap
                                );
                                break;

                            case BatchInput::InputType::Unicode:
                                mod->rdp_input_unicode(
                                    checked_int(input.flags_or_locks),
                                    checked_int(input.sc_or_uc_or_x)
                                );
                                break;

                            case BatchInput::InputType::KeyLock:
                                mod->rdp_input_synchronize(checked_int(input.flags_or_locks));
                                break;

                            case BatchInput::InputType::Mouse:
                                mod->rdp_input_mouse(
                                    checked_int(input.flags_or_locks),
                                    input.sc_or_uc_or_x,
                                    input.y
                                );
                                break;
                        }

                        ++i_input;
                        if (i_input < batch.inputs.size()) {
                            e.add_timeout_delay(input.type == BatchInput::InputType::Mouse
                                ? repl.mouse_delay
                                : repl.key_delay
                            );
                            return;
                        }

                        i_input = 0;
                        batch.inputs.clear();
                    }

                    ignored_fd = -1;
                };

                auto read_auto_input = [&](Event&) {
                    auto line = repl.repl.read_command(repl.fd);
                    ++i_line;
                    printf("%zu: %.*s\n", i_line, int(line.size()), line.data());
                    if (repl.execute_command(front, batch, line)) {
                        timer_input(*time_ev.get_optional_event());
                    }
                };

                time_ev = event_container.event_creator()
                    .create_event_timeout("script-timer", nullptr, MonotonicTimePoint::duration(), timer_input);

                input_ev = event_container.event_creator()
                    .create_event_fd_without_timeout("script", nullptr, input.fd(), read_auto_input);
            }
            else {
                show_prompt();

                mk_delayed_cmd_when_available();

                auto read_input = [&](Event&) {
                    if (repl.execute_command(front, *mod)) {
                        mk_delayed_cmd_when_available();
                        show_prompt();
                    }
                };

                input_ev = event_container.event_creator()
                    .create_event_fd_without_timeout("stdin", nullptr, input.fd(), read_input);
            }

            while (is_actif() && wait_and_draw_event(*sck_trans, event_manager, ignored_fd)) {
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
