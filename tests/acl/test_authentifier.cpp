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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong
*/

// include "test_only/test_framework/redemption_unit_tests.hpp"


// include "acl/authentifier.hpp"
// include "utils/timebase.hpp"
// include "utils/genfstat.hpp"
// include "test_only/lcg_random.hpp"
// include "test_only/transport/test_transport.hpp"
// include "test_only/front/fake_front.hpp"
// include "RAIL/client_execute.hpp"
//
// inline void init_keys(CryptoContext & cctx)
// {
//     cctx.set_master_key(cstr_array_view(
//         "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
//         "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
//         "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
//         "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
//     ));
//     cctx.set_hmac_key(cstr_array_view(
//          "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
//          "\x49\x21\x57\x87\x47\x74\x08\x8a"
//          "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
//          "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
//     ));
// }

int main(){}

// Test Disabled because when called in test
// ModuleManager should not try to actually connect to target
// which is what the current module is actually doing.
// We shoudl probably provide hime with some connection provider object to avoid that

//TODO: FIX TESTS LATER

//RED_AUTO_TEST_CASE(TestAuthentifierNoKeepalive)
//{
//    BackEvent_t signal       = BACK_EVENT_NONE;
//    BackEvent_t front_signal = BACK_EVENT_NONE;

//    LCGRandom rnd1;
//    LCGTime timeobj;
//    CryptoContext cctx1;
//    init_keys(cctx1);

//    Inifile ini;

//    ini.set<cfg::globals::keepalive_grace_delay>(cfg::globals::keepalive_grace_delay::type{30});
//    ini.set<cfg::globals::session_timeout>(cfg::globals::session_timeout::type{900});

//    ClientInfo info;
//    info.keylayout             = 0x040C;
//    info.console_session       = false;
//    info.brush_cache_code      = 0;
//    info.screen_info.bpp       = BitsPerPixel{16};
//    info.screen_info.width     = 1024;
//    info.screen_info.height    = 768;
//    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
//                                 | PERF_DISABLE_FULLWINDOWDRAG
//                                 | PERF_DISABLE_MENUANIMATIONS;

//    memset(info.order_caps.orderSupport, 0xFF, sizeof(info.order_caps.orderSupport));
//    info.order_caps.orderSupportExFlags = 0xFFFF;

//    FakeFront front(info.screen_info);

//    // load font for internal pages
//    Font font = Font(app_path(AppPath::DefaultFontFile),
//        ini.get<cfg::globals::spark_view_specific_glyph_width>());;

//    // load theme for internal pages
//    auto & theme_name = ini.get<cfg::internal_mod::theme>();
//    LOG_IF(ini.get<cfg::debug::config>(), LOG_INFO, "LOAD_THEME: %s", theme_name);

//    Theme theme;
//    ::load_theme(theme, theme_name);

//    TimeBase time_base({0,0});

//    ClientExecute rail_client_execute(time_base, front.gd(), front,
//                                    info.window_list_caps,
//                                    ini.get<cfg::debug::mod_internal>() & 1);

//    ModWrapper mod_wrapper;

//    windowing_api* winapi = nullptr;
//
//    BGRPalette palette = BGRPalette::classic_332();

//    ModOSD mod_osd(mod_wrapper, front, palette, front.gd(), info, font, theme, rail_client_execute, winapi, ini);

//    Keymap2 keymap;

//    ModuleManager mm(time_base, front, front.gd(), keymap, info, winapi, mod_wrapper, rail_client_execute, mod_osd, font, theme, ini, cctx1, rnd1, timeobj);

//    auto outdata =
//        // Time: 10011
//        "\x00\x00\x01\x85"
//        "bpp\n!24\n"
//        "width\n!800\n"
//        "height\n!600\n"
//        "selector_current_page\n!1\n"
//        "selector_device_filter\n!\n"
//        "selector_group_filter\n!\n"
//        "selector_proto_filter\n!\n"
//        "selector_lines_per_page\n!0\n"
//        "target_password\nASK\n"
//        "target_host\nASK\n"
//        "proto_dest\nASK\n"
//        "password\nASK\n"
//        "reporting\n!\n"
//        "auth_channel_target\n!\n"
//        "accept_message\n!False\n"
//        "display_message\n!False\n"
//        "real_target_device\n!\n"
//        "login\nASK\n"
//        "ip_client\n!\n"
//        "ip_target\n!\n"
//        "target_device\nASK\n"
//        "target_login\nASK\n"

//        // Time: 10043
//        "\x00\x00\x00\x0E"
//        "keepalive\nASK\n"
//        ""_av
//    ;

//    auto indata =
//        "\x00\x00\x00\x8F"
//        "login\n!toto\n"
//        "password\n!totopass\n"
//        "target_device\n!win\n"
//        "target_login\n!user\n"
//        "target_password\n!whoknows\n"
//        "proto_dest\n!RDP\n"
//        "module\n!RDP\n"
//        "authenticated\n!True\n"
//        ""_av
//    ;

