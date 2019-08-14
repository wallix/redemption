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
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "acl/acl_serializer.hpp"
#include "configs/config.hpp"
#include "main/version.hpp"
#include "utils/arcsight.hpp"
#include "utils/genfstat.hpp"
#include "transport/file_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/log_buffered.hpp"

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
    class NullTransport : public Transport
    {
        void do_send(const uint8_t * const /*buffer*/, size_t /*len*/) override
        {
            // TEST test
        }
    };

    Inifile ini;
    NullTransport trans;
    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));
    ini.set<cfg::context::forcemodule>(true);
    RED_CHECK_NO_THROW(acl.send_acl_data());

    struct ThrowTransport : Transport
    {
        void do_send(const uint8_t* /*buffer*/, size_t /*len*/) override
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
    stream.out_copy_bytes(string_from_authid(cfg::globals::auth_user::index));
    stream.out_copy_bytes("\nASK\n"_av);
    stream.out_copy_bytes(string_from_authid(cfg::context::password::index));
    stream.out_copy_bytes("\nASK\n"_av);

    stream.out_copy_bytes(string_from_authid(cfg::context::session_id::index));
    stream.out_copy_bytes("\n!6455\n"_av);
    stream.stream_at(0).out_uint32_be(stream.get_offset() - 4);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(stream.get_bytes());
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
    OutStream big_stream({u.get(), sz});
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(0xFFFF);
    big_stream.out_copy_bytes(string_from_authid(cfg::globals::auth_user::index));
    big_stream.out_copy_bytes("\n!"_av);
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
    big_stream.out_copy_bytes("a\n"_av);

    GeneratorTransport transexcpt({u.get(), big_stream.get_offset()});
    transexcpt.disable_remaining_error();
    AclSerializer aclexcpt(ini, 10010, transexcpt, cctx, rnd, fstat, to_verbose_flags(0));
    RED_CHECK_EXCEPTION_ERROR_ID(aclexcpt.incoming(), ERR_ACL_MESSAGE_TOO_BIG);
}

RED_AUTO_TEST_CASE(TestAclSerializerIncoming)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s = str_concat(
        "----",
        string_from_authid(cfg::context::password::index),
        "\nASK\n",
        string_from_authid(cfg::globals::auth_user::index),
        "\n!didier\n");
    OutStream({&s[0], 4}).out_uint32_be(s.size() - 4u);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(s);
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    RED_CHECK(not ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "");

    ini.ask<cfg::context::opt_bpp>();
    ini.set_acl<cfg::context::reporting>("didier");

    acl.incoming();

    RED_CHECK(ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "didier");
}


