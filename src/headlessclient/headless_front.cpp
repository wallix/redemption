/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/client_info.hpp"
#include "headlessclient/headless_front.hpp"
#include "headlessclient/compute_headless_wrm_path.hpp"
#include "headlessclient/headless_wrm_capture.hpp"
#include "headlessclient/headless_configuration.hpp"
#include "configs/config.hpp"
#include "utils/timebase.hpp"
#include "mod/mod_api.hpp"
#include "utils/static_string.hpp"


gdi::GraphicApi& HeadlessFront::gd()
{
    if (enable_wrm) {
        return gds;
    }
    return headless_gd;
}

void HeadlessFront::dump_png(zstring_view filename, uint16_t mouse_x, uint16_t mouse_y)
{
    headless_gd.dump_png(filename, mouse_x, mouse_y);
}

bool HeadlessFront::can_be_start_capture(SessionLogApi& session_log)
{
    (void)session_log;

    if (enable_wrm) {
        time_base.monotonic_time = MonotonicTimePoint::clock::now();
        time_base.real_time = RealTimePoint::clock::now();

        auto filename = compute_headless_wrm_path(cmd_ctx.wrm_path, cmd_ctx.session_id, time_base.real_time);
        auto fd = unique_fd(filename, O_WRONLY | O_CREAT, 0664);
        if (!fd) {
            int errnum = errno;
            LOG(LOG_ERR, "Open wrm file error (%s): %s", filename, strerror(errnum));
            throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errnum);
        }

        wrm_gd.reset(new HeadlessWrmCapture(
            std::move(fd), headless_gd.drawable(),
            time_base.monotonic_time, time_base.real_time,
            client_info.screen_info.bpp, client_info.remote_program,
            ini.get<cfg::video::wrm_compression_algorithm>(),
            safe_cast<RDPSerializerVerbose>(ini.get<cfg::debug::capture>())
        ));

        gds.add_graphic(headless_gd);
        gds.add_graphic(wrm_gd->gd());

        return true;
    }

    return false;
}

void HeadlessFront::must_flush_capture()
{
    if (wrm_gd) {
        wrm_gd->unpdate_timestamp(time_base.monotonic_time);
    }
}

bool HeadlessFront::must_be_stop_capture()
{
    gds.clear();
    if (wrm_gd) {
        wrm_gd->unpdate_timestamp(time_base.monotonic_time);
        wrm_gd.reset();
    }
    return true;
}

HeadlessFront::ResizeResult HeadlessFront::server_resize(ScreenInfo screen_server)
{
    headless_gd.resize(screen_server.width, screen_server.height);
    cmd_ctx.screen_width = screen_server.width;
    cmd_ctx.screen_height = screen_server.height;
    return ResizeResult::instant_done;
}

void HeadlessFront::print_output_resut()
{
    fprintf(stderr, "%.*s\n", int(cmd_ctx.output_message.size()), cmd_ctx.output_message.data());
}

void HeadlessFront::print_fail_result()
{
    fprintf(stderr, "%s at index %u: %.*s\n",
        HeadlessCommand::error_to_cstring(cmd_ctx.error_type), cmd_ctx.index_param_error,
        int(cmd_ctx.output_message.size()), cmd_ctx.output_message.data()
    );
}

void HeadlessFront::read_config_str()
{
    load_headless_config_from_string(
        ini, client_info,
        static_string<1024>(truncated_bounded_array_view(cmd_ctx.output_message)).data()
    );
}

void HeadlessFront::read_config_file()
{
    load_headless_config_from_file(
        ini, client_info,
        static_string<1024>(truncated_bounded_array_view(cmd_ctx.output_message)).c_str()
    );
}

void HeadlessFront::disconnect_command(mod_api& mod)
{
    mod.disconnect();
}