//    Fstat fstat;
//    CryptoContext cctx;
//    init_keys(cctx);
//    LCGRandom rnd;

//    TestTransport acl_trans(indata, outdata);
//    AclSerializer acl_serial(ini, 10010, acl_trans, cctx, rnd, fstat, AclSerializer::Verbose::variable);
//    Authentifier sesman(ini, cctx, Authentifier::Verbose::state);
//    sesman.set_acl_serial(&acl_serial);
//    signal = BACK_EVENT_NEXT;

//    bool has_activity = true;
//    // Ask next_module, send inital data to ACL
//    acl_serial.check(sesman, sesman, mm, 10011, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    // Receive answer, OK to connect
//    sesman.receive();
//    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10012, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10042, signal, front_signal,has_activity);
//    // Send keepalive=ASK
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10043, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10072, signal, front_signal,has_activity);
//    // still connected
//    RED_CHECK(not mm.last_module);
//    // If no keepalive is received after 30 seconds => disconnection
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10073, signal, front_signal,has_activity);
//    RED_CHECK(has_activity);
//    RED_CHECK(mm.last_module);
//}



//RED_AUTO_TEST_CASE(TestAuthentifierKeepalive)
//{

//    BackEvent_t signal       = BACK_EVENT_NONE;
//    BackEvent_t front_signal = BACK_EVENT_NONE;

//    Inifile ini;

//    ini.set<cfg::globals::keepalive_grace_delay>(cfg::globals::keepalive_grace_delay::type{30});
//    ini.set<cfg::globals::session_timeout>(cfg::globals::session_timeout::type{900});

//    TimeBase time_base({0,0});
//    MMIni mm(time_base, ini);

//    auto outdata =
//        // Time 10011
//        "\x00\x00\x01\x85"
//        "bpp\n!24\n"
//        "width\n!800\n"
//        "height\n!600\n"
//        "selector_current_page\n!1\n"
//        "selector_device_filter\n!\n"
//        "selector_group_filter\n!\n"
//        "selector_proto_filter\n!\n"
//        "selector_lines_per_page\n!0\n"
//        "target_password\nASK\n"
//        "target_host\nASK\n"
//        "proto_dest\nASK\n"
//        "password\nASK\n"
//        "reporting\n!\n"
//        "auth_channel_target\n!\n"
//        "accept_message\n!False\n"
//        "display_message\n!False\n"
//        "real_target_device\n!\n"
//        "login\nASK\n"
//        "ip_client\n!\n"
//        "ip_target\n!\n"
//        "target_device\nASK\n"
//        "target_login\nASK\n"

//        // Time 10043
//        "\x00\x00\x00\x0E"
//        "keepalive\nASK\n"

//        "\x00\x00\x00\x0E"
//        "keepalive\nASK\n"

//        ""_av
//    ;

//    auto indata =
//        "\x00\x00\x00\x8F"
//        "login\n!toto\n"
//        "password\n!totopass\n"
//        "target_device\n!win\n"
//        "target_login\n!user\n"
//        "target_password\n!whoknows\n"
//        "proto_dest\n!RDP\n"
//        "authenticated\n!True\n"
//        "module\n!RDP\n"

//        // Time 10045
//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        // Time 10072 : bad message
//        "\x00\x00\x00\x10"
//        "koopalive\n!True\n"

//        ""_av
//    ;

//    Fstat fstat;
//    LCGRandom rnd;
//    CryptoContext cctx;
//    init_keys(cctx);

//    TestTransport acl_trans(indata, outdata);
//    AclSerializer acl_serial(ini, 10010, acl_trans, cctx, rnd, fstat, to_verbose_flags(0));
//    Authentifier sesman(ini, cctx, Authentifier::Verbose(0));
//    sesman.set_acl_serial(&acl_serial);
//    signal = BACK_EVENT_NEXT;

//    bool has_activity = true;
//    // Ask next_module, send inital data to ACL
//    acl_serial.check(sesman, sesman, mm, 10011, signal, front_signal,has_activity);
//    // Receive answer, OK to connect
//    sesman.receive();
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
//    acl_serial.check(sesman, sesman, mm, 10012, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10042, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    // Send keepalive=ASK
//    acl_serial.check(sesman, sesman, mm, 10043, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;

//    sesman.receive();
//    //  keepalive=True
//    acl_serial.check(sesman, sesman, mm, 10045, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;

//    // koopalive=True => unknown var...
//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10072, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10075, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    RED_CHECK(not mm.last_module);  // still connected

//    // Renew Keepalive time:
//    // Send keepalive=ASK
//    acl_serial.check(sesman, sesman, mm, 10076, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    acl_serial.check(sesman, sesman, mm, 10105, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    has_activity = true;
//    RED_CHECK(not mm.last_module); // still connected

