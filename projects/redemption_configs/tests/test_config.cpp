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

#define RED_TEST_MODULE TestConfig
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/config.hpp"
#include <sstream>
#include <fstream>

template<class Ch, class Tr, class E>
typename std::enable_if<
    std::is_enum<E>::value,
    std::basic_ostream<Ch, Tr>&
>::type
operator<<(std::basic_ostream<Ch, Tr> & out, E const & e)
{
    return out << +underlying_cast(e); // '+' for transform u8/s8 to int
}

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

RED_AUTO_TEST_CASE(TestConfigFromFile)
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
    configuration_load(ini.configuration_holder(), "/tmp/tmp-rdpproxy.ini");
    RED_CHECK_EQUAL("/tmp/raw/movie/",       ini.get<cfg::globals::wrm_path>());
    RED_CHECK_EQUAL("./tmp/raw/persistent/", ini.get<cfg::globals::persistent_path>());
}

RED_AUTO_TEST_CASE(TestConfigDefaultEmpty)
{
    // default config
    Inifile             ini;

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                      ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::localfile_hashed,
                                                        ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>().data(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    RED_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>().data(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(40000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_up>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_height>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::opt_width>());

    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::context::auth_error_message>());

    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_current_page>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_device_filter>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_group_filter>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::selector_lines_per_page>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::context::selector>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::context::selector_current_page>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::context::selector_device_filter>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::context::selector_group_filter>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::context::selector_lines_per_page>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::context::selector_number_of_pages>());

    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target_device>());
    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_password>());
    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_port>());
    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::target_protocol>());
    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::target_user>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::host>());
    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::globals::target>());

    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::globals::auth_user>());
    RED_CHECK_EQUAL(true,                             ini.is_asked<cfg::context::password>());


    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::context::password>());

    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::auth_channel_target>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_answer>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::context::auth_channel_target>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::context::message>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::context::accept_message>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::context::display_message>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::context::rejected>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::context::authenticated>());


    RED_CHECK_EQUAL(false,                            ini.is_asked<cfg::context::keepalive>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::context::keepalive>());


    RED_CHECK_EQUAL("",                               ini.get<cfg::context::session_id>());


    RED_CHECK_EQUAL(0,                                ini.get<cfg::context::end_date_cnx>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::context::end_time>());

    RED_CHECK_EQUAL(RdpModeConsole::allow,            ini.get<cfg::context::mode_console>());
    RED_CHECK_EQUAL(-3600,                            ini.get<cfg::context::timezone>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::context::real_target_device>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::context::authentication_challenge>());
}


