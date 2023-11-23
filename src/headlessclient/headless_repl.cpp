/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_repl.hpp"
#include "headlessclient/headless_configuration.hpp"
#include "utils/static_string.hpp"
#include "utils/key_qvalue_pairs.hpp"


HeadlessRepl::SessionEventGuard::SessionEventGuard(HeadlessRepl& repl, RdpInput& mod)
: repl(repl)
{
    repl.make_ipng_capture_event();
    repl.make_repetition_command_event(mod);
    repl.session_event = this;
}

HeadlessRepl::SessionEventGuard::~SessionEventGuard()
{
    repl.session_event = nullptr;
}

bool HeadlessRepl::SessionEventGuard::execute_timer(Event& e, InputCollector& input_collector, Keymap& keymap)
{
    if (repl.sleep_delay.count()) {
        e.add_timeout_delay(repl.sleep_delay);
        repl.sleep_delay = repl.sleep_delay.zero();
        return true;
    }

    repl.input_mod = &input_collector;
    switch (input_collector.send_next_input(repl, keymap))
    {
        case InputCollector::ConsumedInput::None:
            return false;
        case InputCollector::ConsumedInput::KeyEvent:
            e.add_timeout_delay(repl.key_delay);
            return true;
        case InputCollector::ConsumedInput::MouseEvent:
            e.add_timeout_delay(repl.mouse_delay);
            return true;
    }

    REDEMPTION_UNREACHABLE();
}


HeadlessRepl::HeadlessRepl(chars_view home)
{
    str_assign(home_variable, home, '/');
    screen_repetition_prefix_path.compile("%i%s%E"_av);
    client_info.screen_info = {800, 600, BitsPerPixel::BitsPP24};
}

bool HeadlessRepl::execute_command(RdpInput& mod, chars_view cmd_line)
{
    input_mod = &mod;

    switch (cmd_ctx.execute_command(cmd_line, *this)) {
        case HeadlessCommand::Result::KbdChange:
        case HeadlessCommand::Result::Ok:
            break;

        case HeadlessCommand::Result::Sleep:
            sleep_delay = cmd_ctx.delay;
            break;

        case HeadlessCommand::Result::PrefixPath:
            prefix_path.compile(cmd_ctx.output_message);
            is_regular_png_path = false;
            break;

        case HeadlessCommand::Result::ScreenRepetitionPrefixPath:
            screen_repetition_prefix_path.compile(cmd_ctx.output_message);
            break;

        case HeadlessCommand::Result::Username:
            username = cmd_ctx.output_message.as<std::string_view>();
            break;

        case HeadlessCommand::Result::Password:
            password = cmd_ctx.output_message.as<std::string_view>();
            break;

        case HeadlessCommand::Result::WrmPath:
            if (!cmd_ctx.output_message.empty()) {
                wrm_path = cmd_ctx.output_message.as<std::string_view>();
            }
            enable_wrm = cmd_ctx.output_bool;
            break;

        case HeadlessCommand::Result::RecordTransportPath:
            record_transport_path = cmd_ctx.output_message.as<std::string_view>();
            enable_record_transport = cmd_ctx.output_bool;
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
            if (!cmd_ctx.output_message.empty()) {
                png_path = cmd_ctx.output_message.as<std::string_view>();
            }

            screenshot([&]{
                zstring_view path = png_path;
                if (!is_regular_png_path) {
                    auto ctx = HeadlessPath::Context{
                        .counter = ++counters.png,
                        .global_counter = ++counters.total,
                        .real_time = event_manager.get_time_base().real_time,
                        .extension = ".png"_av,
                        .filename = path,
                        .suffix = ""_av,
                        .home = home_variable,
                    };
                    auto computed_path = prefix_path.compute_path(ctx);
                    is_regular_png_path = computed_path.is_regular;
                    path = computed_path.path;
                    // prevent self-assignment (str_assign() use .clear() before assignment)
                    if (is_regular_png_path && png_path.data() != path.data()) {
                        str_assign(png_path, path);
                    }
                }
                return path;
            });
            break;
        }

        case HeadlessCommand::Result::EnableScreen:
            enable_png = cmd_ctx.output_bool;
            first_png = true;
            break;

        case HeadlessCommand::Result::ScreenRepetition:
            ipng_delay = cmd_ctx.delay;
            ipng_suffix = cmd_ctx.output_message.as<std::string_view>();
            if (!make_ipng_capture_event()) {
                if (session_event) {
                    session_event->ipng_event.garbage();
                }

                if (!drawable) {
                    // just for logging message
                    screenshot([]{ return ""_zv; });
                }
            }
            break;

        case HeadlessCommand::Result::KeyDelay:
            key_delay = cmd_ctx.delay;
            break;

        case HeadlessCommand::Result::MouseDelay:
            mouse_delay = cmd_ctx.delay;
            break;

        case HeadlessCommand::Result::RepetitionCommand:
            if (cmd_ctx.delay.count()) {
                cmd_delay = cmd_ctx.delay;
                delayed_cmd = cmd_ctx.output_message.as<std::string_view>();
                has_delay_cmd = true;
                make_repetition_command_event(mod);
            }
            else {
                has_delay_cmd = false;
                if (session_event) {
                    session_event->repetition_cmd_event.garbage();
                }
            }
            break;

        case HeadlessCommand::Result::Quit:
            disconnection = true;
            quit = true;
            return false;
    }

    return true;
}

gdi::GraphicApi& HeadlessRepl::prepare_gd()
{
    first_png = true;

    if (enable_wrm || enable_png) {
        if (!drawable) {
            drawable = std::make_unique<HeadlessGraphics>(
                client_info.screen_info.width,
                client_info.screen_info.height
            );
            gds.add_graphic(*drawable);
        }
        return gds;
    }

    return gdi::null_gd();
}