RED_AUTO_TEST_CASE(TestAclSerializeSendBigData)
{
    Inifile ini;
    ini.clear_send_index();

    size_t const k64 = 64 * 1024 - 1;
    size_t const sz_string = 1024*66;
    auto const key = string_from_authid(cfg::context::rejected::index);
    auto const total_sz = sz_string + 8u + key.size() + 3;
    std::unique_ptr<char[]> u(new char[total_sz]);
    OutStream big_stream({u.get(), total_sz});
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(0xffff - 4);
    big_stream.out_copy_bytes(key);
    big_stream.out_copy_bytes("\n!"_av);
    memset(big_stream.get_current(), 'a', k64 - big_stream.get_offset());
    auto subsz = k64 - big_stream.get_offset();
    big_stream.out_skip_bytes(subsz);
    big_stream.out_uint16_be(0);
    big_stream.out_uint16_be(sz_string - subsz + 1);
    memset(big_stream.get_current(), 'a', sz_string - subsz);
    big_stream.out_skip_bytes(sz_string - subsz);
    big_stream.out_copy_bytes("\n"_av);

    RED_REQUIRE_EQUAL(total_sz, big_stream.get_offset());

    CheckTransport trans({u.get(), big_stream.get_offset()});

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
    auto const total_sz = sz_string + 8u + key.size() + 3;
    std::unique_ptr<char[]> u(new char[total_sz]);
    OutStream big_stream({u.get(), total_sz});
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(0xffff - 4);
    big_stream.out_copy_bytes(key);
    big_stream.out_copy_bytes("\n!"_av);
    memset(big_stream.get_current(), 'a', k64 - big_stream.get_offset());
    auto subsz = k64 - big_stream.get_offset();
    big_stream.out_skip_bytes(subsz);
    big_stream.out_uint16_be(0);
    big_stream.out_uint16_be(sz_string - subsz + 1);
    memset(big_stream.get_current(), 'a', sz_string - subsz);
    big_stream.out_skip_bytes(sz_string - subsz);
    big_stream.out_copy_bytes("\n"_av);

    RED_REQUIRE_EQUAL(total_sz, big_stream.get_offset());

    GeneratorTransport trans({u.get(), big_stream.get_offset()});

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

RED_AUTO_TEST_CASE(TestAclSerializeReceiveKeyMultiPacket)
{
    Inifile ini;
    ini.clear_send_index();

    auto const key1 = string_from_authid(cfg::context::rejected::index);
    auto const key2 = string_from_authid(cfg::context::message::index);
    size_t const key2_splitted_len = key2.size() / 2;
    auto const total_sz = 4 * 2 + key1.size() + key2.size() + 5 * 2;
    std::unique_ptr<char[]> u(new char[total_sz]);
    OutStream big_stream({u.get(), total_sz});
    big_stream.out_uint16_be(1);
    big_stream.out_uint16_be(key1.size() + 5 + key2_splitted_len);
    big_stream.out_copy_bytes(key1);
    big_stream.out_copy_bytes("\n!aa\n"_av);
    big_stream.out_copy_bytes(key1.first(key2_splitted_len));
    big_stream.out_uint16_be(0);
    big_stream.out_uint16_be(key2.size() - key2_splitted_len + 5);
    big_stream.out_copy_bytes(key2.from_at(key2_splitted_len));
    big_stream.out_copy_bytes("\n!xy\n"_av);

    RED_REQUIRE_EQUAL(total_sz, big_stream.get_offset());

    GeneratorTransport trans({u.get(), big_stream.get_offset()});

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    RED_CHECK_EXCEPTION_ERROR_ID(acl.incoming(), ERR_ACL_UNEXPECTED_IN_ITEM_OUT);

    trans.disable_remaining_error();
}

RED_AUTO_TEST_CASE(TestAclSerializeUnknownKey)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s("----abcd\n!something\nefg\n!other something\n");
    OutStream({&s[0], 4}).out_uint32_be(s.size() - 4u);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(s);
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0));

    RED_CHECK(not ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "");

    ini.ask<cfg::context::opt_bpp>();
    ini.set_acl<cfg::context::reporting>("didier");

    {
        tu::log_buffered logbuf;
        acl.incoming();
        RED_CHECK_EQ(logbuf.buf(),
            "WARNING -- Unexpected receving 'abcd' - 'something'\n"
            "WARNING -- Unexpected receving 'efg' - 'other something'\n");
    }

    RED_CHECK(ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "didier");
}


