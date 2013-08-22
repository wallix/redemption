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
  Author(s): Christophe Grosjean, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthentifierNew
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "authentifier.hpp"
#include "counttransport.hpp"
#include "testtransport.hpp"


BOOST_AUTO_TEST_CASE(TestAuthentifierNoKeepalive)
{
    BackEvent_t signal = BACK_EVENT_NONE;

    Inifile ini;

    ini.globals.keepalive_grace_delay = 30;
    ini.globals.max_tick = 30;
    ini.globals.internal_domain = true;
    ini.debug.auth = 255;

    MMIni mm(ini);

    char outdata[] =
        // Time: 10011
           "\x00\x00\x01\x95"
           "login\nASK\n"
           "ip_client\n!\n"
           "ip_target\n!\n"
           "target_device\nASK\n"
           "target_login\nASK\n"
           "bpp\n!24\n"
           "height\n!600\n"
           "width\n!800\n"
           "selector\n!False\n"
           "selector_current_page\n!1\n"
           "selector_device_filter\n!\n"
           "selector_group_filter\n!\n"
           "selector_proto_filter\n!\n"
           "selector_lines_per_page\n!0\n"
           "target_password\nASK\n"
           "proto_dest\nASK\n"
           "password\nASK\n"
           "auth_channel_result\n!\n"
           "auth_channel_target\n!\n"
           "accept_message\n!\n"
           "display_message\n!\n"
           "proxy_type\n!RDP\n"
           "real_target_device\n!\n"

        // Time: 10043
           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

    ;

//    printf("len=%x\n",
//        (unsigned)strlen(
//        "keepalive\nASK\n"
//        "password\ntotopass\n"
//        "target_device\nwin\n"
//        "target_login\nuser\n"
//        "target_password\nwhoknows\n"
//        "proto_dest\nRDP\n"
//        "authenticated\nTrue\n"
//        ));

//    exit(0);

    char indata[] =
        "\x00\x00\x00\x7c"
        "login\ntoto\n"
        "password\ntotopass\n"
        "target_device\nwin\n"
        "target_login\nuser\n"
        "target_password\nwhoknows\n"
        "proto_dest\nRDP\n"
        "authenticated\nTrue\n"

    ;

    TestTransport acl_trans("test", indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    SessionManager sesman(&mm.ini, acl_trans, 10000, 10010);
    signal = BACK_EVENT_NEXT;

    CountTransport keepalivetrans;
    bool res/* = false*/;

    // Ask next_module, send inital data to ACL
    res = sesman.check(mm, 10011, keepalivetrans, signal);
    // Receive answer, OK to connect
    sesman.receive();
    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
    res = sesman.check(mm, 10012, keepalivetrans, signal);
    res = sesman.check(mm, 10042, keepalivetrans, signal);
    // Send keepalive=ASK
    res = sesman.check(mm, 10043, keepalivetrans, signal);
    res = sesman.check(mm, 10072, keepalivetrans, signal);
    // still connected
    BOOST_CHECK_EQUAL(mm.last_module, false);
    // If no keepalive is received after 30 seconds => disconnection
    res = sesman.check(mm, 10073, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, true);
}


BOOST_AUTO_TEST_CASE(TestAuthentifierKeepalive)
{

    BackEvent_t signal = BACK_EVENT_NONE;

    Inifile ini;

    ini.globals.keepalive_grace_delay = 30;
    ini.globals.max_tick = 30;
    ini.globals.internal_domain = true;
    ini.debug.auth = 255;

    MMIni mm(ini);


    char outdata[] =
        // Time 10011
           "\x00\x00\x01\x95"
           "login\nASK\n"
           "ip_client\n!\n"
           "ip_target\n!\n"
           "target_device\nASK\n"
           "target_login\nASK\n"
           "bpp\n!24\n"
           "height\n!600\n"
           "width\n!800\n"
           "selector\n!False\n"
           "selector_current_page\n!1\n"
           "selector_device_filter\n!\n"
           "selector_group_filter\n!\n"
           "selector_proto_filter\n!\n"
           "selector_lines_per_page\n!0\n"
           "target_password\nASK\n"
           "proto_dest\nASK\n"
           "password\nASK\n"
           "auth_channel_result\n!\n"
           "auth_channel_target\n!\n"
           "accept_message\n!\n"
           "display_message\n!\n"
           "proxy_type\n!RDP\n"
           "real_target_device\n!\n"

        // Time 10043
           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

    ;

//    printf("len=%x\n",
//        (unsigned)strlen(
//        "keepalive\nASK\n"
//        "password\ntotopass\n"
//        "target_device\nwin\n"
//        "target_login\nuser\n"
//        "target_password\nwhoknows\n"
//        "proto_dest\nRDP\n"
//        "authenticated\nTrue\n"
//        ));

//    exit(0);

    char indata[] =
        "\x00\x00\x00\x7c"
        "login\ntoto\n"
        "password\ntotopass\n"
        "target_device\nwin\n"
        "target_login\nuser\n"
        "target_password\nwhoknows\n"
        "proto_dest\nRDP\n"
        "authenticated\nTrue\n"

        // Time 10045
        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        // Time 10072 : bad message
        "\x00\x00\x00\x10"
        "koopalive\n!True\n"

    ;

    TestTransport acl_trans("test", indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    SessionManager sesman(&mm.ini, acl_trans, 10000, 10010);
    signal = BACK_EVENT_NEXT;

    CountTransport keepalivetrans;
    bool res/* = false*/;

    // Ask next_module, send inital data to ACL
    res = sesman.check(mm, 10011, keepalivetrans, signal);
    // Receive answer, OK to connect
    sesman.receive();
    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
    res = sesman.check(mm, 10012, keepalivetrans, signal);
    res = sesman.check(mm, 10042, keepalivetrans, signal);
    // Send keepalive=ASK
    res = sesman.check(mm, 10043, keepalivetrans, signal);

    sesman.receive();
    //  keepalive=True
    res = sesman.check(mm, 10045, keepalivetrans, signal);

    // koopalive=True => unknown var...
    sesman.receive();
    res = sesman.check(mm, 10072, keepalivetrans, signal);
    res = sesman.check(mm, 10075, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false);  // still connected

    // Renew Keepalive time:
    // Send keepalive=ASK
    res = sesman.check(mm, 10076, keepalivetrans, signal);
    res = sesman.check(mm, 10105, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    // Keep alive not received, disconnection
    res = sesman.check(mm, 10106, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, true);  // close box
}

BOOST_AUTO_TEST_CASE(TestAuthentifierInactivity)
{

    BackEvent_t signal = BACK_EVENT_NONE;

    Inifile ini;
    ini.globals.keepalive_grace_delay = 30;
    ini.globals.max_tick = 8; // => 8*30 = 240secs inactivity
    ini.globals.internal_domain = true;
    ini.debug.auth = 255;
    MMIni mm(ini);

    char outdata[] =
        // Time 10011
           "\x00\x00\x01\x95"
           "login\nASK\n"
           "ip_client\n!\n"
           "ip_target\n!\n"
           "target_device\nASK\n"
           "target_login\nASK\n"
           "bpp\n!24\n"
           "height\n!600\n"
           "width\n!800\n"
           "selector\n!False\n"
           "selector_current_page\n!1\n"
           "selector_device_filter\n!\n"
           "selector_group_filter\n!\n"
           "selector_proto_filter\n!\n"
           "selector_lines_per_page\n!0\n"
           "target_password\nASK\n"
           "proto_dest\nASK\n"
           "password\nASK\n"
           "auth_channel_result\n!\n"
           "auth_channel_target\n!\n"
           "accept_message\n!\n"
           "display_message\n!\n"
           "proxy_type\n!RDP\n"
           "real_target_device\n!\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"

           "\x00\x00\x00\x0E"
            "keepalive\nASK\n"
    ;

//    printf("len=%x\n",
//        (unsigned)strlen(
//        "keepalive\nASK\n"
//        "password\ntotopass\n"
//        "target_device\nwin\n"
//        "target_login\nuser\n"
//        "target_password\nwhoknows\n"
//        "proto_dest\nRDP\n"
//        "authenticated\nTrue\n"
//        ));

//    exit(0);

    char indata[] =
        "\x00\x00\x00\x7c"
        "login\ntoto\n"
        "password\ntotopass\n"
        "target_device\nwin\n"
        "target_login\nuser\n"
        "target_password\nwhoknows\n"
        "proto_dest\nRDP\n"
        "authenticated\nTrue\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"

        "\x00\x00\x00\x10"
        "keepalive\n!True\n"
    ;

    TestTransport acl_trans("test", indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);
    CountTransport keepalivetrans;
    SessionManager sesman(&ini, acl_trans, 10000, 10010);
    signal = BACK_EVENT_NEXT;


    bool res/* = false*/;

// Ask next_module, send inital data to ACL
    res = sesman.check(mm, 10011, keepalivetrans, signal);
    // Receive answer, OK to connect
    sesman.receive();
    // instanciate new mod, start keepalive (proxy ASK keepalive and should receive result in less than keepalive_grace_delay)
    res = sesman.check(mm, 10012, keepalivetrans, signal);
    res = sesman.check(mm, 10042, keepalivetrans, signal);
    // Send keepalive=ASK
    res = sesman.check(mm, 10043, keepalivetrans, signal);

    sesman.receive();
    //  keepalive=True
    res = sesman.check(mm, 10045, keepalivetrans, signal);

    // keepalive=True
    sesman.receive();
    res = sesman.check(mm, 10072, keepalivetrans, signal);
    res = sesman.check(mm, 10075, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false);  // still connected

    // Renew Keepalive time:
    // Send keepalive=ASK
    res = sesman.check(mm, 10076, keepalivetrans, signal);
    sesman.receive();
    res = sesman.check(mm, 10079, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected


    // Send keepalive=ASK
    res = sesman.check(mm, 10106, keepalivetrans, signal);
    res = sesman.check(mm, 10135, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    sesman.receive();
    res = sesman.check(mm, 10136, keepalivetrans, signal);
    res = sesman.check(mm, 10165, keepalivetrans, signal);

    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected


    res = sesman.check(mm, 10166, keepalivetrans, signal);
    sesman.receive();
    res = sesman.check(mm, 10195, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected

    sesman.receive();
    res = sesman.check(mm, 10196, keepalivetrans, signal);
    res = sesman.check(mm, 10225, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, false); // still connected


    sesman.receive();
    res = sesman.check(mm, 10227, keepalivetrans, signal);
    res = sesman.check(mm, 10255, keepalivetrans, signal);
    BOOST_CHECK_EQUAL(mm.last_module, true); // disconnected on inactivity
}
