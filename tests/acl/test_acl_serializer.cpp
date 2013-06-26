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


BOOST_AUTO_TEST_CASE(TestAclSerializerOutItem)
{
    
    Inifile ini;
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);

    // test out_item
    // out_item(Stream stream,const char * key) should add in the Stream a string 
    // composed of key and its value if known in Inifile, 
    // or "ASK" if the value is asked.
    BStream stream(1024);
    acl.out_item(stream, STRAUTHID_PROXY_TYPE);
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data, 
                              STRAUTHID_PROXY_TYPE "\n!RDP\n",
                              strlen(STRAUTHID_PROXY_TYPE "\n!RDP\n")
                              )
                      );
    stream.reset();

    ini.context_ask(STRAUTHID_TARGET_PROTOCOL);
    acl.out_item(stream, STRAUTHID_TARGET_PROTOCOL);
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data, 
                              STRAUTHID_TARGET_PROTOCOL "\nASK\n",
                              strlen(STRAUTHID_TARGET_PROTOCOL "\nASK\n")
                              )
                      );
    stream.reset();

    acl.out_item(stream, STRAUTHID_PASSWORD);    
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data, 
                              STRAUTHID_PASSWORD "\nASK\n",
                              strlen(STRAUTHID_PASSWORD "\nASK\n")
                              )
                      );
    ini.context_set_value(STRAUTHID_PASSWORD,"SecureLinux");
    acl.out_item(stream, STRAUTHID_PASSWORD);
    BOOST_CHECK_EQUAL(0, 
                      strncmp((char*)stream.data+13, 
                              STRAUTHID_PASSWORD "\n!SecureLinux\n",
                              strlen(STRAUTHID_PASSWORD "\n!SecureLinux\n")
                              )
                      );
    stream.reset();
    
}
BOOST_AUTO_TEST_CASE(TestAclSerializeAskNextModule)
{
    Inifile ini;
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);
    try {
        acl.ask_next_module_remote();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(TestAclSerializeIncoming)
{
    Inifile ini;
    BStream stream(1024);
    stream.out_uint32_be(0);
    stream.out_concat(STRAUTHID_AUTH_USER "\nASK\n");
    stream.out_concat(STRAUTHID_PASSWORD "\nASK\n");
    
    GeneratorTransport trans((char *)stream.p,stream.get_offset());
    AclSerializer acl(&ini, trans, 0);
    try {
        acl.incoming();
    } catch (const Error & e){
        BOOST_CHECK(false);
    }
}
inline void execute_test_initem(Stream & stream, AclSerializer & acl, const char * strauthid, const char * value)
{
    // create stream with key , ask
    stream.out_copy_bytes(strauthid, strlen(strauthid));
    stream.out_copy_bytes(value, strlen(value));
    stream.mark_end();
    stream.rewind();
        
    // execute in_item
    acl.in_item(stream);
}
inline void test_initem_ask(BStream & stream, Inifile & ini, AclSerializer & acl, const char * strauthid, const char * defaut)
{
    stream.init(strlen(strauthid) + 5);
    
    // Set defaut value to strauthid key
    ini.context_set_value(strauthid, defaut);
    BOOST_CHECK(!ini.context_is_asked(strauthid));

    execute_test_initem(stream,acl,strauthid,"\nASK\n");
    // check key value is known
    BOOST_CHECK(ini.context_is_asked(strauthid));
}
inline void test_initem_receive(BStream & stream, Inifile & ini, AclSerializer & acl, const char * strauthid, const char * value)
{
    stream.init(strlen(strauthid) + strlen(value));
    // set strauthid key to be asked
    ini.context_ask(strauthid);
    BOOST_CHECK(ini.context_is_asked(strauthid));
    
    execute_test_initem(stream,acl,strauthid,value);
    
    // check key value is known
    BOOST_CHECK(!ini.context_is_asked(strauthid)); 
}
BOOST_AUTO_TEST_CASE(TestAclSerializerInItem)
{
    Inifile ini;
    BStream stream(1);
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);    

    test_initem_ask(stream,ini,acl,STRAUTHID_PASSWORD,"SecureLinux");
    test_initem_ask(stream,ini,acl,STRAUTHID_PROXY_TYPE,"VNC");
    test_initem_ask(stream,ini,acl,STRAUTHID_SELECTOR_CURRENT_PAGE,"");
    test_initem_receive(stream,ini,acl,STRAUTHID_SELECTOR_CURRENT_PAGE,"\n2\n");
    test_initem_receive(stream,ini,acl,STRAUTHID_PASSWORD,"\n!SecureLinux\n");
    test_initem_receive(stream,ini,acl,STRAUTHID_PROXY_TYPE,"\nRDP\n");

}

BOOST_AUTO_TEST_CASE(TestAclSerializerInItems)
{
    Inifile ini;
    BStream stream(1024);
    LogTransport trans;
    AclSerializer acl(&ini, trans, 0);
    
    ini.context_set_value(STRAUTHID_PASSWORD, "VerySecurePassword");
    ini.context_ask(STRAUTHID_PROXY_TYPE);
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PROXY_TYPE));
    stream.out_copy_bytes(STRAUTHID_PASSWORD "\nASK\n", strlen(STRAUTHID_PASSWORD "\nASK\n"));
    stream.out_copy_bytes(STRAUTHID_PROXY_TYPE "\nVNC\n", strlen(STRAUTHID_PROXY_TYPE "\nVNC\n"));
    stream.mark_end();
    stream.rewind();
    acl.in_items(stream);
    BOOST_CHECK(ini.context_is_asked(STRAUTHID_PASSWORD));
    BOOST_CHECK(!ini.context_is_asked(STRAUTHID_PROXY_TYPE));
}
