/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthentifier
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "authentifier.hpp"
#include "testtransport.hpp"


BOOST_AUTO_TEST_CASE(TestAuthentifierOutItem)
{
    TODO("Writing an actual test for SessionManager is not so simple because it manage connection to auth_t behind the scene"
         "we should make the auth_t transport explicit dependency to enable writing usefull tests")
    Inifile ini;
    CheckTransport checktrans("TEST",5);
    SessionManager sesman(&ini, checktrans, 30, 30, true, 0);

    // test out_item
    BStream stream(1024);
    sesman.out_item(stream, STRAUTHID_PROXY_TYPE);
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data, 
                              STRAUTHID_PROXY_TYPE "\n!RDP\n",
                              strlen(STRAUTHID_PROXY_TYPE "\n!RDP\n")
                              )
                      );
    stream.reset();

    ini.context_ask(STRAUTHID_TARGET_PROTOCOL);
    sesman.out_item(stream, STRAUTHID_TARGET_PROTOCOL);
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data, 
                              STRAUTHID_TARGET_PROTOCOL "\nASK\n",
                              strlen(STRAUTHID_TARGET_PROTOCOL "\nASK\n")
                              )
                      );
    stream.reset();

    sesman.out_item(stream, STRAUTHID_PASSWORD);    
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data, 
                              STRAUTHID_PASSWORD "\nASK\n",
                              strlen(STRAUTHID_PASSWORD "\nASK\n")
                              )
                      );
    ini.context_set_value(STRAUTHID_PASSWORD,"SecureLinux");
    sesman.out_item(stream, STRAUTHID_PASSWORD);
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data+13, 
                              STRAUTHID_PASSWORD "\n!SecureLinux\n",
                              strlen(STRAUTHID_PASSWORD "\n!SecureLinux\n")
                              )
                      );
    stream.reset();

    // test ask_next_module_remote
    BOOST_CHECK(checktrans.get_status());
    try{
        sesman.ask_next_module_remote();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }
    BOOST_CHECK(!checktrans.get_status());
    // test ask_next_module
    long keepalive_time;
    bool record_video, keep_alive;
    submodule_t nextmod;
    
    try{
        sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    
    // test receive_next_module
    
    
}
// test in_item

// test incoming

BOOST_AUTO_TEST_CASE(TestAuthentifierKeepAlive)
{
    // test keep_alive functions
    // keep alive start
    BStream stream(1024);

    stream.out_uint32_be(strlen(STRAUTHID_KEEPALIVE "\nASK\n") + 4);
    stream.out_copy_bytes(STRAUTHID_KEEPALIVE "\nASK\n", strlen(STRAUTHID_KEEPALIVE "\nASK\n"));

    CheckTransport keepalivetrans((char *)stream.data,stream.get_offset());
    Inifile ini;
    SessionManager sesman(&ini, keepalivetrans, 30, 30, true, 0);
    long keepalivetime = 0;
    
    BOOST_CHECK(keepalivetrans.get_status());
    try{
        sesman.start_keep_alive(keepalivetime);
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK(keepalivetrans.get_status());
    stream.reset();

}

BOOST_AUTO_TEST_CASE(TestAuthentifierAuthChannel)
{
    
    BStream stream(1024);
    
    // TEST ask_auth_channel_target
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n") + 4);
    stream.out_copy_bytes(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n", 
                          strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n"));
    CheckTransport auth_channel_trans((char *)stream.data,stream.get_offset());
    Inifile ini;
    SessionManager sesman(&ini, auth_channel_trans, 30, 30, true, 0);
    
    BOOST_CHECK(auth_channel_trans.get_status());
    try{
        sesman.ask_auth_channel_target("TEST_TARGET");
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK(auth_channel_trans.get_status());
    stream.reset();

    // TEST set_auth_channel_result
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n") + 4);
    stream.out_copy_bytes(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n", 
                          strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n"));
    CheckTransport auth_channel_trans2((char *)stream.data,stream.get_offset());
    Inifile ini2;
    SessionManager sesman2(&ini2, auth_channel_trans2, 30, 30, true, 0);
    
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
    SessionManager sesman(&ini, get_mod_trans, 30, 30, true, 0);
    submodule_t nextmod;
    int res;

    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_EXIT == res);
    BOOST_CHECK(INTERNAL_CARD == nextmod);
    
    ini.context_set_value(AUTHID_TARGET_DEVICE,"autotest");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_INTERNAL == res);
    BOOST_CHECK(INTERNAL_TEST == nextmod);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"");

    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "RDP");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_RDP == res);

    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "VNC");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_VNC == res);

    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "XUP");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_XUP == res);

    // TODO INTERNAL STATUS
    
    
}
