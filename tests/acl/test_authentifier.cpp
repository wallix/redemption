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
   
    Inifile ini;
    CheckTransport checktrans("TEST",5);
    SessionManager sesman(&ini, checktrans, 30, 30, true, 0);

    // test out_item
    // out_item(Stream stream,const char * key) should add in the Stream a string 
    // composed of key and its value if known in Inifile, 
    // or "ASK" if the value is asked.
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
    
    
}
BOOST_AUTO_TEST_CASE(TestAuthentifierInItem)
{
    Inifile ini;
    
}
// test in_item


BOOST_AUTO_TEST_CASE(TestAuthentifierKeepAlive)
{
    // test keep_alive functions
    // keep alive start
    // keep_alive_start should send to the Transport a message asking for keep alive value.
    BStream stream(1024);

    stream.out_uint32_be(strlen(STRAUTHID_KEEPALIVE "\nASK\n"));
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
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n"));
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
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n"));
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

    // no known protocol on target device yet (should be an error case)
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_EXIT == res);
    BOOST_CHECK(INTERNAL_CARD == nextmod);
    

    // auto test case
    ini.context_set_value(AUTHID_TARGET_DEVICE,"autotest");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_INTERNAL == res);
    BOOST_CHECK(INTERNAL_TEST == nextmod);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"");

    // RDP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "RDP");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_RDP == res);

    // VNC protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "VNC");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_VNC == res);

    // XUP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "XUP");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_XUP == res);

    // INTERNAL STATUS
    // test nextmod value 
    ini.context_set_value(AUTHID_TARGET_PROTOCOL,"INTERNAL");
    ini.context_set_value(AUTHID_TARGET_DEVICE,"bouncer2");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_INTERNAL == res);
    BOOST_CHECK(INTERNAL_BOUNCER2 == nextmod);
    
    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_login");
    res = sesman.get_mod_from_protocol(nextmod);
    BOOST_CHECK(MCTX_STATUS_INTERNAL == res);
    BOOST_CHECK(INTERNAL_WIDGET2_LOGIN == nextmod);

}

BOOST_AUTO_TEST_CASE(TestAuthentifierNormalCase)
{

    Inifile ini;


    long keepalive_time;
    bool record_video, keep_alive;
    submodule_t nextmod;

    #include "fixtures/dump_auth_normal_case.hpp"

    const char * name = "Authentifier Normal Case";
    int verbose = 256;
    
    TestTransport trans(name, indata, sizeof(indata), outdata, sizeof(outdata),verbose);
    SessionManager sesman(&ini, trans, 30, 30, true, 0);

    ini.context_set_value(STRAUTHID_PROXY_TYPE,"RDP");
    ini.context_set_value(STRAUTHID_DISPLAY_MESSAGE,"");
    ini.context_set_value(STRAUTHID_ACCEPT_MESSAGE,"");
    ini.context_set_value(STRAUTHID_HOST,"127.0.0.1");
    ini.context_set_value(STRAUTHID_TARGET, "127.0.0.1");
    ini.context_set_value(STRAUTHID_AUTH_USER, "mtan");
    ini.context_ask(STRAUTHID_PASSWORD);
    ini.context_ask(STRAUTHID_TARGET_USER);
    ini.context_ask(STRAUTHID_TARGET_DEVICE);
    ini.context_ask(STRAUTHID_TARGET_PROTOCOL);
    ini.context_ask(STRAUTHID_SELECTOR);
    ini.context_set_value(STRAUTHID_SELECTOR_GROUP_FILTER, "");
    ini.context_set_value(STRAUTHID_SELECTOR_DEVICE_FILTER, "");
    ini.context_set_value(STRAUTHID_SELECTOR_LINES_PER_PAGE, "20");
    ini.context_set_value(STRAUTHID_SELECTOR_CURRENT_PAGE, "1");
    ini.context_ask(STRAUTHID_TARGET_PASSWORD);
    ini.context_set_value(STRAUTHID_OPT_WIDTH, "800");
    ini.context_set_value(STRAUTHID_OPT_HEIGHT, "600");
    ini.context_set_value(STRAUTHID_OPT_BPP, "24");
    ini.context_set_value(STRAUTHID_REAL_TARGET_DEVICE, "");
    

    BOOST_CHECK(trans.get_status());
    //SEND No 1
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 1
    sesman.receive_next_module();

    // Login (at login window, password is still asked)
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // set login / password
    ini.context_set_value(STRAUTHID_AUTH_USER, "x");
    ini.context_set_value(STRAUTHID_PASSWORD, "x");

    //SEND No 2
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 2
    sesman.receive_next_module();
    
    // Selector 
    // Got target users, target devices and target protocols at this point
    // and password is correct.
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // This is a context refresh
    //SEND No 3
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 3
    sesman.receive_next_module();

    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // Target Selected
    //SEND No 4
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 4
    sesman.receive_next_module();
    // At this point, the target is selected and ACL delivers the target password
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

}


