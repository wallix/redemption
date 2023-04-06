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
#include "headlessclient/headless_repl.hpp"
#include "headlessclient/input_collector.hpp"
#include "headlessclient/compute_headless_wrm_path.hpp"
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

struct Repl final : FrontAPI, SessionLogApi
{
    bool quit = false;
    bool start_connection = false;
    bool disconnection = false;
    bool has_delay_cmd = false;
    bool first_png = true;
    bool gd_is_ready = false;

    int fd = 0;

    MonotonicTimePoint::duration cmd_delay;
    MonotonicTimePoint::duration key_delay;
    MonotonicTimePoint::duration mouse_delay;
    MonotonicTimePoint::duration sleep_delay;
    std::string delayed_cmd;
    HeadlessRepl repl;

    bool enable_wrm = false;
    bool enable_png = false;
    bool enable_record_transport = false;
    std::string png_path;
    std::string wrm_path;
    std::string record_transport_path;
    std::string ip_address;
    std::string username;
    std::string password;

    std::string session_id;
    std::string prefix_path;
    std::string screen_repetition_output_directory;

    ClientInfo client_info {};
    Inifile ini;
    EventManager event_manager;
    HeadlessCommand cmd_ctx;

    bool is_eof() const
    {
        return repl.is_eof();
    }

    bool wait_connect()
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

