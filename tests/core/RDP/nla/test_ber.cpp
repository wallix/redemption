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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include <ostream>

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/nla/ber.hpp"
#include "utils/sugar/bounded_array_view.hpp"
#include "utils/strutils.hpp"

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include "utils/sugar/int_to_chars.hpp"
# include "test_only/test_framework/compare_collection.hpp"
# include <cstring>

namespace
{

using PairSizeAv = std::pair<uint32_t, bytes_view>;

static ut::assertion_result test_comp_pair(PairSizeAv a, PairSizeAv b)
{
    ut::assertion_result ar(true);

    if (REDEMPTION_UNLIKELY(!(
        a.first == b.first
     && a.second.size() == b.second.size()
     && 0 == std::memcmp(a.second.data(), b.second.data(), a.second.size())
    ))) {
        ar = false;

        auto put = [&](std::ostream& out, PairSizeAv const& x){
            out << "{" << x.first << ", ";
            ut::put_view(x.second.size(), out,
                ut::flagged_bytes_view{x.second, ut::PatternView::hex}
            );
            out << "}";
        };

        auto& out = ar.message().stream();
        out << "[";
        ut::put_data_with_diff(out, a, "!=", b, put);
        out << "]";
    }

    return ar;
}

}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::PairSizeAv), (::PairSizeAv), ::test_comp_pair)
#endif

RED_AUTO_TEST_CASE(TestBERInteger)
{
    auto ber_pop_integer = [](chars_view av){
        return BER::pop_integer(av, "Integer", ERR_CREDSSP_TS_REQUEST);
    };

    RED_CHECK_EQUAL(BER::mkIntegerField(114178754, 0), "\xa0\x06\x02\x04\x06\xce\x3a\xc2"_av_hex);
    RED_CHECK_EQUAL(PairSizeAv(114178754, ""_av), ber_pop_integer("\x02\x04\x06\xce\x3a\xc2"_av));

    RED_CHECK_EQUAL(BER::mkIntegerField(1, 0), "\xa0\x03\x02\x01\x01"_av_hex);
    RED_CHECK_EQUAL(PairSizeAv(1, ""_av), ber_pop_integer("\x02\x01\x01"_av));

    RED_CHECK_EQUAL(BER::mkIntegerField(52165, 0), "\xa0\x05\x02\x03\x00\xcb\xc5"_av_hex);
    RED_CHECK_EQUAL(PairSizeAv(52165, ""_av), ber_pop_integer("\x02\x03\x00\xcb\xc5"_av));

    RED_CHECK_EQUAL(BER::mkIntegerField(0x0FFF, 0), "\xa0\x04\x02\x02\x0f\xff"_av_hex);
    RED_CHECK_EQUAL(PairSizeAv(0x0FFF, ""_av), ber_pop_integer("\x02\x02\x0f\xff"_av));
}

RED_AUTO_TEST_CASE(TestBEROctetString)
{
    auto oct_str = "1234567"_sized_av;

    RED_CHECK_EQUAL(BER::mkOctetStringHeader(oct_str.size()), "\x04\x07"_av_hex);

    RED_CHECK_EQUAL(PairSizeAv(7, oct_str), BER::pop_tag_length("\x04\x07""1234567"_av,
        BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING,
        "TestOctetString", ERR_CREDSSP_TS_REQUEST));
}

RED_AUTO_TEST_CASE(TestBEROctetString127)
{
    auto oct_str = "1234567890123456789012345678901234567890"
                   "1234567890123456789012345678901234567890"
                   "1234567890123456789012345678901234567890"
                   "1234567"_sized_av;

    RED_CHECK_EQUAL(BER::mkOctetStringHeader(oct_str.size()), "\x04\x7f"_av_hex);

    RED_CHECK_EQUAL(PairSizeAv(127, oct_str), BER::pop_tag_length(str_concat("\x04\x7f", oct_str),
        BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING,
        "TestOctetString", ERR_CREDSSP_TS_REQUEST));
}

RED_AUTO_TEST_CASE(TestBEROctetString128)
{
    auto oct_str = "1234567890123456789012345678901234567890"
                   "1234567890123456789012345678901234567890"
                   "1234567890123456789012345678901234567890"
                   "12345678"_sized_av;

    RED_CHECK_EQUAL(BER::mkOctetStringHeader(oct_str.size()), "\x04\x81\x80"_av_hex);

    RED_CHECK_EQUAL(PairSizeAv(128, oct_str), BER::pop_tag_length(str_concat("\x04\x81\x80", oct_str),
        BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OCTET_STRING,
        "TestOctetString", ERR_CREDSSP_TS_REQUEST));
}

RED_AUTO_TEST_CASE(TestBEROid)
{
    auto oct_str = "1234567"_sized_av;

    RED_CHECK_EQUAL(BER::mkOidField(oct_str, BER::TAG_OBJECT_IDENTIFIER),
        "\xa6\x09\x06\x07\x31\x32\x33\x34\x35\x36\x37"_av_hex);

    RED_CHECK_EQUAL(PairSizeAv(7, oct_str), BER::pop_tag_length("\x06\x07""1234567"_av,
        BER::CLASS_UNIV|BER::PC_PRIMITIVE|BER::TAG_OBJECT_IDENTIFIER,
        "TestOid", ERR_CREDSSP_TS_REQUEST));
}

RED_AUTO_TEST_CASE(TestBERContextual)
{
    uint8_t tag = 0x06;

    auto data = "012"_sized_av;

    RED_CHECK_EQUAL(BER::mkContextualFieldHeader(data.size(), tag), "\xa6\x03"_av_hex);

    RED_CHECK(BER::check_ber_ctxt_tag("\xa6\x03""012"_av, tag));
    RED_CHECK_EQUAL(PairSizeAv(3, data), BER::pop_length("\x03""012"_av,
        "TSBER", ERR_CREDSSP_TS_REQUEST));
}
