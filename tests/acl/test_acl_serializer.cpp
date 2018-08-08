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

#define RED_TEST_MODULE TestAclSerializer
#include "system/redemption_unit_tests.hpp"


#include "acl/acl_serializer.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/lcg_random.hpp"
// Class ACL Serializer is used to Modify config file content from a remote ACL manager
// - Send given fields from config
// - Recover fields from network and update Config

inline void init_keys(CryptoContext & cctx)
{
    cctx.set_master_key(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    ));
    cctx.set_hmac_key(cstr_array_view(
         "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
         "\x49\x21\x57\x87\x47\x74\x08\x8a"
         "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
         "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
    ));
}

RED_AUTO_TEST_CASE(TestAclSerializeAskNextModule)
{
    Inifile ini;
    LogTransport trans;
    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));
    ini.set<cfg::context::forcemodule>(true);
    RED_CHECK_NO_THROW(acl.send_acl_data());

    struct ThrowTransport : Transport
    {
        void do_send(const uint8_t*, size_t) override
        {
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
    };
    ThrowTransport transexcpt;
    AclSerializer aclexcpt(ini, 10010, transexcpt, cctx, rnd, fstat, to_verbose_flags(0));

    ini.set_acl<cfg::globals::auth_user>("Newuser");
    aclexcpt.send_acl_data();
    RED_CHECK(!ini.get<cfg::context::authenticated>());
    RED_CHECK_EQUAL(ini.get<cfg::context::rejected>(), "Authentifier service failed");
}

RED_AUTO_TEST_CASE(TestAclSerializeIncoming)
{
    Inifile ini;
    StaticOutStream<1024> stream;
    // NORMAL CASE WITH SESSION ID CHANGE
    stream.out_uint32_be(0);
    stream.out_string(string_from_authid(AUTHID_GLOBALS_AUTH_USER)); stream.out_string("\nASK\n");
    stream.out_string(string_from_authid(AUTHID_CONTEXT_PASSWORD)); stream.out_string("\nASK\n");

    stream.out_string(string_from_authid(AUTHID_CONTEXT_SESSION_ID)); stream.out_string("\n!6455\n");
    stream.set_out_uint32_be(stream.get_offset() - 4 ,0);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(stream.get_data(), stream.get_offset());
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));
    ini.set<cfg::context::session_id>("");
    ini.set_acl<cfg::globals::auth_user>("testuser");
    RED_CHECK(ini.get<cfg::context::session_id>().empty());
    RED_CHECK(!ini.is_asked<cfg::globals::auth_user>());

    RED_CHECK_NO_THROW(acl.incoming());
    RED_CHECK(ini.is_asked<cfg::globals::auth_user>());
    RED_CHECK(!ini.get<cfg::context::session_id>().empty());

    // CASE EXCEPTION
    // try exception
    size_t const k64 = 64 * 1024 - 1;
    size_t const sz = 1024 * 1024 * 2;
    std::unique_ptr<char[]> u(new char[sz]);
    OutStream big_stream(u.get(), sz);
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(0xFFFF);
    big_stream.out_string(string_from_authid(cfg::globals::auth_user::index));
    big_stream.out_string("\n!");
    memset(big_stream.get_current(), 'a', k64 - big_stream.get_offset());
    big_stream.out_skip_bytes(k64 - big_stream.get_offset());

    RED_CHECK_EQUAL(big_stream.get_offset(), k64);

    while(big_stream.tailroom() > k64 + 1) {
        big_stream.out_uint16_be(1);
        big_stream.out_uint16_be(0xFFFF);
        memset(big_stream.get_current(), 'a', k64 - 4u);
        big_stream.out_skip_bytes(k64 - 4u);
    }
    big_stream.out_uint16_be(0);
    big_stream.out_uint16_be(0x2);
    big_stream.out_string("a\n");

    GeneratorTransport transexcpt(u.get(), big_stream.get_offset());
    transexcpt.disable_remaining_error();
    AclSerializer aclexcpt(ini, 10010, transexcpt, cctx, rnd, fstat, to_verbose_flags(0));
    RED_CHECK_EXCEPTION_ERROR_ID(aclexcpt.incoming(), ERR_ACL_MESSAGE_TOO_BIG);
}

