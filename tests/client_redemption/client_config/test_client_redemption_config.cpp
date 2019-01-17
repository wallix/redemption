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

#define RED_TEST_MODULE TestClientRedemptionConfig
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "client_redemption/client_config/client_redemption_config.hpp"

#include "client_redemption/client_channels/fake_client_mod.hpp"

#include "utils/fileutils.hpp"
#include "utils/sugar/algostring.hpp"
#include "test_only/get_file_contents.hpp"
#include "test_only/working_directory.hpp"

#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <linux/hdreg.h>

RED_TEST_DELEGATE_PRINT(RDPVerbose, long(x))


RED_AUTO_TEST_CASE(TestClientRedemptionConfigDefault)
{

    FakeClient client;
//     char const ** argv = nullptr;
//     int argc = 0;

    ClientRedemptionConfig config(RDPVerbose::none, "");
    ClientConfig::setDefaultConfig(config);

    // GENERAL
    RED_CHECK_EQUAL(config.verbose, RDPVerbose::none);
    RED_CHECK_EQUAL(config.quick_connection_test, true);
    RED_CHECK_EQUAL(config.persist, false);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK_EQUAL(config.is_recording, false);
    RED_CHECK_EQUAL(config.is_spanning, false);
    RED_CHECK_EQUAL(config.rdp_width, 800);
    RED_CHECK_EQUAL(config.rdp_height, 600);
    RED_CHECK_EQUAL(config.is_full_capturing, false);
    RED_CHECK_EQUAL(config.is_full_replaying, false);
    RED_CHECK_EQUAL(config.full_capture_file_name, "");
    RED_CHECK_EQUAL(config.is_replaying, false);
    RED_CHECK_EQUAL(config.is_loading_replay_mod, false);
    RED_CHECK_EQUAL(config.connected, false);
    RED_CHECK_EQUAL(config._movie_name, "");
    RED_CHECK_EQUAL(config._movie_dir, "");
    RED_CHECK_EQUAL(config._movie_full_path, "");
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::PORT_GOT);
    RED_CHECK_EQUAL(config.user_name, "");
    RED_CHECK_EQUAL(config.user_password, "");
    RED_CHECK_EQUAL(config.target_IP, "");
    RED_CHECK_EQUAL(config.port, 3389);
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
    RED_CHECK_EQUAL(config.MAIN_DIR, "");
    RED_CHECK_EQUAL(config.DATA_DIR, "/DATA");
    RED_CHECK_EQUAL(config.REPLAY_DIR, "/DATA/replay");
    RED_CHECK_EQUAL(config.CB_TEMP_DIR, "/DATA/clipboard_temp");
    RED_CHECK_EQUAL(config.DATA_CONF_DIR, "/DATA/config");
    RED_CHECK_EQUAL(config.SOUND_TEMP_DIR, "/DATA/sound_temp");
    RED_CHECK_EQUAL(config.SHARE_DIR, "/home");

    // CLIENT INFO
    RED_CHECK_EQUAL(config.info.screen_info.width, 800);
    RED_CHECK_EQUAL(config.info.screen_info.height, 600);
    RED_CHECK_EQUAL(config.info.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.info.console_session, false);
    RED_CHECK_EQUAL(config.info.brush_cache_code , 0);
    RED_CHECK_EQUAL(config.info.screen_info.bpp, BitsPerPixel{24});

    // REMOTE APP
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "C:\\Windows\\system32\\notepad.exe");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "C:\\Users\\user1");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, "");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.full_cmd_line, "C:\\Windows\\system32\\notepad.exe ");

    // CLIPRDR
    RED_CHECK_EQUAL(config.rDPClipboardConfig.arbitrary_scale, 40);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.server_use_long_format_names, true);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.cCapabilitiesSets, 1);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.generalFlags, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);

    RED_REQUIRE_EQUAL(config.rDPClipboardConfig.formats.size(), 4);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].name, RDPECLIP::FILEGROUPDESCRIPTORW.data());

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].name, RDPECLIP::FILECONTENTS.data());

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].ID, RDPECLIP::CF_TEXT);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].name, "");

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].ID, RDPECLIP::CF_METAFILEPICT);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].name, "");

    RED_CHECK_EQUAL(config.rDPClipboardConfig.path, "/DATA/clipboard_temp");

    // RED_CHECK_EQUAL(config.
    RED_CHECK_EQUAL(config.rDPDiskConfig.enable_drive_type, true);
    RED_CHECK_EQUAL(config.rDPDiskConfig.enable_printer_type, true);
    RED_CHECK_EQUAL(config.rDPDiskConfig.device_list[0].name, "home");
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
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_tls, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_nla, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_sound, false);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_shared_virtual_disk, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_shared_remoteapp, false);
    RED_CHECK_EQUAL(config.enable_shared_clipboard, true);

    // VNC
    RED_CHECK_EQUAL(config.modVNCParamsData.is_apple, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.vnc_encodings, "5,16,0,1,-239");
    RED_CHECK_EQUAL(config.modVNCParamsData.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.modVNCParamsData.width, 800);
    RED_CHECK_EQUAL(config.modVNCParamsData.height, 600);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_tls, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_nla, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_sound, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_shared_clipboard, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.userProfils.size(), 0);
    RED_CHECK_EQUAL(config.modVNCParamsData.current_user_profil, 0);

    // ACCOUNT
    RED_CHECK_EQUAL(config._accountData.size(), 0);
    RED_CHECK_EQUAL(config._accountNB, 0);
    RED_CHECK_EQUAL(config._save_password_account, false);
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
    RED_CHECK_EQUAL(config.quick_connection_test, true);
    RED_CHECK_EQUAL(config.persist, false);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK_EQUAL(config.is_recording, false);
    RED_CHECK_EQUAL(config.is_spanning, false);
    RED_CHECK_EQUAL(config.rdp_width, 1600);
    RED_CHECK_EQUAL(config.rdp_height, 900);
    RED_CHECK_EQUAL(config.is_full_capturing, false);
    RED_CHECK_EQUAL(config.is_full_replaying, false);
    RED_CHECK_EQUAL(config.full_capture_file_name, "");
    RED_CHECK_EQUAL(config.is_replaying, false);
    RED_CHECK_EQUAL(config.is_loading_replay_mod, false);
    RED_CHECK_EQUAL(config.connected, false);
    RED_CHECK_EQUAL(config._movie_name, "");
    RED_CHECK_EQUAL(config._movie_dir, "");
    RED_CHECK_EQUAL(config._movie_full_path, "");
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::COMMAND_VALID);
    RED_CHECK_EQUAL(config.user_name, "user");
    RED_CHECK_EQUAL(config.user_password, "password");
    RED_CHECK_EQUAL(config.target_IP, "10.10.13.12");
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
    RED_CHECK_EQUAL(config.MAIN_DIR, "");
    RED_CHECK_EQUAL(config.DATA_DIR, "/DATA");
    RED_CHECK_EQUAL(config.REPLAY_DIR, "/DATA/replay");
    RED_CHECK_EQUAL(config.CB_TEMP_DIR, "/DATA/clipboard_temp");
    RED_CHECK_EQUAL(config.DATA_CONF_DIR, "/DATA/config");
    RED_CHECK_EQUAL(config.SOUND_TEMP_DIR, "/DATA/sound_temp");
    RED_CHECK_EQUAL(config.SHARE_DIR, "/home");

    // CLIENT INFO
    RED_CHECK_EQUAL(config.info.screen_info.width, 800);
    RED_CHECK_EQUAL(config.info.screen_info.height, 600);
    RED_CHECK_EQUAL(config.info.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.info.console_session, false);
    RED_CHECK_EQUAL(config.info.brush_cache_code , 0);
    RED_CHECK_EQUAL(config.info.screen_info.bpp, BitsPerPixel{24});

    // REMOTE APP
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "C:\\Windows\\system32\\notepad.exe");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "C:\\Users\\user1");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, "");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.full_cmd_line, "C:\\Windows\\system32\\notepad.exe ");

    // CLIPRDR
    RED_CHECK_EQUAL(config.rDPClipboardConfig.arbitrary_scale, 40);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.server_use_long_format_names, true);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.cCapabilitiesSets, 1);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.generalFlags, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);

    RED_REQUIRE_EQUAL(config.rDPClipboardConfig.formats.size(), 4);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].name, RDPECLIP::FILEGROUPDESCRIPTORW.data());

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].name, RDPECLIP::FILECONTENTS.data());

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].ID, RDPECLIP::CF_TEXT);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].name, "");

    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].ID, RDPECLIP::CF_METAFILEPICT);
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].name, "");

    RED_CHECK_EQUAL(config.rDPClipboardConfig.path, "/DATA/clipboard_temp");

    // RED_CHECK_EQUAL(config.
    RED_CHECK_EQUAL(config.rDPDiskConfig.enable_drive_type, true);
    RED_CHECK_EQUAL(config.rDPDiskConfig.enable_printer_type, true);
    RED_CHECK_EQUAL(config.rDPDiskConfig.device_list[0].name, "home");
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
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_tls, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_nla, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_sound, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_shared_virtual_disk, true);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_shared_remoteapp, false);
    RED_CHECK_EQUAL(config.enable_shared_clipboard, true);

    // VNC
    RED_CHECK_EQUAL(config.modVNCParamsData.is_apple, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.vnc_encodings, "5,16,0,1,-239");
    RED_CHECK_EQUAL(config.modVNCParamsData.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.modVNCParamsData.width, 800);
    RED_CHECK_EQUAL(config.modVNCParamsData.height, 600);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_tls, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_nla, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_sound, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.enable_shared_clipboard, false);
    RED_CHECK_EQUAL(config.modVNCParamsData.userProfils.size(), 0);
    RED_CHECK_EQUAL(config.modVNCParamsData.current_user_profil, 0);

    // ACCOUNT
    RED_CHECK_EQUAL(config._accountData.size(), 0);
    RED_CHECK_EQUAL(config._accountNB, 0);
    RED_CHECK_EQUAL(config._save_password_account, false);
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
    RED_CHECK_EQUAL(config.quick_connection_test, true);
    RED_CHECK_EQUAL(config.persist, false);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK_EQUAL(config.is_recording, false);
    RED_CHECK_EQUAL(config.is_spanning, false);
    RED_CHECK_EQUAL(config.rdp_width, 800);
    RED_CHECK_EQUAL(config.rdp_height, 600);
    RED_CHECK_EQUAL(config.is_full_capturing, false);
    RED_CHECK_EQUAL(config.is_full_replaying, false);
    RED_CHECK_EQUAL(config.full_capture_file_name, "");
    RED_CHECK_EQUAL(config.is_replaying, false);
    RED_CHECK_EQUAL(config.is_loading_replay_mod, false);
    RED_CHECK_EQUAL(config.connected, false);
    RED_CHECK_EQUAL(config._movie_name, "");
    RED_CHECK_EQUAL(config._movie_dir, "");
    RED_CHECK_EQUAL(config._movie_full_path, "");
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::PORT_GOT);
    RED_CHECK_EQUAL(config.user_name, "");
    RED_CHECK_EQUAL(config.user_password, "");
    RED_CHECK_EQUAL(config.target_IP, "");
    RED_CHECK_EQUAL(config.port, 3389);
    RED_CHECK_EQUAL(config.mod_state, ClientRedemptionConfig::MOD_RDP_REMOTE_APP);

     // REMOTE APP
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "cmd_to_launch");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "cmd_dir");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, "cmd_args");
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
    RED_CHECK_EQUAL(config.quick_connection_test, true);
    RED_CHECK_EQUAL(config.persist, false);
