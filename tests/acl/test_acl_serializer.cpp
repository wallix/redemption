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

#include "core/log_id.hpp"
#include "acl/acl_serializer.hpp"
#include "configs/config.hpp"
#include "main/version.hpp"
#include "utils/genfstat.hpp"
#include "utils/sugar/algostring.hpp"
#include "transport/file_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/log_buffered.hpp"
#include "acl/sesman.hpp"

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
    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    TimeBase timebase({0, 0});

    AclSerializer acl(ini);
    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);

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
    AclSerializer aclexcpt(ini);

    aclexcpt.set_auth_trans(&transexcpt);

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
    stream.out_copy_bytes("login\nASK\n"_av);
    stream.out_copy_bytes("password\nASK\n"_av);

    stream.out_copy_bytes("session_id\n!6455\n"_av);
    stream.stream_at(0).out_uint32_be(stream.get_offset() - 4);

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    TimeBase timebase({0, 0});

    GeneratorTransport trans(stream.get_produced_bytes());
    AclSerializer acl(ini);

    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);
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
    big_stream.out_copy_bytes("login\n!"_av);
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
    AclSerializer aclexcpt(ini);

    aclexcpt.set_auth_trans(&transexcpt);
    RED_CHECK_EXCEPTION_ERROR_ID(aclexcpt.incoming(), ERR_ACL_MESSAGE_TOO_BIG);
}

RED_AUTO_TEST_CASE(TestAclSerializerIncoming)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s = str_concat(
        "----",
        "password\nASK\n",
        "login\n!didier\n");
    OutStream({&s[0], 4}).out_uint32_be(s.size() - 4u);

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    TimeBase timebase({0, 0});

    GeneratorTransport trans(s);
    AclSerializer acl(ini);

    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);

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
    TimeBase timebase({0, 0});

    size_t const k64 = 64 * 1024 - 1;
    size_t const sz_string = 1024*66;
    auto const key = "rejected"_av;
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

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini);

    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);

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
    auto const key = "rejected"_av;
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

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    TimeBase timebase({0, 0});

    AclSerializer acl(ini);

    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);

    std::string result(sz_string, 'a');
    RED_REQUIRE_NE(ini.get<cfg::context::rejected>(), result);

    acl.incoming();

    RED_REQUIRE_EQUAL(ini.get<cfg::context::rejected>(), result);
}

RED_AUTO_TEST_CASE(TestAclSerializeReceiveKeyMultiPacket)
{
    Inifile ini;
    ini.clear_send_index();

    auto const key1 = "rejected"_av;
    auto const key2 = "message"_av;
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
    big_stream.out_copy_bytes(key2.from_offset(key2_splitted_len));
    big_stream.out_copy_bytes("\n!xy\n"_av);

    RED_REQUIRE_EQUAL(total_sz, big_stream.get_offset());

    GeneratorTransport trans({u.get(), big_stream.get_offset()});

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    TimeBase timebase({0, 0});

    AclSerializer acl(ini);

    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);

    RED_CHECK_EXCEPTION_ERROR_ID(acl.incoming(), ERR_ACL_UNEXPECTED_IN_ITEM_OUT);

    trans.disable_remaining_error();
}

RED_AUTO_TEST_CASE(TestAclSerializeUnknownKey)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s("----abcd\n!something\nefg\n!other something\n");
    OutStream({&s[0], 4}).out_uint32_be(s.size() - 4u);
    TimeBase timebase({0, 0});

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(s);
    AclSerializer acl(ini);

    Sesman sesman(ini, timebase);

    acl.set_auth_trans(&trans);

    RED_CHECK(not ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "");

    ini.ask<cfg::context::opt_bpp>();
    ini.set_acl<cfg::context::reporting>("didier");

    {
        tu::log_buffered logbuf;
        acl.incoming();
        RED_CHECK_EQ(logbuf.buf(),
            "WARNING -- Unexpected receive 'abcd' - 'something'\n"
            "WARNING -- Unexpected receive 'efg' - 'other something'\n");
    }

    RED_CHECK(ini.is_asked<cfg::context::opt_bpp>());
    RED_CHECK_EQUAL(ini.get<cfg::context::reporting>(), "didier");
}