RED_AUTO_TEST_CASE(TestAclSerializerIncoming)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s("----");
    s += string_from_authid(cfg::context::password::index);
    s += "\nASK\n";
    s += string_from_authid(cfg::globals::auth_user::index);
    s += "\n!didier\n";
    OutStream(&s[0], 4).out_uint32_be(s.size() - 4u);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(s.data(), s.size());
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    RED_CHECK_EQUAL(ini.is_asked<cfg::context::opt_bpp>(), false);
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "");

    ini.ask<cfg::context::opt_bpp>();
    ini.set_acl<cfg::context::reporting>("didier");

    acl.incoming();

    RED_CHECK_EQUAL(ini.is_asked<cfg::context::opt_bpp>(), true);
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "didier");
}


RED_AUTO_TEST_CASE(TestAclSerializeSendBigData)
{
    Inifile ini;
    ini.clear_send_index();

    size_t const k64 = 64 * 1024 - 1;
    size_t const sz_string = 1024*66;
    auto const key = string_from_authid(cfg::context::rejected::index);
    auto const total_sz = sz_string + 8u + strlen(key) + 3;
    std::unique_ptr<char[]> u(new char[total_sz]);
    OutStream big_stream(u.get(), total_sz);
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(0xffff - 4);
    big_stream.out_string(key);
    big_stream.out_string("\n!");
    memset(big_stream.get_current(), 'a', k64 - big_stream.get_offset());
    auto subsz = k64 - big_stream.get_offset();
    big_stream.out_skip_bytes(subsz);
    big_stream.out_uint16_be(0);
    big_stream.out_uint16_be(sz_string - subsz + 1);
    memset(big_stream.get_current(), 'a', sz_string - subsz);
    big_stream.out_skip_bytes(sz_string - subsz);
    big_stream.out_string("\n");

    RED_REQUIRE_EQUAL(total_sz, big_stream.get_offset());

    CheckTransport trans(u.get(), big_stream.get_offset());

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    ini.set_acl<cfg::context::rejected>(std::string(sz_string, 'a'));

    RED_REQUIRE_EQUAL(ini.get<cfg::context::rejected>().size(), sz_string);
    RED_REQUIRE_EQUAL(ini.changed_field_size(), 1);

    acl.send_acl_data();
}

RED_AUTO_TEST_CASE(TestAclSerializeReceiveBigData)
{
    Inifile ini;
    ini.clear_send_index();

    size_t const k64 = 64 * 1024 - 1;
    size_t const sz_string = 1024*66;
    auto const key = string_from_authid(cfg::context::rejected::index);
    auto const total_sz = sz_string + 8u + strlen(key) + 3;
    std::unique_ptr<char[]> u(new char[total_sz]);
    OutStream big_stream(u.get(), total_sz);
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(0xffff - 4);
    big_stream.out_string(key);
    big_stream.out_string("\n!");
    memset(big_stream.get_current(), 'a', k64 - big_stream.get_offset());
    auto subsz = k64 - big_stream.get_offset();
    big_stream.out_skip_bytes(subsz);
    big_stream.out_uint16_be(0);
    big_stream.out_uint16_be(sz_string - subsz + 1);
    memset(big_stream.get_current(), 'a', sz_string - subsz);
    big_stream.out_skip_bytes(sz_string - subsz);
    big_stream.out_string("\n");

    RED_REQUIRE_EQUAL(total_sz, big_stream.get_offset());

    GeneratorTransport trans(u.get(), big_stream.get_offset());

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    std::string result(sz_string, 'a');
    RED_REQUIRE_NE(ini.get<cfg::context::rejected>(), result);

    acl.incoming();

    RED_REQUIRE_EQUAL(ini.get<cfg::context::rejected>(), result);
}

