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
#include "system/redemption_unit_tests.hpp"


#include "client_redemption/client_config/client_redemption_config.hpp"

#include "client_redemption/client_channel_managers/fake_client_mod.hpp"


RED_AUTO_TEST_CASE(TestClientRedemptionConfigDefault)
{
    SessionReactor session_reactor;
    FakeClient client;
    char const ** argv = nullptr;
    int argc = 0;

    ClientRedemptionConfig config(session_reactor, argv, argc, RDPVerbose::none, client, "");

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
    RED_CHECK_EQUAL(config.windowsData.config_file_path, "/DATA/config/windows_config.config");
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
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats.size(), 4);
    if (config.rDPClipboardConfig.formats.size() ==  4) {
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].name, RDPECLIP::FILEGROUPDESCRIPTORW.data());

        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].name, RDPECLIP::FILECONTENTS.data());

        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].ID, RDPECLIP::CF_TEXT);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].name, "");

        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].ID, RDPECLIP::CF_METAFILEPICT);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].name, "");
    }
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
    RED_CHECK_EQUAL(config.vnc_conf.is_apple, false);
    RED_CHECK_EQUAL(config.vnc_conf.vnc_encodings, "5,16,0,1,-239");
    RED_CHECK_EQUAL(config.vnc_conf.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.vnc_conf.width, 800);
    RED_CHECK_EQUAL(config.vnc_conf.height, 600);
    RED_CHECK_EQUAL(config.vnc_conf.enable_tls, false);
    RED_CHECK_EQUAL(config.vnc_conf.enable_nla, false);
    RED_CHECK_EQUAL(config.vnc_conf.enable_sound, false);
    RED_CHECK_EQUAL(config.vnc_conf.enable_shared_clipboard, false);
    RED_CHECK_EQUAL(config.vnc_conf.userProfils.size(), 0);
    RED_CHECK_EQUAL(config.vnc_conf.current_user_profil, 0);

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
    SessionReactor session_reactor;
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
                           "--enable-desktop-composition"
//                            "--vnc",
//                            "--remote-app",
//                            "--remote-exe", "cmd to launch"
    };

    int argc = 35;

    ClientRedemptionConfig config(session_reactor, argv, argc, RDPVerbose::none, client, "");

    // GENERAL
    RED_CHECK_EQUAL(config.verbose, RDPVerbose::basic_trace | RDPVerbose::connection | RDPVerbose::capabilities | RDPVerbose::asynchronous_task | RDPVerbose::graphics | RDPVerbose::printer | RDPVerbose::rdpsnd | RDPVerbose::rail | RDPVerbose::cliprdr | RDPVerbose::rdpdr | RDPVerbose::rail_dump | RDPVerbose::basic_trace | RDPVerbose::rdpdr_dump | RDPVerbose::rail_order | RDPVerbose::cliprdr_dump);
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
    RED_CHECK_EQUAL(config.connection_info_cmd_complete, ClientRedemptionConfig::COMMAND_VALID);
    RED_CHECK_EQUAL(config.user_name, "user");
    RED_CHECK_EQUAL(config.user_password, "password");
    RED_CHECK_EQUAL(config.target_IP, "10.10.13.12");
    RED_CHECK_EQUAL(config.port, 1793);
    RED_CHECK_EQUAL(config.mod_state, ClientRedemptionConfig::MOD_RDP);

    // WINDOW DATA
    RED_CHECK_EQUAL(config.windowsData.config_file_path, "/DATA/config/windows_config.config");
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
    RED_CHECK_EQUAL(config.rDPClipboardConfig.formats.size(), 4);
    if (config.rDPClipboardConfig.formats.size() ==  4) {
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[0].name, RDPECLIP::FILEGROUPDESCRIPTORW.data());

        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].ID, ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[1].name, RDPECLIP::FILECONTENTS.data());

        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].ID, RDPECLIP::CF_TEXT);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[2].name, "");

        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].ID, RDPECLIP::CF_METAFILEPICT);
        RED_CHECK_EQUAL(config.rDPClipboardConfig.formats[3].name, "");
    }
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
    RED_CHECK_EQUAL(config.vnc_conf.is_apple, false);
    RED_CHECK_EQUAL(config.vnc_conf.vnc_encodings, "5,16,0,1,-239");
    RED_CHECK_EQUAL(config.vnc_conf.keylayout, 0x040C);
    RED_CHECK_EQUAL(config.vnc_conf.width, 800);
    RED_CHECK_EQUAL(config.vnc_conf.height, 600);
    RED_CHECK_EQUAL(config.vnc_conf.enable_tls, false);
    RED_CHECK_EQUAL(config.vnc_conf.enable_nla, false);
    RED_CHECK_EQUAL(config.vnc_conf.enable_sound, false);
    RED_CHECK_EQUAL(config.vnc_conf.enable_shared_clipboard, false);
    RED_CHECK_EQUAL(config.vnc_conf.userProfils.size(), 0);
    RED_CHECK_EQUAL(config.vnc_conf.current_user_profil, 0);

    // ACCOUNT
    RED_CHECK_EQUAL(config._accountData.size(), 0);
    RED_CHECK_EQUAL(config._accountNB, 0);
    RED_CHECK_EQUAL(config._save_password_account, false);
    RED_CHECK_EQUAL(config._last_target_index, 0);
    RED_CHECK_EQUAL(config.current_user_profil, 0);
    }

    {
    SessionReactor session_reactor;
    FakeClient client;
    char const * argv[] = {"cmd",
                           //"--vnc",
                           "--remote-app",
                           "--remote-exe", "cmd_to_launch cmd_args",
                           "--remote-dir", "cmd_dir"
    };

    int argc = 6;

    ClientRedemptionConfig config(session_reactor, argv, argc, RDPVerbose::none, client, "");

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
    SessionReactor session_reactor;
    FakeClient client;
    char const * argv[] = {"cmd",
                           "--vnc",
    };

    int argc = 2;

    ClientRedemptionConfig config(session_reactor, argv, argc, RDPVerbose::none, client, "");

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


RED_AUTO_TEST_CASE(TestClientRedemptionConfigRead)
{




}

RED_AUTO_TEST_CASE(TestClientRedemptionConfigWrite)
{




}