            if (!execute_command(mod)) {
                return false;
            }
        } while (!repl.is_eof());

        return true;
    }

    bool execute_command(RdpInput& mod)
    {
        return execute_command(mod, repl.read_command(fd));
    }

    bool execute_delayed_command(RdpInput& mod)
    {
        return execute_command(mod, delayed_cmd);
    }

    bool execute_command(RdpInput& mod, chars_view cmd_line)
    {
        switch (cmd_ctx.execute_command(cmd_line, mod)) {
            case HeadlessCommand::Result::KbdChange:
            case HeadlessCommand::Result::Ok:
                break;

            case HeadlessCommand::Result::Sleep:
                sleep_delay = cmd_ctx.delay;
                break;

            case HeadlessCommand::Result::PrefixPath:
                prefix_path = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::Username:
                username = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::Password:
                password = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::WrmPath:
                wrm_path = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::RecordTransportPath:
                record_transport_path = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::ScreenRepetitionDirectory:
                screen_repetition_output_directory = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::OutputResult:
                fprintf(stderr, "%.*s\n", int(cmd_ctx.output_message.size()), cmd_ctx.output_message.data());
                break;

            case HeadlessCommand::Result::Fail:
                fprintf(stderr, "%s at index %u: %.*s\n",
                    HeadlessCommand::error_to_cstring(cmd_ctx.error_type), cmd_ctx.index_param_error,
                    int(cmd_ctx.output_message.size()), cmd_ctx.output_message.data()
                );
                break;

            case HeadlessCommand::Result::Connect:
                if (!cmd_ctx.output_message.empty())
                    ip_address = cmd_ctx.output_message.as<std::string_view>();
                [[fallthrough]];
            case HeadlessCommand::Result::Reconnect:
                disconnection = true;
                start_connection = true;
                return false;

            case HeadlessCommand::Result::Disconnect:
                disconnection = true;
                return false;

            case HeadlessCommand::Result::ConfigStr:
                load_headless_config_from_string(
                    ini, client_info,
                    static_string<1024>(truncated_bounded_array_view(cmd_ctx.output_message)).data()
                );
                break;

            case HeadlessCommand::Result::ConfigFile:
                load_headless_config_from_file(
                    ini, client_info,
                    static_string<1024>(truncated_bounded_array_view(cmd_ctx.output_message)).c_str()
                );
                break;

            case HeadlessCommand::Result::Screen: {
                // TODO use prefix_path / session_id
                char const* err = nullptr;
                if (drawable) {
                    try {
                        err = drawable->dump_png(png_path, cmd_ctx.mouse_x, cmd_ctx.mouse_y);
                        if (!err) {
                            break;
                        }
                    }
                    catch (Error const& e) {
                        LOG(LOG_ERR, "%s: %s", png_path, e.errmsg());
                        if (e.errnum) {
                            err = strerror(e.errnum);
                        }
                    }
                    catch (...) {
                        err = strerror(errno);
                    }
                }
                else if (!gd_is_ready) {
                    err = "Graphics is not ready";
                }
                else if (first_png) {
                    err = "Png capture is disabled";
                    first_png = false;
                }
                else {
                    break;
                }

                LOG(LOG_ERR, "%s: %s", png_path, err);
                break;
            }

            case HeadlessCommand::Result::EnableScreen:
                enable_png = cmd_ctx.output_bool;
                first_png = true;
                break;

            case HeadlessCommand::Result::ScreenRepetition:
                // TODO delay, suffix
                screen_repetition_output_directory = cmd_ctx.output_message.as<std::string_view>();
                break;

            case HeadlessCommand::Result::KeyDelay:
                key_delay = cmd_ctx.delay;
                break;

            case HeadlessCommand::Result::MouseDelay:
                mouse_delay = cmd_ctx.delay;
                break;

            case HeadlessCommand::Result::RepetitionCommand:
                // TODO use prefix_path / session_id
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

    gdi::GraphicApi& prepare_gd()
    {
        first_png = true;

        if (enable_wrm || enable_png) {
            if (!drawable) {
                drawable = std::make_unique<HeadlessGraphics>(cmd_ctx.screen_width, cmd_ctx.screen_height);
                gds.add_graphic(*drawable);
            }
            return gds;
        }

        return gdi::null_gd();
    }


    /*
     * SessionLogApi
     */

    void log6(LogId id, KVLogList kv_list) override
    {
        std::size_t len = safe_size_for_log_format_append_info(id, kv_list);
        char* p = buffer.grow_without_copy(len).as_charp();
        char* end = log_format_append_info(p, id, kv_list);
        fprintf(stderr, "[headless] %.*s\n", int(end - p), p);
    }

    void report(const char * reason, const char * message) override
    {
        fprintf(stderr, "Report: %s: %s\n", reason, message);
    }

    void set_control_owner_ctx(chars_view name) override
    {
        (void)name;
    }


    /*
     * FrontApi
     */

    bool can_be_start_capture(SessionLogApi& session_log) override
    {
        (void)session_log;

        gd_is_ready = true;

        if (enable_wrm && drawable) {
            auto& time_base = event_manager.get_writable_time_base();
            time_base = TimeBase::now();

            // TODO use prefix_path / session_id

            auto filename = compute_headless_wrm_path(wrm_path, session_id, time_base.real_time);
            auto fd = unique_fd(filename, O_WRONLY | O_CREAT, 0664);
            if (!fd) {
                int errnum = errno;
                LOG(LOG_ERR, "Open wrm file error (%s): %s", filename, strerror(errnum));
                throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errnum);
            }

            wrm_gd.reset(new HeadlessWrmCapture(
                std::move(fd), drawable->drawable(),
                time_base.monotonic_time, time_base.real_time,
                client_info.screen_info.bpp, client_info.remote_program,
                ini.get<cfg::video::wrm_compression_algorithm>(),
                safe_cast<RDPSerializerVerbose>(ini.get<cfg::debug::capture>())
            ));

            gds.add_graphic(wrm_gd->gd());
        }

        return bool(drawable);
    }

    void must_flush_capture() override
    {
        if (wrm_gd) {
            wrm_gd->update_timestamp(event_manager.get_time_base().monotonic_time);
        }
    }

    bool must_be_stop_capture() override
    {
        gd_is_ready = false;

        gds.clear();

        if (wrm_gd) {
            wrm_gd->update_timestamp(event_manager.get_time_base().monotonic_time);
            wrm_gd.reset();
        }

        if (drawable) {
            drawable.reset();
        }

        return true;
    }

    bool is_capture_in_progress() const override
    {
        return false;
    }

    ResizeResult server_resize(ScreenInfo screen_server) override
    {
        if (drawable) {
            drawable->resize(screen_server.width, screen_server.height);
        }
        return ResizeResult::instant_done;
    }

    const CHANNELS::ChannelDefArray& get_channel_list() const override
    {
        return cl;
    }

    void send_to_channel(
        CHANNELS::ChannelDef const& channel_def, bytes_view chunk_data,
        std::size_t total_data_len, uint32_t flags) override
    {
        (void)channel_def;
        (void)chunk_data;
        (void)total_data_len;
        (void)flags;
    }

    void update_pointer_position(uint16_t x, uint16_t y) override
    {
        cmd_ctx.mouse_x = x;
        cmd_ctx.mouse_y = y;
    }

    void session_update(MonotonicTimePoint now, LogId id, KVLogList kv_list) override
    {
        (void)now;
        (void)id;
        (void)kv_list;
    }

    void possible_active_window_change() override
    {}

private:
    CHANNELS::ChannelDefArray cl;
    UninitDynamicBuffer buffer;
    // capture variable members
    // -----------------
    gdi::GraphicDispatcher gds;
    std::unique_ptr<HeadlessGraphics> drawable;
    std::unique_ptr<HeadlessWrmCapture> wrm_gd;
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

    Repl repl;
    auto& client_info = repl.client_info;
    auto& ini = repl.ini;
    auto& event_manager = repl.event_manager;

    char const* automation_script = options.headless_script_path;
    bool has_automation_script = automation_script;

    auto input = has_automation_script ? unique_fd(automation_script) : unique_fd(0);
    repl.fd = input.fd();

    if (has_automation_script && !input) {
        printf("Automation open error: %s: %s", automation_script, strerror(errno));
        return 1;
    }

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

    FrontAPI& front = repl;
    SessionLogApi& session_log = repl;

    while (!repl.quit && (!repl.is_eof() || (repl.start_connection && options.persist))) {
        front.must_be_stop_capture();

        /*
         * Wait connect command / ready to connect event
         */
        if (!repl.start_connection && perform_automatic_reconnection == PerformAutomaticReconnection::No) {
            if (!interactive) {
                break;
            }

            repl.wait_connect();
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

                    auto fn = [repeat, &repl, &mod](Event& ev) mutable {
                        if (--repeat == 0) {
                            ev.garbage = true;
                            return;
                        }

                        ev.add_timeout_delay(repl.cmd_delay);
                        repl.execute_delayed_command(*mod);
                        // recursive creation, remove timer
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

            InputCollector input_collector;
            Keymap null_keymap(KeyLayout::null_layout());

            EventRef time_ev;
            EventRef input_ev;

            // TODO ipng

            if (has_automation_script) {
                auto timer_input = [&](Event& e) {
                    if (repl.sleep_delay.count()) {
                        input_ev.raw_event()->fd = -1;
                        e.add_timeout_delay(repl.sleep_delay);
                        repl.sleep_delay = repl.sleep_delay.zero();
                        return;
                    }

                    switch (input_collector.send_next_input(*mod, null_keymap))
                    {
                        case InputCollector::ConsumedInput::None:
                            input_ev.raw_event()->fd = input.fd();
                            break;
                        case InputCollector::ConsumedInput::KeyEvent:
                            input_ev.raw_event()->fd = -1;
                            e.add_timeout_delay(repl.key_delay);
                            break;
                        case InputCollector::ConsumedInput::MouseEvent:
                            input_ev.raw_event()->fd = -1;
                            e.add_timeout_delay(repl.mouse_delay);
                            break;
                    }
                };

                auto read_auto_input = [&, line_number = std::size_t()](Event&) mutable {
                    auto line = repl.repl.read_command(repl.fd);
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

                mk_delayed_cmd_when_available();

                auto read_input = [&](Event&) {
                    if (repl.execute_command(*mod)) {
                        mk_delayed_cmd_when_available();
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

    front.must_be_stop_capture();

    return 0;
}
