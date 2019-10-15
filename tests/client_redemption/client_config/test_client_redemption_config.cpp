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
   Author(s): Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/file.hpp"

#include "client_redemption/client_config/client_redemption_config.hpp"

#include "client_redemption/client_channels/fake_client_mod.hpp"

#include "utils/fileutils.hpp"
#include "utils/sugar/algostring.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include <string_view>

using namespace std::string_view_literals;


RED_TEST_DELEGATE_PRINT_ENUM(RDPVerbose);

inline void write_file(std::string const& filename, array_view_const_char data)
{
    unique_fd fd(filename, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    RED_CHECK_EQ(
        ::write(fd.fd(), data.data(), data.size()),
        static_cast<ssize_t>(data.size()));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigDefault)
{

    FakeClient client;
//     char const ** argv = nullptr;
//     int argc = 0;

    ClientRedemptionConfig config(RDPVerbose::none, "");
    ClientConfig::setDefaultConfig(config);

    // GENERAL
    RED_CHECK_EQUAL(config.verbose, RDPVerbose::none);
    RED_CHECK(config.quick_connection_test);
    RED_CHECK(not config.persist);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK(not config.is_recording);
    RED_CHECK(not config.is_spanning);
    RED_CHECK_EQUAL(config.rdp_width, 800);
    RED_CHECK_EQUAL(config.rdp_height, 600);
    RED_CHECK(not config.is_full_capturing);
    RED_CHECK(not config.is_full_replaying);
    RED_CHECK(config.full_capture_file_name == ""sv);
    RED_CHECK(not config.is_replaying);
    RED_CHECK(not config.is_loading_replay_mod);
    RED_CHECK(not config.connected);
    RED_CHECK(config._movie_name == ""sv);
    RED_CHECK(config._movie_dir == ""sv);
    RED_CHECK(config._movie_full_path == ""sv);
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::PORT_GOT);
    RED_CHECK(config.user_name == ""sv);
    RED_CHECK(config.user_password == ""sv);
    RED_CHECK(config.target_IP == ""sv);
    RED_CHECK_EQUAL(config.port, 3389);
    RED_CHECK_EQUAL(config.mod_state, ClientRedemptionConfig::MOD_RDP);

    // WINDOW DATA
    RED_CHECK(config.WINDOWS_CONF == "/DATA/config/windows_config.config"sv);
    RED_CHECK_EQUAL(config.windowsData.form_x, 0);
    RED_CHECK_EQUAL(config.windowsData.form_y, 0);
    RED_CHECK_EQUAL(config.windowsData.screen_x, 0);
    RED_CHECK_EQUAL(config.windowsData.screen_y, 0);

    // KEYS
    RED_CHECK_EQUAL(config.keyCustomDefinitions.size(), 0);

    // USER
    RED_CHECK_EQUAL(config.userProfils.size(), 1);

    // PATH
    RED_CHECK(config.MAIN_DIR == ""sv);
    RED_CHECK(config.DATA_DIR == "/DATA"sv);
    RED_CHECK(config.REPLAY_DIR == "/DATA/replay"sv);
    RED_CHECK(config.CB_TEMP_DIR == "/DATA/clipboard_temp"sv);
    RED_CHECK(config.DATA_CONF_DIR == "/DATA/config"sv);
    RED_CHECK(config.SOUND_TEMP_DIR == "/DATA/sound_temp"sv);
    RED_CHECK(config.SHARE_DIR == "/home"sv);

    // CLIENT INFO
    RED_CHECK_EQUAL(config.info.screen_info.width, 800);
    RED_CHECK_EQUAL(config.info.screen_info.height, 600);
    RED_CHECK_EQUAL(config.info.keylayout, 0x040C);
    RED_CHECK(not config.info.console_session);
    RED_CHECK_EQUAL(config.info.brush_cache_code , 0);
    RED_CHECK_EQUAL(config.info.screen_info.bpp, BitsPerPixel{24});

    // REMOTE APP
    RED_CHECK(config.rDPRemoteAppConfig.source_of_ExeOrFile == "C:\\Windows\\system32\\notepad.exe"sv);
    RED_CHECK(config.rDPRemoteAppConfig.source_of_WorkingDir == "C:\\Users\\user1"sv);
    RED_CHECK(config.rDPRemoteAppConfig.source_of_Arguments == ""sv);
    RED_CHECK(config.rDPRemoteAppConfig.full_cmd_line == "C:\\Windows\\system32\\notepad.exe "sv);

    // CLIPRDR
    RED_CHECK(int(config.rDPClipboardConfig.arbitrary_scale) == 40);
    RED_CHECK(config.rDPClipboardConfig.server_use_long_format_names);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.cCapabilitiesSets, 1);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.generalFlags, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);

    RED_REQUIRE_EQUAL(config.rDPClipboardConfig.formats.size(), 4);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].format_id(), ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[0].utf8_name(),
        Cliprdr::formats::file_group_descriptor_w.ascii_name);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].format_id(), ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[1].utf8_name(),
        Cliprdr::formats::file_contents.ascii_name);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].format_id(), RDPECLIP::CF_TEXT);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[2].utf8_name(), ""_av);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].format_id(), RDPECLIP::CF_METAFILEPICT);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[3].utf8_name(), ""_av);

    RED_CHECK(config.rDPClipboardConfig.path == "/DATA/clipboard_temp"sv);

    // RED_CHECK_EQUAL(config.
    RED_CHECK(config.rDPDiskConfig.enable_drive_type);
    RED_CHECK(config.rDPDiskConfig.enable_printer_type);
    RED_CHECK(config.rDPDiskConfig.device_list[0].name == "home"sv);
    RED_CHECK_EQUAL(config.rDPDiskConfig.device_list[0].type, rdpdr::RDPDR_DTYP_FILESYSTEM);

    // RDPSND
    RED_CHECK_EQUAL(config.rDPSoundConfig.dwFlags, rdpsnd::TSSNDCAPS_ALIVE | rdpsnd::TSSNDCAPS_VOLUME);
    RED_CHECK_EQUAL(config.rDPSoundConfig.dwVolume, 0x7fff7fff);
    RED_CHECK_EQUAL(config.rDPSoundConfig.dwPitch, 0);
    RED_CHECK_EQUAL(config.rDPSoundConfig.wDGramPort, 0);
    RED_CHECK_EQUAL(config.rDPSoundConfig.wNumberOfFormats, 1);
    RED_CHECK_EQUAL(config.rDPSoundConfig.wVersion, 0x06);

    // RDPPARAM
    RED_CHECK_EQUAL(config.modRDPParamsData.rdp_width, 0);
    RED_CHECK_EQUAL(config.modRDPParamsData.rdp_height, 0);
    RED_CHECK(config.modRDPParamsData.enable_tls);
    RED_CHECK(config.modRDPParamsData.enable_nla);
    RED_CHECK(not config.modRDPParamsData.enable_sound);
    RED_CHECK(config.modRDPParamsData.enable_shared_virtual_disk);
    RED_CHECK(not config.modRDPParamsData.enable_shared_remoteapp);
    RED_CHECK(config.enable_shared_clipboard);

    // VNC
    RED_CHECK(not config.modVNCParamsData.is_apple);
    RED_CHECK(config.modVNCParamsData.vnc_encodings == "5,16,0,1,-239"sv);
    RED_CHECK_EQUAL(config.modVNCParamsData.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.modVNCParamsData.width, 800);
    RED_CHECK_EQUAL(config.modVNCParamsData.height, 600);
    RED_CHECK(not config.modVNCParamsData.enable_tls);
    RED_CHECK(not config.modVNCParamsData.enable_nla);
    RED_CHECK(not config.modVNCParamsData.enable_sound);
    RED_CHECK(not config.modVNCParamsData.enable_shared_clipboard);
    RED_CHECK_EQUAL(config.modVNCParamsData.userProfils.size(), 0);
    RED_CHECK_EQUAL(config.modVNCParamsData.current_user_profil, 0);

    // ACCOUNT
    RED_CHECK_EQUAL(config._accountData.size(), 0);
    RED_CHECK_EQUAL(config._accountNB, 0);
    RED_CHECK(not config._save_password_account);
    RED_CHECK_EQUAL(config._last_target_index, 0);
    RED_CHECK_EQUAL(config.current_user_profil, 0);
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigArgs)
{
    {

    FakeClient client;
    char const * argv[] = {"cmd",
                           "-u", "user",
                           "-p", "password",
                           "-i", "10.10.13.12",
                           "--port", "1793",
                           "--rdpdr",
                           "--rdpsnd",
                           "--cliprdr",
                           "--graphics",
                           "--printer",
                           "--rdpdr",
                           "--rdpdr-dump",
                           "--cliprd-dump",
                           "--basic-trace",
                           "--connection",
                           "--rail-order",
                           "--asynchronous-task",
                           "--capabilities",
                           "--rail",
                           "--rail-dump",
                           "--enable-clipboard",
                           "--enable-nla",
                           "--enable-tls",
                           "--enable-sound",
                           "--enable-fullwindowdrag",
                           "--enable-menuanimations",
                           "--enable-theming",
                           "--enable-cursor-shadow",
                           "--enable-cursorsettings",
                           "--enable-font-smoothing",
                           "--enable-desktop-composition",
                           "--width", "1600",
                           "--height", "900"
    };

    int argc = 39;

    ClientRedemptionConfig config(RDPVerbose::none, "");
    ClientConfig::set_config(argc, argv, config);

    // GENERAL
    RED_CHECK_EQUAL(config.verbose, (RDPVerbose::basic_trace | RDPVerbose::connection | RDPVerbose::capabilities | RDPVerbose::asynchronous_task | RDPVerbose::graphics | RDPVerbose::printer | RDPVerbose::rdpsnd | RDPVerbose::rail | RDPVerbose::cliprdr | RDPVerbose::rdpdr | RDPVerbose::rail_dump | RDPVerbose::basic_trace | RDPVerbose::rdpdr_dump | RDPVerbose::rail_order | RDPVerbose::cliprdr_dump));
    RED_CHECK(config.quick_connection_test);
    RED_CHECK(not config.persist);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK(not config.is_recording);
    RED_CHECK(not config.is_spanning);
    RED_CHECK_EQUAL(config.rdp_width, 1600);
    RED_CHECK_EQUAL(config.rdp_height, 900);
    RED_CHECK(not config.is_full_capturing);
    RED_CHECK(not config.is_full_replaying);
    RED_CHECK_EQUAL(config.full_capture_file_name, ""sv);
    RED_CHECK(not config.is_replaying);
    RED_CHECK(not config.is_loading_replay_mod);
    RED_CHECK(not config.connected);
    RED_CHECK_EQUAL(config._movie_name, ""sv);
    RED_CHECK_EQUAL(config._movie_dir, ""sv);
    RED_CHECK_EQUAL(config._movie_full_path, ""sv);
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::COMMAND_VALID);
    RED_CHECK_EQUAL(config.user_name, "user"sv);
    RED_CHECK_EQUAL(config.user_password, "password"sv);
    RED_CHECK_EQUAL(config.target_IP, "10.10.13.12"sv);
    RED_CHECK_EQUAL(config.port, 1793);
    RED_CHECK_EQUAL(config.mod_state, ClientRedemptionConfig::MOD_RDP);

    // WINDOW DATA
    RED_CHECK_EQUAL(config.WINDOWS_CONF, "/DATA/config/windows_config.config");
    RED_CHECK_EQUAL(config.windowsData.form_x, 0);
    RED_CHECK_EQUAL(config.windowsData.form_y, 0);
    RED_CHECK_EQUAL(config.windowsData.screen_x, 0);
    RED_CHECK_EQUAL(config.windowsData.screen_y, 0);

    // KEYS
    RED_CHECK_EQUAL(config.keyCustomDefinitions.size(), 0);

    // USER
    RED_CHECK_EQUAL(config.userProfils.size(), 1);

    // PATH
    RED_CHECK_EQUAL(config.MAIN_DIR, ""sv);
    RED_CHECK_EQUAL(config.DATA_DIR, "/DATA"sv);
    RED_CHECK_EQUAL(config.REPLAY_DIR, "/DATA/replay"sv);
    RED_CHECK_EQUAL(config.CB_TEMP_DIR, "/DATA/clipboard_temp"sv);
    RED_CHECK_EQUAL(config.DATA_CONF_DIR, "/DATA/config"sv);
    RED_CHECK_EQUAL(config.SOUND_TEMP_DIR, "/DATA/sound_temp"sv);
    RED_CHECK_EQUAL(config.SHARE_DIR, "/home"sv);

    // CLIENT INFO
    RED_CHECK_EQUAL(config.info.screen_info.width, 800);
    RED_CHECK_EQUAL(config.info.screen_info.height, 600);
    RED_CHECK_EQUAL(config.info.keylayout, 0x040C);
    RED_CHECK(not config.info.console_session);
    RED_CHECK_EQUAL(config.info.brush_cache_code , 0);
    RED_CHECK_EQUAL(config.info.screen_info.bpp, BitsPerPixel{24});

    // REMOTE APP
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "C:\\Windows\\system32\\notepad.exe"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "C:\\Users\\user1"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, ""sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.full_cmd_line, "C:\\Windows\\system32\\notepad.exe "sv);

    // CLIPRDR
    RED_CHECK(int(config.rDPClipboardConfig.arbitrary_scale) == 40);
    RED_CHECK(config.rDPClipboardConfig.server_use_long_format_names);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.cCapabilitiesSets, 1);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.generalFlags, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);

    RED_REQUIRE_EQUAL(config.rDPClipboardConfig.formats.size(), 4);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].format_id(), ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[0].utf8_name(),
        Cliprdr::formats::file_group_descriptor_w.ascii_name);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].format_id(), ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[1].utf8_name(),
        Cliprdr::formats::file_contents.ascii_name);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].format_id(), RDPECLIP::CF_TEXT);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[2].utf8_name(), ""_av);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].format_id(), RDPECLIP::CF_METAFILEPICT);
    RED_CHECK_SMEM(config.rDPClipboardConfig.formats[3].utf8_name(), ""_av);

    RED_CHECK_EQUAL(config.rDPClipboardConfig.path, "/DATA/clipboard_temp"sv);

    // RED_CHECK_EQUAL(config.
    RED_CHECK(config.rDPDiskConfig.enable_drive_type);
    RED_CHECK(config.rDPDiskConfig.enable_printer_type);
    RED_CHECK_EQUAL(config.rDPDiskConfig.device_list[0].name, "home"sv);
    RED_CHECK_EQUAL(config.rDPDiskConfig.device_list[0].type, rdpdr::RDPDR_DTYP_FILESYSTEM);

    // RDPSND
    RED_CHECK_EQUAL(config.rDPSoundConfig.dwFlags, rdpsnd::TSSNDCAPS_ALIVE | rdpsnd::TSSNDCAPS_VOLUME);
    RED_CHECK_EQUAL(config.rDPSoundConfig.dwVolume, 0x7fff7fff);
    RED_CHECK_EQUAL(config.rDPSoundConfig.dwPitch, 0);
    RED_CHECK_EQUAL(config.rDPSoundConfig.wDGramPort, 0);
    RED_CHECK_EQUAL(config.rDPSoundConfig.wNumberOfFormats, 1);
    RED_CHECK_EQUAL(config.rDPSoundConfig.wVersion, 0x06);

    // RDPPARAM
    RED_CHECK_EQUAL(config.modRDPParamsData.rdp_width, 0);
    RED_CHECK_EQUAL(config.modRDPParamsData.rdp_height, 0);
    RED_CHECK(config.modRDPParamsData.enable_tls);
    RED_CHECK(config.modRDPParamsData.enable_nla);
    RED_CHECK(config.modRDPParamsData.enable_sound);
    RED_CHECK(config.modRDPParamsData.enable_shared_virtual_disk);
    RED_CHECK(not config.modRDPParamsData.enable_shared_remoteapp);
    RED_CHECK(config.enable_shared_clipboard);

    // VNC
    RED_CHECK(not config.modVNCParamsData.is_apple);
    RED_CHECK_EQUAL(config.modVNCParamsData.vnc_encodings, "5,16,0,1,-239"sv);
    RED_CHECK_EQUAL(config.modVNCParamsData.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.modVNCParamsData.width, 800);
    RED_CHECK_EQUAL(config.modVNCParamsData.height, 600);
    RED_CHECK(not config.modVNCParamsData.enable_tls);
    RED_CHECK(not config.modVNCParamsData.enable_nla);
    RED_CHECK(not config.modVNCParamsData.enable_sound);
    RED_CHECK(not config.modVNCParamsData.enable_shared_clipboard);
    RED_CHECK_EQUAL(config.modVNCParamsData.userProfils.size(), 0);
    RED_CHECK_EQUAL(config.modVNCParamsData.current_user_profil, 0);

    // ACCOUNT
    RED_CHECK_EQUAL(config._accountData.size(), 0);
    RED_CHECK_EQUAL(config._accountNB, 0);
    RED_CHECK(not config._save_password_account);
    RED_CHECK_EQUAL(config._last_target_index, 0);
    RED_CHECK_EQUAL(config.current_user_profil, 0);
    }

    {

    FakeClient client;
    char const * argv[] = {"cmd",
                           //"--vnc",
                           "--remote-app",
                           "--remote-exe", "cmd_to_launch cmd_args",
                           "--remote-dir", "cmd_dir"
    };

    int argc = 6;

    ClientRedemptionConfig config(RDPVerbose::none, "");
    ClientConfig::set_config(argc, argv, config);

    // GENERAL
    RED_CHECK_EQUAL(config.verbose, RDPVerbose::none);
    RED_CHECK(config.quick_connection_test);
    RED_CHECK(not config.persist);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK(not config.is_recording);
    RED_CHECK(not config.is_spanning);
    RED_CHECK_EQUAL(config.rdp_width, 800);
    RED_CHECK_EQUAL(config.rdp_height, 600);
    RED_CHECK(not config.is_full_capturing);
    RED_CHECK(not config.is_full_replaying);
    RED_CHECK_EQUAL(config.full_capture_file_name, ""sv);
    RED_CHECK(not config.is_replaying);
    RED_CHECK(not config.is_loading_replay_mod);
    RED_CHECK(not config.connected);
    RED_CHECK_EQUAL(config._movie_name, ""sv);
    RED_CHECK_EQUAL(config._movie_dir, ""sv);
    RED_CHECK_EQUAL(config._movie_full_path, ""sv);
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::PORT_GOT);
    RED_CHECK_EQUAL(config.user_name, ""sv);
    RED_CHECK_EQUAL(config.user_password, ""sv);
    RED_CHECK_EQUAL(config.target_IP, ""sv);
    RED_CHECK_EQUAL(config.port, 3389);
    RED_CHECK_EQUAL(config.mod_state, ClientRedemptionConfig::MOD_RDP_REMOTE_APP);

     // REMOTE APP
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "cmd_to_launch"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "cmd_dir"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, "cmd_args"sv);
    }

    {

    FakeClient client;
    char const * argv[] = {"cmd",
                           "--vnc",
    };

    int argc = 2;

    ClientRedemptionConfig config(RDPVerbose::none, "");
    ClientConfig::set_config(argc, argv, config);

    // GENERAL
    RED_CHECK_EQUAL(config.verbose, RDPVerbose::none);
    RED_CHECK(config.quick_connection_test);
    RED_CHECK(not config.persist);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK(not config.is_recording);
    RED_CHECK(not config.is_spanning);
    RED_CHECK_EQUAL(config.rdp_width, 800);
    RED_CHECK_EQUAL(config.rdp_height, 600);
    RED_CHECK(not config.is_full_capturing);
    RED_CHECK(not config.is_full_replaying);
    RED_CHECK_EQUAL(config.full_capture_file_name, "");
    RED_CHECK(not config.is_replaying);
    RED_CHECK(not config.is_loading_replay_mod);
    RED_CHECK(not config.connected);
    RED_CHECK_EQUAL(config._movie_name, "");
    RED_CHECK_EQUAL(config._movie_dir, "");
    RED_CHECK_EQUAL(config._movie_full_path, "");
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::PORT_GOT);
    RED_CHECK_EQUAL(config.user_name, "");
    RED_CHECK_EQUAL(config.user_password, "");
    RED_CHECK_EQUAL(config.target_IP, "");
    RED_CHECK_EQUAL(config.port, 3389);
    RED_CHECK_EQUAL(config.mod_state, ClientRedemptionConfig::MOD_VNC);
    }
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigCreateDir)
{
    WorkingDirectory wd("TestClientRedemptionConfigCreateDir");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}


