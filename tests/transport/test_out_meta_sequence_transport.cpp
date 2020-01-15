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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "transport/out_meta_sequence_transport.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/fake_stat.hpp"

RED_AUTO_TEST_CASE(TestMetaSequenceTransport)
{
    WorkingDirectory hash_wd("hash");
    WorkingDirectory recorded_wd("recorded");

    {
        CryptoContext cctx;
        cctx.set_master_key(cstr_array_view(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
        ));
        cctx.set_hmac_key(cstr_array_view("12345678901234567890123456789012"));

        LCGRandom rnd;
        FakeFstat fstat;
        timeval tv;
        tv.tv_usec = 0;
        tv.tv_sec = 1352304810;
        const int groupid = 0;

        cctx.set_trace_type(TraceType::cryptofile);

        OutMetaSequenceTransport crypto_trans(cctx, rnd, fstat, recorded_wd.dirname(), hash_wd.dirname(), "TESTOFS", tv, 800, 600, groupid, nullptr);
        crypto_trans.send("AAAAX", 5);
        crypto_trans.disconnect();
        // check no exception in dtor of OutMetaSequenceTransport
    }

    (void)hash_wd.add_file("TESTOFS.mwrm");
    (void)hash_wd.add_file("TESTOFS-000000.wrm");
    (void)recorded_wd.add_file("TESTOFS.mwrm");
    (void)recorded_wd.add_file("TESTOFS-000000.wrm");

    RED_CHECK_WORKSPACE(hash_wd);
    RED_CHECK_WORKSPACE(recorded_wd);
}
