/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/app_path.hpp"
#include "gdi/screen_info.hpp"
#include "keyboard/keylayout.hpp"

#include <chrono>


struct HeadlessCliOptions
{
    char const* username = "";
    char const* password = "";
    char const* ip_address = "";
    int port = 3389;
    int cert_check = 0;

    bool is_cmd_kbdmap_en = true;
    bool is_vnc = false;
    bool use_fixed_random = false;
    bool enable_wrm_capture = false;
    bool enable_png_capture = false;
    bool print_client_info_section = false;
    bool persist = false;

    ScreenInfo screen_info {800, 600, BitsPerPixel::BitsPP16};
    KeyLayout::KbdId keylayout = KeyLayout::KbdId(0x040C);

    std::chrono::milliseconds connection_establishment_timeout {3'000};
    std::chrono::milliseconds tcp_user_timeout {10'000};
    std::chrono::milliseconds receive_timeout {3'000};
    uint32_t socket_verbose = {};

    uint32_t mod_verbose = {};

    char const* config_filename = app_path(AppPath::CfgIni);
    char const* license_store_path = app_path(AppPath::License);
    char const* output_png_path = "output.png";
    char const* output_wrm_path = "output.wrm";

    char const* load_balance_info = "";

    enum class [[nodiscard]] Result
    {
        Error,
        Exit,
        Ok,
    };

    Result parse(int argc, char const** argv);
};