RED_AUTO_TEST_CASE(TestClientRedemptionConfigReadLine) {

    WorkingDirectory wd("TestClientRedemptionConfigReadLine");

    auto const test_file = wd.add_file("test_file.txt");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    write_file(test_file, "hello\nworld"_av);

    {
        unique_fd fd_read = unique_fd(test_file.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

        std::string line;

        RED_CHECK(ClientConfig::read_line(fd_read.fd(), line));
        RED_CHECK_EQUAL(line, "hello"sv);

        RED_CHECK(not ClientConfig::read_line(fd_read.fd(), line));
        RED_CHECK_EQUAL(line, "world"sv);
    }

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigReadClientInfo)
{
    WorkingDirectory wd("TestClientRedemptionConfigReadClientInfo");

    auto const userConfig = wd.create_subdirectory("DATA/config/")
      .add_file("userConfig.config");

    FakeClient client;

    write_file(userConfig,
        "current_user_profil_id 1\n"
        "id 1\n"
        "name Custom\n"
        "keylayout 1037\n"
        "brush_cache_code 2\n"
        "bpp 16\n"
        "width 1600\n"
        "height 900\n"
        "rdp5_performanceflags 4\n"
        "monitorCount 1\n"
        "span 1\n"
        "record 1\n"
        "tls 0\n"
        "nla 0\n"
        "sound 0\n"
        "console_mode 1\n"
        "enable_shared_clipboard 0\n"
        "enable_shared_virtual_disk 0\n"
        "enable_shared_remoteapp 1\n"
        "share-dir /home/test\n"
        "remote-exe C:\\Windows\\system32\\eclipse.exe -h\n"
        "remote-dir C:\\Users\\user2\n"
        "vnc-applekeyboard 1\n"
        "mod 2\n"
        "\n"
        "id 0\n"
        "name Default\n"
        "keylayout 1036\n"
        "brush_cache_code 0\n"
        "bpp 24\n"
        "width 800\n"
        "height 600\n"
        "rdp5_performanceflags 1\n"
        "monitorCount 1\n"
        "span 0\n"
        "record 0\n"
        "tls 1\n"
        "nla 1\n"
        "tls-min-level 0\n"
        "tls-max_level 0\n"
        "show-common-cipher-list 0\n"
        "sound 0\n"
        "console_mode 0\n"
        "enable_shared_clipboard 1\n"
        "enable_shared_virtual_disk 1\n"
        "enable_shared_remoteapp 0\n"
        "share-dir /home\n"
        "remote-exe C:\\Windows\\system32\\eclipse.exe -h\n"
        "remote-dir C:\\Users\\user1\n"
        "vnc-applekeyboard 0\n"
        "mod 2\n"_av
    );

    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK_EQUAL(config.rdp_width, 1600);
    RED_CHECK_EQUAL(config.rdp_height, 900);
    RED_CHECK_EQUAL(config.info.keylayout, 0x040D);
    RED_CHECK(config.info.console_session);
    RED_CHECK_EQUAL(config.info.brush_cache_code , 2);
    RED_CHECK_EQUAL(config.info.screen_info.bpp, BitsPerPixel{16});
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "C:\\Windows\\system32\\eclipse.exe"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "C:\\Users\\user2"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, "-h"sv);
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.full_cmd_line, "C:\\Windows\\system32\\eclipse.exe -h"sv);
    RED_CHECK(config.is_recording);
    RED_CHECK(config.is_spanning);
    RED_CHECK(not config.enable_shared_clipboard);
    RED_CHECK(not config.modRDPParamsData.enable_tls);
    RED_CHECK(not config.modRDPParamsData.enable_nla);
    RED_CHECK(not config.modRDPParamsData.enable_sound);
    RED_CHECK(not config.modRDPParamsData.enable_shared_virtual_disk);
    RED_CHECK(config.modRDPParamsData.enable_shared_remoteapp);
    RED_CHECK_EQUAL(config.info.rdp5_performanceflags, 4);
    RED_CHECK_EQUAL(config.SHARE_DIR, "/home/test"sv);
    RED_CHECK_EQUAL(config.mod_state, 2);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}
