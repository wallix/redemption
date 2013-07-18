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

BOOST_AUTO_TEST_CASE(TestAuthentifier)
{
    class FakeModuleManager : public MMApi
    {
        public:
        FakeModuleManager() {}
        ~FakeModuleManager() {}
        virtual void remove_mod() {}
        virtual void new_mod(int target_module) {
            printf("new mod %d\n", target_module);
        }
        virtual void record() {}
    } mm;

    BackEvent_t signal = BACK_EVENT_NONE;

    Inifile ini;
    ini.globals.keepalive_grace_delay = 30;
    ini.globals.max_tick = 30;
    ini.globals.internal_domain = true;
    ini.debug.auth = 255;
    
    char outdata[] = 
           "\x00\x00\x01\x96"
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
           "selector_lines_per_page\n!20\n"
           "target_password\nASK\n"
           "proto_dest\nASK\n"
           "password\nASK\n"
           "auth_channel_result\n!\n"
           "auth_channel_target\n!\n"
           "accept_message\n!\n"
           "display_message\n!\n"
           "proxy_type\n!RDP\n"
           "real_target_device\n!\n"
    ;
    
//    printf("len=%x\n", 
//        (unsigned)strlen(
//        "login\ntoto\n"
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
    SessionManager sesman(&ini, acl_trans, 10000, 10010);
    signal = BACK_EVENT_NEXT;

    CountTransport keepalivetrans;
    bool res = false;

    // Connection followed by no keep alive received => disconnection

    res = sesman.check(mm, 10011, keepalivetrans, signal); BOOST_CHECK_EQUAL(true, res);
    sesman.receive();
    res = sesman.check(mm, 10012, keepalivetrans, signal); BOOST_CHECK_EQUAL(true, res);

    res = sesman.check(mm, 10072, keepalivetrans, signal); BOOST_CHECK_EQUAL(true, res);
    BOOST_CHECK_EQUAL(sesman.last_module, false);
    res = sesman.check(mm, 10073, keepalivetrans, signal); BOOST_CHECK_EQUAL(true, res);
    BOOST_CHECK_EQUAL(sesman.last_module, true);
}

