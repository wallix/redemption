/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#define RED_TEST_MODULE TestParser
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/autogen/enums.hpp"
REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wunused-function")
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
#include "configs/io.hpp"
#include "configs/autogen/enums_func_ini.tcc"
REDEMPTION_DIAGNOSTIC_POP()

RED_TEST_DELEGATE_PRINT(parse_error, (_ ? _.c_str() : "no error"));
RED_TEST_DELEGATE_PRINT(std::chrono::seconds, _.count());


#include <ostream>

namespace
{
    bool operator == (parse_error const& lhs, parse_error const& rhs)
    {
        return lhs.c_str() == rhs.c_str();
    }

    bool operator != (parse_error const& lhs, parse_error const& rhs)
    {
        return lhs.c_str() != rhs.c_str();
    }

    struct Octal
    {
        unsigned i;

        operator unsigned () const
        {
            return i;
        }

        friend std::ostream& operator<<(std::ostream& out, Octal const& o)
        {
            char buf[16];
            auto r = std::to_chars(buf, buf+16, o.i, 8);
            out << "0";
            return out.write(buf, r.ptr-buf);
        }
    };
}

RED_AUTO_TEST_CASE(TestEnumParser)
{
    char zbuffer[1024];
    auto zbuf_av = make_writable_array_view(zbuffer);

    // increment
    {
        RED_CHECK( is_valid_enum_value<ServerCertCheck>::is_valid(3));
        RED_CHECK(!is_valid_enum_value<ServerCertCheck>::is_valid(4));

        configs::spec_type<ServerCertCheck> spec{};

        ServerCertCheck v{};
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "3"_zv));
        RED_CHECK(v == ServerCertCheck(3));
        RED_CHECK(no_parse_error != parse_from_cfg(v, spec, "4"_zv));
        RED_CHECK(v == ServerCertCheck(3));

        RED_CHECK("2"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<ServerCertCheck>{}, ServerCertCheck(2)));

        configs::spec_type<std::string> sspec{};

        RED_CHECK(v != ServerCertCheck::succeed_if_exists_and_fails_if_missing);
        RED_CHECK(no_parse_error == parse_from_cfg(v, sspec, "sucCEED_IF_EXISTS_ANd_fails_if_MISSING"_zv));
        RED_CHECK(v == ServerCertCheck::succeed_if_exists_and_fails_if_missing);
        RED_CHECK(no_parse_error != parse_from_cfg(v, sspec, "blablabla"_zv));
        RED_CHECK(v == ServerCertCheck::succeed_if_exists_and_fails_if_missing);

        RED_CHECK("succeed_if_exists_and_fails_if_missing"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<std::string>{}, ServerCertCheck(2)));
    }

    // set
    {
        RED_CHECK( is_valid_enum_value<ColorDepth>::is_valid(15));
        RED_CHECK(!is_valid_enum_value<ColorDepth>::is_valid(20));

        configs::spec_type<ColorDepth> spec{};

        ColorDepth v{};
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "15"_zv));
        RED_CHECK(v == ColorDepth::depth15);
        RED_CHECK(no_parse_error != parse_from_cfg(v, spec, "20"_zv));

        RED_CHECK("15"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<ColorDepth>{}, ColorDepth::depth15));

        configs::spec_type<std::string> sspec{};

        RED_CHECK(v != ColorDepth::depth32);
        RED_CHECK(no_parse_error == parse_from_cfg(v, sspec, "depth32"_zv));
        RED_CHECK(v == ColorDepth::depth32);
        RED_CHECK(no_parse_error != parse_from_cfg(v, sspec, "depth20"_zv));

        RED_CHECK("depth15"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<std::string>{}, ColorDepth::depth15));
    }

    // flags
    {
        RED_CHECK( is_valid_enum_value<CaptureFlags>::is_valid(15));
        RED_CHECK(!is_valid_enum_value<CaptureFlags>::is_valid(16));

        configs::spec_type<CaptureFlags> spec{};

        CaptureFlags v{};
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "15"_zv));
        RED_CHECK(v == CaptureFlags(15));
        RED_CHECK(no_parse_error != parse_from_cfg(v, spec, "16"_zv));

        RED_CHECK("15"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<CaptureFlags>{}, CaptureFlags(15)));
    }
}

RED_AUTO_TEST_CASE(TestColorParser)
{
    using Rgb = ::configs::spec_types::rgb;

    Rgb rgb;
    configs::spec_type<Rgb> stype;

    auto const h32 = ut::hex_int{8};
#ifndef BOOST_TEST
    void BOOST_TEST(bool, ut::hex_int);
#endif
#define TEST_RGB(...) BOOST_TEST(__VA_ARGS__, h32)

    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "0x123456"_zv));
    TEST_RGB(rgb.to_rrggbb() == 0x123456);
    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "0x123"_zv));
    TEST_RGB(rgb.to_rrggbb() == 0x123);
    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "0xaB2"_zv));
    TEST_RGB(rgb.to_rrggbb() == 0xAB2);
    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "#123456"_zv));
    TEST_RGB(rgb.to_rrggbb() == 0x123456);
    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "#123"_zv));
    TEST_RGB(rgb.to_rrggbb() == 0x112233);
    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "red"_zv));
    TEST_RGB(rgb.to_rrggbb() == BGRColor(BGRasRGBColor(RED)).as_u32());
    RED_CHECK(no_parse_error == parse_from_cfg(rgb, stype, "inv_medium_green"_zv));
    TEST_RGB(rgb.to_rrggbb() == BGRColor(BGRasRGBColor(INV_MEDIUM_GREEN)).as_u32());

    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, "bla_bla"_zv));
    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, "0x1234567"_zv));
    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, "#1234567"_zv));
    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, "#1234"_zv));
    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, "#"_zv));
    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, "0"_zv));
    RED_CHECK(no_parse_error != parse_from_cfg(rgb, stype, ""_zv));