//    // Keep alive not received, disconnection
//    acl_serial.check(sesman, sesman, mm, 10106, signal, front_signal,has_activity);
//    RED_CHECK(has_activity);
//    //has_activity = true;
//    RED_CHECK(mm.last_module);  // close box
//}

//RED_AUTO_TEST_CASE(TestAuthentifierInactivity)
//{

//    BackEvent_t signal       = BACK_EVENT_NONE;
//    BackEvent_t front_signal = BACK_EVENT_NONE;

//    Inifile ini;
//    ini.set<cfg::globals::keepalive_grace_delay>(cfg::globals::keepalive_grace_delay::type{30});
//    ini.set<cfg::globals::session_timeout>(cfg::globals::session_timeout::type{240}); // = 8*30 = 240secs inactivity>
//    TimeBase time_base({0,0});
//    MMIni mm(time_base, ini);

//    auto outdata =
//        // Time 10011
//        "\x00\x00\x01\x85"
//        "bpp\n!24\n"
//        "width\n!800\n"
//        "height\n!600\n"
//        "selector_current_page\n!1\n"
//        "selector_device_filter\n!\n"
//        "selector_group_filter\n!\n"
//        "selector_proto_filter\n!\n"
//        "selector_lines_per_page\n!0\n"
//        "target_password\nASK\n"
//        "target_host\nASK\n"
//        "proto_dest\nASK\n"
//        "password\nASK\n"
//        "reporting\n!\n"
//        "auth_channel_target\n!\n"
//        "accept_message\n!False\n"
//        "display_message\n!False\n"
//        "real_target_device\n!\n"
//        "login\nASK\n"
//        "ip_client\n!\n"
//        "ip_target\n!\n"
//        "target_device\nASK\n"
//        "target_login\nASK\n"
//        ""_av
//    ;

//    auto indata =
//        "\x00\x00\x00\x83"
//        "login\n!toto\n"
//        "password\n!totopass\n"
//        "target_device\n!win\n"
//        "target_login\n!user\n"
//        "target_password\n!whoknows\n"
//        "proto_dest\n!RDP\n"
//        "authenticated\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        "\x00\x00\x00\x10"
//        "keepalive\n!True\n"

//        ""_av
//        ;

//    Fstat fstat;
//    LCGRandom rnd;
//    CryptoContext cctx;
//    init_keys(cctx);

//    TestTransport acl_trans(indata, outdata);
//    AclSerializer acl_serial(ini, 10010, acl_trans, cctx, rnd, fstat, to_verbose_flags(0));
//    Authentifier sesman(ini, cctx, Authentifier::Verbose(0));
//    sesman.set_acl_serial(&acl_serial);
//    signal = BACK_EVENT_NEXT;


//    bool has_activity = false;
//    // Ask next_module, send inital data to ACL
//    acl_serial.check(sesman, sesman, mm, 10011, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    // Receive answer, OK to connect
//    sesman.receive();

//    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
//    acl_serial.check(sesman, sesman, mm, 10012, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    acl_serial.check(sesman, sesman, mm, 10042, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    // Send keepalive=ASK
//    acl_serial.check(sesman, sesman, mm, 10043, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);

//    sesman.receive();
//    //  keepalive=True
//    acl_serial.check(sesman, sesman, mm, 10045, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);

//    // keepalive=True
//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10072, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    acl_serial.check(sesman, sesman, mm, 10075, signal, front_signal,has_activity);
//    RED_CHECK(not mm.last_module);  // still connected
//    RED_CHECK(not has_activity);

//    // Renew Keepalive time:
//    // Send keepalive=ASK
//    acl_serial.check(sesman, sesman, mm, 10076, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10079, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    RED_CHECK(not mm.last_module); // still connected


//    // Send keepalive=ASK
//    acl_serial.check(sesman, sesman, mm, 10106, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    acl_serial.check(sesman, sesman, mm, 10135, signal, front_signal,has_activity);
//    RED_CHECK(not mm.last_module); // still connected
//    RED_CHECK(not has_activity);

//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10136, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    acl_serial.check(sesman, sesman, mm, 10165, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);

//    RED_CHECK(not mm.last_module); // still connected


//    acl_serial.check(sesman, sesman, mm, 10166, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10195, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    RED_CHECK(not mm.last_module); // still connected

//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10196, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    acl_serial.check(sesman, sesman, mm, 10225, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    RED_CHECK(not mm.last_module); // still connected

//    sesman.receive();
//    acl_serial.check(sesman, sesman, mm, 10227, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    acl_serial.check(sesman, sesman, mm, 10255, signal, front_signal,has_activity);
//    RED_CHECK(not has_activity);
//    RED_CHECK(mm.last_module); // disconnected on inactivity
//}
