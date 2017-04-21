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


#define LOGNULL
// #define LOGPRINT

#include "acl/acl_serializer.hpp"
#include "test_only/transport/test_transport.hpp"
#include "../includes/test_only/lcg_random.hpp"
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
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, to_verbose_flags(0));
    ini.set<cfg::context::forcemodule>(true);
    RED_CHECK_NO_THROW(acl.send_acl_data());

    // try exception
    CheckTransport transexcpt("", 0);
    AclSerializer aclexcpt(ini, 10010, transexcpt, cctx, rnd, to_verbose_flags(0));
                                  
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
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(stream.get_data(), stream.get_offset());
    AclSerializer acl(ini, 10010, trans, cctx, rnd, to_verbose_flags(0));
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
    big_stream.out_string(string_from_authid(static_cast<authid_t>(cfg::globals::auth_user::index())));
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
    AclSerializer aclexcpt(ini, 10010, transexcpt, cctx, rnd, to_verbose_flags(0));
    RED_CHECK_EXCEPTION_ERROR_ID(aclexcpt.incoming(), ERR_ACL_MESSAGE_TOO_BIG);
}

RED_AUTO_TEST_CASE(TestAclSerializerIncoming)
{
    Inifile ini;
    ini.clear_send_index();

    std::string s("----");
    s += string_from_authid(static_cast<authid_t>(cfg::context::password::index()));
    s += "\nASK\n";
    s += string_from_authid(static_cast<authid_t>(cfg::globals::auth_user::index()));
    s += "\n!didier\n";
    OutStream(&s[0], 4).out_uint32_be(s.size() - 4u);

    LCGRandom rnd(0);
    CryptoContext cctx;
    init_keys(cctx);

    GeneratorTransport trans(s.data(), s.size());
    AclSerializer acl(ini, 10010, trans, cctx, rnd, to_verbose_flags(0));

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
    auto const key = string_from_authid(static_cast<authid_t>(cfg::context::rejected::index()));
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
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, to_verbose_flags(0));

    ini.set_acl<cfg::context::rejected>(std::string(sz_string, 'a'));

    RED_REQUIRE_EQUAL(ini.get<cfg::context::rejected>().size(), sz_string);
    RED_REQUIRE_EQUAL(ini.changed_field_size(), 1);

    acl.send_acl_data();

    RED_CHECK_EQUAL(trans.get_total_sent(), big_stream.get_offset());
}


RED_AUTO_TEST_CASE(TestAclSerializeReceiveBigData)
{
    Inifile ini;
    ini.clear_send_index();

    size_t const k64 = 64 * 1024 - 1;
    size_t const sz_string = 1024*66;
    auto const key = string_from_authid(static_cast<authid_t>(cfg::context::rejected::index()));
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
    CryptoContext cctx;
    init_keys(cctx);

    AclSerializer acl(ini, 10010, trans, cctx, rnd, to_verbose_flags(0));

    std::string result(sz_string, 'a');
    RED_REQUIRE_NE(ini.get<cfg::context::rejected>(), result);

    acl.incoming();

    RED_REQUIRE_EQUAL(ini.get<cfg::context::rejected>(), result);
}
