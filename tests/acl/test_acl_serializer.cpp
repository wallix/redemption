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

   Unit tests for Acl Serializer
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAclSerializer
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "acl_serializer.hpp"
#include "testtransport.hpp"

// Class ACL Serializer is used to Modify config file content from a remote ACL manager
// - Send given fields from config
// - Recover fields from network and update Config

BOOST_AUTO_TEST_CASE(TestAclSerializerOutItem)
{

    Inifile ini;
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);

    // test out_item
    // out_item(Stream stream,authid_t authid) should add in the Stream a string
    // composed of authid key string and its value if known in Inifile,
    // or "ASK" if the value is asked.
    BStream stream(1024);
    acl.out_item(stream, AUTHID_PROXY_TYPE);
    BOOST_CHECK_EQUAL(0,
                      strncmp((char*)stream.get_data(),
                              STRAUTHID_PROXY_TYPE "\n!RDP\n",
                              strlen(STRAUTHID_PROXY_TYPE "\n!RDP\n")));
    stream.reset();

    ini.context_ask(AUTHID_TARGET_PROTOCOL);
    acl.out_item(stream, AUTHID_TARGET_PROTOCOL);
    BOOST_CHECK_EQUAL(0,
                      strncmp((char*)stream.get_data(),
                              STRAUTHID_TARGET_PROTOCOL "\nASK\n",
                              strlen(STRAUTHID_TARGET_PROTOCOL "\nASK\n")));
    stream.reset();

    acl.out_item(stream, AUTHID_PASSWORD);
    BOOST_CHECK_EQUAL(0,
                      strncmp((char*)stream.get_data(),
                              STRAUTHID_PASSWORD "\nASK\n",
                              strlen(STRAUTHID_PASSWORD "\nASK\n")));
    ini.context_set_value(AUTHID_PASSWORD,"SecureLinux");
    acl.out_item(stream, AUTHID_PASSWORD);
    BOOST_CHECK_EQUAL(0,
                      strncmp((char*)stream.get_data()+13,
                              STRAUTHID_PASSWORD "\n!SecureLinux\n",
                              strlen(STRAUTHID_PASSWORD "\n!SecureLinux\n")));
    stream.reset();
}

BOOST_AUTO_TEST_CASE(TestAclSerializeAskNextModule)
{
    Inifile ini;
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);
    ini.context_set_value(AUTHID_TRACE_SEAL, "true");
    try {
        acl.ask_next_module_remote();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }

    // try exception
    CheckTransport transexcpt("something very wrong",21);
    AclSerializer aclexcpt(&ini, transexcpt, 0);
    ini.context_set_value(AUTHID_AUTH_USER, "Newuser");
    aclexcpt.ask_next_module_remote();
    char buffer[256];
    BOOST_CHECK(!ini.context_get_bool(AUTHID_AUTHENTICATED));
    BOOST_CHECK_EQUAL(0,
                      strncmp(ini.context_get_value(AUTHID_REJECTED,buffer,sizeof(buffer)),
                              "Authentifier service failed",
                              strlen("Authentifier service failed")));
}

BOOST_AUTO_TEST_CASE(TestAclSerializeIncoming)
{
    Inifile ini;
    BStream stream(1024);
    // NORMAL CASE WITH SESSION ID CHANGE
    stream.out_uint32_be(0);
    TODO("rename out_concat out_stringz")
    stream.out_concat(STRAUTHID_AUTH_USER "\nASK\n");
    stream.out_concat(STRAUTHID_PASSWORD "\nASK\n");

    stream.out_concat(STRAUTHID_SESSION_ID "\n!6455\n");
    stream.set_out_uint32_be(stream.get_offset() - 4 ,0);

    GeneratorTransport trans((char *)stream.get_data(),stream.get_offset());
    AclSerializer acl(&ini, trans, 0);
    ini.context.session_id.set_empty();
    ini.context_set_value(AUTHID_AUTH_USER,"testuser");
    BOOST_CHECK(ini.context.session_id.get().is_empty());
    BOOST_CHECK(!ini.context_is_asked(AUTHID_AUTH_USER));

    try {
        acl.incoming();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }
    BOOST_CHECK(ini.context_is_asked(AUTHID_AUTH_USER));
    BOOST_CHECK(!ini.context.session_id.get().is_empty());

    // CASE EXCEPTION
    // try exception
    stream.reset();
    stream.out_uint32_be(0xFFFFFFFF);
    stream.out_concat(STRAUTHID_AUTH_USER "\nASK\n");
    stream.out_concat(STRAUTHID_PASSWORD "\nASK\n");

    GeneratorTransport transexcpt((char *)stream.p,stream.get_offset());
    AclSerializer aclexcpt(&ini, transexcpt, 0);
    try {
        aclexcpt.incoming();
    } catch (const Error & e){
        BOOST_CHECK_EQUAL((uint32_t)ERR_ACL_MESSAGE_TOO_BIG, (uint32_t)e.id);
    }

}

