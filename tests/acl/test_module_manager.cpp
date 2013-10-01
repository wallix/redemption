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
#define BOOST_TEST_MODULE TestModuleManager
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "module_manager.hpp"
#include "counttransport.hpp"
#include "testtransport.hpp"

#include "client_info.hpp"
#include "front_api.hpp"
#include "keymap2.hpp"
#include "RDP/RDPDrawable.hpp"

BOOST_AUTO_TEST_CASE(TestModuleManagerNextMod)
{
    Inifile ini;

    MMIni mm(ini);
    int res;

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.globals.auth_user.set_from_cstr("user");
    ini.context.password.set_from_cstr("securepassword");

    ini.context.selector.set(true);
    ini.globals.target_device.set_from_cstr("redjenkins-vbox@127.0.0.1 internal@display_message replay@autotest");
    ini.globals.target_user.set_from_cstr("rdp internal internal");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_SELECTOR);

    ini.globals.target_user.ask();
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.globals.auth_user.set_from_cstr("user");
    ini.context.password.set_from_cstr("securepassword");
    ini.globals.target_device.set_from_cstr("redjenkins-vbox@127.0.0.1 internal@display_message replay@autotest");
    ini.globals.target_user.set_from_cstr("rdp internal internal");
    ini.context.selector.set(false);
    ini.context.display_message.ask();

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE);

    ini.context.display_message.set_from_cstr("message");
    ini.context.accept_message.ask();
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_DIALOG_VALID_MESSAGE);
    ini.context.accept_message.set_from_cstr("message");

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_CLOSE);

    ini.context.authenticated.set(true);
    ini.context.target_protocol.set_from_cstr("RDP");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_RDP);

}

BOOST_AUTO_TEST_CASE(TestModuleManagerGetMod)
{
    // test get mod from protocol
    Inifile ini;
    ini.globals.keepalive_grace_delay = 30;
    ini.globals.max_tick = 30;
    ini.globals.internal_domain = true;
    ini.debug.auth = 4;

    MMIni mm(ini);
    int res;

    // no known protocol on target device yet (should be an error case)
    res = mm.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_EXIT), res);


    // auto test case
    ini.context_set_value(AUTHID_TARGET_DEVICE,"autotest");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_INTERNAL_TEST), res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"");

    // RDP protocol on target
    // ini.context_set_value(AUTHID_TARGET_PROTOCOL, "RDP");
    ini.context.target_protocol.set_from_cstr("RDP");
    mm.connected = false;
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_RDP == res);

    // VNC protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "VNC");
    mm.connected = false;
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_VNC == res);

    // XUP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "XUP");
    mm.connected = false;
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_XUP == res);

    // INTERNAL STATUS
    ini.context_set_value(AUTHID_TARGET_PROTOCOL,"INTERNAL");
    ini.context_set_value(AUTHID_TARGET_DEVICE,"selector");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_SELECTOR == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"login");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"bouncer2");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_BOUNCER2 == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_login");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"close");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_close");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_dialog");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_DIALOG == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_message");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_MESSAGE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"card");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CARD == res);
}
