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
#define BOOST_TEST_MODULE TestAuthentifier
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "authentifier.hpp"
#include "testtransport.hpp"

BOOST_AUTO_TEST_CASE(TestAuthentifierKeepAlive)
{
    // test keep_alive functions
    // keep alive start
    // keep_alive_start should send to the Transport a message asking for keep alive value.
    BStream stream(1024);

    stream.out_uint32_be(strlen(STRAUTHID_KEEPALIVE "\nASK\n"));
    stream.out_copy_bytes(STRAUTHID_KEEPALIVE "\nASK\n", strlen(STRAUTHID_KEEPALIVE "\nASK\n"));

    CheckTransport keepalivetrans((char *)stream.get_data(), stream.get_offset());
    Inifile ini;
    SessionManager sesman(&ini, keepalivetrans, 10000, 10010, 30, 30, true, 0);
    long keepalivetime = 0;

    //    BOOST_CHECK(keepalivetrans.get_status());
    //    try{
    //        sesman.start_keep_alive(keepalivetime);
    //    } catch (const Error & e){
    //        BOOST_CHECK(false);
    //    };
    //    BOOST_CHECK(keepalivetrans.get_status());
    stream.reset();

}

BOOST_AUTO_TEST_CASE(TestAuthentifierAuthChannel)
{

    BStream stream(1024);

    // TEST ask_auth_channel_target
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n"));
    stream.out_copy_bytes(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n",
                          strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n"));
    CheckTransport auth_channel_trans((char *)stream.get_data(), stream.get_offset());
    Inifile ini;
    SessionManager sesman(&ini, auth_channel_trans, 10000, 10010, 30, 30, true, 0);

    BOOST_CHECK(auth_channel_trans.get_status());
    try{
        sesman.ask_auth_channel_target("TEST_TARGET");
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK(auth_channel_trans.get_status());
    stream.reset();

    // TEST set_auth_channel_result
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n"));
    stream.out_copy_bytes(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n",
                          strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n"));
    CheckTransport auth_channel_trans2((char *)stream.get_data(), stream.get_offset());
    Inifile ini2;
    SessionManager sesman2(&ini2, auth_channel_trans2, 10000, 10010, 30, 30, true, 0);

    BOOST_CHECK(auth_channel_trans2.get_status());
    try{
        sesman2.set_auth_channel_result("TEST_RESULT");
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK(auth_channel_trans2.get_status());

}

BOOST_AUTO_TEST_CASE(TestAuthentifierGetMod)
{
    // test get mod from protocol
    LogTransport get_mod_trans;
    Inifile ini;
    SessionManager sesman(&ini, get_mod_trans, 10000, 10010, 30, 30, true, 4);
    int res;

    // no known protocol on target device yet (should be an error case)
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_EXIT), res);


    // auto test case
    ini.context_set_value(AUTHID_TARGET_DEVICE,"autotest");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_INTERNAL_TEST), res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"");

    // RDP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "RDP");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_RDP == res);

    // VNC protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "VNC");
    sesman.connected = false;
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_VNC == res);

    // XUP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "XUP");
    sesman.connected = false;
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_XUP == res);

    // INTERNAL STATUS
    ini.context_set_value(AUTHID_TARGET_PROTOCOL,"INTERNAL");
    ini.context_set_value(AUTHID_TARGET_DEVICE,"selector");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_SELECTOR == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"bouncer2");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_BOUNCER2 == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl_login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"close");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_close");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_dialog");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_DIALOG == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_message");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_MESSAGE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl_login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"card");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CARD == res);
}