BOOST_AUTO_TEST_CASE(TestAuthentifierWrongPassword)
{

    Inifile ini;


    long keepalive_time;
    bool record_video, keep_alive;
    submodule_t nextmod;

    #include "fixtures/dump_auth_wrong_password.hpp"

    const char * name = "Authentifier Wrong Password";
    int verbose = 256;
    
    TestTransport trans(name, indata, sizeof(indata), outdata, sizeof(outdata),verbose);
    SessionManager sesman(&ini, trans, 30, 30, true, 0);

    ini.context_set_value(STRAUTHID_PROXY_TYPE,"RDP");
    ini.context_set_value(STRAUTHID_DISPLAY_MESSAGE,"");
    ini.context_set_value(STRAUTHID_ACCEPT_MESSAGE,"");
    ini.context_set_value(STRAUTHID_HOST,"127.0.0.1");
    ini.context_set_value(STRAUTHID_TARGET, "127.0.0.1");
    ini.context_set_value(STRAUTHID_AUTH_USER, "mtan");
    ini.context_ask(STRAUTHID_PASSWORD);
    ini.context_ask(STRAUTHID_TARGET_USER);
    ini.context_ask(STRAUTHID_TARGET_DEVICE);
    ini.context_ask(STRAUTHID_TARGET_PROTOCOL);
    ini.context_ask(STRAUTHID_SELECTOR);
    ini.context_set_value(STRAUTHID_SELECTOR_GROUP_FILTER, "");
    ini.context_set_value(STRAUTHID_SELECTOR_DEVICE_FILTER, "");
    ini.context_set_value(STRAUTHID_SELECTOR_LINES_PER_PAGE, "20");
    ini.context_set_value(STRAUTHID_SELECTOR_CURRENT_PAGE, "1");
    ini.context_ask(STRAUTHID_TARGET_PASSWORD);
    ini.context_set_value(STRAUTHID_OPT_WIDTH, "800");
    ini.context_set_value(STRAUTHID_OPT_HEIGHT, "600");
    ini.context_set_value(STRAUTHID_OPT_BPP, "24");
    ini.context_set_value(STRAUTHID_REAL_TARGET_DEVICE, "");
    

    BOOST_CHECK(trans.get_status());
    //SEND No 1
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 1
    sesman.receive_next_module();

    // Login (at login window, password is still asked)
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // set login / password
    ini.context_set_value(STRAUTHID_AUTH_USER, "x");
    ini.context_set_value(STRAUTHID_PASSWORD, "wrong"); // wrong is a wrong password

    //SEND No 2
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 2
    sesman.receive_next_module();
    
    // Wrong password, context is still asked
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));


}

BOOST_AUTO_TEST_CASE(TestAuthentifierSelectorLogout)
{

    Inifile ini;


    long keepalive_time;
    bool record_video, keep_alive;
    submodule_t nextmod;

    #include "fixtures/dump_auth_selector_logout.hpp"

    const char * name = "Authentifier Selector Logout";
    int verbose = 256;
    
    TestTransport trans(name, indata, sizeof(indata), outdata, sizeof(outdata),verbose);
    SessionManager sesman(&ini, trans, 30, 30, true, 0);

    ini.context_set_value(STRAUTHID_PROXY_TYPE,"RDP");
    ini.context_set_value(STRAUTHID_DISPLAY_MESSAGE,"");
    ini.context_set_value(STRAUTHID_ACCEPT_MESSAGE,"");
    ini.context_set_value(STRAUTHID_HOST,"127.0.0.1");
    ini.context_set_value(STRAUTHID_TARGET, "127.0.0.1");
    ini.context_set_value(STRAUTHID_AUTH_USER, "mtan");
    ini.context_ask(STRAUTHID_PASSWORD);
    ini.context_ask(STRAUTHID_TARGET_USER);
    ini.context_ask(STRAUTHID_TARGET_DEVICE);
    ini.context_ask(STRAUTHID_TARGET_PROTOCOL);
    ini.context_ask(STRAUTHID_SELECTOR);
    ini.context_set_value(STRAUTHID_SELECTOR_GROUP_FILTER, "");
    ini.context_set_value(STRAUTHID_SELECTOR_DEVICE_FILTER, "");
    ini.context_set_value(STRAUTHID_SELECTOR_LINES_PER_PAGE, "20");
    ini.context_set_value(STRAUTHID_SELECTOR_CURRENT_PAGE, "1");
    ini.context_ask(STRAUTHID_TARGET_PASSWORD);
    ini.context_set_value(STRAUTHID_OPT_WIDTH, "800");
    ini.context_set_value(STRAUTHID_OPT_HEIGHT, "600");
    ini.context_set_value(STRAUTHID_OPT_BPP, "24");
    ini.context_set_value(STRAUTHID_REAL_TARGET_DEVICE, "");
    
    
    BOOST_CHECK(trans.get_status());
    //SEND No 1
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 1
    sesman.receive_next_module();

    // Login (at login window, password is still asked)
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // set login / password
    ini.context_set_value(STRAUTHID_AUTH_USER, "x");
    ini.context_set_value(STRAUTHID_PASSWORD, "x");

    //SEND No 2
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 2
    sesman.receive_next_module();
    
    // Selector 
    // Got target users, target devices and target protocols at this point
    // and password is correct.
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // This is a context refresh
    //SEND No 3
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 3
    sesman.receive_next_module();

    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

    // Selector Logout
    //SEND No 4
    sesman.ask_next_module(keepalive_time,record_video,keep_alive,nextmod);
    BOOST_CHECK(trans.get_status());
    //RECEIVE No 4
    sesman.receive_next_module();
    // get back to login,
    // Password, target devices, target users are asked (but not target protocol)
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_USER));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_TARGET_PASSWORD));

}