inline void execute_test_initem(Stream & stream, AclSerializer & acl, const authid_t authid, const char * value)
{
    // create stream with key , ask
    stream.out_concat(string_from_authid(authid));
    stream.out_concat(value);
    stream.mark_end();
    stream.rewind();

    // execute in_item
    acl.in_item(stream);
}

inline void test_initem_ask(Inifile & ini, AclSerializer & acl, const authid_t authid, const char * defaut)
{
    BStream stream(2048);

    // Set defaut value to strauthid key
    ini.context_set_value(authid, defaut);
    BOOST_CHECK(!ini.context_is_asked(authid));

    execute_test_initem(stream, acl, authid,"\nASK\n");
    // check key value is known
    BOOST_CHECK(ini.context_is_asked(authid));
}

inline void test_initem_receive(Inifile & ini, AclSerializer & acl, const authid_t authid, const char * value)
{
    BStream stream(2048);
    // set strauthid key to be asked
    ini.context_ask(authid);
    BOOST_CHECK(ini.context_is_asked(authid));

    execute_test_initem(stream, acl, authid,value);

    // check key value is known
    BOOST_CHECK(!ini.context_is_asked(authid));
}

BOOST_AUTO_TEST_CASE(TestAclSerializerInItem)
{
    Inifile ini;
    BStream stream(1);
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);

    // SOME NORMAL CASE
    test_initem_ask(ini, acl, AUTHID_PASSWORD,"SecureLinux");
    test_initem_ask(ini, acl, AUTHID_PROXY_TYPE,"VNC");
    test_initem_ask(ini, acl, AUTHID_SELECTOR_CURRENT_PAGE,"");
    test_initem_receive(ini, acl, AUTHID_SELECTOR_CURRENT_PAGE,"\n2\n");
    test_initem_receive(ini, acl, AUTHID_PASSWORD,"\n!SecureLinux\n");
    test_initem_receive(ini, acl, AUTHID_PROXY_TYPE,"\nRDP\n");

    // CASE EXCEPTION
    // try exception
    // stream.init(strlen(STRAUTHID_AUTH_USER "didier"));
    try{
        test_initem_receive(ini,acl,AUTHID_AUTH_USER,"didier");
    } catch (const Error & e) {
         BOOST_CHECK_EQUAL((uint32_t)ERR_ACL_UNEXPECTED_IN_ITEM_OUT, (uint32_t)e.id);
    }
}

BOOST_AUTO_TEST_CASE(TestAclSerializerInItems)
{
    Inifile ini;
    BStream stream(1024);
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);

    ini.context_set_value(AUTHID_PASSWORD, "VerySecurePassword");
    ini.context_ask(AUTHID_PROXY_TYPE);
    BOOST_CHECK(!ini.context_is_asked(AUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(AUTHID_PROXY_TYPE));
    stream.out_copy_bytes(STRAUTHID_PASSWORD "\nASK\n", strlen(STRAUTHID_PASSWORD "\nASK\n"));
    stream.out_copy_bytes(STRAUTHID_PROXY_TYPE "\nVNC\n", strlen(STRAUTHID_PROXY_TYPE "\nVNC\n"));
    stream.mark_end();
    stream.rewind();
    acl.in_items(stream);
    BOOST_CHECK(ini.context_is_asked(AUTHID_PASSWORD));
    BOOST_CHECK(!ini.context_is_asked(AUTHID_PROXY_TYPE));
}
