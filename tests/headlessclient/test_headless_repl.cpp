/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/mod/accumulate_input_mod.hpp"

#include "utils/fileutils.hpp"
#include "headlessclient/headless_repl.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/headless_repl/"


RED_AUTO_TEST_CASE_WD(TestHeadlessRepl, wd)
{
    HeadlessRepl repl("HOME"_av);
    AccumulateInputMod mod;

    std::vector<std::string> paths;
    repl.path_notifier = [&](HeadlessRepl::PathType type, zstring_view path){
        chars_view t = "UNKNOWN"_av;
        switch (type) {
            case HeadlessRepl::PathType::Png: t = "PNG"_av; break;
            case HeadlessRepl::PathType::Wrm: t = "WRM"_av; break;
            case HeadlessRepl::PathType::RecorderTransport: t = "TRANS"_av; break;
        }
        paths.emplace_back(str_concat(t, "  "_av, path));
    };

    RED_CHECK(repl.start_connection == false);

    auto wrm_path = wd.add_file("output.wrm");

    repl.execute_command(mod, "connect ::1"_av);
    repl.execute_command(mod, str_concat("wrm "_av, wrm_path));

    RED_CHECK(repl.start_connection == true);
    RED_CHECK(repl.ip_address == "::1"_av);
    RED_CHECK(mod.events() == ""_av);

    repl.enable_png = true;
    repl.enable_wrm = true;

    uint16_t width = 100;
    uint16_t height = 100;
    repl.client_info.screen_info = {width, height, BitsPerPixel::BitsPP24};

    auto& gd = repl.prepare_gd();

    NullSessionLog log;
    RED_CHECK(repl.can_be_start_capture(log));
    RED_CHECK_EQ_RANGES(paths, std::initializer_list<std::string>{str_concat("WRM  "_av, wrm_path)});
    paths.clear();

    Rect screen_rect(0, 0, width, height);

    gd.draw(RDPOpaqueRect(screen_rect.shrink(20), encode_color24()(YELLOW)), screen_rect, gdi::ColorCtx::depth24());

    repl.must_flush_capture();

    RED_CHECK_IMG(repl.get_image_view(), IMG_TEST_PATH "shrink_20.png");

    auto png_path = wd.add_file("output.png");

    repl.execute_command(mod, str_concat("png "_av, png_path));
    RED_CHECK_EQ_RANGES(paths, std::initializer_list<std::string>{str_concat("PNG  "_av, png_path)});
    paths.clear();

    RED_CHECK_IMG(png_path, IMG_TEST_PATH "shrink_20.png");

    repl.must_be_stop_capture();

    RED_CHECK(filesize(wrm_path.c_str()) == 176);
}