RED_AUTO_TEST_CASE(TestConfig1)
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
        "session_probe_enable_launch_mask=false\n"
        "session_probe_launch_timeout=0\n"
        "session_probe_keepalive_timeout=0\n"
        "\n"
        "[mod_vnc]\n"
        "clipboard_up=yes\n"
        "encodings=16,2,0,1,-239\n"
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
    configuration_load(ini.configuration_holder(), oss);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL("/var/tmp/wab/persistent/rdp/",   ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                      ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL("/mnt/wab/hash/",                 ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL("/mnt/wab/recorded/rdp/",         ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL("/mnt/tmp/wab/recorded/rdp/",     ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(150,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("192.168.1.1",                    ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("redemption",                     ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>().data(),
                                                               "\x00\x11\x22\x33\x44\x55\x66\x77"
                                                               "\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF"
                                                               "\x00\x11\x22\x33\x44\x55\x66\x77"
                                                               "\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF", 32));
    RED_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>().data(),
                                                               "\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88"
                                                               "\x77\x66\x55\x44\x33\x22\x11\x00"
                                                               "\xFF\xEE\xDD\xCC\xBB\xAA\x99\x88"
                                                               "\x77\x66\x55\x44\x33\x22\x11\x00", 32));

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(256,                              ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(128,                              ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp4,             ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(45,                               ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("22",                             ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("audin",                          ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("C:\\WINDOWS\\NOTEPAD.EXE",       ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("C:\\WINDOWS\\",                  ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_vnc::clipboard_up>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_vnc::clipboard_down>());
    RED_CHECK_EQUAL("16,2,0,1,-239",                  ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());
}

RED_AUTO_TEST_CASE(TestConfig1bis)
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
    configuration_load(ini.configuration_holder(), oss);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::medium,                    ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                      ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL("/mnt/wab/hash/",                 ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL("/mnt/wab/recorded/rdp/",         ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL("/mnt/tmp/wab/recorded/rdp/",     ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,         ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::wrm,          ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::localfile,             ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key0>().data(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));
    RED_CHECK_EQUAL(0,                                memcmp(ini.get<cfg::crypto::key1>().data(),
                                                               "\x00\x01\x02\x03\x04\x05\x06\x07"
                                                               "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                                                               "\x10\x11\x12\x13\x14\x15\x16\x17"
                                                               "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F", 32));

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::none,             ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth8,               ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp5,             ini.get<cfg::mod_rdp::rdp_compression>()); RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("*docs",                          ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(3000,                             ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(6000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::utf8,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::duplicated,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(1,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());
}

RED_AUTO_TEST_CASE(TestConfig2)
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
    configuration_load(ini.configuration_holder(), oss);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::high,                      ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                        ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("rdpproxy",                       ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::none,             ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE",
                      ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(40000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());
}

RED_AUTO_TEST_CASE(TestConfig3)
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

    configuration_load(ini.configuration_holder(), oss);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::high,                      ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                        ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::none,          ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(7,                                ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(300,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("127.0.0.1",                      ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("rdpproxy RDP",                   ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL("/var/tmp/wab/recorded/rdp/",     ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL("/var/wab/recorded/rdp/",         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(7,                                ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(1,                                ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::none,             ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE",
                      ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(6000,                             ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(3000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::continued,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());
}

RED_AUTO_TEST_CASE(TestMultiple)
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
    configuration_load(ini.configuration_holder(), oss);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                        ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::localfile,             ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("redemption",                     ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("%HOMEDRIVE%%HOMEPATH%",          ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(40000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());


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
    configuration_load(ini.configuration_holder(), oss2);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3390,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                        ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::cryptofile,            ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("192.168.1.1",                    ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(3,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(3,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(0x28,                              ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("docs,apps",                      ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("%HOMEDRIVE%%HOMEPATH%",          ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(4000,                             ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::ignore_and_continue,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(7000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());
}

RED_AUTO_TEST_CASE(TestNewConf)
{
    // new behavior:
    // init() load default values from main configuration file
    // - options with multiple occurences get the last value
    // - unrecognized lines are ignored
    // - every characters following # are ignored until end of line (comments)
    Inifile             ini;

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());


    // TODO video related values should go to [video] section
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                        ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(120,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::localfile_hashed,
                                                        ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(40000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());

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

    configuration_load(ini.configuration_holder(), ifs2);

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::capture_chunk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::is_rec>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::movie_path>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::auth_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::host>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::glyph_cache>());
    RED_CHECK_EQUAL(3389,                             ini.get<cfg::globals::port>());
    RED_CHECK_EQUAL(Level::low,                       ini.get<cfg::globals::encryptionLevel>());
    RED_CHECK_EQUAL("/tmp/redemption-sesman-sock",    ini.get<cfg::globals::authfile>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::nomouse>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::notimestamp>());
    RED_CHECK_EQUAL(to_string_path(PERSISTENT_PATH),  ini.get<cfg::globals::persistent_path>());

    RED_CHECK_EQUAL((CaptureFlags::png | CaptureFlags::wrm | CaptureFlags::ocr),
                                                        ini.get<cfg::video::capture_flags>());
    RED_CHECK_EQUAL(10,                               ini.get<cfg::video::png_interval>().count());
    RED_CHECK_EQUAL(40,                               ini.get<cfg::video::frame_interval>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::video::break_interval>().count());

    RED_CHECK_EQUAL(5,                                ini.get<cfg::video::png_limit>());

    RED_CHECK_EQUAL(to_string_path(HASH_PATH),        ini.get<cfg::video::hash_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_PATH),      ini.get<cfg::video::record_path>());
    RED_CHECK_EQUAL(to_string_path(RECORD_TMP_PATH),  ini.get<cfg::video::record_tmp_path>());

    RED_CHECK_EQUAL(KeyboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_keyboard_log>());
    RED_CHECK_EQUAL(ClipboardLogFlags::syslog,
                                                        ini.get<cfg::video::disable_clipboard_log>());
    RED_CHECK_EQUAL(FileSystemLogFlags::syslog,
                                                        ini.get<cfg::video::disable_file_system_log>());

    RED_CHECK_EQUAL(ColorDepthSelectionStrategy::depth16,
                                                        ini.get<cfg::video::wrm_color_depth_selection_strategy>());
    RED_CHECK_EQUAL(WrmCompressionAlgorithm::gzip,
                                                        ini.get<cfg::video::wrm_compression_algorithm>());

    RED_CHECK_EQUAL(10,                               ini.get<cfg::globals::handshake_timeout>().count());
    RED_CHECK_EQUAL(900,                              ini.get<cfg::globals::session_timeout>().count());
    RED_CHECK_EQUAL(30,                               ini.get<cfg::globals::keepalive_grace_delay>().count());
    RED_CHECK_EQUAL(300,                              ini.get<cfg::globals::authentication_timeout>().count());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::globals::close_timeout>().count());

    RED_CHECK_EQUAL("/tmp/",                          ini.get<cfg::video::replay_path>());

    RED_CHECK_EQUAL(TraceType::localfile_hashed,
                                                        ini.get<cfg::globals::trace_type>());
    RED_CHECK_EQUAL("0.0.0.0",                        ini.get<cfg::globals::listen_address>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::globals::enable_transparent_mode>());
    RED_CHECK_EQUAL("inquisition",                    ini.get<cfg::globals::certificate_password>());

    RED_CHECK_EQUAL(to_string_path(PNG_PATH),         ini.get<cfg::globals::png_path>());
    RED_CHECK_EQUAL(to_string_path(WRM_PATH),         ini.get<cfg::globals::wrm_path>());

    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_bitmap_update>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_close_box>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::globals::enable_osd_display_remote_target>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::x224>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mcs>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::sec>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::primary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::secondary_orders>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::bitmap>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::capture>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::auth>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::session>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::front>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_rdp>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_vnc>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_internal>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::mod_xup>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::widget>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::input>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::password>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::compression>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::debug::cache>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::keyboard_layout>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::ignore_logon_password>());
    RED_CHECK_EQUAL(0x80,                             ini.get<cfg::client::performance_flags_default>());
    RED_CHECK_EQUAL(0x28,                             ini.get<cfg::client::performance_flags_force_present>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::client::performance_flags_force_not_present>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::tls_support>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::tls_fallback_legacy>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bogus_neg_request>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::bogus_user_id>());
    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::client::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::disable_tsk_switch_shortcuts>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::client::max_color_depth>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::client::bitmap_compression>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::client::fast_path>());

    RED_CHECK_EQUAL(RdpCompression::rdp6_1,           ini.get<cfg::mod_rdp::rdp_compression>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::enable_nla>());
    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_rdp::open_session_timeout>().count());
    RED_CHECK_EQUAL("15,16,17,18,22",                 ini.get<cfg::mod_rdp::extra_orders>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::cache_waiting_list>());
    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>());
    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::allow_channels>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::deny_channels>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::fast_path>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::bogus_sc_net_size>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::proxy_managed_drives>());

    RED_CHECK_EQUAL("*",                              ini.get<cfg::mod_rdp::auth_channel>());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::alternate_shell>());
    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_rdp::shell_working_directory>());

    RED_CHECK_EQUAL(false,                            ini.get<cfg::mod_rdp::enable_session_probe>());
    RED_CHECK_EQUAL(true,                             ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>());
    RED_CHECK_EQUAL(40000,                            ini.get<cfg::mod_rdp::session_probe_launch_timeout>().count());
    RED_CHECK_EQUAL(SessionProbeOnLaunchFailure::retry_without_session_probe,
                                                        ini.get<cfg::mod_rdp::session_probe_on_launch_failure>());
    RED_CHECK_EQUAL(5000,                             ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>().count());

    RED_CHECK_EQUAL("",                               ini.get<cfg::mod_vnc::encodings>());
    RED_CHECK_EQUAL(ClipboardEncodingType::latin1,          ini.get<cfg::mod_vnc::server_clipboard_encoding_type>());
    RED_CHECK_EQUAL(VncBogusClipboardInfiniteLoop::delayed,
                                                        ini.get<cfg::mod_vnc::bogus_clipboard_infinite_loop>());

    RED_CHECK_EQUAL(0,                                ini.get<cfg::mod_replay::on_end_of_data>());

    RED_CHECK_EQUAL(40000,                            ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(5,                                ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(15,                               ini.get<cfg::context::opt_qscale>());
    RED_CHECK_EQUAL(800,                              ini.get<cfg::context::opt_width>());
    RED_CHECK_EQUAL(600,                              ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(ColorDepth::depth24,              ini.get<cfg::context::opt_bpp>());
}

RED_AUTO_TEST_CASE(TestConfigTools)
{
    using namespace configs;

    {
        unsigned u;
        spec_type<unsigned> stype;

        RED_CHECK(!parse(u, stype, cstr_array_view("")));
        RED_CHECK_EQUAL(0, u);

        RED_CHECK(!parse(u, stype, cstr_array_view("0")));
        RED_CHECK_EQUAL(0, u);
        RED_CHECK(!parse(u, stype, cstr_array_view("0x")));
        RED_CHECK_EQUAL(0, u);

        RED_CHECK(!parse(u, stype, cstr_array_view("3")));
        RED_CHECK_EQUAL(3, u);
        RED_CHECK(!parse(u, stype, cstr_array_view("0x3")));
        RED_CHECK_EQUAL(3, u);

        RED_CHECK(!parse(u, stype, cstr_array_view("0x00000007")));
        RED_CHECK_EQUAL(7, u);
        RED_CHECK(!parse(u, stype, cstr_array_view("0x0000000000000007")));
        RED_CHECK_EQUAL(7, u);
        RED_CHECK(!parse(u, stype, cstr_array_view("0x0007")));
        RED_CHECK_EQUAL(7, u);

        RED_CHECK(!parse(u, stype, cstr_array_view("1357")));
        RED_CHECK_EQUAL(1357, u);
        RED_CHECK(!parse(u, stype, cstr_array_view("0x1357")));
        RED_CHECK_EQUAL(0x1357, u);

        RED_CHECK(!parse(u, stype, cstr_array_view("0x0A")));
        RED_CHECK_EQUAL(0x0a, u);
        RED_CHECK(!parse(u, stype, cstr_array_view("0x0a")));
        RED_CHECK_EQUAL(0x0a, u);

        RED_CHECK(!!parse(u, stype, cstr_array_view("0x0000000I")));
        RED_CHECK(!!parse(u, stype, cstr_array_view("I")));

    }

    {
        Level level;
        spec_type<Level> stype;

        RED_CHECK(!parse(level, stype, cstr_array_view("LoW")));
        RED_CHECK_EQUAL(Level::low, level);

        RED_CHECK(!parse(level, stype, cstr_array_view("mEdIuM")));
        RED_CHECK_EQUAL(Level::medium, level);

        RED_CHECK(!parse(level, stype, cstr_array_view("High")));
        RED_CHECK_EQUAL(Level::high, level);

        RED_CHECK(!!parse(level, stype, cstr_array_view("dsifsu")));
    }

    {
        int i;
        spec_type<int> stype;

        RED_CHECK(!parse(i, stype, cstr_array_view("3600")));
        RED_CHECK_EQUAL(3600, i);
        RED_CHECK(!parse(i, stype, cstr_array_view("0")));
        RED_CHECK_EQUAL(0, i);
        RED_CHECK(!parse(i, stype, cstr_array_view("")));
        RED_CHECK_EQUAL(0, i);
        RED_CHECK(!parse(i, stype, cstr_array_view("-3600")));
        RED_CHECK_EQUAL(-3600, i);
    }
}

RED_AUTO_TEST_CASE(TestLogPolicy)
{
    Inifile ini;
    RED_CHECK(ini.get_acl_field(cfg::globals::auth_user::index).is_loggable());
    RED_CHECK(!ini.get_acl_field(cfg::globals::target_application_password::index).is_loggable());
    RED_CHECK(ini.get_acl_field(cfg::context::auth_channel_answer::index).is_loggable());
    ini.set<cfg::context::auth_channel_answer>("blah blah password blah blah");
    RED_CHECK(!ini.get_acl_field(cfg::context::auth_channel_answer::index).is_loggable());
}

RED_AUTO_TEST_CASE(TestContextSetValue)
{
    Inifile             ini;

    // bitrate, framerate, qscale
    ini.get_acl_field(cfg::context::opt_bitrate::index).set(cstr_array_view("80000"));
    ini.get_acl_field(cfg::context::opt_framerate::index).set(cstr_array_view("6"));
    ini.get_acl_field(cfg::context::opt_qscale::index).set(cstr_array_view("16"));

    RED_CHECK_EQUAL(80000, ini.get<cfg::context::opt_bitrate>());
    RED_CHECK_EQUAL(6,     ini.get<cfg::context::opt_framerate>());
    RED_CHECK_EQUAL(16,    ini.get<cfg::context::opt_qscale>());

    RED_CHECK_EQUAL("80000", ini.get_acl_field(cfg::context::opt_bitrate::index).to_string_view().data());
    RED_CHECK_EQUAL("6",     ini.get_acl_field(cfg::context::opt_framerate::index).to_string_view().data());
    RED_CHECK_EQUAL("16",    ini.get_acl_field(cfg::context::opt_qscale::index).to_string_view().data());


    // bpp, height, width
    ini.get_acl_field(cfg::context::opt_bpp::index).ask();
    ini.get_acl_field(cfg::context::opt_height::index).ask();
    ini.get_acl_field(cfg::context::opt_width::index).ask();

    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::opt_height>());
    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::opt_width>());

    ini.get_acl_field(cfg::context::opt_bpp::index).set(cstr_array_view("16"));
    ini.get_acl_field(cfg::context::opt_height::index).set(cstr_array_view("1024"));
    ini.get_acl_field(cfg::context::opt_width::index).set(cstr_array_view("1280"));

    RED_CHECK_EQUAL(false, ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(false, ini.is_asked<cfg::context::opt_height>());
    RED_CHECK_EQUAL(false, ini.is_asked<cfg::context::opt_width>());

    RED_CHECK_EQUAL(ColorDepth::depth16, ini.get<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(1024, ini.get<cfg::context::opt_height>());
    RED_CHECK_EQUAL(1280, ini.get<cfg::context::opt_width>());

    RED_CHECK_EQUAL("16",   ini.get_acl_field(cfg::context::opt_bpp::index).to_string_view().data());
    RED_CHECK_EQUAL("1024", ini.get_acl_field(cfg::context::opt_height::index).to_string_view().data());
    RED_CHECK_EQUAL("1280", ini.get_acl_field(cfg::context::opt_width::index).to_string_view().data());


    // selector, ...
    ini.get_acl_field(cfg::context::selector::index).ask();
    ini.get_acl_field(cfg::context::selector_current_page::index).ask();
    ini.get_acl_field(cfg::context::selector_device_filter::index).ask();
    ini.get_acl_field(cfg::context::selector_group_filter::index).ask();
    ini.get_acl_field(cfg::context::selector_lines_per_page::index).ask();

    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector>());
    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_current_page>());
    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_device_filter>());
    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_group_filter>());
    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::selector_lines_per_page>());

    ini.get_acl_field(cfg::context::selector::index).set(cstr_array_view("True"));
    ini.get_acl_field(cfg::context::selector_current_page::index).set(cstr_array_view("2"));
    ini.get_acl_field(cfg::context::selector_device_filter::index).set(cstr_array_view("Windows"));
    ini.get_acl_field(cfg::context::selector_group_filter::index).set(cstr_array_view("RDP"));
    ini.get_acl_field(cfg::context::selector_lines_per_page::index).set(cstr_array_view("25"));
    ini.get_acl_field(cfg::context::selector_number_of_pages::index).set(cstr_array_view("2"));

    RED_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector>());
    RED_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_current_page>());
    RED_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_device_filter>());
    RED_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_group_filter>());
    RED_CHECK_EQUAL(false,     ini.is_asked<cfg::context::selector_lines_per_page>());

    RED_CHECK_EQUAL(true,      ini.get<cfg::context::selector>());
    RED_CHECK_EQUAL(2,         ini.get<cfg::context::selector_current_page>());
    RED_CHECK_EQUAL("Windows", ini.get<cfg::context::selector_device_filter>());
    RED_CHECK_EQUAL("RDP",     ini.get<cfg::context::selector_group_filter>());
    RED_CHECK_EQUAL(25,        ini.get<cfg::context::selector_lines_per_page>());
    RED_CHECK_EQUAL(2,         ini.get<cfg::context::selector_number_of_pages>());

    RED_CHECK_EQUAL("True",    ini.get_acl_field(cfg::context::selector::index).to_string_view().data());
    RED_CHECK_EQUAL("2",       ini.get_acl_field(cfg::context::selector_current_page::index).to_string_view().data());
    RED_CHECK_EQUAL("Windows", ini.get_acl_field(cfg::context::selector_device_filter::index).to_string_view().data());
    RED_CHECK_EQUAL("RDP",     ini.get_acl_field(cfg::context::selector_group_filter::index).to_string_view().data());
    RED_CHECK_EQUAL("25",      ini.get_acl_field(cfg::context::selector_lines_per_page::index).to_string_view().data());
    RED_CHECK_EQUAL("2",       ini.get_acl_field(cfg::context::selector_number_of_pages::index).to_string_view().data());


    // target_xxxx
    ini.get_acl_field(cfg::globals::target_device::index).set(cstr_array_view("127.0.0.1"));
    ini.get_acl_field(cfg::context::target_password::index).set(cstr_array_view("12345678"));
    ini.get_acl_field(cfg::context::target_port::index).set(cstr_array_view("3390"));
    ini.get_acl_field(cfg::context::target_protocol::index).set(cstr_array_view("RDP"));
    ini.get_acl_field(cfg::globals::target_user::index).set(cstr_array_view("admin"));
    ini.get_acl_field(cfg::globals::target_application::index).set(cstr_array_view("wallix@putty"));

    RED_CHECK_EQUAL(false,          ini.is_asked<cfg::globals::target_device>());
    RED_CHECK_EQUAL(false,          ini.is_asked<cfg::context::target_password>());
    RED_CHECK_EQUAL(false,          ini.is_asked<cfg::context::target_port>());
    RED_CHECK_EQUAL(false,          ini.is_asked<cfg::context::target_protocol>());
    RED_CHECK_EQUAL(false,          ini.is_asked<cfg::globals::target_user>());

    RED_CHECK_EQUAL("127.0.0.1",    ini.get<cfg::globals::target_device>());
    RED_CHECK_EQUAL("12345678",     ini.get<cfg::context::target_password>());
    RED_CHECK_EQUAL(3390,           ini.get<cfg::context::target_port>());
    RED_CHECK_EQUAL("RDP",          ini.get<cfg::context::target_protocol>());
    RED_CHECK_EQUAL("admin",        ini.get<cfg::globals::target_user>());
    RED_CHECK_EQUAL("wallix@putty", ini.get<cfg::globals::target_application>());

    RED_CHECK_EQUAL("127.0.0.1",    ini.get_acl_field(cfg::globals::target_device::index).to_string_view().data());
    RED_CHECK_EQUAL("12345678",     ini.get_acl_field(cfg::context::target_password::index).to_string_view().data());
    RED_CHECK_EQUAL("3390",         ini.get_acl_field(cfg::context::target_port::index).to_string_view().data());
    RED_CHECK_EQUAL("RDP",          ini.get_acl_field(cfg::context::target_protocol::index).to_string_view().data());
    RED_CHECK_EQUAL("admin",        ini.get_acl_field(cfg::globals::target_user::index).to_string_view().data());
    RED_CHECK_EQUAL("wallix@putty", ini.get_acl_field(cfg::globals::target_application::index).to_string_view().data());


    // host
    ini.get_acl_field(cfg::globals::host::index).ask();

    RED_CHECK_EQUAL(true, ini.is_asked<cfg::globals::host>());

    ini.get_acl_field(cfg::globals::host::index).set(cstr_array_view("127.0.0.1"));

    RED_CHECK_EQUAL(false,       ini.is_asked<cfg::globals::host>());

    RED_CHECK_EQUAL("127.0.0.1", ini.get<cfg::globals::host>());

    RED_CHECK_EQUAL("127.0.0.1", ini.get_acl_field(cfg::globals::host::index).to_string_view().data());
    RED_CHECK_EQUAL(9,           ini.get_acl_field(cfg::globals::host::index).to_string_view().size());


    // target
    ini.get_acl_field(cfg::globals::target::index).ask();

    RED_CHECK_EQUAL(true, ini.is_asked<cfg::globals::target>());

    ini.get_acl_field(cfg::globals::target::index).set(cstr_array_view("192.168.0.1"));

    RED_CHECK_EQUAL(false,         ini.is_asked<cfg::globals::target>());

    RED_CHECK_EQUAL("192.168.0.1", ini.get<cfg::globals::target>());

    RED_CHECK_EQUAL("192.168.0.1", ini.get_acl_field(cfg::globals::target::index).to_string_view().data());


    // auth_user
    ini.get_acl_field(cfg::globals::auth_user::index).set(cstr_array_view("admin"));

    RED_CHECK_EQUAL(false,   ini.is_asked<cfg::globals::auth_user>());

    RED_CHECK_EQUAL("admin", ini.get<cfg::globals::auth_user>());

    RED_CHECK_EQUAL("admin", ini.get_acl_field(cfg::globals::auth_user::index).to_string_view().data());


    // password
    ini.get_acl_field(cfg::context::password::index).ask();

    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::password>());

    ini.get_acl_field(cfg::context::password::index).set(cstr_array_view("12345678"));

    RED_CHECK_EQUAL(false,      ini.is_asked<cfg::context::password>());

    RED_CHECK_EQUAL("12345678", ini.get<cfg::context::password>());

    RED_CHECK_EQUAL("12345678", ini.get_acl_field(cfg::context::password::index).to_string_view().data());


    // answer
    ini.get_acl_field(cfg::context::auth_channel_answer::index).set(cstr_array_view("answer"));

    RED_CHECK_EQUAL("answer", ini.get<cfg::context::auth_channel_answer>());


    // authchannel_target
    ini.get_acl_field(cfg::context::auth_channel_target::index).ask();

    RED_CHECK_EQUAL(true, ini.is_asked<cfg::context::auth_channel_target>());

    ini.get_acl_field(cfg::context::auth_channel_target::index).set(cstr_array_view("target"));

    RED_CHECK_EQUAL(false, 	ini.is_asked<cfg::context::auth_channel_target>());

    RED_CHECK_EQUAL("target", ini.get<cfg::context::auth_channel_target>());

    RED_CHECK_EQUAL("target", ini.get_acl_field(cfg::context::auth_channel_target::index).to_string_view().data());


    // message
    ini.get_acl_field(cfg::context::message::index).set(cstr_array_view("message"));

    RED_CHECK_EQUAL("message", ini.get<cfg::context::message>());


    // rejected
    ini.get_acl_field(cfg::context::rejected::index).set(cstr_array_view("rejected"));

    RED_CHECK_EQUAL("rejected", ini.get<cfg::context::rejected>());

    RED_CHECK_EQUAL("rejected", ini.get_acl_field(cfg::context::rejected::index).to_string_view().data());


    // authenticated
    ini.get_acl_field(cfg::context::authenticated::index).set(cstr_array_view("True"));

    RED_CHECK_EQUAL(true,   ini.get<cfg::context::authenticated>());

    RED_CHECK_EQUAL("True", ini.get_acl_field(cfg::context::authenticated::index).to_string_view().data());


    // keepalive
    ini.get_acl_field(cfg::context::keepalive::index).set(cstr_array_view("True"));

    RED_CHECK_EQUAL(false, ini.is_asked<cfg::context::keepalive>());

    RED_CHECK_EQUAL(true,  ini.get<cfg::context::keepalive>());


    // session_id
    ini.get_acl_field(cfg::context::session_id::index).set(cstr_array_view("0123456789"));

    RED_CHECK_EQUAL("0123456789", ini.get<cfg::context::session_id>());


    // end_date_cnx
    ini.get_acl_field(cfg::context::end_date_cnx::index).set(cstr_array_view("12345678"));

    RED_CHECK_EQUAL(12345678, ini.get<cfg::context::end_date_cnx>());


    // end_time
    ini.get_acl_field(cfg::context::end_time::index).set(cstr_array_view("end_time"));

    RED_CHECK_EQUAL("end_time", ini.get<cfg::context::end_time>());


    // mode_console
    ini.get_acl_field(cfg::context::mode_console::index).set(cstr_array_view("forbid"));

    RED_CHECK_EQUAL(RdpModeConsole::forbid, ini.get<cfg::context::mode_console>());


    // timezone
    ini.get_acl_field(cfg::context::timezone::index).set(cstr_array_view("-7200"));

    RED_CHECK_EQUAL(-7200, ini.get<cfg::context::timezone>());


    // real_target_device
    ini.get_acl_field(cfg::context::real_target_device::index).set(cstr_array_view("10.0.0.1"));

    RED_CHECK_EQUAL("10.0.0.1", ini.get<cfg::context::real_target_device>());

    RED_CHECK_EQUAL("10.0.0.1", ini.get_acl_field(cfg::context::real_target_device::index).to_string_view().data());


    // authentication_challenge
    ini.get_acl_field(cfg::context::authentication_challenge::index).set(cstr_array_view("true"));

    RED_CHECK_EQUAL(true, ini.get<cfg::context::authentication_challenge>());
}


RED_AUTO_TEST_CASE(TestAuthentificationKeywordRecognition)
{
   RED_CHECK_EQUAL(MAX_AUTHID, authid_from_string("unknown"_av));
   RED_CHECK_SMEM("target_login"_av, string_from_authid(cfg::globals::target_user::index));
   RED_CHECK_EQUAL(cfg::globals::target_user::index, authid_from_string("target_login"_av));
   RED_CHECK_EQUAL(cfg::context::target_password::index, authid_from_string(string_from_authid(cfg::context::target_password::index)));
   RED_CHECK_EQUAL(cfg::globals::host::index, authid_from_string(string_from_authid(cfg::globals::host::index)));
   RED_CHECK_EQUAL(cfg::context::password::index, authid_from_string(string_from_authid(cfg::context::password::index)));
   RED_CHECK_EQUAL(cfg::globals::auth_user::index, authid_from_string(string_from_authid(cfg::globals::auth_user::index)));
   RED_CHECK_EQUAL(cfg::globals::target_device::index, authid_from_string(string_from_authid(cfg::globals::target_device::index)));
   RED_CHECK_EQUAL(cfg::context::target_port::index, authid_from_string(string_from_authid(cfg::context::target_port::index)));
   RED_CHECK_EQUAL(cfg::context::target_protocol::index, authid_from_string(string_from_authid(cfg::context::target_protocol::index)));
   RED_CHECK_EQUAL(cfg::context::rejected::index, authid_from_string(string_from_authid(cfg::context::rejected::index)));
   RED_CHECK_EQUAL(cfg::context::message::index, authid_from_string(string_from_authid(cfg::context::message::index)));
   RED_CHECK_EQUAL(cfg::context::opt_width::index, authid_from_string(string_from_authid(cfg::context::opt_width::index)));
   RED_CHECK_EQUAL(cfg::context::opt_height::index, authid_from_string(string_from_authid(cfg::context::opt_height::index)));
   RED_CHECK_EQUAL(cfg::context::opt_bpp::index, authid_from_string(string_from_authid(cfg::context::opt_bpp::index)));
   RED_CHECK_EQUAL(cfg::context::authenticated::index, authid_from_string(string_from_authid(cfg::context::authenticated::index)));
   RED_CHECK_EQUAL(cfg::context::selector::index, authid_from_string(string_from_authid(cfg::context::selector::index)));
   RED_CHECK_EQUAL(cfg::context::keepalive::index, authid_from_string(string_from_authid(cfg::context::keepalive::index)));
}


RED_AUTO_TEST_CASE(TestConfigSet)
{
    Inifile             ini;

    cfg::crypto::key0::type akey{{}};
    RED_CHECK(ini.get<cfg::crypto::key0>() != akey);

    unsigned char ckey[32]{1};
    ini.set<cfg::crypto::key0>(ckey);
    akey[0] = 1;
    RED_CHECK(ini.get<cfg::crypto::key0>() == akey);

    unsigned char const cckey[32]{1, 1};
    ini.set<cfg::crypto::key0>(cckey);
    akey[1] = 1;
    RED_CHECK(ini.get<cfg::crypto::key0>() == akey);

    akey[2] = 1;
    ini.set<cfg::crypto::key0>(akey);
    RED_CHECK(ini.get<cfg::crypto::key0>() == akey);
}


RED_AUTO_TEST_CASE(TestConfigNotifications)
{
    Inifile             ini;

    // nothing has been changed initialy
    RED_CHECK(!ini.check_from_acl());

    // auth_user has been changed, so check_from_acl() method will notify that something changed
    ini.get_acl_field(cfg::globals::auth_user::index).set(cstr_array_view("someoneelse"));
    RED_CHECK(ini.check_from_acl());
    RED_CHECK_EQUAL("someoneelse", ini.get<cfg::globals::auth_user>());

    ini.clear_send_index();
    RED_CHECK(!ini.check_from_acl());

    // setting a field without changing it should not notify that something changed
    ini.set_acl<cfg::globals::auth_user>("someoneelse");
    RED_CHECK(!ini.check_from_acl());


    // Using the list of changed fields:
    ini.set_acl<cfg::globals::auth_user>("someuser");
    ini.set_acl<cfg::globals::host>("35.53.0.1");
    ini.set_acl<cfg::context::opt_height>(602);
    ini.set_acl<cfg::globals::target>("35.53.0.2");
    RED_CHECK(!ini.check_from_acl());

    auto list = ini.get_fields_changed();
    {
        std::size_t distance = 0;
        for (auto x : list) {
            (void)x;
            ++distance;
        }
        RED_CHECK_EQUAL(4, distance);
    }

    for (auto var : list) {
        RED_CHECK((
            var.authid() == cfg::globals::auth_user::index
         || var.authid() == cfg::globals::host::index
         || var.authid() == cfg::context::opt_height::index
         || var.authid() == cfg::globals::target::index
        ));
    }
    ini.clear_send_index();
    RED_CHECK(!ini.check_from_acl());
}
