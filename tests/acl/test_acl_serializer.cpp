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
   Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong

   Unit tests for Acl Serializer
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/log_buffered.hpp"

#include "acl/acl_serializer.hpp"
#include "configs/config.hpp"

#include <string_view>


// Class ACL Serializer is used to Modify config file content from a remote ACL manager
// - Send given fields from config
// - Recover fields from network and update Config

RED_AUTO_TEST_CASE(TestAclSerializeAskNextModule)
{
    Inifile ini;

    struct TestTransport : Transport
    {
        BufTransport trans;
        bool excep = false;

        void do_send(const uint8_t* buffer, size_t len) override
        {
            if (excep) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED);
            }
            else {
                trans.send(buffer, len);
            }
        }
    };
    TestTransport trans;

    AclSerializer acl(ini, trans);

    acl.send_acl_data();
    RED_CHECK(trans.trans.buf ==
        "\x00\x17"
        "?\x0ftarget_password"
        "?\x0btarget_host"
        "?\x0aproto_dest"
        "?\x08password"
        "?\x05login"
        "?\x0dtarget_device"
        "?\x0ctarget_login"
        "!\x03""bpp\x00\x00\x00\x02""24"
        "!\x05width\x00\x00\x00\x03""800"
        "!\x06height\x00\x00\x00\x03""600"
        "!\x15selector_current_page\x00\x00\x00\x01\x31"
        "!\x16selector_device_filter\x00\x00\x00\x00"
        "!\x15selector_group_filter\x00\x00\x00\x00"
        "!\x15selector_proto_filter\x00\x00\x00\x00"
        "!\x17selector_lines_per_page\x00\x00\x00\x01\x30"
        "!\x09reporting\x00\x00\x00\x00"
        "!\x13""auth_channel_target\x00\x00\x00\x00"
        "!\x0e""accept_message\x00\x00\x00\x05""False"
        "!\x0f""display_message\x00\x00\x00\x05""False"
        "!\x12real_target_device\x00\x00\x00\x00"
        "!\x09ip_client\x00\x00\x00\x00"
        "!\x09ip_target\x00\x00\x00\x00"
        "!\x0elogin_language\x00\x00\x00\x04""Auto"
        ""_av);

    trans.trans.buf.clear();
    trans.excep = true;

    ini.set_acl<cfg::globals::auth_user>("Newuser");

    RED_CHECK_EXCEPTION_ERROR_ID(acl.send_acl_data(),
        ERR_TRANSPORT_WRITE_FAILED);
}

RED_AUTO_TEST_CASE(TestAclSerializeIncoming)
{
    Inifile ini;
    GeneratorTransport trans(
        "\x00\x03"
        "?\x05login"
        "?\x08password"
        "!\x0asession_id\x00\x00\x00\x04""6455"
        ""_av);
    AclSerializer acl(ini, trans);

    ini.set<cfg::context::session_id>("");
    ini.set_acl<cfg::globals::auth_user>("testuser");
    RED_CHECK(ini.get<cfg::context::session_id>().empty());
    RED_CHECK(!ini.is_asked<cfg::globals::auth_user>());

    AclFieldMask acl_fields;
    RED_CHECK_NO_THROW(acl_fields = acl.incoming());
    RED_CHECK(ini.is_asked<cfg::globals::auth_user>());
    RED_CHECK(ini.get<cfg::context::session_id>() == "6455");

    RED_TEST(acl_fields.has<cfg::context::session_id>());
    RED_TEST(!acl_fields.has<cfg::globals::auth_user>());

    acl_fields.clear(cfg::context::session_id::index);
    RED_TEST(acl_fields.is_empty());
}