RED_AUTO_TEST_CASE_WD(TestAclSerializeLog, wd)
{
    Inifile ini;
    ini.clear_send_index();

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    const timeval time = {0, 0};

    ini.set_acl<cfg::globals::auth_user>("admin");
    ini.set_acl<cfg::globals::target_user>("user1");
    ini.set_acl<cfg::globals::host>("10.10.13.12");
    ini.set<cfg::session_log::enable_arcsight_log>(true);
    // ini.set_acl<cfg::globals::target_host>("13.12.10.10");
    // ini.set_acl<cfg::globals::ip_target>("13.12.10.10");
    // ini.set_acl<cfg::globals::session_id>("0x520");

    auto logfile = wd.add_file("log5_6.log");
    auto hashdir = wd.create_subdirectory("hash");
    auto hashlog = hashdir.add_file("log5_6.log");

    ini.set<cfg::session_log::log_path>(logfile.string());
    ini.set<cfg::video::hash_path>(hashdir.dirname().string());

    GeneratorTransport trans(""_av);
    AclSerializer acl(ini, 10010, trans, cctx, rnd, fstat, to_verbose_flags(0x20));

    acl.start_session_log();

    // MOD SESSION_CREATION FAIL
    {
        tu::log_buffered logbuf;

        acl.log6(LogId::SESSION_CREATION_FAILED, time, {
            KVLog::arcsight("app"_av, "vnc"_av),
            KVLog::arcsight("WallixBastionStatus"_av, "FAIL"_av),
        });

        auto expected6 = cstr_array_view("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"SESSION_CREATION_FAILED\"\nJan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_CREATION|5|WallixBastionUser=admin WallixBastionAccount=user1 WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionSession_id= WallixBastionSessionType=Neutral app=vnc WallixBastionStatus=FAIL\n");
        RED_CHECK_SMEM(logbuf.buf(), expected6);
    }

    // CONNECTION FAIL
    {
        tu::log_buffered logbuf;

        acl.log6(LogId::CONNECTION_FAILED, time, {
            KVLog::arcsight("app"_av, "xup"_av),
            KVLog::direction(LogDirection::ServerDst),
            KVLog::arcsight("WallixBastionStatus"_av, "FAIL"_av),
        });


        auto expected6 = cstr_array_view("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CONNECTION_FAILED\"\nJan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|0|CONNECTION|5|WallixBastionUser=admin WallixBastionAccount=user1 WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionSession_id= WallixBastionSessionType=Neutral suser=admin duser=user1 src=10.10.13.12 dst= app=xup WallixBastionStatus=FAIL\n");

        RED_CHECK_SMEM(logbuf.buf(), expected6);
    }

    // DRIVE_REDIRECTION_RENAME
    {
        tu::log_buffered logbuf;

        acl.log6(LogId::DRIVE_REDIRECTION_RENAME, time, {
            KVLog::all("oldFilePath"_av, "/dir/old_file.ext"_av),
            KVLog::all("filePath"_av, "/dir/new_file.ext"_av),
            KVLog::arcsight("app"_av, "rdp"_av),
        });

        auto expected6 = cstr_array_view("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_RENAME\"\nJan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|0|DRIVE_REDIRECTION_RENAME|5|WallixBastionUser=admin WallixBastionAccount=user1 WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionSession_id= WallixBastionSessionType=Neutral app=rdp oldFilePath=/dir/old_file.ext filePath=/dir/new_file.ext\n");

        RED_CHECK_SMEM(logbuf.buf(), expected6);
    }

    acl.close_session_log();
}


RED_AUTO_TEST_CASE_WD(TestSessionLogFile, wd)
{
    std::string const logname = "acl_log.txt";
    auto filename = wd.add_file(logname);
    auto hashdir = wd.create_subdirectory("hash");
    auto hashname = hashdir.add_file(logname);

    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    SessionLogFile log_file(cctx, rnd, fstat, ReportError([](Error e){
        RED_FAIL(e.errmsg());
        return e;
    }));

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);          // for localtime

    log_file.open(filename, hashname, 0, logname);
    log_file.write_line(1512484183, cstr_array_view("test first"));
    log_file.write_line(1512484185, cstr_array_view("test second"));
    log_file.close();

    RED_CHECK_FILE_CONTENTS(filename,
        "2017-12-05 15:29:43 test first\n"
        "2017-12-05 15:29:45 test second\n"_av);

    InFileTransport t(unique_fd{open(hashname.c_str(), O_RDONLY)});
    MwrmReader mwrm_reader(t);
    MetaLine meta_line;
    mwrm_reader.set_header({WrmVersion::v2, false});
    mwrm_reader.read_meta_hash_line(meta_line);
    RED_CHECK_EQ(meta_line.filename, logname);
    RED_CHECK(not meta_line.with_hash);
    RED_CHECK_EQ(meta_line.size, 63);
}