/*
RED_AUTO_TEST_CASE(TestClientRedemptionConfigReadMovieData)
{
    WorkingDirectory wd("TestClientRedemptionConfigReadMovieData");

    auto subwd = wd.create_subdirectory("DATA/replay/");
    auto const movie1 = subwd.add_file("movie1.mwrm");
    auto const movie2 = subwd.add_file("movie2.mwrm");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    write_file(movie1, str_concat(
        "v2\n"
        "1600 900\n"
        "nochecksum\n"
        "\n"
        "\n",
        movie1, " 515183 33024 1000 1000 2054 4063648 1511190456 1511190456 1511190439 1511190456\n"));

    {
        unique_fd fd_2(movie2, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        std::string string_to_read1 = str_concat(
            "v2\n"
            "640 480\n"
            "nochecksum\n"
            "\n"
            "\n",
            movie2, " 515183 33024 1000 1000 2054 4063648 1511190456 1511190456 1511190139 1511190456\n");
        ::write(fd_2.fd(), string_to_read1.data(), string_to_read1.size());
    }

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    config.set_icon_movie_data();

    RED_REQUIRE_EQUAL(config.icons_movie_data.size(), 2);*/

//     auto test_movie1 = [&](int i){
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_name, "movie1");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_path, movie1);
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_version, "v2");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_resolution, "1600 900");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_checksum, "nochecksum");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].movie_len, 17);
//     };
//     auto test_movie2 = [&](int i){
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_name, "movie2");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_path, movie2);
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_version, "v2");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_resolution, "640 480");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].file_checksum, "nochecksum");
//         RED_CHECK_EQUAL(config.icons_movie_data[i].movie_len, 317);
//     };
//
//     if (config.icons_movie_data[0].movie_len == 17) {
//         test_movie1(0);
//         test_movie2(1);
//     }
//     else {
//         test_movie1(1);
//         test_movie2(0);
//     }

