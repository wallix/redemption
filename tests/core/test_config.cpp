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
  Copyright (C) Wallix 2012-2013
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

  Unit test to config.cpp file
  Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConfig
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "config.hpp"
#include <sstream>

BOOST_AUTO_TEST_CASE(TestConfigFromFile)
{
    // test we can read from a file (and not only from a stream)
    Inifile             ini;
    ConfigurationLoader cfg_loader(ini, "/rdpproxy.ini");
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());
    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_up>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::movie>().c_str());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_width>());

    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_error_message>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_lines_per_page>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::selector>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::context::selector_lines_per_page>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::context::selector_number_of_pages>());

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_port>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target_user>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::context::target_port>());
    BOOST_CHECK_EQUAL("RDP",                            ini.get<cfg::context::target_protocol>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::host>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::target>());

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::password>());


    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::password>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_target>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_result>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_answer>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_result>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::accept_message>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::display_message>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::message>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::accept_message>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::display_message>());

    // BOOST_CHECK_EQUAL("Connection refused by authentifier.",
    //                   ini.get<cfg::context::rejected>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::rejected>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::authenticated>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::keepalive>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::keepalive>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::session_id>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::context::end_date_cnx>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::end_time>());

    BOOST_CHECK_EQUAL("allow",                          ini.get<cfg::context::mode_console>());
    BOOST_CHECK_EQUAL(-3600,                            ini.get<cfg::context::timezone>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::real_target_device>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::authentication_challenge>());
}

BOOST_AUTO_TEST_CASE(TestConfigDefaultEmpty)
{
    // default config
    Inifile             ini;
    ConfigurationLoader cfg_loader(ini);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_up>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::movie>().c_str());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_width>());

    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_error_message>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_lines_per_page>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::selector>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::context::selector_lines_per_page>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::context::selector_number_of_pages>());

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_port>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target_user>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::host>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::target>());

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::password>());


    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::password>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_target>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_result>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_answer>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_result>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::accept_message>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::display_message>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::pattern_kill>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::pattern_notify>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::message>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::accept_message>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::display_message>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::rejected>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::authenticated>());


    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::keepalive>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::keepalive>());


    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::session_id>());


    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::context::end_date_cnx>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::end_time>());

    BOOST_CHECK_EQUAL("allow",                          ini.get<cfg::context::mode_console>());
    BOOST_CHECK_EQUAL(-3600,                            ini.get<cfg::context::timezone>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::real_target_device>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::authentication_challenge>());
}

