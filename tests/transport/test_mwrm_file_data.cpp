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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "transport/mwrm_file_data.hpp"

#include <string_view>

using namespace std::string_view_literals;


constexpr auto is_encrypted = InCryptoTransport::EncryptionMode::Encrypted;
constexpr auto is_not_encrypted = InCryptoTransport::EncryptionMode::NotEncrypted;

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_not_encrypted)
{
    CryptoContext cctx;
    RED_CHECK_EXCEPTION_ERROR_ID(load_mwrm_file_data(
        FIXTURES_PATH "/sample.mwrm", cctx, is_encrypted), ERR_TRANSPORT_READ_FAILED);
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1)
{
    CryptoContext cctx;
    MwrmFileData mwrm_data = load_mwrm_file_data(
        FIXTURES_PATH "/sample.mwrm", cctx, is_not_encrypted);
    RED_TEST(mwrm_data.encryption_mode == is_not_encrypted);
    RED_TEST(mwrm_data.header.version == WrmVersion::v1);
    RED_TEST(mwrm_data.header.has_checksum == false);
    RED_REQUIRE(mwrm_data.wrms.size() == 3);
    RED_CHECK("./tests/fixtures/sample0.wrm"sv == mwrm_data.wrms[0].filename);
    RED_CHECK(1352304810 == mwrm_data.wrms[0].start_time);
    RED_CHECK(1352304870 == mwrm_data.wrms[0].stop_time);
    RED_CHECK(0 == mwrm_data.wrms[0].size);
    RED_CHECK("./tests/fixtures/sample1.wrm"sv == mwrm_data.wrms[1].filename);
    RED_CHECK(1352304870 == mwrm_data.wrms[1].start_time);
    RED_CHECK(1352304930 == mwrm_data.wrms[1].stop_time);
    RED_CHECK(0 == mwrm_data.wrms[1].size);
    RED_CHECK("./tests/fixtures/sample2.wrm"sv == mwrm_data.wrms[2].filename);
    RED_CHECK(1352304930 == mwrm_data.wrms[2].start_time);
    RED_CHECK(1352304990 == mwrm_data.wrms[2].stop_time);
    RED_CHECK(0 == mwrm_data.wrms[2].size);

    RED_TEST(is_not_encrypted == load_mwrm_file_data(
        FIXTURES_PATH "/sample.mwrm", cctx, InCryptoTransport::EncryptionMode::Auto).encryption_mode);
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_v2)
{
    CryptoContext cctx;
    MwrmFileData mwrm_data = load_mwrm_file_data(
        FIXTURES_PATH "/sample_v2.mwrm", cctx, is_not_encrypted);
    RED_TEST(mwrm_data.encryption_mode == is_not_encrypted);
    RED_TEST(mwrm_data.header.version == WrmVersion::v2);
    RED_TEST(mwrm_data.header.has_checksum == false);
    RED_REQUIRE(mwrm_data.wrms.size() == 3);
    RED_CHECK("./tests/fixtures/sample0.wrm"sv == mwrm_data.wrms[0].filename);
    RED_CHECK(1352304810 == mwrm_data.wrms[0].start_time);
    RED_CHECK(1352304870 == mwrm_data.wrms[0].stop_time);
    RED_CHECK(1 == mwrm_data.wrms[0].size);
    RED_CHECK("./tests/fixtures/sample1.wrm"sv == mwrm_data.wrms[1].filename);
    RED_CHECK(1352304870 == mwrm_data.wrms[1].start_time);
    RED_CHECK(1352304930 == mwrm_data.wrms[1].stop_time);
    RED_CHECK(9 == mwrm_data.wrms[1].size);
    RED_CHECK("./tests/fixtures/sample2.wrm"sv == mwrm_data.wrms[2].filename);
    RED_CHECK(1352304930 == mwrm_data.wrms[2].start_time);
    RED_CHECK(1352304990 == mwrm_data.wrms[2].stop_time);
    RED_CHECK(16 == mwrm_data.wrms[2].size);
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM1_v2_checksumed)
{
    CryptoContext cctx;
    MwrmFileData mwrm_data = load_mwrm_file_data(
        FIXTURES_PATH "/sample_v2_checksum.mwrm", cctx, is_not_encrypted);
    RED_TEST(mwrm_data.encryption_mode == is_not_encrypted);
    RED_TEST(mwrm_data.header.version == WrmVersion::v2);
    RED_TEST(mwrm_data.header.has_checksum == true);
    RED_REQUIRE(mwrm_data.wrms.size() == 3);
    RED_CHECK("./tests/fixtures/sample0.wrm"sv == mwrm_data.wrms[0].filename);
    RED_CHECK(1352304810 == mwrm_data.wrms[0].start_time);
    RED_CHECK(1352304870 == mwrm_data.wrms[0].stop_time);
    RED_CHECK(1 == mwrm_data.wrms[0].size);
    RED_CHECK("./tests/fixtures/sample1.wrm"sv == mwrm_data.wrms[1].filename);
    RED_CHECK(1352304870 == mwrm_data.wrms[1].start_time);
    RED_CHECK(1352304930 == mwrm_data.wrms[1].stop_time);
    RED_CHECK(9 == mwrm_data.wrms[1].size);
    RED_CHECK("./tests/fixtures/sample2.wrm"sv == mwrm_data.wrms[2].filename);
    RED_CHECK(1352304930 == mwrm_data.wrms[2].start_time);
    RED_CHECK(1352304990 == mwrm_data.wrms[2].stop_time);
    RED_CHECK(16 == mwrm_data.wrms[2].size);
}

RED_AUTO_TEST_CASE(TestSequenceFollowedTransportWRM_encrypted)
{
    uint8_t hmac_key[] = {
        0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
        0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
        0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
        0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };

    auto trace_fn = [](uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme) {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
        uint8_t trace_key[] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2 };
        static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH );
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    };

    CryptoContext cctx;
    cctx.set_hmac_key(hmac_key);
    cctx.set_get_trace_key_cb(trace_fn);
    cctx.set_master_derivator(cstr_array_view(
        "toto@10.10.43.13,Administrateur@QA@cible,"
        "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"
    ));
    MwrmFileData mwrm_data = load_mwrm_file_data(
        FIXTURES_PATH "/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        cctx, is_encrypted);
    RED_TEST(mwrm_data.encryption_mode == is_encrypted);
    RED_TEST(mwrm_data.header.version == WrmVersion::v2);
    RED_TEST(mwrm_data.header.has_checksum == true);
    RED_REQUIRE(mwrm_data.wrms.size() == 1);
    RED_CHECK("/var/wab/recorded/rdp/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335-000000.wrm"sv == mwrm_data.wrms[0].filename);
    RED_CHECK(1455816611 == mwrm_data.wrms[0].start_time);
    RED_CHECK(1455816633 == mwrm_data.wrms[0].stop_time);
    RED_CHECK(163032 == mwrm_data.wrms[0].size);
}
