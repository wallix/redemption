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
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "acl/module_manager.hpp"

RED_AUTO_TEST_CASE(TestModuleManagerNextMod)
{
    Inifile ini;

    MMIni mm(ini);
    int res;

    ini.set_acl<cfg::context::module>("login");

    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.set_acl<cfg::globals::auth_user>("user");
    ini.set_acl<cfg::context::password>("securepassword");

    ini.set<cfg::context::selector>(true);
    ini.set<cfg::globals::target_device>("redjenkins-vbox@127::0>.0.1 internal@display_message replay@autotest");
    ini.set_acl<cfg::globals::target_user>("rdp internal internal");
    ini.set_acl<cfg::context::module>("selector");
    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_INTERNAL_WIDGET2_SELECTOR);

    ini.ask<cfg::context::password>();
    ini.set_acl<cfg::context::module>("login");
    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.set_acl<cfg::globals::auth_user>("user");
    ini.set_acl<cfg::context::password>("securepassword");
    ini.set<cfg::globals::target_device>("redjenkins-vbox@127::0>.0.1 internal@display_message replay@autotest");
    ini.set_acl<cfg::globals::target_user>("rdp internal internal");
    ini.set<cfg::context::selector>(false);
    ini.set_acl<cfg::context::module>("confirm");
    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE);

    ini.set_acl<cfg::context::display_message>("message");
    ini.set_acl<cfg::context::module>("valid");
    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_INTERNAL_DIALOG_VALID_MESSAGE);
    ini.set_acl<cfg::context::accept_message>("message");

    ini.set_acl<cfg::context::target_password>("securepassword");
    ini.set_acl<cfg::context::module>("close");
    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_INTERNAL_CLOSE);

    ini.set_acl<cfg::context::authenticated>(true);
    ini.set<cfg::context::target_protocol>("RDP");
    ini.set_acl<cfg::context::module>("RDP");
    res = mm.next_module();
    RED_CHECK_EQUAL(res, MODULE_RDP);
}

