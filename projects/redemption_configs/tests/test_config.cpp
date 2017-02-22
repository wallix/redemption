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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
// #define LOGPRINT

#include "configs/config.hpp"
#include <sstream>
#include <fstream>

inline std::string to_string_path(std::string path)
{
    if (path.empty()) {
        return "./";
    }
    if (path.front() != '/') {
        path = "./" + path;
    }
    if (path.back() != '/') {
        path += '/';
    }
    return path;
}

BOOST_AUTO_TEST_CASE(TestConfigFromFile)
{
    // test we can read from a file (and not only from a stream)
    Inifile             ini;
    {
        std::ofstream out("/tmp/tmp-rdpproxy.ini");
        out <<
          "[globals]\n"
          "wrm_path = /tmp/raw/movie\n"
          "persistent_path = tmp/raw/persistent\n"
        ;
    }
    ConfigurationLoader cfg_loader(ini.configuration_holder(), "/tmp/tmp-rdpproxy.ini");
    BOOST_CHECK_EQUAL("/tmp/raw/movie/",       ini.get<cfg::globals::wrm_path>());
    BOOST_CHECK_EQUAL("./tmp/raw/persistent/", ini.get<cfg::globals::persistent_path>());
}

BOOST_AUTO_TEST_CASE(TestConfigDefaultEmpty)
{
    // default config
    Inifile             ini;

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::localfile_hashed,
                                                        ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>().data(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>().data(),
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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(20000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_up>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    BOOST_CHECK_EQUAL("./",                             ini.get<cfg::context::movie>());

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

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_answer>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::context::message>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::accept_message>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::context::display_message>());

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


BOOST_AUTO_TEST_CASE(TestConfig1)
{
    // test we can read a config file with a global section
    std::stringstream oss(
        "[globals]\n"
        "bitmap_cache=yes\n"
        "glyph_cache=yes\n"
        "port=3390\n"
        "encryptionLevel=low\n"
        "trace_type=2\n"
        "listen_address=192.168.1.1\n"
        "enable_transparent_mode=yes\n"
        "certificate_password=redemption\n"
        "png_path=/var/tmp/wab/recorded/rdp\n"
        "wrm_path=/var/wab/recorded/rdp\n"
        "enable_bitmap_update=true\n"
        "persistent_path=/var/tmp/wab/persistent/rdp\n"
        "enable_close_box=false\n"
        "enable_osd=false\n"
        "enable_osd_display_remote_target=false\n"
        "authentication_timeout=150\n"
        "close_timeout=900\n"
        "handshake_timeout=5\n"
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
        "extra_orders=22\n"
        "persistent_disk_bitmap_cache=false\n"
        "cache_waiting_list=no\n"
        "persist_bitmap_cache_on_disk=true\n"
        "allow_channels=audin\n"
        "deny_channels=*\n"
        "fast_path=no\n"
        "proxy_managed_drives=\n"
        "alternate_shell=C:\\WINDOWS\\NOTEPAD.EXE\n"
        "shell_working_directory=C:\\WINDOWS\\\n"
        "enable_session_probe=true\n"
        "enable_session_probe_launch_mask=false\n"
        "session_probe_launch_timeout=0\n"
        "session_probe_keepalive_timeout=0\n"
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
        "encryption_key=00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF\n"
        "sign_key=FFEEDDCCBBAA99887766554433221100FFEEDDCCBBAA99887766554433221100\n"
        "\n"
        "[debug]\n"
        "password=1\n"
        "compression=256\n"
        "cache=128\n"
        "[translation]\n"
        "\n"
    );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini.configuration_holder(), oss);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL("/var/tmp/wab/persistent/rdp/",   ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL("/mnt/wab/hash/",                 ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL("/mnt/wab/recorded/rdp/",         ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL("/mnt/tmp/wab/recorded/rdp/",     ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(150,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("192.168.1.1",                    ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("redemption",                     ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>().data(),
                                                               "\x00\x11\x22\x33\x44\x55\x66\x77"
                                                               "\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
                                                               "\x00\x11\x22\x33\x44\x55\x66\x77"
                                                               "\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>().data(),
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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
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
    BOOST_CHECK_EQUAL(RdpCompression::rdp4,             ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(45,                               ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("22",                             ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("audin",                          ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("C:\\WINDOWS\\NOTEPAD.EXE",       ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("C:\\WINDOWS\\",                  ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_vnc::clipboard_up>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    BOOST_CHECK_EQUAL("16,2,0,1,-239",                  ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
                          "trace_type=0\n"
                          "listen_address=0.0.0.0\n"
                          "enable_transparent_mode=no\n"
                          "certificate_password=\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "shell_working_directory=\n"
                          "enable_bitmap_update=no\n"
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
                          "persistent_disk_bitmap_cache=yes\n"
                          "cache_waiting_list=no\n"
                          "persist_bitmap_cache_on_disk=no\n"
                          "fast_path=yes\n"
                          "bogus_sc_net_size=no\n"
                          "proxy_managed_drives=*docs\n"
                          "alternate_shell=\n"
                          "enable_session_probe=false\n"
                          "session_probe_launch_timeout=3000\n"
                          "session_probe_keepalive_timeout=6000\n"
                          "[mod_replay]\n"
                          "on_end_of_data=1\n"
                          "[video]\n"
                          "hash_path=/mnt/wab/hash/\n"
                          "record_path=/mnt/wab/recorded/rdp/\n"
                          "record_tmp_path=/mnt/tmp/wab/recorded/rdp/\n"
                          "disable_keyboard_log=1\n"
                          "disable_clipboard_log=1\n"
                          "disable_file_system_log=2\n"
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
    ConfigurationLoader cfg_loader(ini.configuration_holder(), oss);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::medium,                    ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL("/mnt/wab/hash/",                 ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL("/mnt/wab/recorded/rdp/",         ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL("/mnt/tmp/wab/recorded/rdp/",     ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,         ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::wrm,          ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::localfile,             ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_bitmap_update>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>().data(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>().data(),
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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
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
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::none,             ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth8,               ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp5,             ini.get<cfg::mod_rdp::rdp_compression>()); BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("*docs",                          ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(6000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::utf8,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::duplicated,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
                          "trace_type=2\n"
                          "listen_address=127.0.0.1\n"
                          "certificate_password=rdpproxy\n"
                          "enable_transparent_mode=true\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "shell_working_directory=\n"
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
                          "alternate_shell=C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE\n"
                          "enable_session_probe=\n"
                          "session_probe_on_launch_failure=2\n"
                          "[mod_replay]\n"
                          "on_end_of_data=0\n"
                          "[video]\n"
                          "disable_keyboard_log=1\n"
                          "wrm_color_depth_selection_strategy=1\n"
                          "wrm_compression_algorithm=1\n"
                          "\n"
                          );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini.configuration_holder(), oss);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::high,                      ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("rdpproxy",                       ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
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
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::none,             ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE",
                      ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(20000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
                          "trace_type=2\n"
                          "listen_address=127.0.0.1\n"
                          "certificate_password=rdpproxy RDP\n"
                          "enable_transparent_mode=true\n"
                          "png_path=/var/tmp/wab/recorded/rdp\n"
                          "wrm_path=/var/wab/recorded/rdp\n"
                          "close_timeout=300\n"
                          "handshake_timeout=7\n"
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
                          "alternate_shell=C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE   \n"
                          "shell_working_directory=\n"
                          "session_probe_launch_timeout=6000\n"
                          "session_probe_keepalive_timeout=3000\n"
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

    ini.set<cfg::mod_rdp::shell_working_directory>("C:\\");

    ConfigurationLoader cfg_loader(ini.configuration_holder(), oss);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::high,                      ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(7,                                ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(300,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("rdpproxy RDP",                   ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
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
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::none,             ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE",
                      ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(6000,                             ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(3000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::continued,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
                          "trace_type=0\n"
                          "listen_address=0.0.0.0\n"
                          "certificate_password=redemption\n"
                          "enable_transparent_mode=False\n"
                          "[client]\n"
                          "bitmap_compression=TRuE\n"
                          "\n"
                          "[mod_rdp]\n"
                          "persistent_disk_bitmap_cache=true\n"
                          "cache_waiting_list=no\n"
                          "shell_working_directory=%HOMEDRIVE%%HOMEPATH%\n"
                          "enable_session_probe=true\n"
                          "\n"
                          );

    Inifile             ini;
    ConfigurationLoader cfg_loader(ini.configuration_holder(), oss);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::localfile,             ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("redemption",                     ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("%HOMEDRIVE%%HOMEPATH%",          ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(20000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
                           "trace_type=2\n"
                           "listen_address=192.168.1.1\n"
                           "certificate_password=\n"
                           "enable_transparent_mode=yes\n"
                           "glyph_cache=yes\n"
                           "[client]\n"
                           "bitmap_compression=no\n"
                           "persist_bitmap_cache_on_disk=yes\n"
                           "bogus_user_id=yes\n"
                           "[mod_rdp]\n"
                           "persist_bitmap_cache_on_disk=yes\n"
                           "proxy_managed_drives=docs,apps\n"
                           "session_probe_launch_timeout=4000\n"
                           "session_probe_on_launch_failure=0\n"
                           "session_probe_keepalive_timeout=7000\n"
                           "[mod_vnc]\n"
                           "bogus_clipboard_infinite_loop=0\n"
                           "[debug]\n"
                           "password=3\n"
                           "compression=0x3\n"
                           "cache=0\n"
                           );
    cfg_loader.cparse(ini.configuration_holder(), oss2);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("192.168.1.1",                    ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(3,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(3,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x28,                              ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("docs,apps",                      ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("%HOMEDRIVE%%HOMEPATH%",          ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(4000,                             ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::ignore_and_continue,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(7000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
    ConfigurationLoader cfg_loader(ini.configuration_holder());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());


    // TODO video related values should go to [video] section
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::localfile_hashed,
                                                        ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(20000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
                           "authentication_timeout=300\n"
                           "yyy\n"
                           "zzz\n"
                           "# unknwon keys are also ignored\n"
                           "yyy=1\n"
                           "[client]\n"
                           "bitmap_compression=no\n"
                           );

    cfg_loader.cparse(ini.configuration_holder(), ifs2);

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    BOOST_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    BOOST_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    BOOST_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    BOOST_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    BOOST_CHECK_EQUAL(CaptureFlags::png | CaptureFlags::wrm,
                                                        ini.get<cfg::video::capture_flags>());
    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    BOOST_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    BOOST_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    BOOST_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    BOOST_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    BOOST_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    BOOST_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    BOOST_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    BOOST_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    BOOST_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    BOOST_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    BOOST_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    BOOST_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    BOOST_CHECK_EQUAL(300,                              ini.get<cfg::globals::authentication_timeout>().count());
    BOOST_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    BOOST_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    BOOST_CHECK_EQUAL(TraceType::localfile_hashed,
                                                        ini.get<cfg::globals::trace_type>());
    BOOST_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    BOOST_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>());

    BOOST_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    BOOST_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

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
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    BOOST_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    BOOST_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    BOOST_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    BOOST_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    BOOST_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    BOOST_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    BOOST_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    BOOST_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe_launch_mask>());
    BOOST_CHECK_EQUAL(20000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    BOOST_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    BOOST_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    BOOST_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    BOOST_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::allow_authentification_retries>());
    BOOST_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    BOOST_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

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
    using namespace configs;

    {
        unsigned u;
        spec_type<unsigned> stype;

        BOOST_CHECK(!parse(u, stype, cstr_array_view("")));
        BOOST_CHECK_EQUAL(0, u);

        BOOST_CHECK(!parse(u, stype, cstr_array_view("0")));
        BOOST_CHECK_EQUAL(0, u);
        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x")));
        BOOST_CHECK_EQUAL(0, u);

        BOOST_CHECK(!parse(u, stype, cstr_array_view("3")));
        BOOST_CHECK_EQUAL(3, u);
        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x3")));
        BOOST_CHECK_EQUAL(3, u);

        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x00000007")));
        BOOST_CHECK_EQUAL(7, u);
        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x0000000000000007")));
        BOOST_CHECK_EQUAL(7, u);
        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x0007")));
        BOOST_CHECK_EQUAL(7, u);

        BOOST_CHECK(!parse(u, stype, cstr_array_view("1357")));
        BOOST_CHECK_EQUAL(1357, u);
        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x1357")));
        BOOST_CHECK_EQUAL(0x1357, u);

        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x0A")));
        BOOST_CHECK_EQUAL(0x0a, u);
        BOOST_CHECK(!parse(u, stype, cstr_array_view("0x0a")));
        BOOST_CHECK_EQUAL(0x0a, u);

        BOOST_CHECK(parse(u, stype, cstr_array_view("0x0000000I")));
        BOOST_CHECK(parse(u, stype, cstr_array_view("I")));

    }

    {
        Level level;
        spec_type<Level> stype;

        BOOST_CHECK(!parse(level, stype, cstr_array_view("LoW")));
        BOOST_CHECK_EQUAL(Level::low, level);

        BOOST_CHECK(!parse(level, stype, cstr_array_view("mEdIuM")));
        BOOST_CHECK_EQUAL(Level::medium, level);

        BOOST_CHECK(!parse(level, stype, cstr_array_view("High")));
        BOOST_CHECK_EQUAL(Level::high, level);

        BOOST_CHECK(parse(level, stype, cstr_array_view("dsifsu")));
    }

    {
        int i;
        spec_type<int> stype;

        BOOST_CHECK(!parse(i, stype, cstr_array_view("3600")));
        BOOST_CHECK_EQUAL(3600, i);
        BOOST_CHECK(!parse(i, stype, cstr_array_view("0")));
        BOOST_CHECK_EQUAL(0, i);
        BOOST_CHECK(!parse(i, stype, cstr_array_view("")));
        BOOST_CHECK_EQUAL(0, i);
        BOOST_CHECK(!parse(i, stype, cstr_array_view("-3600")));
        BOOST_CHECK_EQUAL(-3600, i);
    }
}

BOOST_AUTO_TEST_CASE(TestContextSetValue)
{
    Inifile             ini;
    ConfigurationLoader cfg_loader(ini.configuration_holder());

    // bitrate, framerate, qscale
    ini.get_acl_field(AUTHID_CONTEXT_OPT_BITRATE).set(cstr_array_view("80000"));
    ini.get_acl_field(AUTHID_CONTEXT_OPT_FRAMERATE).set(cstr_array_view("6"));
    ini.get_acl_field(AUTHID_CONTEXT_OPT_QSCALE).set(cstr_array_view("16"));

    BOOST_CHECK_EQUAL(80000, ini.get<cfg::context::opt_bitrate>());
    BOOST_CHECK_EQUAL(6,     ini.get<cfg::context::opt_framerate>());
    BOOST_CHECK_EQUAL(16,    ini.get<cfg::context::opt_qscale>());

    BOOST_CHECK_EQUAL("80000", ini.get_acl_field(AUTHID_CONTEXT_OPT_BITRATE).c_str());
    BOOST_CHECK_EQUAL("6",     ini.get_acl_field(AUTHID_CONTEXT_OPT_FRAMERATE).c_str());
    BOOST_CHECK_EQUAL("16",    ini.get_acl_field(AUTHID_CONTEXT_OPT_QSCALE).c_str());


    // bpp, height, width
    ini.get_acl_field(AUTHID_CONTEXT_OPT_BPP).ask();
    ini.get_acl_field(AUTHID_CONTEXT_OPT_HEIGHT).ask();
    ini.get_acl_field(AUTHID_CONTEXT_OPT_WIDTH).ask();

    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::opt_width>());

    ini.get_acl_field(AUTHID_CONTEXT_OPT_BPP).set(cstr_array_view("16"));
    ini.get_acl_field(AUTHID_CONTEXT_OPT_HEIGHT).set(cstr_array_view("1024"));
    ini.get_acl_field(AUTHID_CONTEXT_OPT_WIDTH).set(cstr_array_view("1280"));

    BOOST_CHECK_EQUAL(false, ini.is_asked<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(false, ini.is_asked<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(false, ini.is_asked<cfg::context::opt_width>());

    BOOST_CHECK_EQUAL(16,   ini.get<cfg::context::opt_bpp>());
    BOOST_CHECK_EQUAL(1024, ini.get<cfg::context::opt_height>());
    BOOST_CHECK_EQUAL(1280, ini.get<cfg::context::opt_width>());

    BOOST_CHECK_EQUAL("16",   ini.get_acl_field(AUTHID_CONTEXT_OPT_BPP).c_str());
    BOOST_CHECK_EQUAL("1024", ini.get_acl_field(AUTHID_CONTEXT_OPT_HEIGHT).c_str());
    BOOST_CHECK_EQUAL("1280", ini.get_acl_field(AUTHID_CONTEXT_OPT_WIDTH).c_str());


    // selector, ...
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR).ask();
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE).ask();
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_DEVICE_FILTER).ask();
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_GROUP_FILTER).ask();
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_LINES_PER_PAGE).ask();

    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector>());
    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_lines_per_page>());

    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR).set(cstr_array_view("True"));
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE).set(cstr_array_view("2"));
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_DEVICE_FILTER).set(cstr_array_view("Windows"));
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_GROUP_FILTER).set(cstr_array_view("RDP"));
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_LINES_PER_PAGE).set(cstr_array_view("25"));
    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_NUMBER_OF_PAGES).set(cstr_array_view("2"));

    BOOST_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector>());
    BOOST_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_lines_per_page>());

    BOOST_CHECK_EQUAL(true,      ini.get<cfg::context::selector>());
    BOOST_CHECK_EQUAL(2,         ini.get<cfg::context::selector_current_page>());
    BOOST_CHECK_EQUAL("Windows", ini.get<cfg::context::selector_device_filter>());
    BOOST_CHECK_EQUAL("RDP",     ini.get<cfg::context::selector_group_filter>());
    BOOST_CHECK_EQUAL(25,        ini.get<cfg::context::selector_lines_per_page>());
    BOOST_CHECK_EQUAL(2,         ini.get<cfg::context::selector_number_of_pages>());

    BOOST_CHECK_EQUAL("True",    ini.get_acl_field(AUTHID_CONTEXT_SELECTOR).c_str());
    BOOST_CHECK_EQUAL("2",       ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE).c_str());
    BOOST_CHECK_EQUAL("Windows", ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_DEVICE_FILTER).c_str());
    BOOST_CHECK_EQUAL("RDP",     ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_GROUP_FILTER).c_str());
    BOOST_CHECK_EQUAL("25",      ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_LINES_PER_PAGE).c_str());
    BOOST_CHECK_EQUAL("2",       ini.get_acl_field(AUTHID_CONTEXT_SELECTOR_NUMBER_OF_PAGES).c_str());


    // target_xxxx
    ini.get_acl_field(AUTHID_GLOBALS_TARGET_DEVICE).set(cstr_array_view("127.0.0.1"));
    ini.get_acl_field(AUTHID_CONTEXT_TARGET_PASSWORD).set(cstr_array_view("12345678"));
    ini.get_acl_field(AUTHID_CONTEXT_TARGET_PORT).set(cstr_array_view("3390"));
    ini.get_acl_field(AUTHID_CONTEXT_TARGET_PROTOCOL).set(cstr_array_view("RDP"));
    ini.get_acl_field(AUTHID_GLOBALS_TARGET_USER).set(cstr_array_view("admin"));
    ini.get_acl_field(AUTHID_GLOBALS_TARGET_APPLICATION).set(cstr_array_view("wallix@putty"));

    BOOST_CHECK_EQUAL(false,          ini.is_asked<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL(false,          ini.is_asked<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(false,          ini.is_asked<cfg::context::target_port>());
    BOOST_CHECK_EQUAL(false,          ini.is_asked<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL(false,          ini.is_asked<cfg::globals::target_user>());

    BOOST_CHECK_EQUAL("127.0.0.1",    ini.get<cfg::globals::target_device>());
    BOOST_CHECK_EQUAL("12345678",     ini.get<cfg::context::target_password>());
    BOOST_CHECK_EQUAL(3390,           ini.get<cfg::context::target_port>());
    BOOST_CHECK_EQUAL("RDP",          ini.get<cfg::context::target_protocol>());
    BOOST_CHECK_EQUAL("admin",        ini.get<cfg::globals::target_user>());
    BOOST_CHECK_EQUAL("wallix@putty", ini.get<cfg::globals::target_application>());

    BOOST_CHECK_EQUAL("127.0.0.1",    ini.get_acl_field(AUTHID_GLOBALS_TARGET_DEVICE).c_str());
    BOOST_CHECK_EQUAL("12345678",     ini.get_acl_field(AUTHID_CONTEXT_TARGET_PASSWORD).c_str());
    BOOST_CHECK_EQUAL("3390",         ini.get_acl_field(AUTHID_CONTEXT_TARGET_PORT).c_str());
    BOOST_CHECK_EQUAL("RDP",          ini.get_acl_field(AUTHID_CONTEXT_TARGET_PROTOCOL).c_str());
    BOOST_CHECK_EQUAL("admin",        ini.get_acl_field(AUTHID_GLOBALS_TARGET_USER).c_str());
    BOOST_CHECK_EQUAL("wallix@putty", ini.get_acl_field(AUTHID_GLOBALS_TARGET_APPLICATION).c_str());


    // host
    ini.get_acl_field(AUTHID_GLOBALS_HOST).ask();

    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::globals::host>());

    ini.get_acl_field(AUTHID_GLOBALS_HOST).set(cstr_array_view("127.0.0.1"));

    BOOST_CHECK_EQUAL(false,       ini.is_asked<cfg::globals::host>());

    BOOST_CHECK_EQUAL("127.0.0.1", ini.get<cfg::globals::host>());

    BOOST_CHECK_EQUAL("127.0.0.1", ini.get_acl_field(AUTHID_GLOBALS_HOST).c_str());
    BOOST_CHECK_EQUAL(9,           ini.get_acl_field(AUTHID_GLOBALS_HOST).to_string_view().size());


    // target
    ini.get_acl_field(AUTHID_GLOBALS_TARGET).ask();

    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::globals::target>());

    ini.get_acl_field(AUTHID_GLOBALS_TARGET).set(cstr_array_view("192.168.0.1"));

    BOOST_CHECK_EQUAL(false,         ini.is_asked<cfg::globals::target>());

    BOOST_CHECK_EQUAL("192.168.0.1", ini.get<cfg::globals::target>());

    BOOST_CHECK_EQUAL("192.168.0.1", ini.get_acl_field(AUTHID_GLOBALS_TARGET).c_str());


    // auth_user
    ini.get_acl_field(AUTHID_GLOBALS_AUTH_USER).set(cstr_array_view("admin"));

    BOOST_CHECK_EQUAL(false,   ini.is_asked<cfg::globals::auth_user>());

    BOOST_CHECK_EQUAL("admin", ini.get<cfg::globals::auth_user>());

    BOOST_CHECK_EQUAL("admin", ini.get_acl_field(AUTHID_GLOBALS_AUTH_USER).c_str());


    // password
    ini.get_acl_field(AUTHID_CONTEXT_PASSWORD).ask();

    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::password>());

    ini.get_acl_field(AUTHID_CONTEXT_PASSWORD).set(cstr_array_view("12345678"));

    BOOST_CHECK_EQUAL(false,      ini.is_asked<cfg::context::password>());

    BOOST_CHECK_EQUAL("12345678", ini.get<cfg::context::password>());

    BOOST_CHECK_EQUAL("12345678", ini.get_acl_field(AUTHID_CONTEXT_PASSWORD).c_str());


    // answer
    ini.get_acl_field(AUTHID_CONTEXT_AUTH_CHANNEL_ANSWER).set(cstr_array_view("answer"));

    BOOST_CHECK_EQUAL("answer", ini.get<cfg::context::auth_channel_answer>());


    // authchannel_target
    ini.get_acl_field(AUTHID_CONTEXT_AUTH_CHANNEL_TARGET).ask();

    BOOST_CHECK_EQUAL(true, ini.is_asked<cfg::context::auth_channel_target>());

    ini.get_acl_field(AUTHID_CONTEXT_AUTH_CHANNEL_TARGET).set(cstr_array_view("target"));

    BOOST_CHECK_EQUAL(false, 	ini.is_asked<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL("target", ini.get<cfg::context::auth_channel_target>());

    BOOST_CHECK_EQUAL("target", ini.get_acl_field(AUTHID_CONTEXT_AUTH_CHANNEL_TARGET).c_str());


    // message
    ini.get_acl_field(AUTHID_CONTEXT_MESSAGE).set(cstr_array_view("message"));

    BOOST_CHECK_EQUAL("message", ini.get<cfg::context::message>());


    // rejected
    ini.get_acl_field(AUTHID_CONTEXT_REJECTED).set(cstr_array_view("rejected"));

    BOOST_CHECK_EQUAL("rejected", ini.get<cfg::context::rejected>());

    BOOST_CHECK_EQUAL("rejected", ini.get_acl_field(AUTHID_CONTEXT_REJECTED).c_str());


    // authenticated
    ini.get_acl_field(AUTHID_CONTEXT_AUTHENTICATED).set(cstr_array_view("True"));

    BOOST_CHECK_EQUAL(true,   ini.get<cfg::context::authenticated>());

    BOOST_CHECK_EQUAL("True", ini.get_acl_field(AUTHID_CONTEXT_AUTHENTICATED).c_str());


    // keepalive
    ini.get_acl_field(AUTHID_CONTEXT_KEEPALIVE).set(cstr_array_view("True"));

    BOOST_CHECK_EQUAL(false, ini.is_asked<cfg::context::keepalive>());

    BOOST_CHECK_EQUAL(true,  ini.get<cfg::context::keepalive>());


    // session_id
    ini.get_acl_field(AUTHID_CONTEXT_SESSION_ID).set(cstr_array_view("0123456789"));

    BOOST_CHECK_EQUAL("0123456789", ini.get<cfg::context::session_id>());


    // end_date_cnx
    ini.get_acl_field(AUTHID_CONTEXT_END_DATE_CNX).set(cstr_array_view("12345678"));

    BOOST_CHECK_EQUAL(12345678, ini.get<cfg::context::end_date_cnx>());


    // end_time
    ini.get_acl_field(AUTHID_CONTEXT_END_TIME).set(cstr_array_view("end_time"));

    BOOST_CHECK_EQUAL("end_time", ini.get<cfg::context::end_time>());


    // mode_console
    ini.get_acl_field(AUTHID_CONTEXT_MODE_CONSOLE).set(cstr_array_view("deny"));

    BOOST_CHECK_EQUAL("deny", ini.get<cfg::context::mode_console>());


    // timezone
    ini.get_acl_field(AUTHID_CONTEXT_TIMEZONE).set(cstr_array_view("-7200"));

    BOOST_CHECK_EQUAL(-7200, ini.get<cfg::context::timezone>());


    // real_target_device
    ini.get_acl_field(AUTHID_CONTEXT_REAL_TARGET_DEVICE).set(cstr_array_view("10.0.0.1"));

    BOOST_CHECK_EQUAL("10.0.0.1", ini.get<cfg::context::real_target_device>());

    BOOST_CHECK_EQUAL("10.0.0.1", ini.get_acl_field(AUTHID_CONTEXT_REAL_TARGET_DEVICE).c_str());


    // authentication_challenge
    ini.get_acl_field(AUTHID_CONTEXT_AUTHENTICATION_CHALLENGE).set(cstr_array_view("true"));

    BOOST_CHECK_EQUAL(true, ini.get<cfg::context::authentication_challenge>());
}


BOOST_AUTO_TEST_CASE(TestAuthentificationKeywordRecognition)
{
   BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("unknown"));
   BOOST_CHECK_EQUAL(AUTHID_GLOBALS_TARGET_USER, authid_from_string(string_from_authid(AUTHID_GLOBALS_TARGET_USER)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_TARGET_PASSWORD, authid_from_string(string_from_authid(AUTHID_CONTEXT_TARGET_PASSWORD)));
   BOOST_CHECK_EQUAL(AUTHID_GLOBALS_HOST, authid_from_string(string_from_authid(AUTHID_GLOBALS_HOST)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_PASSWORD, authid_from_string(string_from_authid(AUTHID_CONTEXT_PASSWORD)));
   BOOST_CHECK_EQUAL(AUTHID_GLOBALS_AUTH_USER, authid_from_string(string_from_authid(AUTHID_GLOBALS_AUTH_USER)));
   BOOST_CHECK_EQUAL(AUTHID_GLOBALS_TARGET_DEVICE, authid_from_string(string_from_authid(AUTHID_GLOBALS_TARGET_DEVICE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_TARGET_PORT, authid_from_string(string_from_authid(AUTHID_CONTEXT_TARGET_PORT)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_TARGET_PROTOCOL, authid_from_string(string_from_authid(AUTHID_CONTEXT_TARGET_PROTOCOL)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_REJECTED, authid_from_string(string_from_authid(AUTHID_CONTEXT_REJECTED)));
   BOOST_CHECK_EQUAL(AUTHID_GLOBALS_IS_REC, authid_from_string(string_from_authid(AUTHID_GLOBALS_IS_REC)));
   BOOST_CHECK_EQUAL(AUTHID_GLOBALS_MOVIE_PATH, authid_from_string(string_from_authid(AUTHID_GLOBALS_MOVIE_PATH)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_MESSAGE, authid_from_string(string_from_authid(AUTHID_CONTEXT_MESSAGE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_OPT_BITRATE, authid_from_string(string_from_authid(AUTHID_CONTEXT_OPT_BITRATE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_OPT_FRAMERATE, authid_from_string(string_from_authid(AUTHID_CONTEXT_OPT_FRAMERATE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_OPT_QSCALE, authid_from_string(string_from_authid(AUTHID_CONTEXT_OPT_QSCALE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_OPT_WIDTH, authid_from_string(string_from_authid(AUTHID_CONTEXT_OPT_WIDTH)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_OPT_HEIGHT, authid_from_string(string_from_authid(AUTHID_CONTEXT_OPT_HEIGHT)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_OPT_BPP, authid_from_string(string_from_authid(AUTHID_CONTEXT_OPT_BPP)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_AUTHENTICATED, authid_from_string(string_from_authid(AUTHID_CONTEXT_AUTHENTICATED)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_SELECTOR, authid_from_string(string_from_authid(AUTHID_CONTEXT_SELECTOR)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_KEEPALIVE, authid_from_string(string_from_authid(AUTHID_CONTEXT_KEEPALIVE)));
   BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("8899676"));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_DISPLAY_MESSAGE, authid_from_string(string_from_authid(AUTHID_CONTEXT_DISPLAY_MESSAGE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_ACCEPT_MESSAGE, authid_from_string(string_from_authid(AUTHID_CONTEXT_ACCEPT_MESSAGE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_MODE_CONSOLE, authid_from_string(string_from_authid(AUTHID_CONTEXT_MODE_CONSOLE)));
   BOOST_CHECK_EQUAL(AUTHID_CONTEXT_TIMEZONE, authid_from_string(string_from_authid(AUTHID_CONTEXT_TIMEZONE)));
}


BOOST_AUTO_TEST_CASE(TestConfigSet)
{
    Inifile             ini;

    cfg::crypto::key0::type akey{{}};
    BOOST_CHECK(ini.get<cfg::crypto::key0>() != akey);

    unsigned char ckey[32]{1};
    ini.set<cfg::crypto::key0>(ckey);
    akey[0] = 1;
    BOOST_CHECK(ini.get<cfg::crypto::key0>() == akey);

    unsigned char const cckey[32]{1, 1};
    ini.set<cfg::crypto::key0>(cckey);
    akey[1] = 1;
    BOOST_CHECK(ini.get<cfg::crypto::key0>() == akey);

    akey[2] = 1;
    ini.set<cfg::crypto::key0>(akey);
    BOOST_CHECK(ini.get<cfg::crypto::key0>() == akey);
}


BOOST_AUTO_TEST_CASE(TestConfigNotifications)
{
    Inifile             ini;

    // nothing has been changed initialy
    BOOST_CHECK(!ini.check_from_acl());

    // auth_user has been changed, so check_from_acl() method will notify that something changed
    ini.get_acl_field(static_cast<authid_t>(cfg::globals::auth_user::index())).set(cstr_array_view("someoneelse"));
    BOOST_CHECK(ini.check_from_acl());
    BOOST_CHECK_EQUAL("someoneelse", ini.get<cfg::globals::auth_user>());

    ini.clear_send_index();
    BOOST_CHECK(!ini.check_from_acl());

    // setting a field without changing it should not notify that something changed
    ini.set_acl<cfg::globals::auth_user>("someoneelse");
    BOOST_CHECK(!ini.check_from_acl());


    // Using the list of changed fields:
    ini.set_acl<cfg::globals::auth_user>("someuser");
    ini.set_acl<cfg::globals::host>("35.53.0.1");
    ini.set_acl<cfg::context::opt_height>(602);
    ini.set_acl<cfg::globals::target>("35.53.0.2");
    BOOST_CHECK(!ini.check_from_acl());

    auto list = ini.get_fields_changed();
    {
        std::size_t distance = 0;
        for (auto && x : list) {
            (void)x;
            ++distance;
        }
        BOOST_CHECK_EQUAL(4, distance);
    }

    for (auto && var : list) {
        BOOST_CHECK(
            var.authid() == cfg::globals::auth_user::index()
         || var.authid() == cfg::globals::host::index()
         || var.authid() == cfg::context::opt_height::index()
         || var.authid() == cfg::globals::target::index()
        );
    }
    ini.clear_send_index();
    BOOST_CHECK(!ini.check_from_acl());
}
