/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "gdi/screen_info.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "keyboard/keylayout.hpp"

#include <string>

namespace qtclient
{

enum class ProtocolMod : uint8_t
{
    RDP,
    VNC,
};

struct Profile
{
    std::string profile_name;

    std::string user_name;
    std::string user_password;
    std::string target_address;
    int target_port = -1;
    ProtocolMod protocol = ProtocolMod::RDP;

    RDPVerbose rdp_verbose = RDPVerbose();

    bool is_spanning = false;
    bool enable_recording = false;
    bool enable_clipboard = true;

    bool enable_nla = false;
    bool enable_tls = true;
    bool enable_sound = false;

    bool enable_remote_app = true;
    std::string remote_app_cmd;
    std::string remote_app_working_directory;

    bool enable_drive = true;
    bool use_home_drive = false;
    std::string drive_path;

    uint32_t tls_min_level = 0;
    uint32_t tls_max_level = 0;
    std::string cipher_string;

    uint32_t rdp5_performance_flags = 0x00000040;

    ScreenInfo screen_info = {800, 600, BitsPerPixel::BitsPP16};
    KeyLayout::KbdId key_layout = KeyLayout::KbdId();
};

} // namespace qtclient