BOOST_AUTO_TEST_CASE(TestConfigDefault)
{
    // test we can read a config file with a global section
    std::stringstream   oss;
    Inifile             ini;
    ConfigurationLoader cfg_loader(ini, oss);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_up>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestConfig1)
{
    // test we can read a config file with a global section
    std::stringstream oss(
                          "[globals]\n"
                          "movie=yes\n"
                          "bitmap_cache=yes\n"
                          "glyph_cache=yes\n"
                          "port=3390\n"
                          "encryptionLevel=low\n"
                          "enable_file_encryption=yes\n"
                          "listen_address=192.168.1.1\n"
                          "enable_ip_transparent=yes\n"
                          "certificate_password=redemption\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "alternate_shell=C:\\WINDOWS\\NOTEPAD.EXE\n"
                          "shell_working_directory=C:\\WINDOWS\\\n"
                          "enable_bitmap_update=true\n"
                          "persistent_path=/var/tmp/wab/persistent/rdp\n"
                          "enable_close_box=false\n"
                          "enable_osd=false\n"
                          "enable_osd_display_remote_target=false\n"
                          "enable_wab_agent=true\n"
                          "wab_agent_launch_timeout=0\n"
                          "wab_agent_keepalive_timeout=0\n"
                          "\n"
                          "[client]\n"
                          "ignore_logon_password=yes\n"
                          "performance_flags_default=0x00000007\n"
                          "performance_flags_force_present=0x1\n"
                          "performance_flags_force_not_present=0x0\n"
                          "tls_fallback_legacy=yes\n"
                          "tls_support=no\n"
                          "rdp_compression=1\n"
                          "bogus_neg_request=yes\n"
                          "bogus_user_id=yes\n"
                          "disable_tsk_switch_shortcuts=yes\n"
                          "max_color_depth=0\n"
                          "persistent_disk_bitmap_cache=yes\n"
                          "cache_waiting_list=no\n"
                          "persist_bitmap_cache_on_disk=yes\n"
                          "bitmap_compression=true\n"
                          "fast_path=true\n"
                          "\n"
                          "[mod_rdp]\n"
                          "disconnect_on_logon_user_change=yes\n"
                          "enable_nla=yes\n"
                          "open_session_timeout=45\n"
                          "certificate_change_action=1\n"
                          "extra_orders=22\n"
                          "persistent_disk_bitmap_cache=false\n"
                          "cache_waiting_list=no\n"
                          "persist_bitmap_cache_on_disk=true\n"
                          "allow_channels=audin\n"
                          "deny_channels=*\n"
                          "fast_path=no\n"
                          "client_device_announce_timeout=1000\n"
                          "proxy_managed_drives=\n"
                          "\n"
                          "[mod_vnc]\n"
                          "clipboard_up=yes\n"
                          "encodings=16,2,0,1,-239\n"
                          "allow_authentification_retries=yes\n"
                          "server_clipboard_encoding_type=latin1\n"
                          "bogus_clipboard_infinite_loop=0\n"
                          "\n"
                          "[video]\n"
                          "hash_path=/mnt/wab/hash\n"
                          "record_path=/mnt/wab/recorded/rdp\n"
                          "record_tmp_path=/mnt/tmp/wab/recorded/rdp\n"
                          "disable_keyboard_log=1\n"
                          "disable_clipboard_log=0\n"
                          "\n"
                          "[crypto]\n"
                          "key0=00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF\n"
                          "key1=FFEEDDCCBBAA99887766554433221100FFEEDDCCBBAA99887766554433221100\n"
                          "\n"
                          "[debug]\n"
                          "password=1\n"
                          "compression=256\n"
                          "cache=128\n"
                          "[translation]\n"
                          "\n"
                          );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini, oss);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL("/var/tmp/wab/persistent/rdp/",   ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL("/mnt/wab/hash/",                 ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL("/mnt/wab/recorded/rdp/",         ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL("/mnt/tmp/wab/recorded/rdp/",     ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::wrm,            ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("192.168.1.1",                    ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("redemption",                     ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp",      ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp",          ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("C:\\WINDOWS\\NOTEPAD.EXE",       ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("C:\\WINDOWS\\",                  ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>(),
                                                               "\x00\x11\x22\x33\x44\x55\x66\x77"
                                                               "\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
                                                               "\x00\x11\x22\x33\x44\x55\x66\x77"
                                                               "\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>(),
                                                               "\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88"
                                                               "\x77\x66\x55\x44\x33\x22\x11\x00"
                                                               "\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88"
                                                               "\x77\x66\x55\x44\x33\x22\x11\x00", 32));

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(256,                              ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(128,                              ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(45,                               ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("22",                             ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("audin",                          ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_vnc::clipboard_up>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    BOOST_CHECK_EQUAL("16,2,0,1,-239",                  ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestConfig1bis)
{
    // test we can read a config file with a global section
    // alternative ways to say yes in file, other values
    std::stringstream oss(
                          "[globals]\n"
                          "bitmap_cache=true\n"
                          "encryptionLevel=medium\n"
                          "enable_file_encryption=no\n"
                          "listen_address=0.0.0.0\n"
                          "enable_ip_transparent=no\n"
                          "certificate_password=\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "alternate_shell=\n"
                          "shell_working_directory=\n"
                          "enable_bitmap_update=no\n"
                          "enable_wab_agent=false\n"
                          "wab_agent_launch_timeout=3000\n"
                          "wab_agent_keepalive_timeout=6000\n"
                          "[client]\n"
                          "performance_flags_default=7\n"
                          "performance_flags_force_present=1\n"
                          "performance_flags_force_not_present=0\n"
                          "tls_support=yes\n"
                          "bogus_neg_request=no\n"
                          "bogus_user_id=no\n"
                          "rdp_compression=0\n"
                          "max_color_depth=8\n"
                          "persistent_disk_bitmap_cache=no\n"
                          "cache_waiting_list=yes\n"
                          "bitmap_compression=on\n"
                          "fast_path=yes\n"
                          "[translation]\n"
                          "[mod_rdp]\n"
                          "rdp_compression=2\n"
                          "disconnect_on_logon_user_change=no\n"
                          "enable_nla=no\n"
                          "open_session_timeout=30\n"
                          "certificate_change_action=0\n"
                          "persistent_disk_bitmap_cache=yes\n"
                          "cache_waiting_list=no\n"
                          "persist_bitmap_cache_on_disk=no\n"
                          "fast_path=yes\n"
                          "bogus_sc_net_size=no\n"
                          "client_device_announce_timeout=1500\n"
                          "proxy_managed_drives=*docs\n"
                          "[mod_replay]\n"
                          "on_end_of_data=1\n"
                          "[video]\n"
                          "hash_path=/mnt/wab/hash/\n"
                          "record_path=/mnt/wab/recorded/rdp/\n"
                          "record_tmp_path=/mnt/tmp/wab/recorded/rdp/\n"
                          "disable_keyboard_log=1\n"
                          "disable_clipboard_log=1\n"
                          "\n"
                          "[mod_vnc]\n"
                          "server_clipboard_encoding_type=utf-8\n"
                          "bogus_clipboard_infinite_loop=1\n"
                          "[crypto]\n"
                          "key0=00112233445566778899AABBCCDDEEFF\n"
                          "key1=FFEEDDCCBBAA99887766554433221100\n"
                          "\n"
                          );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini, oss);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::medium,                    ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(6000,                             ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL("/mnt/wab/hash/",                 ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL("/mnt/wab/recorded/rdp/",         ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL("/mnt/tmp/wab/recorded/rdp/",     ini.get<cfg::video::record_tmp_path>().c_str());
    BOOST_CHECK_EQUAL(KeyboardLogFlags::wrm,            ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,        ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp",      ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp",          ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth8,               ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(2,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1500,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("*docs",                          ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::utf8,      ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestConfig2)
{
    // test we can read a config file with a global section, other values
    std::stringstream oss(
                          "[globals]\n"
                          "bitmap_cache=no\n"
                          "glyph_cache=no\n"
                          "encryptionLevel=high\n"
                          "enable_file_encryption=true\n"
                          "listen_address=127.0.0.1\n"
                          "certificate_password=rdpproxy\n"
                          "enable_ip_transparent=true\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "alternate_shell=C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE\n"
                          "shell_working_directory=\n"
                          "enable_wab_agent=\n"
                          "wab_agent_on_launch_failure=1\n"
                          "[client]\n"
                          "tls_support=yes\n"
                          "performance_flags_default=07\n"
                          "performance_flags_force_present=1\n"
                          "performance_flags_force_not_present=0x\n"
                          "max_color_depth=24\n"
                          "persistent_disk_bitmap_cache=yes\n"
                          "cache_waiting_list=no\n"
                          "persist_bitmap_cache_on_disk=no\n"
                          "bitmap_compression=false\n"
                          "[mod_rdp]\n"
                          "rdp_compression=0\n"
                          "bogus_sc_net_size=yes\n"
                          "proxy_managed_drives=*\n"
                          "[mod_replay]\n"
                          "on_end_of_data=0\n"
                          "[video]\n"
                          "disable_keyboard_log=1\n"
                          "wrm_color_depth_selection_strategy=1\n"
                          "wrm_compression_algorithm=1\n"
                          "\n"
                          );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini, oss);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::high,                      ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::wrm,            ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("rdpproxy",                       ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp",      ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp",          ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE",
                      ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestConfig3)
{
    // test we can read a config file with a global section, other values
    std::stringstream oss(
                          " [ globals ] \n"
                          " bitmap_cache\t= no \n"
                          " glyph_cache = no \n"
                          "encryptionLevel=high\n"
                          "enable_file_encryption=true\n"
                          "listen_address=127.0.0.1\n"
                          "certificate_password=rdpproxy RDP\n"
                          "enable_ip_transparent=true\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "alternate_shell=C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE   \n"
                          "shell_working_directory=\n"
                          "wab_agent_launch_timeout=6000\n"
                          "wab_agent_keepalive_timeout=3000\n"
                          "[client]\t\n"
                          "tls_support=yes\n"
                          "bogus_user_id=yes\n"
                          "performance_flags_default=07\n"
                          "performance_flags_force_present=1\n"
                          "performance_flags_force_not_present=0x\n"
                          "max_color_depth=24\n"
                          "persistent_disk_bitmap_cache=yes\n"
                          "cache_waiting_list=no\n"
                          "persist_bitmap_cache_on_disk=no\n"
                          "bitmap_compression=false\n"
                          "\t[mod_rdp]\n"
                          "rdp_compression=0\n"
                          "bogus_sc_net_size=no\n"
                          "client_device_announce_timeout=1500\n"
                          "[mod_replay]\n"
                          "on_end_of_data=0\n"
                          "[mod_vnc]\n"
                          "bogus_clipboard_infinite_loop=2\n"
                          "[video]\n"
                          "disable_keyboard_log=1\n"
                          "wrm_color_depth_selection_strategy=1\n"
                          "wrm_compression_algorithm=1\n"
                          "disable_clipboard_log=0\n"
                          "\n"
                          );

    Inifile             ini;

    ini.set<cfg::globals::shell_working_directory>("C:\\");

    ConfigurationLoader cfg_loader(ini, oss);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::high,                      ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(6000,                             ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::wrm,            ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("rdpproxy RDP",                   ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp",      ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp",          ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE",
                      ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1500,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(2,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestMultiple)
{
    // test we can read a config file with a global section
    std::stringstream oss(
                          "[globals]\n"
                          "bitmap_cache=no\n"
                          "port=3390\n"
                          "encryptionLevel=low\n"
                          "enable_file_encryption=False\n"
                          "listen_address=0.0.0.0\n"
                          "certificate_password=redemption\n"
                          "enable_ip_transparent=False\n"
                          "shell_working_directory=%HOMEDRIVE%%HOMEPATH%\n"
                          "enable_wab_agent=true\n"
                          "[client]\n"
                          "bitmap_compression=TRuE\n"
                          "\n"
                          "[mod_rdp]\n"
                          "persistent_disk_bitmap_cache=true\n"
                          "cache_waiting_list=no\n"
                          "\n"
                          );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini, oss);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("redemption",                     ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("%HOMEDRIVE%%HOMEPATH%",          ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());


    // see we can change configuration using parse without default setting of existing ini
    std::stringstream oss2(
                           "[globals]\n"
                           "enable_file_encryption=yes\n"
                           "listen_address=192.168.1.1\n"
                           "certificate_password=\n"
                           "enable_ip_transparent=yes\n"
                           "glyph_cache=yes\n"
                          "wab_agent_launch_timeout=4000\n"
                          "wab_agent_on_launch_failure=1\n"
                          "wab_agent_keepalive_timeout=7000\n"
                           "[client]\n"
                           "bitmap_compression=no\n"
                           "persist_bitmap_cache_on_disk=yes\n"
                           "bogus_user_id=yes\n"
                           "[mod_rdp]\n"
                           "persist_bitmap_cache_on_disk=yes\n"
                           "proxy_managed_drives=docs,apps\n"
                           "[mod_vnc]\n"
                           "bogus_clipboard_infinite_loop=0\n"
                           "[debug]\n"
                           "password=3\n"
                           "compression=0x3\n"
                           "cache=0\n"
                           );
    cfg_loader.cparse(ini, oss2);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(4000,                             ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(1,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(7000,                             ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("192.168.1.1",                    ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("%HOMEDRIVE%%HOMEPATH%",          ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(3,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(3,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("docs,apps",                      ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestNewConf)
{
    // new behavior:
    // init() load default values from main configuration file
    // - options with multiple occurences get the last value
    // - unrecognized lines are ignored
    // - every characters following # are ignored until end of line (comments)
    Inifile             ini;
    ConfigurationLoader cfg_loader(ini);
    char                temp_path[1024];

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());


    TODO("video related values should go to [video] section")
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());


    std::stringstream ifs2(
                           "# Here we put global values\n"
                           "[globals]\n"
                           "# below we have lines with syntax errors, but they are just ignored\n"
                           "yyy\n"
                           "zzz\n"
                           "# unknwon keys are also ignored\n"
                           "yyy=1\n"
                           "[client]\n"
                           "bitmap_compression=no\n"
                           );

    cfg_loader.cparse(ini, ifs2);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::movie>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::globals::auth_channel>(), "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::authip>().c_str());
    BOOST_CHECK_EQUAL(3350,                             ini.get<cfg::globals::authport>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, PERSISTENT_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::globals::persistent_path>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_wab_agent>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_launch_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::globals::wab_agent_keepalive_timeout>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm, ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::video::png_interval>());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL((pathncpy(temp_path, HASH_PATH,  sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::hash_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_path>().c_str());
    BOOST_CHECK_EQUAL((pathncpy(temp_path, RECORD_TMP_PATH, sizeof(temp_path)), temp_path),
                                                        ini.get<cfg::video::record_tmp_path>().c_str());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::none,           ini.get<cfg::video::disable_keyboard_log>());

    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>().c_str());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_file_encryption>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_ip_transparent>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>().c_str());

    BOOST_CHECK_EQUAL(PNG_PATH,                         ini.get<cfg::globals::png_path>().c_str());
    BOOST_CHECK_EQUAL(WRM_PATH,                         ini.get<cfg::globals::wrm_path>().c_str());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_int>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x8,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(4,                                ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::certificate_change_action>());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL(1000,                             ini.get<cfg::mod_rdp::client_device_announce_timeout>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>().c_str());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,    ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    BOOST_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    BOOST_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(24,                               ini.get<cfg::context::opt_bpp>());
}

BOOST_AUTO_TEST_CASE(TestConfigTools)
{
    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("0"));
    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("0x"));

    BOOST_CHECK_EQUAL(3,        ulong_from_cstr("3"));
    BOOST_CHECK_EQUAL(3,        ulong_from_cstr("0x3"));

    BOOST_CHECK_EQUAL(7,        ulong_from_cstr("0x00000007"));
    BOOST_CHECK_EQUAL(7,        ulong_from_cstr("0x0000000000000007"));
    BOOST_CHECK_EQUAL(7,        ulong_from_cstr("0x0007    "));

    BOOST_CHECK_EQUAL(1357,     ulong_from_cstr("1357"));
    BOOST_CHECK_EQUAL(4951,     ulong_from_cstr("0x1357"));

    BOOST_CHECK_EQUAL(10,       ulong_from_cstr("0x0A"));
    BOOST_CHECK_EQUAL(10,       ulong_from_cstr("0x0a"));

    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("0x0000000I"));
    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("I"));

    BOOST_CHECK_EQUAL(Level::low,    level_from_cstr("LoW"));
    BOOST_CHECK_EQUAL(Level::medium, level_from_cstr("mEdIuM"));
    BOOST_CHECK_EQUAL(Level::high,   level_from_cstr("High"));

    BOOST_CHECK_EQUAL(3600,     _long_from_cstr("3600"));
    BOOST_CHECK_EQUAL(0,        _long_from_cstr("0"));
    BOOST_CHECK_EQUAL(0,        _long_from_cstr(""));
    BOOST_CHECK_EQUAL(-3600,    _long_from_cstr("-3600"));
}

BOOST_AUTO_TEST_CASE(TestContextSetValue)
{
    Inifile             ini;
    ConfigurationLoader cfg_loader(ini);

    // bitrate, framerate, qscale
    ini.set_by_id(AUTHID_OPT_BITRATE,           "80000");
    ini.set_by_id(AUTHID_OPT_FRAMERATE,         "6");
    ini.set_by_id(AUTHID_OPT_QSCALE,            "16");

    BOOST_CHECK_EQUAL(80000,                          ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(6,                              ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(16,                             ini.get<cfg::context::opt_qscale>());

    BOOST_CHECK_EQUAL(80000,                          ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(6,                              ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(16,                             ini.get<cfg::context::opt_qscale>());


    // bpp, height, width
    ini.ask_by_id(AUTHID_OPT_BPP);
    ini.ask_by_id(AUTHID_OPT_HEIGHT);
    ini.ask_by_id(AUTHID_OPT_WIDTH);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::opt_width>());

    ini.set_by_id(AUTHID_OPT_BPP,               "16");
    ini.set_by_id(AUTHID_OPT_HEIGHT,            "1024");
    ini.set_by_id(AUTHID_OPT_WIDTH,             "1280");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_width>());

    BOOST_CHECK_EQUAL(1280,                             ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(1024,                             ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(16,                               ini.get<cfg::context::opt_bpp>());

    BOOST_CHECK_EQUAL(1280,                             ini.get<cfg::context::opt_width>());
    BOOST_CHECK_EQUAL(1024,                             ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(16,                               ini.get<cfg::context::opt_bpp>());


    ini.set_by_id(AUTHID_AUTH_ERROR_MESSAGE,     "Message d'erreur.");

    BOOST_CHECK_EQUAL("Message d'erreur.", ini.get<cfg::context::auth_error_message>());


    // selector, ...
    ini.ask_by_id(AUTHID_SELECTOR);
    ini.ask_by_id(AUTHID_SELECTOR_CURRENT_PAGE);
    ini.ask_by_id(AUTHID_SELECTOR_DEVICE_FILTER);
    ini.ask_by_id(AUTHID_SELECTOR_GROUP_FILTER);
    ini.ask_by_id(AUTHID_SELECTOR_LINES_PER_PAGE);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::selector>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::selector_lines_per_page>());

    ini.set_by_id(AUTHID_SELECTOR,                  "True");
    ini.set_by_id(AUTHID_SELECTOR_CURRENT_PAGE,     "2");
    ini.set_by_id(AUTHID_SELECTOR_DEVICE_FILTER,    "Windows");
    ini.set_by_id(AUTHID_SELECTOR_GROUP_FILTER,     "RDP");
    ini.set_by_id(AUTHID_SELECTOR_LINES_PER_PAGE,   "25");
    ini.set_by_id(AUTHID_SELECTOR_NUMBER_OF_PAGES,  "2");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_lines_per_page>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::context::selector>());
    BOOST_CHECK_EQUAL(2,                                ini.get<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL("Windows",                        ini.get<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL("RDP",                            ini.get<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(25,                               ini.get<cfg::context::selector_lines_per_page>());
    BOOST_CHECK_EQUAL(2,                                ini.get<cfg::context::selector_number_of_pages>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::context::selector>());
    BOOST_CHECK_EQUAL(2,                                ini.get<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL("Windows",                        ini.get<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL("RDP",                            ini.get<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(25,                               ini.get<cfg::context::selector_lines_per_page>());
    BOOST_CHECK_EQUAL(2,                                ini.get<cfg::context::selector_number_of_pages>());


    // target_xxxx
    ini.set_by_id(AUTHID_TARGET_DEVICE,         "127.0.0.1");
    ini.set_by_id(AUTHID_TARGET_PASSWORD,       "12345678");
    ini.set_by_id(AUTHID_TARGET_PORT,           "3390");
    ini.set_by_id(AUTHID_TARGET_PROTOCOL,       "RDP");
    ini.set_by_id(AUTHID_TARGET_USER,           "admin");
    ini.set_by_id(AUTHID_TARGET_APPLICATION,    "wallix@putty");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::target_port>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::target_user>());

    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("12345678",                       ini.get<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::context::target_port>());
    BOOST_CHECK_EQUAL("RDP",                            ini.get<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL("admin",                          ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("wallix@putty",                   ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("12345678",                       ini.get<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::context::target_port>());
    BOOST_CHECK_EQUAL("RDP",                            ini.get<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL("admin",                          ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("wallix@putty",                   ini.get<cfg::globals::target_application>());


    // host
    ini.ask_by_id(AUTHID_HOST);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::host>());

    ini.set_by_id(AUTHID_HOST,                   "127.0.0.1");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::host>());

    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::host>());

    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get_by_id(AUTHID_HOST));


    // target
    ini.ask_by_id(AUTHID_TARGET);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target>());

    ini.set_by_id(AUTHID_TARGET,                 "192.168.0.1");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::target>());

    BOOST_CHECK_EQUAL("192.168.0.1",                    ini.get<cfg::globals::target>());

    BOOST_CHECK_EQUAL("192.168.0.1",                    ini.get_by_id(AUTHID_TARGET));


    // auth_user
    ini.set_by_id(AUTHID_AUTH_USER,             "admin");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::auth_user>());

    BOOST_CHECK_EQUAL("admin",                          ini.get<cfg::globals::auth_user>());

    BOOST_CHECK_EQUAL("admin",                          ini.get_by_id(AUTHID_AUTH_USER));


    // password
    ini.ask_by_id(AUTHID_PASSWORD);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::password>());

    ini.set_by_id(AUTHID_PASSWORD,              "12345678");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::password>());

    BOOST_CHECK_EQUAL("12345678",                       ini.get<cfg::context::password>());

    BOOST_CHECK_EQUAL("12345678",                       ini.get_by_id(AUTHID_PASSWORD));


    // answer
    ini.set_by_id(AUTHID_AUTH_CHANNEL_ANSWER,    "answer");

    BOOST_CHECK_EQUAL("answer",                         ini.get<cfg::context::auth_channel_answer>());

    BOOST_CHECK_EQUAL("answer",                         ini.get_by_id(AUTHID_AUTH_CHANNEL_ANSWER));


    // authchannel_target
    ini.ask_by_id(AUTHID_AUTH_CHANNEL_TARGET);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::auth_channel_target>());

    ini.set_by_id(AUTHID_AUTH_CHANNEL_TARGET,    "target");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL("target",                         ini.get<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL("target",                         ini.get_by_id(AUTHID_AUTH_CHANNEL_TARGET));


    // authchannel_result
    ini.ask_by_id(AUTHID_AUTH_CHANNEL_RESULT);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::auth_channel_result>());

    ini.set_by_id(AUTHID_AUTH_CHANNEL_RESULT,    "result");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_result>());

    BOOST_CHECK_EQUAL("result",                         ini.get<cfg::context::auth_channel_result>());

    BOOST_CHECK_EQUAL("result",                         ini.get_by_id(AUTHID_AUTH_CHANNEL_RESULT));

    // regex
    ini.set_by_id(AUTHID_PATTERN_KILL,          "Explorer");
    BOOST_CHECK_EQUAL("Explorer",                       ini.get<cfg::context::pattern_kill>());
    BOOST_CHECK_EQUAL("Explorer",                       ini.get_by_id(AUTHID_PATTERN_KILL));

    ini.set_by_id(AUTHID_PATTERN_NOTIFY,        "Word");
    BOOST_CHECK_EQUAL("Word",                           ini.get<cfg::context::pattern_notify>());
    BOOST_CHECK_EQUAL("Word",                           ini.get_by_id(AUTHID_PATTERN_NOTIFY));


    // message
    ini.set_by_id(AUTHID_MESSAGE,               "message");

    BOOST_CHECK_EQUAL("message",                        ini.get<cfg::context::message>());

    BOOST_CHECK_EQUAL("message",                        ini.get_by_id(AUTHID_MESSAGE));


    // accept_message
    ini.ask_by_id(AUTHID_ACCEPT_MESSAGE);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::accept_message>());

    ini.set_by_id(AUTHID_ACCEPT_MESSAGE,        "accept_message");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::accept_message>());

    BOOST_CHECK_EQUAL("accept_message",                 ini.get<cfg::context::accept_message>());

    BOOST_CHECK_EQUAL("accept_message",                 ini.get_by_id(AUTHID_ACCEPT_MESSAGE));


    // display_message
    ini.ask_by_id(AUTHID_DISPLAY_MESSAGE);

    BOOST_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::display_message>());

    ini.set_by_id(AUTHID_DISPLAY_MESSAGE,       "display_message");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::display_message>());

    BOOST_CHECK_EQUAL("display_message",                ini.get<cfg::context::display_message>());

    BOOST_CHECK_EQUAL("display_message",                ini.get_by_id(AUTHID_DISPLAY_MESSAGE));


    // rejected
    ini.set_by_id(AUTHID_REJECTED,              "rejected");

    BOOST_CHECK_EQUAL("rejected",                       ini.get<cfg::context::rejected>());

    BOOST_CHECK_EQUAL("rejected",                       ini.get_by_id(AUTHID_REJECTED));


    // authenticated
    ini.set_by_id(AUTHID_AUTHENTICATED,         "True");

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::context::authenticated>());

    BOOST_CHECK_EQUAL("True",                           ini.get_by_id(AUTHID_AUTHENTICATED));


    // keepalive
    ini.set_by_id(AUTHID_KEEPALIVE,             "True");

    BOOST_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::keepalive>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::context::keepalive>());

    BOOST_CHECK_EQUAL("True",                           ini.get_by_id(AUTHID_KEEPALIVE));


    // session_id
    ini.set_by_id(AUTHID_SESSION_ID,            "0123456789");

    BOOST_CHECK_EQUAL("0123456789",                     ini.get<cfg::context::session_id>());

    BOOST_CHECK_EQUAL("0123456789",                     ini.get_by_id(AUTHID_SESSION_ID));


    // end_date_cnx
    ini.set_by_id(AUTHID_END_DATE_CNX,          "12345678");

    BOOST_CHECK_EQUAL(12345678,                         ini.get<cfg::context::end_date_cnx>());

    BOOST_CHECK_EQUAL("12345678",                       ini.get_by_id(AUTHID_END_DATE_CNX));


    // end_time
    ini.set_by_id(AUTHID_END_TIME,              "end_time");

    BOOST_CHECK_EQUAL("end_time",                       ini.get<cfg::context::end_time>());

    BOOST_CHECK_EQUAL("end_time",                       ini.get_by_id(AUTHID_END_TIME));


    // mode_console
    ini.set_by_id(AUTHID_MODE_CONSOLE,          "deny");

    BOOST_CHECK_EQUAL("deny",                           ini.get<cfg::context::mode_console>());

    BOOST_CHECK_EQUAL("deny",                           ini.get_by_id(AUTHID_MODE_CONSOLE));


    // timezone
    ini.set_by_id(AUTHID_TIMEZONE,              "-7200");

    BOOST_CHECK_EQUAL(-7200,                            ini.get<cfg::context::timezone>());

    BOOST_CHECK_EQUAL("-7200",                          ini.get_by_id(AUTHID_TIMEZONE));


    // real_target_device
    ini.set_by_id(AUTHID_REAL_TARGET_DEVICE,     "10.0.0.1");

    BOOST_CHECK_EQUAL("10.0.0.1",                       ini.get<cfg::context::real_target_device>());

    BOOST_CHECK_EQUAL("10.0.0.1",                       ini.get_by_id(AUTHID_REAL_TARGET_DEVICE));


    // authentication_challenge
    ini.set_by_id(AUTHID_AUTHENTICATION_CHALLENGE,     "true");

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::context::authentication_challenge>());

    BOOST_CHECK_EQUAL("True",                           ini.get_by_id(AUTHID_AUTHENTICATION_CHALLENGE));
}


BOOST_AUTO_TEST_CASE(TestAuthentificationKeywordRecognition)
{
   BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("unknown"));
   BOOST_CHECK_EQUAL(AUTHID_TARGET_USER, authid_from_string(STRAUTHID_TARGET_USER));
   BOOST_CHECK_EQUAL(AUTHID_TARGET_PASSWORD, authid_from_string(STRAUTHID_TARGET_PASSWORD));
   BOOST_CHECK_EQUAL(AUTHID_HOST, authid_from_string(STRAUTHID_HOST));
   BOOST_CHECK_EQUAL(AUTHID_PASSWORD, authid_from_string(STRAUTHID_PASSWORD));
   BOOST_CHECK_EQUAL(AUTHID_AUTH_USER, authid_from_string(STRAUTHID_AUTH_USER));
   BOOST_CHECK_EQUAL(AUTHID_TARGET_DEVICE, authid_from_string(STRAUTHID_TARGET_DEVICE));
   BOOST_CHECK_EQUAL(AUTHID_TARGET_PORT, authid_from_string(STRAUTHID_TARGET_PORT));
   BOOST_CHECK_EQUAL(AUTHID_TARGET_PROTOCOL, authid_from_string(STRAUTHID_TARGET_PROTOCOL));
   BOOST_CHECK_EQUAL(AUTHID_REJECTED, authid_from_string(STRAUTHID_REJECTED));
   BOOST_CHECK_EQUAL(AUTHID_OPT_MOVIE, authid_from_string(STRAUTHID_OPT_MOVIE));
   BOOST_CHECK_EQUAL(AUTHID_OPT_MOVIE_PATH, authid_from_string(STRAUTHID_OPT_MOVIE_PATH));
   BOOST_CHECK_EQUAL(AUTHID_MESSAGE, authid_from_string(STRAUTHID_MESSAGE));
   BOOST_CHECK_EQUAL(AUTHID_OPT_BITRATE, authid_from_string(STRAUTHID_OPT_BITRATE));
   BOOST_CHECK_EQUAL(AUTHID_OPT_FRAMERATE, authid_from_string(STRAUTHID_OPT_FRAMERATE));
   BOOST_CHECK_EQUAL(AUTHID_OPT_QSCALE, authid_from_string(STRAUTHID_OPT_QSCALE));
   BOOST_CHECK_EQUAL(AUTHID_OPT_WIDTH, authid_from_string(STRAUTHID_OPT_WIDTH));
   BOOST_CHECK_EQUAL(AUTHID_OPT_HEIGHT, authid_from_string(STRAUTHID_OPT_HEIGHT));
   BOOST_CHECK_EQUAL(AUTHID_OPT_BPP, authid_from_string(STRAUTHID_OPT_BPP));
   BOOST_CHECK_EQUAL(AUTHID_AUTHENTICATED, authid_from_string(STRAUTHID_AUTHENTICATED));
   BOOST_CHECK_EQUAL(AUTHID_SELECTOR, authid_from_string(STRAUTHID_SELECTOR));
   BOOST_CHECK_EQUAL(AUTHID_KEEPALIVE, authid_from_string(STRAUTHID_KEEPALIVE));
   BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("8899676"));
   BOOST_CHECK_EQUAL(AUTHID_DISPLAY_MESSAGE, authid_from_string(STRAUTHID_DISPLAY_MESSAGE));
   BOOST_CHECK_EQUAL(AUTHID_ACCEPT_MESSAGE, authid_from_string(STRAUTHID_ACCEPT_MESSAGE));
   BOOST_CHECK_EQUAL(AUTHID_AUTH_ERROR_MESSAGE, authid_from_string(STRAUTHID_AUTH_ERROR_MESSAGE));
   BOOST_CHECK_EQUAL(AUTHID_MODE_CONSOLE, authid_from_string(STRAUTHID_MODE_CONSOLE));
   BOOST_CHECK_EQUAL(AUTHID_TIMEZONE, authid_from_string(STRAUTHID_TIMEZONE));
}



BOOST_AUTO_TEST_CASE(TestConfigNotifications)
{
//     Inifile             ini;
//     ConfigurationLoader cfg_loader(ini);
    /*
    // nothing has been changed initialy
    //    BOOST_CHECK(!ini.check());

    // auth_user has been changed, so check() method will notify that something changed
    ini.set<cfg::globals::auth_user>("someoneelse");
    BOOST_CHECK(ini.check());
    BOOST_CHECK_EQUAL("someoneelse",       ini.get<cfg::get_changed_list()::back()->()get_value());>


    ini.reset();
    BOOST_CHECK(!ini.check());

    // setting a field without changing it should not notify that something changed
    ini.set<cfg::globals::auth_user>("someoneelse");
    BOOST_CHECK(!ini.check());


    // Using the list of changed fields:
    ini.set<cfg::globals::auth_user>("someuser");
    ini.set<cfg::globals::host>("35.53.0.1");
    ini.set<cfg::context::opt_height>(602);
    ini.set<cfg::globals::target>("35.53.0.2");
    ini.set<cfg::context::selector>(true);
    BOOST_CHECK(ini.check());
    std::list< Inifile::BaseField * > list = ini.get_changed_list();
    BOOST_CHECK_EQUAL("True",      list.back()->get_value());
    list.pop_back();
    BOOST_CHECK_EQUAL("35.53.0.2", list.back()->get_value());
    list.pop_back();
    BOOST_CHECK_EQUAL("602",       list.back()->get_value());
    list.pop_back();
    BOOST_CHECK_EQUAL("35.53.0.1", list.back()->get_value());
    list.pop_back();
    BOOST_CHECK_EQUAL("someuser",  list.back()->get_value());
    ini.reset();
    BOOST_CHECK(!ini.check());
    */
}

// BOOST_AUTO_TEST_CASE(TestConfigFieldGetValue)
// {
//     Inifile             ini;
//     ConfigurationLoader cfg_loader(ini);
//     // Test get_value()
//
//     ini.ask<cfg::globals::target_user>());
//     BOOST_CHECK_EQUAL("ASK",       ini.get<cfg::globals::target_user>().get_value());
//     ini.set<cfg::globals::target_user>("linuxuser");
//     BOOST_CHECK_EQUAL("linuxuser", ini.get<cfg::globals::target_user>().get_value());
//     ini.ask<cfg::globals::target_user>());
//     BOOST_CHECK_EQUAL("ASK",       ini.get<cfg::globals::target_user>().get_value());
//
//     ini.set<cfg::globals::enable_file_encryption>(true);
//     BOOST_CHECK_EQUAL("True",      ini.get<cfg::globals::enable_file_encryption>().get_value());
//     ini.set<cfg::globals::enable_file_encryption>(false);
//     BOOST_CHECK_EQUAL("False",     ini.get<cfg::globals::enable_file_encryption>().get_value());
//     ini.ask<cfg::globals::enable_file_encryption>());
//     BOOST_CHECK_EQUAL("ASK",       ini.get<cfg::globals::enable_file_encryption>().get_value());
//
//     ini.ask<cfg::context::opt_bpp>());
//     BOOST_CHECK_EQUAL("ASK",       ini.get<cfg::context::opt_bpp>().get_value());
//     ini.set<cfg::context::opt_bpp>(123);
//     BOOST_CHECK_EQUAL("123",       ini.get<cfg::context::opt_bpp>().get_value());
//     ini.set<cfg::context::opt_bpp>(741258);
//     BOOST_CHECK_EQUAL("741258",    ini.get<cfg::context::opt_bpp>().get_value());
//     ini.ask<cfg::context::opt_bpp>());
//     BOOST_CHECK_EQUAL("ASK",       ini.get<cfg::context::opt_bpp>().get_value());
//
//     ini.set<cfg::context::selector_lines_per_page>(1111155555);
//     BOOST_CHECK_EQUAL("1111155555",ini.context.selector_lines_per_page.get_value());
//     ini.ask<cfg::context::selector_lines_per_page>());
//     BOOST_CHECK_EQUAL("ASK",       ini.get<cfg::context::selector_lines_per_page>().get_value());
//
// }
