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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "mod/rdp/channels/sespro_channel.hpp"

#include "acl/session_logfile.hpp"

#include "utils/parse_server_message.hpp"
#include "utils/genfstat.hpp"

#include "test_only/lcg_random.hpp"
#include "test_only/transport/test_transport.hpp"

RED_AUTO_TEST_CASE_WD(TestLog6, wd)
{
    char message[] =
/* 0000 */ "\x5f\x01\x42\x45\x53\x54\x53\x41\x46\x45\x5f\x53\x45\x52\x56\x49" // _.BESTSAFE_SERVI
/* 0010 */ "\x43\x45\x5f\x4c\x4f\x47\x3d\x4f\x70\x65\x72\x61\x74\x69\x6f\x6e" // CE_LOG=Operation
/* 0020 */ "\x01\x32\x30\x31\x01\x4c\x6f\x67\x54\x79\x70\x65\x01\x53\x45\x53" // .201.LogType.SES
/* 0030 */ "\x53\x49\x4f\x4e\x01\x50\x72\x6f\x64\x75\x63\x74\x4e\x61\x6d\x65" // SION.ProductName
/* 0040 */ "\x01\x53\x42\x53\x01\x50\x72\x6f\x64\x75\x63\x74\x4d\x6f\x64\x75" // .SBS.ProductModu
/* 0050 */ "\x6c\x65\x01\x53\x42\x53\x41\x47\x45\x4e\x54\x01\x50\x72\x6f\x64" // le.SBSAGENT.Prod
/* 0060 */ "\x75\x63\x74\x56\x65\x72\x73\x69\x6f\x6e\x01\x76\x34\x2e\x32\x2e" // uctVersion.v4.2.
/* 0070 */ "\x30\x2e\x32\x34\x01\x55\x54\x43\x44\x61\x74\x65\x01\x32\x30\x32" // 0.24.UTCDate.202
/* 0080 */ "\x30\x2e\x31\x31\x2e\x32\x37\x01\x55\x54\x43\x54\x69\x6d\x65\x01" // 0.11.27.UTCTime.
/* 0090 */ "\x31\x36\x3a\x31\x34\x3a\x30\x34\x01\x4c\x6f\x63\x61\x6c\x44\x61" // 16:14:04.LocalDa
/* 00a0 */ "\x74\x65\x01\x32\x30\x32\x30\x2e\x31\x31\x2e\x32\x37\x01\x4c\x6f" // te.2020.11.27.Lo
/* 00b0 */ "\x63\x61\x6c\x54\x69\x6d\x65\x01\x31\x37\x3a\x31\x34\x3a\x30\x34" // calTime.17:14:04
/* 00c0 */ "\x01\x49\x50\x41\x64\x64\x72\x65\x73\x73\x01\x31\x30\x2e\x31\x30" // .IPAddress.10.10
/* 00d0 */ "\x2e\x34\x35\x2e\x31\x35\x32\x01\x4f\x70\x65\x72\x61\x74\x69\x6f" // .45.152.Operatio
/* 00e0 */ "\x6e\x43\x6f\x64\x65\x01\x53\x45\x53\x53\x49\x4f\x4e\x01\x48\x6f" // nCode.SESSION.Ho
/* 00f0 */ "\x73\x74\x4e\x61\x6d\x65\x01\x57\x49\x4e\x2d\x32\x30\x31\x39\x01" // stName.WIN-2019.
/* 0100 */ "\x55\x73\x65\x72\x4e\x61\x6d\x65\x01\x53\x59\x53\x54\x45\x4d\x01" // UserName.SYSTEM.
/* 0110 */ "\x52\x65\x73\x75\x6c\x74\x43\x6f\x64\x65\x01\x30\x78\x30\x30\x30" // ResultCode.0x000
/* 0120 */ "\x30\x30\x30\x30\x30\x01\x41\x63\x74\x69\x6f\x6e\x01\x52\x45\x4d" // 00000.Action.REM
/* 0130 */ "\x4f\x54\x45\x5f\x44\x49\x53\x43\x4f\x4e\x4e\x45\x43\x54\x01\x53" // OTE_DISCONNECT.S
/* 0140 */ "\x65\x73\x73\x69\x6f\x6e\x49\x64\x01\x33\x37\x01\x41\x63\x63\x6f" // essionId.37.Acco
/* 0150 */ "\x75\x6e\x74\x01\x32\x30\x31\x39\x51\x41\x5c\x75\x73\x65\x72\x31" // unt.2019QA.user1
/* 0160 */ "\x00"                                                             // .
;

    std::string              order_;
    // TODO vector<string_view>
    std::vector<std::string> parameters_;
    const bool parse_server_message_result =
        ::parse_server_message(message, order_, parameters_);

/*
    std::vector<KVLog> vecLog;
    for (size_t i = 0, c = parameters_.size() / 2; i < c; ++i) {
        LOG(LOG_INFO, "\"%s\" : \"%s\"", parameters_[i * 2], parameters_[i * 2 + 1]);
        vecLog.emplace_back(KVLog{parameters_[i * 2], parameters_[i * 2 + 1]});
    }
*/


    Inifile ini;
    ini.clear_send_index();

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    TimeBase timebase({0, 0});

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

    ini.set<cfg::capture::record_filebase>("log5_6");
    ini.set<cfg::video::record_path>(wd.dirname().string());
    ini.set<cfg::video::hash_path>(hashdir.dirname().string());

    GeneratorTransport trans(""_av);
    std::string session_type;

    Sesman sesman(ini, timebase);

    auto notify_error = [&sesman](const Error & error)
    {
        if (error.errnum == ENOSPC) {
            // error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
            sesman.report("FILESYSTEM_FULL", "100|unknown");
        }
    };

    SessionLogFile log_file(ini, timebase, cctx, rnd, fstat, notify_error);

    log_file.open_session_log();

    std::vector<KVLog> vecLog;
    for (size_t i = 0, c = parameters_.size() / 2; i < c; ++i) {
        vecLog.emplace_back(KVLog{parameters_[i * 2], parameters_[i * 2 + 1]});
    }

    log_file.log6(LogId::BESTSAFE_SERVICE_LOG, {vecLog});

    KVListBuffer buffer;
    buffer.append(timeval{1, 0}, LogId::BESTSAFE_SERVICE_LOG, {vecLog});

    auto it = buffer.begin();
    RED_REQUIRE((it != buffer.end()));

    KVListBuffer::KVEvent kv_event {};
    kv_event = *it;
    RED_CHECK(kv_event.id == LogId::BESTSAFE_SERVICE_LOG);
    RED_CHECK(kv_event.time.tv_sec == 1);
    RED_CHECK(kv_event.time.tv_usec == 0);
    RED_REQUIRE(kv_event.kv_list.size() == 17);

    RED_CHECK(kv_event.kv_list[0].key == "Operation"_av);
    RED_CHECK(kv_event.kv_list[0].value == "201"_av);
    RED_CHECK(kv_event.kv_list[16].key == "Account"_av);
    RED_CHECK(kv_event.kv_list[16].value == "2019QA\\user1"_av);
}