//     RED_CHECK_WORKSPACE(wd.add_files({
//         "DATA/config/",
//         "DATA/clipboard_temp/",
//         "DATA/sound_temp/"}));
// }

RED_AUTO_TEST_CASE(TestClientRedemptionConfigReadWindowsData)
{
    WorkingDirectory wd("TestClientRedemptionConfigReadWindowsData");

    auto const windows_config = wd.create_subdirectory("DATA/config")
      .add_file("windows_config.config");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    write_file(windows_config,
        "form_x 1920\n"
        "form_y 351\n"
        "screen_x 465\n"
        "screen_y 259\n"_av);

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK(not config.windowsData.no_data);
    RED_CHECK_EQUAL(config.windowsData.form_x, 1920);
    RED_CHECK_EQUAL(config.windowsData.form_y, 351);
    RED_CHECK_EQUAL(config.windowsData.screen_x, 465);
    RED_CHECK_EQUAL(config.windowsData.screen_y, 259);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/replay/",
        "DATA/clipboard_temp/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigReadCustomKeyConfig)
{
    WorkingDirectory wd("TestClientRedemptionConfigReadCustomKeyConfig");

    auto const keySetting = wd.create_subdirectory("DATA/config")
      .add_file("keySetting.config");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    write_file(keySetting,
        "Key Setting\n"
        "- 1 2 x 1 key1\n"
        "- 5 6 y 0 key2\n"_av);

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_REQUIRE_EQUAL(config.keyCustomDefinitions.size(), 2);

    RED_CHECK_EQUAL(config.keyCustomDefinitions[0].qtKeyID, 1);
    RED_CHECK_EQUAL(config.keyCustomDefinitions[0].scanCode, 2);
    RED_CHECK_EQUAL(config.keyCustomDefinitions[0].ASCII8, "x");
    RED_CHECK_EQUAL(config.keyCustomDefinitions[0].extended, 0x0100);
    RED_CHECK_EQUAL(config.keyCustomDefinitions[0].name, "key1");

    RED_CHECK_EQUAL(config.keyCustomDefinitions[1].qtKeyID, 5);
    RED_CHECK_EQUAL(config.keyCustomDefinitions[1].scanCode, 6);
    RED_CHECK_EQUAL(config.keyCustomDefinitions[1].ASCII8, "y");
    RED_CHECK_EQUAL(config.keyCustomDefinitions[1].extended, 0);
    RED_CHECK_EQUAL(config.keyCustomDefinitions[1].name, "key2");

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/replay/",
        "DATA/clipboard_temp/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigReadAccountData)
{
    WorkingDirectory wd("TestClientRedemptionConfigReadAccountData");

    auto const login = wd.create_subdirectory("DATA/config")
      .add_file("login.config");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    write_file(login,
        "save_pwd true\n"
        "last_target 1\n"
        "\n"
        "title 10.10.45.55 - user1\n"
        "IP 10.10.45.55\n"
        "name user1\n"
        "protocol 1\n"
        "pwd mdp\n"
        "port 3389\n"
        "options_profil 0\n"
        "\n"
        "title 10.10.45.87 - measure\n"
        "IP 10.10.45.87\n"
        "name measure\n"
        "protocol 1\n"
        "pwd mdp_\n"
        "port 3389\n"
        "options_profil 0\n"_av);

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK_EQUAL(config._last_target_index, 1);
    RED_CHECK(config._save_password_account);
    RED_REQUIRE_EQUAL(config._accountData.size(), 2);

    RED_CHECK_EQUAL(config._accountData[0].title, "10.10.45.55 - user1"sv);
    RED_CHECK_EQUAL(config._accountData[0].IP, "10.10.45.55"sv);
    RED_CHECK_EQUAL(config._accountData[0].name, "user1"sv);
    RED_CHECK_EQUAL(config._accountData[0].pwd, "mdp"sv);
    RED_CHECK_EQUAL(config._accountData[0].port, 3389);
    RED_CHECK_EQUAL(config._accountData[0].options_profil, 0);
    RED_CHECK_EQUAL(config._accountData[0].index, 0);
    RED_CHECK_EQUAL(config._accountData[0].protocol, 1);

    RED_CHECK_EQUAL(config._accountData[1].title, "10.10.45.87 - measure"sv);
    RED_CHECK_EQUAL(config._accountData[1].IP, "10.10.45.87"sv);
    RED_CHECK_EQUAL(config._accountData[1].name, "measure"sv);
    RED_CHECK_EQUAL(config._accountData[1].pwd, "mdp_"sv);
    RED_CHECK_EQUAL(config._accountData[1].port, 3389);
    RED_CHECK_EQUAL(config._accountData[1].options_profil, 0);
    RED_CHECK_EQUAL(config._accountData[1].index, 1);
    RED_CHECK_EQUAL(config._accountData[1].protocol, 1);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/replay/",
        "DATA/clipboard_temp/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigWriteClientInfo)
{
    WorkingDirectory wd("TestClientRedemptionConfigWriteClientInfo");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    ClientConfig::writeClientInfo(config);

    RED_CHECK_FILE_CONTENTS(wd.add_file("DATA/config/userConfig.config"),
        "current_user_profil_id 0"
        "\n"
        "\nid 0"
        "\nname Default"
        "\nkeylayout 1036"
        "\nbrush_cache_code 0"
        "\nbpp 24"
        "\nwidth 800"
        "\nheight 600"
        "\nrdp5_performanceflags 1"
        "\nmonitorCount 1"
        "\nspan 0"
        "\nrecord 0"
        "\ntls 1"
        "\nnla 1"
        "\ntls-min-level 0"
        "\ntls-max-level 0"
        "\ntls-cipher-string "
        "\nshow_common_cipher_list 0"
        "\nsound 0"
        "\nconsole_mode 0"
        "\nremotefx 0"
        "\nenable_shared_clipboard 1"
        "\nenable_shared_virtual_disk 1"
        "\nenable_shared_remoteapp 0"
        "\nshare-dir /home"
        "\nremote-exe C:\\Windows\\system32\\notepad.exe "
        "\nremote-dir C:\\Users\\user1"
        "\nvnc-applekeyboard 0"
        "\nmod 1"
        "\n"_av);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/",
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigWriteCustomKey)
{
    WorkingDirectory wd("TestClientRedemptionConfigWriteCustomKey");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    config.keyCustomDefinitions.emplace_back(1, 2, "x", 0x100, "key_x");
    config.keyCustomDefinitions.emplace_back(3, 4, "y", 0, "key_y");

    ClientConfig::writeCustomKeyConfig(config);

    RED_CHECK_FILE_CONTENTS(wd.add_file("DATA/config/keySetting.config"),
        "Key Setting\n\n"
        "- 1 2 x 256 key_x\n"
        "- 3 4 y 0 key_y\n"_av);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/",
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigWriteAccountData)
{
    WorkingDirectory wd("TestClientRedemptionConfigWriteAccountData");


    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    config.connected = true;
    ClientConfig::writeAccoundData("10.10.12.13", "account_name", "mdp", 3389, config);
    ClientConfig::writeAccoundData("10.10.13.12", "account_name2", "mdp_", 5900, config);

    RED_CHECK_FILE_CONTENTS(wd.add_file("DATA/config/login.config"),
        "save_pwd false\n"
        "last_target 2\n"
        "\n"
        "title 10.10.12.13 - account_name\n"
        "IP 10.10.12.13\n"
        "name account_name\n"
        "protocol 1\n"
        "pwd \n"
        "port 3389\n"
        "options_profil 0\n"
        "\n"
        "title 10.10.13.12 - account_name2\n"
        "IP 10.10.13.12\n"
        "name account_name2\n"
        "protocol 1\n"
        "pwd \n"
        "port 5900\n"
        "options_profil 0\n\n"_av);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/",
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigWriteWindowsData)
{
    WorkingDirectory wd("TestClientRedemptionConfigWriteWindowsData");

    FakeClient client;
    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    config.windowsData.form_x   = 1;
    config.windowsData.form_y   = 2;
    config.windowsData.screen_x = 3;
    config.windowsData.screen_y = 4;

    ClientConfig::writeWindowsData(config.windowsData);

    RED_CHECK_FILE_CONTENTS(wd.path_of("DATA/config/windows_config.config"),
        "form_x 1\n"
        "form_y 2\n"
        "screen_x 3\n"
        "screen_y 4\n"_av);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/",
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/",
        "DATA/config/windows_config.config"}));
}
