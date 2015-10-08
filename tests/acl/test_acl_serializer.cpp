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

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "acl_serializer.hpp"
#include "test_transport.hpp"

// Class ACL Serializer is used to Modify config file content from a remote ACL manager
// - Send given fields from config
// - Recover fields from network and update Config

BOOST_AUTO_TEST_CASE(TestAclSerializeAskNextModule)
{
    Inifile ini;
    LogTransport trans;
    AclSerializer acl(ini, trans, 0);
    ini.set<cfg::context::forcemodule>(true);
    try {
        acl.send_acl_data();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }

    // try exception
    CheckTransport transexcpt("something very wrong",21);
    AclSerializer aclexcpt(ini, transexcpt, 0);
    ini.set_acl<cfg::globals::auth_user>("Newuser");
    aclexcpt.send_acl_data();
    BOOST_CHECK(!ini.get<cfg::context::authenticated>());
    BOOST_CHECK_EQUAL(ini.get<cfg::context::rejected>(), "Authentifier service failed");
}

BOOST_AUTO_TEST_CASE(TestAclSerializeIncoming)
{
    Inifile ini;
    StaticOutStream<1024> stream;
    // NORMAL CASE WITH SESSION ID CHANGE
    stream.out_uint32_be(0);
    stream.out_string(string_from_authid(AUTHID_GLOBALS_AUTH_USER)); stream.out_string("\nASK\n");
    stream.out_string(string_from_authid(AUTHID_CONTEXT_PASSWORD)); stream.out_string("\nASK\n");

    stream.out_string(string_from_authid(AUTHID_CONTEXT_SESSION_ID)); stream.out_string("\n!6455\n");
    stream.set_out_uint32_be(stream.get_offset() - 4 ,0);

    GeneratorTransport trans((char *)stream.get_data(),stream.get_offset());
    AclSerializer acl(ini, trans, 0);
    ini.set<cfg::context::session_id>("");
    ini.set_acl<cfg::globals::auth_user>("testuser");
    BOOST_CHECK(ini.get<cfg::context::session_id>().empty());
    BOOST_CHECK(!ini.is_asked<cfg::globals::auth_user>());

    try {
        acl.incoming();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }
    BOOST_CHECK(ini.is_asked<cfg::globals::auth_user>());
    BOOST_CHECK(!ini.get<cfg::context::session_id>().empty());

    // CASE EXCEPTION
    // try exception
    stream.rewind();
    stream.out_uint32_be(0xFFFFFFFF);
    stream.out_string(string_from_authid(AUTHID_GLOBALS_AUTH_USER)); stream.out_string("\nASK\n");
    stream.out_string(string_from_authid(AUTHID_CONTEXT_PASSWORD)); stream.out_string("\nASK\n");

    GeneratorTransport transexcpt(reinterpret_cast<char *>(stream.get_current()) ,stream.get_offset());
    AclSerializer aclexcpt(ini, transexcpt, 0);
    try {
        aclexcpt.incoming();
    } catch (const Error & e){
        BOOST_CHECK_EQUAL((uint32_t)ERR_ACL_MESSAGE_TOO_BIG, (uint32_t)e.id);
    }

}

inline void execute_test_initem(OutStream & stream, AclSerializer & acl, const authid_t authid, const char * value)
{
    // create stream with key , ask
    stream.out_string(string_from_authid(authid));
    stream.out_string(value);

    AclSerializer::ArrayItemsView view{stream.get_data(), stream.get_data() + stream.get_offset()};

    // execute in_item
    acl.in_item(view);
}

template<class Cfg, class U>
inline void test_initem_ask(Inifile & ini, AclSerializer & acl, const authid_t authid, U const & defaut)
{
    StaticOutStream<2048> stream;

    // Set defaut value to strauthid key
    ini.set_acl<Cfg>(defaut);
    BOOST_CHECK(!ini.is_asked<Cfg>());

    execute_test_initem(stream, acl, authid, "\nASK\n");
    // check key value is known
    BOOST_CHECK(ini.is_asked<Cfg>());
}

template<class Cfg>
inline void test_initem_receive(Inifile & ini, AclSerializer & acl, const authid_t authid, char const * value)
{
    StaticOutStream<2048> stream;
    // set strauthid key to be asked
    ini.ask<Cfg>();
    BOOST_CHECK(ini.is_asked<Cfg>());

    execute_test_initem(stream, acl, authid, value);

    // check key value is known
    BOOST_CHECK(!ini.is_asked<Cfg>());
}

BOOST_AUTO_TEST_CASE(TestAclSerializerInItem)
{
    Inifile ini;
    StaticOutStream<1> stream;
    LogTransport trans;
    AclSerializer acl(ini, trans, 0);

    // SOME NORMAL CASE
    test_initem_ask<cfg::context::password>(ini,acl, AUTHID_CONTEXT_PASSWORD, "SecureLinux");
    test_initem_ask<cfg::context::selector_current_page>(ini,acl, AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE, 0);
    test_initem_receive<cfg::context::selector_current_page>(ini,acl, AUTHID_CONTEXT_SELECTOR_CURRENT_PAGE, "\n2\n");
    test_initem_receive<cfg::context::password>(ini,acl, AUTHID_CONTEXT_PASSWORD, "\n!SecureLinux\n");

    // CASE EXCEPTION
    // try exception
    // stream.init(strlen(STRAUTHID_AUTH_USER "didier"));
    try{
        test_initem_receive<cfg::globals::auth_user>(ini, acl, AUTHID_GLOBALS_AUTH_USER, "didier");
        BOOST_CHECK(!"No exception throw");
    } catch (const Error & e) {
         BOOST_CHECK_EQUAL((uint32_t)ERR_ACL_UNEXPECTED_IN_ITEM_OUT, (uint32_t)e.id);
    }
}

BOOST_AUTO_TEST_CASE(TestAclSerializerInItems)
{
    Inifile ini;
    StaticOutStream<1024> stream;
    LogTransport trans;
    AclSerializer acl(ini, trans, 0);

    ini.set_acl<cfg::context::password>("VerySecurePassword");
    BOOST_CHECK(!ini.is_asked<cfg::context::password>());
    auto s = string_from_authid(AUTHID_CONTEXT_PASSWORD);
    stream.out_copy_bytes(s, strlen(s)); stream.out_copy_bytes("\nASK\n", 5);
    AclSerializer::ArrayItemsView view{stream.get_data(), stream.get_data() + stream.get_offset()};
    acl.in_items(view);
    BOOST_CHECK(ini.is_asked<cfg::context::password>());
}