//     std::chrono::milliseconds time_out_disconnection(5000);
//     RED_CHECK_EQUAL(config.time_out_disconnection, time_out_disconnection);
    RED_CHECK_EQUAL(config.keep_alive_freq, 100);
    RED_CHECK_EQUAL(config.is_recording, false);
    RED_CHECK_EQUAL(config.is_spanning, false);
    RED_CHECK_EQUAL(config.rdp_width, 800);
    RED_CHECK_EQUAL(config.rdp_height, 600);
    RED_CHECK_EQUAL(config.is_full_capturing, false);
    RED_CHECK_EQUAL(config.is_full_replaying, false);
    RED_CHECK_EQUAL(config.full_capture_file_name, "");
    RED_CHECK_EQUAL(config.is_replaying, false);
    RED_CHECK_EQUAL(config.is_loading_replay_mod, false);
    RED_CHECK_EQUAL(config.connected, false);
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

    {
        unique_fd fd(test_file, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        auto text = "hello\nworld"_av;
        ::write(fd.fd(), text.data(), text.size());
    }

    {
        unique_fd fd_read = unique_fd(test_file, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

        std::string line;

        RED_CHECK_EQUAL(ClientConfig::read_line(fd_read.fd(), line), true);
        RED_CHECK_EQUAL(line, "hello");

        RED_CHECK_EQUAL(ClientConfig::read_line(fd_read.fd(), line), false);
        RED_CHECK_EQUAL(line, "world");
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

    unique_fd fd(userConfig, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

    const char * toReadData =
                        "current_user_profil_id 1"
                        "\n"
                        "\nid 1"
                        "\nname Custom"
                        "\nkeylayout 1037"
                        "\nbrush_cache_code 2"
                        "\nbpp 16"
                        "\nwidth 1600"
                        "\nheight 900"
                        "\nrdp5_performanceflags 4"
                        "\nmonitorCount 1"
                        "\nspan 1"
                        "\nrecord 1"
                        "\ntls 0"
                        "\nnla 0"
                        "\nsound 0"
                        "\nconsole_mode 1"
                        "\nenable_shared_clipboard 0"
                        "\nenable_shared_virtual_disk 0"
                        "\nenable_shared_remoteapp 1"
                        "\nshare-dir /home/test"
                        "\nremote-exe C:\\Windows\\system32\\eclipse.exe -h"
                        "\nremote-dir C:\\Users\\user2"
                        "\nvnc-applekeyboard 1"
                        "\nmod 2"
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
                        "\nsound 0"
                        "\nconsole_mode 0"
                        "\nenable_shared_clipboard 1"
                        "\nenable_shared_virtual_disk 1"
                        "\nenable_shared_remoteapp 0"
                        "\nshare-dir /home"
                        "\nremote-exe C:\\Windows\\system32\\eclipse.exe -h"
                        "\nremote-dir C:\\Users\\user1"
                        "\nvnc-applekeyboard 0"
                        "\nmod 2"
                        "\n";

    std::string string_to_read(toReadData);
    ::write(fd.fd(), toReadData, string_to_read.length());
    fd.close();

    char const * argv[] = {"cmd"};
    int argc = 1;

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK_EQUAL(config.rdp_width, 1600);
    RED_CHECK_EQUAL(config.rdp_height, 900);
    RED_CHECK_EQUAL(config.info.keylayout, 0x040D);
    RED_CHECK_EQUAL(config.info.console_session, true);
    RED_CHECK_EQUAL(config.info.brush_cache_code , 2);
    RED_CHECK_EQUAL(config.info.screen_info.bpp, BitsPerPixel{16});
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_ExeOrFile, "C:\\Windows\\system32\\eclipse.exe");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_WorkingDir, "C:\\Users\\user2");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.source_of_Arguments, "-h");
    RED_CHECK_EQUAL(config.rDPRemoteAppConfig.full_cmd_line, "C:\\Windows\\system32\\eclipse.exe -h");
    RED_CHECK_EQUAL(config.is_recording, true);
    RED_CHECK_EQUAL(config.is_spanning, true);
    RED_CHECK_EQUAL(config.enable_shared_clipboard, false);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_tls, false);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_nla, false);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_sound, false);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_shared_virtual_disk, false);
    RED_CHECK_EQUAL(config.modRDPParamsData.enable_shared_remoteapp, true);
    RED_CHECK_EQUAL(config.info.rdp5_performanceflags, 4);
    RED_CHECK_EQUAL(config.SHARE_DIR, "/home/test");
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

    {
        unique_fd fd_1(movie1, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        std::string string_to_read1 = str_concat(
            "v2\n"
            "1600 900\n"
            "nochecksum\n"
            "\n"
            "\n",
            movie1, " 515183 33024 1000 1000 2054 4063648 1511190456 1511190456 1511190439 1511190456\n");
        ::write(fd_1.fd(), string_to_read1.data(), string_to_read1.size());
    }

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

    {
        unique_fd fd(windows_config, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        auto toReadData =
            "form_x 1920\n"
            "form_y 351\n"
            "screen_x 465\n"
            "screen_y 259\n"_av;
        ::write(fd.fd(), toReadData.data(), toReadData.size());
    }

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK_EQUAL(config.windowsData.no_data, false);
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

    {
        unique_fd fd(keySetting, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        auto toReadData =
            "Key Setting\n"
            "- 1 2 x 1 key1\n"
            "- 5 6 y 0 key2\n"_av;
        ::write(fd.fd(), toReadData.data(), toReadData.size());
    }

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

    {
        unique_fd fd(login, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        auto toReadData =
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
            "options_profil 0\n"_av;
        ::write(fd.fd(), toReadData.data(), toReadData.size());
    }

    ClientRedemptionConfig config(RDPVerbose::none, wd.dirname());
    ClientConfig::set_config(argc, argv, config);

    RED_CHECK_EQUAL(config._last_target_index, 1);
    RED_CHECK_EQUAL(config._save_password_account, true);
    RED_REQUIRE_EQUAL(config._accountData.size(), 2);

    RED_CHECK_EQUAL(config._accountData[0].title, "10.10.45.55 - user1");
    RED_CHECK_EQUAL(config._accountData[0].IP, "10.10.45.55");
    RED_CHECK_EQUAL(config._accountData[0].name, "user1");
    RED_CHECK_EQUAL(config._accountData[0].pwd, "mdp");
    RED_CHECK_EQUAL(config._accountData[0].port, 3389);
    RED_CHECK_EQUAL(config._accountData[0].options_profil, 0);
    RED_CHECK_EQUAL(config._accountData[0].index, 0);
    RED_CHECK_EQUAL(config._accountData[0].protocol, 1);

    RED_CHECK_EQUAL(config._accountData[1].title, "10.10.45.87 - measure");
    RED_CHECK_EQUAL(config._accountData[1].IP, "10.10.45.87");
    RED_CHECK_EQUAL(config._accountData[1].name, "measure");
    RED_CHECK_EQUAL(config._accountData[1].pwd, "mdp_");
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

    const char * expected_data =
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
                           "\nsound 0"
                           "\nconsole_mode 0"
                           "\nenable_shared_clipboard 1"
                           "\nenable_shared_virtual_disk 1"
                           "\nenable_shared_remoteapp 0"
                           "\nshare-dir /home"
                           "\nremote-exe C:\\Windows\\system32\\notepad.exe "
                           "\nremote-dir C:\\Users\\user1"
                           "\nvnc-applekeyboard 0"
                           "\nmod 1"
                           "\n";

    auto const userConfig = wd.add_file("DATA/config/userConfig.config");
    RED_CHECK_EQUAL(get_file_contents(userConfig), expected_data);

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

    const char * expected_data =
                           "Key Setting\n\n"
                           "- 1 2 x 256 key_x\n"
                           "- 3 4 y 0 key_y\n";

    auto key_setting = wd.add_file("DATA/config/keySetting.config");
    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/",
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));

    RED_CHECK_EQUAL(get_file_contents(key_setting), expected_data);
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

    const char * expected_data =
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
                           "options_profil 0\n\n";

    auto login = wd.add_file("DATA/config/login.config");
    RED_CHECK_EQUAL(get_file_contents(login), expected_data);

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

    auto win_data = wd.add_file("DATA/config/windows_config.config");

    const char * expected_data =
        "form_x 1\n"
        "form_y 2\n"
        "screen_x 3\n"
        "screen_y 4\n";

    RED_CHECK_EQUAL(get_file_contents(wd.path_of("DATA/config/windows_config.config")), expected_data);

    RED_CHECK_WORKSPACE(wd.add_files({
        "DATA/",
        "DATA/config/",
        "DATA/clipboard_temp/",
        "DATA/replay/",
        "DATA/sound_temp/"}));
}
