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

    ini.context.module.set_from_cstr("login");

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.globals.auth_user.set_from_cstr("user");
    ini.context.password.set_from_cstr("securepassword");

    ini.context.selector.set(true);
    ini.globals.target_device.set_from_cstr("redjenkins-vbox@127.0.0.1 internal@display_message replay@autotest");
    ini.globals.target_user.set_from_cstr("rdp internal internal");
    ini.context.module.set_from_cstr("selector");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_SELECTOR);

    ini.context.password.ask();
    ini.context.module.set_from_cstr("login");
    res = mm.next_module();
    BOOST_CHECK_EQUAL(res, (int)MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.globals.auth_user.set_from_cstr("user");
    ini.context.password.set_from_cstr("securepassword");
    ini.globals.target_device.set_from_cstr("redjenkins-vbox@127.0.0.1 internal@display_message replay@autotest");
    ini.globals.target_user.set_from_cstr("rdp internal internal");
    ini.context.selector.set(false);
    ini.context.display_message.ask();
    ini.context.module.set_from_cstr("confirm");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE);

    ini.context.display_message.set_from_cstr("message");
    ini.context.accept_message.ask();
    ini.context.module.set_from_cstr("valid");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_DIALOG_VALID_MESSAGE);
    ini.context.accept_message.set_from_cstr("message");

    ini.context.target_password.set_from_cstr("securepassword");
    ini.context.module.set_from_cstr("close");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_CLOSE);

    ini.context.authenticated.set(true);
    ini.context.target_protocol.set_from_cstr("RDP");
    ini.context.module.set_from_cstr("RDP");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_RDP);

}