RED_AUTO_TEST_CASE(TestAclSerializeUnknownKey)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s("----abcd\n!something\nefg\n!other something\n");
    OutStream(&s[0], 4).out_uint32_be(s.size() - 4u);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(s.data(), s.size());
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    RED_CHECK_EQUAL(ini.is_asked<cfg::context::opt_bpp>(), false);
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "");

    ini.ask<cfg::context::opt_bpp>();
    ini.set_acl<cfg::context::reporting>("didier");

    {
        LOG__REDEMPTION__BUFFERED logbuf;
        acl.incoming();
        RED_CHECK_EQ(logbuf.buf(),
            "WARNING - Unexpected receving 'abcd' - 'something'\n"
            "WARNING - Unexpected receving 'efg' - 'other something'\n");
    }

    RED_CHECK_EQUAL(ini.is_asked<cfg::context::opt_bpp>(), true);
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "didier");
}


RED_AUTO_TEST_CASE(TestAclSerializeLog)
{
    Inifile ini;
    ini.clear_send_index();

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;

    ini.set_acl<cfg::globals::auth_user>("admin");
    ini.set_acl<cfg::globals::target_user>("user1");
    ini.set_acl<cfg::globals::host>("10.10.13.12");

    {
        char const* logfile = "/tmp/test_acl_dir/log5_6.log";
        char const* hashdir = "/tmp/test_acl_dir/hash/";
        char const* hashlog = "/tmp/test_acl_dir/hash/log5_6.log";
        unlink(logfile);
        unlink(hashlog);
        ini.set<cfg::session_log::log_path>(logfile);
        ini.set<cfg::video::hash_path>(hashdir);
    }

    const std::string red_version(VERSION);

    GeneratorTransport trans("", 0);
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, AclSerializer::Verbose::log_arcsight);

    const size_t date_length(21);

    acl.start_session_log();

    // MOD SESSION_CREATION FAIL
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"SESSION_CREATION_FAILED\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"SESSION_CREATION_FAILED\"\n");
        //RED_CHECK_EQ(logbuf.buf().length(), expected5.length());

        //LOG(LOG_INFO, "logbuf.buf=%s", logbuf.buf());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "SESSION_CREATION";
        arc_info.ApplicationProtocol = "vnc";
        arc_info.WallixBastionStatus = "FAIL";
        acl.log6("type=\"SESSION_CREATION_FAILED\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_CREATION|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=vnc WallixBastionStatus=FAIL\n");
        //RED_CHECK_EQ(undated_buf.length(), expected6.length());
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // CONNECTION FAIL
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"CONNECTION_FAILED\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CONNECTION_FAILED\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "CONNECTION";
        arc_info.ApplicationProtocol = "xup";
        arc_info.WallixBastionStatus = "FAIL";
        acl.log6("type=\"CONNECTION_FAILED\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|CONNECTION|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=xup WallixBastionStatus=FAIL\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // NEW_REMOTE_MOD
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"NEW_REMOTE_MOD\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"NEW_REMOTE_MOD\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "NEW_REMOTE_MOD";
        acl.log6("type=\"NEW_REMOTE_MOD\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|NEW_REMOTE_MOD|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst= \n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // CERTIFICATE
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"CERTIFICATE_CHECK_SUCCESS\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CERTIFICATE_CHECK_SUCCESS\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "CERTIFICATE_CHECK";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.WallixBastionStatus = "SUCCESS";
        arc_info.message = "Connexion to server allowed";
        acl.log6("type=\"CERTIFICATE_CHECK_SUCCESS\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|CERTIFICATE_CHECK|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp WallixBastionStatus=SUCCESS msg=\"Connexion<space>to<space>server<space>allowed\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // SESSION_DISCONNECTION
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"SESSION_DISCONNECTION\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"SESSION_DISCONNECTION\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "SESSION_DISCONNECTION";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.message = "duration:906";
        acl.log6("type=\"SESSION_DISCONNECTION\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_DISCONNECTION|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp msg=\"duration:906\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // SESSION_ESTABLISHED
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"SESSION_ESTABLISHED_SUCCESSFULLY\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"SESSION_ESTABLISHED_SUCCESSFULLY\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "SESSION_ESTABLISHED";
        arc_info.ApplicationProtocol = "rdp";
         arc_info.WallixBastionStatus = "SUCCESS";
        acl.log6("type=\"SESSION_ESTABLISHED_SUCCESSFULLY\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_ESTABLISHED|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp WallixBastionStatus=SUCCESS\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // DRIVE_REDIRECTION_USE
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"DRIVE_REDIRECTION_USE\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_USE\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "DRIVE_REDIRECTION_USE";
        arc_info.ApplicationProtocol = "rdp";
         arc_info.message = "device_name:winxp device_type=win_server";
        acl.log6("type=\"DRIVE_REDIRECTION_USE\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|DRIVE_REDIRECTION_USE|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp msg=\"device_name:winxp<space>device_type\\=win_server\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // DRIVE_REDIRECTION_USE
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"DRIVE_REDIRECTION_READ\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_READ\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "DRIVE_REDIRECTION_READ";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.filePath = "/dir/file.ext";
        acl.log6("type=\"DRIVE_REDIRECTION_READ\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|DRIVE_REDIRECTION_READ|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp filePath=/dir/file.ext\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // DRIVE_REDIRECTION _READ/_WRITE/_DELETE
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"DRIVE_REDIRECTION_READ\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_READ\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "DRIVE_REDIRECTION_READ";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.filePath = "/dir/file.ext";
        acl.log6("type=\"DRIVE_REDIRECTION_READ\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|DRIVE_REDIRECTION_READ|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp filePath=/dir/file.ext\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // DRIVE_REDIRECTION_RENAME
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"DRIVE_REDIRECTION_RENAME\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_RENAME\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "DRIVE_REDIRECTION_RENAME";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.oldFilePath = "/dir/old_file.ext";
        arc_info.filePath = "/dir/new_file.ext";
        acl.log6("type=\"DRIVE_REDIRECTION_RENAME\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|DRIVE_REDIRECTION_RENAME|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp oldFilePath=/dir/old_file.ext filePath=/dir/new_file.ext\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // CB_COPYING_PASTING_FILE _TO_REMOTE_SESSION/_FROM_REMOTE_SESSION
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.filePath = "/dir/file.ext";
        arc_info.fileSize = "1312";
        acl.log6("type=\"CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp filePath=/dir/file.ext fsize=1312\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // CLIENT_EXECUTE_REMOTEAPP
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"CLIENT_EXECUTE_REMOTEAPP\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CLIENT_EXECUTE_REMOTEAPP\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "CLIENT_EXECUTE_REMOTEAPP";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.filePath = "/dir/file.exe";
        acl.log6("type=\"CLIENT_EXECUTE_REMOTEAPP\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|CLIENT_EXECUTE_REMOTEAPP|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp filePath=/dir/file.exe\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // SESSION_ENDING_IN_PROGRESS
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"SESSION_ENDING_IN_PROGRESS\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"SESSION_ENDING_IN_PROGRESS\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "SESSION_ENDING_IN_PROGRESS";
        arc_info.ApplicationProtocol = "rdp";
        acl.log6("type=\"SESSION_ENDING_IN_PROGRESS\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_ENDING_IN_PROGRESS|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // PASSWORD_TEXT_BOX_GET_FOCUS
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"PASSWORD_TEXT_BOX_GET_FOCUS\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"PASSWORD_TEXT_BOX_GET_FOCUS\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "PASSWORD_TEXT_BOX_GET_FOCUS";
        arc_info.WallixBastionStatus = "OnFocus";
        arc_info.ApplicationProtocol = "rdp";
        acl.log6("type=\"PASSWORD_TEXT_BOX_GET_FOCUS\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|PASSWORD_TEXT_BOX_GET_FOCUS|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp WallixBastionStatus=OnFocus\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // INPUT_LANGUAGE
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"INPUT_LANGUAGE\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"INPUT_LANGUAGE\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "INPUT_LANGUAGE";
        arc_info.message = "identifier:1 display_name:eng";
        arc_info.ApplicationProtocol = "rdp";
        acl.log6("type=\"INPUT_LANGUAGE\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|INPUT_LANGUAGE|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp msg=\"identifier:1<space>display_name:eng\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // STARTUP_APPLICATION
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"STARTUP_APPLICATION\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"STARTUP_APPLICATION\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "STARTUP_APPLICATION";
        arc_info.WallixBastionStatus = "FAIL_TO_RUN";
        arc_info.ApplicationProtocol = "rdp";
        arc_info.message = "application_name:notepad raw_result:-3698768";
        acl.log6("type=\"STARTUP_APPLICATION\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|STARTUP_APPLICATION|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  app=rdp WallixBastionStatus=FAIL_TO_RUN msg=\"application_name:notepad<space>raw_result:-3698768\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // KBD_INPUT
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"KBD_INPUT\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"KBD_INPUT\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "KBD_INPUT";
        acl.log6("type=\"KBD_INPUT\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|KBD_INPUT|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst= \n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // CHANGE_TITLE_BAR
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"CHANGE_TITLE_BAR\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CHANGE_TITLE_BAR\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "CHANGE_TITLE_BAR";
        arc_info.message = "title:new_title";
        acl.log6("type=\"CHANGE_TITLE_BAR\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|CHANGE_TITLE_BAR|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  msg=\"title:new_title\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    // KILL_PATTERN_DETECTED / NOTIFY_PATTERN_DETECTED
    {
        LOG__REDEMPTION__BUFFERED logbuf;

        acl.log5("type=\"KILL_PATTERN_DETECTED\"");
        std::string expected5("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"KILL_PATTERN_DETECTED\"\n");
        RED_CHECK_EQ(logbuf.buf().length(), expected5.length());
        RED_CHECK_EQ(logbuf.buf(), expected5);
        logbuf.clear();

        ArcsightLogInfo arc_info;
        arc_info.name = "KILL_PATTERN_DETECTED";
        arc_info.message = "message";
        acl.log6("type=\"KILL_PATTERN_DETECTED\"", arc_info);
        std::string undated_buf(logbuf.buf().substr(date_length+expected5.length(), logbuf.buf().length()));
        auto expected6 = cstr_array_view("host message CEF:1|Wallix|Bastion|" VERSION "|0|KILL_PATTERN_DETECTED|5|suser=admin duser=user1 WallixBastionSession_id= WallixBastionSessionType=Neutral src=10.10.13.12 dst=  msg=\"message\"\n");
        RED_CHECK_SMEM(undated_buf, expected6);
    }

    acl.close_session_log();
}


#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include "test_only/get_file_contents.hpp"

RED_AUTO_TEST_CASE(TestSessionLogFile)
{
    std::string const prefix_path = "/tmp/test_acl_dir/";
    std::string const hashdir = prefix_path + "hash/";
    std::string const logname = "acl_log.txt";
    std::string const filename = prefix_path + logname;
    std::string const hashname = hashdir + logname;

    ::unlink(filename.c_str());
    ::unlink(hashname.c_str());
    mkdir(prefix_path.c_str(), 0777);
    mkdir(hashdir.c_str(), 0777);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    SessionLogFile log_file(cctx, rnd, fstat, ReportError([](Error e){
        RED_CHECK(false);
        return e;
    }));

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);          // for localtime

    log_file.open(filename, hashname, 0, logname);
    log_file.write_line(1512484183, cstr_array_view("test first"));
    log_file.write_line(1512484185, cstr_array_view("test second"));
    log_file.close();

    RED_CHECK_EQ(get_file_contents(filename),
        "2017-12-05 15:29:43 test first\n"
        "2017-12-05 15:29:45 test second\n");

    InFileTransport t(unique_fd{open(hashname.c_str(), O_RDONLY)});
    MwrmReader mwrm_reader(t);
    MetaLine meta_line;
    mwrm_reader.set_header({WrmVersion::v2, false});
    mwrm_reader.read_meta_hash_line(meta_line);
    RED_CHECK_EQ(meta_line.filename, logname);
    RED_CHECK_EQ(meta_line.with_hash, false);
    RED_CHECK_EQ(meta_line.size, 63);
}