RED_AUTO_TEST_CASE(TestAclSerializeIncomingMulti)
{
    Inifile ini;
    GeneratorTransport trans(
        "\x00\x03"
        "?\x05login"
        "?\x08password"
        "!\x0asession_id\x00\x00\x00\x04""6455"
        "\x00\x01"
        "!\x0asession_id\x00\x00\x00\x04""1234"
        ""_av);
    AclSerializer acl(ini, trans);

    ini.set<cfg::context::session_id>("");
    ini.set_acl<cfg::globals::auth_user>("testuser");
    RED_CHECK(ini.get<cfg::context::session_id>().empty());
    RED_CHECK(!ini.is_asked<cfg::globals::auth_user>());

    RED_CHECK_NO_THROW(acl.incoming());
    RED_CHECK(ini.is_asked<cfg::globals::auth_user>());
    RED_CHECK(ini.get<cfg::context::session_id>() == "1234");
}

RED_AUTO_TEST_CASE(TestAclSerializeTooBigMessage)
{
    Inifile ini;

    auto prefix =
        "\x00\x01"
        "!\x05login\x00\x10\x00\x01"
        ""_av;
    std::string message(1024*1024+1 + prefix.size(), 'a');
    memcpy(message.data(), prefix.data(), prefix.size());

    GeneratorTransport trans(message);
    trans.disable_remaining_error();
    AclSerializer acl(ini, trans);

    RED_CHECK_EXCEPTION_ERROR_ID(acl.incoming(), ERR_ACL_MESSAGE_TOO_BIG);
}

RED_AUTO_TEST_CASE(TestAclSerializeSendBigData)
{
    Inifile ini;
    ini.clear_acl_fields_changed();

    size_t const sz_string = 1024*66;
    auto prefix =
        "\x00\x01"
        "!\x08password\x00\x01\x08\x00"
        ""_av;
    std::string message(1024*66 + prefix.size(), 'a');
    memcpy(message.data(), prefix.data(), prefix.size());

    BufTransport trans;
    AclSerializer acl(ini, trans);

    ini.set_acl<cfg::context::password>(std::string(sz_string, 'a'));

    RED_CHECK(ini.get<cfg::context::password>().size() == sz_string);
    RED_CHECK(ini.get_acl_fields_changed().size() == 1);
    RED_CHECK(acl.send_acl_data() == 1);
    RED_CHECK(trans.buf == bytes_view(message));
}

RED_AUTO_TEST_CASE(TestAclSerializeReceiveBigData)
{
    Inifile ini;

    size_t const sz_string = 1024*66;
    auto prefix =
        "\x00\x01"
        "!\x08rejected\x00\x01\x08\x00"
        ""_av;
    std::string message(1024*66 + prefix.size(), 'a');
    memcpy(message.data(), prefix.data(), prefix.size());

    GeneratorTransport trans(message);
    AclSerializer acl(ini, trans);

    std::string result(sz_string, 'a');
    RED_REQUIRE_NE(ini.get<cfg::context::rejected>(), result);

    acl.incoming();

    RED_REQUIRE_EQ(ini.get<cfg::context::rejected>(), result);
}

RED_AUTO_TEST_CASE(TestAclSerializeTruncateKeyInternal)
{
    Inifile ini;

    auto prefix =
        "\x00\x02"
        "!\x08rejected\x00\x00\xff\xe8"
        ""_av;
    std::string message(65512 + prefix.size(), 'a');
    memcpy(message.data(), prefix.data(), prefix.size());

    using namespace std::string_view_literals;
    message += "!\x07message\x00\x00\x00\x02xy"sv;

    GeneratorTransport trans(message);
    AclSerializer acl(ini, trans);

    acl.incoming();

    RED_CHECK(ini.get<cfg::context::rejected>().size() == 65512);
    RED_CHECK(ini.get<cfg::context::message>() == "xy");
}

RED_AUTO_TEST_CASE(TestAclSerializeUnknownKey)
{
    Inifile ini;

    GeneratorTransport trans(
        "\x00\x02"
        "!\x04""abcd\x00\x00\x00\x09""something"
        "!\x03""efg\x00\x00\x00\x0f""other something"
        ""_av);
    AclSerializer acl(ini, trans);

    {
        tu::log_buffered logbuf;
        acl.incoming();
        RED_CHECK(logbuf.buf() ==
            "WARNING -- Unexpected receive 'abcd' - 'something'\n"
            "WARNING -- Unexpected receive 'efg' - 'other something'\n"_av);
    }
}