#undef TEST_RGB
}

RED_AUTO_TEST_CASE(TestOtherParser)
{
    // unsigned
    {
        unsigned u;
        configs::spec_type<unsigned> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0"_zv));
        RED_CHECK(0 == u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "3"_zv));
        RED_CHECK(3 == u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x3"_zv));
        RED_CHECK(3 == u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x00000007"_zv));
        RED_CHECK(7 == u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0000000000000007"_zv));
        RED_CHECK(7 == u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0007"_zv));
        RED_CHECK(7 == u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "1357"_zv));
        RED_CHECK(1357 == u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x1357"_zv));
        RED_CHECK(0x1357 == u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0A"_zv));
        RED_CHECK(0x0a == u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0a"_zv));
        RED_CHECK(0x0a == u);

        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "0x0000000I"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "-0"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "-1"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "0x"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "I"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, ""_zv));
    }

    // int
    {
        int i;
        configs::spec_type<int> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "3600"_zv));
        RED_CHECK(3600 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0xA3600"_zv));
        RED_CHECK(0xA3600 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0"_zv));
        RED_CHECK(0 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "-3600"_zv));
        RED_CHECK(-3600 == i);

        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "999999999999999999999999999999"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, ""_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "-0xA3600"_zv));
    }

    // int [-2, 10]
    {
        int i;
        configs::spec_type<configs::spec_types::range<int, -2, 10>> stype;

        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "-3"_zv));
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "-2"_zv));
        RED_CHECK(-2 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "-1"_zv));
        RED_CHECK(-1 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0"_zv));
        RED_CHECK(0 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "5"_zv));
        RED_CHECK(5 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "10"_zv));
        RED_CHECK(10 == i);
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "11"_zv));
    }

    // unsigned [2, 10]
    {
        unsigned i;
        configs::spec_type<configs::spec_types::range<unsigned, 2, 10>> stype;

        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "-1"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "0"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "1"_zv));
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "2"_zv));
        RED_CHECK(2 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "5"_zv));
        RED_CHECK(5 == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "10"_zv));
        RED_CHECK(10 == i);
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "11"_zv));
    }

    // std:chrono [0, 10]
    {
        using namespace std::chrono_literals;

        std::chrono::seconds i;
        configs::spec_type<configs::spec_types::range<std::chrono::seconds, 0, 10>> stype;

        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "-1"_zv));
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0"_zv));
        RED_CHECK(0s == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "1"_zv));
        RED_CHECK(1s == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "2"_zv));
        RED_CHECK(2s == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "5"_zv));
        RED_CHECK(5s == i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "10"_zv));
        RED_CHECK(10s == i);
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "11"_zv));
    }

    // FilePermissions
    {
        FilePermissions perms(0);
        configs::spec_type<FilePermissions> stype;

        // octal

        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "0000777"_zv));
        RED_CHECK(Octal{0777} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "0000123"_zv));
        RED_CHECK(Octal{0123} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "0777"_zv));
        RED_CHECK(Octal{0777} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "777"_zv));
        RED_CHECK(Octal{0777} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "123"_zv));
        RED_CHECK(Octal{0123} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "23"_zv));
        RED_CHECK(Octal{0023} == Octal{perms.permissions_as_uint()});

        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, ""_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "1234"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "288"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "-36"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "a66"_zv));

        // symbolic mode

        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "=rwx"_zv));
        RED_CHECK(Octal{0775} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "=rwxrrrxxw"_zv));
        RED_CHECK(Octal{0775} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "-r"_zv));
        RED_CHECK(Octal{0000} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "+rw"_zv));
        RED_CHECK(Octal{0664} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "u=rx"_zv));
        RED_CHECK(Octal{0500} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "g=rx"_zv));
        RED_CHECK(Octal{0050} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "o=rx"_zv));
        RED_CHECK(Octal{0005} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "a=rwx"_zv));
        RED_CHECK(Octal{0777} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "u=rw, g+r"_zv));
        RED_CHECK(Octal{0640} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "u=rw, g=r"_zv));
        RED_CHECK(Octal{0040} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "a=rwx, o-x"_zv));
        RED_CHECK(Octal{0776} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "=w, +x, "_zv));
        RED_CHECK(Octal{0331} == Octal{perms.permissions_as_uint()});
        RED_CHECK(no_parse_error == parse_from_cfg(perms, stype, "="_zv));
        RED_CHECK(Octal{0000} == Octal{perms.permissions_as_uint()});

        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "a=rwxo-x"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "=rwx=x"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "=a,"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "aa"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "a"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, "==w"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(perms, stype, ","_zv));
    }

    // fixed_binary
    {
        std::array<unsigned char, 4> d;
        configs::spec_type<configs::spec_types::fixed_binary> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(d, stype, "12345678"_zv));
        RED_CHECK("\x12\x34\x56\x78"_av == d);
        RED_CHECK(no_parse_error == parse_from_cfg(d, stype, "abcdefab"_zv));
        RED_CHECK("\xAB\xCD\xEF\xAB"_av == d);

        RED_CHECK(no_parse_error != parse_from_cfg(d, stype, "ababab"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(d, stype, "ababababab"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(d, stype, "aa-bb-cc"_zv));
    }
}