ImageView HeadlessRepl::get_image_view()
{
    if (drawable) {
        return drawable->drawable();
    }

    return ImageView::create_null_view();
}


/*
 * SessionLogApi
 */

void HeadlessRepl::log6(LogId id, KVLogList kv_list)
{
    std::size_t len = safe_size_for_log_format_append_info(id, kv_list);
    char* p = buffer.grow_without_copy(len).as_charp();
    char* end = log_format_append_info(p, id, kv_list);
    fprintf(stderr, "[headless] %.*s\n", int(end - p), p);
}

void HeadlessRepl::report(const char * reason, const char * message)
{
    fprintf(stderr, "Report: %s: %s\n", reason, message);
}


/*
 * FrontApi
 */

bool HeadlessRepl::can_be_start_capture(SessionLogApi& session_log)
{
    (void)session_log;

    gd_is_ready = true;

    if (enable_wrm && drawable) {
        auto& time_base = event_manager.get_writable_time_base();

        auto ctx = HeadlessPath::Context{
            .counter = ++counters.wrm,
            .global_counter = ++counters.total,
            .real_time = time_base.real_time,
            .extension = ".wrm"_av,
            .filename = wrm_path,
            .suffix = ""_av,
            .home = home_variable,
        };
        auto path = prefix_path.compute_path(ctx).path;

        path_notifier(PathType::Wrm, path);
        auto fd = unique_fd(path, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        if (!fd) {
            int errnum = errno;
            LOG(LOG_ERR, "Open wrm file error (%s): %s", path, strerror(errnum));
            throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errnum);
        }

        wrm_gd.reset(new HeadlessWrmCapture(
            std::move(fd), drawable->drawable(), drawable->get_pointer_cache(),
            time_base.monotonic_time, time_base.real_time,
            client_info.screen_info.bpp, client_info.remote_program,
            ini.get<cfg::capture::wrm_compression_algorithm>(),
            safe_cast<RDPSerializerVerbose>(ini.get<cfg::debug::capture>())
        ));

        gds.add_graphic(wrm_gd->gd());
    }

    return bool(drawable);
}

void HeadlessRepl::must_flush_capture()
{
    if (wrm_gd) {
        auto& time_base = event_manager.get_time_base();
        wrm_gd->update_timestamp_and_mouse_position(time_base.monotonic_time, cmd_ctx.mouse_x, cmd_ctx.mouse_y);
    }
}

bool HeadlessRepl::must_be_stop_capture()
{
    gd_is_ready = false;

    gds.clear();

    if (wrm_gd) {
        must_flush_capture();
        wrm_gd.reset();
    }

    if (drawable) {
        drawable.reset();
    }

    return true;
}

FrontAPI::ResizeResult HeadlessRepl::server_resize(ScreenInfo screen_server)
{
    if (drawable) {
        drawable->resize(screen_server.width, screen_server.height);

        if (wrm_gd) {
            wrm_gd->resized();
        }
    }


    return ResizeResult::instant_done;
}

template<class MakePath>
void HeadlessRepl::screenshot(MakePath make_path)
{
    char const* err = nullptr;
    if (drawable) {
        zstring_view path = make_path();

        try {
            path_notifier(PathType::Png, path);
            err = drawable->dump_png(path, cmd_ctx.mouse_x, cmd_ctx.mouse_y);
            if (!err) {
                return;
            }
        }
        catch (Error const& e) {
            LOG(LOG_ERR, "%s: %s", path, e.errmsg());
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
        return;
    }

    LOG(LOG_ERR, "Screenshot error: %s: %s", png_path, err);
}

bool HeadlessRepl::make_ipng_capture_event()
{
    if (!session_event || !drawable || !ipng_delay.count()) {
        return false;
    }

    auto event_fn = [this](Event& ev) {
        screenshot([this]{
            auto ctx = HeadlessPath::Context{
                .counter = ++counters.ipng,
                .global_counter = ++counters.total,
                .real_time = event_manager.get_time_base().real_time,
                .extension = ".png"_av,
                .filename = ""_zv,
                .suffix = ipng_suffix,
                .home = home_variable,
            };
            auto computed_path = screen_repetition_prefix_path.compute_path(ctx);
            ctx.filename = computed_path.path;
            return prefix_path.compute_path(ctx).path;
        });
        ev.add_timeout_delay(ipng_delay);
    };

    session_event->ipng_event.set_timeout_or_create_event(
        ipng_delay, event_manager.get_events(), "ipng", this, event_fn
    );

    return true;
}

void HeadlessRepl::make_repetition_command_event(RdpInput& mod)
{
    if (!session_event || !has_delay_cmd) {
        return;
    }

    has_delay_cmd = false;

    auto repeat = (cmd_ctx.repeat_delay < 0)
        ? ~uint64_t(0) // infinite loop
        : static_cast<uint64_t>(cmd_ctx.repeat_delay);

    auto event_fn = [repeat, &mod, this](Event& ev) mutable {
        if (--repeat == 0) {
            ev.garbage = true;
            return;
        }

        ev.add_timeout_delay(cmd_delay);
        execute_delayed_command(mod);

        // recursive creation, remove event
        if (has_delay_cmd) {
            has_delay_cmd = false;
            ev.garbage = true;
        }
    };

    session_event->repetition_cmd_event.set_timeout_or_create_event(
        ipng_delay, event_manager.get_events(), "repetition_cmd", this, event_fn
    );
}
